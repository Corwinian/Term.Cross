/*-----------------------------------------------------------------------------
    multich.cpp
-------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <c_lib.hpp>
#include <orbmodel.hpp>
#include <Log.hpp>

#include "auto_ptr.hpp"
#include "astronom.hpp"

#ifndef LOG_FORMAT
#define LOG_FORMAT "%d %t %e %m"
#endif

char useMsg[] = "\n\
Использование: multich [опции] имя_конфигурации файл_данных\n\
файл_данных     Распакованный и откалиброванный файл четвертого канала.\n\
*** специфических опций утилита не имеет ***\n\
Опции, управляющие печатью сообщений в log-файл:\n\
-llevel={dump,debug,info,warning,error,fatal} уровень вывода сообщений.\n\
-lerr           Выводить сообщения в стандартный поток ошибок.\n\
-lout           Выводить сообщения в стандартный поток вывода.\n\
-lfile=<name>   Выводить сообщения в файл с именем name,\n\
                файл создаётся заново\n\
-lappend=<name> Выводить сообщения в файл с именем name,\n\
                сообщения дописываются в конец файла\n";

// Значение переменной  среды TERM_ROOT
const char * term_root = ".";

/*
	Опции из коммандной строки имеют более высокий приоритет чем опции из файла конфигураций.
*/

nsCLog::eSeverity logLevel = nsCLog::info;
bool useStdErr = false;
bool useStdOut = false;
string logFileName = string();
bool append = false;
CLog *logfile;

char exeFileName[MAX_PATH];    // argv[0]
char cfgName[MAX_FNAME];

// "nxxxxxx_3.clb", "nxxxxxx_4.clb", "nxxxxxx_5.clb"
char inputFileName[3][MAX_PATH];
char outputFileName[MAX_PATH];

int ch3_exist_flag = 1;	// == 1 если файл третьего канала существует и третий канал является инфракрасным

// Параметры атмосферной коррекции
struct TMultichParams {
	// Углы восхождения на Солнце
	// Если угол восхождения на Солнце для данной точки больше angle_day --- считаем что день,
	// Если угол восхождения на Солнце для данной точки меньше больше angle_night --- считаем что ночь.
	double angle_day;
	double angle_night;

	// Параметры фильтрации
	double min_delta_day;  // Если T_4 - T_5 \not\in [min_delta_day, max_delta_day]
	double max_delta_day;  // точку считаем потерянной

	double min_delta_night; // Если T_4 - T_5 \not\in [min_delta_night, max_delta_night]
	double max_delta_night; // точку считаем потерянной
	// Параметры коррекции
	double a_day[6];   // для дня
	double a_night[6]; // дня ночи

	// Способ коррекции
	// Существуют триа основных способа коррекции
	//    MCSST, NLSST и Andy NCSST Triple.
	// MCSST = A1*T4 + A2*(T4-T5) + A3*(T4-T5)*(sec(sate_Angle)-1)+A4
	// NLSST = A1*T4 + A2*(T4-T5)*Tsfc + A3*(T4-T5)*(sec(sate_Angle)-1)+A4
	// Andy NLSST Triple = 
	//       = A1*T4+A2*T3+A3*T5+A4*(T3-T5)*(sec(sate_Angle)-1)+A5*(sec(sate_Angle)-1)+A6
	// Коэффициенты могут браться на странице
	//   http://noaasis.noaa.gov/NOAASIS/pubs/SST/
	char day_type;    // Если == 'M' | 'm' используется коррекция MCSST
	char night_type;  //      == 'N' | 'n' используется коррекция NLSST
	                  //      == 'T' | 't' используется коррекция Andy MCSST Triple
	// В случае, если должна использоваться формула Andy MCSST Triple
	// а третий канал осутствует, производится попытка использования 
	// формулы MCSST с использованием коэффициентов другой части дня
	// о чем делается сообщение
};

const int lost_point = -4;      // этим значением будут помечаться потерянные пиксели
const double KELVIN0 = 273.16;

char msg[64000];                      // Буфер для сообщений лога

#include "multich.hpp"

