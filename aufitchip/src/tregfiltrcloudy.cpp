#include <tregfiltrcloudy.hpp>
#include <unistd.h>

long TRegionFiltrCloudy::NamberRegionsforFiltrCloudy = 0L;


long TRegionFiltrCloudy::X1Primorye = 0L, TRegionFiltrCloudy::Y1Primorye = 0L,
     TRegionFiltrCloudy::X2Primorye = 0L,TRegionFiltrCloudy::Y2Primorye = 0L,
     TRegionFiltrCloudy::X3Primorye = 0L,TRegionFiltrCloudy::Y3Primorye = 0L,
     TRegionFiltrCloudy::X4Primorye = 0L,TRegionFiltrCloudy::Y4Primorye = 0L,
     TRegionFiltrCloudy::X5Primorye = 0L,TRegionFiltrCloudy::Y5Primorye = 0L,
     TRegionFiltrCloudy::X6Primorye = 0L,TRegionFiltrCloudy::Y6Primorye = 0L,
     TRegionFiltrCloudy::X7Primorye = 0L,TRegionFiltrCloudy::Y7Primorye = 0L,
     TRegionFiltrCloudy::X8Primorye = 0L,TRegionFiltrCloudy::Y8Primorye = 0L;



long TRegionFiltrCloudy::X1JapanKorea = 0L, TRegionFiltrCloudy::Y1JapanKorea = 0L,
     TRegionFiltrCloudy::X2JapanKorea = 0L, TRegionFiltrCloudy::Y2JapanKorea = 0L,
     TRegionFiltrCloudy::X3JapanKorea = 0L, TRegionFiltrCloudy::Y3JapanKorea = 0L,
     TRegionFiltrCloudy::X4JapanKorea = 0L, TRegionFiltrCloudy::Y4JapanKorea = 0L,
     TRegionFiltrCloudy::X5JapanKorea = 0L, TRegionFiltrCloudy::Y5JapanKorea = 0L,
     TRegionFiltrCloudy::X6JapanKorea = 0L, TRegionFiltrCloudy::Y6JapanKorea = 0L;

long TRegionFiltrCloudy::X1MaterikLikes = 0L, TRegionFiltrCloudy::Y1MaterikLikes = 0L,
     TRegionFiltrCloudy::X2MaterikLikes = 3700L, TRegionFiltrCloudy::Y2MaterikLikes = 0L,
     TRegionFiltrCloudy::X3MaterikLikes = 3700L, TRegionFiltrCloudy::Y3MaterikLikes = 0L,
     TRegionFiltrCloudy::X4MaterikLikes = 4300L, TRegionFiltrCloudy::Y4MaterikLikes = 0L,
     TRegionFiltrCloudy::X5MaterikLikes = 4300L, TRegionFiltrCloudy::Y5MaterikLikes = 0L,
     TRegionFiltrCloudy::X6MaterikLikes = 4480L, TRegionFiltrCloudy::Y6MaterikLikes = 0L;

long TRegionFiltrCloudy::X1SahalinKurils = 0L, TRegionFiltrCloudy::Y1SahalinKurils = 0L,
     TRegionFiltrCloudy::X2SahalinKurils = 0L, TRegionFiltrCloudy::Y2SahalinKurils = 0L,
     TRegionFiltrCloudy::X3SahalinKurils = 0L, TRegionFiltrCloudy::Y3SahalinKurils = 0L,
     TRegionFiltrCloudy::X4SahalinKurils = 0L, TRegionFiltrCloudy::Y4SahalinKurils = 0L;

long TRegionFiltrCloudy::X1KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y1KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X2KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y2KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X3KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y3KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X4KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y4KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X5KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y5KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X6KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y6KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X7KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y7KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X8KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y8KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X9KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y9KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X10KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y10KamchatkaOhotskoe = 0L;

