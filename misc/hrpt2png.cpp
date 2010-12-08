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

#include <c_lib.hpp>
#include <Log.hpp>
#include <hrpt.hpp>
#include <y_util.hpp>
#include <orbmodel.hpp>

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
void img_4c( void );
//void apply_net( void );
void writeNet( ) throw (TException);

/*
 * Log
 */
#define DEBUG
#ifdef DEBUG
#define _LOG_FORMAT_ "%d %t %e %m"
//nsCLog::eSeverity logLevel = nsCLog::info;
CLog logger(__FILE__);
#endif

/* файловые переменные */
FILE* inpHandle = NULL;
FILE* outHandle = NULL;

char* inpFile = NULL;
char* outDir = NULL;
char* coastFileName = NULL;

char netFileName[MAX_PATH];
char pngFileName[MAX_PATH];
//
char drive[MAX_DRIVE];
char dir[MAX_DIR];
char outName[MAX_FNAME];
char pngExt[MAX_EXT] = ".png";
char netExt[MAX_EXT] = ".net";

int chNum = -1; /* номер канала для распаковки */

THRPTUnpacker* unp = NULL;

/* переменные орбитальной модели */
TIniSatParams* isp = NULL;
TNOAAImageParams* nip = NULL;
TCorrectionParams* cop = NULL;
// прямая и обратная задача
TStraightReferencer* srPtr = NULL;
TInverseReferencer* irPtr = NULL;
int isAscend;
//
int totalFrameNum = 0;
int imageWidth = 0;
int imageHeight = 0;
int imageSize = 0;

uint16_t* scan = NULL;
uint16_t* avhrrBuf[] = { (uint16_t*)NULL, (uint16_t*)NULL, (uint16_t*)NULL, (uint16_t*)NULL, (uint16_t*)NULL };

png_bytep image = png_bytep_NULL;
png_bytepp imageRows = png_bytepp_NULL;

///* переменные для работы функций времени */
//static short months[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273,
//304, 334, 365 };
//static short leapMonths[13] = { 0, 31, 60, 91, 121, 152, 182, 213, 244,
//274, 305, 335, 366 };
//short h_time[6];
//double fraction;
//double entire;
//short n_day;
//short n_date[2];
//unsigned short nYear;

/* переменные орбитальной модели */
//TIniSatParams isp;
//TNOAAImageParams nip;
//TCorrectionParams cop = { 0.0, 0.0, 0.0, 0.0 };

/* географические координаты изображения */
double rightDownLon;
double rightDownLat;
double midDownLon;
double midDownLat;
double leftDownLon;
double leftDownLat;
double rightUpLon;
double rightUpLat;
double midUpLon;
double midUpLat;
double leftUpLon;
double leftUpLat;

void printUsage( ){
	fprintf( stderr, "Syntax: hrpt2png [-dir <output dir>] [-chan {1|2|3|4|5}] [-net <coast_file_name>] <hrpt input file> \n" );
	fprintf( stderr, "<hrpt input file>\n" );
	fprintf( stderr, "\tinput HRPT file of f34 format;\n" );
	fprintf( stderr, "channel num\n" );
	fprintf( stderr, "\tinput integer number in the range 1-5, that defines\n" );
	fprintf( stderr, "\tchannel to unpack from input file and to build grayscale PCX from it.\n" );
	fprintf( stderr, "\tThis parameter is necessary only for the normal HRPT file, i.e.\n" );
	fprintf( stderr, "\tfor 5-channel NOAA HRPT file, received on one of three hrpt\n" );
	fprintf( stderr, "\treceiving stations; if this parameter is not defined, program\n" );
	fprintf( stderr, "\tby default will unpack the first available channel from file.\n" );
	fprintf( stderr, "\tFor the artificial HRPT file, e.g. 1-channel HRPT file, that\n" );
	fprintf( stderr, "\twas built from 10-channel FENGYUN CHRPT file, this parameter\n" );
	fprintf( stderr, "\tis NOT necessary and ignored if defined.\n" );
	fprintf( stderr, "\tThe name of output grayscale PNG file is generated automatically\n" );
	fprintf( stderr, "\tfrom the name of input file and from channel num.\n" );
	fprintf( stderr, "coast_file_name\n" );
	fprintf( stderr, "\toptional parameter, that defines the full path and name\n" );
	fprintf( stderr, "\tof the binary coast file, e.g. C:\\COAST.DAT. If this parameter defined,\n" );
	fprintf( stderr, "\tprogram will apply the geographical net and coast contour to the\n" );
	fprintf( stderr, "\tresulting grayscale image.\n" );
}

