/*-------------------------------------------------------------------------
	b0.cpp
-------------------------------------------------------------------------*/
#include <iostream>
#include <string>

using namespace std;

#include <c_lib.hpp>
#include <y_util.hpp>

string fixed_part_title = "\
---------------------------------------------------------------------------\n\
    Фиксированная часть\n\
---------------------------------------------------------------------------\n";

string specific_part_title = "\
---------------------------------------------------------------------------\n\
    Специфическая часть\n\
---------------------------------------------------------------------------\n";

string b0_0 = "\
0    тип формата                                    ";
string b0_1 = "\
1    имя спутника                                   ";
string b0_14 = "\
14   идентификатор спутника                         ";
string b0_18 = "\
18   номер витка                                    ";
string b0_22 = "\
22   год                                            ";
string b0_24 = "\
24   день в году                                    ";
string b0_24_2 = "\
24b  дата                                           ";

string b0_26 = "\
26   время дня (мсек,UTC)                           ";
string b0_26_2 = "\
26b  время дня (UTC)                                ";
string b0_62 = "\
62   тип данных [1]                                 ";
string b0_63 = "\
63   тип данных [2]                                 ";

string bh_64 = "\
64   количество принятых кадров                     ";
string bh_66 = "\
66   количество кадров со сбоем синхронизации       ";
string bh_68 = "\
68   количество кадров без сбоев по времени         ";
string bh_70 = "\
70   количество кадров с исправленным временем      ";
string bh_72 = "\
72   количество пропусков данных                    ";
string bh_74 = "\
74   тип упаковки                                   ";
string bh_76 = "\
76   полная длина строки в пикселах                 ";
string bh_78 = "\
78   маска кадра                                    ";
string bh_82 = "\
82   количество пропущенных пикселов                ";
string bh_84 = "\
84   количество принятых пикселов                   ";
string bh_86 = "\
86   тип витка (0 - нисходящий, 1 - восходящий)     ";

string ba_64 = "\
64   стадия обработки данных                        ";
string ba_68 = "\
68   номер канала                                   ";
string ba_70 = "\
70   количество строк                               ";
string ba_72 = "\
72   полная длина строки в пикселах                 ";
string ba_74 = "\
74   количество пропущенных пикселов                ";
string ba_76 = "\
76   количество принятых пикселов                   ";
string ba_78 = "\
78   тип витка (0 - нисходящий, 1 - восходящий)     ";
string ba_80 = "\
80   максимальное значение среди значимых пикселов  ";
string ba_82 = "\
82   коэффициент пересчёта A                        ";
string ba_90 = "\
90   коэффициент пересчёта B                        ";

string bp_64 = "\
64   стадия обработки данных                        ";
string bp_68 = "\
68   номер канала                                   ";
string bp_70 = "\
70   максимальное значение среди значимых пикселов  ";
string bp_72 = "\
72   тип проекции                                   ";
string bp_74 = "\
74   количество строк                               ";
string bp_76 = "\
76   количество пикселов в строке                   ";
string bp_78 = "\
78   широта (град)                                  ";
string bp_82 = "\
82   долгота (град)                                 ";
string bp_86 = "\
86   размер по широте (град)                        ";
string bp_90 = "\
90   размер по долготе (град)                       ";
string bp_94 = "\
94   шаг по широте (сек)                            ";
string bp_98 = "\
98   шаг по долготе (сек)                           ";
string bp_102 = "\
102  коэффициент пересчёта A                        ";
string bp_110 = "\
110  коэффициент пересчёта B                        ";

string bt_64 = "\
64   количество строк                               ";
string bt_66 = "\
66   номер канала                                   ";

string n_128 = "\
128  номер опорного витка                           ";
string n_132 = "\
132  номер набора                                   ";
string n_134 = "\
134  тип эфемерид                                   ";
string n_136 = "\
136  год                                            ";
string n_138 = "\
138  полное время в году                            ";
string n_146 = "\
146  среднее движение (рад/мин)                     ";
string n_154 = "\
154  BSTAR                                          ";
string n_162 = "\
162  наклонение орбиты (рад)                        ";
string n_170 = "\
170  прямое восхождение (рад)                       ";
string n_178 = "\
178  эксцентриситет                                 ";
string n_186 = "\
186  аргумент перигея (рад)                         ";
string n_194 = "\
194  средняя аномалия (рад)                         ";

string c_256 = "\
256  номер версии коррекции                         ";
string c_258 = "\
258  поправка времени по TBUS (мсек)                ";
string c_260 = "\
260  поправка времени (мсек)                        ";
string c_262 = "\
262  крен (roll)                                    ";
string c_270 = "\
270  тангаж (pitch)                                 ";
string c_278 = "\
278  рысканье (yaw)                                 ";

