/*-----------------------------------------------------------------------------
 calibr.cpp
------------------------------------------------------------------------------*/
#include <tc_config.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <orbmodel.hpp>
#include <c_lib.hpp>
#include <Log.hpp>

#include <xml.hpp>
#include <auto_ptr.hpp>
#include <astronom.hpp>

#ifndef LOG_FORMAT
#define LOG_FORMAT "%d %t %e %m"
#endif

using namespace libconfig;
const char* useMsg = "\
Использование: calibr [опции] имя_конфигурации файл_данных\n\
файл_данных     Файл распакованных данных HRPT.\n\
Опции:\n\
-a[1]            Делить значения альбедо на косинус угла восхождения на Солнце.\n\
-a0              Не делить значения альбедо на косинус угла восхождения на Солнце.\n\
Опции, управляющие печатью сообщений в log-файл:\n\
-llevel={dump,debug,info,warning,error,fatal} уровень вывода сообщений.\n\
-lerr           Выводить сообщения в стандартный поток ошибок.\n\
-lout           Выводить сообщения в стандартный поток вывода.\n\
-lfile=<name>   Выводить сообщения в файл с именем name,\n\
                файл создаётся заново\n\
-lappend=<name> Выводить сообщения в файл с именем name,\n\
                сообщения дописываются в конец файла\n";


// Значение, которым мы будем отмечать пустые пиксели
const short lost_value = -3;

// Значение переменной  среды TERM_ROOT
const char * term_root = ".";
/*
	Опции из коммандной строки имеют более высокий приоритет чем опции из файла конфигураций.
*/

nsCLog::eSeverity logLevel = nsCLog::info;
bool useStdErr = false;
bool useStdOut = false;
string logFileName = string();
bool append = false;
CLog *logfile;

// деление значений альбедо на косинус угла восхождения на солнце
bool option_ascent;

char exeFileName[MAX_PATH];    // argv[0]
char cfgName[MAX_PATH];
//char cfgFileName[MAX_PATH];

char inputFileName[MAX_PATH];          // "nxxxxxx_C.avh"
char telemetryFileName[MAX_PATH];      // "nxxxxxx_C.tlm"
char outputFileName[MAX_PATH];         // "nxxxxxx_C.clb"

TBlk0_AVHRR in_blk0;
TBlk0_AVHRR out_blk0;

int satId;
uint32_t totalFrameNum;

// Минимальное значение синуса  угла восхождения на солнце
// при котором данным видимых каналов можно доверять
const double sinalpha_minlimit=0.0001*DR;
const short minLimitOutPoint = -12;         // Значения, которым которым будем отсекать пиксели, значения альбедо которых выходит за границы [0,100] v1.5
const short maxLimitOutPoint = -13;

const int INCNT = 1024;  // Количество значений принимаемых элементом исходного файла
const double KELVIN0 = 273.16;   // 0°C
const double MINT = -250.;       // Диапазон обрабатываемых
const double MAXT =  150.;       //        температур
const double delta_t = 0.1;   // Шаг температур в выходном файле
// длина диапазона температур во внутренних единицах измерения
const int NUMBERST = 4010;            // по крайней мере равен ( MAXT - MINT ) / DELTAT + 1

// параметры представления альбедо для обычных спутников
const double delta_alb = 0.1;     // Шаг альбедо в выходном файле
const double alb0 = 0.0;          // Минимальное значение альбедо в выходном файле
const int max_albedo_value = 1000;  // Максимальное возможное значение альбедо

// параметры представления альбедо для NOAA KLM
const double delta_alb_klm = 0.05;    // Шаг альбедо в выходном файле
const double alb0_klm = 0.0;          // Минимальное значение альбедо в выходном файле
const int max_albedo_value_klm = 2000;   // Максимальное возможное значение альбедо

#define LTEL 30                   // длина записи файла телеметрии (в словах)

// Таблица показаний PRT
const int NPRT = 4;         // Количество PRT-ов
const int NAA = 5;          // Количество параметров Ai для преобразования PRTi -> °K

// Normalized response functions и параметры ее задания
const int NRF_LENGTH = 60;             // Количество чисел в таблице

const int NNCOF = 6;         // количество параметров нелинейной коррекции
// Параметры нелинейной коррекции
struct TCorrParams {
    // способ нелинейной коррекции
    // 0 --- нелинейная коррекция для старых спутников (коррекция температуры)
    // 1 --- нелинейная коррекция для новых спутников (коррекция радиации)
    int new_calibr_flag;
    double table[NNCOF];    // Параметры нелинейной коррекции

    // Данные NLTBL текущего спутника (если присутствуют)
    int tbl_flag;                // если 1 --- присутствуют данные NLTBL
    double tbl_itt[10];
    int tbl_itt_length;
    double tbl_st[20];
    int tbl_st_length;
    double tbl_data[200];
    int tbl_data_length;
};

struct TInputParams {
    int sat;
    int chan;
    int chanAB;
    XML *pxml;
};

struct TAlbedoCalParams {
    double slope_value;
    double intercept_value;
    double slope_value2;
    double intercept_value2;
    char used_additional_values;
};

// Результат разбора файла телеметрии
struct TTelemetryData {
    short prt[NPRT];    // показания PRT
    short space_value;
    short target_value;
};

struct T_NFR {
    double table[NRF_LENGTH];           // Таблица значений NRF
    double starting_wave;         // Параметры задания
    double increment_wave;        //      таблицы
};

#include "calibr.hpp"

char msg[64000];                      // Буфер для сообщений лога

int main(int argc, char *argv[]) {

    if ( argc < 3 ) {
        printf( "%s", useMsg );
        return 0;
    }

    char * t;
    if (NULL!=(t = getenv("TERM_ROOT"))) {
        term_root = strdup(t);
    } else {
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

    logfile = new CLog("calibr.exe");
    if (useStdOut) {
        //logfile.addAppender(new ConsoleLogAppender(logLevel,"%s | %e | %m"));
        //logfile->addAppender(new CConsoleLogAppender(logLevel, "%e | %m"));
        logfile->addAppender(new CConsoleLogAppender(logLevel, LOG_FORMAT));
    }
    if (useStdErr) {
        //logfile.addAppender(new EConsoleLogAppender(logLevel,"%s | %e | %m"));
        //logfile->addAppender(new EConsoleLogAppender(logLevel, "%e | %m"));
        logfile->addAppender(new EConsoleLogAppender(logLevel, LOG_FORMAT));
    }
    if (!logFileName.empty()) {
        try {
            //logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, "%s | %e | %m"));
            logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, LOG_FORMAT));
        } catch (string e) {
            fprintf(stderr,"%s\n",e.c_str());
            exit(1);
        }
    }

    logfile->info( string("calibr.exe - ") + inputFileName );

    TAutoPtr<CLog> alogfile(logfile);

    construct_file_names();

    try {
        logfile->info( "чтение нулевого блока" );
        readBlk0();
        verifyBlk0(in_blk0, inputFileName);

        totalFrameNum = in_blk0.totalFrameNum;
        satId = in_blk0.b0.satId;

        logfile->info( "чтение файла настроек (calibr.dat)" );
        logfile->debug( "readCalibrDatFile begin" );
        XML* pxml;
        readCalibrDatFile( &pxml );
        logfile->debug( "readCalibrDatFile finished" );
        TAutoPtr<XML> xml(pxml);

        calibr_processing( in_blk0, inputFileName, telemetryFileName, outputFileName, *xml );
    } catch (...) {
        return 1;
    }
    logfile->info( "" );
    logfile->info( "работа программы завершена успешно !" );
    return 0;
}
/*
* конец функции main()
*/

