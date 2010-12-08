/*-------------------------------------------------------------------------
	b0.cpp
-------------------------------------------------------------------------*/
#include <iostream>
#include <string>

using namespace std;

#include <c_lib.hpp>
#include <y_util.hpp>

string fixed_part_title = \
"---------------------------------------------------------------------------\n\
    Фиксированная часть\n\
---------------------------------------------------------------------------\n";

string specific_part_title = \
"---------------------------------------------------------------------------\n\
    Специфическая часть\n\
---------------------------------------------------------------------------\n";

bool is_b0_0 = false;
string b0_0[] = { "b0_0   ", "0    тип формата                                    " };
bool is_b0_1 = false;
string b0_1[] = { "b0_1   ", "1    имя спутника                                   " };
bool is_b0_14 = false;
string b0_14[] = { "b0_14  ", "14   идентификатор спутника                         " };
bool is_b0_18 = false;
string b0_18[] = { "b0_18  ", "18   номер витка                                    " };
bool is_b0_22 = false;
string b0_22[] = { "b0_22  ", "22   год                                            " };
bool is_b0_24 = false;
string b0_24[] = { "b0_24  ", "24   день в году                                    " };
bool is_b0_24b = false;
string b0_24b[] = { "b0_24b ", "24b  дата                                           " };
bool is_b0_26 = false;
string b0_26[] = { "b0_26  ", "26   время дня (мсек,UTC)                           " };
bool is_b0_26b = false;
string b0_26b[] = { "b0_26b ", "26b  время дня (UTC)                                " };
bool is_b0_62 = false;
string b0_62[] = { "b0_62  ", "62   тип данных [1]                                 " };
bool is_b0_63 = false;
string b0_63[] = { "b0_63  ", "63   тип данных [2]                                 " };

bool is_bh_64 = false;
string bh_64[] = { "bh_64  ", "64   количество принятых кадров                     " };
bool is_bh_66 = false;
string bh_66[] = { "bh_66  ", "66   количество кадров со сбоем синхронизации       " };
bool is_bh_68 = false;
string bh_68[] = { "bh_68  ", "68   количество кадров без сбоев по времени         " };
bool is_bh_70 = false;
string bh_70[] = { "bh_70  ", "70   количество кадров с исправленным временем      " };
bool is_bh_72 = false;
string bh_72[] = { "bh_72  ", "72   количество пропусков данных                    " };
bool is_bh_74 = false;
string bh_74[] = { "bh_74  ", "74   тип упаковки                                   " };
bool is_bh_76 = false;
string bh_76[] = { "bh_76  ", "76   полная длина строки в пикселах                 " };
bool is_bh_78 = false;
string bh_78[] = { "bh_78  ", "78   маска кадра                                    " };
bool is_bh_82 = false;
string bh_82[] = { "bh_82  ", "82   количество пропущенных пикселов                " };
bool is_bh_84 = false;
string bh_84[] = { "bh_84  ", "84   количество принятых пикселов                   " };
bool is_bh_86 = false;
string bh_86[] = { "bh_86  ", "86   тип витка (0 - нисходящий, 1 - восходящий)     " };

bool is_ba_64 = false;
string ba_64[] = { "ba_64  ", "64   стадия обработки данных                        " };
bool is_ba_68 = false;
string ba_68[] = { "ba_68  ", "68   номер канала                                   " };
bool is_ba_70 = false;
string ba_70[] = { "ba_70  ", "70   количество строк                               " };
bool is_ba_72 = false;
string ba_72[] = { "ba_72  ", "72   полная длина строки в пикселах                 " };
bool is_ba_74 = false;
string ba_74[] = { "ba_74  ", "74   количество пропущенных пикселов                " };
bool is_ba_76 = false;
string ba_76[] = { "ba_76  ", "76   количество принятых пикселов                   " };
bool is_ba_78 = false;
string ba_78[] = { "ba_78  ", "78   тип витка (0 - нисходящий, 1 - восходящий)     " };
bool is_ba_80 = false;
string ba_80[] = { "ba_80  ", "80   максимальное значение среди значимых пикселов  " };
bool is_ba_82 = false;
string ba_82[] = { "ba_82  ", "82   коэффициент пересчёта A                        " };
bool is_ba_90 = false;
string ba_90[] = { "ba_90  ", "90   коэффициент пересчёта B                        " };

