/*-----------------------------------------------------------------------------
 filtrcloudy.hpp
------------------------------------------------------------------------------*/

#ifndef _FILTRCLOUDY_HPP_
#define _FILTRCLOUDY_HPP_

#include <tc_config.h>
//#include <c_>
#include <astronom.hpp>
#include <tregfiltrcloudy.hpp>

// позиции отдельных фильтров в массиве статистики
const int MASK_ALBEDO_1_FILTR = 1;
const int MASK_ALBEDO_2_FILTR = 2;
const int MASK_NDVI_FILTR = 4;
const int MASK_TEMP_FILTR = 8;
const int MASK_34_DELTA_FILTR = 16;
const int MASK_35_DELTA_FILTR = 32;
const int MASK_45_DELTA_FILTR = 64;
const int N_MASK_VALUE = 64 * 2 + 2;
const int MASK_MULTICH = N_MASK_VALUE - 2;
const int MASK_BORDER_FILTR = N_MASK_VALUE - 1;


// Значения, которыми программа калибровки отмечает ошибочные пиксели
const short minLimitOutPoint = -12;    // значение меньше представимого
const short maxLimitOutPoint = -13;    // значение больше представимого

//struct TFiltrParams;
//
// Параметры фильтрации облачности
//
struct TFiltrParams {
    // фильтрация по альбедо
    int albedo1_flag[5];
    int albedo2_flag[5];

    int ndvi_flag[5];
    int assent_flag[5];    // при фильтрации по альбедо учитывать
                          // угол восхождения на Солнце

    double max_albedo1[5];
    double max_albedo2[5];

    double min_ndvi[5], max_ndvi[5];

    // фильтрация по температуре
    int temp_flag[5];
    double min_temp[5], max_temp[5];

    // фильтрация по разности четвертого/пятого каналов
    int day_delta45_flag[5];
    double day_min_delta45[5],  day_max_delta45[5];
    int night_delta45_flag[5];
    double night_min_delta45[5],  night_max_delta45[5];

    // фильтрация по разности третьего/четвертого каналов
    int day_delta34_flag[5];
    double day_min_delta34[5],  day_max_delta34[5];
    int night_delta34_flag[5];
    double night_min_delta34[5],  night_max_delta34[5];

    // фильтрация по разности третьего/пятого каналов
    int day_delta35_flag[5];
    double day_min_delta35[5],  day_max_delta35[5];
    int night_delta35_flag[5];
    double night_min_delta35[5],  night_max_delta35[5];

    // фильтрация границ облачности
    int cloud_border_flag[5];
    int cloud_border_win_size[5];
    double max_filtered_percent[5];

    // после окончания работы вывести статистику
    int stat_flag[5];

    // Минимальное значение синуса  угла восхождения на солнце
    // при котором данным видимых каналов можно доверять.
    static const double sinalpha_minlimit;

    static const char * FILTR_ALBEDO_1_ENABLE[5];
    static const char * FILTR_MAX_ALBEDO_1_VALUE[5];
    static const char * FILTR_ALBEDO_2_ENABLE[5];
    static const char * FILTR_MAX_ALBEDO_2_VALUE[5];
    static const char * FILTR_NDVI_ENABLE[5];
    static const char * FILTR_MIN_NDVI_VALUE[5];
    static const char * FILTR_MAX_NDVI_VALUE[5];
    static const char * FILTR_ASSENT_ALBEDO_ENABLE[5];
    static const char * FILTR_TEMP_ENABLE[5];
    static const char * FILTR_MIN_TEMP[5];
    static const char * FILTR_MAX_TEMP[5];
    static const char * FILTR_DAY_DELTA45_ENABLE[5];
    static const char * FILTR_MIN_DAY_DELTA45[5];
    static const char * FILTR_MAX_DAY_DELTA45[5];
    static const char * FILTR_NIGHT_DELTA45_ENABLE[5];
    static const char * FILTR_MIN_NIGHT_DELTA45[5];
    static const char * FILTR_MAX_NIGHT_DELTA45[5];
    static const char * FILTR_DAY_DELTA35_ENABLE[5];
    static const char * FILTR_MIN_DAY_DELTA35[5];
    static const char * FILTR_MAX_DAY_DELTA35[5];
    static const char * FILTR_NIGHT_DELTA35_ENABLE[5];
    static const char * FILTR_MIN_NIGHT_DELTA35[5];
    static const char * FILTR_MAX_NIGHT_DELTA35[5];
    static const char * FILTR_DAY_DELTA34_ENABLE[5];
    static const char * FILTR_MIN_DAY_DELTA34[5];
    static const char * FILTR_MAX_DAY_DELTA34[5];
    static const char * FILTR_NIGHT_DELTA34_ENABLE[5];
    static const char * FILTR_MIN_NIGHT_DELTA34[5];
    static const char * FILTR_MAX_NIGHT_DELTA34[5];
    static const char * FILTR_CLOUD_BORDER_ENABLE[5];
    static const char * FILTR_CLOUD_BORDER_WINDOW_SIZE[5];
    static const char * FILTR_MAX_FILTERED_PERCENT[5];
    static const char * FILTR_STAT_ENABLE[5];
};

