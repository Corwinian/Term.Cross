/*---------------------------------------------------------------------
	GEN_META.C
	Программа генерации промежуточных метафайлов ( информационных
	текстовых файлов нашего собственного формата ) для включения
	информации о спутниках в различные базы данных.
	Программа генерирует метафайлы на основе информации, полученной
	из 0-блока файла ( TBlk0, f34_uni.h ), а также на основе
	информации, рассчитанной по орбитальной модели ( SGP8 ) с помощью
	телеграммы norad. Телеграмма берется из того же 0-блока. Если
	в 0-блоке телеграмма отсутствует, то программа выдает сообщение
	и завершается.

	Date: 7 june 2008
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <c_lib.hpp>
#include <hrpt.hpp>
#include <y_util.hpp>
#include <orbmodel.hpp>

/* прототипы функций */
void close_all ( void );

/* файловые переменные */
FILE* inp_handle = NULL;
FILE* out_handle = NULL;
char inp_file_name[100];
char out_file_name[100];
int inp_file_len;

/* переменные файла hrpt */
struct TBlk0 blk0;
uint16_t frame_c;		/* общее количество кадров входного файла */
uint16_t lframe;		/* длина кадра входного файла в 10-битовых словах */
uint16_t nword;		/* длина упакованного кадра в 16-битовых словах */
uint16_t nword2;		/* длина упакованного кадра в байтах */
int hrpt_disp[15];		/* массив смещений полей в кадре hrpt
			( подробнее см. описание функции build_mask_hrpt ) */
uint16_t channel_c;	/* количество каналов во входном файле */

/* идентификатор файла */
char id_str[100];

void err_exit( int rc ) {
	close_all();
	exit(rc);
}

