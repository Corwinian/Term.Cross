/*-----------------------------------------------------------------------------
    hunp.cpp
-----------------------------------------------------------------------------*/
#include <tc_config.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
using namespace std;

#include <c_lib.hpp>
#include <hrpt.hpp>
#include <Log.hpp>
#include "hunp.hpp"

#ifndef LOG_FORMAT
#define LOG_FORMAT "%d %t %e %m"
#endif
/*
	dump,debug,info,warning,error,fatal
*/
char useMsg[] = "\
Утилита распаковки файлов NOAA HRPT.\n\
hunp [опции] имя_конфигурации файл_данных\n\
файл_данных     Файл исходных данных NOAA HRPT.\n\
Опции:\n\
-a              Распаковать все имеющиеся каналы AVHRR.\n\
-a{1|2|3|4|5}   Распаковать указанные каналы AVHRR.\n\
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

bool is_a_present = false;	// флаг присутствия в командной строке опции -a
bool channels[6];			// флаги каналов; элемент [0] не используется
bool channels_cfg[6];		// соответствует параметру UNP_AVHRR_CHANNELS файла конфигурации; элемент [0] не используется
bool channels_a[6];			// флаги присутствия цифр в опции -a; элемент [0] не используется

char exeFileName[MAX_PATH];            // argv[0]
char cfgName[MAX_PATH];
//char cfgFileName[MAX_PATH];
char inputFileName[MAX_PATH];
char outputFileNames[8][MAX_PATH];     // "nxxxxx_1.avh", ..., "nxxxxx_5.avh", "nxxxxx_3.tlm", ..., "nxxxxx_5.tlm"

TBlk0_HRPT bh;
TBlk0_AVHRR ba;
TBlk0_TLM bt;

uint32_t totalFrameNum;

uint16_t scanBuf[2048];
uint16_t *avhrrBuf[5];
uint16_t *tlmBuf[3];

