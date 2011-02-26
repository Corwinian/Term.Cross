/*-------------------------------------------------------------------------

	hold.cpp
	Программа преобразования файлов проекций нового формата в старый.

	ВНИМАНИЕ !!!
	Поле <NOAA_номер_спутника> получаемого файла для спутника NOAA-16 будет
	содержать 15.
-------------------------------------------------------------------------*/
#include "h2old.hpp"
#include <libconfig.h++>

#ifndef LOG_FORMAT
#define LOG_FORMAT "%d %t %e %m"
#endif

#define KELVIN 273.16

using namespace libconfig;

const char useMsg[] = "\
Утилита преобразования файлов проекций в формат TERMOS.\n\
h2old [опции] имя_конфигурации файл_данных\n\
файл_данных     Файл проекции, данные в нем должны быть откалиброваны.\n\
Опции:\n\
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
string logFileName = string( );
bool append = false;
CLog *logfile;

double tempTreshold = -3.0;
int tempTreshold_is_defined_flag = 0;
float tempStep = .125;
int tempStep_is_defined_flag = 0;

char exeFileName[MAX_PATH]; // argv[0]
char cfgName[MAX_FNAME];
char inputFileName[MAX_PATH];
char outputFileName[MAX_PATH];

int16_t * inputDataBuf;
uint8_t * outputDataBuf;

TBlk0_Proj newb0;
TOldBlk0 oldb0;

// размеры исходной проекции в пикселах
// из-за странной трактовки смысла географического размера проекции и разрешения, размер проекции TERMOS по X при тех же значениях этих параметров получается на 1 меньше
// программа просто обрезает крайний столбец, сохраняя те же значения географических размеров
uint32_t sizeX;
uint32_t sizeY;

int main( int argc, char * argv[] ){

	//  #ifdef _VISUALAGECPP_
	//    DosSetPriority( PRTYS_PROCESSTREE, PRTYC_IDLETIME, 31, 0 );
	//  #endif
	atexit(atExit);

	if( argc < 3 ){
		printf("%s", useMsg);
		return 0;
	}

	char * t;
	if( NULL != (t = getenv("TERM_ROOT")) ){
		term_root = strdup(t);
		//fprintf( stdout, "Переменная TERM_ROOT = %s\n",term_root );
		//fflush(stdout);
	}else{
		fprintf(stderr, "Переменная TERM_ROOT не установлена, будет использован текущий путь.\n");
		fflush(stderr);
	}

	strcpy(exeFileName, argv[0]);
	strcpy(inputFileName, argv[argc - 1]);
	strcpy(cfgName, argv[argc - 2]);

	try{
		readCfg();
		parseCommandString(argc, argv);
	}catch( TException e ){
		fprintf(stderr, "%s\n", e.what());
		exit(-1);
	}

	logfile = new CLog(("h2old.exe"));
	if( useStdOut ){
		//logfile.addAppender(new ConsoleLogAppender(logLevel,"%s | %e | %m"));
		//logfile->addAppender(new CConsoleLogAppender(logLevel, "%e | %m"));
		logfile->addAppender(new CConsoleLogAppender(logLevel, LOG_FORMAT));
	}
	if( useStdErr ){
		//logfile.addAppender(new EConsoleLogAppender(logLevel,"%s | %e | %m"));
		//logfile->addAppender(new EConsoleLogAppender(logLevel, "%e | %m"));
		logfile->addAppender(new EConsoleLogAppender(logLevel, LOG_FORMAT));
	}
	if( !logFileName.empty() ){
		try{
			//logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, "%s | %e | %m"));
			logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, LOG_FORMAT));
		}catch( string e ){
			fprintf(stderr, "%s\n", e.c_str());
			exit(1);
		}
	}

	logfile->info(string("h2old.exe - ") + inputFileName);
	if( !tempStep_is_defined_flag ){
		logfile->warning("Переменная конфигурационного файла TERMOS_CONV_TEMP_STEP не установлена - используется знечение 0.125 .");
	}
	if( !tempTreshold_is_defined_flag ){
		logfile->warning("Переменная конфигурационного файла TERMOS_CONV_TEMP_TRESHOLD не установлена - используется знечение -3 .");
	}

	// инициализация параметров, получаемых из файла конфигурации или командной строки
	//tempTreshold = cfgTempTreshold;

	try{
		readInputFile();

		constructOutputFileName();
		constructOldBlk0();
		calculateOutputData();

		saveOutputFile();
	}catch( TException e ){
		logfile->error(e.text());
		exit(1);
	}

	delete [] inputDataBuf;
	delete [] outputDataBuf;

	logfile->info("");
	logfile->info("работа программы завершена успешно !");

	return 0;
}

