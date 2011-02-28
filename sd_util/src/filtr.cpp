/*-----------------------------------------------------------------------------
 filtr.cpp
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <orbmodel.hpp>
#include <c_lib.hpp>
#include <Log.hpp>

#include "auto_ptr.hpp"
#include "astronom.hpp"

#ifndef LOG_FORMAT
#define LOG_FORMAT "%d %t %e %m"
#endif

using namespace libconfig;

const char* useMsg = "\
 Использование: filtr [опции] имя_конфигурации файл_данных\n\
 файл_данных     Файл корректированных температур\n\
 (результат работы программы multich).\n\
 Опции:\n\
 -s[1]           Выводить в статистику\n\
 -s0             Не выводить в статистику\n\
 Опции, управляющие печатью сообщений в log-файл:\n\
 -llevel={dump,debug,info,warning,error,fatal} уровень вывода сообщений.\n\
 -lerr           Выводить сообщения в стандартный поток ошибок.\n\
 -lout           Выводить сообщения в стандартный поток вывода.\n\
 -lfile=<name>   Выводить сообщения в файл с именем name,\n\
                 файл создаётся заново\n\
 -lappend=<name> Выводить сообщения в файл с именем name,\n\
                 сообщения дописываются в конец файла\n";

// Значение переменной  среды TERM_ROOT
const char* term_root = ".";

/*
	Опции из коммандной строки имеют более высокий приоритет чем опции из файла конфигураций.
*/

nsCLog::eSeverity logLevel = nsCLog::info;
bool useStdErr = false;
bool useStdOut = false;
string logFileName = string();
bool append = false;
CLog *logfile;

// Значение, которым мы будем отмечать отфильтрованные пиксели
const int lostValue = -5;
const int multichLostValue = -4;
const int tempLostValue = -1000;

// Значения, которыми программа калибровки отмечает ошибочные пиксели
const short minLimitOutPoint = -12;    // значение меньше представимого
const short maxLimitOutPoint = -13;    // значение больше представимого

// позиции отдельных фильтров в массиве статистики
const int MASK_ALBEDO_FILTR = 1;
const int MASK_TEMP_FILTR = 2;
const int MASK_34_DELTA_FILTR = 4;
const int MASK_35_DELTA_FILTR = 8;
const int MASK_45_DELTA_FILTR = 16;
const int MASK_TEMP_SMOOTH_FILTR = 32;
const int MASK_ALBEDO_SMOOTH_FILTR = 64;
const int N_MASK_VALUE = MASK_ALBEDO_SMOOTH_FILTR * 2 + 2;
const int MASK_MULTICH = N_MASK_VALUE - 2;
const int MASK_BORDER_FILTR = N_MASK_VALUE - 1;

//char option_s_present;

char exeFileName[MAX_PATH];    // argv[0]
char cfgName[MAX_FNAME];
//char cfgFileName[MAX_PATH];

// Имена файлов, используемых при фильтрации
char inputFileName[MAX_PATH];          // "nxxxxxx_m.mch"
char inputFileName2[MAX_PATH];         // "nxxxxxx_2.clb"
char inputFileName3[MAX_PATH];         // "nxxxxxx_3.clb"
char inputFileName4[MAX_PATH];         // "nxxxxxx_4.clb"
char inputFileName5[MAX_PATH];         // "nxxxxxx_5.clb"
char outputFileName[MAX_PATH];         // "nxxxxxx_m.flt"

// Соответствующие нулевые блоки
TBlk0_AVHRR in_blk0;
TBlk0_AVHRR in2_blk0;
TBlk0_AVHRR in3_blk0;
TBlk0_AVHRR in4_blk0;
TBlk0_AVHRR in5_blk0;
TBlk0_AVHRR out_blk0;

// данные
short * in_data;
short * in2_data;
short * in3_data;
short * in4_data;
short * in5_data;
short * out_data;

// Минимальное значение синуса  угла восхождения на солнце
// при котором данным видимых каналов можно доверять
const double sinalpha_minlimit=10.0*DR;

char msg[64000];                      // Буфер для сообщений лога

//
// Параметры фильтрации облачности
//
struct TFiltrParams {
	// фильтрация по альбедо
	int albedo_flag;
	int assent_flag;    // при фильтрации по альбедо учитывать
	// угол восхождения на Солнце
	double max_albedo;
	double alb_precision;   // абсолютная точность определения альбедо

	// фильтрация по температуре
	int temp_flag;
	double min_temp, max_temp;

	// фильтрация по разности четвертого/пятого каналов
	int day_delta45_flag;
	double day_min_delta45,  day_max_delta45;
	int night_delta45_flag;
	double night_min_delta45,  night_max_delta45;

	// фильтрация по разности третьего/четвертого каналов
	int day_delta34_flag;
	double day_min_delta34,  day_max_delta34;
	int night_delta34_flag;
	double night_min_delta34,  night_max_delta34;

	// фильтрация по разности третьего/пятого каналов
	int day_delta35_flag;
	double day_min_delta35,  day_max_delta35;
	int night_delta35_flag;
	double night_min_delta35,  night_max_delta35;

	// фильтрация однородности по температуре
	int temp_uniformity_flag;
	double temp_uniformity_threshold;

	// фильтрация однородности по альбедо
	int albedo_uniformity_flag;
	double albedo_uniformity_threshold;

	// фильтрация границ облачности
	int cloud_border_flag;
	int cloud_border_win_size;
	double max_filtered_percent;

	// после окончания работы вывести статистику
	int stat_flag;
};

class TFiltr {
private:
	static TFiltrParams p;
	static TBlk0_AVHRR in_blk0;
	static TBlk0_AVHRR in2_blk0;
	static TBlk0_AVHRR in3_blk0;
	static TBlk0_AVHRR in4_blk0;
	static TBlk0_AVHRR in5_blk0;
	static TBlk0_AVHRR out_blk0;
	static short *in_data;
	static short *in2_data;
	static short *in3_data;
	static short *in4_data;
	static short *in5_data;
	static short *out_data;
	static int cols;
	static int scans;