int main( int argc, char * argv[] ) {
	atexit( atExit );

	if( argc < 3 ) {
		printf( "%s", useMsg );
		return 0;
	}

	char * t;
	if(NULL!=(t = getenv("TERM_ROOT"))){
		term_root = strdup(t);
		//fprintf( stdout, "Переменная TERM_ROOT = %s\n",term_root );
		//fflush(stdout);
	}else{
		fprintf( stderr, "Переменная TERM_ROOT не установлена, будет использован текущий путь.\n" );
		fflush(stderr);
	}

	strcpy( exeFileName, argv[0] );
	strcpy( inputFileName, argv[argc-1] );
	strcpy( cfgName, argv[argc-2] );

	try {
		readCfg();
		parseCommandString( argc, argv );
	} catch (TException e) {
		fprintf( stderr, "%s\n", e.text() );
		exit(-1);
	}

	logfile = new CLog(("hunp.exe"));
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

	logfile->info( string("hunp.exe - ") + inputFileName );

	// Инициализация распаковываемых каналов с учетом параметров в конфигурационном файле. 
	memcpy( channels, is_a_present ? channels_a : channels_cfg, sizeof( channels ) );

	construct_file_names();
	logfile->debug( exeFileName );
	logfile->debug( inputFileName );
	logfile->debug( cfgName );
	for(int i=0;i<8;i++)
		logfile->debug( outputFileNames[i] );

	// Чтение 0-блока.
	{
		FILE * f = fopen( inputFileName, "rb" );
		if( f == NULL || fread( &bh, 512, 1, f ) != 1 ) {
			logfile->error( "ошибка доступа к входному файлу данных" );
			exit(1);
		}
		fclose( f );
	}

	// Проверяем, чтобы входной файл имел 0-блок нового формата. 
	if( bh.b0.formatType != 0xff ) {
		logfile->error( "неправильный формат входного файла данных" );
		exit(1);
	}

	totalFrameNum = bh.frameNum + bh.lostFrameNum;

	// Создание объекта-распаковщика. 
	logfile->info( "чтение входного файла данных" );
	THRPTUnpacker* unp = NULL;
	try {
		unp = new THRPTUnpacker( inputFileName, 1 );
	} catch( TException e ) {
		logfile->error( e.text() );
		exit(1);
	}

	construct_blk0();

	// если среди имеющихся в файле каналов нет выбранных для распаковки 
	if( !( (channels[1] && unp->channelAvailable(0)) || (channels[2] && unp->channelAvailable(1)) ||
			(channels[3] && unp->channelAvailable(2)) || (channels[4] && unp->channelAvailable(3)) ||
			(channels[5] && unp->channelAvailable(4)) ) )
		goto no_present_channels_selected;

	// Поочередная распаковка каналов AVHRR и запись в файлы. 
	for( int i = 0; i < 5; i++ ) {
		if( !(channels[i+1] && unp->channelAvailable( i )) )
			continue;

		char msg[100];
		sprintf( msg, "распаковка и запись в файл данных %d-го канала", i+1 );
		logfile->info( msg );

		FILE* f = NULL;
		if( NULL == ( f = fopen( outputFileNames[i], "wb" ) ) ) {
			logfile->error( "Ошибка записи файла !" );
		}

		ba.channel = i + 1;
		fwrite( &ba, 1, 512, f );

		memset( avhrrBuf, 0, sizeof( avhrrBuf ) );
		avhrrBuf[i] = scanBuf;

		unp->setCurrentFrameNumber( 0 );

		for( uint32_t j = 0; j < totalFrameNum; j++ ) {
			try {
				unp->unpackNextFrame( avhrrBuf, tlmBuf );
			} catch( TException e ) {
				if( e.id() != 2 ) {    // все исключения, кроме сбоя синхронизации при приёме кадра HRPT 
					logfile->error( e.text() );
					exit(1);
				}
				uint16_t *dst = avhrrBuf[i];
				uint16_t *dst_end = dst + bh.totalPixNum;
				while( dst != dst_end )
					*dst++ = (uint16_t)(-1);
			}
			fwrite( avhrrBuf[i], 2, bh.totalPixNum, f );
		}

		fclose( f );
	}

no_present_channels_selected:

	// считаем, что если в файле есть температурный канал, то есть и соответствующие ему данные телеметрии 
	if( !( (unp->channelAvailable(2) && channels[3]) || (unp->channelAvailable(3) && channels[4])
			|| (unp->channelAvailable(4) && channels[5]) ) )
		goto finish;

	// выделение памяти под буфера для телеметрии 
	for( int i = 3; i <= 5; i++ ) {
		if( channels[i] ) {
			tlmBuf[i-3] = new uint16_t [30 * totalFrameNum];
		}
	}
	uint16_t * pt[3];
	memcpy( pt, tlmBuf, sizeof( tlmBuf ) );

	memset( avhrrBuf, 0, sizeof( avhrrBuf ) );

	unp->setCurrentFrameNumber( 0 );

	// распаковка телеметрии 
	logfile->info( "распаковка данных телеметрии" );
	for( uint32_t i = 0; i < totalFrameNum; i++ ) {
		try {
			unp->unpackNextFrame( avhrrBuf, pt );
		} catch( TException e ) {   // очередной кадр HRPT был принят со сбоем синхронизации 
			if( e.id() != 2 ) {    // все исключения, кроме сбоя синхронизации при приёме кадра HRPT 
				logfile->error( e.text() );
				exit(1);
			}
			for( int j=0; j<3; j++ )
				if( pt[j] )
					memset( (char*)pt[j], 0, 60 );
		}
		for( int j = 0; j < 3; j++ )
			if( pt[j] )
				pt[j] += 30;
	}

	// запись файлов телеметрии 
	logfile->info( "запись файлов телеметрии" );
	for( int i = 3; i <= 5; i++ ) {
		if( channels[i] ) {
			FILE * f = fopen( outputFileNames[i+2], "wb" );
			bt.channel = i;
			fwrite( &bt, 1, 512, f );
			fwrite( tlmBuf[i-3], 2, 30 * totalFrameNum, f );
			fclose( f );
			delete [] tlmBuf[i-3];
		}
	}

finish:
	delete unp;

	logfile->info( "" );
	logfile->info( "работа программы завершена успешно !" );
	return 0;
}

void atExit() {
	if( logfile ) {
		logfile->debug( "atExit" );
		delete logfile;
	}
}

void readCfg() throw ( TException ) {
	//char msg[200];
	char path[MAX_PATH];
	const char * s;
	TCfg * cfg = NULL;

	if((NULL == strchr(cfgName,'\\'))&&
			(NULL == strchr(cfgName,'/'))&&
			(NULL == strchr(cfgName,'.'))) {
		strcpy(path,term_root);
		int t = strlen(path);
		// Если надо, в конце добавляем разделитель 
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

	if(cfg->containsParamWithKey( "LOG_LEVEL" )) {
		s = cfg->getValue( "LOG_LEVEL" );           // допустимые значения: dump,debug,info,warning,error,fatal 
		if( nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(string(s))))
			logLevel = nsCLog::info;
	}
	if(cfg->containsParamWithKey("LOG_STDERR")) {
		s = cfg->getValue( "LOG_STDERR" );           //
		if('1' == *s)
			useStdErr = true;
	}
	if(cfg->containsParamWithKey("LOG_STDOUT")) {
		s = cfg->getValue( "LOG_STDOUT" );           //
		if('1' == *s)
			useStdOut = true;
	}
	if(cfg->containsParamWithKey("LOG_APPEND")) {
		s = cfg->getValue( "LOG_APPEND" );           //
		logFileName = s;
		append = true;
	}
	if(cfg->containsParamWithKey("LOG_FILE")) {
		s = cfg->getValue( "LOG_FILE" );         // имя файла 
		logFileName = s;
		append = false;
	}

	try {
		// допустимые значения: строки из символов '1','2','3','4','5', например "245" 
		s = cfg->getValue( "UNP_AVHRR_CHANNELS" );

		// в buf формируется строка, очищенная от пробелов 
		char * buf = new char [strlen(s) + 1];
		const char * src = s;
		char * dst = buf;
		while( *src ) {
			if( !isspace( *src ) )
				*dst++ = *src;
			src++;
		}
		*dst = '\0';

		int t = atoi( buf );
		delete [] buf;              // больше не нужен 
		if( t <= 0 )
			throw TRequestExc( 100, "неправильное значение параметра UNP_AVHRR_CHANNELS" );
		while( t ) {
			int r = t % 10;
			if( r == 0 || r > 5 )
				TRequestExc( 100, "неправильное значение параметра UNP_AVHRR_CHANNELS" );
			channels_cfg[r] = true;
			t /= 10;
		}
	} catch( TRequestExc e ) {
		delete cfg;
		throw e;
	}

	delete cfg;
}


