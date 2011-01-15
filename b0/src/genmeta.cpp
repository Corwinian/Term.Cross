/*---------------------------------------------------------------------
	HRPT2PCX.C
	Программа построения grayscale pcx. В отличие от своего
	прототипа HRPT2GS, данная программа не делает ничего
	специального с выходным файлом *.pcx. Выходной файл содержит
	только изображение канала и не содержит ни специальной панели
	с информацией о файле hrpt, ни дополнительных полей в заголовке.
	Данная программа позволяет пользователю получить и сетку и
	береговую линию, соответствующую данному снимку hrpt, только
	в отличие от HRPT2GS, сетка и береговая линия не накладываются
	на изображение внутри программы, а записываются в отдельный
	текстовый файл в векторном виде.
	Такое отделение сетки от изображения канала предназначается
	для программ, которые предполагают при выводе изображения
	производить zoom и resampling. 
	Date: 10 - 20 june 2008
---------------------------------------------------------------------*/
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <png.h>
#include <time.h>

#include <c_lib.hpp>
#include <Log.hpp>
#include <hrpt.hpp>
#include <y_util.hpp>
#include <orbmodel.hpp>

// Правка от SD
// В libpng1.4 данных макросов нет
#ifndef png_bytep_NULL 
#define png_bytep_NULL ((png_bytep)NULL)
#endif
#ifndef png_bytepp_NULL 
#define png_bytepp_NULL ((png_bytepp)NULL)
#endif

/* структура заголовка блока бинарного файла */
struct tCiaHdr{
	uint16_t type; /* тип блока ( линии ) */
	uint16_t count; /* количество точек в блоке ( линии ) */
	uint32_t ident; /* идентификационный номер блока */
};

/* структура данных бинарного файла */
struct tLatLon{
	float lat;
	float lon;
};

/* прототипы функций */
void parseCommandString( int argc, char* argv[] ) throw (TException);
void closeAll( );
//void vvector8( int, int, int, int, int, unsigned char*, int );
//
void img4c( void );
//void apply_net( void );
void writeNet( ) throw (TException);
void writeMeta( ) throw (TException);
void writePng( ) throw (TException);

/*
 * Log
 */
#ifdef DEBUG
#define _LOG_FORMAT_ "%d %t %e %m"
nsCLog::eSeverity logLevel = nsCLog::dump;
CLog logger( __FILE__ );
#endif

/* файловые переменные */
char* inpFile = NULL;
char* outDir = NULL;
char* coastFileName = NULL;

char pngFileName[MAX_PATH];
char metaFileName[MAX_PATH];
char netFileName[MAX_PATH];
//
char drive[MAX_DRIVE];
char dir[MAX_DIR];
char outName[MAX_FNAME];
char pngExt[MAX_EXT] = ".png";
char metaExt[MAX_EXT] = ".meta";
char netExt[MAX_EXT] = ".net";

char inpFileName[MAX_FNAME];
char inpFileExt[MAX_EXT];

int chNum = -1; /* номер канала для распаковки */

THRPTUnpacker* unp = NULL;

/* переменные орбитальной модели */
TIniSatParams* isp = NULL;
TNOAAImageParams* nip = NULL;
TCorrectionParams* cop = NULL;
// прямая и обратная задача
TStraightReferencer* srPtr = NULL;
TInverseReferencer* irPtr = NULL;

//
bool isWriteNet = false;
bool isWritePng = false;
bool isWriteMeta = false;
bool isVerbose = false;

//
int scanSize = 0;
int totalFrameNum = 0;
int imageWidth = 0;
int imageHeight = 0;
int imageSize = 0;
bool isAscend;

//
int year;
int month;
int date;
int hour;
int minute;
int sec;

/* координаты полигона */
double lat1, lon1;
double lat2, lon2;
double lat3, lon3;
double lat4, lon4;
/* географические координаты изображения */
double rightDownLon;
double rightDownLat;
//double midDownLon;
//double midDownLat;
double leftDownLon;
double leftDownLat;
double rightUpLon;
double rightUpLat;
//double midUpLon;
//double midUpLat;
double leftUpLon;
double leftUpLat;

uint16_t* scan = NULL;
uint16_t* avhrrBuf[] = { (uint16_t*)NULL, (uint16_t*)NULL, (uint16_t*)NULL, (uint16_t*)NULL, (uint16_t*)NULL };

png_bytep image = png_bytep_NULL;
png_bytepp imageRows = png_bytepp_NULL;

void printUsage( ){
	fprintf( stderr, "Syntax: genmeta [-v] [-dir <output dir>] [-meta|-png] [-chan <channel num>] [-net <coast_file_name>] <hrpt input file> \n" );
	fprintf( stderr, "<hrpt input file>\n" );
	fprintf( stderr, "\tinput HRPT file of f34 format;\n" );
	fprintf( stderr, "<output dir>\n" );
	fprintf( stderr, "\tdirectory where data is recorded;\n" );
	fprintf( stderr, "-meta|-png\n" );
	fprintf( stderr, "\tgenerate metadata or grayscale PNG, if both options are miss,\n" );
	fprintf( stderr, "\tgenerate metadata and grayscale PNG.\n" );
	fprintf( stderr, "<channel num>\n" );
	fprintf( stderr, "\tinput integer number in the range 1-5, that defines\n" );
	fprintf( stderr, "\tchannel to unpack from input file and to build grayscale PNG from it.\n" );
	fprintf( stderr, "\tThis parameter is necessary only for the normal HRPT file, i.e.\n" );
	fprintf( stderr, "\tfor 5-channel NOAA HRPT file, received on one of three hrpt\n" );
	fprintf( stderr, "\treceiving stations; if this parameter is not defined, program\n" );
	fprintf( stderr, "\tby default will unpack the first available channel from file.\n" );
	fprintf( stderr, "\tFor the artificial HRPT file, e.g. 1-channel HRPT file, that\n" );
	fprintf( stderr, "\twas built from 10-channel FENGYUN CHRPT file, this parameter\n" );
	fprintf( stderr, "\tis NOT necessary and ignored if defined.\n" );
	fprintf( stderr, "\tThe name of output grayscale PNG file is generated automatically.\n" );
	//	fprintf( stderr, "\tfrom the name of input file and from channel num.\n" );
	fprintf( stderr, "<coast file name>\n" );
	fprintf( stderr, "\toptional parameter, that defines the full path and name\n" );
	fprintf( stderr, "\tof the binary coast file, e.g. C:\\COAST.DAT. If this parameter defined,\n" );
	fprintf( stderr, "\tprogram will apply the geographical net and coast contour to the\n" );
	fprintf( stderr, "\tresulting grayscale image.\n" );
	fprintf( stderr, "-v\n\t generate verbouse output; print to STDOUT generated filenames.\n" );
}