void calibr_processing(
    TBlk0_AVHRR& in_blk0,
    const char * inputFileName,
    const char * telemetryFileName,
    const char * outputFileName,
    XML & xml ) throw (int)
{

    int sat =  in_blk0.b0.satId;            // номер спутника
    int chan = in_blk0.channel;          // номер канала
    int scans = in_blk0.totalFrameNum;   // общее число строк
    int cols = in_blk0.totalPixNum;      // количество пикселей в строке

    // Читаем исходный файл
    logfile->info("чтение файла данных");
    int blk0len = sizeof(TBlk0_AVHRR);
    int inp_data_len = scans*cols;
    int inp_data_file_len = inp_data_len*2;
    TAutoPtr<short> inpData(new short[inp_data_len]);
    readDataFile( inpData, inputFileName, inp_data_file_len, blk0len );

    {
        sprintf(msg, "номер спутника: %d", sat );
        logfile->debug(msg);
        sprintf(msg, "номер канала: %d", chan );
        logfile->debug(msg);
        sprintf(msg, "общее число строк: %d", scans );
        logfile->debug(msg);
        sprintf(msg, "количество пикселей в строке: %d", cols );
    }

    // Для инфракрасных каналов и, возможно, для 3A
    // считываем и разбираем файл телеметрии,
    // получаем target_value и space_value
    TTelemetryData telemetry;
    if (chan >= 3) {
        logfile->info( "чтение файла телеметрии" );
        int blk0len = sizeof(TBlk0_AVHRR);
        int tel_data_len = scans*LTEL;
        int tel_data_file_len = tel_data_len * 2;
        TAutoPtr<short> telData(new short[tel_data_len]);
        readDataFile( telData, telemetryFileName, tel_data_file_len, blk0len );
        telemetry_processing( telemetry, telData, scans, telemetryFileName );
    }

    int chanAB;       // Дополнительный номер канала. Введен для обработки каналов 3A, 3B.
    // 0 --- обычный инфракрасный третий канал
    // 1 --- 3A
    // 2 --- 3B
    if (chan == 3) {                  // Если имеем дело с третьим каналом, определяем значение chanAB
        char mask1[20];
        char *mask[] = {mask1, (char*)"CHAN 3"};


        sprintf( mask[0], "SAT %d", sat);

        try
        {
            string s = xml.get_text(2, mask);

            if (!s.empty())
            {                // Строка не пуста -> есть данные по обычному третьему каналу
                chanAB = 0;
                logfile->debug("обычный 3-ий канал.");
            }
            else
            {
                int dist = abs(telemetry.space_value - telemetry.target_value);
                chanAB = (dist < 10) ? 1 : 2;
                logfile -> debug( "результаты автоматического определения типа 3-го канала:" );
                logfile -> debug( chanAB == 1 ? "    3B" : "    3B" );
            }
        }
        catch (TException& e)
        {
            logfile->error( "ошибка разбора файла calibr.dat:" );
            logfile->error( e.text() );
            throw 1;
        }
    }

    logfile->debug("построение look_up_table");

    if (chan < 3 || (chan == 3 && chanAB == 1)) {   // для видимого канала
        sprintf( msg, "построение калибровочной таблицы для видимого (%d-го) канала", chan );
        logfile->info( msg );
        TAlbedoCalParams params;
        // извлекаем из файла кооф-ты альбедо
        albcof( sat, chan, xml, params );
        short look_up_table[1024];

        double ka;
        double kb;
        int maxV;
        tabalb( look_up_table, 1024, params, &ka, &kb, &maxV );   // и по ним строим look-up-table

        // создание нулевого блока для видимого канала
        out_blk0 = in_blk0;
        out_blk0.processLevel |= 0x01;        // взводим флаг калибровка проведена

        out_blk0.ka = ka;              // Параметры представления данных
        out_blk0.kb = kb;
        out_blk0.maxPixelValue = maxV;

        int out_data_len = inp_data_len;
        TAutoPtr<short> outData(new short[inp_data_len] );
        logfile->info( "преобразование данных" );
        processLUT( inpData, outData, scans*cols, look_up_table );
        logfile->info( "преобразование данных завершено" );
        if ( option_ascent ) {
            TStraightReferencer *r;
            double julian_date;
            navigationSystemInit( (TBlk0&)in_blk0, &r, &julian_date );
            proccesAscent( outData, scans, cols, *r, julian_date, out_blk0.ka, out_blk0.kb );
            delete r;
        }
        logfile->info( "сохранение результатов калибровки" );
        writeFileNOAA( outputFileName, out_blk0, outData, out_data_len );
        logfile->debug( "after writeFileNOAA" );
    } else {                       // для инфракрасных каналов
        sprintf( msg, "построение калибровочной таблицы для инфракрасного (%d-го) канала", chan );
        logfile->info( msg );
        TInputParams p;     // набор значения для получения
        p.sat = sat;        // параметров калибровки
        p.chan = chan;
        p.chanAB = chanAB;
        p.pxml = &xml;

        T_NFR nfr;
        ini_nfr( p, nfr );
        static double temp2rad[NUMBERST];
        ini_lut_T2R( nfr, temp2rad, MINT, delta_t, NUMBERST );
        double board_t;
        double radianse_target;
        calculateTarget( p, telemetry, nfr, &radianse_target, &board_t );
        TCorrParams corrParams;
        ini_corrParams( p, corrParams );
        double slope;       // параметры линейного преобразования
        double intercept;   //      значения -> температуры
        calcLinearConversionParams( radianse_target, telemetry, corrParams, &slope, &intercept );
        // Определяем параметры minT_pixel_value и maxT_pixel_value
        short min_pixel_value, max_pixel_value;     // минимальное и максимальное значения
        findMinMaxPositiveValue( inpData, scans*cols, &min_pixel_value, &max_pixel_value );
        // у инфракрасных каналов инверсия --- большие значения соответствуют меньшим температурам
        double minT_pixel_value = get_temp(max_pixel_value, slope, intercept, board_t, corrParams, temp2rad, MINT, delta_t, NUMBERST );
        while ( max_pixel_value >= 0 && minT_pixel_value == MINT ) {
            max_pixel_value--;
            minT_pixel_value =
                get_temp(max_pixel_value, slope, intercept, board_t, corrParams, temp2rad, MINT, delta_t, NUMBERST );
        }
        double maxT_pixel_value = get_temp(min_pixel_value, slope, intercept, board_t, corrParams, temp2rad, MINT, delta_t, NUMBERST );
        double t0 = floor(minT_pixel_value);

        {
            sprintf(msg, "минимальное исходное значение: %d", min_pixel_value);
            logfile->debug( msg );
            sprintf(msg, "максимальное исходное значение: %d", max_pixel_value);
            logfile->debug( msg );
            sprintf(msg, "минимальная температура: %lf", minT_pixel_value );
            logfile->debug( msg );
            sprintf(msg, "максимальное температура: %lf", maxT_pixel_value );
            logfile->debug( msg );
            sprintf(msg, "ka = %lf", delta_t );
            logfile->debug( msg );
            sprintf(msg, "kb = %lf", t0 );
            logfile->debug( msg );
        }

        short look_up_table[1024];
        logfile->debug( "outputFileName:" );
        logfile->debug( outputFileName );
        calculateLook_up_table( look_up_table, 1024, slope, intercept, board_t, corrParams, temp2rad, MINT, delta_t, NUMBERST, t0 );

        logfile->debug( "calculateLook_up_table finished" );
        logfile->debug( outputFileName );

        // Формируем паспорт выходного файла
        out_blk0 = in_blk0;
        out_blk0.processLevel |= 0x01;        // взводим флаг калибровка проведена
        out_blk0.ka = delta_t;
        out_blk0.kb = t0;
        out_blk0.maxPixelValue = short((maxT_pixel_value-t0)/delta_t+1.);

        int out_data_len = inp_data_len;
        TAutoPtr<short> outData(new short[inp_data_len] );
        logfile->info( "преобразование данных" );
        processLUT( inpData, outData, scans*cols, look_up_table );
        {
            sprintf(msg, "T[400] = %d, T[450] = %d, T[500] = %d",
                    int(look_up_table[400]), int(look_up_table[450]), int(look_up_table[500]) );
            logfile->info( msg );
        }
        logfile->debug( "processLUT finished" );
        logfile->debug( outputFileName );
        // корректируем out_blk0.maxPixelValue
        findMinMaxPositiveValue( outData, scans*cols, &min_pixel_value, &max_pixel_value );
        out_blk0.maxPixelValue = max_pixel_value;
        logfile->info( "сохранение результатов калибровки" );
        logfile->debug( "before writeFileNOAA" );
        logfile->debug( outputFileName );
        writeFileNOAA( outputFileName, out_blk0, outData, out_data_len );
        logfile->debug( "after writeFileNOAA" );
        //return ret;
    }
}

