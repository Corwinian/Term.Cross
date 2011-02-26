/*-------------------------------------------------------------------------
    hprj.cpp

    Утилита построения проекций.

    На вход программе может быть подан любой файл, переменная часть паспорта
    которого имеет формат TBlk0_AVHRR.

    Для файлов калиброванных данных температурных каналов производится
    нормирование пикселов проекции, с тем чтобы минимальное значение среди
    них стало равным 0. Соответственно изменяется значение полей паспорта
    kb и maxPixelValue.
-------------------------------------------------------------------------*/
#include <c_lib.hpp>
#include <Log.hpp>
#include <orbmodel.hpp>
#include <y_util.hpp>
#include <unistd.h>
#include <string>
using namespace std;
using namespace libconfig;

#include "hprj.hpp"

#define OUT_OF_IMAGE_PIXEL_VALUE (-2)
#define LAND_MASK_PIXEL_VALUE (-7)


#define IMPOSSIBLE_MIN_PIXEL_VALUE  32767

#ifndef LOG_FORMAT
#define LOG_FORMAT "%d %t %e %m"
#endif

char useMsg[] = "\
Утилита построения картографических проекций по файлам данных AVHRR.\n\
hprj [опции] имя_конфигурации файл_данных\n\
файл_данных     Файл одноканальных данных AVHRR.\n\
Опции:\n\
-m0             Не производить маскирование суши.\n\
-m1             Произвести маскирование суши.\n\
Опции, управляющие печатью сообщений в log-файл:\n\
-llevel={dump,debug,info,warning,error,fatal} уровень вывода сообщений.\n\
-lerr           Выводить сообщения в стандартный поток ошибок.\n\
-lout           Выводить сообщения в стандартный поток вывода.\n\
-lfile=<name>   Выводить сообщения в файл с именем name,\n\
файл создаётся заново\n\
-lappend=<name> Выводить сообщения в файл с именем name,\n\
сообщения дописываются в конец файла\n";

/*
	Опции из коммандной строки имеют более высокий приоритет чем опции из файла конфигураций.
*/
nsCLog::eSeverity logLevel = nsCLog::info;
bool useStdErr = false;
bool useStdOut = false;
string logFileName = string();
bool append = false;
CLog *logfile;

// Значение переменной  среды TERM_ROOT
const char* term_root = ".";

char exeFileName[MAX_PATH];        // argv[0]
char cfgName[MAX_FNAME];
char inputFileName[MAX_PATH];

/* вид маскирования: false - не маскировать ничего, true - маскировать сушу */
bool prjMask;

short * inputDataBuf;
short * prjDataBuf;

TBlk0_AVHRR ba;
TBlk0_Proj bp;

/*
 * параметры проекции - читаются напрямую из файла конфигурации,
 * как опции командной строки недоступны
 */
double prjLon, prjLat, prjLonSize, prjLatSize;      /* в градусах */
double prjRes;                                      /* в секундах */
/* тип проекции: 1 - меркаторская, 2 - равнопромежуточная */
int prjType;

/* размеры исходного изображения */
//long imgSizeX;
//long imgSizeY;
uint32_t imgSizeX;
uint32_t imgSizeY;

/* размеры проекции */
//long prjSizeX;
//long prjSizeY;
uint32_t prjSizeX;
uint32_t prjSizeY;

short maxPixel = 0;                             /* Вычисляется при построении проекции; корректируется в функции correctPixels() */
short minPixel = IMPOSSIBLE_MIN_PIXEL_VALUE;    /* Вычисляется при построении проекции с целью коррекции значений её пикселов. */

TProjMapper * mapper;

int main( int argc, char* argv[] ) {

	atexit( atExit );

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

	try {
		readCfg();
		parseCommandString( argc, argv );
	} catch (TException e) {
		fprintf( stderr, "%s\n", e.what() );
		exit(-1);
	}

	logfile = new CLog("hprj.exe");
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

	logfile->info( string("hprj.exe - ") + inputFileName );
	logfile->debug( exeFileName );
	logfile->debug( inputFileName );
	logfile->debug( cfgName );

	try {
		readInputFile();
		buildProj();
		saveProjFile();
	} catch(TException e) {
		logfile->error(e.text());
		exit(1);
	}

	delete [] inputDataBuf;
	delete [] prjDataBuf;

	logfile->info( "" );
	logfile->info( "работа программы завершена успешно !" );

	return 0;
}


