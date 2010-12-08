/*-----------------------------------------------------------------------------
	CHECK_A0.C
	Программа проверяет состоятельность файла *.a0, то есть компрессированного
	файла данных формата f34_uni. Программа проверяет большинство полей 0-блока.
	Может работать самостоятельно, либо в составе скрипта. Возвращает все
	возможные ошибки и сообщения.
	Date: 11 may 2010
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "f34_uni.h"
#include "hrpt.h"

/* прототипы функций */
uint16_t build_mask_hrpt ( uint32_t );
void _split_path ( char*, char*, char*, char* );
void print_inp_file_name ( void );
void close_all ( void );
/* определения констант для _split_path и _make_path */
#ifndef NAME_MAX
	#define NAME_MAX 255
#endif
#ifndef PATH_MAX
	#define PATH_MAX 4096
#endif

/* файловые переменные */
FILE* inp_handle = NULL;
char inp_file_name[NAME_MAX];
char inp_file_ext[NAME_MAX];
int inp_file_len;

/* переменные файла hrpt */
struct TBlk0_uni b;
uint16_t frame_c;		/* общее количество кадров входного файла */
uint16_t lframe;		/* длина кадра входного файла в 10-битовых словах */
uint16_t nword;		/* длина упакованного кадра в 16-битовых словах */
uint16_t nword2;		/* длина упакованного кадра в байтах */
int hrpt_disp[15];		/* массив смещений полей в кадре hrpt
					( подробнее см. описание функции build_mask_hrpt ) */
uint16_t channel_c;	/* количество каналов во входном файле */
uint16_t pack_type;
unsigned rev_num4;

/* структура информации о спутниках */
struct t_sat_info {
	uint16_t serial_num;
	uint32_t sat_idx;
	const char* p_prefix;
	const char* p_name;
};
struct t_sat_info s_info[11] = { 9, 15427, "n9", "NOAA 9",
	10, 16969, "n0", "NOAA 10",
	11, 19531, "n1", "NOAA 11",
	12, 21263, "n2", "NOAA 12",
	14, 23455, "n4", "NOAA 14",
	15, 25338, "n5", "NOAA 15",
	16, 26536, "n6", "NOAA 16",
	17, 27453, "n7", "NOAA 17",
	18, 28654, "n8", "NOAA 18",
	19, 33591, "n9", "NOAA 19",
	0, 0L, 0, 0 };

int ret_cod;
int file_out_flag = 0;