void saveOutputFile( ) throw (TException){
	FILE * f;

	logfile->info("запись выходного файла данных");

	if( (f = fopen(outputFileName, "wb")) == NULL ){
		throw TException(100, "Ошибка создания файла");
	}

	if( fwrite(&oldb0, 512, 1, f) != 1 ){
		throw TException(100, "Ошибка записи файла");
	}

	if( fwrite(outputDataBuf, 1, (sizeX - 1) * sizeY, f) != (sizeX - 1) * sizeY ){
		throw TException(100, "Ошибка записи файла");
	}

	fclose(f);
}

void readInputFile( ) throw (TException){
	FILE * f;

	logfile->info("чтение входного файла данных");

	if( (f = fopen(inputFileName, "rb")) == NULL || fread(&newb0, 512, 1, f) != 1 ){
		throw TException(100, "ошибка доступа ко входному файлу данных");
	}

	if( newb0.b0.formatType != L34_FORMAT ||
			newb0.b0.dataType1 != B0DT1_PRJ ||
			(newb0.b0.dataType2 != B0DT2_HRPT_NOAA &&
			newb0.b0.dataType2 != B0DT2_HRIT) ){
		throw TException(100, "неправильный формат входного файла данных");
	}

	if( (newb0.processLevel & 0x1) != 0x1 ){
		throw TException(100, "данные входного файла не калиброваны");
	}

	if( newb0.latRes != newb0.lonRes ){
		throw TException(100, "разрешения по широте и долготе не совпадают");
	}

	sizeX = newb0.pixNum;
	sizeY = newb0.scanNum;
	inputDataBuf = new int16_t [sizeX * sizeY];

	if( fread(inputDataBuf, 2, sizeX * sizeY, f) != sizeX * sizeY ){
		throw TException(100, "ошибка доступа ко входному файлу данных");
	}

	fclose(f);
}

int isVisualChannel( const TBlk0_Proj & b0 ){
	// Проекция MTSAT нового формата
	if( b0.b0.dataType1 == B0DT1_PRJ && b0.b0.dataType2 == B0DT2_HRIT ){
		if( b0.channel == 5 ) return 1;
	}

	// Проекция NOAA нового формата
	//   (видимый канал 3 (1.2 мкм) обрабатывается с ошибкой)
	if( b0.b0.dataType1 == B0DT1_PRJ && b0.b0.dataType2 == B0DT2_HRPT_NOAA ){
		if( b0.channel == 1 || b0.channel == 2 ) return 1;
	}
	return 0;
}