int main( int argc, char* argv[] ){

	if( argc < 2 ){
		printUsage( );
		return 1;
	}
#ifdef DEBUG
	//	TCharString msg;
	logger.addAppender(new EConsoleLogAppender(nsCLog::dump, _LOG_FORMAT_));
#endif

	try{
		parseCommandString( argc, argv );

		try{
			unp = new THRPTUnpacker( inpFile, 0 );
			TBlk0& blk0 = *(unp->blk0());
			isp = new TIniSatParams( blk0 );
			nip = new TNOAAImageParams( blk0 );
			cop = new TCorrectionParams( blk0 );
			srPtr = new TStraightReferencer( *isp, *nip, *cop );
			irPtr = new TInverseReferencer( *isp, *nip, *cop );
		}catch( TException e ){
			throw TException( 2, "ошибка доступа к входному файлу данных" );
		}
		TBlk0_Common& blk0c = unp->blk0()->b0;

		if( blk0c.formatType != L34_FORMAT )
			throw TException( 3, "неправильный формат входного файла данных" );

		// В данной версии обрабатываем исходные NOAA HRPT
		if( blk0c.dataType1 != B0DT1_SRC && blk0c.dataType2 != B0DT2_HRPT_NOAA )
			throw TException( 3, "неподдерживаемый формат данных" );

		TBlk0_HRPT& hrb0 = *((TBlk0_HRPT*)unp->blk0( ));

		isAscend = hrb0.ascendFlag;
		totalFrameNum = hrb0.frameNum + hrb0.lostFrameNum;
		imageWidth = hrb0.totalPixNum / 2;
		imageHeight = totalFrameNum / 2 + (totalFrameNum & 1);
		imageSize = imageWidth * imageHeight;

		scan = new ushort[hrb0.totalPixNum];
//		scan = new unsigned short[2048];

		//Если не указаны каналы или указанный канал недоступен, то берём первый попавшийся.
		if( chNum < 0 || chNum > 4 || !(unp->channelAvailable( chNum )) ){
			for(int i = 0; i < 5; i++ ){
				chNum = i;
				if (unp->channelAvailable( i )) break;
			}
			if( chNum < 0 || chNum > 4 || !unp->channelAvailable( chNum ) ) throw TException( 5, "Fatal error! There are no accessible channels!" );
		}

		avhrrBuf[chNum] = scan;
#ifdef DEBUG
		logger.dump("avhrrBuf", (char*)avhrrBuf, sizeof(unsigned short*)*5);
		logger.dump("scan",(char*)scan, sizeof(unsigned short)*16);
#endif

		/* читаем время приема */
		int month;
		int date;
		/* дата начала приема */
		dayToDate( blk0c.year, blk0c.day, &month, &date );
		/* вычисляем часы, минуты, секунды */
		long ts = blk0c.dayTime / 1000;
		uint16_t sec = ts % 60;
		ts = ts / 60;
		uint16_t minute = ts % 60;
		uint16_t hour = ts / 60;

		sprintf( outName, "%s_%4d%02d%02d_%02d%02d%02d", blk0c.satName, blk0c.year, month, date, hour, minute, sec );

		splitpath( inpFile, drive, dir, 0, 0 );

		if( outDir != NULL )
			makepath( pngFileName, 0, outDir, outName, pngExt );
		else
			makepath( pngFileName, drive, dir, outName, pngExt );

		if( coastFileName != NULL ){
			if( outDir != NULL )
				makepath( netFileName, 0, outDir, outName, netExt );
			else
				makepath( netFileName, drive, dir, outName, netExt );
		}

		int hist[256];
		int sumHist[256];
		int lookup[256];
		memset(hist,0,sizeof(hist));
		memset(sumHist,0,sizeof(hist));
		memset(lookup,0,sizeof(hist));
#ifdef DEBUG
		logger.dump("hist", (char*)hist, sizeof(int)*256);
		logger.dump("sumHist",(char*)sumHist, sizeof(int)*256);
		logger.dump("lookup",(char*)lookup, sizeof(int)*256);
#endif

		int i,j,k;
		int lowLevel = 1;
		int highLevel = 85;
		int autoLow = 0;
		int autoHigh = 0;
		uint16_t chMin = 1023;
		uint16_t chMax = 0;
		int minIdx = 255;
		int maxIdx = 0;
		int fIdx;
		int bIdx;

		image = new png_byte[imageSize];
		png_bytep imagePtr = image;
		if( isAscend ) imagePtr += imageSize - 1;
//		imageRows = new png_bytep[imageHeight];
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
		logger.dump("hist", (char*)hist, sizeof(int)*256);
		logger.dump("sumHist", (char*)sumHist, sizeof(int)*256);
		logger.dump("lookup", (char*)lookup, sizeof(int)*256);
#endif
		/* транслируем изображение по таблице */
		imagePtr = image;
		while( imagePtr < image + imageSize ){
			i = (int)(*imagePtr) & 0xff;
			*imagePtr++ = (unsigned char)lookup[i];
		}


		outHandle = fopen( pngFileName, "wb+" );
		if( !outHandle ) throw TException( 5, "[write_png_file] Error to open output file" );

		/* initialize stuff */
		png_structp pngPtr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

		if( !pngPtr )
			throw TException( 5, "[write_png_file] png_create_write_struct failed" );

		png_infop infoPtr = png_create_info_struct( pngPtr );
		if( !infoPtr )
			throw TException( 5, "[write_png_file] png_create_info_struct failed" );

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
		outHandle = NULL;

		/* если задано наложение сетки и береговой линии */
		if( netFileName ){
#ifdef DEBUG
			logger.debug("has net name");
			logger.debug(netFileName);
#endif
			img_4c( );
			writeNet( );
		}
	}catch(TException e){
		fprintf( stderr, "%s\n", e.text( ) );
		closeAll( );
		return( e.id( ) );
	}

	closeAll( );
	return 0;

	//#ifdef DEBUG
	//	//	log = new CLog();
	//	TCharString msg;
	//	logger.addAppender(new EConsoleLogAppender(nsCLog::dump, LOG_FORMAT));
	//#endif
	//
	//	if( argc < 2 ){
	//		printUsage( );
	//		return 1;
	//	}
	//
	//	try{
	//
	//		parseCommandString( argc, argv );
	//
	//		try{
	//			unp = new THRPTUnpacker( inpFileName, 0 );
	//			TBlk0& blk0 =  *(unp->blk0( ));
	//			isp = new TIniSatParams( blk0 );
	//			nip = new TNOAAImageParams( blk0 );
	//			cop = new TCorrectionParams( blk0 );
	//			srPtr = new TStraightReferencer( *isp, *nip, *cop );
	//			irPtr = new TInverseReferencer( *isp, *nip, *cop );
	//		}catch( TException e ){
	//			throw TException( 2, "ошибка доступа к входному файлу данных" );
	//		}
	//
	//		TBlk0_Common& blk0c = unp->blk0()->b0;
	//
	//		if( blk0c.formatType != L34_FORMAT )
	//			throw TException( 3, "неправильный формат входного файла данных" );
	//
	//		// В данной версии обрабатываем исходные NOAA HRPT
	//		if( blk0c.dataType1 != B0DT1_SRC && blk0c.dataType2 != B0DT2_HRPT_NOAA )
	//			throw TException( 3, "неподдерживаемый формат данных" );
	//
	//		/* читаем время приема */
	//		int month;
	//		int date;
	//		/* дата начала приема */
	//		dayToDate( blk0c.year, blk0c.day, &month, &date );
	//		/* вычисляем часы, минуты, секунды */
	//		long ts = blk0c.dayTime / 1000;
	//		ushort sec = ts % 60;
	//		ts = ts / 60;
	//		ushort minute = ts % 60;
	//		ushort hour = ts / 60;
	//
	//		sprintf( outName, "%s_%4d%02d%02d_%02d%02d%02d", blk0c.satName, blk0c.year, month, date, hour, minute, sec );
	//
	//		splitpath( inpFileName, drive, dir, 0, 0 );
	//
	//		if( outDir != NULL )
	//			makepath( pngFileName, 0, outDir, outName, pngExt );
	//		else
	//			makepath( pngFileName, drive, dir, outName, pngExt );
	//
	//		if( coastFileName != NULL ){
	//			if( outDir != NULL )
	//				makepath( netFileName, 0, outDir, outName, netExt );
	//			else
	//				makepath( netFileName, drive, dir, outName, netExt );
	//		}
	//
	//		int i, j, k;
	//
	//		TBlk0_HRPT& hrb0 = *((TBlk0_HRPT*)unp->blk0( ));
	//
	//		scan = new ushort[hrb0.totalPixNum];
	//
	//		//Если не указаны каналы или указанный канал недоступен, то берём первый попавшийся.
	//		if( chNum < 0 || chNum > 4 || !(unp->channelAvailable( chNum )) ){
	//			for( i = 0; i < 5; i++ ){
	//				chNum = i;
	//				if (unp->channelAvailable( i )) break;
	//			}
	//			if( chNum < 0 || chNum > 4 || !unp->channelAvailable( chNum ) ) throw TException( 5, "Fatal error! There are no accessible channels!" );
	//		}
	//
	//		avhrrBuf[chNum] = scan;
	//#ifdef DEBUG
	////		msg.assign("chNum:").append(chNum);
	//		logger.debug(msg.assign("chNum:").append(chNum));
	//#endif
	//
	//		isAscend = hrb0.ascendFlag;
	//		totalFrameNum = hrb0.frameNum + hrb0.lostFrameNum;
	//		imageWidth = hrb0.totalPixNum / 2;
	//		imageHeight = totalFrameNum / 2 + (totalFrameNum & 1);
	//		imageSize = imageWidth * imageHeight;
	//
	//		/* переменные для расчета гистограмм и порогов */
	//		int* hist = new int[256];
	//		int* sumHist = new int[256];
	//		int* lookup = new int[256];
	////		memset(hist,0,sizeof(hist));
	////		memset(sumHist,0,sizeof(hist));
	////		memset(lookup,0,sizeof(hist));
	//#ifdef DEBUG
	//		logger.dump("hist", (char*)hist, sizeof(int)*256);
	//		logger.dump("sumHist",(char*)sumHist, sizeof(int)*256);
	//		logger.dump("lookup",(char*)lookup, sizeof(int)*256);
	//#endif
	//
	//		int lowLevel = 1;
	//		int highLevel = 85;
	//		int autoLow = 0;
	//		int autoHigh = 0;
	//		ushort chMin = 1023;
	//		ushort chMax = 0;
	//		int minIdx = 255;
	//		int maxIdx = 0;
	//		int fIdx;
	//		int bIdx;
	//
	//		image = new png_byte[imageSize];
	//		png_bytep imagePtr = image;
	//		if( isAscend ) imagePtr += imageSize - 1;
	//		imageRows = new png_bytep[imageHeight];
	//		for( i = 0; i < imageHeight; i++ ) imageRows[i] = image + i * imageWidth;
	//
	//		for( i = 0; i < totalFrameNum; i += 2 ){
	//			unp->setCurrentFrameNumber( i );
	//			try{
	//				unp->unpackNextFrame( avhrrBuf );
	//			}catch( TException e ){
	//				if( e.id( ) != 2 ) throw e;
	//				imagePtr += isAscend ? -imageWidth : imageWidth;
	//				continue;
	//			}
	//			ushort* scanPtr = scan;
	//
	//			for( j = 0; j < imageWidth; j++ ){
	//				png_byte p = (png_byte)((*scanPtr) >> 2);
	//				*imagePtr = p;
	//				/* гистограммируем изображение */
	//				hist[(int)(*imagePtr)]++;
	//				/* находим диапазон канала */
	//				if( (*scanPtr) > chMax ) chMax = *scanPtr;
	//				if( (*scanPtr) < chMin ) chMin = *scanPtr;
	//				/* следующая точка ( zoom = 2 ) */
	//				scanPtr += 2;
	//				/* учитываем, какой виток, восходящий или нет */
	//				imagePtr += isAscend ? -1 : 1;
	//				//imagePtr++;
	//			}
	//
	//		}
	//
	//		/* подсчитываем интеграл от гистограммы */
	//		j = 0;
	//		hist[0] = 0; /* при вычислении порогов индекс цвета 0 не учитывается */
	//		for( i = 0; i < 256; i++ ){
	//			j += hist[i];
	//			sumHist[i] = j;
	//		}
	//		/* вычисляем пороги */
	//		j = (int)((double)sumHist[255] * (double)lowLevel / 100.0);
	//		i = 0;
	//		while( sumHist[i] < j ) i++;
	//		autoLow = i;
	//		j = (int)((double)sumHist[255] * (double)highLevel / 100.0);
	//		i = 255;
	//		while( sumHist[i] >= j ) i--;
	//		autoHigh = i;
	//
	//		/* делаем таблицу lookup по форме интеграла от гистограммы
	//				в диапазоне от auto_low до auto_high */
	//		for( i = 0; i < autoLow; i++ ) lookup[i] = 0;
	//		for( i = autoHigh; i < 256; i++ ) lookup[i] = 0xff;
	//		/* нормировочный коэффициент */
	//		k = (sumHist[autoHigh] - sumHist[autoLow]) / 255;
	//		for( i = autoLow; i < autoHigh; i++ ) lookup[i] = (sumHist[i] - sumHist[autoLow]) / k;
	//
	//#ifdef DEBUG
	//			logger.dump("hist", (char*)hist, 256);
	//			logger.dump("sumHist", (char*)sumHist, 256);
	//			logger.dump("lookup", (char*)lookup, 256);
	//#endif
	//		/* транслируем изображение по таблице */
	//		imagePtr = image;
	//		while( imagePtr < image + imageSize ){
	//			i = (int)(*imagePtr) & 0xff;
	//			*imagePtr++ = (unsigned char)lookup[i];
	//		}
	//
	//
	//		outHandle = fopen( pngFileName, "wb+" );
	//		if( !outHandle ) throw TException( 5, "[write_png_file] Error to open output file" );
	//
	//		/* initialize stuff */
	//		png_structp pngPtr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	//
	//		if( !pngPtr )
	//			throw TException( 5, "[write_png_file] png_create_write_struct failed" );
	//
	//		png_infop infoPtr = png_create_info_struct( pngPtr );
	//		if( !infoPtr )
	//			throw TException( 5, "[write_png_file] png_create_info_struct failed" );
	//
	//		png_init_io( pngPtr, outHandle );
	//
	//		/* write header */
	//		//	if (setjmp(png_jmpbuf(png_ptr)))
	//		//		abort_("[write_png_file] Error during writing header");
	//
	//		png_set_IHDR( pngPtr, infoPtr, imageWidth, imageHeight,
	//				8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
	//				PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
	//
	//#ifdef PNG_TEXT_SUPPORTED
	//		/* Optionally write comments into the image */
	//		png_text textPtr[3];
	//		textPtr[0].key = (char*)"provider";
	//		textPtr[0].text = (char*)"SML IACP RAS";
	//		textPtr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	//		textPtr[1].key = (char*)"collection";
	//		textPtr[1].text = (char*)"SML.NOAA_HRPT.LONGTIME";
	//		textPtr[1].compression = PNG_TEXT_COMPRESSION_NONE;
	//		textPtr[2].key = (char*)"id";
	//		textPtr[2].text = outName;
	//		textPtr[2].compression = PNG_TEXT_COMPRESSION_NONE;
	//		png_set_text( pngPtr, infoPtr, textPtr, 3 );
	//#endif
	//
	//		png_write_info( pngPtr, infoPtr );
	//
	//		/* write bytes */
	//		//	if (setjmp(png_jmpbuf(png_ptr)))
	//		//		abort_("[write_png_file] Error during writing bytes");
	//
	//		png_write_image( pngPtr, imageRows );
	//
	//
	//		/* end write */
	//		//	if (setjmp(png_jmpbuf(png_ptr)))
	//		//		abort_("[write_png_file] Error during end of write");
	//
	//		png_write_end( pngPtr, NULL );
	//
	//		fclose( outHandle );
	//		outHandle = NULL;
	//
	//		/* если задано наложение сетки и береговой линии */
	//		if( netFileName ){
	//			img_4c( );
	//			write_net( );
	//		}
	//
	//	}catch( TException e ){
	//		fprintf( stderr, "%s\n", e.text( ) );
	//		closeAll( );
	//		exit( e.id( ) );
	//	}
	//
	//	/* заключительные операции */
	//	closeAll( );
}