void readCfg() throw (TException) {
    char path[MAX_PATH];
    const char * s;

    if ((NULL == strchr(cfgName,'\\'))&&
            (NULL == strchr(cfgName,'/'))&&
            (NULL == strchr(cfgName,'.'))) {
        strcpy(path,term_root);
        int t = strlen(path);
        /* Если надо, в конце добавляем разделитель */
        if (path[t-1]!='/'||path[t-1]!='\\') {
            path[t] = DIRD;
            path[t+1] = '\0';
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
	const Setting& Log = conf.getRoot()["Log"];

	#warning повторяеться код
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
	#warning "сделал пока тупо как значение"
	conf.lookupValue("termos_conv_temp_step", option_ascent);
		/* если что-либо другое, считаем что CALIBR_ASCENT = true */
}

void construct_file_names() {
    char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], fext[MAX_EXT]; //, s[MAX_FNAME];
    splitpath( inputFileName, drive, dir, fname, fext );

    makepath( telemetryFileName, drive, dir, fname, "tlm" );
    makepath( outputFileName, drive, dir, fname, "clb" );

    logfile->debug( "input files:" );
    logfile->debug( inputFileName );
    logfile->debug( telemetryFileName );

    logfile->debug( "output file:" );
    logfile->debug( outputFileName );
    return;
}

void parseCommandString( int argc, char* argv[] ) throw (TException) {
    for ( int i = 1; i < argc; i++ ) {
        char * s = argv[i];
        if ( *s == '-' ) {
            s++;
            if ( *s == 'l' ) {
                s++;
                if (!strcmp(s,"err"))
                    useStdErr = true;
                else if (!strcmp(s,"out"))
                    useStdOut = true;
                else if (!strncmp(s,"file=",5)) {
                    s+=5;
                    if (strlen(s)==0)
                        throw TException(100, "Недопустимое значение параметра -lfile=<file name>");
                    logFileName.assign((char*)s);
                    append = false;
                } else if (!strncmp(s,"append=",7)) {
                    s+=7;
                    if (strlen(s)==0)
                        throw TException(100, "Недопустимое значение параметра -lappend=<file name>");
                    logFileName.assign((char*)s);
                    append = true;
                } else if (!strncmp(s,"level=",6)) {
                    s+=6;
                    if (nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(string(s))))
                        throw TException(100, "Недопустимое значение параметра -llevel=<message level>");
                } else {
                    sprintf(msg,"Неизвестный параметр %s",argv[i]);
                    throw TException(100, msg);
                }
            } else if ( *s == 'a' ) {
                /* опция -a */
                s++;
                if (*s == '\0' || *s == '1')
                    option_ascent = true;
                else if (*s == '0')
                    option_ascent = false;
                else {
                    sprintf(msg,"Неизвестный параметр %s",argv[i]);
                    throw TException(100, msg);
                }
            } else {
                /* указана неизвестная опция */
                sprintf(msg,"Неизвестный параметр %s",argv[i]);
                throw TException(100, msg);
            }
        }
    }
}


void readBlk0() throw (int) {
    FILE* f = fopen( inputFileName, "rb" );
    if ( f == 0 ) {
        sprintf( msg, "ошибка открытия файла %s", inputFileName );
        logfile->error( msg );
        throw 1;
    }
    if ( fread( &in_blk0, sizeof(in_blk0), 1, f ) != 1 ) {
        sprintf( msg, "ошибка чтения файла %s", inputFileName );
        logfile->error( msg );
        throw 1;
    }
    fclose( f );
}

void verifyBlk0( TBlk0_AVHRR & blk0, const char * name ) throw ( int ) {
    if ( blk0.b0.formatType == 0xFF
            && blk0.b0.dataType1 == 2             // одноканальные данные
            && blk0.b0.dataType2 == 1             // тип сенсора --- AVHRR
            && (blk0.processLevel & 1) == 0      // калибровка не проведена
       ) {
        return;
    }
    sprintf( msg, "неправильный формат нулевого блока файла %s", name );
    logfile->error( msg );
    throw 1;
}


void readCalibrDatFile( XML** xml )throw(int) {
    char path[MAX_PATH];
    //static char buf[2048];

    strcpy(path,term_root);
    int t = strlen(path);
    /* Если надо, в конце добавляем разделитель */
    if (path[t-1]!='/'||path[t-1]!='\\') {
        path[t] = DIRD;
        path[t+1] = '\0';
    }
#if DIRD == '/'
    strcat( path, "data/calibr.dat" );
#else
    strcat( path, "data\\calibr.dat" );
#endif
    logfile->debug( "calibr data file path:" );
    logfile->debug( path );
    logfile->debug( "" );

    try {
        *xml = new XML( path );
    } catch ( TException & e ) {
        sprintf( msg, "ошибка разбора/получения файла %s:", path );
        logfile->error( msg );
        logfile->error( e.text() );
        logfile->error( "" );
        throw 1;
    }
}

void readDataFile( void * buf, const char * name, int len, int blk0_len = 0 ) throw (int) {
    FILE * f = fopen( name, "rb" );
    if ( f == 0 ) {
        sprintf( msg, "ошибка открытия файла %s", name );
        logfile->error( msg );
        throw 1;
    }
    if ( fseek( f, blk0_len, SEEK_SET ) != 0 ) {
        sprintf( msg, "ошибка позиционирования в файле %s", name );
        logfile->error( msg );
        throw 1;
    }
    if ( fread( buf, 1, len, f ) != (size_t)len ) {
        sprintf( msg, "ошибка чтения файла %s", name );
        logfile->error( msg );
        throw 1;
    }
    fclose(f);
}

void writeFileNOAA( const char * name, TBlk0_AVHRR& blk0, short * data, int len ) throw (int) {
    FILE * f = fopen( name, "wb" );
    if ( f == 0 ) {
        sprintf( msg, "ошибка открытия файла %s для записи", name );
        logfile->error( msg );
        throw 1;
    }
    if ( fwrite( &blk0, sizeof(blk0), 1, f ) != 1 ) {
        sprintf( msg, "ошибка записи в файл %s", name );
        logfile->error( msg );
        throw 1;
    }
    if ( fwrite( data, 2, len, f ) != (size_t)len ) {
        sprintf( msg, "ошибка записи в файл %s", name );
        logfile->error( msg );
        throw 1;
    }
    fclose(f);
}