	static int albedoTest( int i, int j, double ang );
	static int tempTest( int i, int j, double ang );
	static int delta34Test( int scan, int j, double ang );
	static int delta35Test( int scan, int j, double ang );
	static int delta45Test( int scan, int j, double ang );
	static int deltaTest( int scan, int j, double ang,
						  short* in1, short* in2,
						  TBlk0_AVHRR& b1, TBlk0_AVHRR& b2, double min_delta, double max_delta );
	static void calcUniformity( int scan, int j, double ang, short* in,
								TBlk0_AVHRR& b0, double * uniformity, int *tooBigDataFlag );
	static int tempUniformityTest( int scan, int j, double ang );
	static int albedoUniformityTest( int scan, int j, double ang );
	static void borderProcessing( int win_size, double max_persent, int *stat );
public:
	static int filtr_processing( struct TFiltrParams &p,
								 TBlk0_AVHRR &in_blk0,  short *in_data,
								 TBlk0_AVHRR &in2_blk0, short *in2_data,
								 TBlk0_AVHRR &in3_blk0, short *in3_data,
								 TBlk0_AVHRR &in4_blk0, short *in4_data,
								 TBlk0_AVHRR &in5_blk0, short *in5_data,
								 TBlk0_AVHRR &out_blk0, short *out_data,
								 int *filtr_stat = 0 );
};
TBlk0_AVHRR TFiltr::in_blk0;
TBlk0_AVHRR TFiltr::in2_blk0;
TBlk0_AVHRR TFiltr::in3_blk0;
TBlk0_AVHRR TFiltr::in4_blk0;
TBlk0_AVHRR TFiltr::in5_blk0;
TBlk0_AVHRR TFiltr::out_blk0;
short *TFiltr::in_data;
short *TFiltr::in2_data;
short *TFiltr::in3_data;
short *TFiltr::in4_data;
short *TFiltr::in5_data;
short *TFiltr::out_data;
TFiltrParams TFiltr::p;
int TFiltr::cols;
int TFiltr::scans;

#include "filtr.hpp"

int main(int argc, char *argv[]) {

	if( argc < 3 ) {
		printf( "%s", useMsg );
		return 0;
	}

	char * t;
	if(NULL!=(t = getenv("TERM_ROOT"))){
		term_root = strdup(t);
	}else{
		fprintf( stderr, "Переменная TERM_ROOT не установлена, будет использован текущий путь.\n" );
	}

	strcpy( exeFileName, argv[0] );
	strcpy( inputFileName, argv[argc-1] );
	strcpy( cfgName, argv[argc-2] );

	TFiltrParams filtrParams;

	try {
		readCfg( filtrParams );
		parseCommandString( argc, argv, filtrParams );
	} catch (TException e) {
		fprintf( stderr, "%s\n", e.what() );
		exit(-1);
	}

	logfile = new CLog("filtr.exe");
	if(useStdOut) {
		//logfile.addAppender(new ConsoleLogAppender(logLevel,"%s | %e | %m"));
		//logfile->addAppender(new CConsoleLogAppender(logLevel, "%e | %m"));
		logfile->addAppender(new CConsoleLogAppender(logLevel, LOG_FORMAT));
	}
	if(useStdErr) {
		//logfile.addAppender(new EConsoleLogAppender(logLevel,"%s | %e | %m"));
		//logfile->addAppender(new EConsoleLogAppender(logLevel, "%e | %m"));
		logfile->addAppender(new EConsoleLogAppender(logLevel, LOG_FORMAT));
	}
	if(!logFileName.empty()) {
		try {
			//logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, "%s | %e | %m"));
			logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, LOG_FORMAT));
		} catch(string e) {
			fprintf(stderr,"%s\n",e.c_str());
			exit(1);
		}
	}

	logfile->info( string("filtr.exe - ") + inputFileName );

	{ // Для того, чтобы был вызван деструктор logfile
		TAutoPtr<CLog> aLogFile( logfile );

		logfile->info( "чтение входных файлов" );
		construct_file_names();
		if( read_and_verify_Blk0s(filtrParams) != 0 ) {
			return 1;
		}
		int length = in_blk0.totalFrameNum * in_blk0.totalPixNum;
		if( readData(filtrParams, length ) != 0 ) {
			return 1;
		}

		// для автоматического удаления данных по окончанию работы
		TAutoPtr<short> a_in_data(in_data);
		TAutoPtr<short> a_in2_data(in2_data);
		TAutoPtr<short> a_in3_data(in3_data);
		TAutoPtr<short> a_in4_data(in4_data);
		TAutoPtr<short> a_in5_data(in5_data);

		// для результата
		out_data = new short[length];
		TAutoPtr<short> a_out_data(out_data);
		out_blk0 = in_blk0;

		int *filtr_stat = new int[N_MASK_VALUE];
		TAutoPtr<int> a_filtr_stat(filtr_stat);
		for( int i = 0; i < N_MASK_VALUE; i++ ) {
			filtr_stat[i] = 0;
		}

		logfile->info( "фильтрация облачности" );
		try {
			// собственно фильтрация
			int res = TFiltr::filtr_processing(
						  filtrParams,
						  in_blk0,  in_data,
						  in2_blk0, in2_data,
						  in3_blk0, in3_data,
						  in4_blk0, in4_data,
						  in5_blk0, in5_data,
						  out_blk0, out_data,
						  filtr_stat);
			if( res != 0 ) {
				return 1;
			}
			// фильтрация завершена
		} catch (TRequestExc& e) {
			logfile->error( e.text() );
			return 1;
		}

		if( filtrParams.stat_flag ) {
			printStats( filtr_stat );
		}

		logfile->info( "коррекция нулевого блока" );
		if( correctData( out_data, out_blk0 ) != 0 ) {
			return 1;
		}

		logfile->info( "запись результатов фильтрации" );
		if( writeFileNOAA( outputFileName, out_blk0, out_data, out_blk0.totalFrameNum * out_blk0.totalPixNum ) != 0 ) {
			return 1;
		}
		logfile->info( "" );
		logfile->info( "работа программы завершена успешно !" );
	}	// Для того, чтобы был вызван деструктор logfile
	return 0;
}
/*
* конец функции main()
*/