int main ( int argc, char* argv[] )
{
	int i, j, k, l;

	if ( argc < 2 ) {
		printf ( "Syntax: check_a0.exe <hrpt_a0_input_file>\n" );
		printf ( "hrpt_a0_input_file\n\tinput HRPT file of f34 format;\n" );
		return 1;
	}

	ret_cod = 0;
	/* открываем файл */
	inp_handle = fopen ( argv[1], "rb" );
	if ( inp_handle == NULL ) {
		printf ( "Can't open input file: %s\n", argv[1] );
		close_all();
		return 2;
	}
	/* получаем имя входного файла */
	_split_path ( argv[1], NULL, inp_file_name, inp_file_ext );
	/* длина файла */
	fseek ( inp_handle, 0, SEEK_END );
	inp_file_len = (int) ftell ( inp_handle );
	fseek ( inp_handle, 0, SEEK_SET );
	/* если длина файла меньше 512 байт, это не файл данных, выход */
	if ( inp_file_len < sizeof( struct TBlk0_uni )) {
		print_inp_file_name();
		printf ( "Input file is too small: %d\n", inp_file_len );
		goto iff;
	}

	/* читаем 0-блок */
	fread ( &b, 1, sizeof( struct TBlk0_uni ), inp_handle );

	/* проверяем формат файла ( f34 ) */
	if ( b.formatType != 0xff ) {
		print_inp_file_name();
		printf ( "Invalid formatType: 0x%02x\n", b.formatType );
iff:		close_all();
		return 3;
	}
	/* тип данных */
	if ( b.dataType1 != 1 ) {
		print_inp_file_name();
		printf ( "Invalid dataType1: 0x%02x\n", b.dataType1 );
		goto iff;
	}
	if ( b.dataType2 != 1 ) {
		print_inp_file_name();
		printf ( "Invalid dataType2: 0x%02x\n", b.dataType2 );
		goto iff;
	}
	/* проверяем первые 4 символа названия спутника;
	в данной версии программы распознаются только спутники NOAA */
//	if ( strnicmp ( b.satName, "noaa", 4 ) != 0 ) {
	if ( strncasecmp ( b.satName, "noaa", 4 ) != 0 ) {
		print_inp_file_name();
		printf ( "Satellite is not supported %s\n", b.satName );
		goto iff;
	}

	/* вычисляем переменные кадра входного файла */
	frame_c = b.frame_c + b.frame_lost;
	pack_type = b.pack_type;
	/* проверяем маску кадра */
	if ( b.frame_mask != 0xfd5f ) {
		print_inp_file_name();
		printf ( "Frame mask ( %d ) does not fit to level 0\n", (unsigned)b.frame_mask );
		goto iff;
	}
	/* получаем распакованную длину кадра входного файла, а также
	устанавливаем массив смещений для всех полей кадра согласно маске */
	lframe = build_mask_hrpt ( b.frame_mask );
	if ( pack_type == 0 ) nword = lframe;
	else if ( pack_type == 2 ) {
		nword = ( lframe * 5 ) / 8;
		if (( lframe * 5 ) % 8 ) nword++;
		nword++;
	}
	else {
		print_inp_file_name();
		printf ( "Unsupported pack type: %d\n", b.pack_type );
		goto iff;	/* неправильный тип упаковки */
	}
	/* проверяем размер файла */
	nword2 = nword * 2;
	if ((uint32_t)nword2 * frame_c + sizeof( struct TBlk0_uni ) != inp_file_len ) {
		print_inp_file_name();
		printf ( "Invalid input file length %d or frame count: %d\n", inp_file_len, frame_c );
		ret_cod = 4;
	}
	/* проверяем количество каналов AVHRR */
	if ( channel_c == 0 ) {
		print_inp_file_name();
		printf ( "Invalid channel count: %d\n", channel_c );
		ret_cod = 4;
	}
	/* находим спутник в таблице по индексу */
	for ( i = 0, j = 0; s_info[i].sat_idx != 0L; i++ ) {
		if ( s_info[i].sat_idx == b.satId ) {
			j = i;
			break;
		}
	}
	if ( j == 0 ) {
		print_inp_file_name();
		printf ( "Invalid satId: %d\n", b.satId );
		goto iff;
	}

	/* выделяем остаток номера витка */
	rev_num4 = atoi( inp_file_name+2 );

	/* проверяем название спутника */
	//if ( strnicmp ( s_info[j].p_name, b.satName, strlen( s_info[j].p_name )) != 0 ) {
	if ( strncasecmp ( s_info[j].p_name, b.satName, strlen( s_info[j].p_name )) != 0 ) {
		print_inp_file_name();
		printf ( "Satellite name %s doesnt match to satellite index: %d\n", b.satName, b.satId );
		ret_cod = 4;
	}
	//if ( strnicmp ( s_info[j].p_prefix, inp_file_name, 2 ) != 0 ) {
	if ( strncasecmp ( s_info[j].p_prefix, inp_file_name, 2 ) != 0 ) {
		print_inp_file_name();
		printf ( "Input file name %s doesnt match to 0-block satName: %s\n", inp_file_name, b.satName );
		ret_cod = 4;
	}
	if (( b.revNum % 10000 ) != rev_num4 ) {
		print_inp_file_name();
		printf ( "Input file name %s doesnt match to 0-block revNum: %d\n", inp_file_name, b.revNum );
		ret_cod = 4;
	}
	
	/* заключительные операции */
	close_all();
	return ret_cod;
}