/*---------------------------------------*\
ALBCOF.C
Данная функция извлекает из файла "calibr.dat"
параметры slope values и intercept values
калибровки канала chan спутника sat. Если
зафиксирована ошибка  - возвращается единица,
в противном случае - 0.
\*---------------------------------------*/
void albcof( int sat, int chan, XML & xml, TAlbedoCalParams & p ) throw ( int ) {
    char mask1[20];
    char mask2[20];
    char *mask[] = {mask1, mask2, (char*)"ALBCOF"};

    string s;

    sprintf(mask[0], "SAT %d", sat);
    if (chan < 3)
        sprintf(mask[1], "CHAN %d", chan);
    else
        mask[1] = (char*)"CHAN 3A";

    try	{
        xml.toBegin();
        s = xml.get_text(3, mask);
    } catch (TException& e) {
        logfile->error( "ошибка разбора файла calibr.dat" );
        logfile->error( e.text() );
        throw 1;
    }

    if (s.empty())
	{
        logfile->error(strformat("в файле calibr.dat отсутствует информация по каналу %d спутнику %d", chan, sat));
        throw 1;
    }

    int n_params = strscanf( s, "%lf %lf %lf %lf", &p.slope_value, &p.intercept_value, &p.slope_value2, &p.intercept_value2 );

    if (n_params == 2)
	{
        p.used_additional_values = 0; // Дополнительные параметры не используются
    }
    else if (n_params == 4)
	{
        p.used_additional_values = 1; // Дополнительные параметры не используются
    }
    else
	{
        logfile->error(strformat("в файле calibr.dat отсутствуют (или присутствуют не полностью) параметры калибровки по каналу %d спутника %d.", chan, sat));
        logfile->error(strformat("полученная из файла строка:%s", s.c_str()));
        logfile->error( "параметрами калибровки являются два или четыре плавающих числа, разделенных пробелами.");
        throw 1;
    }
    {
        logfile->debug( "тип калибровки:" );
        logfile->debug( p.used_additional_values ?
                        " с использованием четырех параметров" :
                        " с использованием двух параметров" );
        if ( p.used_additional_values) {
            sprintf(msg, "параметры калибровки: %lf %lf %lf %lf",
                    p.slope_value, p.intercept_value,
                    p.slope_value2, p.intercept_value2 );
        } else {
            sprintf(msg, "параметры калибровки: %lf %lf",
                    p.slope_value, p.intercept_value);
        }
        logfile->debug(msg);
    }
    return;
}

/*--------------------------------------*\
TABALB
Инициализация таблицы преобразования
<count> --> <albedo>
\*--------------------------------------*/
void tabalb( short *table, int length, TAlbedoCalParams & p, double *p_ka, double *p_kb, int *p_maxV ) {
    int i;
    double a;

    double kb = alb0;
    double ka = delta_alb;
    int maxV = max_albedo_value;

    // Уточнение для новых спутников
    if ( p.used_additional_values ) {
        kb = alb0_klm;
        ka = delta_alb_klm;
        maxV = max_albedo_value_klm;
    }

    for (i = 0; i < length; i++) {
        if (p.used_additional_values) {
            // Обработка для NOAA 15 и новых спутников
            if (i < 500)
                a = (double) i * p.slope_value + p.intercept_value;
            else
                a = (double) i * p.slope_value2 + p.intercept_value2;
        } else {
            a = (double) i * p.slope_value + p.intercept_value;
        }

        if (a < 0.) {
            table[i] = 0;
        } else if (a > 100.) {
            table[i] = maxLimitOutPoint;
        } else
            table[i] = short(floor((a - kb) / ka + 0.5));
        sprintf(msg, "%3d -> %3d (%lf)", i, int(table[i]), a );
        logfile->debug(msg);
    }

    *p_kb = kb;
    *p_ka = ka;
    *p_maxV = maxV;
    //return;
}


//----------------------------------------------
// INI_PRT_A.C
// Функция инициализации массива prt_a[NPRT][NAA]
// спутника sat на основе данных считываемых из файла
// calibr.dat . Если зафиксирована ошибка  - возвращается 1,
// в противном случае возвращаем 0.
//----------------------------------------------
void ini_prt_a( TInputParams &p, double (*prt_a)[NAA] ) throw (int) {
    char mask1[20];
    char *mask[] = { mask1, (char*)"PRT_A" };

    string s;

    sprintf(mask[0], "SAT %d", p.sat);

    try {
        p.pxml->toBegin();
        s = p.pxml->get_text(2, mask);
    } catch (TException& e) {
        logfile->error( "ошибка разбора файла calibr.dat" );
        logfile->error( e.text() );
        throw 1;
    }

    if (s.length() == 0 ) {
        sprintf(msg, "в файле calibr.dat отсутствует информация PRT_A по каналу %d спутника %d", p.chan, p.sat);
        logfile->error(msg);
        throw 1;
    }

#warning Ацкое зло
    char *a = const_cast<char *>(s.c_str());
    char *a1;
    for (int i = 0; i < NPRT; i++) {
        for (int j = 0; j < NAA; j++) {
            prt_a[i][j] = strtod(a, &a1);
            if (a == a1) {
                logfile->error(strformat("в файле calibr.dat отсутствует информация PRT_A по каналу %d спутника %d", p.chan, p.sat));
                throw 1;
            }
            a = a1;
        }
    }

    {
        logfile->debug("таблица PRT_A");
        for (int i = 0; i < NPRT; i++) {
            *msg = 0;
            for (int j = 0; j < NAA; j++) {
                sprintf(msg + strlen(msg), "%lg ", prt_a[i][j]);
            }
            logfile->debug(msg);
        }
    }
}

//-----------------------------------------------
// INI_PRT_B.C
// Функция инициализации массива prt_b[NPRT]
// для канала chan спутника sat
// на основе данных считываемых из файла calibr.dat.
// Если зафиксирована ошибка  - возвращается 1,
// в противном случае 0.
//-----------------------------------------------
void ini_prt_b( TInputParams& p, double *prt_b ) throw (int) {
    char mask1[20];
    char *mask[] = { mask1, (char*)"PRT_B" };
    string s;

    sprintf(mask[0], "SAT %d", p.sat);
    try {
        p.pxml->toBegin();
        s = p.pxml->get_text(2, mask);
    } catch (TException& e) {
        logfile->error( "ошибка разбора файла calibr.dat" );
        logfile->error( e.text() );
        throw 1;
    }

    if (s.length() == 0) {
        sprintf(msg, "в файле calibr.dat отсутствует информация PRT_B по каналу %d спутника %d", p.chan, p.sat);
        logfile->error(msg);
        throw 1;
    }

    int col = strscanf( s, " %lf %lf %lf %lf ", &prt_b[0], &prt_b[1], &prt_b[2], &prt_b[3]);

    if (col != 4) {
        logfile->error(strformat("в файле calibr.dat отсутствует информация PRT_B по каналу %d спутника %d", p.chan, p.sat));
        throw 1;
    }

    {
        logfile->debug("таблица PRT_B");
        sprintf(msg, "%lf %lf %lf %lf", prt_b[0], prt_b[1], prt_b[2], prt_b[3]);
        logfile->debug(msg);
    }
}