void parseCommandString( int argc, char* argv[], TFiltrParams& params ) throw (TException) {
	for( int i = 1; i < argc; i++ ) {
		char * s = argv[i];
		if( *s == '-' ) {
			s++;
			if( *s == 'l' ) {
				s++;
				if(!strcmp(s,"err"))
					useStdErr = true;
				else if(!strcmp(s,"out"))
					useStdOut = true;
				else if(!strncmp(s,"file=",5)) {
					s+=5;
					if(strlen(s)==0)
						throw TException(100, "Недопустимое значение параметра -lfile=<file name>");
					logFileName.assign((char*)s);
					append = false;
				} else if(!strncmp(s,"append=",7)) {
					s+=7;
					if(strlen(s)==0)
						throw TException(100, "Недопустимое значение параметра -lappend=<file name>");
					logFileName.assign((char*)s);
					append = true;
				} else if(!strncmp(s,"level=",6)) {
					s+=6;
					if(nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(string(s))))
						throw TException(100, "Недопустимое значение параметра -llevel=<message level>");
				} else {
					sprintf(msg,"Неизвестный параметр %s",argv[i]);
					throw TException(100, msg);
				}
			} else if( *s == 's' ) {
				/* опция -a */
				s++;
				if(*s == '\0' || *s == '1')
					params.stat_flag = true;
				else if(*s == '0')
					params.stat_flag = false;
				else {
					sprintf(msg,"Неизвестный параметр %s",argv[i]);
					throw TException(100, msg);
				}
			} else {
				/* указана неизвестная опция */
				sprintf(msg,"Неизвестный параметр %s",argv[i]);
				throw TException(100, msg);
			}
		}
	}
}

void readCfg( TFiltrParams& params ) throw (TException) {
	char path[MAX_PATH];
	const char * s;

	if((NULL == strchr(cfgName,'\\'))&&
			(NULL == strchr(cfgName,'/'))&&
			(NULL == strchr(cfgName,'.'))) {
		strcpy(path,term_root);
		int t = strlen(path);
		/* Если надо, в конце добавляем разделитель */
		if(path[t-1]!='/'||path[t-1]!='\\'){
			path[t] = DIRD; path[t+1] = '\0';
		}
#if DIRD == '/'
		strcat( path, "cfg/" );
#else
		strcat( path, "cfg\\" );
#endif
		strcat( path, cfgName );
		strcat( path, ".cfg" );
	} else {
		strcpy(path,cfgName);
	}

 Config conf;
	conf.readFile(path);
	delete path; 
	//const Setting& root = cfg.getRoot();

	const Setting& Log = conf.getRoot()["Log"];

	//string name = cfg.lookup("name");

	#warning повторяеться код
	string sLogLevel;
	if (Log.lookupValue("level", sLogLevel))
	{
		#warning подмать могет передлать функцию
		if( nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(sLogLevel)))
			logLevel = nsCLog::info;
	}

	Log.lookupValue("stderr", useStdErr);
	Log.lookupValue("stdout", useStdOut);

	append = Log.lookupValue("append", logFileName);

	if (Log.lookupValue("file", logFileName))
		append = false;

	// Здесь должно быть чтение параметров фильтрации
	readFiltrParams( conf.getRoot()["Filtr"], params );
}

void readFiltrParams( const Setting& cfg, TFiltrParams &p ) throw (TException) {
	cfg.lookupValue("albedo_enable",  p.albedo_flag );
	cfg.lookupValue("assent_albedo_enable",  p.assent_flag );
	cfg.lookupValue("max_albedo_value",  p.max_albedo);//readDbl( cfg, "FILTR_", p.max_albedo );
	cfg.lookupValue("albedo_precision",  p.alb_precision ); //readDbl( cfg, "FILTR_", p.alb_precision );
	cfg.lookupValue("temp_enable",  p.temp_flag); //readFlg( cfg, "FILTR_", p.temp_flag );
	cfg.lookupValue("max_temp",  p.max_temp); //readDbl( cfg, "FILTR_",  );
	cfg.lookupValue("min_temp",  p.min_temp ); //readDbl( cfg, "FILTR_MIN_TEMP",);
	
	
	cfg.lookupValue("day_delta45_enable",  p.day_delta45_flag); //readFlg( cfg, "FILTR_DAY_DELTA45_ENABLE", p.day_delta45_flag);
	cfg.lookupValue("min_day_delta45",  p.day_min_delta45); //readDbl( cfg, "FILTR_MIN_DAY_DELTA45", p.day_min_delta45 );
	cfg.lookupValue("max_day_delta45",  p.day_max_delta45); //readDbl( cfg, "FILTR_MAX_DAY_DELTA45", p.day_max_delta45 );

	cfg.lookupValue("night_delta45_enable",  p.night_delta45_flag); //readFlg( cfg, "FILTR_NIGHT_DELTA45_ENABLE", p.night_delta45_flag);
	cfg.lookupValue("min_night_delta45",  p.night_min_delta45); //readDbl( cfg, "FILTR_MIN_NIGHT_DELTA45", p.night_min_delta45 );
	cfg.lookupValue("max_night_delta45",  p.night_max_delta45); //readDbl( cfg, "FILTR_MAX_NIGHT_DELTA45", p.night_max_delta45 );
	
	cfg.lookupValue("day_delta34_enable",  p.day_delta34_flag);
	cfg.lookupValue("min_day_delta34",  p.day_min_delta34);
	cfg.lookupValue("max_day_delta34",  p.day_max_delta34);

	cfg.lookupValue("night_delta34_enable",  p.night_delta34_flag);
	cfg.lookupValue("min_night_delta34",  p.night_min_delta34);
	cfg.lookupValue("max_night_delta34",  p.night_max_delta34);

	cfg.lookupValue("day_delta35_enable",  p.day_delta35_flag);
	cfg.lookupValue("min_day_delta35",  p.day_min_delta35);
	cfg.lookupValue("max_day_delta35",  p.day_max_delta35);

	cfg.lookupValue("night_delta35_enable",  p.night_delta35_flag);
	cfg.lookupValue("min_night_delta35",  p.night_min_delta35);
	cfg.lookupValue("max_night_delta35",  p.night_max_delta35);

	cfg.lookupValue("temp_uniformity_enable",  p.temp_uniformity_flag); //readFlg( cfg, "FILTR_TEMP_UNIFORMITY_ENABLE", p.temp_uniformity_flag);
	cfg.lookupValue("temp_uniformity_threshold",  p.temp_uniformity_threshold); //readDbl( cfg, "FILTR_TEMP_UNIFORMITY_THRESHOLD", p.temp_uniformity_threshold );
	cfg.lookupValue("albedo_uniformity_enable",  p.albedo_uniformity_flag); //readFlg( cfg, "FILTR_ALBEDO_UNIFORMITY_ENABLE", p.albedo_uniformity_flag);
	cfg.lookupValue("albedo_uniformity_threshold",  p.albedo_uniformity_threshold); //readDbl( cfg, "FILTR_ALBEDO_UNIFORMITY_THRESHOLD", p.albedo_uniformity_threshold );
	cfg.lookupValue("cloud_border_enable",  p.cloud_border_flag); //readFlg( cfg, "FILTR_CLOUD_BORDER_ENABLE", p.cloud_border_flag );
	cfg.lookupValue("cloud_border_window_size",  p.cloud_border_win_size); //readInt( cfg, "FILTR_CLOUD_BORDER_WINDOW_SIZE", p.cloud_border_win_size );
	cfg.lookupValue("max_filtered_percent",  p.max_filtered_percent); //readDbl( cfg, "FILTR_MAX_FILTERED_PERCENT", p.max_filtered_percent );
	cfg.lookupValue("stat_enable",  p.stat_flag); //readFlg( cfg, "FILTR_STAT_ENABLE", p.stat_flag );
}