class TFiltrCloudy {
public:

    TFiltrCloudy( char ParametrsFiltrCloudyDir[MAX_PATH], double j_d, int month, char * mcloudy, TStraightReferencer * sr,
                  bool availableChs[5], short * calibrData[5], uint16_t maxPixelValue[5], double ka[5], double kb[5] );

    virtual ~TFiltrCloudy();

    friend void print_log( const char * );

	friend void errorMessage( const char * );

    void readCfg( char ParametrsFiltrCloudyDir[MAX_PATH], int month ) throw( TAccessExc, TRequestExc );

    long filtr_processing( long, long, long, long );

    long filtr_processing_for_chips( long, long, long, long, uint8_t );

    void defineRegionFiltrCloudy( long x, long y );


    // Значение, которым мы будем отмечать отфильтрованные пиксели
    static const char lostValue;

    TRegionFiltrCloudy fRFC; // Параметры регионов для фильтрации облачности.

private:

    void readFiltrParams( TCfg & ) throw ( TRequestExc );

    int readFlg( TCfg &, const char *, int & ) throw ( TRequestExc );

    int readDbl( TCfg &, const char *, double & ) throw ( TRequestExc );

    int readInt( TCfg &, const char *, int & ) throw ( TRequestExc );

    double angle( int, int );

    TFiltrParams FP;

    TStraightReferencer * fSR;

    double julian_date;

    bool fAvailableChannels[5];

    short * fCalibrData[5];    // Калиброванные данные 1, 2, 3, 4, 5 каналов AVHRR с младшими битами.

    uint16_t fmaxPixValue[5];

    double fka[5];
    double fkb[5];

    char * fCloudyMask;  // маска облачности изображения.

    char * filtr_stat;

    TAstronom fAstronom;

    uint8_t freg;

    int albedo1Test( int i, int j, double ang );
    int albedo2Test( int i, int j, double ang );
    int ndviTest( int i, int j, double ang );
    int tempTest( int i, int j, double ang );
    int delta34Test( int scan, int j, double ang );
    int delta35Test( int scan, int j, double ang );
    int delta45Test( int scan, int j, double ang );
    int deltaTest( int i, int j, double ang,
                   short* in1, short* in2, double maxPix1, double maxPix2,
                   double ka1, double ka2, double kb1, double kb2,
                   double min_delta, double max_delta );
//    void borderProcessing( int win_size, double max_persent, int *stat );
};