int main( int argc, char* argv[] ){

	if( argc < 2 ){
		printUsage( );
		return 1;
	}
#ifdef DEBUG
	//	TCharString msg;
	logger.addAppender( new EConsoleLogAppender( logLevel, _LOG_FORMAT_ ) );
#endif

	try{
		parseCommandString( argc, argv );

		try{
			unp = new THRPTUnpacker( inpFile, 0 );
			TBlk0& blk0 = *(unp->blk0( ));
			isp = new TIniSatParams( blk0 );
			nip = new TNOAAImageParams( blk0 );
			cop = new TCorrectionParams( blk0 );
			srPtr = new TStraightReferencer( *isp, *nip, *cop );
			irPtr = new TInverseReferencer( *isp, *nip, *cop );
		}catch( TException e ){
			throw TException( 2, "ошибка доступа к входному файлу данных" );
		}
		TBlk0_Common& blk0c = unp->blk0( )->b0;

		if( blk0c.formatType != L34_FORMAT )
			throw TException( 3, "неправильный формат входного файла данных" );

		// В данной версии обрабатываем исходные NOAA HRPT
		if( blk0c.dataType1 != B0DT1_SRC && blk0c.dataType2 != B0DT2_HRPT_NOAA )
			throw TException( 3, "неподдерживаемый формат данных" );

		//		scan = new unsigned short[2048];

		//Если не указаны каналы или указанный канал недоступен, то берём первый попавшийся.
		if( chNum < 0 || chNum > 4 || !(unp->channelAvailable( chNum )) ){
			for( int i = 0; i < 5; i++ ){
				chNum = i;
				if( unp->channelAvailable( i ) ) break;
			}
			if( chNum > 4 ) throw TException( 5, "Fatal error! There are no accessible channels!" );
		}
//		avhrrBuf[chNum] = scan;

		TBlk0_HRPT& hrb0 = *((TBlk0_HRPT*)unp->blk0( ));

		isAscend = hrb0.ascendFlag;
		totalFrameNum = hrb0.frameNum + hrb0.lostFrameNum;
		scanSize = hrb0.totalPixNum;
		imageWidth = hrb0.totalPixNum / 2;
		imageHeight = totalFrameNum / 2 + (totalFrameNum & 1);
		imageSize = imageWidth * imageHeight;

		//проверяем файл на целостность, т.е. читаем последний фрейм
		unp->setCurrentFrameNumber( totalFrameNum - 1 );
		/* читаем время приема */
		/* дата начала приема */
		dayToDate1( blk0c.year, blk0c.day, &month, &date );
		year = blk0c.year;
		/* вычисляем часы, минуты, секунды */
		long ts = blk0c.dayTime / 1000;
		sec = ts % 60;
		ts = ts / 60;
		minute = ts % 60;
		hour = ts / 60;

		/* формируем идентификатор */
		/* удаляем концевые пробелы в b.satName */
		for (int i = strlen(blk0c.satName) - 1; i > 0; i--) {
			if (isalnum(blk0c.satName[i])) break;
			else blk0c.satName[i] = (char) 0;
		}

		sprintf( outName, "%s_%4d%02d%02d_%02d%02d%02d", blk0c.satName, blk0c.year, month, date, hour, minute, sec );

		int j = strlen( outName );
		for(int i = 0; i < j; i++ ){
			if( isalnum( outName[i] ) )
				outName[i] = toupper( outName[i] );
			else outName[i] = (char)'_';
		}
		splitpath( inpFile, drive, dir, inpFileName, inpFileExt );
		strcat(inpFileName,".");
		strcat(inpFileName,inpFileExt);

		img4c( );

		if( isWritePng ) writePng();
		if( isWriteMeta ) writeMeta();
#ifdef DEBUG
		if( isWriteNet ){
			logger.debug( "has net name" );
			logger.debug( netFileName );
		}
#endif
		/* если задано наложение сетки и береговой линии */
		if( isWriteNet ) writeNet( );
	}catch( TException e ){
		fprintf( stderr, "%s\n", e.text( ) );
		closeAll( );
		return ( e.id( ));
	}

	closeAll( );
	return 0;

}