//------------------------------------------
// INI_NLCOF.C
// Функция открывает файл calibr.dat и считывает
// из него параметры нелинейной коррекции для
// канала chan спутника sat. Результаты считывания
// заносятся в массив nlcof. При этом если
// номер спутника меньше 13 считываются
// 6 значений, согласно NESS 107.
// Если зафиксирована ошибка - возвращается 1,
// в противном случае 0.
//------------------------------------------
void ini_corrParams( TInputParams &p, TCorrParams &c ) throw ( int ) {
    c.new_calibr_flag =
        ((p.sat == 15427) ||        // NOAA-9
         (p.sat == 16969) ||       // NOAA-10
         (p.sat == 19531) ||       // NOAA-11
         (p.sat == 21263)          // NOAA-12
        ) ? 0 : 1;

    logfile->debug( "способ нелинейной коррекции:" );
    if ( c.new_calibr_flag ) {
        logfile->debug( " новый (для NOAA-13,14)" );
    } else {
        logfile->debug( " старый (для NOAA 9,10,11,12)" );
    }

    char mask1[20];               // SAT <Номер спутника>
    sprintf(mask1, "SAT %d", p.sat);

    char mask2[20];               // CHAN <номер канала>
    if (p.chan == 3 && p.chanAB == 2)
        strcpy(mask2, "CHAN 3B");
    else
        sprintf(mask2, "CHAN %d", p.chan);

    char *mask[] = {
        mask1,
        mask2,
        (char*)"NLTBL",
        (char*)"ITT"
    };
    string s;
    try	{
        p.pxml->toBegin();
        s = p.pxml->get_text(4, mask);
    } catch (TException &e) {
        logfile->error( "ошибка разбора файла calibr.dat" );
        logfile->error( e.text() );
        throw 1;
    }
    if (s.length() != 0) {                    // NLTBL присутствует
        c.tbl_flag = 1;
        {                                 // ITT считываем
            char *mask[] = {
                mask1,
                mask2,
                (char*)"NLTBL",
                (char*)"ITT"
            };

#warning повторяющися код
            try {
				p.pxml->toBegin();
				string tmp = p.pxml->get_text(4, mask);
				string::iterator rest = tmp.end();
				c.tbl_itt_length = parseStringOfDouble(tmp, c.tbl_itt, 10, -1, &rest);
				if ( rest != tmp.end() ) {
					string rstr = tmp.substr(rest - tmp.begin());
					logfile->debug(strformat("остались неразобранные данные в <%s> <%s> <%s> <%s>: [%s]",
						mask[0], mask[1], mask[2], mask[3], rstr.c_str()));
				}
            } catch (TException &e) {
                logfile->error( "ошибка разбора файла calibr.dat" );
				logfile->error(strformat("путь: <%s> <%s> <%s> <%s>", mask[0], mask[1], mask[2], mask[3]));
                logfile->error( e.text() );
                throw 1;
            }
        }
        {                                 // ST считываем
            char *mask[] = {
                mask1,
                mask2,
                (char*)"NLTBL",
                (char*)"ST"
            };

#warning
            try {
               p.pxml->toBegin();
				string tmp = p.pxml->get_text(4, mask);
				string::iterator rest = tmp.end();
				c.tbl_itt_length = parseStringOfDouble(tmp, c.tbl_itt, 20, -1, &rest);
				if ( rest != tmp.end() ) {
					string rstr = tmp.substr(rest - tmp.begin());
					logfile->debug(strformat("остались неразобранные данные в <%s> <%s> <%s> <%s>: [%s]",
						mask[0], mask[1], mask[2], mask[3], rstr.c_str()));
				}

            } catch (TException &e) {
                logfile->error( "ошибка разбора файла calibr.dat" );
                sprintf(msg, "путь: <%s> <%s> <%s> <%s>",
                        mask[0], mask[1], mask[2], mask[3] );
                logfile->error(msg);
                logfile->error( e.text() );
                throw 1;
            }
        }
        {                                 // DATA считываем
            char *mask[] = {
                mask1,
                mask2,
                (char*)"NLTBL",
                (char*)"DATA"
            };

#warning повторяющийся код
            try {
               p.pxml->toBegin();
				string tmp = p.pxml->get_text(4, mask);
				string::iterator rest = tmp.end();
				c.tbl_itt_length = parseStringOfDouble(tmp, c.tbl_itt, 200, -1, &rest);
				if ( rest != tmp.end() ) {
					string rstr = tmp.substr(rest - tmp.begin());
					logfile->debug(strformat("остались неразобранные данные в <%s> <%s> <%s> <%s>: [%s]",
						mask[0], mask[1], mask[2], mask[3], rstr.c_str()));
				}
            } catch (TException &e) {
                logfile->error( "ошибка разбора файла calibr.dat" );
                sprintf(msg, "путь: <%s> <%s> <%s> <%s>",
                        mask[0], mask[1], mask[2], mask[3] );
                logfile->error(msg);
                throw 1;
            }
        }
        if (c.tbl_data_length != c.tbl_itt_length * c.tbl_st_length) {
            sprintf(msg, "\
					число элементов <%s> <%s> <NLTBL> <DATA> должно равняться \
					числу сочетаний <%s> <%s> <NLTBL> <ITT> и <%s> <%s> <NLTBL> <ST>",
                    mask1, mask2, mask1, mask2, mask1, mask2);
            logfile->error(msg);
            throw 1;
        }
    } else {
        logfile->debug("NLTBL отсутствует, считываем NLCOF");
        c.tbl_flag = 0;
        try {
            mask[2] = (char*)"NLCOF";
            p.pxml->toBegin();
            s = p.pxml->get_text(3, mask);
        } catch (TException &e) {
            logfile->error( "ошибка разбора файла calibr.dat" );
            logfile->error( e.text() );
            throw 1;
        }

        if (s.length() == 0) {
            sprintf(msg, "в файле calibr.dat отсутствует информация NLCOF по каналу %d спутника %d", p.chan, p.sat);
            logfile->error(msg);
            throw 1;
        }

        if (!c.new_calibr_flag) {
            // для спутников 9-12
            int col = strscanf( s, " %lf %lf %lf %lf %lf %lf ",
                              &(c.table[0]), &(c.table[1]), &(c.table[2]),
                              &(c.table[3]), &(c.table[4]), &(c.table[5]) );
            if (col != 6) {
                sprintf(msg, "в файле calibr.dat отсутствует информация NLCOF по каналу %d спутника %d", p.chan, p.sat);
                logfile->error(msg);
                throw 1;
            }
        } else {
            // Для спутников 12-14
				int col = strscanf( s, " %lf %lf %lf %lf ",
                              &(c.table[0]), &(c.table[1]),
                              &(c.table[2]), &(c.table[3]) );
            if (col != 4) {
                sprintf(msg, "в файле calibr.dat отсутствует информация NLCOF по каналу %d спутника %d", p.chan, p.sat);
                logfile->error(msg);
                throw 1;
            }
        }
        {
            sprintf(msg,"!!! параметры нелинейной коррекции канала %d спутника %d:", p.chan, p.sat );
            logfile->debug(msg);
            if (!c.new_calibr_flag) {
                sprintf( msg, "%lf %lf %lf %lf %lf %lf",
                         c.table[0], c.table[1], c.table[2],
                         c.table[3], c.table[4], c.table[5]);
            } else {
                sprintf(msg, "%lf %lf %lf %lf",
                        c.table[0], c.table[1], c.table[2], c.table[3]);
            }
            logfile->debug(msg);
        }
    }
}

//-----------------------------------------
// INI_RAD.C
// Функция инициализирует массив преобразования
// температуры -> радиации
// для чего считывает таблицу функции отклика.
//-----------------------------------------
void ini_lut_T2R( T_NFR &nfr, double *temp2rad, double minT, double stepT, int len ) {
    for (int j = 0; j < len; j++) {
        double ti = KELVIN0 + minT + stepT * j;
        temp2rad[j] = tem2rad(ti, nfr );
    }

    logfile->debug("таблица перевода температуры -> радиации" );
    logfile->debug("<номер в таблице> <температура (K)> <значение радиации>");
    for (int j = 0; j < NUMBERST; j++) {
        double ti = KELVIN0 + minT + stepT * j;
        sprintf(msg, "lut_T2R: %d %lf %lf", j, ti, double(temp2rad[j]) );
        logfile->debug(msg);
    }
}