int main( int argc, char * argv[] ) {

	if( argc < 3 ) {
		printf( "%s", useMsg );
		return 0;
	}

	char * t;
	if(NULL!=(t = getenv("TERM_ROOT"))){
		term_root = strdup(t);
	} else{
		fprintf( stderr, "Переменная TERM_ROOT не установлена, будет использован текущий путь.\n" );
	}

	strcpy( exeFileName, argv[0] );
	strcpy( inputFileName[0], argv[argc-1] );
	strcpy( cfgName, argv[argc-2] );

	try {
		readCfg();
		parseCommandString( argc, argv );
	} catch (TException e) {
		fprintf( stderr, "%s\n", e.text() );
		exit(-1);
	}

	logfile = new CLog("multich.exe");
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

	TAutoPtr<CLog> aLogFile( logfile );

	logfile->info( string("multich.exe - ") + inputFileName[0] );

	construct_file_names();

	TBlk0_AVHRR in3_blk0;
	TBlk0_AVHRR in4_blk0;
	TBlk0_AVHRR in5_blk0;
	TBlk0_AVHRR out_blk0;

	if( readBlk0( inputFileName[0], in3_blk0 ) != 0 ||
		verifyBlk0( in3_blk0, inputFileName[0] ) != 0 || 
	        (in3_blk0.ka == 0.0 && 
                     (in3_blk0.maxPixelValue == 1000 || in3_blk0.maxPixelValue == 2000 )
	        ) )  
	{
		logfile->warning( "Канал 3 видимый" );
		ch3_exist_flag = 0;
	}
	else {
		logfile->warning( "Канал 3 инфракрасный" );
	}


	if( readBlk0( inputFileName[1], in4_blk0 ) != 0 ||
	    verifyBlk0( in4_blk0, inputFileName[1] ) != 0 ) {
		return 1;
	}

	if( readBlk0( inputFileName[2], in5_blk0 ) != 0 ||
	    verifyBlk0( in5_blk0, inputFileName[2] ) ) {
		return 1;
	}

	logfile->info( "чтение файла настроек (multich.dat)" );
	int satId = in4_blk0.b0.satId;
	TMultichParams mParams;
	if( readDatFile( satId, mParams ) != 0 ) {
		return 1;
	}

	int length = in4_blk0.totalFrameNum*in4_blk0.totalPixNum;
	short *in3_data = 0;
	if( ch3_exist_flag ) in3_data = new short[length];
	short *in4_data = new short[length];
	short *in5_data = new short[length];
	short *out_data = new short[length];

	if( ch3_exist_flag ){
		logfile->info( "чтение файла данных 3-го канала" );
		if( readData( in3_data, inputFileName[0], length*sizeof(short), 512 ) != 0 ) {
			return 1;
		}
	}

	logfile->info( "чтение файла данных 4-го канала" );
	if( readData( in4_data, inputFileName[1], length*sizeof(short), 512 ) != 0 ) {
		return 1;
	}

	logfile->info( "чтение файла данных 5-го канала" );
	if( readData( in5_data, inputFileName[2], length*sizeof(short), 512 ) != 0 ) {
		return 1;
	}

	logfile->info( "построение скорректированных температур" );
	if( multich_processing( mParams,
							in3_blk0, in3_data,
							in4_blk0, in4_data,
							in5_blk0, in5_data,
							out_blk0, out_data ) != 0 ) {
		return 1;
	}


	logfile->info( "запись скорректированных температур" );
	writeData( out_data, out_blk0, outputFileName, sizeof(short)*length );

	delete out_data;
	delete in5_data;
	delete in4_data;
	if( ch3_exist_flag ) {
		delete in3_data;
	}

	logfile->info( "" );
	logfile->info( "работа программы завершена успешно !" );
	return 0;
}

void readCfg() throw (TException) {
	char path[MAX_PATH];
	const char * s;
	TCfg* cfg = NULL;

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

	cfg = new TCfg( path );

	try {
		s = cfg->getValue( "LOG_LEVEL" );           /* допустимые значения: dump,debug,info,warning,error,fatal */
		if( nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(string(s))))
			logLevel = nsCLog::info;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_STDERR" );           //
		if('1' == *s)
			useStdErr = true;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_STDOUT" );           //
		if('1' == *s)
			useStdOut = true;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_APPEND" );           //
		logFileName = s;
		append = true;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_FILE" );         /* имя файла */
		logFileName = s;
		append = false;
	} catch(...) {}

	delete cfg;
}