bool is_bp_64 = false;
string bp_64[] = { "bp_64  ", "64   стадия обработки данных                        " };
bool is_bp_68 = false;
string bp_68[] = { "bp_68  ", "68   номер канала                                   " };
bool is_bp_70 = false;
string bp_70[] = { "bp_70  ", "70   максимальное значение среди значимых пикселов  " };
bool is_bp_72 = false;
string bp_72[] = { "bp_72  ", "72   тип проекции                                   " };
bool is_bp_74 = false;
string bp_74[] = { "bp_74  ", "74   количество строк                               " };
bool is_bp_76 = false;
string bp_76[] = { "bp_76  ", "76   количество пикселов в строке                   " };
bool is_bp_78 = false;
string bp_78[] = { "bp_78  ", "78   широта (град)                                  " };
bool is_bp_82 = false;
string bp_82[] = { "bp_82  ", "82   долгота (град)                                 " };
bool is_bp_86 = false;
string bp_86[] = { "bp_86  ", "86   размер по широте (град)                        " };
bool is_bp_90 = false;
string bp_90[] = { "bp_90  ", "90   размер по долготе (град)                       " };
bool is_bp_94 = false;
string bp_94[] = { "bp_94  ", "94   шаг по широте (сек)                            " };
bool is_bp_98 = false;
string bp_98[] = { "bp_98  ", "98   шаг по долготе (сек)                           " };
bool is_bp_102 = false;
string bp_102[] = { "bp_102 ", "102  коэффициент пересчёта A                        " };
bool is_bp_110 = false;
string bp_110[] = { "bp_110 ", "110  коэффициент пересчёта B                        " };

bool is_bt_64 = false;
string bt_64[] = { "bt_64  ", "64   количество строк                               " };
bool is_bt_66 = false;
string bt_66[] = { "bt_66  ", "66   номер канала                                   " };

bool is_bn_128 = false;
string bn_128[] = { "bn_128 ", "128  номер опорного витка                           " };
bool is_bn_132 = false;
string bn_132[] = { "bn_132 ", "132  номер набора                                   " };
bool is_bn_134 = false;
string bn_134[] = { "bn_134 ", "134  тип эфемерид                                   " };
bool is_bn_136 = false;
string bn_136[] = { "bn_136 ", "136  год                                            " };
bool is_bn_138 = false;
string bn_138[] = { "bn_138 ", "138  полное время в году                            " };
bool is_bn_146 = false;
string bn_146[] = { "bn_146 ", "146  среднее движение (рад/мин)                     " };
bool is_bn_154 = false;
string bn_154[] = { "bn_154 ", "154  BSTAR                                          " };
bool is_bn_162 = false;
string bn_162[] = { "bn_162 ", "162  наклонение орбиты (рад)                        " };
bool is_bn_170 = false;
string bn_170[] = { "bn_170 ", "170  прямое восхождение (рад)                       " };
bool is_bn_178 = false;
string bn_178[] = { "bn_178 ", "178  эксцентриситет                                 " };
bool is_bn_186 = false;
string bn_186[] = { "bn_186 ", "186  аргумент перигея (рад)                         " };
bool is_bn_194 = false;
string bn_194[] = { "bn_194 ", "194  средняя аномалия (рад)                         " };

bool is_bc_256 = false;
string bc_256[] = { "bc_256 ", "256  номер версии коррекции                         " };
bool is_bc_258 = false;
string bc_258[] = { "bc_258 ", "258  поправка времени по TBUS (мсек)                " };
bool is_bc_260 = false;
string bc_260[] = { "bc_260 ", "260  поправка времени (мсек)                        " };
bool is_bc_262 = false;
string bc_262[] = { "bc_262 ", "262  крен (roll)                                    " };
bool is_bc_270 = false;
string bc_270[] = { "bc_270 ", "270  тангаж (pitch)                                 " };
bool is_bc_278 = false;
string bc_278[] = { "bc_278 ", "278  рысканье (yaw)                                 " };