void print_fixed_part( TBlk0* b );
void print_specific_part( TBlk0* b);
void print_old_format( TOldBlk0* b );
void print_blk0HRPT( TBlk0_HRPT* b );
void print_blk0AVHRR( TBlk0_AVHRR* b );
void print_blk0proj( TBlk0_Proj* b );
void print_blk0TLM( TBlk0_TLM* a );
void print_norad( void *blk0_specific_part );
void print_georef_correction( void *blk0_specific_part );

//int isLeapYear( int );
//void dayToDate( int, int, int *, int * );


int main( int argc, char** argv ) {
	if( argc < 2 ){
		cout << "\nУниверсальная программа просмотра содержимого 0-блока файлов данных.\n"
			"Использование:\n"
			"B0 <файл>\n";
		return 0;
	}

	char buff[BLK0_SZ];
	TBlk0* blk0 = (TBlk0*) buff;
	TOldBlk0 *oblk0 = (TOldBlk0 *) buff;

	FILE *file = fopen(argv[1], "rb");
	if( file == NULL || fread( buff, 1, BLK0_SZ, file ) != BLK0_SZ ){
		cerr << "Ошибка доступа к файлу " << argv[1] << endl;
		fclose( file );
		return 1;
	}
	fclose( file );

	cout << "Файл: " << argv[1] << endl << endl;
	if( blk0->b0.formatType == 0xff ){    // файл нового формата
		cout << fixed_part_title;
		print_fixed_part( blk0 );
		cout << endl << endl << specific_part_title;
		print_specific_part( blk0 );
		} else {                               // файл старого формата
		print_old_format( oblk0 );
	}

	return 0;
}



void print_fixed_part( TBlk0* blk0fs ) {
	int month = 0;
	int day = 0;
	dayToDate1(blk0fs->b0.year,blk0fs->b0.day,&month,&day);
	//month++;
	//day++; //??? 
	
	//string smonth = month < 10 ? string("0") : string();
	//smonth += (month+1);
	//string sday = day < 10 ? string("0") : string();
	//sday += (day+1);
	//cerr << blk0fs->b0.year << " " << blk0fs->b0.day << " " << smonth << " " << sday << endl;
	cout
		<< b0_0 << hex << (unsigned)blk0fs->b0.formatType << dec << endl
		<< b0_1 << blk0fs->b0.satName << endl
		<< b0_14 << blk0fs->b0.satId << " (" << hex << blk0fs->b0.satId << dec << ")" << endl
		<< b0_18 << blk0fs->b0.revNum << endl
		<< b0_22 << blk0fs->b0.year << endl
		<< b0_24 << blk0fs->b0.day << endl
		<< b0_24_2 << blk0fs->b0.year << "-" << (month<10?"0":"") << month << "-" << (day<10?"0":"") << day << endl
		<< b0_26 << blk0fs->b0.dayTime << endl;

	long hh,mm,ss;
	long ts = blk0fs->b0.dayTime/1000;
	ss = ts%60;
	ts = ts/60;
	mm = ts%60;
	hh = ts/60;
	cout << b0_26_2 << hh << ":" << mm << ":" << ss << endl;

	cout << b0_62 << (unsigned)blk0fs->b0.dataType1;
	switch( blk0fs->b0.dataType1 ){
	case 1:
		cout << " (исходные данные)";
		break;
	case 2:
		cout << " (одноканальные данные)";
		break;
	case 3:
		cout << " (проекция)";
		break;
	case 4:
		cout << " (телеметрия)";
		break;
	}

	cout << endl << b0_63 << (unsigned)blk0fs->b0.dataType2;
	switch( blk0fs->b0.dataType1 ){
	case 1:
		switch( blk0fs->b0.dataType2 ){
		case 1:
			cout << " (HRPT NOAA)";
			break;
		case 2:
			cout << " (HRPT SeaStar)";
			break;
		case 16:
			cout << " (GMS LRFAX)";
			break;
		case 17:
			cout << " (GMS S-VISSR)";
			break;
		}
		break;
	case 2:
	case 3:
		switch( blk0fs->b0.dataType2 ){
		case 1:
			cout << " (HRPT NOAA)";
			break;
		case 2:
			cout << " (NOAA HIRS)";
			break;
		case 3:
			cout << " (NOAA MSU)";
			break;
		case 4:
			cout << " (NOAA SSU)";
			break;
		case 11:
			cout << " (SeaStar SeaWiFS)";
			break;
		case 21:
			cout << " (GMS LRFAX)";
			break;
		case 22:
			cout << " (GMS S-VISSR)";
			break;
		}
		break;
	case 4:
		switch( blk0fs->b0.dataType2 ){
		case 1:
			cout << " (HRPT NOAA)";
			break;
		}
		break;
	}
}