void construct_file_names() {
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], fext[MAX_EXT]; //, s[_MAX_FNAME];
	splitpath( inputFileName[0], drive, dir, fname, fext );
	int l = strlen(fname) - 1;

	fname[l] = '3';
	makepath( inputFileName[0], drive, dir, fname, "clb" );
	fname[l] = '4';
	makepath( inputFileName[1], drive, dir, fname, "clb" );
	fname[l] = '5';
	makepath( inputFileName[2], drive, dir, fname, "clb" );
	fname[l] = 'm';
	makepath( outputFileName, drive, dir, fname, "mch" );

	logfile->debug( "файлы данных:" );
	//  logfile->debug( inputFileName[0] );   // пока третий канал у нас не используется
	logfile->debug( inputFileName[1] );
	logfile->debug( inputFileName[2] );

	logfile->debug( "файл результата:" );
	logfile->debug( outputFileName );
}

void parseCommandString( int argc, char* argv[] ) throw (TException) {
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
			} else {
				/* указана неизвестная опция */
				sprintf(msg,"Неизвестный параметр %s",argv[i]);
				throw TException(100, msg);
			}
		}
	}
}

int readBlk0( const char* name, TBlk0_AVHRR &b0 ) {
	FILE* f = fopen( name, "rb" );
	if( f == 0 ) {
		sprintf( msg, "ошибка открытия файла %s", name );
		logfile->error( msg );
		return 1;
	}
	if( fread( &b0, sizeof(b0), 1, f ) != 1 ) {
		sprintf( msg, "ошибка чтения файла %s", name );
		logfile->error( msg );
		return 1;
	}
	fclose( f );
	return 0;
}

int verifyBlk0( TBlk0_AVHRR &blk0, const char * name = 0 ) {
	if( blk0.b0.formatType != 0xFF ) {
		if( name != 0 ) {
			sprintf( msg, "неправильный формат нулевого блока файла %s", inputFileName[1] );
			logfile->error( msg );
		} else {
			logfile->error( "неправильный формат нулевого блока" );
		}
		return 1;
	}
	return 0;
}


int readDatFile( int satId, TMultichParams & mParams ) {
	char path[MAX_PATH];
	static char buf[2048];

	strcpy(path,term_root);
	int t = strlen(path);
	/* Если надо, в конце добавляем разделитель */
	if(path[t-1]!='/'||path[t-1]!='\\'){
		path[t] = DIRD; path[t+1] = '\0';
	}
#if DIRD == '/'
		strcat( path, "data/multich.dat" );
#else
		strcat( path, "data\\multich.dat" );
#endif
	logfile->debug( "файл multich.dat:" );
	logfile->debug( path );
	logfile->debug( "" );

	FILE * f = fopen( path, "r" );
	if( f == 0 ) {
		sprintf( msg, "ошибка открытия файла %s", path );
		logfile->error( msg );
		return 1;
	}

	int line = 0;
	int res = 2;
	int id;
	while ( fgets( buf, sizeof(buf), f ) && res == 2 ) {
		line++;
		if( !strIsEmpty(buf) ) {
			if( strParse(buf, id, mParams ) != 0 ) {
				res = 1;
			} else if( id == satId ) {
				res = 0;
			}
		}
	}

	if( res == 2 ) {
		sprintf( msg, "в файле %s отсутствуют данные по спутнику %d", path, satId );
		logfile->error( msg );
	} else if( res == 1 ) {
		sprintf( msg, "ошибка разбора строки %d файла %s", line, path );
		logfile->error( msg );
	}
	fclose(f);

	return res;
}

static int strIsEmpty( const char * buf ) {
	const char * p = buf;
	while( *p && isspace(*p) )
		p++;
	if( *p == '\0' || *p == '#' )
		return 1;
	return 0;
}

