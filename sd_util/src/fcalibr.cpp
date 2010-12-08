/*---------------------------------------------------------------------
 FCalibr
 Программа калибровки одноканальных файлов видимый каналов
 формата NOAA AVHRR полученных из файлов FY-1c.

 Параметры калибровки для FY-1c получены из
 http://nsmc.cma.gov.cn/F1CUpdate_eng.html
 а для FY-1d из документа 
 CURRENT SATUS OF FY-1D METEOROLOGICAL SATELLITE
 расположенного по адресу 
 http://www.eumetsat.de/en/area2/cgms/cgms_xxxi/CGMS-XXXI_Working_Papers/CGMS-XXXI_PRC_WPs/CGMS-XXXI_PRC-WP-02.pdf

 Channel     1               2               6               7               8               9               10
 Gain        9.385068e-02    9.444013e-02    8.823577e-02    5.175893e-02    5.199454e-02    5.140914e-02    9.621714e-02
 Intercept   -5.810457e-01   -6.586441e-01   -7.790230e-01   -2.831052e-01   -3.228278e-01   -2.619047e-01   -7.173655e-01

 Запуск программы:
 FCalibr <имя исходного файла> <имя конечного файла>
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <c_lib.hpp>

const double ka = 0.1;
const double kb = 0;

const double FY_1c_vis[10][2] = {
 9.385068e-02, -5.810457e-01, // 1
 9.444013e-02, -6.586441e-01, // 2
          0.0,           0.0, // 3
          0.0,           0.0, // 4
          0.0,           0.0, // 5
 8.823577e-02, -7.790230e-01, // 6
 5.175893e-02, -2.831052e-01, // 7
 5.199454e-02, -3.228278e-01, // 8
 5.140914e-02, -2.619047e-01, // 9
 9.621714e-02, -7.173655e-01  // 10
};

const double FY_1d_vis[10][2] = {
    8.930E-02, -1.0719, // 1
    9.980E-02, -1.1972, // 2
          0.0,     0.0, // 3
          0.0,     0.0, // 4
          0.0,     0.0, // 5
    8.310E-02, -2.4113, // 6
    4.230E-02, -0.5498, // 7
    6.310E-02,   0.757, // 8
    8.170E-02, -1.0624, // 9
    8.920E-02, -1.2486  // 10
};

#define MAX_V 1024
short lut[MAX_V];


int fCalibrFiles( const char *source, const char* target );
int verifyBlk0( const TBlk0_AVHRR &blk0 );
int isVisualChannel( const TBlk0_AVHRR &blk0 );

const char * useMsg = " FCalibr\n\
 Программа калибровки одноканальных файлов видимый каналов \n\
 формата NOAA AVHRR полученных из файлов FY-1c и FY-1d.\n\
\n\
 Запуск программы:\n\
 FCalibr <имя исходного файла> <имя конечного файла>\n\
 FCalibr <имя исходного файла с расширением avh>\n\
\n\
 При использовании второго способа вызова программы результат будет занесен в\n\
 файл с расширением clb.\n\
";

int main ( int argc, char* argv[] )
{
	int ret;
	if( argc == 3 ){
		ret = fCalibrFiles( argv[1], argv[2] );
	}
	else if( argc == 2 ){
		int l = strlen(argv[1]);
		if( l >= 4 &&
			argv[1][l-4] == '.' &&
			argv[1][l-3] == 'a' &&
			argv[1][l-2] == 'v' &&
			argv[1][l-1] == 'h' ){
			char *a = (char*)malloc( l + 1 );
			strcpy( a, argv[1] );
			strcpy( a + l - 4, ".clb" );
			ret = fCalibrFiles( argv[1], a );
			free(a);
		}
		else {
			fprintf( stderr, "Если программе передается имя только одного файла"
                             " оно должно иметь расширение .avh.\n" );
			ret = 2;
		}
	}
	else {
		printf( "%s", useMsg );
		return 1;
	}
	return ret;
}

int fCalibrFiles( const char *source, const char* target ){
	FILE* sourcef = fopen( source, "rb" );
	if( sourcef == NULL ) {
		perror( source );
		return 2;
	}
	
	TBlk0_AVHRR blk0;
	if( fread( &blk0, sizeof(blk0), 1, sourcef ) != 1 ){
		fprintf( stderr, "Ошибка чтения файла %s\n", source );
		fclose( sourcef );
		return 3;
	}

	int leng = blk0.totalFrameNum * blk0.totalPixNum;
	short *data = (short*)malloc( leng*sizeof(short) );
	if( data == 0 ){
		fprintf( stderr, "Ошибка выделения памяти\n" );
		fclose( sourcef );
		return 4;
	}

	if( fread( data, sizeof(*data), leng, sourcef ) != leng ){
		fprintf( stderr, "Ошибка чтения файла %s\n", source );
		fclose( sourcef );
        free( data );
		return 5;
	}

	fclose( sourcef );

	if( verifyBlk0( blk0 ) ){
		fprintf( stderr, "Ошибка формата нулевого блока файла %s\n", source );
        free( data );
		return 6;
	}

	if( !isVisualChannel( blk0 ) ){
		fprintf( stderr, "Ожидается файл видимого канала (инфракрасные каналы не обрабатываются)\n" );
        free( data );
		return 7;
	}

	const double (*cal_param)[2];
	if( blk0.b0.satId == 25730 ) cal_param = FY_1c_vis;
	else /* blk0.satId == 27431 */ 
		 cal_param = FY_1d_vis;

	int chan = blk0.channel;
	for( int i = 0; i < MAX_V; i++ ){
		double b = i * cal_param[chan - 1][0] + cal_param[chan - 1 ][1];
        lut[i] = (short int)(floor(( b - kb ) / ka + 0.5));
		if( lut[i] < 0 ) lut[i] = -2;
	}

	int m = 0;
	for( int i = 0; i < leng; i++ ){
		if( data[i] >= 0 ){
			data[i] = lut[data[i]];
			if( data[i] > m ) m = data[i];
		}
	}

	/* Коррекция нулевого блока */
	blk0.ka = ka;
	blk0.kb = kb;
	blk0.maxPixelValue = m;
	blk0.processLevel |= 1;

	/* Запись результата */
	FILE* targetf = fopen( target, "wb" );
	if( sourcef == NULL ) {
		perror( target );
        free( data );
		return 8;
	}
	
	if( fwrite( &blk0, sizeof(blk0), 1, targetf ) != 1 ){
		fprintf( stderr, "Ошибка записи в файл %s\n", target );
		fclose( targetf );
        free( data );
		return 9;
	}

	if( fwrite( data, sizeof(*data), leng, sourcef ) != leng ){
		fprintf( stderr, "Ошибка записи в файл %s\n", target );
		fclose( targetf );
        free( data );
		return 10;
	}

	fclose( targetf );
	return 0;
}

/************************************************\
 int verifyBlk0( const TBlk0_AVHRR &blk0 )
 Проверка того, что нулевой блок blk0 является
 нулевым блоком канала распакованного файла
 FY-1c.
\************************************************/
int verifyBlk0( const TBlk0_AVHRR &blk0 ){
	if( blk0.b0.formatType != 0xff ) return 1; // тип формата --- новый
	if( blk0.b0.dataType1 != 2 ) return 1;	// (одноканальные данные)
	if( blk0.b0.dataType2 != 1 ) return 1; // (HRPT NOAA)
	if( blk0.b0.satId != 25730 && blk0.b0.satId != 27431 ) return 1; // не FY-1c и не FY-1d
    return 0;
}


/************************************************\
\************************************************/
int isVisualChannel( const TBlk0_AVHRR &blk0 ){
	int ch = blk0.channel;
	if( ch <= 2 || ch >= 6 ) return 1;
    return 0;
}

