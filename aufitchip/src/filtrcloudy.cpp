/*-----------------------------------------------------------------------------
 filtrcloudy.cpp
------------------------------------------------------------------------------*/
#include <tc_config.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <orbmodel.hpp>
#include <c_lib.hpp>
#include <filtrcloudy.hpp>

using namespace libconfig;
using namespace std;

// Значение, которым мы будем отмечать отфильтрованные пиксели
const char TFiltrCloudy::lostValue = 1;

const double TFiltrParams::sinalpha_minlimit = 10.0*DR;

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

void errorMessage( const char * error_message_text ) ;

TFiltrCloudy::TFiltrCloudy( std::string ParametrsFiltrCloudyDir, double j_d, int month, char * mcloudy, TStraightReferencer * sr,
							bool availableChs[5], short * calibrData[5], uint16_t maxPixelValue[5], double ka[5], double kb[5] ) :
                            julian_date( j_d )
                            ,fCloudyMask( mcloudy )
                            ,fSR( sr )
                            ,freg( 0 )
                            ,filtr_stat( 0 )
{
    for( int i = 0; i < 5; i++ )
        if( (fAvailableChannels[i] = availableChs[i]) ){
            fCalibrData[i] = calibrData[i];
            fmaxPixValue[i] = maxPixelValue[i];
            fka[i] = ka[i];
            fkb[i] = kb[i];
        }

    filtr_stat = new char [N_MASK_VALUE];

    try{
        fRFC.loadRegionsforFiltrCloudyCfg( ParametrsFiltrCloudyDir );
        readCfg( ParametrsFiltrCloudyDir, month );
    }
    catch( TAccessExc ae ){
        errorMessage( ae.what() );
    }
    catch( TRequestExc re ){
        errorMessage( re.what() );
    }
}


TFiltrCloudy::~TFiltrCloudy()
{
    delete [] filtr_stat;
}


void TFiltrCloudy::readCfg( string &ParametrsFiltrCloudyDir, int month ) throw( TAccessExc, TRequestExc )
{
	TAccessExc ae1( 1, "ERROR: Ошибка доступа к cfg-файлу параметров фильтрации облачности!!!" );

	char drive[MAX_DRIVE], dir[MAX_DIR], path[MAX_PATH];
	// Конструирование полного имени cfg-файла параметров регионов для фильтрации облачности.
	// Проверка существования каталога ParametrsFiltrCloudyDir.
	if( check_dir( ParametrsFiltrCloudyDir ) == 0 )
		splitpath( ParametrsFiltrCloudyDir, drive, dir, 0, 0 );
	else{
		getcwd( path, MAX_PATH );
		int t = strlen(path);
		if( path[t-1] != DIRD ) {
			path[t] = DIRD;
			path[t+1] = '\0';
		}
		splitpath( path, drive, dir, 0, 0 );
	}

    char fname[128];
    sprintf( fname, "PARAMETRSFILTRCLOUDY.CFG%cfiltr_%d", DIRD, month+1 );

    makepath( path, drive, dir, fname, "cfg" );

    Config  fCfg;
    try{
        fCfg.readFile( path );
    }
    catch ( ... ){
        throw ae1;
    }

    // Здесь должно быть чтение параметров фильтрации

    try{
        readFiltrParams( fCfg.getRoot()["Filtr"] );
    }
    catch ( TRequestExc & re1 ){
        throw;
    }
}