static int strParse( const char * buf, int& satId, TMultichParams & params ) {
	int id;
	int ns = sscanf( buf, "\
			 %d \
			 %lf %lf\
		 	 %lf %lf\
			 %lf %lf\
			 %c %lf %lf %lf %lf %lf %lf\
			 %c %lf %lf %lf %lf %lf %lf",
			 &id,
			 &params.angle_day,
			 &params.angle_night,
			 &params.min_delta_day,
			 &params.max_delta_day,
			 &params.min_delta_night,
			 &params.max_delta_night,
			 &params.day_type,   
				&params.a_day[0], &params.a_day[1], &params.a_day[2], 
			        &params.a_day[3], &params.a_day[4], &params.a_day[5],
			 &params.night_type, 
			        &params.a_night[0], &params.a_night[1], &params.a_night[2], 
                                &params.a_night[3], &params.a_night[4], &params.a_night[5] );

	if( params.day_type == 'm' ) params.day_type = 'M';
	if( params.day_type == 'n' ) params.day_type = 'N';
	if( params.day_type == 't' ) params.day_type = 'T';
	if( params.day_type != 'M' && params.day_type != 'N' && params.day_type != 'T' ) {
		return 1;
	}

	if( params.night_type == 'm' ) params.night_type = 'M';
	if( params.night_type == 'n' ) params.night_type = 'N';
	if( params.night_type == 't' ) params.night_type = 'T';
	if( params.night_type != 'M' && params.night_type != 'N' && params.night_type != 'T' ) {
		return 1;
	}

	if( ns == 21  ) {
		sprintf( msg, "satId = %d", id );
		logfile->debug( msg );
		sprintf( msg, "angle_day = %lf    angle_night = %lf", params.angle_day, params.angle_night );
		logfile->debug( msg );
		sprintf( msg, "min_delta_day = %lf     max_delta_day = %lf", params.min_delta_day, params.max_delta_day );
		logfile->debug( msg );
		sprintf( msg, "min_delta_night = %lf   max_delta_night = %lf", params.min_delta_night, params.max_delta_night );
		logfile->debug( msg );
		logfile->debug( "MCSST = B_1 (T_11) + B_2 (T_11-T_12) + B_3 (T_11-T_12) (sec(theta)-1) + B_4" );
		logfile->debug( "NLSST = B_1 (T_11) + B_2 (T_11-T_12) Tsfc + B_3 (T_11-T_12) (sec(theta)-1) + B_4" );
		logfile->debug( " Andy Triple NLSST = "
			"B_1*T4+B_2*T3+B_3*T5+B_4*(T3-T5)*(sec(sate_Angle)-1)+B_5*(sec(sate_Angle)-1)+B_6" );
		sprintf( msg, "day   B_1 = %lf   B_2 = %lf   B_3 = %lf   B_4 = %lf  B_5 = %lf B_6 = %lf",
				 params.a_day[0], params.a_day[1], params.a_day[2], 
				 params.a_day[3], params.a_day[4], params.a_day[5] );
		logfile->debug( msg );

		if( params.day_type == 'M' ){
			logfile->debug( "MCSST must be used for day points" );
		}
		else if (params.day_type == 'N') {
			logfile->debug( "NLSST must be used for day points" );
		}
		else {
			logfile->debug( "Andy Triple MCSST must be used for day points" );
		}
		sprintf( msg, "night B_1 = %lf   B_2 = %lf   B_3 = %lf   B_4 = %lf B_5 = %lf B_6 = %lf",
				 params.a_night[0], params.a_night[1], params.a_night[2], 
				 params.a_night[3], params.a_night[4], params.a_night[5] );
		logfile->debug( msg );

		if( params.night_type == 'M' ){
			logfile->debug( "MCSST must be used for night points" );
		}
		else if (params.day_type == 'N') {
			logfile->debug( "NLSST must be used for night points" );
		}
		else {
			logfile->debug( "Andy Triple MCSST must be used for night points" );
		}
		satId = id;
		return 0;
	}
	return 1;
}