void construct_file_names() {
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], ext[MAX_EXT], s[MAX_FNAME];

	splitpath( inputFileName, drive, dir, fname, ext );

	for( int i = 0; i < 8; i++ ) {
		sprintf( s, "%s_%d", fname, i < 5 ? i + 1 : i - 2 );
		makepath( outputFileNames[i], drive, dir, s, i < 5 ? "avh" : "tlm" );
	}
}

//
//static int cmpicase( const char * c, const char * pd ){
//#if defined STRICMP
//    return stricmp( c, pd );
//#elif defined STRCASECMP
//	return strcasecmp( c, pd );
//#else
//#error "see config.h"
//#endif
//}

void construct_blk0() {
	// Постоянная часть паспорта. 
	memcpy( &ba, &bh, 62 );
	memcpy( &bt, &bh, 62 );
	// Коррекция: работа с файлами формата 1999 года. 
	if( strcmp( bh.b0.satName, "NOAA" ) == 0 && (bh.b0.satId & 0xffff) == 0 ) {
		ba.b0.satId =
			bh.b0.satId == 0x090000 ? TSatInfoTable::sat_id_noaa_9 :
			bh.b0.satId == 0x0a0000 ? TSatInfoTable::sat_id_noaa_10 :
			bh.b0.satId == 0x0b0000 ? TSatInfoTable::sat_id_noaa_11 :
			bh.b0.satId == 0x0c0000 ? TSatInfoTable::sat_id_noaa_12 :
			bh.b0.satId == 0x0e0000 ? TSatInfoTable::sat_id_noaa_14 : TSatInfoTable::sat_id_noaa_15;
		TSatInfoTable st;
		st.setToSatelliteWithId( ba.b0.satId );
		strcpy( ba.b0.satName, st.satName() );

		bt.b0.satId = ba.b0.satId;
		strcpy( bt.b0.satName, ba.b0.satName );
	}
	ba.b0.dataType1 = 2;    // одноканальные данные */
	ba.b0.dataType2 = 1;    // AVHRR */

	bt.b0.dataType1 = 4;    // телеметрия */
	bt.b0.dataType2 = 1;    // AVHRR

	// Переменная часть паспорта. */
	ba.totalFrameNum = totalFrameNum;
	ba.totalPixNum = bh.totalPixNum;
	ba.pixGap = bh.pixGap;
	ba.pixNum = bh.pixNum;
	ba.ascendFlag = bh.ascendFlag;
	ba.maxPixelValue = 1023;
	ba.ka = 1.;
	ba.kb = 0.;
	memcpy( (char*)&ba + 128, (char*)&bh + 128, 74 );   // Телеграмма NORAD. */
	memcpy( (char*)&ba + 256, (char*)&bh + 256, 30 );   // Параметры коррекции. */

	bt.totalFrameNum = totalFrameNum;
}


void parseCommandString( int argc, char* argv[] ) throw ( TException ) {
	string msg;
	int i = 1;
	while( i < argc ) {
		char * s = argv[i++];
		if( *s == '-' ) {
			s++;
			if( *s == 'a' ) {
				is_a_present = 1;
				s++;
				if( *s != '\0' ) {       // опция "-a{1|2|3|4|5}" */
					int t = atoi( s );
					if( t <= 0 )
						throw TException(100, "Недопустимая цифра в опции -a{1|2|3|4|5}");
					while( t ) {
						int r = t % 10;
						// Недопустимая цифра в опции -a{1|2|3|4|5} 
						if( r == 0 || r > 5 )
							throw TException(100, "Недопустимая цифра в опции -a{1|2|3|4|5}");
						channels_a[r] = 1;
						t /= 10;
					}
				} else {                   // опция -a */
					memset( channels_a, true, sizeof( channels_a ) );
				}
			} else if( *s == 'l' ) {
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
					//logLevel = nsCLog::getThresholdFromString(string(s));
				} else {
					msg.assign("Неизвестный параметр -l").append(s);
					throw TException(100, msg.c_str());
				}
			} else {
				msg.assign("Неизвестный параметр -").append(s);
				throw TException(100, msg.c_str());
			}
		}
	}
}