void maskProj() throw (TException) {
	FILE * f;
	char drive[MAX_DRIVE], dir[MAX_DIR], maskpath[MAX_PATH], rmaskpath[MAX_PATH];
	TMaskFileHdr hdr;
	TMaskFileHdr rhdr;
	char * mask;        /* маска, соответствующая проекции */
	char * rmask;       /* маска всего региона приема */
	uint32_t rsizex, rsizey;
	double rlonres, rlatres;
	int rscan;

	splitpath( inputFileName, drive, dir, 0, 0 );
	makepath( maskpath, drive, dir, "hprj", "msk" );

	try {
		if( access( maskpath, 0 ) != 0 ) {
			throw int(1);               /* файл маски отсутствует */
		}

		logfile->info( "чтение файла маски проекции" );

		if( (f = fopen( maskpath, "rb" )) == NULL ) {
			throw TException( 100, "Ошибка открытия файла маски.");
		}

		if( fread( &hdr, sizeof( TMaskFileHdr ), 1, f ) != 1 ) {
			throw TException( 100, "Ошибка чтения файла маски." );
		}

		if( prjLon != hdr.lon || prjLat != hdr.lat ||
				prjLonSize != hdr.lonSize || prjLatSize != hdr.latSize ||
				prjSizeX != hdr.pixNum || prjSizeY != hdr.scanNum ||
				prjType != hdr.projType ) {    /* Файл маски не соответствует проекции, которая сейчас строится */

			fclose( f );
			throw int(1);
		}

		mask = new char [prjSizeX * prjSizeY];
		if( fread( mask, 1, prjSizeX * prjSizeY, f ) != prjSizeX * prjSizeY ) {
			throw TException( 100, "Ошибка чтения файла маски." );
		}

		fclose( f );

	} catch( int ) {
		/* Строим маску; записываем ее в файл, чтобы пользоваться ею при построении
		   проекций с такими же параметрами, что и сейчас. */
		strcpy(rmaskpath,term_root);
		int t = strlen(rmaskpath);
		/* Если надо, в конце добавляем разделитель */
		if(rmaskpath[t-1]!='/'||rmaskpath[t-1]!='\\'){
			rmaskpath[t] = DIRD; rmaskpath[t+1] = '\0';
		}
#if DIRD == '/'
		strcat( rmaskpath, "data/region.msk" );
#else
		strcat( rmaskpath, "data\\region.msk" );
#endif

		logfile->info( "чтение файла маски региона" );

		if( (f = fopen( rmaskpath, "rb" )) == NULL ) {
			throw TException( 100, "ошибка открытия файла маски региона" );
		}

		if( fread( &rhdr, sizeof( TMaskFileHdr ), 1, f ) != 1 ) {
			throw TException( 100, "ошибка чтения файла маски региона" );
		}

		rlonres = rhdr.lonSize / double( rhdr.pixNum );   /* градусы */
		rlatres = rhdr.latSize / double( rhdr.scanNum );  /* градусы */
		rsizex = rhdr.pixNum;
		rsizey = rhdr.scanNum;
		rmask = new char [rsizex * rsizey];

		if( fread( rmask, 1, rsizex * rsizey, f ) != rsizex * rsizey ) {
			throw TException( 100, "ошибка чтения файла маски региона" );
		}

		fclose( f );

		/* построение маски, соответствующей проекции */
		logfile->info( "построение маски проекции" );

		mask = new char [prjSizeX * prjSizeY];
		char * p = mask;
		for( uint32_t i = 0; i < prjSizeY; i++ ) {
			rscan = (int)((mapper->lat(i) * RD - rhdr.lat) / rlatres);
			char * q = rmask + rscan * rhdr.pixNum;
			for( uint32_t j = 0; j < prjSizeX; j++ )
				*p++ = q[int( (mapper->lon(j) * RD - rhdr.lon) / rlonres )];
		}

		delete [] rmask;

		/* заполнение заголовка файла маски */
		memset( &hdr, 0, sizeof( TMaskFileHdr ) );
		strcpy( hdr.signature, "MASK" );
		hdr.dataFormat = 1;
		hdr.projType = prjType;
		hdr.lon = prjLon;
		hdr.lat = prjLat;
		hdr.lonSize = prjLonSize;
		hdr.latSize = prjLatSize;
		hdr.pixNum = prjSizeX;
		hdr.scanNum = prjSizeY;

		/* запись файла маски */
		logfile->info( "запись файла маски проекции" );

		if( (f = fopen( maskpath, "wb" )) == NULL ) {
			throw TException( 100, "ошибка открытия для записи файла маски" );
		}

		if( fwrite( &hdr, sizeof( TMaskFileHdr ), 1, f ) != 1 ) {
			throw TException( 100, "ошибка записи файла маски" );
		}

		if( fwrite( mask, 1, prjSizeX * prjSizeY, f ) != prjSizeX * prjSizeY ) {
			throw TException( 100, "ошибка записи файла маски" );
		}

		fclose( f );
	}

	/* маскирование пикселов проекции */
	logfile->info( "маскирование проекции" );

	short * p = prjDataBuf;
	short * p_end = p + prjSizeX * prjSizeY;
	char * m = mask;
	do {
		if( *m++ )
			*p = LAND_MASK_PIXEL_VALUE;
	} while( ++p != p_end );

	delete [] mask;
}