void TFiltrCloudy::readFiltrParams( const Setting &cfg ) throw ( TRequestExc )
{
    TRequestExc re1( 1, "ERROR: Недопустимый формат cfg-файла параметров фильтрации облачности!!!" );

    for( int i = 0; i < FP.Size; i++ )
	{
		if(

			!(cfg.lookupValue(FP.FILTR_ALBEDO_1_ENABLE[i]			 , FP.albedo1_flag[i]              ) &&
            cfg.lookupValue(FP.FILTR_ALBEDO_2_ENABLE[i]				 , FP.albedo2_flag[i]			   ) &&
			cfg.lookupValue( FP.FILTR_ALBEDO_2_ENABLE[i]             , FP.albedo2_flag[i]              ) &&
            cfg.lookupValue( FP.FILTR_NDVI_ENABLE[i]                 , FP.ndvi_flag[i]                 ) &&
            cfg.lookupValue( FP.FILTR_ASSENT_ALBEDO_ENABLE[i]        , FP.assent_flag[i]               ) &&
            cfg.lookupValue( FP.FILTR_MAX_ALBEDO_1_VALUE[i]          , FP.max_albedo1[i]               ) &&
            cfg.lookupValue( FP.FILTR_MAX_ALBEDO_2_VALUE[i]          , FP.max_albedo2[i]               ) &&
            cfg.lookupValue( FP.FILTR_MIN_NDVI_VALUE[i]              , FP.min_ndvi[i]                  ) &&
            cfg.lookupValue( FP.FILTR_MAX_NDVI_VALUE[i]              , FP.max_ndvi[i]                  ) &&
            cfg.lookupValue( FP.FILTR_TEMP_ENABLE[i]                 , FP.temp_flag[i]                 ) &&
            cfg.lookupValue( FP.FILTR_MAX_TEMP[i]                    , FP.max_temp[i]                  ) &&
            cfg.lookupValue( FP.FILTR_MIN_TEMP[i]                    , FP.min_temp[i]                  ) &&
            cfg.lookupValue( FP.FILTR_DAY_DELTA45_ENABLE[i]          , FP.day_delta45_flag[i]          ) &&
            cfg.lookupValue( FP.FILTR_MIN_DAY_DELTA45[i]             , FP.day_min_delta45[i]           ) &&
            cfg.lookupValue( FP.FILTR_MAX_DAY_DELTA45[i]             , FP.day_max_delta45[i]           ) &&
            cfg.lookupValue( FP.FILTR_NIGHT_DELTA45_ENABLE[i]        , FP.night_delta45_flag[i]        ) &&
            cfg.lookupValue( FP.FILTR_MIN_NIGHT_DELTA45[i]           , FP.night_min_delta45[i]         ) &&
            cfg.lookupValue( FP.FILTR_MAX_NIGHT_DELTA45[i]           , FP.night_max_delta45[i]         ) &&
            cfg.lookupValue( FP.FILTR_DAY_DELTA34_ENABLE[i]          , FP.day_delta34_flag[i]          ) &&
            cfg.lookupValue( FP.FILTR_MIN_DAY_DELTA34[i]             , FP.day_min_delta34[i]           ) &&
            cfg.lookupValue( FP.FILTR_MAX_DAY_DELTA34[i]             , FP.day_max_delta34[i]           ) &&
            cfg.lookupValue( FP.FILTR_NIGHT_DELTA34_ENABLE[i]        , FP.night_delta34_flag[i]        ) &&
            cfg.lookupValue( FP.FILTR_MIN_NIGHT_DELTA34[i]           , FP.night_min_delta34[i]         ) &&
            cfg.lookupValue( FP.FILTR_MAX_NIGHT_DELTA34[i]           , FP.night_max_delta34[i]         ) &&
            cfg.lookupValue( FP.FILTR_DAY_DELTA35_ENABLE[i]          , FP.day_delta35_flag[i]          ) &&
            cfg.lookupValue( FP.FILTR_MIN_DAY_DELTA35[i]             , FP.day_min_delta35[i]           ) &&
            cfg.lookupValue( FP.FILTR_MAX_DAY_DELTA35[i]             , FP.day_max_delta35[i]           ) &&
            cfg.lookupValue( FP.FILTR_NIGHT_DELTA35_ENABLE[i]        , FP.night_delta35_flag[i]        ) &&
            cfg.lookupValue( FP.FILTR_MIN_NIGHT_DELTA35[i]           , FP.night_min_delta35[i]         ) &&
            cfg.lookupValue( FP.FILTR_MAX_NIGHT_DELTA35[i]           , FP.night_max_delta35[i]         ) &&
            cfg.lookupValue( FP.FILTR_CLOUD_BORDER_ENABLE[i]         , FP.cloud_border_flag[i]         ) &&
            cfg.lookupValue(FP.FILTR_CLOUD_BORDER_WINDOW_SIZE[i]    , FP.cloud_border_win_size[i]     ) &&
            cfg.lookupValue( FP.FILTR_MAX_FILTERED_PERCENT[i]        , FP.max_filtered_percent[i]      ) &&
            cfg.lookupValue( FP.FILTR_STAT_ENABLE[i]                 , FP.stat_flag[i]                 ))
          )throw re1;
    }
}

bool defineRegionFiltr( long x, long y, size_t count,  TCoordinates *X,  TCoordinates *Y)
{
	#warning не забыть избавиться от дефайнов
    for (int i =0; i + 1< count; i +=2 )
	{
		if( x > X[i] && x < X[i + 1]
			&& y > Y[i] && y < Y[i +1])
			return false;
	}
	return true;
}