void constructOldBlk0( ){
	int month, date;
	int hour, minute, second, tic;
	dayToDate(newb0.b0.year, newb0.b0.day - 1, &month, &date);
	dayFractionToTime(double(newb0.b0.dayTime) / 86400000., &hour, &minute, &second, &tic);
	oldb0.day = date + 1;
	oldb0.month = month + 1;
	oldb0.year = newb0.b0.year < 2000 ? newb0.b0.year - 1900 : newb0.b0.year - 2000;
	oldb0.hour = hour;
	oldb0.minute = minute;
	oldb0.sec = second;
	oldb0.tic = tic;

	uint32_t sat_num = 0; // номер спутника NOAA
	if( strcmp(newb0.b0.satName, "NOAA") == 0 && (newb0.b0.satId & 0xffff) == 0 ){ // если файл формата 1999 года
		sat_num = newb0.b0.satId >> 16;
	}else{ // если поле new_b0->satNum содержит уникальный идентификатор спутника
		switch( newb0.b0.satId ){
			case TSatInfoTable::sat_id_noaa_9:
				sat_num = 9;
				break;
			case TSatInfoTable::sat_id_noaa_10:
				sat_num = 10;
				break;
			case TSatInfoTable::sat_id_noaa_11:
				sat_num = 11;
				break;
			case TSatInfoTable::sat_id_noaa_12:
				sat_num = 12;
				break;
			case TSatInfoTable::sat_id_noaa_14:
				sat_num = 14;
				break;
			case TSatInfoTable::sat_id_noaa_15:
				sat_num = 15;
				break;
			case TSatInfoTable::sat_id_noaa_16:
				sat_num = 16;
				break;
			case TSatInfoTable::sat_id_noaa_17:
				sat_num = 17;
				break;
			case TSatInfoTable::sat_id_noaa_18:
				sat_num = 18;
				break;
			case TSatInfoTable::sat_id_noaa_19:
				sat_num = 19;
				break;
			default:
				char b[100]; // 100 is enough for long int ;)
				sprintf(b, "%d", newb0.b0.satId);
				logfile->warning(string("Unknown satID ") + string(b));
				sat_num = 1;
		}
	}
	oldb0.satNumber = (uint8_t) (0x80L | (sat_num << 1));

	oldb0.revNum = (uint16_t) (newb0.b0.revNum % 10000);

	oldb0.totalScans = 1; // просто чтобы работал SEAFLOW

	if( isVisualChannel(newb0) ){ // видимые каналы
		oldb0.minT = 0.;
		oldb0.stepT = 0.390625; // как в TERMOSе
		oldb0.calCode = 3;
	}else{ // температурные каналы
		oldb0.minT = float( tempTreshold) + KELVIN;
		oldb0.stepT = tempStep; // как в TERMOSе
		oldb0.calCode = 2;
	}

	oldb0.chNumber = newb0.channel;
	oldb0.typeOfFile = newb0.projType == B0PRJ_MERC ? 6 : 7; // меркаторская или равнопромежуточная

	oldb0.projPixStrLength = sizeX - 1;
	oldb0.projNumberOfScans = sizeY;
	oldb0.projPixelBytes = 1;
	oldb0.projPixelSize = newb0.latRes / 3600.;
	oldb0.minProjLat = newb0.lat;
	oldb0.maxProjLat = newb0.lat + newb0.latSize;
	oldb0.minProjLon = newb0.lon;
	oldb0.maxProjLon = newb0.lon + newb0.lonSize;

	oldb0.processState = 0;
	oldb0.mKHR = 0; // Вычислять - лень !!!
}


// !!!!!!!!   Может быть, нужно что-то делать в связи с тем, что в старых проекциях
// градусы Кельвина.

void calculateOutputData( ){
	logfile->info("преобразование данных");

	outputDataBuf = new uint8_t [(sizeX - 1) * sizeY];

	// в проекциях TERMOS строки располагаются сверху вниз, а не снизу вверх, как в новых
	// также обрезаем крайний столбец, чтобы проекции формально совпадали с получаемыми в TERMOS при тех же значениях параметров
	for( uint32_t i = 0; i < sizeY; i++ ){
		int16_t * src = inputDataBuf + i * sizeX;
		uint8_t * dst = outputDataBuf + (sizeY - 1 - i)*(sizeX - 1);
		uint8_t * dst_end = dst + sizeX - 1;

		if( isVisualChannel( newb0 ) ){ // видимые каналы
			while( dst != dst_end ){
				int16_t pixel = *src++;
				if( pixel >= 0 ){ // непустой пиксел
					*dst++ = (uint8_t) ((double(pixel) * newb0.ka + newb0.kb) * 256. / 100. + .5);
				}else{
					*dst++ = 0;
				}
			}
		}else{ // температурные каналы
			while( dst != dst_end ){
				int16_t pixel = *src++;
				uint8_t d = 0;
				if( pixel >= 0 ){ // непустой пиксел
					double v = double(pixel) * newb0.ka + newb0.kb;
					if( v >= tempTreshold ){ // пиксел выше температурного порога
						double v_old = floor((v - tempTreshold) / tempStep + .5);

						if( v_old >= 255 )
							d = 255;
						else
							d = (uint8_t) (v_old);
					}else{ // пиксел ниже температурного порога
						d = 0;
					}
				}else{
					d = 0;
				}
				*dst++ = d;
			}
		}
	}
}