//-------------------------------------------------
// INI_NFR.C
// Функция считывает из файла calibr.dat
// данные для normalized response functions
// спутника sat и канала chan. При этом
// параметры задания normalized response functions
// помещаются в переменные nfr_starting_wave и
// nfr_increment_wave.
// Если зафиксирована ошибка  - возвращаем 1,
// в противном случае 0.
//-------------------------------------------------
void ini_nfr( TInputParams& p, T_NFR &nfr ) throw ( int ) {
    {                                     // Читаем starting wave
        char mask1[20];
        char mask2[20];
        char *mask[] = {
            mask1,
            mask2,
            (char*)"FICOF",
            (char*)"SW"
        };
        string s;

        sprintf(mask[0], "SAT %d", p.sat);
        if (p.chan == 3 && p.chanAB == 2)
            mask[1] = (char*)"CHAN 3B";
        else
            sprintf(mask[1], "CHAN %d", p.chan);

        try {
            p.pxml->toBegin();
            s = p.pxml->get_text(4, mask);
        } catch (TException &e) {
            logfile->error( "ошибка разбора файла calibr.dat" );
            logfile->error( e.text() );
            throw 1;
        }
        if ( strscanf( s, " %lf ", &nfr.starting_wave) != 1 ) {
			logfile->error(strformat("в файле calibr.dat отсутствует информация starting wave таблицы NFR по каналу %d спутника %d", p.chan, p.sat));
			logfile->error(strformat("полученная из XML строка: %s", s.c_str() ));
            throw 1;
        }
    }

    {                                     // Читаем increment wave
        char mask1[20];
        char mask2[20];
        char *mask[] = {
            mask1,
            mask2,
            (char*)"FICOF",
            (char*)"INC"
        };
        string s;

        sprintf(mask[0], "SAT %d", p.sat);
        if (p.chan == 3 && p.chanAB == 2)
            mask[1] = (char*)"CHAN 3B";
        else
            sprintf(mask[1], "CHAN %d", p.chan);

        try {
            p.pxml->toBegin();
            s = p.pxml->get_text(4, mask);
        } catch (TException &e) {
            logfile->error( "ошибка разбора файла calibr.dat" );
            logfile->error( e.text() );
            throw 1;
        }

		if (s.empty() || strscanf( s, " %lf ", &nfr.increment_wave) != 1) {
            logfile->error(strformat("в файле calibr.dat отсутствует информация INC таблицы NFR по каналу %d спутника %d", p.chan, p.sat));
            throw 1;
        }
    }

    {                                     // Извлекаем данные NFR
        char mask1[20];
        char mask2[20];
        char *mask[] = {
            mask1,
            mask2,
            (char*)"FICOF",
            (char*)"DATA"
        };
        string s;

        sprintf(mask[0], "SAT %d", p.sat);
        if (p.chan == 3 && p.chanAB == 2)
            mask[1] = (char*)"CHAN 3B";
        else
            sprintf(mask[1], "CHAN %d", p.chan);

        try {
            p.pxml->toBegin();
            s = p.pxml->get_text(4, mask);
        } catch (TException &e) {
            logfile->error( "ошибка разбора файла calibr.dat" );
            logfile->error( e.text() );
            throw 1;
        }

        if (s.empty()) {
            sprintf(msg, "в файле calibr.dat отсутствует информация DATA таблицы NFR по каналу %d спутника %d", p.chan, p.sat);
            logfile->error(msg);
            throw 1;
        }

#warning Ацкое зло
        char *a =  const_cast<char*>(s.c_str());
        char *a1;

        for (int i = 0; i < NRF_LENGTH; i++) {
            nfr.table[i] = strtod(a, &a1);
            if (a == a1) {
                sprintf(msg, "в файле calibr.dat отсутствует информация DATA таблицы NFR по каналу %d спутника %d", p.chan, p.sat);
                logfile->error(msg);
                throw 1;
            }
            a = a1;
        }
    }

    {
        logfile->debug( "normalized response functions" );
        sprintf(msg, "starting wave = %lf", nfr.starting_wave);
        logfile->debug( msg );
        sprintf(msg, "increment = %lf", nfr.increment_wave);
        logfile->debug( msg );
        msg[0] = 0;
        for (int i = 0; i < NRF_LENGTH; i++) {
            sprintf(msg + strlen(msg), "%lg ", nfr.table[i]);
            if (strlen(msg) > 75) {
                logfile->debug(msg);
                msg[0] = 0;
            }
        }
        if (strlen(msg) > 2) {
            msg[strlen(msg) - 2] = 0;
            logfile->debug(msg);
        }
    }
}


void calculateTarget( TInputParams &p, TTelemetryData &telemetry, T_NFR &nfr, double *radiance, double *temperature ) throw (int) {
    double prt_a[NPRT][NAA];  // параметры a2, a1, a2, a3, a4 для каждого PRT
    double prt_b[NPRT];       // PRT WEIGHTING FACTORS


    ini_prt_a( p, prt_a );
    ini_prt_b( p, prt_b );

    double board_t = 0.;
    for (int i = 0; i < NPRT; i++) {
        double prtx = telemetry.prt[i];
        double p = 1.0;
        double ti = 0.0;
        for (int j = 0; j < NAA; j++) {
            ti += p * prt_a[i][j];
            p *= prtx;
        }
        board_t += prt_b[i] * ti;
    }
    *radiance = tem2rad(board_t, nfr);
    *temperature = board_t;
    {
        sprintf(msg, "температура бортового излучателя: %lf", board_t);
        logfile->debug(msg);
        sprintf(msg, "излучение внутренней цели: %lf", *radiance );
        logfile->debug(msg);
    }
}


//-----------------------------------------------
// TEM2RAD.C
// Функция возвращает значение радиации, соответствующие
// температуре t(K).
// Интеграл от функции Планка по nfr.
//-----------------------------------------------
double tem2rad(double t, T_NFR &nfr ) {
    static const double c1 = 1.191065E-5;          // константы для
    static const double c2 = 1.438833;             // формулы Планка

    double c2t = c2 / t;
    double rad = 0.;
    for (int i = 0; i < NRF_LENGTH; i++) {
        double vi = nfr.starting_wave + i*nfr.increment_wave;
        double ri = c1 * vi * vi * vi;
        ri /= exp(c2t * vi) - 1.0;
        ri *= (nfr.table[i] * nfr.increment_wave);
        rad += ri;
    }
    return rad;
};


//------------------------------------------
// TELEMETRY_PROCESSING.C
// Функция читает файл телеметрии, и, на основе прочитанных данных,
// инициализирует
// массив prt значениями PRT[i] а переменные target_values, space_values
// показаниями радиометра для эталонной цели и космического пространства
// соответственно.
// Если зафиксирована ошибка  - выполнение  программы прерывается
// (функцией exit). Коды
// ошибок, возвращаемые программой, перечислены в файле calibr.err .
//------------------------------------------
void telemetry_processing( TTelemetryData& tel, short* data, int scans, const char * fname = 0 ) throw (int) {
    //  short tel[LTEL];
    //int fp;
    short prts;
    short hbs[INCNT];
    short hsp[INCNT];
    short hprt[NPRT][INCNT];
    int scan;           // номер текущей анализируемой строки
    int med_err;        // код ошибки, возвращаемый функцией medin

    // Обнуляем массивы гистограмм
    memset((char *) hbs, 0, INCNT * sizeof(short));
    memset((char *) hsp, 0, INCNT * sizeof(short));
    memset((char *) hprt, 0, NPRT * INCNT * sizeof(short));

    for (scan = 0; scan < scans; scan++) {
        if ( medin( data+LTEL*scan, 5, 7, 2, &med_err ) <= 10 &&
                med_err == 0 ) {
            break;
        }
    }

    if (scan == scans - 1) {  // Если достигнут конец файла...
        sprintf(msg, "в файле телеметрии %s слишком мало данных для калибровки.", fname);
        logfile->error(msg);
        throw 1;
    }

    int bas = scan;
    int n_hbs = 0;
    int n_hsp = 0;
    for (;scan < scans; scan++) {
        int n = (scan - bas) % 5;
        short int *tel = data + 30 * scan;
        if ((n > 0) && (n <= NPRT)) {
            prts = medin(tel, 5, 7, 2, &med_err );
            if ( med_err == 0 ) {
                hprt[n - 1][prts] += 1;
            }
        }
        prts = medin(tel, 10, 19, 5, &med_err );
        if ( med_err == 0 ) {
            hbs[prts] += 1;
            n_hbs++;
        }
        prts = medin(tel, 20, 29, 5, &med_err );
        if ( med_err == 0 ) {
            hsp[prts] += 1;
            n_hsp++;
        }
    }

    if ( n_hbs < 10 || n_hsp < 10 ) {
        sprintf(msg, "в файле телеметрии %s слишком мало данных для калибровки.", fname );
        //logfile->error(msg);
        logfile->warning(msg);
    }

    tel.space_value = medh(hsp, n_hsp / 2, INCNT);
    tel.target_value = medh(hbs, n_hbs / 2, INCNT);
    {
        sprintf(msg, "space value = %d", int(tel.space_value) );
        logfile->debug(msg);
        sprintf(msg, "target value = %d", int(tel.target_value) );
        logfile->debug(msg);
    }

    for (int j = 0; j < NPRT; j++) {
        int n = 0;
        for (int i = 0; i < INCNT; i++)
            n += hprt[j][i];
        if (n < 10) {
            sprintf(msg, "в файле телеметрии %s слишком мало данных для калибровки.", fname );
            //logfile->error(msg);
            logfile->warning(msg);
        }
        n /= 2;
        tel.prt[j] = medh(hprt[j], n, INCNT);
    }

    {
        logfile->debug("показания prt-ов");
        msg[0] = 0;
        for (int i = 0; i <= NPRT-1; i++)
            sprintf(msg + strlen(msg), "PRT[%d]=%d, ", i, int(tel.prt[i]));
        msg[strlen(msg)-2] = 0;
        logfile->debug(msg);
    }
}