/*---------------------------------------------------------------------
	CLOSE_ALL.C
---------------------------------------------------------------------*/
void close_all ()
{
	if ( inp_handle ) {
		fclose ( inp_handle );
		inp_handle = 0;
	}
}

/*---------------------------------------------------------------------
	BUILD_MASK_HRPT.C
	Функция, аналогичная старой функции build_mask. Предназначена для
	вычисления длины кадра hrpt по маске hrpt.

	Кроме этого, функция вычисляет массив смещений для каждого поля hrpt
	в зависимости от заданной маски. Массив смещений задается как
	глобальная переменная hprt_disp[18] типа int. Каждый элемент
	этого массива задает смещение соответствующего маске поля в кадре
	входного файла ( смещение >= 0 ). Если поле отсутствует в кадре,
	элемент массива, соответствующий этому полю, имеет значение -1.
	Все поля в кадре обрабатываются по порядку. Порядок следования
	полей задан во второй части файла hrpt.h. Первым полем в кадре
	считается поле IDN, а последним - поле FSY ( это не совсем
	соответствует константам маски, описанным в первой части файла hrpt.h ).
	Для единоообразной обработки всех полей кадра hrpt введены локальные
	статические массивы, определенные через константы кадра hrpt.

	Также функция вычисляет количество каналов AVHRR во входном файле.
	Это значение необходимо для выделения отдельных каналов из битового
	потока, в котором все имеющиеся каналы чередуются ( т.н. interleaving ).
	Значение передается в функцию main через глобальную переменную channel_c.

	Используемые глобальные переменные: hrpt_disp[], channel_c
---------------------------------------------------------------------*/
uint16_t build_mask_hrpt ( uint32_t mask )
{
	int i;
	int j;
	static uint16_t m_len[15] = { LIDN, LTCD, LTLM, LBSC,
		LSPC, LHSY, LTIP-LTNO, LSPR, LAVHRR, LAVHRR, LAVHRR, LAVHRR,
		LAVHRR, LASY, LFSY };
	static uint32_t def_masks[15] = { MIDN, MTCD, MTLM, MBSC,
		MSPC, MHSY, MTIP, MSPR, M1, M2, M3, M4, M5, MASY, MFSY };
	uint16_t d;

	/* проходим по всем полям кадра, отмечаем отсутствующие поля,
	вычисляем смещения */
	d = 0;	/* начальное смещение для первого поля равно 0 */
	for ( i = 0; i < 15; i++ ) {
		/* если поле присутствует */
		if ( mask & def_masks[i] ) {
			/* записываем в поле накопленное значение смещения */
			hrpt_disp[i] = (int)d;
			/* и увеличиваем смещение для следующего поля */
			d += m_len[i];
		}
		else {	/* если текущее поле отсутствует, просто отмечаем поле
				 как отсутствующее; текущее накопленное смещение не меняется */
			hrpt_disp[i] = - 1;
		}
	}

	/* находим смещение первого присутствующего в файле канала AVHRR;
	отметим, что каналы AVHRR занимают в массиве hrpt_disp по порядку
	элементы с 8 по 12 включительно; это следует из определения
	статического массива def_masks */
	for ( i = 0; i < 5; i++ ) {
		if ( hrpt_disp[i+8] != - 1 ) {
			j = hrpt_disp[i+8];
			break;
		}
	}
	/* корректируем	смещения каналов AVHRR с учетом interleaving'а;
	заодно считаем количество каналов AVHRR */
	channel_c = 0;
	for ( i = 0; i < 5; i++ ) {
		if ( hrpt_disp[i+8] != - 1 ) {
			hrpt_disp[i+8] = j;
			j++;	/* смещения всех каналов отличаются на 1 слово */
			channel_c++;
		}
	}

	/* минимальную длину кадра в этом варианте программы вычислять не нужно;
	она уже вычислена в переменной d после окончания цикла */
	/* увеличиваем ее до числа, кратного 8 */
	while ( d & 7 ) d++;
	return d;
}