void construct_file_names() {
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], fext[MAX_EXT]; //, s[_MAX_FNAME];
	splitpath( inputFileName, drive, dir, fname, fext );

	int t = strlen(fname)-1;
	fname[t] = '2';
	makepath( inputFileName2, drive, dir, fname, "clb" );
	fname[t] = '3';
	makepath( inputFileName3, drive, dir, fname, "clb" );
	fname[t] = '4';
	makepath( inputFileName4, drive, dir, fname, "clb" );
	fname[t] = '5';
	makepath( inputFileName5, drive, dir, fname, "clb" );
	fname[t] = 'm';
	makepath( outputFileName, drive, dir, fname, "flt" );

	logfile->debug( "исходные файлы:" );
	logfile->debug( inputFileName );
	logfile->debug( inputFileName2 );
	logfile->debug( inputFileName3 );
	logfile->debug( inputFileName4 );
	logfile->debug( inputFileName5 );

	logfile->debug( "файл результата:" );
	logfile->debug( outputFileName );
	return;
}

int read_and_verify_Blk0s( TFiltrParams& filtrParams ) {
	if( readBlk0( inputFileName, in_blk0 ) != 0) {
		return 1;
	}

	if( verifyBlk0(in_blk0, inputFileName) != 0 ) {
		return 1;
	}

	if( chan2_is_need(filtrParams) ) {
		if( readBlk0( inputFileName2, in2_blk0 ) != 0) {
			logfile->info( "отсутствует 2-ой канал" );
			chan2_disable( filtrParams );
		}
	}
	if( chan3_is_need(filtrParams) ) {
		if( readBlk0( inputFileName3, in3_blk0 ) != 0 ||
				isVisualChannel( in3_blk0 ) ) {
			logfile->info( "отсутствует 3-ий канал" );
			chan3_disable( filtrParams );
		}
	}
	if( chan4_is_need(filtrParams) ) {
		if( readBlk0( inputFileName4, in4_blk0 ) != 0) {
			return 1;
		}
	}
	if( chan5_is_need(filtrParams) ) {
		if( readBlk0( inputFileName5, in5_blk0 ) != 0) {
			return 1;
		}
	}
	return 0;
}

int chan2_is_need( TFiltrParams& f ) {
	if( f.albedo_flag || f.albedo_uniformity_flag )
		return 1;
	return 0;
}

void chan2_disable( TFiltrParams& f ) {
	if( f.albedo_flag ) {
		logfile->info( "отключаем фильтрацию по альбедо" );
		f.albedo_flag = 0;
	}
	if( f.albedo_uniformity_flag ) {
		logfile->info( "отключаем фильтрацию по неоднородности альбедо" );
		f.albedo_uniformity_flag = 0;
	}
	return;
}


int chan3_is_need( TFiltrParams& f ) {
	if( f.day_delta35_flag || f.night_delta35_flag ||
			f.day_delta34_flag || f.night_delta34_flag )
		return 1;
	return 0;
}

void chan3_disable( TFiltrParams& f ) {
	if( f.day_delta35_flag || f.night_delta35_flag ) {
		logfile->info( "отключаем фильтрацию по разности 3-го 5-го каналов" );
		f.day_delta35_flag = 0;
		f.night_delta35_flag = 0;
	}
	if( f.day_delta34_flag || f.night_delta34_flag ) {
		logfile->info( "отключаем фильтрацию по разности 3-го 4-го каналов" );
		f.day_delta34_flag = 0;
		f.night_delta34_flag = 0;
	}
	return;
}


int chan4_is_need( TFiltrParams& f ) {
	if( f.day_delta34_flag || f.night_delta34_flag ||
			f.day_delta45_flag || f.night_delta45_flag || f.temp_uniformity_flag )
		return 1;
	return 0;
}

int chan5_is_need( TFiltrParams& f ) {
	if( f.day_delta35_flag || f.night_delta35_flag ||
			f.day_delta45_flag || f.night_delta45_flag )
		return 1;
	return 0;
}

int readBlk0( const char * name, TBlk0_AVHRR &blk0 ) {
	FILE* f = fopen( name, "rb" );
	if( f == 0 ) {
		sprintf( msg, "ошибка открытия файла %s", name );
		logfile->error( msg );
		return 1;
	}
	if( fread( &blk0, sizeof(blk0), 1, f ) != 1 ) {
		sprintf( msg, "ошибка чтения файла %s", name );
		logfile->error( msg );
		return 1;
	}
	fclose( f );
	return 0;
}

int verifyBlk0( TBlk0_AVHRR & blk0, const char * name ) {
	if( blk0.b0.formatType == 0xFF
			&& blk0.b0.dataType1 == 2           // одноканальные данные
			&& blk0.b0.dataType2 == 1           // тип сенсора --- AVHRR
			&& (blk0.processLevel & 1) == 1     // калибровка  проведена
			&& (blk0.processLevel & 2) == 2     // атм.коррекция проведена
	  ) {
		return 0;
	}
	sprintf( msg, "неправильный формат нулевого блока файла %s", name );
	logfile->error( msg );
	return 1;
}