void constructOutputFileName( ){
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], ext[4] = "prc";

	splitpath(inputFileName, drive, dir, fname, 0);
	if(fname[strlen(fname) - 2] == '_')
		fname[strlen(fname) - 2] = '\0'; // если! имя имеет стандартный вид "nXXXXX_Y.pro", обрезаем "_Y"
	if( (newb0.processLevel & 0x3) == 1 ){ // проведена только калибровка (без атмосферной коррекции) ?
		ext[2] = '0' + newb0.channel; // конструируем расширение вида "prY", Y - 1..5
	}
	makepath(outputFileName, drive, dir, fname, ext);
}

void readCfg( ) throw (TException){
	char path[MAX_PATH];
	const char * s;



	if( (NULL == strchr(cfgName, '\\')) &&
			(NULL == strchr(cfgName, '/')) &&
			(NULL == strchr(cfgName, '.')) ){
		strcpy(path, term_root);
		int t = strlen(path);
		/* Если надо, в конце добавляем разделитель */
		if( path[t - 1] != '/' || path[t - 1] != '\\' ){
			path[t] = DIRD;
			path[t + 1] = '\0';
		}
#if DIRD == '/'
		strcat(path, "cfg/");
#else
		strcat(path, "cfg\\");
#endif
		strcat(path, cfgName);
		strcat(path, ".cfg");
	}else{
		strcpy(path, cfgName);
	}

	Config conf;
	conf.readFile(path);
	delete path;
	//const Setting& root = cfg.getRoot();

	const Setting& Log = conf.getRoot()["Log"];

	//string name = cfg.lookup("name");

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


	try
	{	tempStep_is_defined_flag = conf.lookupValue("termos_conv_temp_step", tempStep);}
	catch( TRequestExc )
	{	tempStep = .125;}

	try
	{	tempTreshold_is_defined_flag  = conf.lookupValue("termos_conv_temp_treshold", tempTreshold);}
	catch( TRequestExc )
	{	tempTreshold = -3.0;}
}

void parseCommandString( int argc, char* argv[] ) throw ( TException){
	string msg;
	int i = 1;
	while( i < argc ){
		char * s = argv[i++];
		if( *s == '-' ){
			s++;
			if( *s == 'l' ){
				s++;
				if( !strcmp(s, "err") )
					useStdErr = true;
				else if( !strcmp(s, "out") )
					useStdOut = true;
				else if( !strncmp(s, "file=", 5) ){
					s += 5;
					if( strlen(s) == 0 )
						throw TException(100, "Недопустимое значение параметра -lfile=<file name>");
					logFileName.assign((char*) s);
					append = false;
				}else if( !strncmp(s, "append=", 7) ){
					s += 7;
					if( strlen(s) == 0 )
						throw TException(100, "Недопустимое значение параметра -lappend=<file name>");
					logFileName.assign((char*) s);
					append = true;
				}else if( !strncmp(s, "level=", 6) ){
					s += 6;
					if( nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(string(s))) )
						throw TException(100, "Недопустимое значение параметра -llevel=<message level>");
					//logLevel = nsCLog::getThresholdFromString(string(s));
				}else{
					msg.assign("Неизвестный параметр -l").append(s);
					throw TException(100, msg.c_str());
				}
			}else{
				msg.assign("Неизвестный параметр -").append(s);
				throw TException(100, msg.c_str());
			}
		}
	}
}

void atExit( ){
	if( logfile ){
		logfile->debug("atExit");
		delete logfile;
	}
}