int multich_processing( const TMultichParams &mParams,
						TBlk0_AVHRR &in3_blk0, short *in3_data,
						TBlk0_AVHRR &in4_blk0, short *in4_data,
						TBlk0_AVHRR &in5_blk0, short *in5_data,
						TBlk0_AVHRR &out_blk0, short *out_data
					  ) {
	int lines = in4_blk0.totalFrameNum;
	int cols = in4_blk0.totalPixNum;
	int length = lines*cols;

	float *chmFloat = new float [length];
	double *theta = new double[cols];

	// для определения угла восхождения на солнце
	TNOAAImageParams NOAAImageParams( (TBlk0&)in4_blk0 );
	TCorrectionParams corrParams((TBlk0&)in4_blk0);
	double julian_date = julian(NOAAImageParams.fYear, NOAAImageParams.fYearTime + 1);
	TIniSatParams iniSatParams( (TBlk0&)in4_blk0);
	TOrbitalModel orbitalModel(iniSatParams, NOAAImageParams.fYear, NOAAImageParams.fYearTime + 1, corrParams );
	TStraightReferencer straightReferencer(iniSatParams, NOAAImageParams, corrParams );

	double avarage_h = calcSatHeight( orbitalModel, lines );
	sprintf(msg, "средняя высота спутника на снимке: %.2lf километров.", avarage_h);
	logfile->debug(msg);

	logfile->debug("");
	logfile->debug("атмосферная масса");
	logfile->debug("  кол.      угол.           m");
	// рассчитываем косекансы углов наклона линии сканирования
	for (int i = 0; i < cols; i++) {
		double fi = fabs(54 * DR * ((2.*i)/cols-1));
		theta[i] = calcAthm( fi, avarage_h );
		{
			sprintf(msg, "%6d %10lf %10lf", i, fi, theta[i] );
			logfile->debug(msg);
		}
	}

	//
	// Собственно обработка
	//
	for( int i = 0; i < lines; i++ ) {
		int col1 = 0;
		int col2 = 128;
		double angle1 = calcAngle( i, col1, julian_date, straightReferencer );   // Угол восхождения на Солнце
		double angle2 = calcAngle( i, col2, julian_date, straightReferencer );   // Угол восхождения на Солнце
		for( int j = 0; j < cols; j++ ) {        // Цикл по строке снимка
			int pix = i*cols+j;
			double angle;   // угол восхождения на Солнце
			if( j > col2 ) {
				col1 = col2;
				col2 += 128;
				if( col2 >= cols )
					col2 = cols - 1;
				angle1 = angle2;
				angle2 = calcAngle( i, col2, julian_date, straightReferencer );
			}
			if( j == col1 ) {
				angle = angle1;
			} 
			else if( j == col2 ) {
				angle = angle2;
			} 
			else {
				angle = angle1 + (angle2 - angle1)*double(j-col1)/double(col2-col1);
			}

			if ( in4_data[pix] < 0 ) {
				out_data[pix] = in4_data[pix];
				continue;
			}

			// расчитываем среднюю разницу четвертого-пятого каналов
			double delta45 = 0;
			int n = 0;
			for( int i1 = -1; i1 <=1; i1++ ) {
				for( int j1 = -1; j1 <= 1; j1++ ) {
					if( i + i1 >= 0 && i + i1 < lines &&
							j + j1 >= 0 && j + j1 < cols ) {
						int l_pix = cols*(i+i1) + (j+j1);
						if( in4_data[l_pix] >= 0 && in5_data[l_pix] >= 0 ) {
							double T4 = in4_data[l_pix]*in4_blk0.ka + in4_blk0.kb;
							double T5 = in5_data[l_pix]*in5_blk0.ka + in5_blk0.kb;
							delta45 += (T4-T5);
							n++;
						}
					}
				}
			}

			if( n == 0 ) {
				out_data[pix] = lost_point;
				continue;
			}
			delta45 /= double(n);

			double t4 = in4_data[pix] * in4_blk0.ka + in4_blk0.kb + KELVIN0;
			double t3;
			if( in3_data )
				t3 = in3_data[pix] * in3_blk0.ka + in3_blk0.kb + KELVIN0;
			double t5 = in5_data[pix] * in5_blk0.ka + in5_blk0.kb + KELVIN0;
 
			double T_day = -50.0;	
			int T_day_exist = 1;
			if( mParams.day_type == 'M' ) {
				double A1 = mParams.a_day[0];
				double A2 = mParams.a_day[1];
				double A3 = mParams.a_day[2];
				double A4 = mParams.a_day[3];
				T_day = t4*A1 + delta45*A2 + delta45*A3*(theta[j] -1.0)+A4;
			}
			else if( mParams.day_type == 'N' ) {
				double A1 = mParams.a_day[0];
				double A2 = mParams.a_day[1];
				double A3 = mParams.a_day[2];
				double A4 = mParams.a_day[3];
				if( fabs(1.0 - delta45*A2) < 0.1 ) 
					T_day_exist = 0;
				else {
					T_day = (t4*A1 + delta45*A3*(theta[j] -1.0)+A4) /
				        	(1.0 - delta45*A2);  // У нас пока нет оценок Tsfc, 
					        // необходимых для вычисления NLSST поэтому мы вынуждены...
				}
			}
			else { // if( params.day_type == 'T' ) 
				if( in3_data ){
					double A1 = mParams.a_day[0];
					double A2 = mParams.a_day[1];
					double A3 = mParams.a_day[2];
					double A4 = mParams.a_day[3];
					double A5 = mParams.a_day[4];
					double A6 = mParams.a_day[5];
					T_day = A1*t4 + A2*t3 + A3*t5 + 
					        A4*delta45*(theta[j]-1.0) + A5*(theta[j]-1.0)+A6;
				}
				else {
					T_day_exist = 0;
				} 
			}

			double T_night = -50.0;	
			int T_night_exist = 1;
			if( mParams.night_type == 'M' ) {
				double A1 = mParams.a_night[0];
				double A2 = mParams.a_night[1];
				double A3 = mParams.a_night[2];
				double A4 = mParams.a_night[3];
				T_night = t4*A1 + delta45*A2 + delta45*A3*(theta[j] -1.0)+A4;
			}
			else if( mParams.night_type == 'N' ) {
				double A1 = mParams.a_night[0];
				double A2 = mParams.a_night[1];
				double A3 = mParams.a_night[2];
				double A4 = mParams.a_night[3];
				if( fabs(1.0 - delta45*A2) < 0.1 ) 
					T_night_exist = 0;
				else {
					T_night = (t4*A1 + delta45*A3*(theta[j] -1.0)+A4) /
				        	(1.0 - delta45*A2);  // У нас пока нет оценок Tsfc, 
					        // необходимых для вычисления NLSST поэтому мы вынуждены...
				}
			}
			else { // if( params.night_type == 'T' ) 
				if( in3_data ){
					double A1 = mParams.a_night[0];
					double A2 = mParams.a_night[1];
					double A3 = mParams.a_night[2];
					double A4 = mParams.a_night[3];
					double A5 = mParams.a_night[4];
					double A6 = mParams.a_night[5];
					T_night = A1*t4 + A2*t3 + A3*t5 + 
					        A4*delta45*(theta[j]-1.0) + A5*(theta[j]-1.0)+A6;
				}
				else {
					T_night_exist = 0;
				} 
			}

			if( angle > mParams.angle_day ) {    // Обработка для дневных точек
				if ( (delta45 < mParams.min_delta_day) ||
				     (delta45 > mParams.max_delta_day) ) 
				{
					out_data[pix] = lost_point;
				} 
				else {
					if( T_day_exist ){
						chmFloat[pix] = T_day;
						out_data[pix] = 1;
					} 
					else if ( T_night_exist ){
						chmFloat[pix] = T_night;
						out_data[pix] = 1;
						// warning
					}
					else {
						// error...
						out_data[pix] = lost_point;
					}
				}
			} 
			else if( angle < mParams.angle_night ) {    // Обработка для ночных точек
				if ((delta45 < mParams.min_delta_night) ||
				    (delta45 > mParams.max_delta_night)) {
					out_data[pix] = lost_point;
				} else {
					if( T_night_exist ){
						chmFloat[pix] = T_night;
						out_data[pix] = 1;
					} 
					else if ( T_day_exist ){
						chmFloat[pix] = T_day;
						out_data[pix] = 1;
						// warning
					}
					else {
						// error...
						out_data[pix] = lost_point;
					}
				}
			} else {      // точки, расположенные между дневными и ночными,
				// не пропускаем, а придаем им промежуточные значения
				if( (delta45 < mParams.min_delta_night) ||
				      (delta45 < mParams.min_delta_day)   ||
				      (delta45 > mParams.max_delta_night) ||
				      (delta45 > mParams.max_delta_day) ) 
				{
					out_data[pix] = lost_point;
				} else {
					if( T_day_exist && T_night_exist ){
						double k1 = (angle - mParams.angle_night) / 
						            (mParams.angle_day - mParams.angle_night);
						double k2 = (mParams.angle_day - angle) / 
						            (mParams.angle_day - mParams.angle_night);
						double a = T_day * k1 + T_night * k2;
						chmFloat[pix] = a;
						out_data[pix] = 1;
					}
					else if( T_day_exist ){
						chmFloat[pix] = T_day;
						out_data[pix] = 1;
					}
					else if( T_night_exist ){
						chmFloat[pix] = T_night;
						out_data[pix] = 1;
					}
					else {
						out_data[pix] = lost_point;
					}
				}
			}
		}
	}
	//
	// обработка завершена
	//

	// Определение параметров хранения данных
	double minT = 0;
	double maxT = 0;
	int flag = 1;
	for( int i = 0; i < lines; i++ ) {
		for( int j = 0; j < cols; j++ ) {
			int pix = i * cols + j;
			if( out_data[pix] >= 0 ) {
				if( flag || chmFloat[pix] < minT ) {
					flag = 0;
					minT = chmFloat[pix];
				}
			}
			if( flag || chmFloat[pix] > maxT ) {
				flag = 0;
				maxT = chmFloat[pix];
			}
		}
	}
	if( flag ) {
		logfile->error( "все точки файла отфильтрованы" );
		delete chmFloat;
		delete theta;
		return 1;
	}

	sprintf( msg, "минимальная встреченная температура %lf", minT );
	logfile->debug( msg );
	sprintf( msg, "максимальная встреченная температура %lf", maxT );
	logfile->debug( msg );

	// Создание нового нулевого блока
	out_blk0 = in4_blk0;
	out_blk0.ka = in4_blk0.ka;
	out_blk0.kb = floor(minT);
	out_blk0.processLevel |= 2;

	// Заполнение блока данных
	int maxValue = 0;
	for( int i = 0; i < lines; i++ ) {
		for( int j = 0; j < cols; j++ ) {
			int pix = i * cols + j;
			if( out_data[pix] >= 0 ) {
				out_data[pix] =
					int(floor((chmFloat[pix] - out_blk0.kb)/out_blk0.ka + 0.5));
				if( out_data[pix] > maxValue )
					maxValue = out_data[pix];
			}
		}
	}
	out_blk0.maxPixelValue = maxValue;
	delete chmFloat;
	delete theta;
	return 0;
}