int readData( TFiltrParams& filtrParams, int length ) {
	in_data = new short[length];
	if( readDataFile( in_data, inputFileName, 2*length, 512 ) != 0) {
		return 1;
	}

	if( chan2_is_need(filtrParams) ) {
		in2_data = new short[length];
		if( readDataFile( in2_data, inputFileName2, 2*length, 512 ) != 0) {
			return 1;
		}
	}
	if( chan3_is_need(filtrParams) ) {
		in3_data = new short[length];
		if( readDataFile( in3_data, inputFileName3, 2*length, 512 ) != 0) {
			return 1;
		}
	}
	if( chan4_is_need(filtrParams) ) {
		in4_data = new short[length];
		if( readDataFile( in4_data, inputFileName4, 2*length, 512 ) != 0) {
			return 1;
		}
	}
	if( chan5_is_need(filtrParams) ) {
		in5_data = new short[length];
		if( readDataFile( in5_data, inputFileName5, 2*length, 512 ) != 0) {
			return 1;
		}
	}
	return 0;
}


int readDataFile( void * buf, const char * name, int len, int blk0_len = 0 ) {
	FILE * f = fopen( name, "rb" );
	if( f == 0 ) {
		sprintf( msg, "ошибка открытия файла %s", name );
		logfile->error( msg );
		return 1;
	}
	if( fseek( f, blk0_len, SEEK_SET ) != 0 ) {
		sprintf( msg, "ошибка позиционирования в файле %s", name );
		logfile->error( msg );
		return 1;
	}
	if( fread( buf, 1, len, f ) != (size_t)len ) {
		sprintf( msg, "ошибка чтения файла %s", name );
		logfile->error( msg );
		return 1;
	}
	fclose(f);
	return 0;
}

int writeFileNOAA( const char * name, TBlk0_AVHRR& blk0, short * data, int len ) {
	FILE * f = fopen( name, "wb" );
	if( f == 0 ) {
		sprintf( msg, "ошибка открытия файла %s для записи", name );
		logfile->error( msg );
		return 1;
	}
	if( fwrite( &blk0, sizeof(blk0), 1, f ) != 1 ) {
		sprintf( msg, "ошибка записи в файл %s", name );
		logfile->error( msg );
		return 1;
	}
	if( fwrite( data, 2, len, f ) != (size_t)len ) {
		sprintf( msg, "ошибка записи в файл %s", name );
		logfile->error( msg );
		return 1;
	}
	fclose(f);
	return 0;
}

/*
* Фильтрация облачности
*/
int TFiltr::filtr_processing( struct TFiltrParams &p,
							  TBlk0_AVHRR &in_blk0,  short *in_data,
							  TBlk0_AVHRR &in2_blk0, short *in2_data,
							  TBlk0_AVHRR &in3_blk0, short *in3_data,
							  TBlk0_AVHRR &in4_blk0, short *in4_data,
							  TBlk0_AVHRR &in5_blk0, short *in5_data,
							  TBlk0_AVHRR &out_blk0, short *out_data,
							  int *filtr_stat ) {
	// Перевод параметров в статические члены класса
	TFiltr::p = p;
	TFiltr::in_blk0 = in_blk0;
	TFiltr::in2_blk0 = in2_blk0;
	TFiltr::in3_blk0 = in3_blk0;
	TFiltr::in4_blk0 = in4_blk0;
	TFiltr::in5_blk0 = in5_blk0;
	TFiltr::out_blk0 = out_blk0;
	TFiltr::in_data = in_data;
	TFiltr::in2_data = in2_data;
	TFiltr::in3_data = in3_data;
	TFiltr::in4_data = in4_data;
	TFiltr::in5_data = in5_data;
	TFiltr::out_data = out_data;
	TFiltr::cols = in_blk0.totalPixNum;
	TFiltr::scans = in_blk0.totalFrameNum;

	static const int step = 128;
	int length = scans * cols;

	// навигационная подсистема
	double julian_date;
	TStraightReferencer* r = navigationSystemInit( (TBlk0&)in_blk0, &julian_date );
	TAutoPtr<TStraightReferencer> a_t(r);

	for( int i = 0; i < length; i++ )
		out_data[i] = 0;

	for (int scan = 0; scan < scans; scan++) {
		int col1 = 0;
		int col2 = col1 + step;
		double ang1 = angle( scan, col1, *r, julian_date );
		double ang2 = angle( scan, col2, *r, julian_date );
		for (int j = 0; j < cols; j++) {
			// Определяем синус угла восхождения на Солнце
			double ang;
			if( j == col1 ) {
				ang = ang1;
			} else if ( j == col2 ) {
				ang = ang2;
			} else if ( j > col2 ) {
				col1 = col2;
				col2 = col1 + step;
				ang1 = ang2;
				ang2 = angle( scan, col2, *r, julian_date );
				ang =  ang1+(j-col1)*(ang2-ang1)/double(step);
			} else {
				ang = ang1+(j-col1)*(ang2-ang1)/double(step);
			}
			// синус угла восхождения на Солнце определен

			int ind = scan*cols + j;
			out_data[ind] = in_data[ind];
			if( in_data[ind] < 0 ) {
				out_data[ind] = in_data[ind];
				if( filtr_stat && in_data[ind] == multichLostValue ) {
					filtr_stat[MASK_MULTICH]++;
				}
			} else {
				//  маска, показывающая каким фильтрам удовлетворил
				//  текущий пиксель
				unsigned int mask_filtr = 0;

				//  Фильтрация по альбедо
				if (p.albedo_flag) {
					if( albedoTest( scan, j, ang ) ) {
						out_data[ind] = lostValue;
						mask_filtr = MASK_ALBEDO_FILTR;
					}
				}

				//  Фильтрация по температуре
				if (p.temp_flag) {
					if( tempTest( scan, j, ang ) ) {
						out_data[ind] = lostValue;
						mask_filtr |= MASK_TEMP_FILTR;
					}
				}

				//  Фильтрация по разности третьего - четвертого каналов
				if (p.day_delta34_flag || p.night_delta34_flag ) {
					if( delta34Test( scan, j, ang ) ) {
						out_data[ind] = lostValue;
						mask_filtr |= MASK_34_DELTA_FILTR;
					}
				}

				//  Фильтрация по разности третьего - пятого каналов
				if (p.day_delta35_flag || p.night_delta35_flag ) {
					if( delta35Test( scan, j, ang ) ) {
						out_data[ind] = lostValue;
						mask_filtr |= MASK_35_DELTA_FILTR;
					}
				}

				//  Фильтрация по разности эго - пятого каналов
				if (p.day_delta45_flag || p.night_delta45_flag ) {
					if( delta45Test( scan, j, ang ) ) {
						out_data[ind] = lostValue;
						mask_filtr |= MASK_45_DELTA_FILTR;
					}
				}

				//  Фильтрация по критерию пространственной неоднородности для температуры
				if (p.temp_uniformity_flag ) {
					if( scan > 0 && scan < scans && j > 0 && j < cols ) {
						if( tempUniformityTest( scan, j, ang ) ) {
							out_data[ind] = lostValue;
							mask_filtr |= MASK_TEMP_SMOOTH_FILTR;
						}
					}
				}

				if (p.albedo_uniformity_flag ) {
					if( scan > 0 && scan < scans && j > 0 && j < cols ) {
						if( albedoUniformityTest( scan, j, ang ) ) {
							out_data[ind] = lostValue;
							mask_filtr |= MASK_ALBEDO_SMOOTH_FILTR;
						}
					}
				}

				if( p.stat_flag && out_data[ind] == lostValue) {
					out_data[ind] = -mask_filtr-256;
				}

				if(filtr_stat )
					filtr_stat[mask_filtr]++;   // Для статистики
			}
		}
	}   // Конец большого цикла по всем пикселям

	if( p.cloud_border_flag ) {
		borderProcessing( p.cloud_border_win_size, p.max_filtered_percent, &(filtr_stat[MASK_BORDER_FILTR]) );
	}
	out_blk0.processLevel |= 4;
	return 0;
}