int bIdx = 0;
bool is_all = false;

void print_fixed_part( TBlk0* b );
void print_specific_part( TBlk0* b );
void print_old_format( TOldBlk0* b );
void print_blk0HRPT( TBlk0_HRPT* b );
void print_blk0AVHRR( TBlk0_AVHRR* b );
void print_blk0proj( TBlk0_Proj* b );
void print_blk0TLM( TBlk0_TLM* a );
void print_norad( void* blk0_specific_part );
void print_georef_correction( void* blk0_specific_part );
void print_usage( );
void parseCommandString( int argc, char* argv[] ) throw ( TException);

//int isLeapYear( int );
//void dayToDate( int, int, int *, int * );

int main( int argc, char** argv ){
	if( argc < 2 ){
		print_usage( );
		return 1;
	}

	try{
		parseCommandString( argc, argv );

	}catch( TException e ){
		cerr << e.text( ) << endl;
		exit( -1 );
	}

	char buff[512];
	TBlk0* blk0 = (TBlk0*)buff;
	TOldBlk0 *oblk0 = (TOldBlk0 *)buff;

	FILE *file = fopen( argv[argc - 1], "rb" );
	if( file == NULL || fread( buff, 1, 512, file ) != 512 ){
		cerr << "Ошибка доступа к файлу " << argv[argc - 1] << endl;
		fclose( file );
		return -1;
	}
	fclose( file );

	if( bIdx ) cout << "Файл: " << argv[argc - 1] << endl << endl;
	if( blk0->b0.formatType == 0xff ){ // файл нового формата
		if( bIdx ) cout << fixed_part_title;
		print_fixed_part( blk0 );
		if( bIdx ) cout << endl << endl << specific_part_title;
		print_specific_part( blk0 );
	}else{ // файл старого формата
		print_old_format( oblk0 );
	}
	return 0;
}