/*---------------------------------------*\
MEDIN.C
Функция возвращает nm-ый член неупорядоченной выборки чисел
типа short str[i1-i2].
Примечание: числа должны быть не меньше нуля и не больше чем INCNT-1.
Выполнение данного требования не контролируется, но его несоблюдение
может иметь самые пагубные последствия связанные с ошибками адресации.
\*---------------------------------------*/
short medin(short *str, int i1, int i2, int nm, int *err_code ) {
    short h[INCNT];
    memset( (char *)h, 0, INCNT * sizeof(short));
    for ( int i = i1; i <= i2; i++ ) {
        if ( str[i] >= INCNT ) {
            *err_code = 1;
            return 0;
        }
        h[str[i]]++;
    }
    *err_code = 0;
    return (medh(h, nm, INCNT));
}

/*---------------------------------------*\
MEDH.C
Функция возвращает nm-ое значение упорядоченной
выборки заданной гистограммой распределений
h[lh].
\*---------------------------------------*/
short medh(short *h, int nm, int lh) {
    int sum = 0;
    int i = 0;

    while ((sum < nm) && (i < lh))
        sum += h[i++];
    return (--i);
}

void calcLinearConversionParams( double radianse_target, TTelemetryData &tel, TCorrParams &corr, double *slope, double *intercept ) {
    double radianse_space = corr.new_calibr_flag ? corr.table[3] : 0.0;
    *slope = (radianse_space - radianse_target) /
             double(tel.space_value - tel.target_value);
    *intercept = radianse_space - (*slope) * tel.space_value;

    sprintf(msg, "излучение внутренней цели: %lf", radianse_target);
    logfile->debug(msg);
    sprintf(msg, "излучение космического пространства: %lf", radianse_space);
    logfile->debug(msg);
    sprintf(msg, "slope_values: %.12lf", double(*slope) );
    logfile->debug(msg);
    sprintf(msg, "intercept_values: %.12lf", double(*intercept) );
    logfile->debug(msg);
}

// --------------------------------------------
//  GETCOFCAL.C
//  Функция на основании данных таблиц prt, prt_a,
//  prt_b и значений space_value и target_value
//  определяет значения slope_value, intercept_value
//  и board_t .
// --------------------------------------------
// void getCofCal( TInputParams& p, TTelemetryData& tel, double* temp2rad, double* result )
// {
//     double board_t = 0.;
//     for (int i = 0; i < NPRT; i++) {
//         double prtx = prt[i];
//         double p = 1.0;
//         double ti = 0.0;
//         for (int j = 0; j < NAA; j++) {
//             ti += p * prt_a[i][j];
//             p *= prtx;
//         };
//         board_t += prt_b[i] * ti;
//     };
//
//     radianse_target = tem2rad(board_t);
//     radianse_space = (new_calibr_flag) ? nlcof[3] : 0.0;
//
//     slope_value = (radianse_space - radianse_target) /
//         double(space_value - target_value);
//     intercept_value = radianse_space - slope_value * double(space_value);
//
//     return;
// }

/*-------------------------------------------------*\
GET_TEMP
Функция выдает температуру (в градусах Цельсия)
соответствующую исходному значению x.
\*-------------------------------------------------*/
double get_temp(int i, double slope, double intercept, double board_t, TCorrParams corr, double* temp2rad, double minT, double stepT, int length ) {
    double x =  intercept + slope * double(i);
    if (corr.new_calibr_flag && !corr.tbl_flag) {   // Дли новых спутников нелинейная коррекция выполняется для радиаций
        if (corr.table[0] > 0.5 && corr.table[0] < 1.5)   // Существуют формулы двух видов, отличающиеся на 1 в corr.table[0]
            x = corr.table[0] * x + corr.table[1] * x * x + corr.table[2];
        else
            x += corr.table[0] * x + corr.table[1] * x * x + corr.table[2];
    }
    double temp = find_t(x, temp2rad, minT, stepT, length );

    // Если вышли за пределы представимых температур
    // сводим границы к пределам
    if (temp < MINT)
        return MINT;
    if (temp > MAXT)
        return MAXT;

    {                                     // Коррекция для старых спутников
        double d = 0.0;           // расчет коррекции

        if (corr.tbl_flag) {             // Расчет коррекции по данным NLTBL
            double itt = board_t - KELVIN0;
            double st = temp + KELVIN0;
            int i = 1;
            while (itt > corr.tbl_itt[i] && i < corr.tbl_itt_length)
                i++;
            int j = 1;
            while (st > corr.tbl_st[j] && j < corr.tbl_st_length)
                j++;

            double itt_norm = (itt - corr.tbl_itt[i - 1]) /
                              (corr.tbl_itt[i] - corr.tbl_itt[i - 1]);
            double st_norm = (st - corr.tbl_st[j - 1]) /
                             (corr.tbl_st[j] - corr.tbl_st[j - 1]);

            double wc = 1. - 2. * max(fabs(itt_norm - 0.5), fabs(st_norm - 0.5));
            double w1 = 1. - (itt_norm + st_norm);
            double w2 = 1. - ((1. - itt_norm) + st_norm);
            double w3 = 1. - ((1. - itt_norm) + (1. - st_norm));
            double w4 = 1. - (itt_norm + (1. - st_norm));

            double cij = corr.tbl_data[(j - 1) * corr.tbl_itt_length + (i - 1)];
            double cipj = corr.tbl_data[(j - 1) * corr.tbl_itt_length + i];
            double cijp = corr.tbl_data[j * corr.tbl_itt_length + (i - 1)];
            double cipjp = corr.tbl_data[j * corr.tbl_itt_length + i];

            d += wc * 0.25 * (cij + cipjp + cipj + cijp);
            if (w1 > 0.)
                d += w1 * cij;
            if (w2 > 0.)
                d += w2 * cipj;
            if (w3 > 0.)
                d += w3 * cipjp;
            if (w4 > 0.)
                d += w4 * cijp;
        } else if (!corr.new_calibr_flag) {
            double p = 1.;
            for (int k = 0; k < 3; k++) {
                d += corr.table[k] * p;
                p *= temp + KELVIN0;
            }
            p = board_t;
            for (int k = 3; k < NNCOF; k++) {
                d += corr.table[k] * p;
                p *= temp + KELVIN0;
            }
        }
        temp += d;      // добавляем нелинейную поправку
    }
    return temp;
}

/*--------------------------------------------------*\
TABTR.C
Процедура для вычисления look up table.
\*--------------------------------------------------*/
void calculateLook_up_table( short *LUT, int length, double slope, double intercept, double board_t, TCorrParams &corr, double *temp2rad, double MINT, double delta_t, int NUMBERST, double t0 ) {
    for (int i = 0; i < length; i++) {
        double x = get_temp(i, slope, intercept, board_t, corr, temp2rad, MINT, delta_t, NUMBERST);
        if (x >= t0)
            LUT[i] = short(floor((x - t0) / delta_t + 0.5));
        else
            LUT[i] = lost_value;
    }

    //{
    logfile->debug("calibr temp: <исходное значение> <получаемое значение> <температура> <температура до нелинейной коррекции> <радиация> ");
    for (int i = 0; i < length; i++) {
        double r = slope * i + intercept;
        double x = get_temp(i, slope, intercept, board_t, corr, temp2rad, MINT, delta_t, NUMBERST);
        sprintf(msg, "%4d %4d %10.3lf %10.3lf %10.3lf",
                i, int(LUT[i]), x, double(find_t(r, temp2rad, MINT, delta_t, NUMBERST)), r );
        logfile->debug(msg);
    }
    //}
    return;
}