/*
// Значение, которым мы будем отмечать отфильтрованные пиксели
const char TFiltrCloudy::lostValue = 1;

const double TFiltrParams::sinalpha_minlimit=10.0*DR;

const char * TFiltrParams::FILTR_ALBEDO_1_ENABLE[5] = {
"FILTR_PRIMORYE_ALBEDO_1_ENABLE",
"FILTR_JAPAN_KOREA_ALBEDO_1_ENABLE",
"FILTR_MATERIK_LIKES_ALBEDO_1_ENABLE",
"FILTR_SAHALIN_KURILS_ALBEDO_1_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_ALBEDO_1_ENABLE" };

const char * TFiltrParams::FILTR_MAX_ALBEDO_1_VALUE[5] = {
"FILTR_PRIMORYE_MAX_ALBEDO_1_VALUE",
"FILTR_JAPAN_KOREA_MAX_ALBEDO_1_VALUE",
"FILTR_MATERIK_LIKES_MAX_ALBEDO_1_VALUE",
"FILTR_SAHALIN_KURILS_MAX_ALBEDO_1_VALUE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_ALBEDO_1_VALUE" };

const char * TFiltrParams::FILTR_ALBEDO_2_ENABLE[5] = {
"FILTR_PRIMORYE_ALBEDO_2_ENABLE",
"FILTR_JAPAN_KOREA_ALBEDO_2_ENABLE",
"FILTR_MATERIK_LIKES_ALBEDO_2_ENABLE",
"FILTR_SAHALIN_KURILS_ALBEDO_2_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_ALBEDO_2_ENABLE" };

const char * TFiltrParams::FILTR_MAX_ALBEDO_2_VALUE[5] = {
"FILTR_PRIMORYE_MAX_ALBEDO_2_VALUE",
"FILTR_JAPAN_KOREA_MAX_ALBEDO_2_VALUE",
"FILTR_MATERIK_LIKES_MAX_ALBEDO_2_VALUE",
"FILTR_SAHALIN_KURILS_MAX_ALBEDO_2_VALUE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_ALBEDO_2_VALUE" };

const char * TFiltrParams::FILTR_NDVI_ENABLE[5] = {
"FILTR_PRIMORYE_NDVI_ENABLE",
"FILTR_JAPAN_KOREA_NDVI_ENABLE",
"FILTR_MATERIK_LIKES_NDVI_ENABLE",
"FILTR_SAHALIN_KURILS_NDVI_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_NDVI_ENABLE" };

const char * TFiltrParams::FILTR_MIN_NDVI_VALUE[5] = {
"FILTR_PRIMORYE_MIN_NDVI_VALUE",
"FILTR_JAPAN_KOREA_MIN_NDVI_VALUE",
"FILTR_MATERIK_LIKES_MIN_NDVI_VALUE",
"FILTR_SAHALIN_KURILS_MIN_NDVI_VALUE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_NDVI_VALUE" };

const char * TFiltrParams::FILTR_MAX_NDVI_VALUE[5] = {
"FILTR_PRIMORYE_MAX_NDVI_VALUE",
"FILTR_JAPAN_KOREA_MAX_NDVI_VALUE",
"FILTR_MATERIK_LIKES_MAX_NDVI_VALUE",
"FILTR_SAHALIN_KURILS_MAX_NDVI_VALUE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_NDVI_VALUE" };

const char * TFiltrParams::FILTR_ASSENT_ALBEDO_ENABLE[5] = {
"FILTR_PRIMORYE_ASSENT_ALBEDO_ENABLE",
"FILTR_JAPAN_KOREA_ASSENT_ALBEDO_ENABLE",
"FILTR_MATERIK_LIKES_ASSENT_ALBEDO_ENABLE",
"FILTR_SAHALIN_KURILS_ASSENT_ALBEDO_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_ASSENT_ALBEDO_ENABLE" };

const char * TFiltrParams::FILTR_TEMP_ENABLE[5] = {
"FILTR_PRIMORYE_TEMP_ENABLE",
"FILTR_JAPAN_KOREA_TEMP_ENABLE",
"FILTR_MATERIK_LIKES_TEMP_ENABLE",
"FILTR_SAHALIN_KURILS_TEMP_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_TEMP_ENABLE" };

const char * TFiltrParams::FILTR_MIN_TEMP[5] = {
"FILTR_PRIMORYE_MIN_TEMP",
"FILTR_JAPAN_KOREA_MIN_TEMP",
"FILTR_MATERIK_LIKES_MIN_TEMP",
"FILTR_SAHALIN_KURILS_MIN_TEMP",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_TEMP" };

const char * TFiltrParams::FILTR_MAX_TEMP[5] = {
"FILTR_PRIMORYE_MAX_TEMP",
"FILTR_JAPAN_KOREA_MAX_TEMP",
"FILTR_MATERIK_LIKES_MAX_TEMP",
"FILTR_SAHALIN_KURILS_MAX_TEMP",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_TEMP" };

const char * TFiltrParams::FILTR_DAY_DELTA45_ENABLE[5] = {
"FILTR_PRIMORYE_DAY_DELTA45_ENABLE",
"FILTR_JAPAN_KOREA_DAY_DELTA45_ENABLE",
"FILTR_MATERIK_LIKES_DAY_DELTA45_ENABLE",
"FILTR_SAHALIN_KURILS_DAY_DELTA45_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_DAY_DELTA45_ENABLE" };

const char * TFiltrParams::FILTR_MIN_DAY_DELTA45[5] = {
"FILTR_PRIMORYE_MIN_DAY_DELTA45",
"FILTR_JAPAN_KOREA_MIN_DAY_DELTA45",
"FILTR_MATERIK_LIKES_MIN_DAY_DELTA45",
"FILTR_SAHALIN_KURILS_MIN_DAY_DELTA45",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_DAY_DELTA45" };

const char * TFiltrParams::FILTR_MAX_DAY_DELTA45[5] = {
"FILTR_PRIMORYE_MAX_DAY_DELTA45",
"FILTR_JAPAN_KOREA_MAX_DAY_DELTA45",
"FILTR_MATERIK_LIKES_MAX_DAY_DELTA45",
"FILTR_SAHALIN_KURILS_MAX_DAY_DELTA45",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_DAY_DELTA45" };

const char * TFiltrParams::FILTR_NIGHT_DELTA45_ENABLE[5] = {
"FILTR_PRIMORYE_NIGHT_DELTA45_ENABLE",
"FILTR_JAPAN_KOREA_NIGHT_DELTA45_ENABLE",
"FILTR_MATERIK_LIKES_NIGHT_DELTA45_ENABLE",
"FILTR_SAHALIN_KURILS_NIGHT_DELTA45_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_NIGHT_DELTA45_ENABLE" };

const char * TFiltrParams::FILTR_MIN_NIGHT_DELTA45[5] = {
"FILTR_PRIMORYE_MIN_NIGHT_DELTA45",
"FILTR_JAPAN_KOREA_MIN_NIGHT_DELTA45",
"FILTR_MATERIK_LIKES_MIN_NIGHT_DELTA45",
"FILTR_SAHALIN_KURILS_MIN_NIGHT_DELTA45",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_NIGHT_DELTA45" };

const char * TFiltrParams::FILTR_MAX_NIGHT_DELTA45[5] = {
"FILTR_PRIMORYE_MAX_NIGHT_DELTA45",
"FILTR_JAPAN_KOREA_MAX_NIGHT_DELTA45",
"FILTR_MATERIK_LIKES_MAX_NIGHT_DELTA45",
"FILTR_SAHALIN_KURILS_MAX_NIGHT_DELTA45",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_NIGHT_DELTA45" };

const char * TFiltrParams::FILTR_DAY_DELTA34_ENABLE[5] = {
"FILTR_PRIMORYE_DAY_DELTA34_ENABLE",
"FILTR_JAPAN_KOREA_DAY_DELTA34_ENABLE",
"FILTR_MATERIK_LIKES_DAY_DELTA34_ENABLE",
"FILTR_SAHALIN_KURILS_DAY_DELTA34_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_DAY_DELTA34_ENABLE" };

const char * TFiltrParams::FILTR_MIN_DAY_DELTA34[5] = {
"FILTR_PRIMORYE_MIN_DAY_DELTA34",
"FILTR_JAPAN_KOREA_MIN_DAY_DELTA34",
"FILTR_MATERIK_LIKES_MIN_DAY_DELTA34",
"FILTR_SAHALIN_KURILS_MIN_DAY_DELTA34",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_DAY_DELTA34" };

const char * TFiltrParams::FILTR_MAX_DAY_DELTA34[5] = {
"FILTR_PRIMORYE_MAX_DAY_DELTA34",
"FILTR_JAPAN_KOREA_MAX_DAY_DELTA34",
"FILTR_MATERIK_LIKES_MAX_DAY_DELTA34",
"FILTR_SAHALIN_KURILS_MAX_DAY_DELTA34",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_DAY_DELTA34" };

const char * TFiltrParams::FILTR_NIGHT_DELTA34_ENABLE[5] = {
"FILTR_PRIMORYE_NIGHT_DELTA34_ENABLE",
"FILTR_JAPAN_KOREA_NIGHT_DELTA34_ENABLE",
"FILTR_MATERIK_LIKES_NIGHT_DELTA34_ENABLE",
"FILTR_SAHALIN_KURILS_NIGHT_DELTA34_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_NIGHT_DELTA34_ENABLE" };

const char * TFiltrParams::FILTR_MIN_NIGHT_DELTA34[5] = {
"FILTR_PRIMORYE_MIN_NIGHT_DELTA34",
"FILTR_JAPAN_KOREA_MIN_NIGHT_DELTA34",
"FILTR_MATERIK_LIKES_MIN_NIGHT_DELTA34",
"FILTR_SAHALIN_KURILS_MIN_NIGHT_DELTA34",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_NIGHT_DELTA34" };

const char * TFiltrParams::FILTR_MAX_NIGHT_DELTA34[5] = {
"FILTR_PRIMORYE_MAX_NIGHT_DELTA34",
"FILTR_JAPAN_KOREA_MAX_NIGHT_DELTA34",
"FILTR_MATERIK_LIKES_MAX_NIGHT_DELTA34",
"FILTR_SAHALIN_KURILS_MAX_NIGHT_DELTA34",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_NIGHT_DELTA34" };

const char * TFiltrParams::FILTR_DAY_DELTA35_ENABLE[5] = {
"FILTR_PRIMORYE_DAY_DELTA35_ENABLE",
"FILTR_JAPAN_KOREA_DAY_DELTA35_ENABLE",
"FILTR_MATERIK_LIKES_DAY_DELTA35_ENABLE",
"FILTR_SAHALIN_KURILS_DAY_DELTA35_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_DAY_DELTA35_ENABLE" };

const char * TFiltrParams::FILTR_MIN_DAY_DELTA35[5] = {
"FILTR_PRIMORYE_MIN_DAY_DELTA35",
"FILTR_JAPAN_KOREA_MIN_DAY_DELTA35",
"FILTR_MATERIK_LIKES_MIN_DAY_DELTA35",
"FILTR_SAHALIN_KURILS_MIN_DAY_DELTA35",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_DAY_DELTA35" };

const char * TFiltrParams::FILTR_MAX_DAY_DELTA35[5] = {
"FILTR_PRIMORYE_MAX_DAY_DELTA35",
"FILTR_JAPAN_KOREA_MAX_DAY_DELTA35",
"FILTR_MATERIK_LIKES_MAX_DAY_DELTA35",
"FILTR_SAHALIN_KURILS_MAX_DAY_DELTA35",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_DAY_DELTA35" };

const char * TFiltrParams::FILTR_NIGHT_DELTA35_ENABLE[5] = {
"FILTR_PRIMORYE_NIGHT_DELTA35_ENABLE",
"FILTR_JAPAN_KOREA_NIGHT_DELTA35_ENABLE",
"FILTR_MATERIK_LIKES_NIGHT_DELTA35_ENABLE",
"FILTR_SAHALIN_KURILS_NIGHT_DELTA35_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_NIGHT_DELTA35_ENABLE" };

const char * TFiltrParams::FILTR_MIN_NIGHT_DELTA35[5] = {
"FILTR_PRIMORYE_MIN_NIGHT_DELTA35",
"FILTR_JAPAN_KOREA_MIN_NIGHT_DELTA35",
"FILTR_MATERIK_LIKES_MIN_NIGHT_DELTA35",
"FILTR_SAHALIN_KURILS_MIN_NIGHT_DELTA35",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MIN_NIGHT_DELTA35" };

const char * TFiltrParams::FILTR_MAX_NIGHT_DELTA35[5] = {
"FILTR_PRIMORYE_MAX_NIGHT_DELTA35",
"FILTR_JAPAN_KOREA_MAX_NIGHT_DELTA35",
"FILTR_MATERIK_LIKES_MAX_NIGHT_DELTA35",
"FILTR_SAHALIN_KURILS_MAX_NIGHT_DELTA35",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_NIGHT_DELTA35" };

const char * TFiltrParams::FILTR_CLOUD_BORDER_ENABLE[5] = {
"FILTR_PRIMORYE_CLOUD_BORDER_ENABLE",
"FILTR_JAPAN_KOREA_CLOUD_BORDER_ENABLE",
"FILTR_MATERIK_LIKES_CLOUD_BORDER_ENABLE",
"FILTR_SAHALIN_KURILS_CLOUD_BORDER_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_CLOUD_BORDER_ENABLE" };

const char * TFiltrParams::FILTR_CLOUD_BORDER_WINDOW_SIZE[5] = {
"FILTR_PRIMORYE_CLOUD_BORDER_WINDOW_SIZE",
"FILTR_JAPAN_KOREA_CLOUD_BORDER_WINDOW_SIZE",
"FILTR_MATERIK_LIKES_CLOUD_BORDER_WINDOW_SIZE",
"FILTR_SAHALIN_KURILS_CLOUD_BORDER_WINDOW_SIZE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_CLOUD_BORDER_WINDOW_SIZE" };

const char * TFiltrParams::FILTR_MAX_FILTERED_PERCENT[5] = {
"FILTR_PRIMORYE_MAX_FILTERED_PERCENT",
"FILTR_JAPAN_KOREA_MAX_FILTERED_PERCENT",
"FILTR_MATERIK_LIKES_MAX_FILTERED_PERCENT",
"FILTR_SAHALIN_KURILS_MAX_FILTERED_PERCENT",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_MAX_FILTERED_PERCENT" };

const char * TFiltrParams::FILTR_STAT_ENABLE[5] = {
"FILTR_PRIMORYE_STAT_ENABLE",
"FILTR_JAPAN_KOREA_STAT_ENABLE",
"FILTR_MATERIK_LIKES_STAT_ENABLE",
"FILTR_SAHALIN_KURILS_STAT_ENABLE",
"FILTR_KAMCHATKA_OHOTSKOE_SEA_STAT_ENABLE" };
*/

#endif