void print_specific_part( TBlk0* blk0 ) {
	switch( blk0->b0.dataType1 ){
	case 1:
		print_blk0HRPT( (TBlk0_HRPT*)blk0 );
		break;
	case 2:
		print_blk0AVHRR( (TBlk0_AVHRR*)blk0 );
		break;
	case 3:
		print_blk0proj( (TBlk0_Proj*)blk0 );
		break;
	case 4:
		print_blk0TLM( (TBlk0_TLM*)blk0 );
		break;
	}
}

void print_blk0HRPT( TBlk0_HRPT* b ) {
	cout
		<< bh_64 << b->frameNum << endl
		<< bh_66 << b->lostFrameNum << endl
		<< bh_68 << b->validTimeFrameNum << endl
		<< bh_70 << b->correctedTimeFrameNum << endl
		<< bh_72 << b->gapNum << endl
		<< bh_74 << b->packType << endl
		<< bh_76 << b->totalPixNum << endl
		<< bh_78 << hex << b->frameMask << dec << endl
		<< bh_82 << b->pixGap << endl
		<< bh_84 << b->pixNum << endl
		<< bh_86 << b->ascendFlag << endl;
	print_norad( b );
	print_georef_correction( b );
}

void print_blk0AVHRR( TBlk0_AVHRR * b ) {
	cout
		<< ba_64 << hex << b->processLevel << dec << endl
		<< ba_68 << b->channel << endl
		<< ba_70 << b->totalFrameNum << endl
		<< ba_72 << b->totalPixNum << endl
		<< ba_74 << b->pixGap << endl
		<< ba_76 << b->pixNum << endl
		<< ba_78 << b->ascendFlag << endl
		<< ba_80 << b->maxPixelValue << endl
		<< ba_82 << b->ka << endl
		<< ba_90 << b->kb << endl;
	print_norad( b );
	print_georef_correction( b );
}


void print_blk0proj( TBlk0_Proj * b ) {
	cout
		<< bp_64 << hex << b->processLevel << dec << endl
		<< bp_68 << b->channel << endl
		<< bp_70 << b->maxPixelValue << endl
		<< bp_72 << b->projType << " (" << (b->projType == 1 ? "меркаторская" : "равнопромежуточная") << ")" << endl
		<< bp_74 << b->scanNum << endl
		<< bp_76 << b->pixNum << endl
		<< bp_78 << b->lat << endl
		<< bp_82 << b->lon << endl
		<< bp_86 << b->latSize << endl
		<< bp_90 << b->lonSize << endl
		<< bp_94 << b->latRes << endl
		<< bp_98 << b->lonRes << endl
		<< bp_102 << b->ka << endl
		<< bp_110 <<  b->kb << endl;
	print_norad( b );
	print_georef_correction( b );
}


void print_blk0TLM( TBlk0_TLM * a ) {
	cout
		<< bt_64 << a->totalFrameNum << endl
		<< bt_66 << a->channel << endl;
}


void print_norad( void *blk0_specific_part ) {
	TBlk0_HRPT *b = (TBlk0_HRPT *)blk0_specific_part;
	cout
		<< endl << "Телеграмма NORAD:" << endl
		<< n_128 << b->revNum << endl
		<< n_132 << b->setNum << endl
		<< n_134 << b->ephemType << endl
		<< n_136 << b->year << endl
		<< n_138 << b->yearTime << endl
		<< n_146 << b->n0 << endl
		<< n_154 << b->bstar << endl
		<< n_162 << b->i0 << endl
		<< n_170 << b->raan << endl
		<< n_178 << b->e0 << endl
		<< n_186 << b->w0 << endl
		<< n_194 << b->m0 << endl;
}		


void print_georef_correction( void *blk0_specific_part ) {
	TBlk0_HRPT *b = (TBlk0_HRPT *)blk0_specific_part;
	cout
		<< endl <<  "Параметры коррекции географической привязки:" << endl
		<< c_256 << b->corVersion << endl
		<< c_258 << b->corTBUSTime << endl
		<< c_260 << b->corTime << endl
		<< c_262 << b->corRoll << endl
		<< c_270 << b->corPitch << endl
		<< c_278 << b->corYaw << endl;
}