void buildProj() throw ( TException ) {
	mapper = new TProjMapper( (TProjMapper::ProjType)prjType, prjLon * DR, prjLat * DR, prjLonSize * DR,
							  prjLatSize * DR, prjRes * DR / 3600., prjRes * DR / 3600. );
	prjSizeX = mapper->sizeX();
	prjSizeY = mapper->sizeY();

	prjDataBuf = new short [prjSizeX * prjSizeY];
	memset( prjDataBuf, 0, prjSizeX * prjSizeY );

	/* маскирование в буфере prjDataBuf пикселов суши значением LAND_MASK_PIXEL_VALUE */
	if( prjMask )
		maskProj();

	logfile->info( "построение проекции" );

	TIniSatParams isp( (TBlk0 &)ba );
	TNOAAImageParams nip( (TBlk0 &)ba );
	TCorrectionParams cop( (TBlk0 &)ba );
	TInverseReferencer ir( isp, nip, cop );

	int x, y;
	double lon, lat;
	short pixel;
	short * src = inputDataBuf;
	short * dst = prjDataBuf;

	for( uint32_t i = 0; i < prjSizeY; i++ ) {
		lat = mapper->lat( i );
		for( uint32_t j = 0; j < prjSizeX; j++, dst++ ) {
			if( *dst == LAND_MASK_PIXEL_VALUE )
				continue;  /* если пиксел проекции уже маскирован */

			lon = mapper->lon( j );
			if( lon > PI )
				lon -= PI;

			if( ir.ll2xy( lon, lat, &x, &y ) ) {     /* если пиксел проекции попадает в снимок */
				pixel = src[y*imgSizeX + x];
				if( pixel >= 0 ) {   /* если пиксел - значащий */
					if( pixel < minPixel )
						minPixel = pixel;
					if( pixel > maxPixel )
						maxPixel = pixel;
				}
				*dst = pixel;
			} else {                   /* пиксел проекции не попал в снимок */
				*dst = OUT_OF_IMAGE_PIXEL_VALUE;
			}
		}
	}

	delete mapper;

	if( minPixel == IMPOSSIBLE_MIN_PIXEL_VALUE )
		minPixel = 0;  /* если в проекции не оказалось ни одного значащего пиксела */

	correctPixels();
}


void saveProjFile() throw ( TException ) {
	FILE * f;
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], projfpath[MAX_PATH];

	logfile->info( "запись файла проекции" );

	splitpath( inputFileName, drive, dir, fname, 0 );
	makepath( projfpath, drive, dir, fname, "pro" );

	constructProjBlk0();

	if( (f = fopen( projfpath, "wb" )) == NULL ) {
		throw TException( 100, "ошибка создания файла проекции" );
	}

	if( fwrite( &bp, 512, 1, f ) != 1 ) {
		throw TException( 100, "ошибка записи файла проекции" );
	}

	if( fwrite( prjDataBuf, 2, prjSizeX * prjSizeY, f ) != prjSizeX * prjSizeY ) {
		throw TException( 100, "ошибка записи файла проекции" );
	}
	fclose( f );
}