int readData( void* data, const char* fName, int length, int blk0Len = 0 ) {
	FILE * f = fopen( fName, "rb" );
	if( f == 0 ) {
		sprintf( msg, "ошибка открытия файла %s", fName );
		logfile->error( msg );
		return 1;
	}
	if( fseek( f, blk0Len, SEEK_SET ) != 0 ) {
		sprintf( msg, "ошибка позиционирования в файле %s", fName );
		logfile->error( msg );
		return 1;
	}
	if( fread( data, length, 1, f ) != 1 ) {
		sprintf( msg, "ошибка чтения файла %s", fName );
		logfile->error( msg );
		return 1;
	}
	fclose(f);
	return 0;
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
double calcAngle(int nscan, int col, double date, TStraightReferencer & r ) {
	double lat;
	double lon;
	double fi;

	r.xy2ll(col, nscan, &lon, &lat);
	if ((lon >= 0.) && (lon <= PI))
		lon = -lon;
	fi = shcrds( date, lon, lat);
	return (fi);
}


int writeData( void* data, TBlk0_AVHRR & blk0, const char* fName, int length ) {
	FILE * f = fopen( fName, "wb" );
	if( f == 0 ) {
		sprintf( msg, "ошибка открытия файла %s", fName );
		logfile->error( msg );
		return 1;
	}
	if( fwrite( &blk0, sizeof(TBlk0_AVHRR), 1, f ) != 1 ||
			fwrite( data, length, 1, f ) != 1 ) {
		sprintf( msg, "ошибка записи в файл %s", fName );
		logfile->error( msg );
		return 1;
	}
	fclose(f);
	return 0;
}

double calcSatHeight( TOrbitalModel &o, int scans ) {
	double a = 0;
	int n = 0;
	for (int i = 0; i < scans; i += 10) {
		o.model(i / 6.0 / (60. * 60. * 24.));
		a += sqrt(o.r[0] * o.r[0] + o.r[1] * o.r[1] + o.r[2] * o.r[2]);
		n++;
	}
	a /= n;      // Теперь avarage_h --- расстояние от центра Земли до спутника
	return a - 6370.;   // 6370. --- радиус Земли
}

/*
 Расчет атмосферной массы
 С учетом искривления земной поверхности
 sec(theta) = 1/cos(theta)
 cos(theta) = sqrt(1 - sin^2(theta))
 sin(theta) = sin(fi)*(1+avarage_h/R)	(по формуле синусов)
 */
double calcAthm( double fi, double avarage_h ) {
	const double R = 6370.;      // Радиус Земли
	//const double h = 820.;      // Средняя высота спутника над горизонтом
	//    double sinAzimut = (1. + avarage_h / R) * sin(fi);
	//    return  1. /sqrt(1. - sinAzimut * sinAzimut) - 1.;
	double sinAzimut = (1. + avarage_h / R) * sin(fi);
	return  1. /sqrt(1. - sinAzimut * sinAzimut);
}