void print_usage( ){
	cout << "Универсальная программа просмотра содержимого 0-блока файлов данных." << endl;
	cout << "Использование:" << endl;
	cout << "b0info [-a|<ключ>] [-v] <файл>" << endl;
	cout << "\t" << "-v      информативный вывод полей" << endl;
	cout << "\t" << "-a      вывод всех полей" << endl;
	cout << "\t" << "ключи" << endl;
	cout << "\t" << "общая часть" << endl;
	cout << "\t" << "-b0_0   тип формата" << endl;
	cout << "\t" << "-b0_1   имя спутника" << endl;
	cout << "\t" << "-b0_14  идентификатор спутника" << endl;
	cout << "\t" << "-b0_18  номер витка" << endl;
	cout << "\t" << "-b0_22  год" << endl;
	cout << "\t" << "-b0_24  день в году" << endl;
	cout << "\t" << "-b0_24b дата" << endl;
	cout << "\t" << "-b0_26  время дня (мсек,UTC)" << endl;
	cout << "\t" << "-b0_26b время дня (UTC)" << endl;
	cout << "\t" << "-b0_62  тип данных [1]" << endl;
	cout << "\t" << "-b0_63  тип данных [2]" << endl;

	cout << "\t" << "Исходный файл HRPT" << endl;
	cout << "\t" << "-bh_64  количество принятых кадров" << endl;
	cout << "\t" << "-bh_66  количество кадров со сбоем синхронизации" << endl;
	cout << "\t" << "-bh_68  количество кадров без сбоев по времени" << endl;
	cout << "\t" << "-bh_70  количество кадров с исправленным временем" << endl;
	cout << "\t" << "-bh_72  количество пропусков данных" << endl;
	cout << "\t" << "-bh_74  тип упаковки" << endl;
	cout << "\t" << "-bh_76  полная длина строки в пикселах" << endl;
	cout << "\t" << "-bh_78  маска кадра" << endl;
	cout << "\t" << "-bh_82  количество пропущенных пикселов" << endl;
	cout << "\t" << "-bh_84  количество принятых пикселов" << endl;
	cout << "\t" << "-bh_86  тип витка (0 - нисходящий, 1 - восходящий)" << endl;

	cout << "\t" << "Рапакованные данные канала AVHRR" << endl;
	cout << "\t" << "-ba_64  стадия обработки данных" << endl;
	cout << "\t" << "-ba_68  номер канала" << endl;
	cout << "\t" << "-ba_70  количество строк" << endl;
	cout << "\t" << "-ba_72  полная длина строки в пикселах" << endl;
	cout << "\t" << "-ba_74  количество пропущенных пикселов" << endl;
	cout << "\t" << "-ba_76  количество принятых пикселов" << endl;
	cout << "\t" << "-ba_78  тип витка (0 - нисходящий, 1 - восходящий)" << endl;
	cout << "\t" << "-ba_80  максимальное значение среди значимых пикселов" << endl;
	cout << "\t" << "-ba_82  коэффициент пересчёта A" << endl;
	cout << "\t" << "-ba_90  коэффициент пересчёта B" << endl;

	cout << "\t" << "Файл проекции" << endl;
	cout << "\t" << "-bp_64  стадия обработки данных" << endl;
	cout << "\t" << "-bp_68  номер канала" << endl;
	cout << "\t" << "-bp_70  максимальное значение среди значимых пикселов" << endl;
	cout << "\t" << "-bp_72  тип проекции" << endl;
	cout << "\t" << "-bp_74  количество строк" << endl;
	cout << "\t" << "-bp_76  количество пикселов в строке" << endl;
	cout << "\t" << "-bp_78  широта (град)" << endl;
	cout << "\t" << "-bp_82  долгота (град)" << endl;
	cout << "\t" << "-bp_86  размер по широте (град)" << endl;
	cout << "\t" << "-bp_90  размер по долготе (град)" << endl;
	cout << "\t" << "-bp_94  шаг по широте (сек)" << endl;
	cout << "\t" << "-bp_98  шаг по долготе (сек)" << endl;
	cout << "\t" << "-bp_102 коэффициент пересчёта A" << endl;
	cout << "\t" << "-bp_110 коэффициент пересчёта B" << endl;

	cout << "\t" << "Данные телеметрии" << endl;
	cout << "\t" << "-bt_64  количество строк" << endl;
	cout << "\t" << "-bt_66  номер канала" << endl;

	cout << "\t" << "Телеграмма NORAD" << endl;
	cout << "\t" << "-bn_128 номер опорного витка" << endl;
	cout << "\t" << "-bn_132 номер набора" << endl;
	cout << "\t" << "-bn_134 тип эфемерид" << endl;
	cout << "\t" << "-bn_136 год" << endl;
	cout << "\t" << "-bn_138 полное время в году" << endl;
	cout << "\t" << "-bn_146 среднее движение (рад/мин)" << endl;
	cout << "\t" << "-bn_154 BSTAR" << endl;
	cout << "\t" << "-bn_162 наклонение орбиты (рад)" << endl;
	cout << "\t" << "-bn_170 прямое восхождение (рад)" << endl;
	cout << "\t" << "-bn_178 эксцентриситет" << endl;
	cout << "\t" << "-bn_186 аргумент перигея (рад)" << endl;
	cout << "\t" << "-bn_194 средняя аномалия (рад)" << endl;

	cout << "\t" << "Параметры коррекции географической привязки" << endl;
	cout << "\t" << "-bc_256 номер версии коррекции" << endl;
	cout << "\t" << "-bc_258 поправка времени по TBUS (мсек)" << endl;
	cout << "\t" << "-bc_260 поправка времени (мсек)" << endl;
	cout << "\t" << "-bc_262 крен (roll)" << endl;
	cout << "\t" << "-bc_270 тангаж (pitch)" << endl;
	cout << "\t" << "-bc_278 рысканье (yaw)" << endl;

}