/*************************************************\
Инициализация навигационной подсистемы.
\*************************************************/
TStraightReferencer* navigationSystemInit( const TBlk0& blk0, double *pjulian_date ) throw (TRequestExc) {
	try {
		TNOAAImageParams NOAAImageParams(blk0);
		TCorrectionParams corrParams(blk0);
		*pjulian_date =
			julian(NOAAImageParams.fYear, NOAAImageParams.fYearTime+1);
		TIniSatParams iniSatParams( blk0);
		TOrbitalModel orbitalModel(iniSatParams, NOAAImageParams.fYear, NOAAImageParams.fYearTime + 1, corrParams);
		return (new TStraightReferencer(iniSatParams, NOAAImageParams, corrParams));
	} catch(...) {
		throw TRequestExc( 1, "navigationSystemInit: ошибка инициализации навигационной подсистемы" );
	}
	//return 0;
}

/*----------------------------------------*\
ANGLE.C
Функция осуществляет вычисление угла
склонения Солнца для точки исходного снимка
(в градусах).
Вызов:
angle( nscan, col )
nscan - номер строки
col - номер пикселя в строке
Должны быть проинициализированы глобальные
переменные julian_date и satParams.
Кроме этого заранее вызвано функция ini_sgp8
\*----------------------------------------*/
double angle(int nscan, int col, TStraightReferencer& r, double julian_date) {
	double lat;
	double lon;
	double fi;
	r.xy2ll(col, nscan, &lon, &lat);
	lon = -lon;
	fi = shcrds(julian_date, lon, lat);
	return (fi);
}

int TFiltr::albedoTest( int i, int j, double ang ) {
	int ind = i*cols + j;
	if ( in2_data[ind] >= 0) {
		double value = in2_data[ind] * in2_blk0.ka + in2_blk0.kb;
		if( p.assent_flag ) {
			//            value += p.alb_precision;		// Это старый, классический код
			//            value /= sin(ang*DR);
			const double minAngle = -4;
			const double angScale = 0.2659;
			const double expScale = 0.4;
			const double constLimit = 1.0;
			if( ang < minAngle )
				return 0;
			double lim1 = 1.0/angScale*log(constLimit/expScale);
			if( ang < lim1 ) {
				double a = exp(angScale*ang)*expScale;
				if( value - p.alb_precision <= a )
					return 0;
				return 1;
			}
			if( ang < 1.2*RD/p.max_albedo ) {
				if( value - p.alb_precision <= constLimit )
					return 0;
				return 1;
			}
			value -= p.alb_precision;
			value /= sin(ang*DR);
		}
		if (value > p.max_albedo) {
			return 1;
		}
	} else if (in2_data[ind] == maxLimitOutPoint ) {
		return 1;
	}
	return 0;
}

int TFiltr::tempTest( int i, int j, double ang ) {
	int ind = i*cols + j;
	if (in_data[ind] >= 0) {
		double value = in_data[ind] * in_blk0.ka + in_blk0.kb;
		if ( value < p.min_temp || value > p.max_temp ) {
			return 1;
		}
	} else if ( in_data[ind] == minLimitOutPoint ||
				in_data[ind] == maxLimitOutPoint ) {
		return 1;
	}
	return 0;
}

int TFiltr::deltaTest( int i, int j, double ang,
					   short* in1, short* in2, TBlk0_AVHRR& b1, TBlk0_AVHRR& b2,
					   double min_delta, double max_delta ) {
	int ind = i*cols + j;
	if ( in1[ind] >= 0 && in2[ind] >= 0 ) {
		double value1 = in1[ind] * b1.ka + b1.kb;
		double value2 = in2[ind] * b2.ka + b2.kb;
		double delta = value1 - value2;
		if ( delta<min_delta || delta>max_delta ) {
			return 1;
		}
	} else if ( in1[ind] == minLimitOutPoint ||
				in1[ind] == maxLimitOutPoint ||
				in2[ind] == minLimitOutPoint ||
				in2[ind] == maxLimitOutPoint ) {
		return 1;
	}
	return 0;
}

int TFiltr::delta34Test( int scan, int j, double ang ) {
	if( ang > 3. ) {
		if( p.day_delta34_flag ) {
			return deltaTest( scan, j, ang,
							  in3_data, in4_data, in3_blk0, in4_blk0,
							  p.day_min_delta34, p.day_max_delta34 );
		}
	} else {
		if( p.night_delta34_flag ) {
			return deltaTest( scan, j, ang,
							  in3_data, in4_data, in3_blk0, in4_blk0,
							  p.night_min_delta34, p.night_max_delta34 );
		}
	}
	return 0;
}