/*
    Возвращаемые значения:
    0   ok
    1   Ошибка доступа ко входному файлу.
*/
void readInputFile() throw ( TException ) {
	FILE * f;

	logfile->info( "чтение входного файла данных" );

	if( (f  = fopen( inputFileName, "rb" )) == NULL || fread( &ba, 512, 1, f ) != 1 ) {
		throw TException( 100, "ошибка доступа ко входному файлу данных" );
	}

	if( ba.b0.formatType != 0xff || ba.b0.dataType1 != 2 || ba.b0.dataType2 != 1 ) {
		throw TException( 100, "неправильный формат входного файла данных" );
	}

	imgSizeX = ba.totalPixNum;
	imgSizeY = ba.totalFrameNum;

	inputDataBuf = new short [imgSizeX * imgSizeY];

	if( fread( inputDataBuf, 2, imgSizeX * imgSizeY, f ) != imgSizeX * imgSizeY ) {
		throw TException( 100, "ошибка доступа ко входному файлу данных" );
	}

	fclose( f );

}

void correctPixels() {
	logfile->info( "коррекция значений пикселов проекции" );

	/* для откалиброванных данных температурных каналов производим нормирование значений пикселов на 0 */
	if( ba.processLevel && (ba.channel == 3 || ba.channel == 4 || ba.channel == 5) && minPixel > 0 ) {
		short * p = prjDataBuf;
		short * p_end = p + prjSizeX * prjSizeY;
		do {
			if( *p > 0 )
				*p -= minPixel;
		} while( ++p != p_end );

		maxPixel -= minPixel;
	}
}

void constructProjBlk0() {
	/* Постоянная часть паспорта. */
	memcpy( &bp, &ba, 64 );
	bp.b0.dataType1 = 3;

	/* Переменная часть паспорта. */
	bp.processLevel = ba.processLevel;
	if( prjMask == 1 )
		bp.processLevel |= 0x00000008;  /* произведено маскирование суши */

	bp.channel = ba.channel;
	bp.maxPixelValue = maxPixel;
	bp.projType = prjType;

	bp.scanNum = prjSizeY;
	bp.pixNum = prjSizeX;

	bp.lat = prjLat;
	bp.lon = prjLon;
	bp.latSize = prjLatSize;
	bp.lonSize = prjLonSize;
	bp.latRes = prjRes;
	bp.lonRes = prjRes;

	bp.ka = ba.ka;
	bp.kb = ba.kb;

	/* для откалиброванных данных температурных каналов производим коррекцию поля kb */
	if( ba.processLevel && (ba.channel == 3 || ba.channel == 4 || ba.channel == 5) )
		bp.kb += ba.ka * double( minPixel );

	/* Телеграмма NORAD. */
	memcpy( (char *)&bp + 128, (char *)&ba + 128, 74 );

	/* Параметры коррекции. */
	memcpy( (char *)&bp + 256, (char *)&ba + 256, 30 );
}

void readCfg() throw ( TException ) {
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
	const Setting& Log = conf.getRoot()["Log"];

	#warning в соседнем лежит тоже самое подмать как бы вынести

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

	const Setting& prj = conf.getRoot()["Prj"];


	prj.lookupValue("lon", prjLon);  // градусы
	prj.lookupValue("lat", prjLat);  // градусы
	prj.lookupValue("lon_size", prjLonSize);  // градусы
	prj.lookupValue("lat_size", prjLatSize);  // градусы

	prj.lookupValue("res", prjRes);  // секунды
	prj.lookupValue("type", prjType);   // "1" - меркаторская, "2" - равнопромежуточная

	if( prjType != 1 && prjType != 2 ) {
		throw TRequestExc( 100, "Неправильное значение параметра PRJ_TYPE" );
	}

	prj.lookupValue("mask", prjMask);   // "0" - ничего не маскировать, "1" - маскировать сушу

	if( prjMask != 0 && prjMask != 1 ) {
		throw TRequestExc( 100, "Неправильное значение параметра PRJ_MASK" );
	}

}

void parseCommandString( int argc, char* argv[] ) throw ( TException ) {
	string msg;
	int i = 1;
	while( i < argc ) {
		char * s = argv[i++];
		if( *s == '-' ) {
			s++;
			if( *s == 'm' ) {
				s++;
				if( *s == '1' ) {        // опция "-m1"
					prjMask = true;
				} else if( *s == '0' ) {
					prjMask = false;
				} else {                   // неверный символ после "-m"
					throw TRequestExc( 100, "Неверный символ после -m" );
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

void atExit() {
	if( logfile ) {
		logfile->debug( "atExit" );
		delete logfile;
	}
}