void parseCommandString( int argc, char* argv[] ) throw ( TException){
	string msg;
	int i = 1;
	while( i < argc ){
		char * s = argv[i++];
		if( *s == '-' ){
			s++;
			if( *s == 'a' ){
				is_all = true;
			}else if( *s == 'v' ){
				bIdx = 1;
			}else if( *s == 'b' ){
				s++;
				if( strchr( "0haptnc", *s ) != NULL && *(s + 1) == '_' ){
					char * t;
					long int v = strtol( s + 2, &t, 10 );
					if( *t != '\0' && !( *t == 'b' && *(t + 1) == '\0' ) ){
						msg.assign( "Неизвестный параметр -b" ).append( s );
						throw TException( 100, msg.c_str( ) );
					}

					switch( *s ){
						case '0':
							switch( v ){
								case 0:
									is_b0_0 = true;
									break;
								case 1:
									is_b0_1 = true;
									break;
								case 14:
									is_b0_14 = true;
									break;
								case 18:
									is_b0_18 = true;
									break;
								case 22:
									is_b0_22 = true;
									break;
								case 24:
									if( *t == '\0' ) is_b0_24 = true;
									else is_b0_24b = true;
									break;
								case 26:
									if( *t == '\0' ) is_b0_26 = true;
									else is_b0_26b = true;
									break;
								case 62:
									is_b0_62 = true;
									break;
								case 63:
									is_b0_63 = true;
									break;
								default:
									msg.assign( "Неизвестный параметр -b" ).append( s );
									throw TException( 100, msg.c_str( ) );
							}
							break;
						case 'h':
							switch( v ){
								case 64:
									is_bh_64 = true;
									break;
								case 66:
									is_bh_66 = true;
									break;
								case 68:
									is_bh_68 = true;
									break;
								case 70:
									is_bh_70 = true;
									break;
								case 72:
									is_bh_72 = true;
									break;
								case 74:
									is_bh_74 = true;
									break;
								case 76:
									is_bh_76 = true;
									break;
								case 78:
									is_bh_78 = true;
									break;
								case 82:
									is_bh_82 = true;
									break;
								case 84:
									is_bh_84 = true;
									break;
								case 86:
									is_bh_86 = true;
									break;
								default:
									msg.assign( "Неизвестный параметр -b" ).append( s );
									throw TException( 100, msg.c_str( ) );
							}
							break;
						case 'a':
							switch( v ){
								case 64:
									is_ba_64 = true;
									break;
								case 68:
									is_ba_68 = true;
									break;
								case 70:
									is_ba_70 = true;
									break;
								case 72:
									is_ba_72 = true;
									break;
								case 74:
									is_ba_74 = true;
									break;
								case 76:
									is_ba_76 = true;
									break;
								case 78:
									is_ba_78 = true;
									break;
								case 80:
									is_ba_80 = true;
									break;
								case 82:
									is_ba_82 = true;
									break;
								case 90:
									is_ba_90 = true;
									break;
								default:
									msg.assign( "Неизвестный параметр -b" ).append( s );
									throw TException( 100, msg.c_str( ) );
							}
							break;
						case 'p':
							switch( v ){
								case 64:
									is_bp_64 = true;
									break;
								case 68:
									is_bp_68 = true;
									break;
								case 70:
									is_bp_70 = true;
									break;
								case 72:
									is_bp_72 = true;
									break;
								case 74:
									is_bp_74 = true;
									break;
								case 76:
									is_bp_76 = true;
									break;
								case 78:
									is_bp_78 = true;
									break;
								case 82:
									is_bp_82 = true;
									break;
								case 86:
									is_bp_86 = true;
									break;
								case 90:
									is_bp_90 = true;
									break;
								case 94:
									is_bp_94 = true;
									break;
								case 98:
									is_bp_98 = true;
									break;
								case 102:
									is_bp_102 = true;
									break;
								case 110:
									is_bp_110 = true;
									break;
								default:
									msg.assign( "Неизвестный параметр -b" ).append( s );
									throw TException( 100, msg.c_str( ) );
							}
							break;
						case 't':
							switch( v ){
								case 64:
									is_bt_64 = true;
									break;
								case 66:
									is_bt_66 = true;
									break;
								default:
									msg.assign( "Неизвестный параметр -b" ).append( s );
									throw TException( 100, msg.c_str( ) );
							}
							break;
						case 'n':
							switch( v ){
								case 128:
									is_bn_128 = true;
									break;
								case 132:
									is_bn_132 = true;
									break;
								case 134:
									is_bn_134 = true;
									break;
								case 136:
									is_bn_136 = true;
									break;
								case 138:
									is_bn_138 = true;
									break;
								case 146:
									is_bn_146 = true;
									break;
								case 154:
									is_bn_154 = true;
									break;
								case 162:
									is_bn_162 = true;
									break;
								case 170:
									is_bn_170 = true;
									break;
								case 178:
									is_bn_178 = true;
									break;
								case 186:
									is_bn_186 = true;
									break;
								case 194:
									is_bn_194 = true;
									break;
								default:
									msg.assign( "Неизвестный параметр -b" ).append( s );
									throw TException( 100, msg.c_str( ) );
							}
							break;
						case 'c':
							switch( v ){
								case 256:
									is_bc_256 = true;
									break;
								case 258:
									is_bc_258 = true;
									break;
								case 260:
									is_bc_260 = true;
									break;
								case 262:
									is_bc_262 = true;
									break;
								case 270:
									is_bc_270 = true;
									break;
								case 278:
									is_bc_278 = true;
									break;
								default:
									msg.assign( "Неизвестный параметр -b" ).append( s );
									throw TException( 100, msg.c_str( ) );
							}
							break;
					}
				}else{
					msg.assign( "Неизвестный параметр -b" ).append( s );
					throw TException( 100, msg.c_str( ) );
				}
			}else{
				msg.assign( "Неизвестный параметр -" ).append( s );
				throw TException( 100, msg.c_str( ) );
			}
		}
	}
}

