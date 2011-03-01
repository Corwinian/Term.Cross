#include <tregfiltrcloudy.hpp>
#include <unistd.h>

#include <libconfig.h++>
using namespace libconfig;
using namespace std;
TCoordinates TRegionFiltrCloudy::NamberRegionsforFiltrCloudy = 0L;

//const size_t TRegionFiltrCloudy::PRIMORYE_COUNT =8;

/*TCoordinates TRegionFiltrCloudy::X1Primorye = 0L, TRegionFiltrCloudy::Y1Primorye = 0L,
     TRegionFiltrCloudy::X2Primorye = 0L,TRegionFiltrCloudy::Y2Primorye = 0L,
     TRegionFiltrCloudy::X3Primorye = 0L,TRegionFiltrCloudy::Y3Primorye = 0L,
     TRegionFiltrCloudy::X4Primorye = 0L,TRegionFiltrCloudy::Y4Primorye = 0L,
     TRegionFiltrCloudy::X5Primorye = 0L,TRegionFiltrCloudy::Y5Primorye = 0L,
     TRegionFiltrCloudy::X6Primorye = 0L,TRegionFiltrCloudy::Y6Primorye = 0L,
     TRegionFiltrCloudy::X7Primorye = 0L,TRegionFiltrCloudy::Y7Primorye = 0L,
     TRegionFiltrCloudy::X8Primorye = 0L,TRegionFiltrCloudy::Y8Primorye = 0L;



TCoordinates TRegionFiltrCloudy::X1JapanKorea = 0L, TRegionFiltrCloudy::Y1JapanKorea = 0L,
     TRegionFiltrCloudy::X2JapanKorea = 0L, TRegionFiltrCloudy::Y2JapanKorea = 0L,
     TRegionFiltrCloudy::X3JapanKorea = 0L, TRegionFiltrCloudy::Y3JapanKorea = 0L,
     TRegionFiltrCloudy::X4JapanKorea = 0L, TRegionFiltrCloudy::Y4JapanKorea = 0L,
     TRegionFiltrCloudy::X5JapanKorea = 0L, TRegionFiltrCloudy::Y5JapanKorea = 0L,
     TRegionFiltrCloudy::X6JapanKorea = 0L, TRegionFiltrCloudy::Y6JapanKorea = 0L;

TCoordinates TRegionFiltrCloudy::X1MaterikLikes = 0L, TRegionFiltrCloudy::Y1MaterikLikes = 0L,
     TRegionFiltrCloudy::X2MaterikLikes = 3700L, TRegionFiltrCloudy::Y2MaterikLikes = 0L,
     TRegionFiltrCloudy::X3MaterikLikes = 3700L, TRegionFiltrCloudy::Y3MaterikLikes = 0L,
     TRegionFiltrCloudy::X4MaterikLikes = 4300L, TRegionFiltrCloudy::Y4MaterikLikes = 0L,
     TRegionFiltrCloudy::X5MaterikLikes = 4300L, TRegionFiltrCloudy::Y5MaterikLikes = 0L,
     TRegionFiltrCloudy::X6MaterikLikes = 4480L, TRegionFiltrCloudy::Y6MaterikLikes = 0L;

TCoordinates TRegionFiltrCloudy::X1SahalinKurils = 0L, TRegionFiltrCloudy::Y1SahalinKurils = 0L,
     TRegionFiltrCloudy::X2SahalinKurils = 0L, TRegionFiltrCloudy::Y2SahalinKurils = 0L,
     TRegionFiltrCloudy::X3SahalinKurils = 0L, TRegionFiltrCloudy::Y3SahalinKurils = 0L,
     TRegionFiltrCloudy::X4SahalinKurils = 0L, TRegionFiltrCloudy::Y4SahalinKurils = 0L;

TCoordinates TRegionFiltrCloudy::X1KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y1KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X2KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y2KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X3KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y3KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X4KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y4KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X5KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y5KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X6KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y6KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X7KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y7KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X8KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y8KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X9KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y9KamchatkaOhotskoe = 0L,
     TRegionFiltrCloudy::X10KamchatkaOhotskoe = 0L, TRegionFiltrCloudy::Y10KamchatkaOhotskoe = 0L;
*/

void loadRegionforFiltrCloudy(const Setting& set, size_t count, TCoordinates *xArray, TCoordinates *yArray, char *xStr, char *yStr)
{
	for (int i = 0 ; i < count; i++)
	{
		set.lookupValue(strformat(xStr, i), xArray[i]);
		set.lookupValue(strformat(yStr, i), yArray[i]);
	}
}

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
    Config cfg;

    try{
		cfg.readFile(path);
    }
    catch(...){
        throw ae1;
    }

	const Setting &root  = cfg.getRoot();
    root.lookupValue("NUMBER_REGIONS_FILTR_CLOUDY", NamberRegionsforFiltrCloudy);

// Приморье
	loadRegionforFiltrCloudy(root, PRIMORYE_COUNT, XPrimorye, YPrimorye,
					"X%d_REGION_FILTR_CLOUDY_PRIMORYE", "Y%d_REGION_FILTR_CLOUDY_PRIMORYE");

// Япония и Корея
	loadRegionforFiltrCloudy(root, JAPAN_KOREA_COUNT, XJapanKorea, YJapanKorea,
					 "X%d_REGION_FILTR_CLOUDY_JAPAN_KOREA", "Y%d_REGION_FILTR_CLOUDY_JAPAN_KOREA");

// Материковые озера
	loadRegionforFiltrCloudy(root, MATERIK_LIKES_COUNT, XMaterikLikes, YMaterikLikes,
					"X%d_REGION_FILTR_CLOUDY_CONTINENT_LAKES", "Y%d_REGION_FILTR_CLOUDY_CONTINENT_LAKES");

// о. Сахалин и Курильская гряда
	loadRegionforFiltrCloudy(root, SAHALIN_KURILS_COUNT, XSahalinKurils, YSahalinKurils,
					"X%d_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS", "Y%d_REGION_FILTR_CLOUDY_SAHALIN_KURILS_ISLANDS");

// п-ов Камчатка и побережье Охотского моря
	loadRegionforFiltrCloudy(root, KAMCHATKA_OHOTSKOE_COUNT, XKamchatkaOhotskoe, YKamchatkaOhotskoe,
					"X%d_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST", "Y%d_REGION_FILTR_CLOUDY_KAMCHATKA_OHOTSKOE_COAST");

}