/*---------------------------------------------------------------------
	_SPLIT_PATH.C
	Аналог известной из DOS и Windows функции, входящей в stdlib.
	В отличие от своего прототипа, данная функция будет работать
	в любой операционной системе, поскольку использует внесистемные
	конвенции.
	Эти конвенции следующие:
	1. Любой полный путь файла функция делит не на 4, а на 3 части,
	а именно, собственно путь, имя файла без расширения и расширение.
	2. Граница между собственно путем и именем файла проходит по
	последнему в исходной строке символу '\' или '/'.
	3. Граница между именем файла без расширения и расширением
	проходит по последней точке '.' в имени файла.
	Прототип: void _split_path ( char* full_path, char* path,
		char* name, char* ext );
	где
	full_path		входная строка;
	path, name, ext 	подаваемые пользователем массивы для
		записи выходных компонентов строки или NULL.

	Примечания:
	1. Если во входной строке отсутствует любой из компонентов,
	функция возвращает для данного компонента строку нулевой длины.
	2. Подаваемая в функцию строка и выходные компоненты должны иметь
	достаточный размер для хранения выделяемых строк. Для Windows это
	может быть _MAX_DIR и _MAX_FNAME, а для Linux - PATH_MAX и NAME_MAX.
	3. Пользователь может задать NULL в качестве выходного компонента
	строки. В этом случае функция не определяет данный компонент.
	4. Последний символ '\' или '/', разделяющий путь и имя файла,
	присоединяется к выходной строке path. Последняя точка, разделяющая
	имя файла и расширение, присодиняется спереди к расширению ext.

	Date: 4 december 2008
----------------------------------------------------------------------*/
void _split_path ( char* full_path, char* path, char* name, char* ext )
{
	char* slash_p;
	char* back_slash_p;
	char* point_p;
	char* name_p;
	int path_len;
	int name_len;

	/* если строка нулевой длины, выход */
	if ( strlen ( full_path ) == 0 ) {
		if ( path ) path[0] = (char)0;
		if ( name ) name[0] = (char)0;
		if ( ext ) ext[0] = (char)0;
		return;
	}

	/* находим во входной строке базовые опорные символы */
	slash_p = strrchr ( full_path, '/' );
	back_slash_p = strrchr ( full_path, '\\' );

	/* производим разбор полученных результатов */
	/* если отсутствует путь */
	if ( slash_p == NULL && back_slash_p == NULL ) {
		if ( path ) path[0] = (char)0;
		name_p = full_path;
	}
	else {	/* если путь присутствует */
		name_p = ( slash_p > back_slash_p )? slash_p: back_slash_p;
		name_p ++;	/* присоединяем слэш к path */
		/* копируем его в выходную строку */
		if ( path ) {
			path_len = name_p - full_path;
			strncpy ( path, full_path, path_len );
			path[path_len] = (char)0;
		}
	}

	/* если остаток строки нулевой длины, выход */
	if ( strlen ( name_p ) == 0 ) {
		if ( name ) name[0] = (char)0;
		if ( ext ) ext[0] = (char)0;
		return;
	}

	/* ищем расширение */
	point_p = strrchr ( name_p, '.' );
	/* если расширение отсутствует */
	if ( point_p == NULL ) {
		/* копируем остаток в имя */
		if ( name ) strcpy ( name, name_p );
		if ( ext ) ext[0] = (char)0;
	}
	else {	/* расширение присутствует */
		/* копируем имя */
		if ( name ) {
			name_len = point_p - name_p;
			strncpy ( name, name_p, name_len );
			name[name_len] = (char)0;
		}
//		point_p++;	/* пропускаем точку */
		/* копируем расширение */
		if ( ext ) {
			if ( strlen ( point_p ) == 0 ) ext[0] = (char)0;
			else strcpy ( ext, point_p );
		}
	}
}

void print_inp_file_name ( void )
{
	if ( file_out_flag == 0 ) {
		printf ( "\n%s%s\n", inp_file_name, inp_file_ext );
		file_out_flag = 1;
	}
}