void print_fixed_part( TBlk0* blk0fs ){
	if( is_all || is_b0_0 ) cout << b0_0[bIdx] << hex << (unsigned)blk0fs->b0.formatType << dec << endl;
	if( is_all || is_b0_1 ) cout << b0_1[bIdx] << blk0fs->b0.satName << endl;
	if( is_all || is_b0_14 ) cout << b0_14[bIdx] << blk0fs->b0.satId << " (" << hex << blk0fs->b0.satId << dec << ")" << endl;
	if( is_all || is_b0_18 ) cout << b0_18[bIdx] << blk0fs->b0.revNum << endl;
	if( is_all || is_b0_22 ) cout << b0_22[bIdx] << blk0fs->b0.year << endl;
	if( is_all || is_b0_24 ) cout << b0_24[bIdx] << blk0fs->b0.day << endl;
	if( is_all || is_b0_24b ){
		int month = 0;
		int day = 0;
		dayToDate1( blk0fs->b0.year, blk0fs->b0.day, &month, &day );
		cout << b0_24b[bIdx] << blk0fs->b0.year << "-" << (month < 10 ? "0" : "") << month << "-" << (day < 10 ? "0" : "") << day << endl;
	}
	if( is_all || is_b0_26 ) cout << b0_26[bIdx] << blk0fs->b0.dayTime << endl;

	if( is_all || is_b0_26b ){
		long hh, mm, ss;
		long ts = blk0fs->b0.dayTime / 1000;
		ss = ts % 60;
		ts = ts / 60;
		mm = ts % 60;
		hh = ts / 60;
		cout << b0_26b[bIdx] << (hh < 10 ? "0" : "") << hh << ":" << (mm < 10 ? "0" : "") << mm << ":" << (ss < 10 ? "0" : "") << ss << endl;
	}

	if( is_all || is_b0_62 ) cout << b0_62[bIdx] << (unsigned)blk0fs->b0.dataType1;
	if( bIdx ) switch( blk0fs->b0.dataType1 ){
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
	cout << endl;

	if( is_all || is_b0_63 ) cout << b0_63[bIdx] << (unsigned)blk0fs->b0.dataType2;
	if( bIdx ) switch( blk0fs->b0.dataType1 ){
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
	cout << endl;
}

//char blk0[512];
//TBlk0* blk0fs = (TBlk0*)blk0;
//TBlk0_HRPT* blk0HRPT = (TBlk0_HRPT*)blk0;
//TBlk0_AVHRR* blk0AVHRR = (TBlk0_AVHRR*)blk0;
//TBlk0_Proj* blk0Proj = (TBlk0_Proj*)blk0;
//TBlk0_TLM* blk0TLM = (TBlk0_TLM*)blk0;
//TOldBlk0 *oblk0 = (TOldBlk0 *)blk0;

void print_specific_part( TBlk0* blk0 ){
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

void print_blk0HRPT( TBlk0_HRPT* b ){
	if( is_all || is_bh_64 ) cout << bh_64[bIdx] << b->frameNum << endl;
	if( is_all || is_bh_66 ) cout << bh_66[bIdx] << b->lostFrameNum << endl;
	if( is_all || is_bh_68 ) cout << bh_68[bIdx] << b->validTimeFrameNum << endl;
	if( is_all || is_bh_70 ) cout << bh_70[bIdx] << b->correctedTimeFrameNum << endl;
	if( is_all || is_bh_72 ) cout << bh_72[bIdx] << b->gapNum << endl;
	if( is_all || is_bh_74 ) cout << bh_74[bIdx] << b->packType << endl;
	if( is_all || is_bh_76 ) cout << bh_76[bIdx] << b->totalPixNum << endl;
	if( is_all || is_bh_78 ) cout << bh_78[bIdx] << hex << b->frameMask << dec << endl;
	if( is_all || is_bh_82 ) cout << bh_82[bIdx] << b->pixGap << endl;
	if( is_all || is_bh_84 ) cout << bh_84[bIdx] << b->pixNum << endl;
	if( is_all || is_bh_86 ) cout << bh_86[bIdx] << b->ascendFlag << endl;
	print_norad( b );
	print_georef_correction( b );
}

void print_blk0AVHRR( TBlk0_AVHRR * b ){
	if( is_all || is_ba_64 ) cout << ba_64[bIdx] << hex << b->processLevel << dec << endl;
	if( is_all || is_ba_68 ) cout << ba_68[bIdx] << b->channel << endl;
	if( is_all || is_ba_70 ) cout << ba_70[bIdx] << b->totalFrameNum << endl;
	if( is_all || is_ba_72 ) cout << ba_72[bIdx] << b->totalPixNum << endl;
	if( is_all || is_ba_74 ) cout << ba_74[bIdx] << b->pixGap << endl;
	if( is_all || is_ba_76 ) cout << ba_76[bIdx] << b->pixNum << endl;
	if( is_all || is_ba_78 ) cout << ba_78[bIdx] << b->ascendFlag << endl;
	if( is_all || is_ba_80 ) cout << ba_80[bIdx] << b->maxPixelValue << endl;
	if( is_all || is_ba_82 ) cout << ba_82[bIdx] << b->ka << endl;
	if( is_all || is_ba_90 ) cout << ba_90[bIdx] << b->kb << endl;
	print_norad( b );
	print_georef_correction( b );
}

void print_blk0proj( TBlk0_Proj * b ){
	if( is_all || is_bp_64 ) cout << bp_64[bIdx] << hex << b->processLevel << dec << endl;
	if( is_all || is_bp_68 ) cout << bp_68[bIdx] << b->channel << endl;
	if( is_all || is_bp_70 ) cout << bp_70[bIdx] << b->maxPixelValue << endl;
	if( is_all || is_bp_72 ){
		cout << bp_72[bIdx] << b->projType;
		if( bIdx ) cout << " (" << (b->projType == 1 ? "меркаторская" : "равнопромежуточная") << ")" << endl;
	}
	if( is_all || is_bp_74 ) cout << bp_74[bIdx] << b->scanNum << endl;
	if( is_all || is_bp_76 ) cout << bp_76[bIdx] << b->pixNum << endl;
	if( is_all || is_bp_78 ) cout << bp_78[bIdx] << b->lat << endl;
	if( is_all || is_bp_82 ) cout << bp_82[bIdx] << b->lon << endl;
	if( is_all || is_bp_86 ) cout << bp_86[bIdx] << b->latSize << endl;
	if( is_all || is_bp_90 ) cout << bp_90[bIdx] << b->lonSize << endl;
	if( is_all || is_bp_94 ) cout << bp_94[bIdx] << b->latRes << endl;
	if( is_all || is_bp_98 ) cout << bp_98[bIdx] << b->lonRes << endl;
	if( is_all || is_bp_102 ) cout << bp_102[bIdx] << b->ka << endl;
	if( is_all || is_bp_110 ) cout << bp_110[bIdx] << b->kb << endl;
	print_norad( b );
	print_georef_correction( b );
}

void print_blk0TLM( TBlk0_TLM * a ){
	if( is_all || is_bt_64 ) cout << bt_64[bIdx] << a->totalFrameNum << endl;
	if( is_all || is_bt_66 ) cout << bt_66[bIdx] << a->channel << endl;
}

void print_norad( void *blk0_specific_part ){
	TBlk0_HRPT *b = (TBlk0_HRPT *)blk0_specific_part;
	if( bIdx ) cout << endl << "Телеграмма NORAD:" << endl;
	if( is_all || is_bn_128 ) cout << bn_128[bIdx] << b->revNum << endl;
	if( is_all || is_bn_132 ) cout << bn_132[bIdx] << b->setNum << endl;
	if( is_all || is_bn_134 ) cout << bn_134[bIdx] << b->ephemType << endl;
	if( is_all || is_bn_136 ) cout << bn_136[bIdx] << b->year << endl;
	if( is_all || is_bn_138 ) cout << bn_138[bIdx] << b->yearTime << endl;
	if( is_all || is_bn_146 ) cout << bn_146[bIdx] << b->n0 << endl;
	if( is_all || is_bn_154 ) cout << bn_154[bIdx] << b->bstar << endl;
	if( is_all || is_bn_162 ) cout << bn_162[bIdx] << b->i0 << endl;
	if( is_all || is_bn_170 ) cout << bn_170[bIdx] << b->raan << endl;
	if( is_all || is_bn_178 ) cout << bn_178[bIdx] << b->e0 << endl;
	if( is_all || is_bn_186 ) cout << bn_186[bIdx] << b->w0 << endl;
	if( is_all || is_bn_194 ) cout << bn_194[bIdx] << b->m0 << endl;
}

void print_georef_correction( void *blk0_specific_part ){
	TBlk0_HRPT *b = (TBlk0_HRPT *)blk0_specific_part;
	if( bIdx ) cout << endl << "Параметры коррекции географической привязки:" << endl;
	if( is_all || is_bc_256 ) cout << bc_256[bIdx] << b->corVersion << endl;
	if( is_all || is_bc_258 ) cout << bc_258[bIdx] << b->corTBUSTime << endl;
	if( is_all || is_bc_260 ) cout << bc_260[bIdx] << b->corTime << endl;
	if( is_all || is_bc_262 ) cout << bc_262[bIdx] << b->corRoll << endl;
	if( is_all || is_bc_270 ) cout << bc_270[bIdx] << b->corPitch << endl;
	if( is_all || is_bc_278 ) cout << bc_278[bIdx] << b->corYaw << endl;
}

void print_old_format( TOldBlk0 *b ){
	unsigned long sat_num = ((unsigned long)b->satNumber >> 1) & 0x3f;
	string file_types[8] = { "NOAA APT", "NOAA HRPT", "распакованный HRPT (2 байта на пиксел)",
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
	for( int i = 0; i < 20; i++ ) cout << 310 + i * sizeof (float) <<
		"                                       " << b->polyCoeffs[i] << endl;

}