void TFiltrCloudy::defineRegionFiltrCloudy( long x, long y )
{
	// Приморье.
	#warning не забыть избавиться от дефайнов
	if (defineRegionFiltr(x, y, PRIMORYE_COUNT, fRFC.XPrimorye, fRFC.YPrimorye) )
		freg = 0;
	 // Япония и Корея.
	else if (defineRegionFiltr(x, y, JAPAN_KOREA_COUNT, fRFC.XJapanKorea, fRFC.YJapanKorea))
		freg = 1;

	// Материковые озера.
    else if (defineRegionFiltr(x, y, MATERIK_LIKES_COUNT, fRFC.XMaterikLikes, fRFC.YMaterikLikes))
		freg = 2;

    // Сахалин и Курилы.
    else if (defineRegionFiltr(x, y, SAHALIN_KURILS_COUNT, fRFC.XSahalinKurils, fRFC.YSahalinKurils))
		freg = 3;

	// Камчатка и Охотское море.
    else if (defineRegionFiltr(x, y, KAMCHATKA_OHOTSKOE_COUNT, fRFC.XKamchatkaOhotskoe , fRFC.YKamchatkaOhotskoe ))
		freg = 4;

    freg = 0;
}


/*
 * Фильтрация облачности для чипов.
 */
long TFiltrCloudy::filtr_processing_for_chips( long x1, long y1, long x2, long y2, uint8_t reg )
{
    freg = reg;
    long perc = 0L;
    for ( int scan = y1; scan <= y2; scan++ ){
        int col1 = x1;
        int col2 = x2;
        double ang1 = angle( scan, col1 );
        double ang2 = angle( scan, col2 );
        for ( int j = x1; j <= x2; j++ ){
            // Определяем угол восхождения на Солнце
            double ang;
            if( j == col1 ){
                ang = ang1;
            }
            else{ if ( j == col2 ){
                      ang = ang2;
                 }
                 else{
                      ang = ang1 + ( j - col1 ) * ( ang2 - ang1 ) / double( x2 - x1 + 1 );
                 }
            }
            // угол восхождения на Солнце определен

            int ind = scan * 2048 + j;

            {
                //  маска, показывающая каким фильтрам удовлетворил
                //  текущий пиксель
                char mask_filtr = 0;

                //  Фильтрация по альбедо 1-го канала.
                if( fAvailableChannels[0] && FP.albedo1_flag[freg] && ang > 7. ){
                    if( albedo1Test( scan, j, ang ) ){
                        fCloudyMask[ind] = lostValue;
                        mask_filtr = MASK_ALBEDO_1_FILTR;
                    }
                }

                //  Фильтрация по альбедо 2-го канала.
                if( fAvailableChannels[1] && FP.albedo2_flag[freg] && ang > 7. ){
                    if( albedo2Test( scan, j, ang ) ){
                        fCloudyMask[ind] = lostValue;
                        mask_filtr |= MASK_ALBEDO_2_FILTR;
                    }
                }

                //  Фильтрация по NDVI (вегетационный индекс).
                if( fAvailableChannels[0] && fAvailableChannels[1] && FP.ndvi_flag[freg] && ang > 7. ){
                    if( ndviTest( scan, j, ang ) ){
                        fCloudyMask[ind] = lostValue;
                        mask_filtr |= MASK_NDVI_FILTR;
                    }
                }

                //  Фильтрация по температуре.
                if( fAvailableChannels[3] && FP.temp_flag[freg] ){
                    if( tempTest( scan, j, ang ) ){
                        fCloudyMask[ind] = lostValue;
                        mask_filtr |= MASK_TEMP_FILTR;
                    }
                }

                //  Фильтрация по разности третьего - четвертого каналов.
                if( fAvailableChannels[2] && fAvailableChannels[3] && (FP.day_delta34_flag[freg] || FP.night_delta34_flag[freg]) ){
                    if( delta34Test( scan, j, ang ) ){
                        fCloudyMask[ind] = lostValue;
                        mask_filtr |= MASK_34_DELTA_FILTR;
                    }
                }

                //  Фильтрация по разности третьего - пятого каналов.
                if( fAvailableChannels[2] && fAvailableChannels[4] && (FP.day_delta35_flag[freg] || FP.night_delta35_flag[freg]) ){
                    if( delta35Test( scan, j, ang ) ){
                        fCloudyMask[ind] = lostValue;
                        mask_filtr |= MASK_35_DELTA_FILTR;
                    }
                }

                //  Фильтрация по разности четвертого - пятого каналов.
                if( fAvailableChannels[3] && fAvailableChannels[4] && (FP.day_delta45_flag[freg] || FP.night_delta45_flag[freg]) ){
                    if( delta45Test( scan, j, ang ) ){
                        fCloudyMask[ind] = lostValue;
                        mask_filtr |= MASK_45_DELTA_FILTR;
                    }
                }

                if( fCloudyMask[ind] == lostValue ){
                    perc++;
                    if( FP.stat_flag[freg] ) fCloudyMask[ind] = mask_filtr;
                }

                if( filtr_stat != 0 ) filtr_stat[mask_filtr]++;   // Для статистики
            }
        }
    }   // Конец большого цикла по всем пикселям чипа.

    if( FP.cloud_border_flag[freg] ){
//        borderProcessing( p.cloud_border_win_size[freg], p.max_filtered_percent[freg], &(filtr_stat[MASK_BORDER_FILTR]) );
    }

    return perc;
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
 переменные julian_date и fSR.
\*----------------------------------------*/
double TFiltrCloudy::angle( int nscan, int col )
{
    double lat, lon;
    fSR->xy2ll( col, nscan, &lon, &lat );
    lon = -lon;
    return ( fAstronom.shcrds( julian_date, lon, lat ) );
}


int TFiltrCloudy::albedo1Test( int i, int j, double ang )
{
    int ind = i*2048 + j;
    if( fCalibrData[0][ind] >= 0 && fCalibrData[0][ind] <= fmaxPixValue[0] ){
        double value = fCalibrData[0][ind] * fka[0] + fkb[0];
        if( FP.assent_flag[freg] ){
//            value += p.alb_precision;		// Это старый, классический код
//            value /= sin(ang*DR);
            const double minAngle = -4.;
            const double angScale = .2659;
            const double expScale = .4;
            const double constLimit = 1.;
			if( ang < minAngle ) return 0;
            double lim1 = 1. / angScale * log( constLimit / expScale );
            if( ang < lim1 ){
                double a = exp( angScale * ang ) * expScale;
                if( (value - .1) <= a ) return 0;
                return 1;
            }
            if( ang < 1.2 * RD / FP.max_albedo1[freg] ){
                if( (value - .1) <= constLimit ) return 0;
                return 1;
            }
            value -= .1;
            value /= sin( ang * DR );
        }
        if( value > FP.max_albedo1[freg] ) {
            return 1;
        }
    }
    else if ( fCalibrData[0][ind] < 0 || fCalibrData[0][ind] > fmaxPixValue[0] ){
        return 1;
    }
    return 0;
}


int TFiltrCloudy::albedo2Test( int i, int j, double ang )
{
    int ind = i*2048 + j;
    if( fCalibrData[1][ind] >= 0  && fCalibrData[1][ind] <= fmaxPixValue[1] ){
        double value = fCalibrData[1][ind] * fka[1] + fkb[1];
        if( FP.assent_flag[freg] ){
//            value += p.alb_precision;		// Это старый, классический код
//            value /= sin(ang*DR);
            const double minAngle = -4.;
            const double angScale = .2659;
            const double expScale = .4;
            const double constLimit = 1.;
			if( ang < minAngle ) return 0;
            double lim1 = 1. / angScale * log( constLimit / expScale );
            if( ang < lim1 ){
                double a = exp( angScale * ang ) * expScale;
                if( (value - .1) <= a ) return 0;
                return 1;
            }
            if( ang < 1.2 * RD / FP.max_albedo2[freg] ){
                if( (value - .1) <= constLimit ) return 0;
                return 1;
            }
            value -= .1;
            value /= sin( ang * DR );
        }
        if( value > FP.max_albedo2[freg] ) {
            return 1;
        }
    }
    else if ( fCalibrData[1][ind] < 0 || fCalibrData[1][ind] > fmaxPixValue[1] ){
        return 1;
    }
    return 0;
}


int TFiltrCloudy::ndviTest( int i, int j, double ang )
{
    int ind = i*2048 + j;
    if( (fCalibrData[0][ind] >= 0  && fCalibrData[0][ind] <= fmaxPixValue[0]) &&
        (fCalibrData[1][ind] >= 0  && fCalibrData[1][ind] <= fmaxPixValue[1]) ){
        double value = double( (fCalibrData[1][ind] * fka[1] + fkb[1]) - (fCalibrData[0][ind] * fka[0] + fkb[0]) ) /
                       double( (fCalibrData[0][ind] * fka[0] + fkb[0]) + (fCalibrData[1][ind] * fka[1] + fkb[1]) );
        if( (value > FP.min_ndvi[freg]) && (value < FP.max_ndvi[freg]) ) {
            return 1;
        }
    }
    else if ( fCalibrData[0][ind] < 0 || fCalibrData[0][ind] > fmaxPixValue[0] ||
              fCalibrData[1][ind] < 0 || fCalibrData[1][ind] > fmaxPixValue[1] ){
        return 1;
    }

    return 0;
}


int TFiltrCloudy::tempTest( int i, int j, double ang )
{
    int ind = i*2048 + j;
    if( fCalibrData[3][ind] >= 0  && fCalibrData[3][ind] <= fmaxPixValue[3] ){
        double value = fCalibrData[3][ind] * fka[3] + fkb[3];
        if( value < FP.min_temp[freg] || value > FP.max_temp[freg] ){
            return 1;
        }
    }
    else if ( fCalibrData[3][ind] < 0 || fCalibrData[3][ind] > fmaxPixValue[3] ){
        return 1;
    }
    return 0;
}

int TFiltrCloudy::deltaTest( int i, int j, double ang,
        short* in1, short* in2, double maxPix1, double maxPix2,
        double ka1, double ka2, double kb1, double kb2,
        double min_delta, double max_delta )
{
    int ind = i*2048 + j;
    if( in1[ind] >= 0 && in2[ind] >= 0 && in1[ind] <= maxPix1 && in2[ind] <= maxPix2 ) {
        double value1 = in1[ind] * ka1 + kb1;
        double value2 = in2[ind] * ka2 + kb2;
        double delta = value1 - value2;
        if( delta < min_delta || delta > max_delta ){
                return 1;
        }
    }
    else if( in1[ind] < 0 ||
              in1[ind] > maxPix1 ||
              in2[ind] < 0 ||
              in2[ind] > maxPix2 ){
        return 1;
    }

    return 0;
}

int TFiltrCloudy::delta34Test( int scan, int j, double ang )
{
    if( ang > 3. ){
        if( FP.day_delta34_flag[freg] ){
            return deltaTest( scan, j, ang,
                        fCalibrData[2], fCalibrData[3],
                        fmaxPixValue[2], fmaxPixValue[3],
                        fka[2], fka[3], fkb[2], fkb[3],
                        FP.day_min_delta34[freg], FP.day_max_delta34[freg] );
        }
    }
    else {
        if( FP.night_delta34_flag[freg] ){
            return deltaTest( scan, j, ang,
                        fCalibrData[2], fCalibrData[3],
                        fmaxPixValue[2], fmaxPixValue[3],
                        fka[2], fka[3], fkb[2], fkb[3],
                        FP.night_min_delta34[freg], FP.night_max_delta34[freg] );
        }
    }

    return 0;
}


int TFiltrCloudy::delta35Test( int scan, int j, double ang )
{
    if( ang > 3. ) {
        if( FP.day_delta35_flag[freg] ){
            return deltaTest( scan, j, ang,
                        fCalibrData[2], fCalibrData[4],
                        fmaxPixValue[2], fmaxPixValue[4],
                        fka[2], fka[4], fkb[2], fkb[4],
                        FP.day_min_delta35[freg], FP.day_max_delta35[freg] );
        }
    }
    else {
        if( FP.night_delta35_flag[freg] ){
            return deltaTest( scan, j, ang,
                        fCalibrData[2], fCalibrData[4],
                        fmaxPixValue[2], fmaxPixValue[4],
                        fka[2], fka[4], fkb[2], fkb[4],
                        FP.night_min_delta35[freg], FP.night_max_delta35[freg] );
        }
    }

    return 0;
}


int TFiltrCloudy::delta45Test( int scan, int j, double ang )
{
    if( ang > 3. ) {
        if( FP.day_delta45_flag[freg] ){
            return deltaTest( scan, j, ang,
                        fCalibrData[3], fCalibrData[4],
                        fmaxPixValue[3], fmaxPixValue[4],
                        fka[3], fka[4], fkb[3], fkb[4],
                        FP.day_min_delta45[freg], FP.day_max_delta45[freg] );
        }
    }
    else {
        if( FP.night_delta45_flag[freg] ){
            return deltaTest( scan, j, ang,
                        fCalibrData[3], fCalibrData[4],
                        fmaxPixValue[3], fmaxPixValue[4],
                        fka[3], fka[4], fkb[3], fkb[4],
                        FP.night_min_delta45[freg], FP.night_max_delta45[freg] );
        }
    }

    return 0;
}