// hrpt2png [-dir <output dir>] [-chan {1|2|3|4|5}] [-net <coast_file_name>] <hrpt_input_file>

void parseCommandString( int argc, char* argv[] ) throw (TException){
	TCharString msg;
	int i = 1;
	while( i < argc ){
		char* s = argv[i++];
		if( *s == '-' ){
			s++;
			if( strcmp( s, "chan" ) == 0 ){
				if( i == argc || strlen( argv[i] ) == 0 )
					throw TException( 1, msg.assign( "Invalid param -chan" ) );
				s = argv[i++];
				if( isdigit( *s ) ){ // опция "-chan {1|2|3|4|5}" */
					int t = atoi( s );
					if( t < 1 || t > 5 ) throw TException( 1, msg.assign( "Invalid channel number: " ).append( s ) );
					chNum = t - 1;
				}else throw TException( 1, msg.assign( "Invalid param -chan " ).append( s ) );
			}else if( strcmp( s, "dir" ) == 0 ){
				if( i == argc || strlen( argv[i] ) == 0 )
					throw TException( 1, msg.assign( "Invalid param -dir" ) );
				outDir = argv[i++];
			}else if( strcmp( s, "net" ) == 0 ){
				if( i == argc || strlen( argv[i] ) == 0 )
					throw TException( 1, msg.assign( "Invalid param -net" ) );
				coastFileName = argv[i++];
			}else throw TException( 1, msg.assign( "Invalid param -" ).append( s ) );
		}else inpFile = s;
		/*
		} else if( i == argc ){
			inpFile = s;
		} else {
			throw TException(1, msg.assign("The input file is not specified"));
		}
		 */
	}
	if( inpFile == NULL )
		throw TException( 1, msg.assign( "The input file is not specified" ) );
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
	if( inpHandle ) fclose( inpHandle );
	if( outHandle )	fclose( outHandle );
}