void TRegionFiltrCloudy :: loadRegionsforFiltrCloudyCfg( char ParametrsRegionsforFiltrCloudyDir[MAX_PATH] ) throw( TAccessExc ){
	TAccessExc ae1( 1, "ERROR: Ошибка доступа к cfg-файлу регионов для фильтрации облачности!!!" );

	char drive[MAX_DRIVE], dir[MAX_DIR], /*fname[MAX_FNAME],*/ path[MAX_PATH];
	// Конструирование полного имени cfg-файла параметров регионов для фильтрации облачности.
	// Проверка существования каталога ParametrsRegionsforFiltrCloudyDir.
	if( check_dir( ParametrsRegionsforFiltrCloudyDir ) == 0 ) splitpath( ParametrsRegionsforFiltrCloudyDir, drive, dir, 0, 0 );
	else{
		getcwd( path, MAX_PATH );
		splitpath( path, drive, dir, 0, 0 );
	}
	//!!!DEBUG
	//printf("%s\n",path);
	//printf("%s\n",drive);
	//printf("%s\n",dir);

    makepath( path, drive, dir, "regfiltrcloud", "cfg" );

	//printf("%s\n",path);

    //int i;
    TCfg * cfg;

    try{
        cfg = new TCfg( path );
    }
    catch(...){
        throw ae1;
    }

    if( cfg->containsParamWithKey( "NUMBER_REGIONS_FILTR_CLOUDY" ) ){
        NamberRegionsforFiltrCloudy = atoi( cfg->getValue( "NUMBER_REGIONS_FILTR_CLOUDY" ) );
    }

// Приморье
    if( cfg->containsParamWithKey( "X1_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X1Primorye = atoi( cfg->getValue( "X1_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y1_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y1Primorye = atoi( cfg->getValue( "Y1_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "X2_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X2Primorye = atoi( cfg->getValue( "X2_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y2_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y2Primorye = atoi( cfg->getValue( "Y2_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "X3_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X3Primorye = atoi( cfg->getValue( "X3_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y3_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y3Primorye = atoi( cfg->getValue( "Y3_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "X4_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X4Primorye = atoi( cfg->getValue( "X4_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y4_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y4Primorye = atoi( cfg->getValue( "Y4_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "X5_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X5Primorye = atoi( cfg->getValue( "X5_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y5_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y5Primorye = atoi( cfg->getValue( "Y5_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "X6_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X6Primorye = atoi( cfg->getValue( "X6_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y6_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y6Primorye = atoi( cfg->getValue( "Y6_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "X7_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X7Primorye = atoi( cfg->getValue( "X7_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y7_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y7Primorye = atoi( cfg->getValue( "Y7_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "X8_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        X8Primorye = atoi( cfg->getValue( "X8_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }
    if( cfg->containsParamWithKey( "Y8_REGION_FILTR_CLOUDY_PRIMORYE" ) ){
        Y8Primorye = atoi( cfg->getValue( "Y8_REGION_FILTR_CLOUDY_PRIMORYE" ) );
    }

// Япония и Корея
    if( cfg->containsParamWithKey( "X1_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        X1JapanKorea = atoi( cfg->getValue( "X1_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "Y1_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        Y1JapanKorea = atoi( cfg->getValue( "Y1_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "X2_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        X2JapanKorea = atoi( cfg->getValue( "X2_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "Y2_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        Y2JapanKorea = atoi( cfg->getValue( "Y2_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "X3_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        X3JapanKorea = atoi( cfg->getValue( "X3_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "Y3_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        Y3JapanKorea = atoi( cfg->getValue( "Y3_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "X4_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        X4JapanKorea = atoi( cfg->getValue( "X4_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "Y4_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        Y4JapanKorea = atoi( cfg->getValue( "Y4_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "X5_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        X5JapanKorea = atoi( cfg->getValue( "X5_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "Y5_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        Y5JapanKorea = atoi( cfg->getValue( "Y5_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "X6_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        X6JapanKorea = atoi( cfg->getValue( "X6_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }
    if( cfg->containsParamWithKey( "Y6_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) ){
        Y6JapanKorea = atoi( cfg->getValue( "Y6_REGION_FILTR_CLOUDY_JAPAN_KOREA" ) );
    }

// Материковые озера
    if( cfg->containsParamWithKey( "X1_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        X1MaterikLikes = atoi( cfg->getValue( "X1_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "Y1_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        Y1MaterikLikes = atoi( cfg->getValue( "Y1_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "X2_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        X2MaterikLikes = atoi( cfg->getValue( "X2_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "Y2_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        Y2MaterikLikes = atoi( cfg->getValue( "Y2_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "X3_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        X3MaterikLikes = atoi( cfg->getValue( "X3_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "Y3_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        Y3MaterikLikes = atoi( cfg->getValue( "Y3_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "X4_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        X4MaterikLikes = atoi( cfg->getValue( "X4_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "Y4_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        Y4MaterikLikes = atoi( cfg->getValue( "Y4_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "X5_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        X5MaterikLikes = atoi( cfg->getValue( "X5_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "Y5_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        Y5MaterikLikes = atoi( cfg->getValue( "Y5_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "X6_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        X6MaterikLikes = atoi( cfg->getValue( "X6_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }
    if( cfg->containsParamWithKey( "Y6_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) ){
        Y6MaterikLikes = atoi( cfg->getValue( "Y6_REGION_FILTR_CLOUDY_CONTINENT_LAKES" ) );
    }

// о. Сахалин и Курильская гряда
    if( cfg->containsParamWithKey( "X1_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        X1SahalinKurils = atoi( cfg->getValue( "X1_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }
    if( cfg->containsParamWithKey( "Y1_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        Y1SahalinKurils = atoi( cfg->getValue( "Y1_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }
    if( cfg->containsParamWithKey( "X2_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        X2SahalinKurils = atoi( cfg->getValue( "X2_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }
    if( cfg->containsParamWithKey( "Y2_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        Y2SahalinKurils = atoi( cfg->getValue( "Y2_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }
    if( cfg->containsParamWithKey( "X3_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        X3SahalinKurils = atoi( cfg->getValue( "X3_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }
    if( cfg->containsParamWithKey( "Y3_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        Y3SahalinKurils = atoi( cfg->getValue( "Y3_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }
    if( cfg->containsParamWithKey( "X4_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        X4SahalinKurils = atoi( cfg->getValue( "X4_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }
    if( cfg->containsParamWithKey( "Y4_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) ){
        Y4SahalinKurils = atoi( cfg->getValue( "Y4_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS" ) );
    }

// п-ов Камчатка и побережье Охотского моря
    if( cfg->containsParamWithKey( "X1_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X1KamchatkaOhotskoe = atoi( cfg->getValue( "X1_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y1_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y1KamchatkaOhotskoe = atoi( cfg->getValue( "Y1_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X2_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X2KamchatkaOhotskoe = atoi( cfg->getValue( "X2_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y2_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y2KamchatkaOhotskoe = atoi( cfg->getValue( "Y2_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X3_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X3KamchatkaOhotskoe = atoi( cfg->getValue( "X3_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y3_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y3KamchatkaOhotskoe = atoi( cfg->getValue( "Y3_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X4_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X4KamchatkaOhotskoe = atoi( cfg->getValue( "X4_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y4_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y4KamchatkaOhotskoe = atoi( cfg->getValue( "Y4_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X5_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X5KamchatkaOhotskoe = atoi( cfg->getValue( "X5_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y5_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y5KamchatkaOhotskoe = atoi( cfg->getValue( "Y5_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X6_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X6KamchatkaOhotskoe = atoi( cfg->getValue( "X6_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y6_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y6KamchatkaOhotskoe = atoi( cfg->getValue( "Y6_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X7_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X7KamchatkaOhotskoe = atoi( cfg->getValue( "X7_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y7_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y7KamchatkaOhotskoe = atoi( cfg->getValue( "Y7_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X8_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X8KamchatkaOhotskoe = atoi( cfg->getValue( "X8_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y8_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y8KamchatkaOhotskoe = atoi( cfg->getValue( "Y8_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X9_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X9KamchatkaOhotskoe = atoi( cfg->getValue( "X9_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y9_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y9KamchatkaOhotskoe = atoi( cfg->getValue( "Y9_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "X10_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        X10KamchatkaOhotskoe = atoi( cfg->getValue( "X10_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }
    if( cfg->containsParamWithKey( "Y10_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) ){
        Y10KamchatkaOhotskoe = atoi( cfg->getValue( "Y10_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST" ) );
    }

    delete cfg;
}