int TFiltr::delta35Test( int scan, int j, double ang ) {
	if( ang > 3. ) {
		if( p.day_delta35_flag ) {
			return deltaTest( scan, j, ang,
							  in3_data, in5_data, in3_blk0, in5_blk0,
							  p.day_min_delta35, p.day_max_delta35 );
		}
	} else {
		if( p.night_delta35_flag ) {
			return deltaTest( scan, j, ang,
							  in3_data, in5_data, in3_blk0, in5_blk0,
							  p.night_min_delta35, p.night_max_delta35 );
		}
	}
	return 0;
}


int TFiltr::delta45Test( int scan, int j, double ang ) {
	if( ang > 3. ) {
		if( p.day_delta45_flag ) {
			return deltaTest( scan, j, ang,
							  in4_data, in5_data, in4_blk0, in5_blk0,
							  p.day_min_delta45, p.day_max_delta45 );
		}
	} else {
		if( p.night_delta45_flag ) {
			return deltaTest( scan, j, ang,
							  in4_data, in5_data, in4_blk0, in5_blk0,
							  p.night_min_delta45, p.night_max_delta45 );
		}
	}
	return 0;
}

void TFiltr::calcUniformity( int scan, int j, double ang, short* in,
							 TBlk0_AVHRR& b0, double * uniformity, int *tooBigDataFlag ) {
	double delta = 0;
	int k = 0;
	for( int i1 = -1; i1 <= 1; i1++ ) {
		for( int j1 = -1; j1 <= 1; j1++ ) {
			int i2 = scan + i1;
			int j2 = j + j1;
			int ind = i2 * cols + j2;
			if (in[ind] >= 0) {
				double value = in[ind]* b0.ka + b0.kb;
				delta += value;
				k++;
			} else if( in[ind] == minLimitOutPoint ||
					   in[ind] == maxLimitOutPoint ) {
				*tooBigDataFlag = 1;
				*uniformity = 0;
				return;
			}
		}
	}
	delta /= k;
	delta -= in[scan * cols + j] * b0.ka + b0.kb;
	*uniformity = delta;
	*tooBigDataFlag = 0;
	return;
}

int TFiltr::tempUniformityTest ( int scan, int j, double ang ) {
	if( in4_data[scan*cols+j] < 0 )
		return 0;
	double uniformity;
	int tooBigDelta;
	calcUniformity( scan, j, ang, in_data, in_blk0, &uniformity, &tooBigDelta );
	if( tooBigDelta )
		return 1;
	if( fabs(uniformity) > p.temp_uniformity_threshold )
		return 1;
	return 0;
}


int TFiltr::albedoUniformityTest( int scan, int j, double ang ) {
	if( in2_data[scan*cols+j] < 0 )
		return 0;
	double uniformity;
	int tooBigDelta;
	calcUniformity( scan, j, ang, in2_data, in2_blk0, &uniformity, &tooBigDelta );
	if( tooBigDelta )
		return 1;
	if( fabs(uniformity) > p.albedo_uniformity_threshold )
		return 1;
	return 0;
}


void TFiltr::borderProcessing( int win_size, double max_percent, int *stat ) {
	*stat = 0;
	// выбираем отступы перед и после текущей точки
	int before = win_size / 2;
	int after = win_size - before - 1;
	if (after < 0)
		after = 0;
	if (before < 0)
		before = 0;

	int before2 = before;
	if( before2 == 0 )
		before2 = 1;
	int after2 = after;
	if( after2 == 0 )
		after2 = 1;

	for (int i = before2; i < scans - after2; i++) {
		for (int j = before2; j < cols - after2; j++) {
			if (out_data[i * cols + j] >= 0) {
				int f = 0;
				for (int i1 = i - 1; i1 <= i + 1; i1++) {
					for (int j1 = j - 1; j1 <= j + 1; j1++) {
						short int val = out_data[i1 * cols + j1];
						if( val != tempLostValue && ( val == lostValue ||
													  val == multichLostValue ||
													  val <= -256 )
						  ) {
							out_data[i * cols + j] = tempLostValue;
							f = 1;
						}
					}
				}
				if ( f == 0 ) {
					long int n_filtred = 0;
					long int n_points = 0;
					for (int i1 = i - before; i1 <= i + after; i1++) {
						for (int j1 = j - before; j1 <= j + after; j1++) {
							short int val = out_data[i1 * cols + j1];
							if( val != tempLostValue && (
										val == lostValue ||
										val == multichLostValue ||
										val <= -256
									)
							  ) {
								n_filtred++;
							}
							if( val >= 0 || val == tempLostValue || val == lostValue || val == multichLostValue || val <= -256 ) {
								n_points++;
							}
						}
					}

					if (n_points > 0 &&
							(double(n_filtred)/n_points >= 0.01 * max_percent)) {
						out_data[i * cols + j] = tempLostValue;
						(*stat)++;
					}
				}
			}
		}
	}

	for (int i = before; i < scans - after; i++) {
		for (int j = before; j < cols - after; j++) {
			if (out_data[i * cols + j] == tempLostValue)
				out_data[i * cols + j] = lostValue;
		}
	}
}