/*---------------------------------------------------------------------
	IMG_4C.C
	Данная функция воспроизводит работу старой программы DOS img_4c.exe,
	т.е. вычисляет с помощью орбитальной модели координаты четырех
	углов изображения и две точки входа и выхода. Вся работа с орбитальной
	моделью локализована здесь. Кроме этого, функция преобразует
	полученные координаты в целочисленный вид и заполняет соответствующие
	строки информационной панели grayscale pcx.
---------------------------------------------------------------------*/
void img_4c( ){
	double dx;

	//	/* инициализируем основные структуры sgp8 */
	//	TIniSatParams isp( *unp->blk0() );
	//	TNOAAImageParams nip( *unp->blk0() );
	/* инициализация прямой задачи */
	//iniIJ2LL( &isp, &nip, &cop );

	TStraightReferencer& sr = *srPtr;
	/* вычисляем и записываем координаты углов и подспутниковой
	трассы ( считаем, что виток восходящий ) */
	//	IJ2LL( 0.0, 0.0, &right_down_lon, &right_down_lat );
	sr.xy2ll( 0, 0, &rightDownLon, &rightDownLat );
	//	IJ2LL( 1024.0, 0.0, &mid_down_lon, &mid_down_lat );
	sr.xy2ll( 1024, 0, &midDownLon, &midDownLat );
	//	IJ2LL( 2047.0, 0.0, &left_down_lon, &left_down_lat );
	sr.xy2ll( 2047, 0, &leftDownLon, &leftDownLat );
	//	IJ2LL( 0.0, (double)(frame_c - 1), &right_up_lon, &right_up_lat );
	sr.xy2ll( 0, unp->totalFrames( ) - 1, &rightUpLon, &rightUpLat );
	//	IJ2LL( 1024.0, (double)(frame_c - 1), &mid_up_lon, &mid_up_lat );
	sr.xy2ll( 1024, unp->totalFrames( ) - 1, &midUpLon, &midUpLat );
	//	IJ2LL( 2047.0, (double)(frame_c - 1), &left_up_lon, &left_up_lat );
	sr.xy2ll( 2047, unp->totalFrames( ) - 1, &leftUpLon, &leftUpLat );
	/* если виток нисходящий, меняем местами углы крест-накрест */
	//	if( mid_up_lat < mid_down_lat ){
	if( isAscend ){
		dx = leftUpLat;
		leftUpLat = rightDownLat;
		rightDownLat = dx;
		dx = leftUpLon;
		leftUpLon = rightDownLon;
		rightDownLon = dx;
		dx = rightUpLat;
		rightUpLat = leftDownLat;
		leftDownLat = dx;
		dx = rightUpLon;
		rightUpLon = leftDownLon;
		leftDownLon = dx;
	}
}