void print_old_format( TOldBlk0 *b ) {
    unsigned long sat_num = ((unsigned long)b->satNumber >> 1) & 0x3f;
    string  file_types[8] = { "NOAA APT", "NOAA HRPT", "распакованный HRPT (2 байта на пиксел)",
        "распакованный HRPT (1 байт на пиксел)", "GMS LRFAX", "GMS HRFAX", "меркаторская проекция",
        "равнопромежуточная проекция" };

    cout <<
"0-блок файла - старого формата !!!" << endl << endl <<

"0    число                                " << (unsigned long)b->day << endl <<
"1    месяц                                " << (unsigned long)b->month << endl <<
"2    год                                  " << (unsigned long)b->year << endl <<
"3    номер спутника                       " << hex << (unsigned long)b->satNumber << dec << " (" << sat_num << ")" << endl <<
"4    час                                  " << (unsigned long)b->hour << endl <<
"5    минута                               " << (unsigned long)b->minute << endl <<
"6    секунда                              " << (unsigned long)b->sec << endl <<
"7    тик                                  " << (unsigned long)b->tic << endl << endl <<

"8    номер витка                          " << (unsigned long)b->revNum << endl <<
"10   число сканов                         " << (unsigned long)b->totalScans << endl <<
"52   тип спутника                         " << (unsigned long)b->satType << " (" << (b->satType ? "GMS" : "NOAA") << ")" << endl <<
"54   год запуска спутника                 " << (unsigned long)b->launchYear << endl << endl <<

"184  минимальная температура              " << b->minT << endl <<
"188  шаг по температуре                   " << b->stepT << endl <<
"192  номер канала                         " << (unsigned long)b->chNumber << endl <<
"193  код калибровки                       " << (unsigned long)b->calCode << endl <<
"194  число классов                        " << (unsigned long)b->numberOfClasses << endl << endl <<

"256  тип файла                            " << (unsigned long)b->typeOfFile << " (" << file_types[b->typeOfFile] << ")" << endl <<
"257  тип упаковки                         " << (unsigned long)b->typeOfPack << endl <<
"258  количество малых кадров HRPT         " << (unsigned long)b->numberOfFrames << endl <<
"260  длина кадра HRPT                     " << (unsigned long)b->frameLength << endl <<
"264  маска кадра HRPT                     " << hex << (unsigned long)b->frameSegMask << dec << endl <<
"266  число пропущенных пикселов           " << (unsigned long)b->pixStrPassed << endl <<
"268  число принятых пикселов              " << (unsigned long)b->pixStrLength << endl << endl <<

"270  длина строки проекции                " << (unsigned long)b->projPixStrLength << endl <<
"272  число строк проекции                 " << (unsigned long)b->projNumberOfScans << endl <<
"274  число байт на пиксел                 " << (unsigned long)b->projPixelBytes << endl <<
"276  размер пиксела проекции              " << b->projPixelSize << endl <<
"280  минимальная широта                   " << b->minProjLat << endl <<
"284  максимальная широта                  " << b->maxProjLat << endl <<
"288  минимальная долгота                  " << b->minProjLon << endl <<
"292  максимальная долгота                 " << b->maxProjLon << endl <<
"296  сдвиг проекции по горизонтали        " << (unsigned long)b->hProjShift << endl <<
"298  сдвиг проекции по вертикали          " << (unsigned long)b->vProjShift << endl <<
"300  стадия обработки                     " << hex << (unsigned long)b->processState << dec << endl <<
"302  отношение высоты к радиусу орбиты    " << b->mKHR << endl << endl <<

"306  степень полинома h                   " << (unsigned long)b->hPolyDegree << endl <<
"308  степень полинома v                   " << (unsigned long)b->vPolyDegree << endl <<
"     Коэффициенты полинома:" << endl;
    for( int i=0; i<20; i++ ) cout << 310 + i*sizeof(float) <<
   "                                       " << b->polyCoeffs[i] << endl;

}

/*
static int months[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
static int leapMonths[13] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };


int isLeapYear( int year ) {
	return ( (year%4==0 && year%100!=0) || year%400==0 ) ? 1 : 0;
}

void dayToDate( int year, int year_day, int *month, int *date ) {
	cerr << "isLeapYear(year)=" << isLeapYear(year) << endl;
	int *m = isLeapYear(year) ? leapMonths : months;
	cerr << "isLeapYear(year)=" << isLeapYear(year) << endl;
	int i = 1;
	while( m[i] <= year_day )
		i++;
	*month = i-1;
	*date = year_day - m[i-1];
}
*/