int main ( int argc, char* argv[] ) {
	int i, j;

	if ( argc < 2 ) {
		fprintf( stderr, "Syntax: gen_meta.exe <inp_file_name.a0>\n" );
		return 1;
	}
	/* открываем файл */
	inp_handle = fopen ( argv[1], "rb" );
	if ( inp_handle == NULL ) {
		fprintf( stderr, "Can't open input file: %s\n", argv[1] );
		err_exit(2);
	}
	/* длина файла */
	fseek ( inp_handle, 0, SEEK_END );
	inp_file_len = ftell ( inp_handle );
	fseek ( inp_handle, 0, SEEK_SET );
	/* если длина файла меньше 512 байт, это не файл данных, выход */
	if ( inp_file_len < sizeof( struct TBlk0 )){
		fprintf( stderr, "file %s size < 512\n", argv[1] );
		err_exit(3);
	}

	/* читаем 0-блок */
	fread ( &blk0, 1, BLK0_SZ, inp_handle );

	/* проверяем формат файла ( f34 ) */
	if ( blk0.b0.formatType != 0xff ){
		fprintf( stderr, "incorrect file format.\n" );
		err_exit(3);
	}
	if ( blk0.b0.dataType1 != B0DT1_SRC && blk0.b0.dataType2 != B0DT2_HRPT_NOAA){
		fprintf( stderr, "unsupported file format.\n" );
		err_exit(3);
	}
	TBlk0_HRPT& h0 = (TBlk0_HRPT&)blk0;
	/* вычисляем переменные кадра входного файла */
	frame_c = h0.frameNum + h0.lostFrameNum;
	/* получаем распакованную длину кадра входного файла, а также
	устанавливаем массив смещений для всех полей кадра согласно маске */
	lframe = build_mask_hrpt( h0.frameMask, hrpt_disp, &channel_c );
	if ( h0.packType == A0_PACK_1_2 ) nword = lframe;
	else if ( h0.packType == A0_PACK_8_10 ) {
		nword = ( lframe * 5 ) / 8;
		if (( lframe * 5 ) % 8 ) nword++;
		nword++;
	} else err_exit(3); /* неправильный тип упаковки */

	/* проверяем размер файла */
	nword2 = nword * 2;
	int t_2 = (int)nword2 * frame_c + sizeof( struct TBlk0 );
	if ( t_2 != inp_file_len ){
		fprintf( stderr, "file size(%d) not equals expected size(%d).\n", inp_file_len, t_2 );
		err_exit(3);
	}
	/* проверяем количество каналов AVHRR */
	if ( channel_c == 0 ) err_exit(3);
	/* проверяем, есть ли в файле информация norad */
	if ( h0.year == 0 || h0.yearTime == 0.0 || h0.revNum == 0L ) {
		fprintf ( stderr, "Norad TLE info is absent in input file: %s\n", argv[1] );
		err_exit(3);
	}

	/* получаем имя входного файла без пути; для совместимости с ANSI
	не используем функцию _splitpath() */
	j = strlen ( argv[1] );
	for ( i = j - 1; i >= 0; i-- ) {
		if ( argv[1][i] == (char)'\\' || argv[1][i] == (char)'/' ||
			argv[1][i] == ':' ) break;
	}
	i++;
	strcpy ( inp_file_name, &argv[1][i] );

	/* читаем время приема */
	int scan_year = h0.b0.year;
	int month;
	int date;
	/* дата начала приема */
	dayToDate ( h0.b0.year, h0.b0.day, &month, &date );
	/* вычисляем часы, минуты, секунды */
	long ts = h0.b0.dayTime/1000;
	uint16_t sec = ts%60;
	ts = ts/60;
	uint16_t minute = ts%60;
	uint16_t hour = ts/60;

	double scan_time = (double)h0.b0.day + (double)h0.b0.dayTime/86400000.0;
//	printf ( "Receiving date and time: %2d %s %4d, %02d:%02d:%02d\n",
//		date, month_name[month], scan_year, hour, minute, sec );

	/* формируем идентификатор */
	/* удаляем концевые пробелы в b.satName */
	for ( i = strlen ( h0.b0.satName ) - 1; i > 0; i-- ) {
		if ( isalnum ( h0.b0.satName[i] )) break;
		else h0.b0.satName[i] = (char)0;
	}
	sprintf ( id_str, "%s_%4d%02d%02d_%02d%02d%02d", h0.b0.satName, scan_year,
		month, date, hour, minute, sec );
	/* заменяем посторонние символы на подчерк, приводим к верхнему
	регистру */
	j = strlen ( id_str );
	for ( i = 0; i < j; i++ ) {
		if ( isalnum ( id_str[i] ))
			id_str[i] = toupper( id_str[i] );
		else id_str[i] = (char)'_';
	}

	/* вычисляем время завершения сеанса */
	int end_year = scan_year;
	int end_month;
	int end_date;
	double end_time = scan_time;
	dayAdd ( &end_year, &end_time, (double)(frame_c-1)/6.0/86400.0 );
	double entire;
	double fraction = modf ( end_time, &entire );
	dayToDate ( end_year, entire, &end_month, &end_date );
	fraction = modf ( fraction * 86400000.0, &entire );
	/* вычисляем часы, минуты, секунды */
	ts = (long)entire/1000;
	ushort end_sec = ts%60;
	ts = ts/60;
	ushort end_minute = ts%60;
	ushort end_hour = ts/60;

	/* координаты полигона */
	double lat_1, lon_1;
	double lat_2, lon_2;
	double lat_3, lon_3;
	double lat_4, lon_4;
	/* инициализируем структуры орбитальной модели */
	TIniSatParams isp( blk0 );
	TNOAAImageParams nip( blk0 );
	TCorrectionParams cop( blk0 );
	/* инициализация прямой задачи */
	TStraightReferencer sr( isp, nip, cop );
	/* вычисляем координаты полигона */
	//IJ2LL ( 0.0, 0.0, &lon_1, &lat_1 );
	sr.xy2ll( 0, 0, &lon_1, &lat_1 );
	//IJ2LL ( 2047.0, 0.0, &lon_2, &lat_2 );
	sr.xy2ll( 2047, 0, &lon_2, &lat_2 );
	//IJ2LL ( 2047.0, (double)(frame_c-1), &lon_3, &lat_3 );
	sr.xy2ll( 2047, frame_c-1, &lon_3, &lat_3 );
	//IJ2LL ( 0.0, (double)(frame_c-1), &lon_4, &lat_4 );
	sr.xy2ll( 0, frame_c-1, &lon_4, &lat_4 );

	/* формируем имя выходного файла из идентификатора */
	sprintf ( out_file_name, "%s.meta", id_str );

	/* создаем ( открываем ) выходной файл */
	out_handle = fopen ( out_file_name, "w" );
	if ( out_handle == NULL ) {
		printf ( "Can't open output file: %s\n", out_file_name );
		close_all();
		return 2;
	}

	/* записываем информацию в выходной файл */
	fprintf ( out_handle, "collection=SML.NOAA_HRPT.LONGTIME\n" );
	fprintf ( out_handle, "id=%s\n", id_str );
	fprintf ( out_handle, "file=%s\n", inp_file_name );
	fprintf ( out_handle, "satellite=%s\n", blk0.b0.satName );
	fprintf ( out_handle, "satelliteNum=%d\n", blk0.b0.satId );
	fprintf ( out_handle, "revNum=%d\n", blk0.b0.revNum );
	fprintf ( out_handle, "orbitType=%c\n", (nip.fAscendFlag)? 'A':'D' );
	fprintf ( out_handle, "timeBegin=%4d-%02d-%02dT%02d:%02d:%02dZ\n",
		scan_year, month, date, hour, minute, sec );
	fprintf ( out_handle, "timeEnd=%4d-%02d-%02dT%02d:%02d:%02dZ\n",
		end_year, end_month, end_date, end_hour, end_minute, end_sec );
	fprintf ( out_handle, "polygon=%.2lf,%.2lf %.2lf,%.2lf %.2lf,%.2lf %.2lf,%.2lf\n",
		lat_1*RD, lon_1*RD, lat_2*RD, lon_2*RD,
		lat_3*RD, lon_3*RD, lat_4*RD, lon_4*RD );
	fprintf ( out_handle, "totalScans=%d\n", frame_c );
	fprintf ( out_handle, "goodScans=%d\n", h0.frameNum );

	/* если присутствуют не все каналы AVHRR, распечатываем их номера */
	if ( channel_c < 5 ) {
		fprintf ( out_handle, "channels=" );
		for ( i = 8; i <= 12; i++ ) {
			if ( hrpt_disp[i] != -1 )
				fprintf ( out_handle, "%c", (char)('0'+i-7));
		}
		fprintf ( out_handle, "\n" );
	}

	close_all();
	return 0;
}

/*---------------------------------------------------------------------
	CLOSE_ALL.C
---------------------------------------------------------------------*/
void close_all()
{
	if ( inp_handle ) {
		fclose ( inp_handle );
		inp_handle = 0;
	}
	if ( out_handle ) {
		fclose ( out_handle );
		out_handle = 0;
	}
}