/*-------------------------------------------*\
FIND_T.C
Функция возвращает значение температуры
соответствующее радиации r. Поиск соответствующего
значения ведется в массиве rad методом
деления пополам.
\*-------------------------------------------*/
double find_t(double r, double *temp2rad, double minT, double stepT, int nT ) {
    int i_left = 0;
    int i_right = nT - 1;
    int leng;
    int i_mid;
    double ir;

    if (r <= temp2rad[0])
        return minT;
    if (r >= temp2rad[nT-1])
        return (minT + (nT-1)*stepT);

    leng = i_right - i_left + 1;
    while (leng > 2) {
        i_mid = i_left + (leng >> 1);
        if (temp2rad[i_mid] == r)
            return (minT + stepT * i_mid);
        if (r < temp2rad[i_mid])
            i_right = i_mid;
        else
            i_left = i_mid;
        leng = i_right - i_left + 1;
    };
    ir = double(i_left) +
         (r - temp2rad[i_left]) *
         (i_right-i_left)/(temp2rad[i_right] - temp2rad[i_left]);
    return (minT + stepT * ir);
}


/***************************************************\
parseStringOfDouble
Функция разбирает строку плавающих чисел, разделенных пробельными символами,
результаты разбора помещает в массив data длины N.
Можно задать ограничитель объема считываемых данных, указав limitN.
В случае переполнения массива бросается exception
типа TCharString.
Указатель на остаток строки, который не удалось разобрать,
помещается по адресу rest.  Если строка прочитана полностью, **rest == (char)0;
Функция возвращает количество прочитанных чисел.
*/
int parseStringOfDouble(const char *string, double *data, int N, int limitN = -1, char **rest = NULL) throw (TException) {
    int index = 0;
    const char *p1 = string;
    const char *p2;
    double a;

    while (a = strtod(p1, (char **) (&p2)), p1 != p2 && index != N && index != limitN) {
        data[index] = a;
        p1 = p2;
        index++;
    }
    if (index == N && p1 != p2 && index != limitN) {
        throw TException(1, "parseStringOfDouble: переполнение массива data.");
    }
    if (rest) {
        if (index == limitN)
            *rest = (char *) p1;
        else
            *rest = (char *) p2;
        while ((**rest) && isspace(**rest))
            (*rest)++;
    }
    return index;
}
/*
 *\***************************************************/
int parseStringOfDouble(string& str, double *data, int N, int limitN = -1, string::iterator* rest = NULL) throw (TException) {
    int index = 0;
    double a;
    istringstream ins(str);
    streampos t = ins.tellg();
    while ( ins >> a && index != N && index != limitN ) {
        data[index] = a;
        t = ins.tellg();
        index++;
    }
    if (index == N && ins.good() && index != limitN) {
        throw TException(1, "parseStringOfDouble: переполнение массива data.");
    }
    if (rest) {
        if (index == limitN)
            *rest = str.begin() + ins.tellg();
        else
            *rest = str.begin() + t;
    }
    return index;
}




/*************************************************\
Инициализация навигационной подсистемы.
\*************************************************/
void navigationSystemInit( const TBlk0& blk0, TStraightReferencer** r, double *julian_date ) throw (int) {
    try {
        TNOAAImageParams NOAAImageParams(blk0);
        TCorrectionParams corrParams(blk0);
        *julian_date =
            julian(NOAAImageParams.fYear, NOAAImageParams.fYearTime + 1);
        TIniSatParams iniSatParams( blk0);
        TOrbitalModel orbitalModel(iniSatParams, NOAAImageParams.fYear, NOAAImageParams.fYearTime + 1, corrParams );
        *r = new TStraightReferencer(iniSatParams, NOAAImageParams, corrParams );
    } catch (...) {
        logfile->debug( "ошибка инициализации навигационной подсистемы" );
        throw 1;
    }
    //return 0;
}

/*----------------------------------------*\
ANGLE.C
Функция осуществляет вычисление угла
склонения Солнца для точки исходного снимка
(в градусах).
Вызов:
angle( nscan, col )
nscan - номер строки
col - номер пикселя в строке
Должны быть проинициализированы глобальные
переменные julian_date и satParams.
Кроме этого заранее вызвано функция ini_sgp8
\*----------------------------------------*/
double angle(int nscan, int col, TStraightReferencer& r, double julian_date) {
    double lat;
    double lon;
    double fi;
    r.xy2ll(col, nscan, &lon, &lat);
    lon = -lon;
    fi = shcrds(julian_date, lon, lat);
    return (fi);
}

int findMinMaxPositiveValue( short * d, int length, short *minv, short *maxv ) {
    int i = 0;
    while ( i < length && d[i] < 0 )
        i++;
    if ( i == length )
        return 1;
    *minv = d[i];
    *maxv = d[i];

    for (; i < length; i++) {
        if (d[i] >= 0) {
            if (d[i] < *minv)
                *minv = d[i];
            if (d[i] > *maxv)
                *maxv = d[i];
        }
    }
    return 0;
}

void processLUT( short* in, short* out, int length, short* LUT ) {
    static char msg[256];
    for (int i = 0; i < length; i++) {
        if ( in[i] >= INCNT ) {	// Затычка, связанная с возможной порчей памяти на ASA-PCI
            sprintf( msg, "Ошибочное значение по адресу %d "
                     "(встретилось %d, ожидается значение меньшее %d)", i, int(in[i]), int(INCNT) );
            //logfile->error( msg );
            logfile->warning(msg);
            out[i] = -7;
        } else if (in[i] < 0) {
            out[i] = in[i];
        } else {
            out[i] = LUT[in[i]];
        }
    }
}

void proccesAscent( short * data, int scans, int cols, TStraightReferencer& r, double julian_date, double ka, double kb ) {
    static const int step = 128;
    logfile->debug( "деление значений альбедо на косинус угла восхождения на Солнце." );
    for (int i = 0; i < scans; i++) {
        int col1 = 0;
        int col2 = col1 + step;
        double sinang1 = sin(angle( i, col1, r, julian_date )*DR);
        double sinang2 = sin(angle( i, col2, r, julian_date )*DR);
        for (int j = 0; j < cols; j++) {
            double sinang;
            if ( j == col1 ) {
                sinang = sinang1;
            } else if ( j == col2 ) {
                sinang = sinang2;
            } else if ( j > col2 ) {
                col1 = col2;
                col2 = col1 + step;
                sinang1 = sinang2;
                sinang2 = sin(angle( i, col2, r, julian_date )*DR);
                sinang = sinang1+(j-col1)*(sinang2-sinang1)/double(step);
            } else {
                sinang = sinang1+(j-col1)*(sinang2-sinang1)/double(step);
            }

            short inta = data[i*cols+j];
            if ( inta > 0 ) {
                //              ang = angle( i, j )*DR;
                if ( sinang < sinalpha_minlimit ) {
                    // угол восхождения на солнце столь низок,
                    // что данным канала нельзя доверять
                    inta = maxLimitOutPoint;    // Вообще, при делении на 0 мы получаем +\infinity
                } else {
                    double a = inta * ka + kb;
                    a /= sinang;
                    if ( a < 0. )
                        inta = 0;
                    else if ( a > 100. )
                        inta = maxLimitOutPoint;
                    else
                        inta = int( floor((a-kb)/ka + 0.5) );
                }
                data[i*cols+j] = inta;
            }
        }
    }
    logfile->debug( "деление завершено." );
}