/*---------------------------------------------------------------------
	APPLY_NET.C
	Функция накладывает на изображение сетку и береговую линию.
---------------------------------------------------------------------*/
//void apply_net( ){
//	int i;
//	int j;
//	//	unsigned long l = 0;
//	unsigned short nxRes = 0;
//	unsigned short nyRes = 0;
//	unsigned short nColor = 0x180;
//	unsigned short x, y, xOld, yOld;
//	unsigned short extremFlag;
//
//	double step5 = 5.0;
//	double step1 = 1.0;
//	double step5dr;
//	double step1dr;
//	double step;
//	double tLon;
//	double lon;
//	double lat;
//	double minLon;
//	double minLat;
//	double maxLon;
//	double maxLat;
//	double cx;
//	double cy;
//	double di;
//	double dx;
//	double dj;
//	double dy;
//	double curDi, curDj;
//	double saveL;
//
//	TStraightReferencer& sr = *srPtr;
//	TInverseReferencer& ir = *irPtr;
//
//
//	/* переменные для чтения бинарного файла */
//	struct t_ciahdr blk;
//	struct t_ll cur;
//	FILE* coastFile = NULL;
//	int coastFileLen;
//	unsigned char* buf = NULL;
//	unsigned char* p = NULL;
//	unsigned short blk_type = 0;
//	unsigned short line_c = 0;
//	unsigned short start_off = 0;
//
//	/* находим минимум и максимум широты и долготы */
//	extremFlag = 0;
//	if( isAscend ){ /* восходящий виток */
//		/* начальные установки */
//		minLon = leftUpLon;
//		if( minLon < 0.0 ) minLon += 2.0 * PI;
//		maxLon = rightDownLon;
//		if( maxLon < 0.0 ) maxLon += 2.0 * PI;
//		minLat = leftDownLat;
//		maxLat = rightUpLat;
//		for( i = 1; i < LAVHRR / 2; i++ ){
//			//			IJ2LL( (double)i, (double)(frame_c - 1), &lon, &lat );
//			sr.xy2ll( i, totalFrameNum - 1, &lon, &lat );
//			/* непрерывный переход через 180 градусов долготы */
//			if( lon < 0.0 ) lon += 2.0 * PI;
//			/* находим максимум долготы и широты */
//			if( lon > maxLon ) maxLon = lon;
//			else extremFlag |= 1;
//			if( lat > maxLat ) maxLat = lat;
//			else extremFlag |= 2;
//			if( extremFlag == 3 ) break;
//		}
//	}else{ /* нисходящий виток */
//		/* начальные установки */
//		minLon = leftDownLon;
//		if( minLon < 0.0 ) minLon += 2.0 * PI;
//		maxLon = rightUpLon;
//		if( maxLon < 0.0 ) maxLon += 2.0 * PI;
//		minLat = rightDownLat;
//		maxLat = leftUpLat;
//		for( i = 1; i < LAVHRR / 2; i++ ){
//			//			IJ2LL( (double)i, (double)0, &lon, &lat );
//			sr.xy2ll( i, 0, &lon, &lat );
//			/* непрерывный переход через 180 градусов долготы */
//			if( lon < 0.0 ) lon += 2.0 * PI;
//			/* находим минимум долготы и максимум широты */
//			if( lon < minLon ) minLon = lon;
//			else extremFlag |= 1;
//			if( lat > maxLat ) maxLat = lat;
//			else extremFlag |= 2;
//			if( extremFlag == 3 ) break;
//		}
//	}
//	/* минимум и максимум устанавливается как число градусов,
//	больше максимума или меньше минимума и кратное step_5 градусов */
//	step5dr = step5 * DR;
//	modf( minLat / step5dr, &minLat );
//	minLat *= step5dr;
//	modf( minLon / step5dr, &minLon );
//	minLon *= step5dr;
//	modf( maxLat / step5dr, &maxLat );
//	maxLat = (maxLat + 1.0) * step5dr;
//	modf( maxLon / step5dr, &maxLon );
//	maxLon = (maxLon + 1.0) * step5dr;
//
//	/* вычисляем коэффициенты для перевода i, j
//	в номера экранных пикселов */
//	cx = (double)(imageWidth - 1) / (double)(LAVHRR - 1);
//	cy = (double)((totalFrameNum - 1) / 2) / (double)(totalFrameNum - 1);
//
//	/* размер экранной сетки - 1 */
//	modf( (double)(LAVHRR - 1) * cx, &dx );
//	nxRes = (unsigned short)dx;
//	modf( (double)(totalFrameNum - 1) * cy, &dy );
//	nyRes = (unsigned short)dy;
//
//	/* инициализация обратной задачи */
//	//	i = iniLL2IJ( &isp, &nip, &cop );
//	//	if( i == -1 ){
//	//		printf( "Error to initialize LL2IJ\n" );
//	//		return;
//	//	}
//
//	/* отрисовываем сетку долготы и широты */
//	step1dr = step1 * DR;
//	for( lon = minLon + step5dr; lon < maxLon - step5dr / 2.; lon += step5dr ){
//		tLon = lon;
//		if( lon >= PI ) tLon -= 2.0 * PI;
//		lat = minLat;
//		m3:
//		;
//		/* ищем попадание кривой в область */
//		//		while( LL2IJ( tLon, lat, &di, &dj ) == 0 ){
//		while( ir.ll2xy( tLon, lat, &di, &dj ) == 0 ){
//			if( lat < maxLat + step1dr / 2. ) lat += step1dr;
//			else goto m2; /* кривая не пересекает область */
//		}
//		/* первое попадание кривой в область */
//		modf( di + 0.5, &di );
//		modf( di * cx, &dx );
//		x = (unsigned short)dx;
//		modf( dj + 0.5, &dj );
//		modf( dj * cy, &dy );
//		y = (unsigned short)dy;
//		if( isAscend ){
//			x = nxRes - x;
//			y = nyRes - y;
//		}
//		/* производим аппроксимацию назад для получения точки
//		пересечения с границей */
//		saveL = lat;
//		curDi = di;
//		curDj = dj;
//		step = step1dr / 2.0;
//		/* и вычисляем пересечение методом дихотомии */
//		while( 1 ){
//			//			if( LL2IJ( tLon, lat - step, &di, &dj ) ){
//			if( ir.ll2xy( tLon, lat - step, &di, &dj ) ){
//				modf( di + 0.5, &di );
//				modf( dj + 0.5, &dj );
//				lat -= step;
//				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
//				else{
//					curDi = di;
//					curDj = dj;
//				}
//			}
//			step /= 2.0;
//		}
//		/* точка пересечения в качестве начальной точки */
//		modf( di * cx, &dx );
//		xOld = (unsigned short)dx;
//		modf( dj * cy, &dy );
//		yOld = (unsigned short)dy;
//		if( isAscend ){
//			xOld = nxRes - xOld;
//			yOld = nyRes - yOld;
//		}
//		/* выводим недостающий вектор */
//		if( xOld != x || yOld != y ){
//			vvector8( (int)nColor, (int)xOld, (int)yOld, (int)x, (int)y, image, imageWidth );
//			xOld = x;
//			yOld = y;
//		}
//		/* восстанавливаем широту */
//		lat = saveL;
//		if( lat < maxLat + step1dr / 2. ) lat += step1dr;
//
//		/* ищем выпадание кривой из области */
//		while( ir.ll2xy( tLon, lat, &di, &dj ) ){
//			modf( di + 0.5, &di );
//			modf( di * cx, &dx );
//			x = (unsigned short)dx;
//			modf( dj + 0.5, &dj );
//			modf( dj * cy, &dy );
//			y = (unsigned short)dy;
//			if( isAscend ){
//				x = nxRes - x;
//				y = nyRes - y;
//			}
//			/* выводим очередной вектор */
//			vvector8( (int)nColor, (int)xOld, (int)yOld, (int)x, (int)y, image, imageWidth );
//			xOld = x;
//			yOld = y;
//			if( lat < maxLat + step1dr / 2. ) lat += step1dr;
//		}
//
//		/* производим аппроксимацию вперед для получения точки
//		пересечения с границей */
//		saveL = lat;
//		step = step1dr / 2.0;
//		if( lat > minLat + step ) lat -= step1dr;
//		curDi = di;
//		curDj = dj;
//		/* и вычисляем пересечение методом дихотомии */
//		while( 1 ){
//			if( ir.ll2xy( tLon, lat + step, &di, &dj ) ){
//				modf( di + 0.5, &di );
//				modf( dj + 0.5, &dj );
//				lat += step;
//				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
//				else{
//					curDi = di;
//					curDj = dj;
//				}
//			}
//			step /= 2.0;
//		}
//		/* точка пересечения в качестве конечной точки */
//		modf( di * cx, &dx );
//		x = (unsigned short)dx;
//		modf( dj * cy, &dy );
//		y = (unsigned short)dy;
//		if( isAscend ){
//			x = nxRes - x;
//			y = nyRes - y;
//		}
//		/* выводим недостающий вектор */
//		if( xOld != x || yOld != y ){
//			vvector8( (int)nColor, (int)xOld, (int)yOld, (int)x, (int)y, image, imageWidth );
//			xOld = x;
//			yOld = y;
//		}
//		/* восстанавливаем широту */
//		lat = saveL;
//
//		/* на всякий случай ищем второе попадание в область */
//		goto m3;
//		m2:
//		;
//	}
//
//	for( lat = minLat + step5dr; lat < maxLat - step5dr / 2.; lat += step5dr ){
//		/* ищем попадание кривой в область */
//		lon = minLon;
//		m5:
//		;
//		while( 1 ){
//			tLon = lon;
//			if( tLon >= PI ) tLon -= 2.0 * PI;
//			if( ir.ll2xy( tLon, lat, &di, &dj ) ) break;
//			if( lon < maxLon + step1dr / 2. ) lon += step1dr;
//			else goto m4; /* кривая не пересекает область */
//		}
//		/* первое попадание кривой в область */
//		modf( di + 0.5, &di );
//		modf( di * cx, &dx );
//		x = (unsigned short)dx;
//		modf( dj + 0.5, &dj );
//		modf( dj * cy, &dy );
//		y = (unsigned short)dy;
//		if( isAscend ){
//			x = nxRes - x;
//			y = nyRes - y;
//		}
//		/* производим аппроксимацию назад для получения точки
//		пересечения с границей */
//		saveL = lon;
//		curDi = di;
//		curDj = dj;
//		step = step1dr / 2.0;
//		/* и вычисляем пересечение методом дихотомии */
//		while( 1 ){
//			tLon = lon - step;
//			if( tLon >= PI ) tLon -= 2.0 * PI;
//			if( ir.ll2xy( tLon, lat, &di, &dj ) ){
//				modf( di + 0.5, &di );
//				modf( dj + 0.5, &dj );
//				lon -= step;
//				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
//				else{
//					curDi = di;
//					curDj = dj;
//				}
//			}
//			step /= 2.0;
//		}
//		/* точка пересечения в качестве начальной точки */
//		modf( di * cx, &dx );
//		xOld = (unsigned short)dx;
//		modf( dj * cy, &dy );
//		yOld = (unsigned short)dy;
//		if( isAscend ){
//			xOld = nxRes - xOld;
//			yOld = nyRes - yOld;
//		}
//		/* выводим недостающий вектор */
//		if( xOld != x || yOld != y ){
//			vvector8( (int)nColor, (int)xOld, (int)yOld, (int)x, (int)y, image, imageWidth );
//			xOld = x;
//			yOld = y;
//		}
//		/* восстанавливаем долготу */
//		lon = saveL;
//		if( lon < maxLon + step1dr / 2. ) lon += step1dr;
//
//		/* ищем выпадание кривой из области */
//		while( 1 ){
//			tLon = lon;
//			if( tLon >= PI ) tLon -= 2.0 * PI;
//			if( ir.ll2xy( tLon, lat, &di, &dj ) == 0 ) break;
//			modf( di + 0.5, &di );
//			modf( di * cx, &dx );
//			x = (unsigned short)dx;
//			modf( dj + 0.5, &dj );
//			modf( dj * cy, &dy );
//			y = (unsigned short)dy;
//			if( isAscend ){
//				x = nxRes - x;
//				y = nyRes - y;
//			}
//			vvector8( (int)nColor, (int)xOld, (int)yOld, (int)x, (int)y, image, imageWidth );
//			xOld = x;
//			yOld = y;
//			if( lon < maxLon + step1dr / 2. ) lon += step1dr;
//		}
//
//		/* производим аппроксимацию вперед для получения точки
//		пересечения с границей */
//		saveL = lon;
//		step = step1dr / 2.0;
//		if( lon > minLon + step ) lon -= step1dr;
//		curDi = di;
//		curDj = dj;
//		/* и вычисляем пересечение методом дихотомии */
//		while( 1 ){
//			tLon = lon + step;
//			if( tLon >= PI ) tLon -= 2.0 * PI;
//			if( ir.ll2xy( tLon, lat, &di, &dj ) ){
//				modf( di + 0.5, &di );
//				modf( dj + 0.5, &dj );
//				lon += step;
//				if( fabs( curDi - di ) < 1.0 && fabs( curDj - dj ) < 1.0 ) break;
//				else{
//					curDi = di;
//					curDj = dj;
//				}
//			}
//			step /= 2.0;
//		}
//		/* точка пересечения в качестве конечной точки */
//		modf( di * cx, &dx );
//		x = (unsigned short)dx;
//		modf( dj * cy, &dy );
//		y = (unsigned short)dy;
//		if( isAscend ){
//			x = nxRes - x;
//			y = nyRes - y;
//		}
//		/* выводим недостающий вектор */
//		if( xOld != x || yOld != y ){
//			vvector8( (int)nColor, (int)xOld, (int)yOld, (int)x, (int)y, image, imageWidth );
//			xOld = x;
//			yOld = y;
//		}
//		/* восстанавливаем долготу */
//		lon = saveL;
//
//		/* на всякий случай ищем второе попадание в область */
//		goto m5;
//		m4:
//		;
//	}
//
//	/* построение береговой линии */
//	/* открываем входной файл */
//	coastFile = fopen( coastFileName, "rb" );
//	if( coastFile == NULL ){
//		printf( "Invalid coast file name: %s\n", coastFileName );
//		goto m6;
//	}
//	/* определяем длину файла */
//	fseek( coastFile, 0, SEEK_END );
//	coastFileLen = ftell( coastFile );
//	fseek( coastFile, 0, SEEK_SET );
//	/* выделяем буфер ввода/вывода */
//	buf = (unsigned char*)malloc( coastFileLen );
//	if( buf == NULL ) goto m6;
//	/* читаем весь файл в буфер */
//	fread( buf, 1, coastFileLen, coastFile );
//	fclose( coastFile );
//
//	p = buf;
//	while( 1 ){
//		/* здесь p всегда указывает на заголовок */
//		memcpy( &blk, p, 8 );
//
//		/* получаем из заголовка параметры блока */
//		blk_type = blk.type;
//		if( blk_type == 0 ) break;
//		line_c = blk.count;
//		if( line_c == 0 ) break;
//
//		/* позиционируемся на минимумы блока */
//		p += 8;
//		/* получаем минимумы блока */
//		memcpy( &cur, p, 8 );
//		minLat = (double)cur.lat;
//		minLon = (double)cur.lon;
//
//		/* позиционируемся на максимумы блока */
//		p += 8;
//		/* получаем максимумы блока */
//		memcpy( &cur, p, 8 );
//		maxLat = (double)cur.lat;
//		maxLon = (double)cur.lon;
//
//		/* позиционируемся на первую точку блока */
//		p += 8;
//		/* если ни одна из точек блока не попадает в приемную область */
//		if( ir.ll2xy( minLon, minLat, &di, &dj ) == 0 &&
//				ir.ll2xy( minLon, maxLat, &di, &dj ) == 0 &&
//				ir.ll2xy( maxLon, minLat, &di, &dj ) == 0 &&
//				ir.ll2xy( maxLon, maxLat, &di, &dj ) == 0 ){
//			/* позиционируемся на следующий блок */
//			for( j = 0; j < line_c; j++ ) p += 8;
//			/* и идем на начало цикла */
//			continue;
//		}
//
//		/* получаем координаты первой точки и выставляем флаги */
//		memcpy( &cur, p, 8 );
//		lat = (double)cur.lat;
//		lon = (double)cur.lon;
//		/* если точка попала в область */
//		if( ir.ll2xy( lon, lat, &di, &dj ) ){
//			/* вычисляем ее положение в качестве начальной точки */
//			modf( di + 0.5, &di );
//			modf( dj + 0.5, &dj );
//			modf( di * cx, &dx );
//			xOld = (unsigned short)dx;
//			modf( dj * cy, &dy );
//			yOld = (unsigned short)dy;
//			if( isAscend ){
//				xOld = nxRes - xOld;
//				yOld = nyRes - yOld;
//			}
//			start_off = 0;
//		}else start_off = 1;
//
//		/* позиционируемся на вторую точку блока */
//		p += 8;
//		/* отрисовка блока */
//		for( j = 1; j < line_c; j++ ){
//			/* получаем очередные координаты */
//			memcpy( &cur, p, 8 );
//			lat = (double)(cur.lat);
//			lon = (double)(cur.lon);
//			/* если очередная точка блока попала в область */
//			if( ir.ll2xy( lon, lat, &di, &dj ) ){
//				/* и предыдущая точка тоже попала */
//				if( start_off == 0 ){
//					/* вычисляем положение очередной точки
//						в качестве конечной */
//					modf( di + 0.5, &di );
//					modf( dj + 0.5, &dj );
//					modf( di * cx, &dx );
//					x = (unsigned short)dx;
//					modf( dj * cy, &dy );
//					y = (unsigned short)dy;
//					if( isAscend ){
//						x = nxRes - x;
//						y = nyRes - y;
//					}
//					/* выводим вектор */
//					if( xOld != x || yOld != y ){
//						vvector8( (int)nColor, (int)xOld, (int)yOld, (int)x, (int)y, image, imageWidth );
//						xOld = x;
//						yOld = y;
//					}
//				}else{ /* предыдущая не попала */
//					/* вычисляем положение очередной точки
//						в качестве начальной */
//					modf( di + 0.5, &di );
//					modf( dj + 0.5, &dj );
//					modf( di * cx, &dx );
//					xOld = (unsigned short)dx;
//					modf( dj * cy, &dy );
//					yOld = (unsigned short)dy;
//					if( isAscend ){
//						xOld = nxRes - xOld;
//						yOld = nyRes - yOld;
//					}
//					start_off = 0;
//				}
//			}else{ /* очередная точка не попала в область */
//				/* а предыдущая попала; меняем флаг */
//				if( start_off == 0 ) start_off = 1;
//			}
//
//			/* позиционируемся на следующую точку блока */
//			p += 8;
//		}
//	}
//	free( buf );
//
//	/* завершающие операции */
//	m6:
//	;
//}