void writeMeta( ) throw (TException){
	string msg;

	/* формируем имя выходного файла из идентификатора */
	if( outDir != NULL ){
		makepath( metaFileName, 0, outDir, outName, metaExt );
	}else{
		makepath( metaFileName, drive, dir, outName, metaExt );
	}

	/* создаем ( открываем ) выходной файл */
	FILE* outFile = fopen( metaFileName, "w" );
	if( outFile == NULL ){
#warning char*
		throw TException( 2, msg.assign( "Can't open output file:" ).append(metaFileName).c_str() );
	}

	TBlk0_HRPT& hrb0 = *((TBlk0_HRPT*)unp->blk0( ));

	double scan_time = (double)hrb0.b0.day + (double)hrb0.b0.dayTime/86400000.0;

	int endYear = hrb0.b0.year;
	int endMonth;
	int endDate;
	double endTime = scan_time;
	dayAdd ( &endYear, &endTime, (double)(totalFrameNum-1)/6.0/86400.0 );
	double entire;
	double fraction = modf ( endTime, &entire );
	dayToDate1 ( endYear, entire, &endMonth, &endDate );
	fraction = modf ( fraction * 86400000.0, &entire );
	/* вычисляем часы, минуты, секунды */
	int64_t ts = (int64_t)entire/1000;
	uint16_t end_sec = ts%60;
	ts = ts/60;
	uint16_t end_minute = ts%60;
	uint16_t end_hour = ts/60;

	/* записываем информацию в выходной файл */
	fprintf( outFile, "collection=SML.NOAA_HRPT.LONGTIME\n" );
	fprintf( outFile, "id=%s\n", outName );
	fprintf( outFile, "file=%s\n", inpFileName );
	fprintf( outFile, "satellite=%s\n", hrb0.b0.satName );
	fprintf( outFile, "satelliteNum=%d\n", hrb0.b0.satId );
	fprintf( outFile, "revNum=%d\n", hrb0.b0.revNum );
	fprintf( outFile, "orbitType=%c\n", (isAscend) ? 'A' : 'D' );
	fprintf( outFile, "timeBegin=%4d-%02d-%02dT%02d:%02d:%02dZ\n",
			year, month, date, hour, minute, sec );
	fprintf( outFile, "timeEnd=%4d-%02d-%02dT%02d:%02d:%02dZ\n",
			endYear, endMonth, endDate, end_hour, end_minute, end_sec );
	fprintf( outFile, "polygon=%.2lf,%.2lf %.2lf,%.2lf %.2lf,%.2lf %.2lf,%.2lf\n",
			lat1*RD, lon1*RD, lat2*RD, lon2*RD,
			lat3*RD, lon3*RD, lat4*RD, lon4*RD );
	fprintf( outFile, "eoliPolygon=%.2lf,%.2lf %.2lf,%.2lf %.2lf,%.2lf %.2lf,%.2lf\n",
			lat1*RD, (lon1<0?lon1+2*PI:lon1)*RD, lat2*RD, (lon2<0?lon2+2*PI:lon2)*RD,
			lat3*RD, (lon3<0?lon3+2*PI:lon3)*RD, lat4*RD, (lon4<0?lon4+2*PI:lon4)*RD );
	fprintf( outFile, "totalScans=%d\n", totalFrameNum );
	fprintf( outFile, "goodScans=%d\n", hrb0.frameNum );

	int channels[5];
	int chCount = 0;
	unp->channelsAvailability(channels);
	for(int i = 0; i<5; i++) if(channels[i]) chCount++;

	/* если присутствуют не все каналы AVHRR, распечатываем их номера */
	if( chCount < 5 ){
		fprintf( outFile, "channels=" );
		for( int i = 0; i < 5; i++ ){
			if( channels[i] )
				fprintf( outFile, "%c", (char)('0' + i ) );
		}
		fprintf( outFile, "\n" );
	}

	time_t ctm;
	struct tm *gtm;

	time(&ctm);
	gtm = gmtime(&ctm);
	fprintf( outFile, "metadataTime=%4d-%02d-%02dT%02d:%02d:%02dZ\n",
			1900+gtm->tm_year, gtm->tm_mon, gtm->tm_mday, gtm->tm_hour, gtm->tm_min, gtm->tm_sec );

	fclose(outFile);
	if(isVerbose) printf("%s\n", metaFileName);
}