void printStats( int *msk ) {
	int i;
	logfile->info("статистика");
	sprintf(msg, "%9d неотфильтровано", msk[0] - msk[MASK_BORDER_FILTR] );
	logfile->info(msg);

	logfile->info("по отдельным фильтрам");
	int f1[7];
	for (i = 0; i < 7; i++)
		f1[i] = 0;
	for (i = 1; i < N_MASK_VALUE-1; i++) {
		if ( msk[i] ) {
			if (i & MASK_ALBEDO_FILTR)
				f1[0] += msk[i];
			if (i & MASK_TEMP_FILTR)
				f1[1] += msk[i];
			if (i & MASK_34_DELTA_FILTR)
				f1[2] += msk[i];
			if (i & MASK_35_DELTA_FILTR)
				f1[3] += msk[i];
			if (i & MASK_45_DELTA_FILTR)
				f1[4] += msk[i];
			if (i & MASK_TEMP_SMOOTH_FILTR)
				f1[5] += msk[i];
			if (i & MASK_ALBEDO_SMOOTH_FILTR)
				f1[6] += msk[i];
		}
	}
	logfile->info( "count1 count2 descr" );
//	sprintf(msg, "%9d %9ld %s", f1[0], msk[MASK_ALBEDO_FILTR], "альбедо" );
	sprintf(msg, "%9d %9d %s", f1[0], msk[MASK_ALBEDO_FILTR], "альбедо" );
	logfile->info(msg);
//	sprintf(msg, "%9d %9ld %s", f1[1], msk[MASK_TEMP_FILTR], "температура" );
	sprintf(msg, "%9d %9d %s", f1[1], msk[MASK_TEMP_FILTR], "температура" );
	logfile->info(msg);
//	sprintf(msg, "%9d %9ld  %s", f1[2], msk[MASK_34_DELTA_FILTR], "разность температур 3-го и 4-го каналов" );
	sprintf(msg, "%9d %9d  %s", f1[2], msk[MASK_34_DELTA_FILTR], "разность температур 3-го и 4-го каналов" );
	logfile->info(msg);
//	sprintf(msg, "%9d %9ld  %s", f1[3], msk[MASK_35_DELTA_FILTR], "разность температур 3-го и 5-го каналов" );
	sprintf(msg, "%9d %9d  %s", f1[3], msk[MASK_35_DELTA_FILTR], "разность температур 3-го и 5-го каналов" );
	logfile->info(msg);
//	sprintf(msg, "%9d %9ld  %s", f1[4], msk[MASK_45_DELTA_FILTR], "разность температур 4-го и 5-го каналов" );
	sprintf(msg, "%9d %9d  %s", f1[4], msk[MASK_45_DELTA_FILTR], "разность температур 4-го и 5-го каналов" );
	logfile->info(msg);
//	sprintf(msg, "%9d %9ld  %s", f1[5], msk[MASK_TEMP_SMOOTH_FILTR], "неоднородность температуры" );
	sprintf(msg, "%9d %9d  %s", f1[5], msk[MASK_TEMP_SMOOTH_FILTR], "неоднородность температуры" );
	logfile->info(msg);
//	sprintf(msg, "%9d %9ld  %s", f1[6], msk[MASK_ALBEDO_SMOOTH_FILTR], "неоднородность альбедо" );
	sprintf(msg, "%9d %9d  %s", f1[6], msk[MASK_ALBEDO_SMOOTH_FILTR], "неоднородность альбедо" );
	logfile->info(msg);
//	sprintf(msg, "%9ld %9ld  %s", msk[MASK_MULTICH], msk[MASK_MULTICH], "при атмосферной коррекции" );
	sprintf(msg, "%9d %9d  %s", msk[MASK_MULTICH], msk[MASK_MULTICH], "при атмосферной коррекции" );
	logfile->info(msg);
	logfile->info("");

	logfile->info("по сочетаниям фильтров");
	logfile->info( "mask value     count  descr" );
	for (i = 1; i < N_MASK_VALUE-1; i++) {
		if ( msk[i] ) {
//			sprintf(msg, "%5d %5d %9ld ", i, -256-i, msk[i]);
			sprintf(msg, "%5d %5d %9d ", i, -256-i, msk[i]);
			strcat(msg, " ");
			if (i & MASK_ALBEDO_FILTR)
				strcat(msg, "альбедо, ");
			if (i & MASK_TEMP_FILTR)
				strcat(msg, "температура, ");
			if (i & MASK_34_DELTA_FILTR)
				strcat(msg, "разность температур 3-го и 4-го каналов, ");
			if (i & MASK_35_DELTA_FILTR)
				strcat(msg, "разность температур 3-го и 5-го каналов, ");
			if (i & MASK_45_DELTA_FILTR)
				strcat(msg, "разность температур 4-го и 5-го каналов, ");
			if (i & MASK_TEMP_SMOOTH_FILTR)
				strcat(msg, "неоднородность температуры, ");
			if (i & MASK_ALBEDO_SMOOTH_FILTR)
				strcat(msg, "неоднородность альбедо, ");
			if (msg[strlen(msg) - 1] == ' ')
				msg[strlen(msg) - 1] = 0;
			if (msg[strlen(msg) - 1] == ',')
				msg[strlen(msg) - 1] = 0;
			logfile->info(msg);
		}
	}
//	sprintf(msg, "%9ld при атмосферной коррекции", msk[MASK_MULTICH] );
	sprintf(msg, "%9d при атмосферной коррекции", msk[MASK_MULTICH] );
	logfile->info(msg);
//	sprintf(msg, "%9ld границы облачности", msk[MASK_BORDER_FILTR]);
	sprintf(msg, "%9d границы облачности", msk[MASK_BORDER_FILTR]);
	logfile->info(msg);
}

int correctData( short* data, TBlk0_AVHRR& blk0 ) {
	double t0 = 999.;
	int exist_flag = 0;
	int scans = in_blk0.totalFrameNum;
	int cols = in_blk0.totalPixNum;
	for (int i = 0; i < scans * cols; i++) {
		if ( data[i] >= 0) {
			double value = data[i] * blk0.ka + blk0.kb;
			if ( !exist_flag || value < t0) {
				t0 = value;
				exist_flag = 1;
			}
		}
	}
	if (!exist_flag) {
		logfile->error("все точки файла отфильтрованы");
		blk0.kb = 0;
		blk0.maxPixelValue = 0;
		return 0;
	}
	t0 = floor(t0);

	blk0.maxPixelValue = 0;
	for (int i = 0; i < scans * cols; i++) {
		if (data[i] >= 0) {
			double value =    data[i] * blk0.ka + blk0.kb;
			data[i] = int((value - t0) / blk0.ka + 0.5);
			if ( data[i] > blk0.maxPixelValue)
				blk0.maxPixelValue = data[i];
		}
	}
	blk0.kb = t0;
	return 0;
}

int isVisualChannel( TBlk0_AVHRR &b0 ) {
	// для обычных спутников
	if( (b0.processLevel&1) &&  // проведена калибровка
			(b0.ka == 0.1) &&   // шаг = 0.1
			(b0.kb == 0.0) &&   // начальное значение --- 0
			(b0.maxPixelValue == 1000)
	  )
		return 1;
	// для NOAA KLM
	if( (b0.processLevel&1) &&  // проведена калибровка
			(b0.ka == 0.05) &&   // шаг = 0.1
			(b0.kb == 0.0) &&   // начальное значение --- 0
			(b0.maxPixelValue == 2000)
	  )
		return 1;
	return 0;
}