/*---------------------------------------------------------------------
	VVECTOR8.C
	Функция представляет другую реализацию известной функции v_vector.asm.
	Строит виртуальный вектор, то есть записывает его не на экран,
	а в виртуальный буфер, заданный пользователем.
	Прототип: void vvector8 ( int color, int start_x, int start_y,
		int end_x, int end_y, unsigned char* buf, int buf_len );
	где
	color		индекс цвета ( 0 - 255 );
	start_x		координаты начальной
	start_y
	end_x		и конечной точек вектора 
	end_y			относительно левого верхнего угла буфера ( левый
				верхний угол буфера по определению совпадает с его началом );
	buf			указатель на буфер;
	buf_len		длина строки буфера ( в пикселах или байтах );
	Примечание. Несмотря на то, что координаты точек описаны как int,
		они должны быть положительными целыми числами. Пользователь
		должен сам следить за тем, чтобы вектор без переполнения
		умещался в виртуальном буфере. Функция не в состоянии следить
		за переполнением, так как полный размер виртуального буфера
		ей неизвестен.
	Date: 25 june 2004
---------------------------------------------------------------------*/
//void vvector8( int color, int start_x, int start_y, int end_x,
//		int end_y, unsigned char* buf, int buf_len ){
//	int dx; /* горизонтальная дистанция */
//	int x_inc; /* приращение по x */
//	int straight_count; /* удвоенная горизонтальная дистанция */
//	int dy; /* вертикальная дистанция */
//	int y_inc; /* приращение по y */
//	int diag_count; /* удвоенная вертикальная дистанция */
//	int d; /* количество точек вектора - 1 */
//	int cnt; /* то же, что и d; играет роль счетчика цикла */
//	unsigned char* p; /* адрес текущей точки вектора */
//	int xor_flag;
//
//	xor_flag = (color >> 8) & 0xff;
//	color &= 0xff;
//
//	/* вычисление горизонтальной дистанции */
//	dx = end_x - start_x;
//	if( dx >= 0 ){
//		x_inc = 1;
//		straight_count = dx * 2;
//	}else{
//		x_inc = -1;
//		straight_count = -dx * 2;
//	}
//
//	/* вычисление вертикальной дистанции */
//	dy = end_y - start_y;
//	if( dy >= 0 ){
//		y_inc = buf_len;
//		diag_count = dy * 2;
//	}else{
//		y_inc = -buf_len;
//		diag_count = -dy * 2;
//	}
//
//	/* адрес первой точки вектора */
//	p = buf + start_y * buf_len + start_x;
//
//	/* если длина вектора по горизонтали больше, прямые сегменты
//		горизонтальны */
//	if( straight_count >= diag_count ){
//		d = straight_count >> 1; /* горизонтальная дистанция */
//		cnt = d; /* счетчик цикла */
//		while( cnt ){
//			/* записываем текущую точку */
//			if( xor_flag ){
//				*p ^= (unsigned char)color;
//				if( *p < (unsigned char)0x80 ) *p = (char)0;
//				else *p = (unsigned char)0xff;
//			}else *p = (unsigned char)color;
//			/* переходим к следующей точке */
//			p += x_inc;
//			if( (d -= diag_count) < 0 ){
//				p += y_inc;
//				d += straight_count;
//			}
//			cnt--;
//		}
//	}/* иначе прямые сегменты отрезка вертикальны; здесь straight_count
//		и diag_count меняются местами */
//	else{
//		d = diag_count >> 1; /* вертикальная дистанция */
//		cnt = d; /* счетчик цикла */
//		while( cnt ){
//			/* записываем текущую точку */
//			if( xor_flag ){
//				*p ^= (unsigned char)color;
//				if( *p < (unsigned char)0x80 ) *p = (char)0;
//				else *p = (unsigned char)0xff;
//			}else *p = (unsigned char)color;
//			/* переходим к следующей точке */
//			p += y_inc;
//			if( (d -= straight_count) < 0 ){
//				p += x_inc;
//				d += diag_count;
//			}
//			cnt--;
//		}
//	}
//	/* записываем последнюю точку вектора ( end_x, end_y ) */
//	if( !xor_flag ) *p = (unsigned char)color;
//}

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
	if( netFile == NULL ) throw TException(5, "can'nt write to net file");

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
		throw TException( 5, "zero length coast file" );;
	}
	fseek( coastFile, 0, SEEK_SET );
	/* выделяем буфер ввода/вывода */
	buf = (unsigned char*)malloc( coastFileLen );
	if( buf == NULL ){
		fclose( netFile );
		fclose( coastFile );
		throw TException( 5, "mem allocation error" );;
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
}