void writePng( ) throw (TException){
	int i, j, k;
	int lowLevel = 1;
	int highLevel = 85;
	int autoLow = 0;
	int autoHigh = 0;
	uint16_t chMin = 1023;
	uint16_t chMax = 0;
	//		int minIdx = 255;
	//		int maxIdx = 0;
	//		int fIdx;
	//		int bIdx;
	int hist[256];
	int sumHist[256];
	int lookup[256];
	memset( hist, 0, sizeof (hist) );
	memset( sumHist, 0, sizeof (hist) );
	memset( lookup, 0, sizeof (hist) );

	if( outDir != NULL ){
		makepath( pngFileName, 0, outDir, outName, pngExt );
	}else{
		makepath( pngFileName, drive, dir, outName, pngExt );
	}

	scan = new uint16_t[scanSize];
	avhrrBuf[chNum] = scan;
	image = new png_byte[imageSize];
	png_bytep imagePtr = image;
	if( isAscend ) imagePtr += imageSize - 1;

	png_bytep imageRows[imageHeight];
	for( i = 0; i < imageHeight; i++ ) imageRows[i] = image + i * imageWidth;

	for( i = 0; i < totalFrameNum; i += 2 ){
		unp->setCurrentFrameNumber( i );
		try{
			unp->unpackNextFrame( avhrrBuf );
		}catch( TException e ){
			if( e.id( ) != 2 ) throw e;
			imagePtr += isAscend ? -imageWidth : imageWidth;
			continue;
		}
		uint16_t* scanPtr = scan;

		for( j = 0; j < imageWidth; j++ ){
			png_byte p = (png_byte)((*scanPtr) >> 2);
			*imagePtr = p;
			/* гистограммируем изображение */
			hist[(int)(*imagePtr)]++;
			/* находим диапазон канала */
			if( (*scanPtr) > chMax ) chMax = *scanPtr;
			if( (*scanPtr) < chMin ) chMin = *scanPtr;
			/* следующая точка ( zoom = 2 ) */
			scanPtr += 2;
			/* учитываем, какой виток, восходящий или нет */
			imagePtr += isAscend ? -1 : 1;
			//imagePtr++;
		}

	}

	/* подсчитываем интеграл от гистограммы */
	j = 0;
	hist[0] = 0; /* при вычислении порогов индекс цвета 0 не учитывается */
	for( i = 0; i < 256; i++ ){
		j += hist[i];
		sumHist[i] = j;
	}
	/* вычисляем пороги */
	j = (int)((double)sumHist[255] * (double)lowLevel / 100.0);
	i = 0;
	while( sumHist[i] < j ) i++;
	autoLow = i;
	j = (int)((double)sumHist[255] * (double)highLevel / 100.0);
	i = 255;
	while( sumHist[i] >= j ) i--;
	autoHigh = i;

	/* делаем таблицу lookup по форме интеграла от гистограммы
					в диапазоне от auto_low до auto_high */
	for( i = 0; i < autoLow; i++ ) lookup[i] = 0;
	for( i = autoHigh; i < 256; i++ ) lookup[i] = 0xff;
	/* нормировочный коэффициент */
	k = (sumHist[autoHigh] - sumHist[autoLow]) / 255;
	for( i = autoLow; i < autoHigh; i++ ) lookup[i] = (sumHist[i] - sumHist[autoLow]) / k;

#ifdef DEBUG
	logger.dump( "hist", (char*)hist, sizeof (int)*256 );
	logger.dump( "sumHist", (char*)sumHist, sizeof (int)*256 );
	logger.dump( "lookup", (char*)lookup, sizeof (int)*256 );
#endif
	/* транслируем изображение по таблице */
	imagePtr = image;
	while( imagePtr < image + imageSize ){
		i = (int)(*imagePtr) & 0xff;
		*imagePtr++ = (unsigned char)lookup[i];
	}


	FILE* outHandle = fopen( pngFileName, "wb+" );
	if( !outHandle ) throw TException( 5, "[write_png_file] Error to open output file" );

	/* initialize stuff */
	png_structp pngPtr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

	if( !pngPtr ){
		fclose( outHandle );
		throw TException( 5, "[write_png_file] png_create_write_struct failed" );
	}

	png_infop infoPtr = png_create_info_struct( pngPtr );
	if( !infoPtr ){
		fclose( outHandle );
		throw TException( 5, "[write_png_file] png_create_info_struct failed" );
	}

	png_init_io( pngPtr, outHandle );

	/* write header */
	//	if (setjmp(png_jmpbuf(png_ptr)))
	//		abort_("[write_png_file] Error during writing header");

	png_set_IHDR( pngPtr, infoPtr, imageWidth, imageHeight,
			8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

#ifdef PNG_TEXT_SUPPORTED
	/* Optionally write comments into the image */
	png_text textPtr[3];
	textPtr[0].key = (char*)"provider";
	textPtr[0].text = (char*)"SML IACP RAS";
	textPtr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	textPtr[1].key = (char*)"collection";
	textPtr[1].text = (char*)"SML.NOAA_HRPT.LONGTIME";
	textPtr[1].compression = PNG_TEXT_COMPRESSION_NONE;
	textPtr[2].key = (char*)"id";
	textPtr[2].text = outName;
	textPtr[2].compression = PNG_TEXT_COMPRESSION_NONE;
	png_set_text( pngPtr, infoPtr, textPtr, 3 );
#endif

	png_write_info( pngPtr, infoPtr );

	/* write bytes */
	//	if (setjmp(png_jmpbuf(png_ptr)))
	//		abort_("[write_png_file] Error during writing bytes");

	png_write_image( pngPtr, imageRows );


	/* end write */
	//	if (setjmp(png_jmpbuf(png_ptr)))
	//		abort_("[write_png_file] Error during end of write");

	png_write_end( pngPtr, NULL );

	fclose( outHandle );
	if(isVerbose) printf("%s\n", pngFileName);
}
// hrpt2png [-dir <output dir>] [-chan {1|2|3|4|5}] [-net <coast_file_name>] <hrpt_input_file>

void parseCommandString( int argc, char* argv[] ) throw (TException){
	string msg;
	int i = 1;
	while( i < argc ){
		char* s = argv[i++];
		if( *s == '-' ){
			s++;
			if( strcmp( s, "chan" ) == 0 ){
				if( i == argc || strlen( argv[i] ) == 0 )
					#warning char*
					throw TException( 1, msg.assign( "Invalid param -chan" ).c_str() );
				s = argv[i++];
				if( isdigit( *s ) ){ // опция "-chan {1|2|3|4|5}" */
					int t = atoi( s );
					#warning char*
					if( t < 1 || t > 5 ) throw TException( 1, msg.assign( "Invalid channel number: " ).append( s ).c_str() );
					chNum = t - 1;
				}
				#warning char*
				else throw TException( 1, msg.assign( "Invalid param -chan " ).append( s ).c_str() );
			}else if( strcmp( s, "meta" ) == 0 ){
				isWriteMeta = true;
			}else if( strcmp( s, "png" ) == 0 ){
				isWritePng = true;
			}else if( strcmp( s, "v" ) == 0 ){
				isVerbose = true;
			}else if( strcmp( s, "dir" ) == 0 ){
				if( i == argc || strlen( argv[i] ) == 0 )
				  #warning char*
					throw TException( 1, msg.assign( "Invalid param -dir" ).c_str() );
				outDir = argv[i++];
			}else if( strcmp( s, "net" ) == 0 ){
				if( i == argc || strlen( argv[i] ) == 0 )
				  #warning char*
					throw TException( 1, msg.assign( "Invalid param -net" ).c_str() );
				coastFileName = argv[i++];
				isWriteNet = true;
			}
			#warning char*
			else throw TException( 1, msg.assign( "Invalid param -" ).append( s ).c_str() );
		}else inpFile = s;
		/*
		} else if( i == argc ){
			inpFile = s;
		} else {
			throw TException(1, msg.assign("The input file is not specified"));
		}
		 */
	}
	if( !(isWriteMeta || isWritePng) ) isWriteMeta = isWritePng = true;

	if( inpFile == NULL )
	  #warning char*
		throw TException( 1, msg.assign( "The input file is not specified" ).c_str() );
}

/*---------------------------------------------------------------------
	CLOSE_ALL.C
---------------------------------------------------------------------*/
void closeAll( ){
	if( irPtr ) delete irPtr;
	if( srPtr ) delete srPtr;
	if( cop ) delete cop;
	if( nip ) delete nip;
	if( isp ) delete isp;
	if( unp ) delete unp;
	if( scan ) delete scan;
	if( imageRows ) delete imageRows;
	if( image ) delete image;
	//	if( inpHandle ) fclose( inpHandle );
	//	if( outHandle )	fclose( outHandle );
}

/**
 * Географические координаты охватывающего полигона.
 * Порядок:
 *   первая (правая) точка первой строки, последняя (левая) точка первой строки,
 *   N промежуточных точек левой границы,
 *   последняя (левая) точка последней строки, первая (правая) точка последней строки
 *   N промежуточных точек правой границы
 *
 *   Для нисходящих:                 Для восходящих:
 *                   2     1                         4     3
 *                   .     .                         .     .
 *                   .     .                         .     .
 *                   .     .                         .     .
 *                   3     4                         1     2
 *
 */
void img4c( ){
	TStraightReferencer& sr = *srPtr;
	/* вычисляем и записываем координаты углов и подспутниковой
	трассы ( считаем, что виток восходящий ) */

	//sr.xy2ll( 0, 0, &rightDownLon, &rightDownLat );
	//sr.xy2ll( 0, 0, &lon1, &lat1 );
	sr.xy2ll( scanSize - 1, 0, &lon1, &lat1 );

	//sr.xy2ll( 2047, 0, &leftDownLon, &leftDownLat );
	sr.xy2ll( 0, 0, &lon2, &lat2 );

	//	sr.xy2ll( 1024, 0, &midDownLon, &midDownLat );
	//	sr.xy2ll( 1024, totalFrameNum - 1, &midUpLon, &midUpLat );

	//sr.xy2ll( 2047, totalFrameNum - 1, &leftUpLon, &leftUpLat );
	sr.xy2ll( 0, totalFrameNum - 1, &lon3, &lat3 );

	//sr.xy2ll( 0, totalFrameNum - 1, &rightUpLon, &rightUpLat );
	sr.xy2ll( scanSize - 1, totalFrameNum - 1, &lon4, &lat4 );

	/* если виток нисходящий, меняем местами углы крест-накрест */;
	leftUpLon = isAscend ? lon4 : lon2;
	leftUpLat = isAscend ? lat4 : lat2;
	rightUpLon = isAscend ? lon3 : lon1;
	rightUpLat = isAscend ? lat3 : lat1;
	rightDownLon = isAscend ? lon2 : lon4;
	rightDownLat = isAscend ? lat2 : lat4;
	leftDownLon = isAscend ? lon1 : lon3;
	leftDownLat = isAscend ? lat1 : lat3;
}

/*---------------------------------------------------------------------
	WRITE_NET.C
	Функция аналогична apply_net.c, только она не накладывает
	сетку и береговую линию на изображение, а записывает эту
	информацию в векторном виде в выходной файл.

	Отметим, что в отличие от aplly_net, сетка и береговая линия
	записываются в выходной файл в координатах функции LL2IJ,
	то есть 0 <= x <= 2047, 0 <= y <= frame_c-1.
	Date: 20 june 2008

	Изменение: сетка и береговая линия опять записываются в
	выходной файл в координатах 0 <= x <= 1023.
	Date: 25 june 2008
---------------------------------------------------------------------*/
void writeNet( ) throw (TException){
	int i;
	int j;
	unsigned short nxRes = 0;
	unsigned short nyRes = 0;
	unsigned short x, y, xOld, yOld;
	unsigned short extremFlag;

	double step5 = 5.0;
	double step1 = 1.0;
	double step5dr;
	double step1dr;
	double step;
	double tLon;
	double lon;
	double lat;
	double minLon;
	double minLat;
	double maxLon;
	double maxLat;
	double cx;
	double cy;
	double di;
	double dx;
	double dj;
	double dy;
	double curDi, curDj;
	double saveL;

	/* переменные для чтения бинарного файла */
	struct tCiaHdr blk;
	struct tLatLon cur;
	FILE* coastFile = NULL;
	int coastFileLen;
	unsigned char* buf = NULL;
	unsigned char* p = NULL;
	unsigned short blkType = 0;
	unsigned short pointCount = 0;
	unsigned short startOff = 0;

	TStraightReferencer& sr = *srPtr;
	TInverseReferencer& ir = *irPtr;

	/* переменные для записи текстового файла */
	FILE* netFile = NULL;
	if( outDir != NULL )
		makepath( netFileName, 0, outDir, outName, netExt );
	else
		makepath( netFileName, drive, dir, outName, netExt );

	/* находим минимум и максимум широты и долготы */
	extremFlag = 0;
	if( isAscend ){ /* восходящий виток */
		/* начальные установки */
		minLon = leftUpLon;
		if( minLon < 0.0 ) minLon += 2.0 * PI;
		maxLon = rightDownLon;
		if( maxLon < 0.0 ) maxLon += 2.0 * PI;
		minLat = leftDownLat;
		maxLat = rightUpLat;
		for( i = 1; i < LAVHRR / 2; i++ ){
			//			IJ2LL( (double)i, (double)(frame_c - 1), &lon, &lat );
			sr.xy2ll( i, totalFrameNum - 1, &lon, &lat );
			/* непрерывный переход через 180 градусов долготы */
			if( lon < 0.0 ) lon += 2.0 * PI;
			/* находим максимум долготы и широты */
			if( lon > maxLon ) maxLon = lon;
			else extremFlag |= 1;
			if( lat > maxLat ) maxLat = lat;
			else extremFlag |= 2;
			if( extremFlag == 3 ) break;
		}
	}else{ /* нисходящий виток */
		/* начальные установки */
		minLon = leftDownLon;
		if( minLon < 0.0 ) minLon += 2.0 * PI;
		maxLon = rightUpLon;
		if( maxLon < 0.0 ) maxLon += 2.0 * PI;
		minLat = rightDownLat;
		maxLat = leftUpLat;
		for( i = 1; i < LAVHRR / 2; i++ ){
			sr.xy2ll( (double)i, (double)0, &lon, &lat );
			/* непрерывный переход через 180 градусов долготы */
			if( lon < 0.0 ) lon += 2.0 * PI;
			/* находим минимум долготы и максимум широты */
			if( lon < minLon ) minLon = lon;
			else extremFlag |= 1;
			if( lat > maxLat ) maxLat = lat;
			else extremFlag |= 2;
			if( extremFlag == 3 ) break;
		}
	}
	/* минимум и максимум устанавливается как число градусов,
	больше максимума или меньше минимума и кратное step_5 градусов */
	step5dr = step5 * DR;
	modf( minLat / step5dr, &minLat );
	minLat *= step5dr;
	modf( minLon / step5dr, &minLon );
	minLon *= step5dr;
	modf( maxLat / step5dr, &maxLat );
	maxLat = (maxLat + 1.0) * step5dr;
	modf( maxLon / step5dr, &maxLon );
	maxLon = (maxLon + 1.0) * step5dr;

	/* вычисляем коэффициенты для перевода i, j
	в номера экранных пикселов */
	cx = (double)(imageWidth - 1) / (double)(LAVHRR - 1);
	cy = (double)((totalFrameNum - 1) / 2) / (double)(totalFrameNum - 1);
	//	cx = 1.0;
	//	cy = 1.0;

	/* размер экранной сетки - 1 */
	modf( (double)(LAVHRR - 1) * cx, &dx );
	nxRes = (unsigned short)dx;
	modf( (double)(totalFrameNum - 1) * cy, &dy );
	nyRes = (unsigned short)dy;

	/* инициализация обратной задачи */
	//	i = iniLL2IJ( &isp, &nip, &cop );
	//	if( i == -1 ) return;

	/* открываем выходной файл */
	netFile = fopen( netFileName, "w" );
	if( netFile == NULL ) throw TException( 5, "can'nt write to net file" );

	/* отрисовываем сетку меридианов */
	step1dr = step1 * DR;
	for( lon = minLon + step5dr; lon < maxLon - step5dr / 2.; lon += step5dr ){
		tLon = lon;
		if( lon >= PI ) tLon -= 2.0 * PI;
		lat = minLat;
m3:
		;
		/* ищем попадание кривой в область */
		while( ir.ll2xy( tLon, lat, &di, &dj ) == 0 ){
			if( lat < maxLat + step1dr / 2. ) lat += step1dr;
			else goto m2; /* кривая не пересекает область */
		}
		/* первое попадание кривой в область */
		modf( di + 0.5, &di );
		modf( di * cx, &dx );
		x = (unsigned short)dx;
		modf( dj + 0.5, &dj );
		modf( dj * cy, &dy );
		y = (unsigned short)dy;
		if( isAscend ){
			x = nxRes - x;
			y = nyRes - y;
		}
		/* производим аппроксимацию назад для получения точки
		пересечения с границей */
		saveL = lat;
		curDi = di;
		curDj = dj;
		step = step1dr / 2.0;
		/* и вычисляем пересечение методом дихотомии */
		while( 1 ){
			if( ir.ll2xy( tLon, lat - step, &di, &dj ) ){
				modf( di + 0.5, &di );
				modf( dj + 0.5, &dj );
				lat -= step;
				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
				else{
					curDi = di;
					curDj = dj;
				}
			}
			step /= 2.0;
		}
		/* точка пересечения в качестве начальной точки */
		modf( di * cx, &dx );
		xOld = (unsigned short)dx;
		modf( dj * cy, &dy );
		yOld = (unsigned short)dy;
		if( isAscend ){
			xOld = nxRes - xOld;
			yOld = nyRes - yOld;
		}
		/* записываем информацию в выходной файл */
		fprintf( netFile, "# M %.0lf\n", tLon * RD );
		fprintf( netFile, "%d %d\n", xOld, yOld );
		/* выводим недостающий вектор */
		if( xOld != x || yOld != y ){
			fprintf( netFile, "%d %d\n", x, y );
			xOld = x;
			yOld = y;
		}
		/* восстанавливаем широту */
		lat = saveL;
		if( lat < maxLat + step1dr / 2. ) lat += step1dr;

		/* ищем выпадание кривой из области */
		while( ir.ll2xy( tLon, lat, &di, &dj ) ){
			modf( di + 0.5, &di );
			modf( di * cx, &dx );
			x = (unsigned short)dx;
			modf( dj + 0.5, &dj );
			modf( dj * cy, &dy );
			y = (unsigned short)dy;
			if( isAscend ){
				x = nxRes - x;
				y = nyRes - y;
			}
			/* выводим очередной вектор */
			fprintf( netFile, "%d %d\n", x, y );
			xOld = x;
			yOld = y;
			if( lat < maxLat + step1dr / 2. ) lat += step1dr;
		}

		/* производим аппроксимацию вперед для получения точки
		пересечения с границей */
		saveL = lat;
		step = step1dr / 2.0;
		if( lat > minLat + step ) lat -= step1dr;
		curDi = di;
		curDj = dj;
		/* и вычисляем пересечение методом дихотомии */
		while( 1 ){
			if( ir.ll2xy( tLon, lat + step, &di, &dj ) ){
				modf( di + 0.5, &di );
				modf( dj + 0.5, &dj );
				lat += step;
				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
				else{
					curDi = di;
					curDj = dj;
				}
			}
			step /= 2.0;
		}
		/* точка пересечения в качестве конечной точки */
		modf( di * cx, &dx );
		x = (unsigned short)dx;
		modf( dj * cy, &dy );
		y = (unsigned short)dy;
		if( isAscend ){
			x = nxRes - x;
			y = nyRes - y;
		}
		/* выводим недостающий вектор */
		if( xOld != x || yOld != y ){
			fprintf( netFile, "%d %d\n", x, y );
			xOld = x;
			yOld = y;
		}
		/* восстанавливаем широту */
		lat = saveL;

		/* на всякий случай ищем второе попадание в область */
		goto m3;
m2:
		;
	}

	/* отрисовываем сетку параллелей */
	for( lat = minLat + step5dr; lat < maxLat - step5dr / 2.; lat += step5dr ){
		/* ищем попадание кривой в область */
		lon = minLon;
m5:
		;
		while( 1 ){
			tLon = lon;
			if( tLon >= PI ) tLon -= 2.0 * PI;
			if( ir.ll2xy( tLon, lat, &di, &dj ) ) break;
			if( lon < maxLon + step1dr / 2. ) lon += step1dr;
			else goto m4; /* кривая не пересекает область */
		}
		/* первое попадание кривой в область */
		modf( di + 0.5, &di );
		modf( di * cx, &dx );
		x = (unsigned short)dx;
		modf( dj + 0.5, &dj );
		modf( dj * cy, &dy );
		y = (unsigned short)dy;
		if( isAscend ){
			x = nxRes - x;
			y = nyRes - y;
		}
		/* производим аппроксимацию назад для получения точки
		пересечения с границей */
		saveL = lon;
		curDi = di;
		curDj = dj;
		step = step1dr / 2.0;
		/* и вычисляем пересечение методом дихотомии */
		while( 1 ){
			tLon = lon - step;
			if( tLon >= PI ) tLon -= 2.0 * PI;
			if( ir.ll2xy( tLon, lat, &di, &dj ) ){
				modf( di + 0.5, &di );
				modf( dj + 0.5, &dj );
				lon -= step;
				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
				else{
					curDi = di;
					curDj = dj;
				}
			}
			step /= 2.0;
		}
		/* точка пересечения в качестве начальной точки */
		modf( di * cx, &dx );
		xOld = (unsigned short)dx;
		modf( dj * cy, &dy );
		yOld = (unsigned short)dy;
		if( isAscend ){
			xOld = nxRes - xOld;
			yOld = nyRes - yOld;
		}
		/* записываем информацию в выходной файл */
		fprintf( netFile, "# P %.0lf\n", lat * RD );
		fprintf( netFile, "%d %d\n", xOld, yOld );
		/* выводим недостающий вектор */
		if( xOld != x || yOld != y ){
			fprintf( netFile, "%d %d\n", x, y );
			xOld = x;
			yOld = y;
		}
		/* восстанавливаем долготу */
		lon = saveL;
		if( lon < maxLon + step1dr / 2. ) lon += step1dr;

		/* ищем выпадание кривой из области */
		while( 1 ){
			tLon = lon;
			if( tLon >= PI ) tLon -= 2.0 * PI;
			if( ir.ll2xy( tLon, lat, &di, &dj ) == 0 ) break;
			modf( di + 0.5, &di );
			modf( di * cx, &dx );
			x = (unsigned short)dx;
			modf( dj + 0.5, &dj );
			modf( dj * cy, &dy );
			y = (unsigned short)dy;
			if( isAscend ){
				x = nxRes - x;
				y = nyRes - y;
			}
			fprintf( netFile, "%d %d\n", x, y );
			xOld = x;
			yOld = y;
			if( lon < maxLon + step1dr / 2. ) lon += step1dr;
		}

		/* производим аппроксимацию вперед для получения точки
		пересечения с границей */
		saveL = lon;
		step = step1dr / 2.0;
		if( lon > minLon + step ) lon -= step1dr;
		curDi = di;
		curDj = dj;
		/* и вычисляем пересечение методом дихотомии */
		while( 1 ){
			tLon = lon + step;
			if( tLon >= PI ) tLon -= 2.0 * PI;
			if( ir.ll2xy( tLon, lat, &di, &dj ) ){
				modf( di + 0.5, &di );
				modf( dj + 0.5, &dj );
				lon += step;
				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
				else{
					curDi = di;
					curDj = dj;
				}
			}
			step /= 2.0;
		}
		/* точка пересечения в качестве конечной точки */
		modf( di * cx, &dx );
		x = (unsigned short)dx;
		modf( dj * cy, &dy );
		y = (unsigned short)dy;
		if( isAscend ){
			x = nxRes - x;
			y = nyRes - y;
		}
		/* выводим недостающий вектор */
		if( xOld != x || yOld != y ){
			fprintf( netFile, "%d %d\n", x, y );
			xOld = x;
			yOld = y;
		}
		/* восстанавливаем долготу */
		lon = saveL;

		/* на всякий случай ищем второе попадание в область */
		goto m5;
m4:
		;
	}

	/* построение береговой линии */
	/* открываем входной файл */
	coastFile = fopen( coastFileName, "rb" );
	if( coastFile == NULL ){
		fclose( netFile );
		throw TException( 5, "can'nt open coast file" );
	}
	/* определяем длину файла */
	fseek( coastFile, 0, SEEK_END );
	coastFileLen = ftell( coastFile );
	if( coastFileLen == 0 ){
		fclose( netFile );
		fclose( coastFile );
		throw TException( 5, "zero length coast file" );
		;
	}
	fseek( coastFile, 0, SEEK_SET );
	/* выделяем буфер ввода/вывода */
	buf = (unsigned char*)malloc( coastFileLen );
	if( buf == NULL ){
		fclose( netFile );
		fclose( coastFile );
		throw TException( 5, "mem allocation error" );
		;
	}
	/* читаем весь файл в буфер */
	fread( buf, 1, coastFileLen, coastFile );
	fclose( coastFile );

	p = buf;
	while( 1 ){
		/* здесь p всегда указывает на заголовок */
		memcpy( &blk, p, 8 );

		/* получаем из заголовка параметры блока */
		blkType = blk.type;
		if( blkType == 0 ) break;
		pointCount = blk.count;
		if( pointCount == 0 ) break;

		/* позиционируемся на минимумы блока */
		p += 8;
		/* получаем минимумы блока */
		memcpy( &cur, p, 8 );
		minLat = (double)cur.lat;
		minLon = (double)cur.lon;

		/* позиционируемся на максимумы блока */
		p += 8;
		/* получаем максимумы блока */
		memcpy( &cur, p, 8 );
		maxLat = (double)cur.lat;
		maxLon = (double)cur.lon;

		/* позиционируемся на первую точку блока */
		p += 8;
		/* если ни одна из точек блока не попадает в приемную область */
		if( ir.ll2xy( minLon, minLat, &di, &dj ) == 0 &&
				ir.ll2xy( minLon, maxLat, &di, &dj ) == 0 &&
				ir.ll2xy( maxLon, minLat, &di, &dj ) == 0 &&
				ir.ll2xy( maxLon, maxLat, &di, &dj ) == 0 ){
			/* позиционируемся на следующий блок */
			for( j = 0; j < pointCount; j++ ) p += 8;
			/* и идем на начало цикла */
			continue;
		}

		/* получаем координаты первой точки и выставляем флаги */
		memcpy( &cur, p, 8 );
		lat = (double)cur.lat;
		lon = (double)cur.lon;
		/* если точка попала в область */
		if( ir.ll2xy( lon, lat, &di, &dj ) ){
			/* вычисляем ее положение в качестве начальной точки */
			modf( di + 0.5, &di );
			modf( dj + 0.5, &dj );
			modf( di * cx, &dx );
			xOld = (unsigned short)dx;
			modf( dj * cy, &dy );
			yOld = (unsigned short)dy;
			if( isAscend ){
				xOld = nxRes - xOld;
				yOld = nyRes - yOld;
			}
			startOff = 0;
			/* записываем информацию в выходной файл */
			fprintf( netFile, "# C\n" );
			fprintf( netFile, "%d %d\n", xOld, yOld );
		}else startOff = 1;

		/* позиционируемся на вторую точку блока */
		p += 8;
		/* отрисовка блока */
		for( j = 1; j < pointCount; j++ ){
			/* получаем очередные координаты */
			memcpy( &cur, p, 8 );
			lat = (double)(cur.lat);
			lon = (double)(cur.lon);
			/* если очередная точка блока попала в область */
			if( ir.ll2xy( lon, lat, &di, &dj ) ){
				/* и предыдущая точка тоже попала */
				if( startOff == 0 ){
					/* вычисляем положение очередной точки
						в качестве конечной */
					modf( di + 0.5, &di );
					modf( dj + 0.5, &dj );
					modf( di * cx, &dx );
					x = (unsigned short)dx;
					modf( dj * cy, &dy );
					y = (unsigned short)dy;
					if( isAscend ){
						x = nxRes - x;
						y = nyRes - y;
					}
					/* выводим вектор */
					if( xOld != x || yOld != y ){
						//					if ( x_old != x && y_old != y ) {
						fprintf( netFile, "%d %d\n", x, y );
						xOld = x;
						yOld = y;
					}
				}else{ /* предыдущая не попала */
					/* вычисляем положение очередной точки
						в качестве начальной */
					modf( di + 0.5, &di );
					modf( dj + 0.5, &dj );
					modf( di * cx, &dx );
					xOld = (unsigned short)dx;
					modf( dj * cy, &dy );
					yOld = (unsigned short)dy;
					if( isAscend ){
						xOld = nxRes - xOld;
						yOld = nyRes - yOld;
					}
					startOff = 0;
					/* записываем информацию в выходной файл */
					fprintf( netFile, "# C\n" );
					fprintf( netFile, "%d %d\n", xOld, yOld );
				}
			}else{ /* очередная точка не попала в область */
				/* а предыдущая попала; меняем флаг */
				if( startOff == 0 ) startOff = 1;
			}

			/* позиционируемся на следующую точку блока */
			p += 8;
		}
	}
	free( buf );

	/* завершающие операции */
	fclose( netFile );
	if(isVerbose) printf("%s\n", netFileName);
}
