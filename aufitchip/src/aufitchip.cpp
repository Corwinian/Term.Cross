/*-------------------------------------------------------------------------
   aufitchip.cpp - первая версия программы автоматической коррекции
               географической привязки только для изображений AVHRR/NOAA
-------------------------------------------------------------------------*/

//#define INCL_DOSPROCESS
//#include <os2.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <utime.h>
#include <tc_config.h>
#include <c_lib.hpp>
#include <aufitchip.hpp>
#include <astronom.hpp>
#include <filtrcloudy.hpp>
#include <anglescan.hpp>
#include <torbcorr.hpp>

#include <libconfig.h++>
using namespace libconfig;
using namespace std;

char useMsg[] = "\n\
				Использование: aufitchip [<опции>] <файл_данных>\n\
				<файл_данных>  Имя файла упакованных данных HRPT (*.a0).\n\
				Опции: Пока все запрещены!\n";


const char * errorMessages[6] = { "ERROR: ?!", // Сообщения об ошибках.
							"ERROR: Неправильная командная строка!!!",
							"ERROR: Ошибка доступа к входному исходному файлу!!!",
							"ERROR: Ошибка открытия log-файла работы программы!!!" };

bool option_ls = false;

bool option_gf = false;

bool option_cor = false;

uint8_t option_chips_to_pcx = 0;

bool option_gcp_to_pcx = false;
bool mask_or_contour = false;

bool option_hgcp = false;
uint8_t number_algorithm = 1;

TConfigSetting fCS;

char pathStatisticaFile[MAX_PATH] = "";

char pathStatFileMax[MAX_PATH] = "";

std::string inputFileName;

char inputFileGCP[MAX_PATH] = "";

char inputFileCorr[MAX_PATH] = "";

char pathJobFile[MAX_PATH] = "";

char pathLogFile[MAX_PATH] = "";

char pathStatisticSeries[MAX_PATH] = "";
char pathStatisticNavCrit[MAX_PATH] = "";

char output_statDirNOAA_12[] = "NOAA 12\\";
char output_statDirNOAA_14[] = "NOAA 14\\";
char output_statDirNOAA_15[] = "NOAA 15\\";
char output_statDirNOAA_16[] = "NOAA 16\\";
char output_statDirNOAA_17[] = "NOAA 17\\";
char output_statDirNOAA_18[] = "NOAA 18\\";
char output_statDirNOAA_19[] = "NOAA 19\\";
char output_statDirSatelliteUnknown[] = "UNKNOWN\\";

char output_rejectGCPDir[] = "REJECT_GCP\\";

char output_autoGCPDir[] = "AUTO_GCP\\";

bool flagMaskCloudyClear = false;

bool flagCreateFiles = false;

// Для общих вычислений и для финальных значений при востановлении параметров по линейной регрессии.
long fGCPNumber = 0L, fGCPNumber2 = 0L;

double // только в autoGCPs, исходные
// средний модуль, rms, максимальный модуль, стандартное отклонение модуля невязки
	fActAvgM = .0, fActRmsM = .0, fActMaxM = .0, fActStdDevM = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fActAvgX = .0, fActRmsX = .0, fActStdDevX = .0, fActAvgMX = .0, fActMaxMX = .0, fActStdDevMX = .0,
   // среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fActAvgY = .0, fActRmsY = .0, fActStdDevY = .0, fActAvgMY = .0, fActMaxMY = .0, fActStdDevMY = .0;

double // только в autoGCPs, остаточные
// средний модуль, rms, максимальный модуль, стандартное отклонение модуля невязки
	fResAvgM = .0, fResRmsM = .0, fResMaxM = .0, fResStdDevM = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fResAvgX = .0, fResRmsX = .0, fResStdDevX = .0, fResAvgMX = .0, fResMaxMX = .0, fResStdDevMX = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fResAvgY = .0, fResRmsY = .0, fResStdDevY = .0, fResAvgMY = .0, fResMaxMY = .0, fResStdDevMY = .0;


double // только в autoGCPs (проверочные), остаточные
// средний модуль, rms, максимальный модуль, стандартное отклонение модуля невязки
	fCheckResAvgM = .0, fCheckResRmsM = .0, fCheckResMaxM = .0, fCheckResStdDevM = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fCheckResAvgX = .0, fCheckResRmsX = .0, fCheckResStdDevX = .0, fCheckResAvgMX = .0, fCheckResMaxMX = .0, fCheckResStdDevMX = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fCheckResAvgY = .0, fCheckResRmsY = .0, fCheckResStdDevY = .0, fCheckResAvgMY = .0, fCheckResMaxMY = .0, fCheckResStdDevMY = .0;

long fCheckGCPNumber = 0L, fCheckGCPNumber2 = 0L;

double fCorResAvgX = .0, fCorResAvgY = .0;

long minConfiguration_ofGroundX = 10000L, maxConfiguration_ofGroundX = 0L;
long minConfiguration_ofGroundY = 10000L, maxConfiguration_ofGroundY = 0L;


// Для финальных значений при востановлении параметров минимизацией.
long fMGCPNumber = 0L, fMGCPNumber2 = 0L;

double // только в autoGCPs, исходные
// средний модуль, rms, максимальный модуль, стандартное отклонение модуля невязки
	fMActAvgM = .0, fMActRmsM = .0, fMActMaxM = .0, fMActStdDevM = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fMActAvgX = .0, fMActRmsX = .0, fMActStdDevX = .0, fMActAvgMX = .0, fMActMaxMX = .0, fMActStdDevMX = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fMActAvgY = .0, fMActRmsY = .0, fMActStdDevY = .0, fMActAvgMY = .0, fMActMaxMY = .0, fMActStdDevMY = .0;

double // только в autoGCPs, остаточные
// средний модуль, rms, максимальный модуль, стандартное отклонение модуля невязки
	fMResAvgM = .0, fMResRmsM = .0, fMResMaxM = .0, fMResStdDevM = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fMResAvgX = .0, fMResRmsX = .0, fMResStdDevX = .0, fMResAvgMX = .0, fMResMaxMX = .0, fMResStdDevMX = .0,
// среднее значение, rms, среднее значение модуля, максимальный модуль, стандартное отклонение модуля X-компонент невязок
	fMResAvgY = .0, fMResRmsY = .0, fMResStdDevY = .0, fMResAvgMY = .0, fMResMaxMY = .0, fMResStdDevMY = .0;

long MminConfiguration_ofGroundX = 10000L, MmaxConfiguration_ofGroundX = 0L;
long MminConfiguration_ofGroundY = 10000L, MmaxConfiguration_ofGroundY = 0L;

double min_base_X = 10e+10, avg_base_X = .0, max_base_X = .0, std_dev_base_X = .0;
double min_base_Y = 10e+10, avg_base_Y = .0, max_base_Y = .0, std_dev_base_Y = .0;
double min_rms_m = 10e+10, avg_rms_m = .0, max_rms_m = .0, std_dev_rms_m = .0;
double min_epselon_roll = 10e+10, avg_epselon_roll = .0, max_epselon_roll = .0, std_dev_epselon_roll = .0;
double min_epselon_pitch = 10e+10, avg_epselon_pitch = .0, max_epselon_pitch = .0, std_dev_epselon_pitch = .0;
double min_epselon_yaw = 10e+10, avg_epselon_yaw = .0, max_epselon_yaw = .0, std_dev_epselon_yaw = .0;
double min_mod_diff_yaw = 10e+10, avg_mod_diff_yaw = .0, max_mod_diff_yaw = .0, std_dev_mod_diff_yaw = .0;

bool flag_yaw = false;

bool flag_little = false;


TDChip * p_chips;

void errorMessage( const char * error_message_text ) ;
void print_log( const char * message_text ) ;

// ----------------- Функция main ------------------------------------
// Главная функция.

int main( int argc, char * argv[] ) {
	//DosSetPriority( PRTYS_PROCESSTREE, PRTYC_IDLETIME, 31, 0 ); // os2.h
	printf( "DEBUG: %s\n", argv[0] );


	if ( argc < 2 ) {
		printf( "%s", useMsg );
		return 0;
	}

	if ( !parseCommandString( argc, argv ) )
		errorMessage( errorMessages[1] ); // Неправильная командная строка.

	loadCfgSet( argv[0] );	// Читаем установки из cfg-файла.

	TDataAVHRR fDAVHRR( inputFileName, option_hgcp );

	// Дата.
	int month, date;
	double julian_date;
	{
		TAstronom fAstronom;
		dayToDate( fDAVHRR.fNIP->fYear, int(fDAVHRR.fNIP->fYearTime), &month, &date );
		julian_date = fAstronom.julian( fDAVHRR.fNIP->fYear, fDAVHRR.fNIP->fYearTime+1 );
	}

	TFiltrCloudy fFC( fCS.ParametrsFiltrCloudyDir, julian_date, month, fDAVHRR.fCloudyMask, fDAVHRR.fSRef,
					  fDAVHRR.fAvailableChannels, fDAVHRR.fCalibrData, fDAVHRR.fmaxPixValue, fDAVHRR.fka, fDAVHRR.fkb );

	TAufitChipMask fACM( fCS.fMaskandChipBaseDir, fCS.fMaskScale, fCS.fMaxNavigationErrorLine, fCS.fMaxNavigationErrorColumn,
						 fDAVHRR.fNIP, fDAVHRR.fISP, fDAVHRR.fSRef, fDAVHRR.fIR, *(fDAVHRR.fRegrCOP),
						 fFC.fRFC.NamberRegionsforFiltrCloudy, fDAVHRR.fCloudyMask,
						 fDAVHRR.fChannels_forCalcGCPs[0], fDAVHRR.fCalibrData[3], fDAVHRR.fmaxPixValue[3], fDAVHRR.fka[3], fDAVHRR.fkb[3],
						 fDAVHRR.fChannels_forCalcGCPs[1], fDAVHRR.fCalibrData[1], fDAVHRR.fmaxPixValue[1], fDAVHRR.fka[1], fDAVHRR.fkb[1] );

	if( option_chips_to_pcx > 0 && option_chips_to_pcx < 5 ) {
		if( option_chips_to_pcx == 3 || option_chips_to_pcx == 4 )
			filtrCloudyChips( fACM, fFC );

		try {
			fACM.result_MaskorContourChips_to_pcx( option_chips_to_pcx, fCS.fGPCXDir, inputFileName );
		} catch( TAccessExc ae ) {
			errorMessage( ae.what() );
		}

		return 0;
	}

	if( option_ls ) {
		char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME];
		#warning ацкое зло, но думать ломы
		splitpath( const_cast<char *> (inputFileName.c_str()), drive, dir, 0, 0 );

		//strlwr( fname );

		int l = strlen( fname );
		char buf[5], *p = fname + l;
		sprintf( buf, "_%d", number_algorithm );
		strcpy( p, buf );

		// Конструирование полного имени файла лога.
		// Проверка существования каталога fLogDir.
		if( check_dir( fCS.fLogDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
			splitpath( const_cast<char *>(fCS.fLogDir.c_str()), drive, dir, 0, 0 );
		else {
			////_fullpath( pathJobFile, ".", MAX_PATH );
			//strcat( pathJobFile, "\\" );
			//splitpath( pathJobFile, drive, dir, 0, 0 );
			getcwd( pathJobFile, MAX_PATH );
			int t = strlen(pathJobFile);
			if( pathJobFile[t-1] != DIRD ) {
				pathJobFile[t] = DIRD;
				pathJobFile[t+1] = '\0';
			}
			splitpath( pathJobFile, drive, dir, 0, 0 );

		}

		makepath( pathJobFile, drive, dir, fname, "job" );

		FILE * fJob;
		if( !( fJob = fopen( pathJobFile, "w" ) ) ) {
			errorMessage( errorMessages[3] );
		}
		fprintf( fJob, "Обработка AUFITCHIP версии 1.00:\n******************************************************************************************\n" );
		fclose( fJob );
	}

	char str_buf[300];
	//uint8_t optin_save_gcp = 0;

	//---------------------------------------------------------------------------------------------------------------------------------------------
	try {
		print_LogFile( "OPEN_LOG_FILE", false, fDAVHRR.fChannels_forCalcGCPs[1] );
	} catch( TAccessExc ae ) {
		errorMessage( ae.what() );
	}

	unsigned long NG = fACM.numberOfGraundChips();
	if( NG > 0 ) {
		sprintf( str_buf, "\nкол-во всех чипов на изображении до проведения процедуры фильтрации облачности:\t%lu;\n", NG );
		print_LogFile( str_buf );
	} else {
		sprintf( str_buf, "\nНет чипов на изображении до проведения процедуры фильтрации облачности!\n" );
		print_LogFile( str_buf );
		print_LogStatisticSeriesFiles( fDAVHRR, fACM );
		print_log( "Работа программы завершена!\n" );
		return 1;
	}

	if( fCS.fFiltrCloudyforChips > 0 ) {
		filtrCloudyChips( fACM, fFC );
		flagMaskCloudyClear = true;
		fACM.set_flagMaskCloudyClear( flagMaskCloudyClear );
	}

	if( fCS.fFiltrCloudyforChips == 2 ) {
		selection_Chips_for_GraundChips( fDAVHRR, fACM );
		sprintf( str_buf, "кол-во реперных чипов на изображении после проведения процедуры отбраковки по процент. количеству облачности:\t%lu;\n", fACM.numberOfGraundChips() );
		print_LogFile( str_buf );
	}

	if( ( fACM.numberOfGraundChips() < 4 ) && fCS.fFiltrCloudyforChips == 2 && flagMaskCloudyClear ) {
		memset( fDAVHRR.fCloudyMask, 0, 2048 * fDAVHRR.fNIP->fScans );
		flagMaskCloudyClear = false;
		fACM.set_flagMaskCloudyClear( flagMaskCloudyClear );
		sprintf( str_buf, "\n\n--------------------------------------------------------------------------------\nОтказ от фильтрации облачности и сброс маски облачности.\n--------------------------------------------------------------------------------\n" );
		print_LogFile( str_buf );
		fACM.setAllToGraundChips();
		sprintf( str_buf, "кол-во реперных чипов на изображении без проведения фильтрации облачности:\t%lu;\n", fACM.numberOfGraundChips() );
		print_LogFile( str_buf );
	}

	double navigProbabilityI, navProbI = -10., navigProbabilityII;
	unsigned centerConfig_X, centerConfig_Y;
	double base_x, base_y;
	do {
		calculateShifts_forGraundChips( fDAVHRR, fACM );

		sprintf( str_buf, "\nКол-во реперных точек после вычисления смещений и отбраковки по нормализ. критерию стат-й знач-ти:" );
		print_LogFile( str_buf );
		sprintf( str_buf, "\nкол-во 'хороших' реперных точек:\t%lu;\n", fACM.numberOfGraundChips() );
		print_LogFile( str_buf );
		sprintf( str_buf, "кол-во 'плохих' реперных точек:\t%lu;\n", fACM.numberOfControlChips() );
		print_LogFile( str_buf );

		try {
			save_intervGCP( false, "a", fDAVHRR, fACM );
			save_intervGCP( true, "q", fDAVHRR, fACM );
		} catch( TAccessExc ae ) {
			errorMessage( ae.what() );
		}
		fACM.setControlToCheckChips();

		// Если мало или нет реперных точек и маску облачности не сбрасывали.
		if( flagMaskCloudyClear && ( fACM.numberOfGraundChips() < 4 ) ) {
			memset( fDAVHRR.fCloudyMask, 0, 2048 * fDAVHRR.fNIP->fScans );
			flagMaskCloudyClear = false;
			fACM.set_flagMaskCloudyClear( flagMaskCloudyClear );
			flagCreateFiles = true;
			sprintf( str_buf, "\n\n--------------------------------------------------------------------------------\nОтказ от фильтрации облачности и сброс маски облачности.\n--------------------------------------------------------------------------------\n" );
			print_LogFile( str_buf );
			fACM.setAllToGraundChips();
			sprintf( str_buf, "кол-во реперных чипов на изображении без проведения фильтрации облачности:\t%lu;\n", fACM.numberOfGraundChips() );
			print_LogFile( str_buf );
			calculateShifts_forGraundChips( fDAVHRR, fACM );
			sprintf( str_buf, "\nКол-во реперных точек после вычисления смещений и отбраковки по нормализ. критерию стат-й знач-ти:" );
			print_LogFile( str_buf );
			sprintf( str_buf, "\nкол-во 'хороших' реперных точек:\t%lu;\n", fACM.numberOfGraundChips() );
			print_LogFile( str_buf );
			sprintf( str_buf, "кол-во 'плохих' реперных точек:\t%lu;\n", fACM.numberOfControlChips() );
			print_LogFile( str_buf );
			try {
				save_intervGCP( false, "a", fDAVHRR, fACM );
				save_intervGCP( true, "q", fDAVHRR, fACM );
			} catch( TAccessExc ae ) {
				errorMessage( ae.what() );
			}
			fACM.setControlToCheckChips();
		}
		if( (!flagMaskCloudyClear) && fACM.numberOfGraundChips() <= 0 ) {
			sprintf( str_buf, "\nНет реперных точек на изображении после вычисления смещений и отбраковки по нормализ. критерию стат. знач-ти!\n" );
			print_LogFile( str_buf );
			print_LogStatisticSeriesFiles( fDAVHRR, fACM );
			print_log( "Работа программы завершена!\n" );
			return 2;
		}

		// Отбор реперных точек по критерию 2-сигма.
		sprintf( str_buf, "\n\n-------------------------------------------------------------------------------------------------------------------------------------\nВостановление углов платформы спутника при помощи линейной регрессии:\n-------------------------------------------------------------------------------------------------------------------------------------\n" );
		print_LogFile( str_buf );
		/*
		          unsigned long quantity_badGCPs = 0;
		          TCorrectionParams cop( 2 );
		          fDAVHRR.setCorrectionParams( true, cop, fACM, true );
		          fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fACM );
		          calcStatParams_for2SigmaCriterion( fACM );
		          NG = fACM.numberOfGraundChips();
		          uint8_t bed_ch;
		          TDChip * bed_chip;
		          TAufitChipMask::ChipCursor cursor( fACM );
		          do {
		              quantity_badGCPs = 0;
		              double bed_stat_signf = 1000000.;
		              if( fResMaxM >= 1. && NG >= 4 ){
		                  for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
		                       TDChip * chip = fACM.chipAt( cursor );
		                       if( chip->fChipType[0] == TDChip::GraundChip ){
		                           if( fabs(fabs(chip->dX[0]) - fResAvgMX) >= (2. * fResStdDevMX) ||
		                               fabs(fabs(chip->dY[0]) - fResAvgMY) >= (2. * fResStdDevMY) ){
		                               quantity_badGCPs++;
		                               if( chip->fNormStatSign[0] <= bed_stat_signf ){
		                                   bed_chip = chip;
		                                   bed_stat_signf = bed_chip->fNormStatSign[0];
		                                   bed_ch = 0;
		                               }
		                           }
		                       }
		                       if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ){
		                           if( fabs(fabs(chip->dX[1]) - fResAvgMX) >= (2. * fResStdDevMX) ||
		                               fabs(fabs(chip->dY[1]) - fResAvgMY) >= (2. * fResStdDevMY) ){
		                               quantity_badGCPs++;
		                               if( chip->fNormStatSign[1] <= bed_stat_signf ){
		                                   bed_chip = chip;
		                                   bed_stat_signf = bed_chip->fNormStatSign[1];
		                                   bed_ch = 1;
		                               }
		                           }
		                       }
		                  }
		              }
		              if( quantity_badGCPs > 0 ){
		                  fDAVHRR.setCorrectionParams( true, cop, fACM, true );
		                  bed_chip->fChipType[bed_ch] = TDChip::ControlChip;
		                  NG--;
		                  fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fACM );
		                  calcStatParams_for2SigmaCriterion( fACM );
		              }
		          } while( (fResMaxM >= 1.) && (NG >= 4) && (quantity_badGCPs > 0) );

		          fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
		*/
		selection_GraundChips_for_autoGCPs_afterCalcCorrectionLineRegres( fDAVHRR, fACM );
		try {
			save_intervGCP( true, "r", fDAVHRR, fACM );
		} catch( TAccessExc ae ) {
			errorMessage( ae.what() );
		}

		// Вычисление углов положения платформы, оценок отстаточных невязок в реперных точках (по всему набору GCPs).
		flag_yaw = false;
		fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fACM );
		calcResDisp_forAutoGCPs( fACM, fCorResAvgX, fCorResAvgY );
		fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fCS.fLinRegresCoeff_dX * fCorResAvgX, fCS.fLinRegresCoeff_dY * fCorResAvgY, fACM, flag_yaw );
		if( fabs(fDAVHRR.beta1_y) > 0.02 ) {
			sprintf( str_buf, "\nПри вычислении угол yaw (=%.14f) превысил установленный лимит (0.02).\n", fDAVHRR.beta1_y );
			print_LogFile( str_buf );
		}

		// Оценка качества привязки всего снимка по вычисленным углам с помощью реперных точек.
		calcConfigBases_forAutoGCPs( fDAVHRR, fACM, centerConfig_X, base_x, centerConfig_Y, base_y );
		fDAVHRR.l_BaseX = base_x;
		fDAVHRR.l_BaseY = base_y;
		calcStatParams_forAutoGCPs( true, false, fACM );
		navigation_probabilityPrecision( base_x, fResRmsM, fGCPNumber, navigProbabilityI, navigProbabilityII );
		printFileLog(  fDAVHRR, centerConfig_X, centerConfig_Y, navigProbabilityI, navigProbabilityII );

		if( navProbI < -1. ) {
			navProbI = navigProbabilityI;
		} else {
			if( navProbI > navigProbabilityI ) {
				sprintf( str_buf, "\n-----------------------------------------------------------------------\nВозврат к фильтрации облачности, т.к. вероятность привязки была лучше.\n-----------------------------------------------------------------------\n" );
				print_LogFile( str_buf );
				flagCreateFiles = false;
				fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
				fACM.setAllToGraundChips();
				if( fCS.fFiltrCloudyforChips > 0 ) {
					filtrCloudyChips( fACM, fFC );
					flagMaskCloudyClear = true;
					fACM.set_flagMaskCloudyClear( flagMaskCloudyClear );
				}
				if( fCS.fFiltrCloudyforChips == 2 )
					selection_Chips_for_GraundChips( fDAVHRR, fACM );

				calculateShifts_forGraundChips( fDAVHRR, fACM );

				sprintf( str_buf, "\nКол-во реперных точек после вычисления смещений и отбраковки по нормализ. критерию стат-й знач-ти:" );
				print_LogFile( str_buf );
				sprintf( str_buf, "\nкол-во 'хороших' реперных точек:\t%lu;\n", fACM.numberOfGraundChips() );
				print_LogFile( str_buf );
				sprintf( str_buf, "кол-во 'плохих' реперных точек:\t%lu;\n", fACM.numberOfControlChips() );
				print_LogFile( str_buf );

				try {
					save_intervGCP( false, "a", fDAVHRR, fACM );
					save_intervGCP( true, "q", fDAVHRR, fACM );
				} catch( TAccessExc ae ) {
					errorMessage( ae.what() );
				}
				fACM.setControlToCheckChips();

				// Отбор реперных точек по критерию 2-сигма.
				sprintf( str_buf, "\n\n-------------------------------------------------------------------------------------------------------------------------------------\nВостановление углов платформы спутника при помощи линейной регрессии:\n-------------------------------------------------------------------------------------------------------------------------------------\n" );
				print_LogFile( str_buf );

				selection_GraundChips_for_autoGCPs_afterCalcCorrectionLineRegres( fDAVHRR, fACM );
				try {
					save_intervGCP( true, "r", fDAVHRR, fACM );
				} catch( TAccessExc ae ) {
					errorMessage( ae.what() );
				}

				// Вычисление углов положения платформы, оценок отстаточных невязок в реперных точках (по всему набору GCPs).
				flag_yaw = false;
				fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fACM );
				calcResDisp_forAutoGCPs( fACM, fCorResAvgX, fCorResAvgY );
				fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fCS.fLinRegresCoeff_dX * fCorResAvgX, fCS.fLinRegresCoeff_dY * fCorResAvgY, fACM, flag_yaw );
				if( fabs(fDAVHRR.beta1_y) > 0.02 ) {
					sprintf( str_buf, "\nПри вычислении угол yaw (=%.14f) превысил установленный лимит (0.02).\n", fDAVHRR.beta1_y );
					print_LogFile( str_buf );
				}

				// Оценка качества привязки всего снимка по вычисленным углам с помощью реперных точек.
				calcConfigBases_forAutoGCPs( fDAVHRR, fACM, centerConfig_X, base_x, centerConfig_Y, base_y );
				fDAVHRR.l_BaseX = base_x;
				fDAVHRR.l_BaseY = base_y;
				calcStatParams_forAutoGCPs( true, false, fACM );
				navigation_probabilityPrecision( base_x, fResRmsM, fGCPNumber, navigProbabilityI, navigProbabilityII );
				printFileLog(  fDAVHRR, centerConfig_X, centerConfig_Y, navigProbabilityI, navigProbabilityII );

				break;
			}
		}

		// Если принято решение, что привязка по трём углам является неудовлетворительной и при этом маску облачности не сбрасывали.
		flag_little = false;
		if( navigProbabilityI < 80. ) {
			if( fCS.fFiltrCloudyforChips > 0 && flagMaskCloudyClear ) {
				fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
				memset( fDAVHRR.fCloudyMask, 0, 2048 * fDAVHRR.fNIP->fScans );
				flagMaskCloudyClear = false;
				fACM.set_flagMaskCloudyClear( flagMaskCloudyClear );
				flagCreateFiles = true;
				sprintf( str_buf, "\n------------------------------------------------------------------\nОтказ от фильтрации облачности и сброс маски облачности.\nТак как привязка по трём углам является неудовлетворительной.\n------------------------------------------------------------------\n" );
				print_LogFile( str_buf );
				fACM.setAllToGraundChips();
				sprintf( str_buf, "кол-во реперных чипов на изображении без проведения фильтрации облачности:\t%lu;\n", fACM.numberOfGraundChips() );
				print_LogFile( str_buf );
				flag_little = true;
			}
		}
	} while ( flag_little );

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

	// Если принято решение, что привязка по трём углам является неудовлетворительной.
	if( navigProbabilityI < 80. ) {
		sprintf( str_buf, "\n\n------------------------------------------------------------------\nТак как привязка по трём углам является неудовлетворительной.\nТогда рассчитываем два угла платфомы спутника по линейной регрессии (угол yaw = 0).\n------------------------------------------------------------------\n" );
		print_LogFile( str_buf );
		fACM.setControlToGraundChips();
		flag_yaw = true;
		selection_GraundChips_for_autoGCPs_afterCalcCorrectionLineRegres( fDAVHRR, fACM );

		flagCreateFiles = false;
		try {
			save_intervGCP( true, "e", fDAVHRR, fACM );
		} catch( TAccessExc ae ) {
			errorMessage( ae.what() );
		}

		// Вычисление углов положения платформы, оценок отстаточных невязок в реперных точках (по всему набору GCPs).
		flag_yaw = true;
		fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fACM );
		calcResDisp_forAutoGCPs( fACM, fCorResAvgX, fCorResAvgY );
		fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fCS.fLinRegresCoeff_dX * fCorResAvgX, fCS.fLinRegresCoeff_dY * fCorResAvgY, fACM, flag_yaw );

		// Оценка качества привязки всего снимка по вычисленным углам с помощью реперных точек.
		calcConfigBases_forAutoGCPs( fDAVHRR, fACM, centerConfig_X, base_x, centerConfig_Y, base_y );
		fDAVHRR.l_BaseX = base_x;
		fDAVHRR.l_BaseY = base_y;
		calcStatParams_forAutoGCPs( true, false, fACM );
		navigation_probabilityPrecision( base_x, fResRmsM, fGCPNumber, navigProbabilityI, navigProbabilityII );
		printFileLog(  fDAVHRR, centerConfig_X, centerConfig_Y, navigProbabilityI, navigProbabilityII );
	}

	try {
		save_Correction( fDAVHRR, fACM, navigProbabilityI, navigProbabilityII, centerConfig_X, centerConfig_Y );
		fDAVHRR.saveCorToA0();
	} catch( TAccessExc ae ) {
		errorMessage( ae.what() );
	}

	fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
	try {
		save_autoGCP( false, fDAVHRR, fACM );
	} catch( TAccessExc ae ) {
		errorMessage( ae.what() );
	}

	print_log( "Работа программы завершена!\n" );
	return 0;
};


void printFileLog(  TDataAVHRR & fDAVHRR, long centerConfig_X, long centerConfig_Y, double navigProbabilityI, double navigProbabilityII ) {
	FILE * fLog = fopen( pathLogFile, "a" );
	fprintf( fLog, "\n<-------------------------------------------------------------------------------------------------------------------------------------------------------->\n" );
	fprintf( fLog, "\nЗначения углов платформы спутника, восстановленных по линейной регрессии (satellite attitude):\n" );
	fprintf( fLog, "%.14f\t; roll\n", fDAVHRR.fRegrCOP->roll );
	fprintf( fLog, "%.14f\t; pitch\n", fDAVHRR.fRegrCOP->pitch );
	fprintf( fLog, "%.14f\t; yaw\n", fDAVHRR.fRegrCOP->yaw );
	fprintf( fLog, "\n<-------------------------------------------------------------------------------------------------------------------------------------------------------->\n" );
	fprintf( fLog, "\nЗначения параметров оценки точности привязки по трём углам (navigate precision):\n" );
	fprintf( fLog, "navigProbabilityI = %.1f;\t navigProbabilityII = %.1f;\t numberGCPs = %ld;\t resRmsM = %.5f;\n",
			 navigProbabilityI, navigProbabilityII, fGCPNumber, fResRmsM );
	fprintf( fLog, "\nЗначения параметров конфигурации реперных точек (configuration of GCPs):\n" );
	fprintf( fLog, "baseGCPs_X = %.5f;\t centerConfig_X = %ld;\t baseGCPs_Y = %.5f;\t centerConfig_Y = %ld;\n",
			 fDAVHRR.l_BaseX, centerConfig_X, fDAVHRR.l_BaseY, centerConfig_Y );
	fprintf( fLog, "\n<-------------------------------------------------------------------------------------------------------------------------------------------------------->\n\n" );
	fprintf( fLog, "\n##############################################################################################################################################################################################\n" );
	fprintf( fLog, "\nЗначения параметров остаточных невязок в реперных точках (residual navigation errors):\n" );
	fprintf( fLog, "resAvgM = %.5f;\t resStdDevM = %.5f;\t resMaxM = %.5f;\n",
			 fResAvgM, fResStdDevM, fResMaxM );
	fprintf( fLog, "fResAvgMX = %.5f;\t fResRmsX = %.5f;\t fResMaxMX = %.5f;\n fResAvgMY = %.5f;\t fResRmsY = %.5f;\t fResMaxMY = %.5f;\n",
			 fResAvgMX, fResRmsX, fResMaxMX, fResAvgMY, fResRmsY, fResMaxMY );
	fprintf( fLog, "\nЗначения корректируемых параметров, связанных с размером пиксела:\n" );
	fprintf( fLog, "corResAvgX = %.14f;\t corResAvgY = %.14f\n", fCorResAvgX, fCorResAvgY );
	fprintf( fLog, "\n##############################################################################################################################################################################################\n\n" );
	fclose( fLog );
};


void navigation_probabilityPrecision( double base_X, double res_navigRMS, long namberGCPs, double & navigProbI, double & navigProbII ) {
	const static float navigationProbabilityI[16][10] = {
		{ 24.3, 51.5, 75.7, 87.5, 96.2, 96.4, 97.7, 99.8, 100.0, 100.0 },
		{ 22.4, 35.7, 62.2, 81.5, 92.8, 96.9, 98.6, 99.8, 100.0, 100.0 },
		{ 17.3, 21.0, 44.6, 64.8, 80.7, 90.4, 94.9, 98.5, 99.5,  100.0 },
		{ 15.2, 15.8, 33.4, 49.6, 67.4, 82.3, 89.3, 94.1, 98.2,  99.5  },
		{ 8.9,  11.5, 23.5, 41.3, 57.3, 72.2, 80.7, 87.9, 94.8,  96.7  },
		{ 8.9,  9.3,  18.8, 30.5, 47.3, 60.3, 72.3, 80.8, 88.6,  96.3  },
		{ 8.0,  9.3,  15.6, 25.1, 36.8, 51.2, 65.4, 73.5, 82.3,  88.5  },
		{ 8.0,  9.3,  12.3, 20.9, 32.0, 42.7, 54.4, 64.3, 74.7,  82.6  },
		{ 7.7,  7.8,  9.3,  18.3, 27.5, 39.3, 47.2, 58.4, 67.9,  75.6  },
		{ 5.5,  7.7,  8.8,  14.9, 21.8, 30.4, 41.2, 49.9, 57.8,  69.1  },
		{ 4.5,  5.5,  6.4,  12.5, 19.9, 25.8, 34.5, 42.8, 50.2,  59.8  },
		{ 4.0,  5.0,  5.5,  11.2, 17.8, 22.9, 30.4, 39.6, 44.1,  51.1  },
		{ 4.0,  5.0,  5.5,  10.2, 13.3, 20.0, 27.1, 35.4, 42.6,  51.1  },
		{ 4.0,  5.0,  5.5,  10.1, 13.0, 20.0, 27.3, 30.1, 38.3,  48.2  },
		{ 4.0,  5.0,  5.5,  8.0,  11.2, 16.6, 24.7, 28.6, 35.0,  45.5  },
		{ 4.0,  5.0,  5.5,  8.0,  9.8,  13.5, 16.7, 28.3, 28.5,  39.5  }};

	const static float navigationProbabilityII[16][10] = {
		{ 50.4, 81.4, 95.0, 97.3, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0 },
		{ 43.9, 62.6, 88.5, 96.1, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0 },
		{ 37.0, 43.7, 75.3, 90.4, 97.5,  100.0, 100.0, 100.0, 100.0, 100.0 },
		{ 30.3, 33.9, 65.2, 81.9, 93.7,  97.8,  100.0, 100.0, 100.0, 100.0 },
		{ 27.4, 29.4, 53.7, 76.3, 87.8,  95.7,  97.5,  100.0, 100.0, 100.0 },
		{ 27.4, 27.5, 45.2, 65.7, 83.1,  92.3,  95.9,  98.1,  100.0, 100.0 },
		{ 21.2, 26.9, 41.0, 60.2, 74.7,  87.7,  93.2,  96.9,  98.8,  100.0 },
		{ 21.2, 26.9, 34.8, 52.5, 69.4,  81.4,  89.4,  95.3,  98.3,  98.3  },
		{ 21.2, 26.3, 29.0, 48.0, 64.9,  76.9,  84.9,  90.9,  96.3,  99.1  },
		{ 21.2, 26.3, 27.6, 43.1, 57.0,  71.8,  82.8,  90.0,  94.1,  96.8  },
		{ 21.2, 23.1, 25.6, 38.8, 53.5,  66.6,  77.2,  83.7,  91.6,  95.2  },
		{ 21.0, 22.0, 23.1, 35.8, 50.8,  64.3,  73.7,  82.4,  88.8,  91.9  },
		{ 20.0, 21.0, 22.0, 31.5, 46.6,  62.1,  70.1,  79.6,  86.6,  90.9  },
		{ 20.0, 21.0, 22.0, 31.5, 45.9,  57.1,  70.2,  78.1,  85.9,  89.3  },
		{ 20.0, 21.0, 22.0, 29.2, 40.5,  53.4,  69.6,  76.8,  84.4,  90.9  },
		{ 20.0, 21.0, 22.0, 27.2, 40.1,  51.6,  62.6,  66.5,  77.2,  81.6  }};

	if( res_navigRMS > 8.0 ) {
		navigProbI = 3.0;
		navigProbII = 20.0;
		return;
	} else {
		long i = long( ceil( 2 * res_navigRMS ) ) - 1;
		long j = long( ceil( 10 * base_X ) ) - 1;
		navigProbI = navigationProbabilityI[i][j];
		navigProbII = navigationProbabilityII[i][j];
		return;
	}
};


// ----------------- Функция parseCommandString ----------------------
// Считывание параметров для программы при её запуске из командной строки.


int parseCommandString( int argc, char * argv[] ) {
	if ( argc < 2 )
		return 0;

	int i = 1;
	while ( i < argc ) {
		char* s = argv[i++];
		//if( *s == '-' || *s == '/' ) {
		if( *s == '-' ) { // символ '/' испльзуется в *nix как разделитель директорий.
			s++;
			if( *s == 'l' ) {
				s++;
				if( *s == 's' )     // опция -ls
					option_ls = true;
			} else {
				if( *s == 'c' ) {
					s++;
					if( *s == 'm' ) {     // опция -cm или -cmf
						if( *(s+1) == 'f')
							option_chips_to_pcx = 3;
						else
							option_chips_to_pcx = 1;
					} else {
						if( *s == 'c' ) {     // опция -cc или -ccf
							if( *(s+1) == 'f')
								option_chips_to_pcx = 4;
							else
								option_chips_to_pcx = 2;
						}
					}
				} else {
					if( *s == 's' ) {
						s++;
						if( *s == 'c' )     // опция -sc
							option_cor = true;
					} else {
						if( *s == 'g' ) {
							s++;
							if( *s == 'f' )     // опция -gf
								option_gf = true;
						} else {
							if( *s == 'p' ) {
								s++;
								if( *s == 'm' ) {     // опция -pm
									option_gcp_to_pcx = true;
									mask_or_contour = true;
								} else {
									if( *s == 'c' ) {     // опция -pc
										option_gcp_to_pcx = true;
										mask_or_contour = false;
									}
								}
							} else {
								if( *s == 'a' ) {
									s++;
									if( *s == '1' || *s == '2' )     // опция -a1|2
										number_algorithm = atoi(s);
								} else
									return 0;  // указана неизвестная опция
							}
						}
					}
				}
			}
		} else {
			if ( inputFileName.empty() ) {
				inputFileName = s;
			} else {
				if ( !strlen( inputFileGCP ) ) {
					strncpy( inputFileGCP, s, MAX_PATH-1 );
					inputFileGCP[MAX_PATH-1] = '\0';
				} else {
					strncpy( inputFileCorr, s, MAX_PATH-1 );
					inputFileCorr[MAX_PATH-1] = '\0';
				}
			}
		}
	}

	if ( inputFileName.empty() )
		return 0; // В командной строке не был указан входной файл.

	//strlwr( inputFileName );

	if( strlen( inputFileGCP ) ) {
		//strlwr( inputFileGCP );
		option_hgcp = true;
	}

	if( strlen( inputFileCorr ) ) {
		//strlwr( inputFileCorr );
		option_hgcp = true;
	}

	return 1;
};



// ----------------- Функция errorMessage -----------------------------------
// Консольный вывод сообщений о возникших ошибках, при аварийном завершении программы.

void errorMessage( const char * error_message_text ) {
	print_log( error_message_text );
	print_log( "Аварийное завершение работы программы...\n" );
	exit( 1 );
};



// ----------------- Функция error -----------------------------------
// Консольный и в log-файл вывод сообщений о ходе работы программы.

void print_log( const char * message_text ) {
	if( option_ls ) {
		FILE * fJob = fopen( pathJobFile, "a" );
		fprintf( fJob, "%s\n", message_text );
		fclose( fJob );
	}

	printf( "%s\n", message_text );
};


void print_LogStatisticSeriesFiles( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия log-файла статистики для записи!!!" );

	uint8_t l;
	char * p;
	FILE * fStat;
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], fname_file[MAX_FNAME];
	#warning ацкое зло, но думать ломы
	splitpath( const_cast<char *> (inputFileName.c_str()), drive, dir, fname_file, 0 );

	// Конструирование полного имени файла лога.
	// Проверка существования каталога fLogDir.

	if( check_dir( fCS.fSourceDataDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath( const_cast<char *> (fCS.fSourceDataDir.c_str()), drive, dir, 0, 0 );
	else {
		getcwd( pathStatisticSeries, MAX_PATH );
		int t = strlen(pathStatisticSeries);
		if( pathStatisticSeries[t-1] != DIRD ) {
			pathStatisticSeries[t] = DIRD;
			pathStatisticSeries[t+1] = '\0';
		}
		splitpath( pathStatisticSeries, drive, dir, 0, 0 );
	}

	l = strlen( dir );
	p = dir + l;
	if( strcmp( fDAVHRR.satName, "NOAA 12" ) == 0 ) {
		strcpy( p, output_statDirNOAA_12 );
	} else if( strcmp( fDAVHRR.satName, "NOAA 15" ) == 0 ) {
		strcpy( p, output_statDirNOAA_15 );
	} else if( strcmp( fDAVHRR.satName, "NOAA 17" ) == 0 ) {
		strcpy( p, output_statDirNOAA_17 );
	} else if( strcmp( fDAVHRR.satName, "NOAA 18" ) == 0 ) {
		strcpy( p, output_statDirNOAA_18 );
	} else if( strcmp( fDAVHRR.satName, "NOAA 19" ) == 0 ) {
		strcpy( p, output_statDirNOAA_19 );
	} else if( strcmp( fDAVHRR.satName, "NOAA 14" ) == 0 ) {
		strcpy( p, output_statDirNOAA_14 );
	} else if( strcmp( fDAVHRR.satName, "NOAA 16" ) == 0 ) {
		strcpy( p, output_statDirNOAA_16 );
	} else {
		strcpy( p, output_statDirSatelliteUnknown );
	}

	sprintf( fname, "datapass" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}

	// Дата.
	int month, date;
	dayToDate( fDAVHRR.fNIP->fYear, int(fDAVHRR.fNIP->fYearTime), &month, &date );
	// Время.
	double day_fraction;
	int hour, minute, second, tic;
	day_fraction = fDAVHRR.fNIP->fYearTime - int(fDAVHRR.fNIP->fYearTime);
	dayFractionToTime( day_fraction, &hour, &minute, &second, &tic );

	fprintf( fStat, "%s\t%02d.%02d.%d\t%02d:%02d:%02d\t", fname_file, date+1, month+1, fDAVHRR.fNIP->fYear, hour, minute, second );

	uint8_t ch=0, fil=0;
	if( fDAVHRR.fChannels_forCalcGCPs[1] == true )
		ch = 1;
	if( flagMaskCloudyClear == true )
		fil = 1;


	fprintf( fStat, "%d\t%d\t%s\t%d\t%d\t%.3f\t%d\t%d\n", fDAVHRR.fNIP->fAscendFlag, fDAVHRR.fBlk0.b0.satId, fDAVHRR.satName,
			 fDAVHRR.fBlk0.b0.revNum, fDAVHRR.fNIP->fScans, fDAVHRR.fHeighSanny_overHorizonDegree, ch, fil );
	fclose( fStat );

	sprintf( fname, "anglesatt" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	/*    fprintf( fStat, "%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\n",
	             fDAVHRR.fMinCOP->roll, fDAVHRR.fMinCOP->pitch, fDAVHRR.fMinCOP->yaw,
	             fDAVHRR.fRegrCOP->roll, fDAVHRR.fRegrCOP->pitch, fDAVHRR.fRegrCOP->yaw,
	             fDAVHRR.fMinCOP->roll - fDAVHRR.fRegrCOP->roll, fDAVHRR.fMinCOP->pitch - fDAVHRR.fRegrCOP->pitch, fDAVHRR.fMinCOP->yaw - fDAVHRR.fRegrCOP->yaw,
	             fDAVHRR.fIniCOP->roll, fDAVHRR.fIniCOP->pitch, fDAVHRR.fIniCOP->yaw,
	             fDAVHRR.fIniCOP->roll - fDAVHRR.fMinCOP->roll, fDAVHRR.fIniCOP->pitch - fDAVHRR.fMinCOP->pitch, fDAVHRR.fIniCOP->yaw - fDAVHRR.fMinCOP->yaw,
	             fDAVHRR.fIniCOP->roll - fDAVHRR.fRegrCOP->roll, fDAVHRR.fIniCOP->pitch - fDAVHRR.fRegrCOP->pitch, fDAVHRR.fIniCOP->yaw - fDAVHRR.fRegrCOP->yaw );
	*/
	fprintf( fStat, "%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\n",
			 1000.*fDAVHRR.fRegrCOP->roll, 1000.*fDAVHRR.fRegrCOP->pitch, 1000.*fDAVHRR.fRegrCOP->yaw,
			 1000.*fDAVHRR.fIniCOP->roll, 1000.*fDAVHRR.fIniCOP->pitch, 1000.*fDAVHRR.fIniCOP->yaw,
			 1000.*(fDAVHRR.fIniCOP->roll - fDAVHRR.fRegrCOP->roll), 1000.*(fDAVHRR.fIniCOP->pitch - fDAVHRR.fRegrCOP->pitch), 1000.*(fDAVHRR.fIniCOP->yaw - fDAVHRR.fRegrCOP->yaw) );
	fclose( fStat );
	/*
	    sprintf( fname, "minresgcp" );
	    makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	    if( !(fStat = fopen( pathStatisticSeries, "a" )) ){
	          throw ae1;
	    }
	    fprintf( fStat, "%d\t%d\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\n",
	             fMGCPNumber, fMGCPNumber2, fMResAvgM, fMResRmsM, fMResMaxM, fMResStdDevM,
	             fMResAvgX, fMResRmsX, fMResStdDevX, fMResAvgMX, fMResMaxMX, fMResStdDevMX,
	             fMResAvgY, fMResRmsY, fMResStdDevY, fMResAvgMY, fMResMaxMY, fMResStdDevMY );
	    fclose( fStat );

	    sprintf( fname, "minactgcp" );
	    makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	    if( !(fStat = fopen( pathStatisticSeries, "a" )) ){
	          throw ae1;
	    }
	    fprintf( fStat, "%d\t%d\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\n",
	             fMGCPNumber, fMGCPNumber2, fMActAvgM, fMActRmsM, fMActMaxM, fMActStdDevM,
	             fMActAvgX, fMActRmsX, fMActStdDevX, fMActAvgMX, fMActMaxMX, fMActStdDevMX,
	             fMActAvgY, fMActRmsY, fMActStdDevY, fMActAvgMY, fMActMaxMY, fMActStdDevMY );
	    fclose( fStat );

	    sprintf( fname, "minbasegcp" );
	    makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	    if( !(fStat = fopen( pathStatisticSeries, "a" )) ){
	          throw ae1;
	    }
	    if( MminConfiguration_ofGroundX == 10000L && MminConfiguration_ofGroundY == 10000L ){
	        MminConfiguration_ofGroundX = 1L;
	        MminConfiguration_ofGroundY = 1L;
	    }
	    fprintf( fStat, "%d\t%d\t%d\t%d\t%d\t%d\t%.5f\t%.5f\n",
	             MminConfiguration_ofGroundX, MminConfiguration_ofGroundY, MmaxConfiguration_ofGroundX, MmaxConfiguration_ofGroundY,
	             MmaxConfiguration_ofGroundX - MminConfiguration_ofGroundX + 1, MmaxConfiguration_ofGroundY - MminConfiguration_ofGroundY + 1,
	             double(MmaxConfiguration_ofGroundX - MminConfiguration_ofGroundX + 1) / 2048.,
	             double(MmaxConfiguration_ofGroundY - MminConfiguration_ofGroundY + 1) / double(fDAVHRR.fNIP->fScans) );
	    fclose( fStat );
	*/
	sprintf( fname, "rlresgcp" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%ld\t%ld\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\n",
			 fGCPNumber, fGCPNumber2, fResAvgM, fResRmsM, fResMaxM, fResStdDevM,
			 fResAvgX, fResRmsX, fResStdDevX, fResAvgMX, fResMaxMX, fResStdDevMX,
			 fResAvgY, fResRmsY, fResStdDevY, fResAvgMY, fResMaxMY, fResStdDevMY );
	fclose( fStat );

	sprintf( fname, "rlactgcp" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%ld\t%ld\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\n",
			 fGCPNumber, fGCPNumber2, fActAvgM, fActRmsM, fActMaxM, fActStdDevM,
			 fActAvgX, fActRmsX, fActStdDevX, fActAvgMX, fActMaxMX, fActStdDevMX,
			 fActAvgY, fActRmsY, fActStdDevY, fActAvgMY, fActMaxMY, fActStdDevMY );
	fclose( fStat );

	sprintf( fname, "rlbasegcp" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	if( minConfiguration_ofGroundX == 10000L && minConfiguration_ofGroundY == 10000L ) {
		minConfiguration_ofGroundX = 1L;
		minConfiguration_ofGroundY = 1L;
	}
	fprintf( fStat, "%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%.5f\t%.5f\n",
			 minConfiguration_ofGroundX, minConfiguration_ofGroundY, maxConfiguration_ofGroundX, maxConfiguration_ofGroundY,
			 maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1, maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1,
			 double(maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1) / 2048.,
			 double(maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1) / double(fDAVHRR.fNIP->fScans) );
	fclose( fStat );

	sprintf( fname, "rlestprec" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%ld\t%.5f\t%.5f\t%.7f\t%.7f\t%.7f\t%.7f\t%.10f\t%.10f\t%.10f\t%.5f\t%.5f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%ld\t%.10f\t%.10f\n",
			 fGCPNumber, fDAVHRR.l_BaseX, fDAVHRR.l_BaseY, fResAvgM, fResStdDevM, fResRmsM, fResMaxM,
			 1000*fDAVHRR.error_roll, 1000*fDAVHRR.error_pitch, 1000*fDAVHRR.error_yaw,
			 fCorResAvgX, fCorResAvgY,
			 fDAVHRR.beta0_y, fDAVHRR.stand_dev_est_beta0_y, fDAVHRR.beta1_y, fDAVHRR.stand_dev_est_beta1_y,
			 fDAVHRR.SS_x, fDAVHRR.SS_y, fDAVHRR.sumX2_y, fDAVHRR.difX_y, fDAVHRR.SR );

	fclose( fStat );

	sprintf( fname, "rlestprecexp" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.7f\t%.7f\t%.7f\t%.7f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\n",
			 min_base_X, avg_base_X, max_base_X, std_dev_base_X, min_base_Y, avg_base_Y, max_base_Y, std_dev_base_Y,
			 min_rms_m, avg_rms_m, max_rms_m, std_dev_rms_m,
			 1000*min_epselon_roll, 1000*avg_epselon_roll, 1000*max_epselon_roll, 1000*std_dev_epselon_roll,
			 1000*min_epselon_pitch, 1000*avg_epselon_pitch, 1000*max_epselon_pitch, 1000*std_dev_epselon_pitch,
			 1000*min_epselon_yaw, 1000*avg_epselon_yaw, 1000*max_epselon_yaw, 1000*std_dev_epselon_yaw,
			 1000*min_mod_diff_yaw, 1000*avg_mod_diff_yaw, 1000*max_mod_diff_yaw, 1000*std_dev_mod_diff_yaw );

	fclose( fStat );
}


void print_LogStatSeriesFiles_onGCPs( double rms, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, uint8_t number, int n_gcp ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия log-файла статистики для записи!!!" );
	/*    fseek( fStat, 0, SEEK_END );
	    ulong length = ftell( fStat );
	    char * block = (char*)malloc( length );
	    fseek( fStat, 0, SEEK_SET );
	    fread( block, 1,  length, fStat );
	    char str = '\n';
	    char * point_write = strstr( block, &str );
	    for( ulong i=2; i < ID; i++ ){
	         point_write += 1;
	         point_write = strstr( point_write, &str );
	    }
	    point_write++;
	    ulong length1 = strlen( point_write );
	    point_write = strstr( point_write, &str );
	    point_write++;
	    fclose( fStat );

	    fwrite( block, 1, length - length1, fStat );
	    fwrite( point_write, 1, strlen( point_write), fStat );
	    free( block );*/

	uint8_t l;
	char * p;
	FILE * fStat;
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], fname_file[MAX_FNAME];
	#warning ацкое зло, но думать ломы
	splitpath( const_cast<char *> (inputFileName.c_str()), drive, dir, 0, 0 );

	// Конструирование полного имени файла лога.
	// Проверка существования каталога fLogDir.
	if( check_dir( fCS.fSourceDataDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath( const_cast<char *> (fCS.fSourceDataDir.c_str()), drive, dir, 0, 0 );
	else {
		//_fullpath( pathStatisticSeries, ".", MAX_PATH );
		//strcat( pathStatisticSeries, "\\" );
		//splitpath( pathStatisticSeries, drive, dir, 0, 0 );
		getcwd( pathStatisticSeries, MAX_PATH );
		int t = strlen(pathStatisticSeries);
		if( pathStatisticSeries[t-1] != DIRD ) {
			pathStatisticSeries[t] = DIRD;
			pathStatisticSeries[t+1] = '\0';
		}
		splitpath( pathStatisticSeries, drive, dir, 0, 0 );
	}

	l = strlen( dir );
	p = dir + l;
	if( strcmp( fDAVHRR.satName, "NOAA 12" ) == 0 ) {
		strcpy( p, output_statDirNOAA_12 );
	} else {
		if( strcmp( fDAVHRR.satName, "NOAA 15" ) == 0 ) {
			strcpy( p, output_statDirNOAA_15 );
		} else {
			if( strcmp( fDAVHRR.satName, "NOAA 17" ) == 0 ) {
				strcpy( p, output_statDirNOAA_17 );
			} else {
				if( strcmp( fDAVHRR.satName, "NOAA 14" ) == 0 ) {
					strcpy( p, output_statDirNOAA_14 );
				} else {
					if( strcmp( fDAVHRR.satName, "NOAA 16" ) == 0 ) {
						strcpy( p, output_statDirNOAA_16 );
					} else {
						strcpy( p, output_statDirSatelliteUnknown );
					}
				}
			}
		}
	}

	if( rms > .1 ) {
		sprintf( fname, "datapass_re" );
		makepath( pathStatisticSeries, drive, dir, fname, "stat" );
		if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
			throw ae1;
		}
		fprintf( fStat, "%s\t%d\t%d\t%.1f\n", fname_file, n_gcp, number, rms );
		fclose( fStat );
		return;
	}

	sprintf( fname, "anglesatt_re" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\t%.14f\n",
			 fDAVHRR.fMinCOP->roll, fDAVHRR.fMinCOP->pitch, fDAVHRR.fMinCOP->yaw,
			 fDAVHRR.fRegrCOP->roll, fDAVHRR.fRegrCOP->pitch, fDAVHRR.fRegrCOP->yaw,
			 fDAVHRR.fMinCOP->roll - fDAVHRR.fRegrCOP->roll, fDAVHRR.fMinCOP->pitch - fDAVHRR.fRegrCOP->pitch, fDAVHRR.fMinCOP->yaw - fDAVHRR.fRegrCOP->yaw,
			 fDAVHRR.fIniCOP->roll, fDAVHRR.fIniCOP->pitch, fDAVHRR.fIniCOP->yaw,
			 fDAVHRR.fIniCOP->roll - fDAVHRR.fMinCOP->roll, fDAVHRR.fIniCOP->pitch - fDAVHRR.fMinCOP->pitch, fDAVHRR.fIniCOP->yaw - fDAVHRR.fMinCOP->yaw,
			 fDAVHRR.fIniCOP->roll - fDAVHRR.fRegrCOP->roll, fDAVHRR.fIniCOP->pitch - fDAVHRR.fRegrCOP->pitch, fDAVHRR.fIniCOP->yaw - fDAVHRR.fRegrCOP->yaw );
	fclose( fStat );

	sprintf( fname, "rlresgcp_re" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%ld\t%ld\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\n",
			 fGCPNumber, fGCPNumber2, fResAvgM, fResRmsM, fResMaxM, fResStdDevM,
			 fResAvgX, fResRmsX, fResStdDevX, fResAvgMX, fResMaxMX, fResStdDevMX,
			 fResAvgY, fResRmsY, fResStdDevY, fResAvgMY, fResMaxMY, fResStdDevMY );
	fclose( fStat );

	sprintf( fname, "rlactgcp_re" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%ld\t%ld\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\n",
			 fGCPNumber, fGCPNumber2, fActAvgM, fActRmsM, fActMaxM, fActStdDevM,
			 fActAvgX, fActRmsX, fActStdDevX, fActAvgMX, fActMaxMX, fActStdDevMX,
			 fActAvgY, fActRmsY, fActStdDevY, fActAvgMY, fActMaxMY, fActStdDevMY );
	fclose( fStat );

	sprintf( fname, "rlestprec_re" );
	makepath( pathStatisticSeries, drive, dir, fname, "stat" );
	if( !(fStat = fopen( pathStatisticSeries, "a" )) ) {
		throw ae1;
	}
	fprintf( fStat, "%ld\t%.5f\t%.5f\t%.7f\t%.7f\t%.7f\t%.7f\t%.10f\t%.10f\t%.10f\t%.10f\t%.5f\t%.5f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%.10f\t%ld\t%.10f\t%.10f\n",
			 fGCPNumber, fDAVHRR.l_BaseX, fDAVHRR.l_BaseY, fResAvgM, fResStdDevM, fResRmsM, fResMaxM, fDAVHRR.max_error_angles,
			 fDAVHRR.error_roll, fDAVHRR.error_pitch, fDAVHRR.error_yaw,
			 fCorResAvgX, fCorResAvgY,
			 fDAVHRR.beta0_y, fDAVHRR.stand_dev_est_beta0_y, fDAVHRR.beta1_y, fDAVHRR.stand_dev_est_beta1_y,
			 fDAVHRR.SS_x, fDAVHRR.SS_y, fDAVHRR.sumX2_y, fDAVHRR.difX_y, fDAVHRR.SR );

	fclose( fStat );
}


//void print_LogFile( const char * message_text, bool flag_createFile = true, bool flag_2ch = false ) throw( TAccessExc ) {
void print_LogFile( const char * message_text, bool flag_createFile, bool flag_2ch ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия log-файла для записи!!!" );

	FILE * fLog;

	if( !flag_createFile ) {
		char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME];
		#warning ацкое зло, но думать ломы
		splitpath( const_cast<char *> (inputFileName.c_str()), drive, dir, 0, 0 );
		//strlwr( fname );

		// Конструирование полного имени файла лога.
		// Проверка существования каталога fLogDir.
		if( check_dir( fCS.fLogDir.c_str() ) == 0 )
			#warning ацкое зло, но думать ломы
			splitpath( const_cast<char *> (fCS.fLogDir.c_str()), drive, dir, 0, 0 );
		else {
			//_fullpath( pathLogFile, ".", MAX_PATH );
			//strcat( pathLogFile, "\\" );
			//splitpath( pathLogFile, drive, dir, 0, 0 );
			getcwd( pathLogFile, MAX_PATH );
			int t = strlen(pathLogFile);
			if( pathLogFile[t-1] != DIRD ) {
				pathLogFile[t] = DIRD;
				pathLogFile[t+1] = '\0';
			}
			splitpath( pathLogFile, drive, dir, 0, 0 );
		}
		makepath( pathLogFile, drive, dir, fname, "log" );

		if( !(fLog = fopen( pathLogFile, "w" )) ) {
			throw ae1;
		}

		fprintf( fLog, "Лог по обработке AUFITCHIP версии 1.00:\n******************************************************************************************\n\n" );
		fprintf( fLog, "max ошибки географ. привязки по строке:\t%ld;\n", fCS.fMaxNavigationErrorLine );
		fprintf( fLog, "max ошибки географ. привязки по столбцу:\t%ld;\n", fCS.fMaxNavigationErrorColumn );
		fprintf( fLog, "подробность маски и изображения:\t%ld;\n", fCS.fMaskScale );
		switch( fCS.fFiltrCloudyforChips ) {
		case 0:
			fprintf( fLog, "фильтрация облачности по чипам:\tнет;\n" );
			break;
		case 1:
			fprintf( fLog, "фильтрация облачности по чипам:\tда, без отбраковки;\n" );
			break;
		case 2:
			fprintf( fLog, "фильтрация облачности по чипам:\tда, с отбраковкой (порог = %.3f);\n", fCS.fMaxPercentageCloudy );
			break;
		}
		switch( number_algorithm ) {
		case 1:
			fprintf( fLog, "алгоритм расчёта параметров autoGCPs:\tморфологический вдоль контура с толщиной в 3 пиксела;\n" );
			break;
		case 2:
			fprintf( fLog, "алгоритм расчёта параметров autoGCPs:\tшаблонный по (расчёт уровней по квантилям: 0.2 и 0.8);\n" );
			break;
		}

		if( fCS.fRepeatCalcShiftsforChips == 1 )
			fprintf( fLog, "количество проходов:\tдва;\n" );
		else
			fprintf( fLog, "количество проходов:\tодин;\n" );

		fprintf( fLog, "порог для отбраковки по нормализ. значению критерия статист. знач-ти:\t%.3f;\n", fCS.fStatisticSignificanceThreshold );

		fprintf( fLog, "коэффициент для поправки параметров линейной регрессии смещения по столбцу:\t%.3f;\n", fCS.fLinRegresCoeff_dX );
		fprintf( fLog, "коэффициент для поправки параметров линейной регрессии смещения по строке:\t%.3f;\n", fCS.fLinRegresCoeff_dY );

		if( flag_2ch )
			fprintf( fLog, "расчёт autoGCPs проводился ли по 2 каналу AVHRR:\tда;\n" );
		else
			fprintf( fLog, "расчёт autoGCPs проводился ли по 2 каналу AVHRR:\tнет;\n" );

		fprintf( fLog, "\n<-------------------------------------------------------------------------------------------------------------------------------------------------------------->\n" );
		fclose( fLog );
	} else {
		fLog = fopen( pathLogFile, "a" );
		fprintf( fLog, "%s", message_text );
		fclose( fLog );
	}
};

const string& CppGetcwd()
{
	static char path[MAX_PATH];
	getcwd( path, MAX_PATH );
	int l = strlen( path);
	if( path[l-1] != DIRD ) {
		path[l] = DIRD;
		path[l+1] = '\0';
	}
	return string(path);
}

void LoadDirName(const Setting& set, const char* Val, string &dest)
{
	if( !set.lookupValue(Val, dest))
		dest = CppGetcwd();
}

void loadCfgSet( char * argv0 ) {
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], path[MAX_PATH];
	string buf;
	int i;
	Config cfg;

	char * t;
	if(NULL!=(t = getenv("TERM_ROOT"))){
		strcpy(dir,t);
		makepath( path, NULL, dir, "bin/aufitchip", "cfg" );
		//term_root = strdup(t);
		//fprintf( stdout, "Переменная TERM_ROOT = %s\n",term_root );
		//fflush(stdout);
	}else{
		fprintf( stderr, "Переменная TERM_ROOT не установлена, будет использован путь к исполняемому файлу.\n" );
		fflush(stderr);
		splitpath( argv0, drive, dir, fname, 0 );
		makepath( path, drive, dir, fname, "cfg" );
	}


	try {
		cfg.readFile( path );
	} catch(...) {
		print_log( "WARNING: Конфигурационный файл не найден! Установки приняты стандартными!" );
	}

	const Setting &root = cfg.getRoot();

	LoadDirName(root, "SourceDataDir", fCS.fSourceDataDir);

	LoadDirName(root, "TargetDataDir", fCS.fTargetDataDir);

	LoadDirName(root, "GPCXDir", fCS.fGPCXDir);

	LoadDirName(root, "LogDir", fCS.fLogDir);

	LoadDirName(root, "StatDir", fCS.fStatDir);

	LoadDirName(root, "Mask_and_ChipBaseDir", fCS.fMaskandChipBaseDir);

	LoadDirName(root, "ParametrsFiltrCloudyDir", fCS.ParametrsFiltrCloudyDir);


	if( !root.lookupValue("MinMethod", fCS.fMinMethod)) {
		fCS.fMinMethod = 2;         // Nelder-Mead
	}

	if( root.lookupValue( "MinParams", buf ) && buf.size() >= COR_PARAM_NUM ) {
	   // разбираем строку
		for( i = 0; i <= COR_PARAM_NUM; i++ ) {
			fCS.fMinParams[i] = (buf[i] == '0') ? 0 : 1;
		}
	} else {
		fCS.fMinParams[0] = 0;
		fCS.fMinParams[1] = 0;
		fCS.fMinParams[2] = 1;
		fCS.fMinParams[3] = 1;
		fCS.fMinParams[4] = 1;
		fCS.fMinParams[5] = 0;
		fCS.fMinParams[6] = 0;
		fCS.fMinParams[7] = 0;
		fCS.fMinParams[8] = 0;
		fCS.fMinParams[9] = 0;
		fCS.fMinParams[10] = 0;
		fCS.fMinParams[11] = 0;
	}

	if( !root.lookupValue("fMaxNavigationErrorLine", fCS.fMaxNavigationErrorLine) || fCS.fMaxNavigationErrorLine > 20){
		fCS.fMaxNavigationErrorLine = 15;
	}

	if( !root.lookupValue("fMaxNavigationErrorColumn", fCS.fMaxNavigationErrorColumn ) || fCS.fMaxNavigationErrorColumn > 15){
		fCS.fMaxNavigationErrorColumn  = 10;
	}

	if( !root.lookupValue("fMaskScale", fCS.fMaskScale ) || fCS.fMaskScale > 10){
		fCS.fMaskScale = 4;
	}
	/*
	    if( cfg->containsParamWithKey( "fFiltrCloudyforChips" ) ){
	        fCS.fFiltrCloudyforChips = atoi( cfg->getValue( "fFiltrCloudyforChips" ) );
	        if( fCS.fFiltrCloudyforChips < 0 || fCS.fFiltrCloudyforChips > 2 ){
	            fCS.fMaxPercentageCloudy = 2;
	        }
	    }
	    else{*/
	fCS.fFiltrCloudyforChips = 2;
	//    }

	if( !root.lookupValue("fMaxPercentageCloudy", fCS.fMaxPercentageCloudy) ||
		fCS.fMaxPercentageCloudy < .0 || fCS.fMaxPercentageCloudy > 90. ) {
		fCS.fMaxPercentageCloudy = 70.;
	}

	if( !root.lookupValue("fRepeatCalcShiftsforChips", fCS.fRepeatCalcShiftsforChips)||
		fCS.fRepeatCalcShiftsforChips != 0 && fCS.fRepeatCalcShiftsforChips != 1 ) {
		fCS.fRepeatCalcShiftsforChips = 1;
	}

	if( !root.lookupValue("fStatisticSignificanceThreshold", fCS.fStatisticSignificanceThreshold) ||
		fCS.fStatisticSignificanceThreshold < 0.01 || fCS.fStatisticSignificanceThreshold > 2. ) {
		fCS.fStatisticSignificanceThreshold = 0.4;
	}

	if( !root.lookupValue("fLinRegresCoeff_dX", fCS.fLinRegresCoeff_dX) ||
		fCS.fLinRegresCoeff_dX < 0.05 || fCS.fLinRegresCoeff_dX > 1.05 ) {
			fCS.fLinRegresCoeff_dX = 0.7;
	}

	if( !root.lookupValue("fLinRegresCoeff_dY", fCS.fLinRegresCoeff_dY) ||
		fCS.fLinRegresCoeff_dY < 0.05 || fCS.fLinRegresCoeff_dY > 1.05 ) {
			fCS.fLinRegresCoeff_dY = 0.95;
	}

};


void filtrCloudyGraundChips( TAufitChipMask & fACM, TFiltrCloudy & fFC ) {

	TCLCursor cursor = fACM.createCursor();

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] != TDChip::GraundChip )
			continue;
		long per = fFC.filtr_processing_for_chips( chip->X1 - fCS.fMaxNavigationErrorColumn, chip->Y1 - fCS.fMaxNavigationErrorColumn,
				   chip->X2 + fCS.fMaxNavigationErrorLine, chip->Y2 + fCS.fMaxNavigationErrorLine, chip->fRegionFiltrCloudy );
		chip->PercCloudy = ( per * 100.) /
						   double( (chip->X2 - chip->X1 + 1 + 2 * fCS.fMaxNavigationErrorColumn) * (chip->Y2 - chip->Y1 + 1 + 2 * fCS.fMaxNavigationErrorLine) );
	}
};


void filtrCloudyChips( TAufitChipMask & fACM, TFiltrCloudy & fFC ) {

	TCLCursor cursor = fACM.createCursor();

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		long per = fFC.filtr_processing_for_chips( chip->X1 - fCS.fMaxNavigationErrorColumn, chip->Y1 - fCS.fMaxNavigationErrorColumn,
				   chip->X2 + fCS.fMaxNavigationErrorLine, chip->Y2 + fCS.fMaxNavigationErrorLine, chip->fRegionFiltrCloudy );
		chip->PercCloudy = ( per * 100.) /
						   double( (chip->X2 - chip->X1 + 1 + 2 * fCS.fMaxNavigationErrorColumn) * (chip->Y2 - chip->Y1 + 1 + 2 * fCS.fMaxNavigationErrorLine) );
	}
};


void selection_Chips_for_GraundChips( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	unsigned N = fACM.numberOfGraundChips();   // Число опорных чипов.

	TCLCursor cursor = fACM.createCursor();
	if( fDAVHRR.fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip ||
					( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) ) {
				if( chip->PercCloudy >= fCS.fMaxPercentageCloudy ) {
					chip->fChipType[0] = TDChip::CheckChip;
					chip->fChipType[1] = TDChip::CheckChip;
				}
			}
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				if( chip->PercCloudy >= fCS.fMaxPercentageCloudy )
					chip->fChipType[0] = TDChip::CheckChip;
			}
		}
	}

	char buffer[100];
	sprintf( buffer, "количествово чипов, отбракованных после фильтрации облачности:\t%lu;\n", N - fACM.numberOfGraundChips() );
	print_LogFile( buffer );
};



void _calculateShifts_forGraundChips( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	TCLCursor cursor = fACM.createCursor();
	fACM.settingShiftsSearchZone( fCS.fMaxNavigationErrorLine, fCS.fMaxNavigationErrorColumn );

	unsigned long NG, NC;
	unsigned ngcp = 1;
	fACM.fChipType = TDChip::GraundChip;
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip || ( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) ) {
			switch(  number_algorithm  ) {
			case 1:
				fACM.calculateShiftChip_Morphological_MaxAverage( ngcp, chip );
				break;
			case 2:
				fACM.calculateShiftChip_Templet_Quant2080( chip );
				break;
			}
			ngcp++;
		}
	}
	fACM.verification_parametrsGraundChips( false );
	fACM.verification_parametrsGraundChipsAbs( false );

	saveStatisticaContour( fDAVHRR, fACM );
	fACM.setAll6ToCheckChips();

	selection_GraundChips_for_GCPs( false, fDAVHRR, fACM );
	save_StatMaxGCP( false, 1, fDAVHRR, fACM );
	save_StatMaxGCP( true, 2, fDAVHRR, fACM );

	NC = fACM.numberOfControlChips();
	{
		FILE * fLog = fopen( pathLogFile, "a" );
		if( NC <= 0 ) {
			fprintf( fLog,"\nпервый проход, отбрак-е опорные чипы по нормализ. критерию стат-й знач-ти:\tнет\n" );
		} else {
			fprintf( fLog,"\nпервый проход, отбрак-е опорные чипы по нормализ. критерию стат-й знач-ти (%lu):\n", NC );
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::ControlChip )
					fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
							 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
							 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
					fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
							 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
							 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
			}
		}
		fclose( fLog );
	}

	NG = fACM.numberOfGraundChips();
	if( fCS.fRepeatCalcShiftsforChips == 1 && NC > 0 && NG > 0 ) {
		ngcp = 1;

		fActAvgX = .0;
		fActAvgY = .0;
		fGCPNumber = 0L;
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				fActAvgX += chip->dX[0];
				fActAvgY += chip->dY[0];
				fGCPNumber++;
			}
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				fActAvgX += chip->dX[1];
				fActAvgY += chip->dY[1];
				fGCPNumber++;
			}
		}
		fACM.settingAvgShifts( fGCPNumber, fActAvgX, fActAvgY );

		{
			FILE * fLog = fopen( pathLogFile, "a" );
			fprintf( fLog,"\nКоличество 'хороших' реперных точек после первого прохода и средняя оценка для смещений, вычисленная по ним:\n" );
			fprintf( fLog, "%ld\t %.5f\t %.5f\n", fGCPNumber, fActAvgX/fGCPNumber, fActAvgY/fGCPNumber );
			fclose( fLog );
		}

		fACM.fChipType = TDChip::ControlChip;
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::ControlChip || ( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip ) ) {
				switch(  number_algorithm  ) {
				case 1:
					fACM.calculateShiftChip_Morphological_MaxAverage( ngcp, chip );
					break;
				case 2:
					fACM.calculateShiftChip_Templet_Quant2080( chip );
					break;
				}
				ngcp++;
			}
		}
		fACM.verification_parametrsGraundChips( true );
		fACM.verification_parametrsGraundChipsAbs( true );

		if( option_gcp_to_pcx ) {
			try {
				fACM.result_MaskorContour_GraundChipsorautoGCPs_to_pcx( false, mask_or_contour, fCS.fGPCXDir, inputFileName );
			} catch( TAccessExc ae ) {
				errorMessage( ae.what() );
			}
		}

		selection_GraundChips_for_GCPs( true, fDAVHRR, fACM );
		save_StatMaxGCP( false, 3, fDAVHRR, fACM );
		save_StatMaxGCP( true, 4, fDAVHRR, fACM );
	}
};


void calculateShifts_forGraundChips( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	TCLCursor cursor = fACM.createCursor();
	fACM.settingShiftsSearchZone( fCS.fMaxNavigationErrorLine, fCS.fMaxNavigationErrorColumn );

	unsigned long NG, NC;
	unsigned ngcp = 1;
	fACM.fChipType = TDChip::GraundChip;
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip || ( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) ) {
			switch(  number_algorithm  ) {
			case 1:
				fACM.calculateShiftChip_Morphological_MaxAverage( ngcp, chip );
				break;
			case 2:
				fACM.calculateShiftChip_Templet_Quant2080( chip );
				break;
			}
			ngcp++;
		}
	}
	fACM.verification_parametrsGraundChips( false );
	//    fACM.verification_parametrsGraundChipsAbs( false );

	//    saveStatisticaContour( fDAVHRR, fACM );
	//    fACM.setAll6ToCheckChips();

	selection_GraundChips_for_GCPs( false, fDAVHRR, fACM );
	//    save_StatMaxGCP( false, 1, fDAVHRR, fACM );
	//    save_StatMaxGCP( true, 2, fDAVHRR, fACM );

	NC = fACM.numberOfControlChips();
	{
		FILE * fLog = fopen( pathLogFile, "a" );
		if( NC <= 0 ) {
			fprintf( fLog,"\nпервый проход, отбрак-е опорные чипы по нормализ. критерию стат-й знач-ти:\tнет\n" );
		} else {
			fprintf( fLog,"\nпервый проход, отбрак-е опорные чипы по нормализ. критерию стат-й знач-ти (%lu):\n", NC );
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::ControlChip )
					fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
							 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
							 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
					fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
							 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
							 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
			}
		}
		fclose( fLog );
	}

	NG = fACM.numberOfGraundChips();
	if( fCS.fRepeatCalcShiftsforChips == 1 && NC > 0 && NG > 0 ) {
		ngcp = 1;

		fActAvgX = .0;
		fActAvgY = .0;
		fGCPNumber = 0L;
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				fActAvgX += chip->dX[0];
				fActAvgY += chip->dY[0];
				fGCPNumber++;
			}
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				fActAvgX += chip->dX[1];
				fActAvgY += chip->dY[1];
				fGCPNumber++;
			}
		}
		fACM.settingAvgShifts( fGCPNumber, fActAvgX, fActAvgY );

		{
			FILE * fLog = fopen( pathLogFile, "a" );
			fprintf( fLog,"\nКоличество 'хороших' реперных точек после первого прохода и средняя оценка для смещений, вычисленная по ним:\n" );
			fprintf( fLog, "%ld\t %.5f\t %.5f\n", fGCPNumber, fActAvgX/fGCPNumber, fActAvgY/fGCPNumber );
			fclose( fLog );
		}

		fACM.fChipType = TDChip::ControlChip;
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::ControlChip || ( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip ) ) {
				switch(  number_algorithm  ) {
				case 1:
					fACM.calculateShiftChip_Morphological_MaxAverage( ngcp, chip );
					break;
				case 2:
					fACM.calculateShiftChip_Templet_Quant2080( chip );
					break;
				}
				ngcp++;
			}
		}
		fACM.verification_parametrsGraundChips( true );
		//        fACM.verification_parametrsGraundChipsAbs( true );

		if( option_gcp_to_pcx ) {
			try {
				fACM.result_MaskorContour_GraundChipsorautoGCPs_to_pcx( false, mask_or_contour, fCS.fGPCXDir, inputFileName );
			} catch( TAccessExc ae ) {
				errorMessage( ae.what() );
			}
		}

		selection_GraundChips_for_GCPs( true, fDAVHRR, fACM );
		//        save_StatMaxGCP( false, 3, fDAVHRR, fACM );
		//        save_StatMaxGCP( true, 4, fDAVHRR, fACM );
	}
};


void selection_GraundChips_for_GCPs( bool flag_passage, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	TCLCursor cursor = fACM.createCursor();

	if( !flag_passage ) {
		if( fDAVHRR.fChannels_forCalcGCPs[1] ) {
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::GraundChip ) {
					if( chip->fNormStatSign[0] < fCS.fStatisticSignificanceThreshold )
						chip->fChipType[0] = TDChip::ControlChip;
				}
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
					if( chip->fNormStatSign[1] < fCS.fStatisticSignificanceThreshold )
						chip->fChipType[1] = TDChip::ControlChip;
				}
			}
		} else {
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::GraundChip ) {
					if( chip->fNormStatSign[0] < fCS.fStatisticSignificanceThreshold )
						chip->fChipType[0] = TDChip::ControlChip;
				}
			}
		}

		{
			FILE * fLog = fopen( pathLogFile, "a" );
			fprintf( fLog, "\nпервый проход, 'хорошие' опорные чипы (%ld):\n", fACM.numberOfGraundChips() );
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::GraundChip )
					fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
							 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
							 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
					fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
							 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
							 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
			}
			fclose( fLog );
		}
	} else {
		print_LogFile( "\nвторой проход, 'хорошие' опорные чипы среди отбракованных по нормализ. критерию стат. знач.:\n" );
		FILE * fLog = fopen( pathLogFile, "a" );
		unsigned n = 0;
		if( fDAVHRR.fChannels_forCalcGCPs[1] ) {
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::ControlChip ) {
					if( chip->fNormStatSign[0] >= fCS.fStatisticSignificanceThreshold )
						chip->fChipType[0] = TDChip::GraundChip;
					if( chip->fChipType[0] == TDChip::GraundChip ) {
						n++;
						fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
								 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
								 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
					}
				}
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip ) {
					if( chip->fNormStatSign[1] >= fCS.fStatisticSignificanceThreshold )
						chip->fChipType[1] = TDChip::GraundChip;
					if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
						n++;
						fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
								 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
								 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
					}
				}
			}
		} else {
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::ControlChip ) {
					if( chip->fNormStatSign[0] >= fCS.fStatisticSignificanceThreshold )
						chip->fChipType[0] = TDChip::GraundChip;
					if( chip->fChipType[0] == TDChip::GraundChip ) {
						n++;
						fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
								 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
								 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
					}
				}
			}
		}
		fprintf( fLog, "кол-во таких реперных точек:\t%u\n", n );
		fclose( fLog );
	}
};


void calcConfigBases_forAutoGCPs( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, unsigned & centerConfig_X, double & base_X, unsigned & centerConfig_Y, double & base_Y ) {
	TCLCursor cursor = fACM.createCursor();
	if( !fACM.numberOfGraundChips() )
		return;

	minConfiguration_ofGroundX = 10000L;
	maxConfiguration_ofGroundX = 0L;
	minConfiguration_ofGroundY = 10000L;
	maxConfiguration_ofGroundY = 0L;

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			if( minConfiguration_ofGroundX >= long(chip->X1+chip->ChipSize().x()/2) )
				minConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( minConfiguration_ofGroundY >= long(chip->Y1+chip->ChipSize().y()/2) )
				minConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
			if( maxConfiguration_ofGroundX <= long(chip->X1+chip->ChipSize().x()/2) )
				maxConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( maxConfiguration_ofGroundY <= long(chip->Y1+chip->ChipSize().y()/2) )
				maxConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
		}

		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			if( minConfiguration_ofGroundX >= long(chip->X1+chip->ChipSize().x()/2) )
				minConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( minConfiguration_ofGroundY >= long(chip->Y1+chip->ChipSize().y()/2) )
				minConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
			if( maxConfiguration_ofGroundX <= long(chip->X1+chip->ChipSize().x()/2) )
				maxConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( maxConfiguration_ofGroundY <= long(chip->Y1+chip->ChipSize().y()/2) )
				maxConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
		}
	}

	centerConfig_X = unsigned( (maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1) / 2 );
	base_X = double(maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1) / 2048.;

	centerConfig_Y = unsigned( (maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1) / 2 );
	base_Y = double(maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1) / double(fDAVHRR.fNIP->fScans);
};


void calcResDisp_forAutoGCPs( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, double & base_x, double & base_y, double & res_m ) {
	TCLCursor cursor = fACM.createCursor();
	fGCPNumber = fACM.numberOfGraundChips();   // Число GCPs.
	if( !fGCPNumber )
		return;

	double m_data;
	minConfiguration_ofGroundX = 10000L;
	maxConfiguration_ofGroundX = 0L;
	minConfiguration_ofGroundY = 10000L;
	maxConfiguration_ofGroundY = 0L;

	res_m = .0;
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			m_data = chip->Modulus( false, 0 );
			res_m += m_data * m_data;
			if( minConfiguration_ofGroundX >= long(chip->X1+chip->ChipSize().x()/2) )
				minConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( minConfiguration_ofGroundY >= long(chip->Y1+chip->ChipSize().y()/2) )
				minConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
			if( maxConfiguration_ofGroundX <= long(chip->X1+chip->ChipSize().x()/2) )
				maxConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( maxConfiguration_ofGroundY <= long(chip->Y1+chip->ChipSize().y()/2) )
				maxConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
		}

		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			m_data = chip->Modulus( false, 1 );
			res_m += m_data * m_data;
			if( minConfiguration_ofGroundX >= long(chip->X1+chip->ChipSize().x()/2) )
				minConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( minConfiguration_ofGroundY >= long(chip->Y1+chip->ChipSize().y()/2) )
				minConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
			if( maxConfiguration_ofGroundX <= long(chip->X1+chip->ChipSize().x()/2) )
				maxConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( maxConfiguration_ofGroundY <= long(chip->Y1+chip->ChipSize().y()/2) )
				maxConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
		}
	}

	res_m = sqrt( res_m / fGCPNumber );
	base_x = double(maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1) / 2048.;
	base_y = double(maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1) / double(fDAVHRR.fNIP->fScans);
};


void calcResDisp_forAutoGCPs( TAufitChipMask & fACM ) {
	TCLCursor cursor = fACM.createCursor();
	fGCPNumber = fACM.numberOfGraundChips();   // Число GCPs.
	if( !fGCPNumber )
		return;

	double * m_data = new double [ fGCPNumber ];
	double * x_data = new double [ fGCPNumber ];
	double * xm_data = new double [ fGCPNumber ];
	double * y_data = new double [ fGCPNumber ];
	double * ym_data = new double [ fGCPNumber ];

	int i=0;
	double var, avg_dev, skew, curt;  //, std_dev  // не используются

	fGCPNumber2 = 0L;
	fResAvgM = .0;
	fResRmsM = .0;
	fResMaxM = .0;
	fResStdDevM = .0;
	fResAvgX = .0;
	fResRmsX = .0;
	fResStdDevX = .0;
	fResAvgMX = .0;
	fResMaxMX = .0;
	fResStdDevMX = .0;
	fResAvgY = .0;
	fResRmsY = .0;
	fResStdDevY = .0;
	fResAvgMY = .0;
	fResMaxMY = .0;
	fResStdDevMY = .0;

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			m_data[i] = chip->Modulus( false, 0 );
			fResRmsM += m_data[i] * m_data[i];
			x_data[i] = chip->dX[0];
			xm_data[i] = fabs( chip->dX[0] );
			fResRmsX += xm_data[i] * xm_data[i];
			y_data[i] = chip->dY[0];
			ym_data[i] = fabs( chip->dY[0] );
			fResRmsY += ym_data[i] * ym_data[i];

			if( m_data[i] > fResMaxM )
				fResMaxM = m_data[i];
			if( xm_data[i] > fResMaxMX )
				fResMaxMX = xm_data[i];
			if( ym_data[i] > fResMaxMY )
				fResMaxMY = ym_data[i];
			i++;
		}

		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			m_data[i] = chip->Modulus( false, 1 );
			fResRmsM += m_data[i] * m_data[i];
			x_data[i] = chip->dX[1];
			xm_data[i] = fabs( chip->dX[1] );
			fResRmsX += xm_data[i] * xm_data[i];
			y_data[i] = chip->dY[1];
			ym_data[i] = fabs( chip->dY[1] );
			fResRmsY += ym_data[i] * ym_data[i];

			if( m_data[i] > fResMaxM )
				fResMaxM = m_data[i];
			if( xm_data[i] > fResMaxMX )
				fResMaxMX = xm_data[i];
			if( ym_data[i] > fResMaxMY )
				fResMaxMY = ym_data[i];
			fGCPNumber2++;
			i++;
		}
	}

	d_moment( m_data-1, fGCPNumber, &fResAvgM, &var, &fResStdDevM, &avg_dev, &skew, &curt );
	d_moment( x_data-1, fGCPNumber, &fResAvgX, &var, &fResStdDevX, &avg_dev, &skew, &curt );
	d_moment( y_data-1, fGCPNumber, &fResAvgY, &var, &fResStdDevY, &avg_dev, &skew, &curt );
	d_moment( xm_data-1, fGCPNumber, &fResAvgMX, &var, &fResStdDevMX, &avg_dev, &skew, &curt );
	d_moment( ym_data-1, fGCPNumber, &fResAvgMY, &var, &fResStdDevMY, &avg_dev, &skew, &curt );

	fResRmsM = sqrt( fResRmsM / fGCPNumber );
	fResRmsX = sqrt( fResRmsX / fGCPNumber );
	fResRmsY = sqrt( fResRmsY / fGCPNumber );

	delete [] m_data;
	delete [] x_data;
	delete [] xm_data;
	delete [] y_data;
	delete [] ym_data;
};


void calcResDisp_forAutoGCPs( TAufitChipMask & fACM, double & res_avg_dx, double & res_avg_dy ) {
	TCLCursor cursor = fACM.createCursor();
	fGCPNumber = fACM.numberOfGraundChips();   // Число GCPs.
	if( !fGCPNumber )
		return;

	res_avg_dx = .0;
	res_avg_dy = .0;

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			res_avg_dx += chip->dX[0];
			res_avg_dy += chip->dY[0];
		}

		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			res_avg_dx += chip->dX[1];
			res_avg_dy += chip->dY[1];
		}
	}

	res_avg_dx = res_avg_dx / double(fGCPNumber);
	res_avg_dy = res_avg_dy / double(fGCPNumber);
};


void calcResDisp_forAutoGCPs( int n, TDChip * chips ) {
	if( !n )
		return;

	double m_data;

	fGCPNumber = 0;   // Число GCPs.
	fResAvgMX = .0;
	fResStdDevMX = .0;
	fResAvgMY = .0;
	fResStdDevMY = .0;
	fResRmsM = .0;
	fResMaxM = .0;

	for( int i = 0; i < n; i++ ) {
		if( chips[i].fChipType[0] == TDChip::GraundChip ) {
			m_data = chips[i].Modulus( false, 0 );
			fResRmsM += m_data * m_data;
			//             if( m_data > fResMaxM ) fResMaxM = m_data;

			fResAvgMX += fabs( chips[i].dX[0] );
			fResStdDevMX += chips[i].dX[0] * chips[i].dX[0];

			fResAvgMY += fabs( chips[i].dY[0] );
			fResStdDevMY += chips[i].dY[0] * chips[i].dY[0];
			fGCPNumber++;
		} else {
			if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
				m_data = chips[i].Modulus( false, 1 );
				fResRmsM += m_data * m_data;
				//                 if( m_data > fResMaxM ) fResMaxM = m_data;

				fResAvgMX += fabs( chips[i].dX[1] );
				fResStdDevMX += chips[i].dX[1] * chips[i].dX[1];

				fResAvgMY += fabs( chips[i].dY[1] );
				fResStdDevMY += chips[i].dY[1] * chips[i].dY[1];
				fGCPNumber++;
			}
		}
	}

	fResRmsM = sqrt( fResRmsM / fGCPNumber );
	fResStdDevMX = sqrt( ( fResStdDevMX - ( ( fResAvgMX * fResAvgMX ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
	fResAvgMX = fResAvgMX / fGCPNumber;
	fResStdDevMY = sqrt( ( fResStdDevMY - ( ( fResAvgMY * fResAvgMY ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
	fResAvgMY = fResAvgMY / fGCPNumber;
};


void calcLinearRegressionCoefficients( int n, TDChip * chips, double & beta0_x, double & beta0_y, double & beta1_y ) {
	if( !n )
		return;
	long gcp_num = 0;
	//-----------------------------------------------------------------------------
	// собственно получение коэффициетов линейной регссии
	//-----------------------------------------------------------------------------
	double lr_x, lr_y, sum_x = .0, summ_x = .0, sum_y = .0, sum_x2 = .0, sum_xy = .0;
	for( int i = 0; i < n; i++ ) {
		if( chips[i].fChipType[0] == TDChip::GraundChip ) {
			lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2);
			lr_y = chips[i].dY[0];
			sum_x += lr_x;
			sum_y += lr_y;
			sum_x2 += lr_x * lr_x;
			sum_xy += lr_x * lr_y;
			gcp_num++;
			summ_x += chips[i].dX[0];
		} else {
			if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
				lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2);
				lr_y = chips[i].dY[1];
				sum_x += lr_x;
				sum_y += lr_y;
				sum_x2 += lr_x * lr_x;
				sum_xy += lr_x * lr_y;
				gcp_num++;
				summ_x += chips[i].dX[1];
			}
		}
	}

	beta0_x = summ_x / double(gcp_num);
	beta1_y = ( gcp_num * sum_xy - sum_x * sum_y ) / double( gcp_num * sum_x2 - sum_x * sum_x );
	beta0_y = ( sum_y - beta1_y * sum_x ) / double(gcp_num);
};


void calcResDisp_forAutoGCPs( int n, TDChip * chips, double beta0_x, double beta0_y, double beta1_y ) {
	if( !n )
		return;
	double x_data, y_data;

	fGCPNumber = 0;   // Число GCPs.
	fResAvgMX = .0;
	fResStdDevMX = .0;
	fResAvgMY = .0;
	fResStdDevMY = .0;
	fResRmsM = .0;

	for( int i = 0; i < n; i++ ) {
		if( chips[i].fChipType[0] == TDChip::GraundChip ) {
			x_data = chips[i].dX[0] - beta0_x;
			y_data = chips[i].dY[0] - beta1_y * (chips[i].X1 + chips[i].ChipSize().x()/2) - beta0_y;
			fResRmsM += pow( x_data, 2. ) + pow( y_data, 2. );


			fResAvgMX += fabs( x_data );
			fResStdDevMX += x_data * x_data;

			fResAvgMY += fabs( y_data );
			fResStdDevMY += y_data * y_data;
			fGCPNumber++;
		} else {
			if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
				x_data = chips[i].dX[1] - beta0_x;
				y_data = chips[i].dY[1] - beta1_y * (chips[i].X1 + chips[i].ChipSize().x()/2) - beta0_y;
				fResRmsM += pow( x_data, 2. ) + pow( y_data, 2. );

				fResAvgMX += fabs( x_data );
				fResStdDevMX += x_data * x_data;

				fResAvgMY += fabs( y_data );
				fResStdDevMY += y_data * y_data;
				fGCPNumber++;
			}
		}
	}

	fResRmsM = sqrt( fResRmsM / fGCPNumber );
	fResStdDevMX = sqrt( ( fResStdDevMX - ( ( fResAvgMX * fResAvgMX ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
	fResAvgMX = fResAvgMX / fGCPNumber;
	fResStdDevMY = sqrt( ( fResStdDevMY - ( ( fResAvgMY * fResAvgMY ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
	fResAvgMY = fResAvgMY / fGCPNumber;
};



void calcStatParams_forAutoGCPs( bool flagMin, bool flagstd, TAufitChipMask & fACM ) {
	TCLCursor cursor = fACM.createCursor();
	fGCPNumber = fACM.numberOfGraundChips();   // Число GCPs.
	if( !fGCPNumber )
		return;

	double dx, dy, mod;
	if( !flagstd ) {
		fGCPNumber2 = 0L;
		fResAvgM = .0;
		fResRmsM = .0;
		fResMaxM = .0;
		fResStdDevM = .0;
		fResAvgX = .0;
		fResRmsX = .0;
		fResStdDevX = .0;
		fResAvgMX = .0;
		fResMaxMX = .0;
		fResStdDevMX = .0;
		fResAvgY = .0;
		fResRmsY = .0;
		fResStdDevY = .0;
		fResAvgMY = .0;
		fResMaxMY = .0;
		fResStdDevMY = .0;

		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				dx = chip->dX[0];
				dy = chip->dY[0];
				mod = chip->Modulus( false, 0 );
				fResAvgM += mod;
				fResRmsM += mod * mod;
				fResAvgX += dx;
				fResAvgMX += fabs( dx );
				fResRmsX += dx * dx;
				fResAvgY += dy;
				fResAvgMY += fabs( dy );
				fResRmsY += dy * dy;

				if( mod > fResMaxM )
					fResMaxM = mod;
				if( fabs( dx ) > fResMaxMX )
					fResMaxMX = fabs( dx );
				if( fabs( dy ) > fResMaxMY )
					fResMaxMY = fabs( dy );
			}

			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				dx = chip->dX[1];
				dy = chip->dY[1];
				mod = chip->Modulus( false, 1 );
				fResAvgM += mod;
				fResRmsM += mod * mod;
				fResAvgX += dx;
				fResAvgMX += fabs( dx );
				fResRmsX += dx * dx;
				fResAvgY += dy;
				fResAvgMY += fabs( dy );
				fResRmsY += dy * dy;

				if( mod > fResMaxM )
					fResMaxM = mod;
				if( fabs( dx ) > fResMaxMX )
					fResMaxMX = fabs( dx );
				if( fabs( dy ) > fResMaxMY )
					fResMaxMY = fabs( dy );
				fGCPNumber2++;
			}
		}

		fResStdDevM = sqrt( ( fResRmsM - ( ( fResAvgM * fResAvgM ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fResAvgM = fResAvgM / fGCPNumber;
		fResRmsM = sqrt( fResRmsM / fGCPNumber );

		fResStdDevX = sqrt( ( fResRmsX - ( ( fResAvgX * fResAvgX ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fResAvgX = fResAvgX / fGCPNumber;
		fResStdDevMX = sqrt( ( fResRmsX - ( ( fResAvgMX * fResAvgMX ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fResAvgMX = fResAvgMX / fGCPNumber;
		fResRmsX = sqrt( fResRmsX / fGCPNumber );

		fResStdDevY = sqrt( ( fResRmsY - ( ( fResAvgY * fResAvgY ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fResAvgY = fResAvgY / fGCPNumber;
		fResStdDevMY = sqrt( ( fResRmsY - ( ( fResAvgMY * fResAvgMY ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fResAvgMY = fResAvgMY / fGCPNumber;
		fResRmsY = sqrt( fResRmsY / fGCPNumber );
		/*
		        FILE * fLog = fopen( pathLogFile, "a" );
		        fprintf( fLog, "\nвеличины первых и вторых моментов, а также rms и максимальные значения остаточных невязок в GCPs (%u):\n", fGCPNumber );
		        fprintf( fLog, "%.5f %.5f %.5f %.5f\t; среднее, rms, максимальное значение модуля невязки, стандартное отклонение\n", fResAvgM, fResRmsM, fResMaxM, fResStdDevM );
		        fprintf( fLog, "%.5f %.5f %.5f %.5f %.5f %.5f\t; среднее, rms и стандартное отклонение значения X-компоненты невязки; среднее, максимальное и стандартное отклонение значения модуля X-компоненты невязки\n", fResAvgX, fResRmsX, fResStdDevX, fResAvgMX, fResMaxMX, fResStdDevMX );
		        fprintf( fLog, "%.5f %.5f %.5f %.5f %.5f %.5f\t; среднее, rms и стандартное отклонение значения Y-компоненты невязки; среднее, максимальное и стандартное отклонение значения модуля Y-компоненты невязки\n", fResAvgY, fResRmsY, fResStdDevY, fResAvgMY, fResMaxMY, fResStdDevMY );
		        fclose( fLog );
		*/
		if( !flagMin ) {
			fMGCPNumber = fGCPNumber;
			fMGCPNumber2 = fGCPNumber2;
			fMResAvgM = fResAvgM;
			fMResRmsM = fResRmsM;
			fMResMaxM = fResMaxM;
			fMResStdDevM = fResStdDevM;
			fMResAvgX = fResAvgX;
			fMResRmsX = fResRmsX;
			fMResStdDevX = fResStdDevX;
			fMResAvgMX = fResAvgMX;
			fMResMaxMX = fResMaxMX;
			fMResStdDevMX = fResStdDevMX;
			fMResAvgY = fResAvgY;
			fMResRmsY = fResRmsY;
			fMResStdDevY = fResStdDevY;
			fMResAvgMY = fResAvgMY;
			fMResMaxMY = fResMaxMY;
			fMResStdDevMY = fResStdDevMY;
		}
	} else {
		fGCPNumber2 = 0L;
		fActAvgM = .0;
		fActRmsM = .0;
		fActMaxM = .0;
		fActStdDevM = .0;
		fActAvgX = .0;
		fActRmsX = .0;
		fActStdDevX = .0;
		fActAvgMX = .0;
		fActMaxMX = .0;
		fActStdDevMX = .0;
		fActAvgY = .0;
		fActRmsY = .0;
		fActStdDevY = .0;
		fActAvgMY = .0;
		fActMaxMY = .0;
		fActStdDevMY = .0;

		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				dx = chip->std_dX[0];
				dy = chip->std_dY[0];
				mod = chip->Modulus( true, 0 );
				fActAvgM += mod;
				fActRmsM += mod * mod;
				fActAvgX += dx;
				fActAvgMX += fabs( dx );
				fActRmsX += dx * dx;
				fActAvgY += dy;
				fActAvgMY += fabs( dy );
				fActRmsY += dy * dy;

				if( mod > fActMaxM )
					fActMaxM = mod;
				if( fabs( dx ) > fActMaxMX )
					fActMaxMX = fabs( dx );
				if( fabs( dy ) > fActMaxMY )
					fActMaxMY = fabs( dy );
			}

			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				dx = chip->std_dX[1];
				dy = chip->std_dY[1];
				mod = chip->Modulus( true, 1 );
				fActAvgM += mod;
				fActRmsM += mod * mod;
				fActAvgX += dx;
				fActAvgMX += fabs( dx );
				fActRmsX += dx * dx;
				fActAvgY += dy;
				fActAvgMY += fabs( dy );
				fActRmsY += dy * dy;

				if( mod > fActMaxM )
					fActMaxM = mod;
				if( fabs( dx ) > fActMaxMX )
					fActMaxMX = fabs( dx );
				if( fabs( dy ) > fActMaxMY )
					fActMaxMY = fabs( dy );
				fGCPNumber2++;
			}
		}

		fActStdDevM = sqrt( ( fActRmsM - ( ( fActAvgM * fActAvgM ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fActAvgM = fActAvgM / fGCPNumber;
		fActRmsM = sqrt( fActRmsM / fGCPNumber );

		fActStdDevX = sqrt( ( fActRmsX - ( ( fActAvgX * fActAvgX ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fActAvgX = fActAvgX / fGCPNumber;
		fActStdDevMX = sqrt( ( fActRmsX - ( ( fActAvgMX * fActAvgMX ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fActAvgMX = fActAvgMX / fGCPNumber;
		fActRmsX = sqrt( fActRmsX / fGCPNumber );

		fActStdDevY = sqrt( ( fActRmsY - ( ( fActAvgY * fActAvgY ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fActAvgY = fActAvgY / fGCPNumber;
		fActStdDevMY = sqrt( ( fActRmsY - ( ( fActAvgMY * fActAvgMY ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
		fActAvgMY = fActAvgMY / fGCPNumber;
		fActRmsY = sqrt( fActRmsY / fGCPNumber );
		/*
		        FILE * fLog = fopen( pathLogFile, "a" );
		        fprintf( fLog, "\nвеличины первых и вторых моментов, а также rms и максимальные значения исходных невязок в GCPs (%u):\n", fGCPNumber );
		        fprintf( fLog, "%.5f %.5f %.5f %.5f\t; среднее, rms, максимальное значение модуля невязки, стандартное отклонение\n", fActAvgM, fActRmsM, fActMaxM, fActStdDevM );
		        fprintf( fLog, "%.5f %.5f %.5f %.5f %.5f %.5f\t; среднее, rms и стандартное отклонение значения X-компоненты невязки; среднее, максимальное и стандартное отклонение значения модуля X-компоненты невязки\n", fActAvgX, fActRmsX, fActStdDevX, fActAvgMX, fActMaxMX, fActStdDevMX );
		        fprintf( fLog, "%.5f %.5f %.5f %.5f %.5f %.5f\t; среднее, rms и стандартное отклонение значения Y-компоненты невязки; среднее, максимальное и стандартное отклонение значения модуля Y-компоненты невязки\n", fActAvgY, fActRmsY, fActStdDevY, fActAvgMY, fActMaxMY, fActStdDevMY );
		        fclose( fLog );
		*/
		if( !flagMin ) {
			fMGCPNumber = fGCPNumber;
			fMGCPNumber2 = fGCPNumber2;
			fMActAvgM = fActAvgM;
			fMActRmsM = fActRmsM;
			fMActMaxM = fActMaxM;
			fMActStdDevM = fActStdDevM;
			fMActAvgX = fActAvgX;
			fMActRmsX = fActRmsX;
			fMActStdDevX = fActStdDevX;
			fMActAvgMX = fActAvgMX;
			fMActMaxMX = fActMaxMX;
			fMActStdDevMX = fActStdDevMX;
			fMActAvgY = fActAvgY;
			fMActRmsY = fActRmsY;
			fMActStdDevY = fActStdDevY;
			fMActAvgMY = fActAvgMY;
			fMActMaxMY = fActMaxMY;
			fMActStdDevMY = fActStdDevMY;
		}
	}
};



void calcStatParams_forControlGCPs( TAufitChipMask & fACM ) {
	fCheckGCPNumber2 = 0L;
	fCheckResAvgM = .0;
	fCheckResRmsM = .0;
	fCheckResMaxM = .0;
	fCheckResStdDevM = .0;
	fCheckResAvgX = .0;
	fCheckResRmsX = .0;
	fCheckResStdDevX = .0;
	fCheckResAvgMX = .0;
	fCheckResMaxMX = .0;
	fCheckResStdDevMX = .0;
	fCheckResAvgY = .0;
	fCheckResRmsY = .0;
	fCheckResStdDevY = .0;
	fCheckResAvgMY = .0;
	fCheckResMaxMY = .0;
	fCheckResStdDevMY = .0;

	TCLCursor cursor = fACM.createCursor();
	fCheckGCPNumber = fACM.numberOfControlChips();   // Число GCPs.
	if( !fCheckGCPNumber )
		return;

	double dx, dy, mod;
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::ControlChip ) {
			dx = chip->dX[0];
			dy = chip->dY[0];
			mod = chip->Modulus( false, 0 );
			fCheckResAvgM += mod;
			fCheckResRmsM += mod * mod;
			fCheckResAvgX += dx;
			fCheckResAvgMX += fabs( dx );
			fCheckResRmsX += dx * dx;
			fCheckResAvgY += dy;
			fCheckResAvgMY += fabs( dy );
			fCheckResRmsY += dy * dy;

			if( mod > fCheckResMaxM )
				fCheckResMaxM = mod;
			if( fabs( dx ) > fCheckResMaxMX )
				fCheckResMaxMX = fabs( dx );
			if( fabs( dy ) > fCheckResMaxMY )
				fCheckResMaxMY = fabs( dy );
		}

		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip ) {
			dx = chip->dX[1];
			dy = chip->dY[1];
			mod = chip->Modulus( false, 1 );
			fCheckResAvgM += mod;
			fCheckResRmsM += mod * mod;
			fCheckResAvgX += dx;
			fCheckResAvgMX += fabs( dx );
			fCheckResRmsX += dx * dx;
			fCheckResAvgY += dy;
			fCheckResAvgMY += fabs( dy );
			fCheckResRmsY += dy * dy;

			if( mod > fCheckResMaxM )
				fCheckResMaxM = mod;
			if( fabs( dx ) > fCheckResMaxMX )
				fCheckResMaxMX = fabs( dx );
			if( fabs( dy ) > fCheckResMaxMY )
				fCheckResMaxMY = fabs( dy );
			fCheckGCPNumber2++;
		}
	}

	if( fCheckGCPNumber > 1 )
		fCheckResStdDevM = sqrt( ( fCheckResRmsM - ( ( fCheckResAvgM * fCheckResAvgM ) / fCheckGCPNumber ) ) / ( fCheckGCPNumber - 1 ) );
	fCheckResAvgM = fCheckResAvgM / fCheckGCPNumber;
	fCheckResRmsM = sqrt( fCheckResRmsM / fCheckGCPNumber );

	if( fCheckGCPNumber > 1 )
		fCheckResStdDevX = sqrt( ( fCheckResRmsX - ( ( fCheckResAvgX * fCheckResAvgX ) / fCheckGCPNumber ) ) / ( fCheckGCPNumber - 1 ) );
	fCheckResAvgX = fCheckResAvgX / fCheckGCPNumber;
	if( fCheckGCPNumber > 1 )
		fCheckResStdDevMX = sqrt( ( fCheckResRmsX - ( ( fCheckResAvgMX * fCheckResAvgMX ) / fCheckGCPNumber ) ) / ( fCheckGCPNumber - 1 ) );
	fCheckResAvgMX = fCheckResAvgMX / fCheckGCPNumber;
	fCheckResRmsX = sqrt( fCheckResRmsX / fCheckGCPNumber );

	if( fCheckGCPNumber > 1 )
		fCheckResStdDevY = sqrt( ( fCheckResRmsY - ( ( fCheckResAvgY * fCheckResAvgY ) / fCheckGCPNumber ) ) / ( fCheckGCPNumber - 1 ) );
	fCheckResAvgY = fCheckResAvgY / fCheckGCPNumber;
	if( fCheckGCPNumber > 1 )
		fCheckResStdDevMY = sqrt( ( fCheckResRmsY - ( ( fCheckResAvgMY * fCheckResAvgMY ) / fCheckGCPNumber ) ) / ( fCheckGCPNumber - 1 ) );
	fCheckResAvgMY = fCheckResAvgMY / fCheckGCPNumber;
	fCheckResRmsY = sqrt( fCheckResRmsY / fCheckGCPNumber );
};


void attitudeEstimation( long N, TDChip ** gChips, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	max_epselon_roll = fDAVHRR.error_roll;
	max_epselon_pitch = fDAVHRR.error_pitch;
	max_epselon_yaw = fDAVHRR.error_yaw;
	max_mod_diff_yaw = .0;

	calcResDisp_forAutoGCPs( fDAVHRR, fACM, min_base_X, min_base_Y, max_rms_m );
	double diff_yaw = fDAVHRR.fRegrCOP->yaw, base_X, base_Y, rms_m;

	//TCLCursor cc = fACM.createCursor();
	//TCLCursor cursor = fACM.createCursor();
	// //TAufitChipMask::ChipCursor cc( fACM ), Cursor( fACM );
	// Выкидываем по одной точке и считаем оценки точности расчёта углов по регрессии.
	for( int i = 0; i < N; i++ ) {
		if( gChips[i]->fChipType[0] == TDChip::GraundChip ) {

			fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
			gChips[i]->fChipType[0] = TDChip::ControlChip;

			fDAVHRR.calculateCorrection_byAutoGCPLineRegres( fACM, true );
			calcResDisp_forAutoGCPs( fACM, fCorResAvgX, fCorResAvgY );
			if( fDAVHRR.fRegrCOP->yaw != fDAVHRR.beta1_y )
				flag_yaw = true;
			else
				flag_yaw = false;
			fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
			fDAVHRR.calculateCorrection_byAutoGCPLineRegres( fCS.fLinRegresCoeff_dX * fCorResAvgX, fCS.fLinRegresCoeff_dY * fCorResAvgY, fACM, flag_yaw );

			calcResDisp_forAutoGCPs( fDAVHRR, fACM, base_X, base_Y, rms_m );
			if( min_base_X > base_X )
				min_base_X = base_X;
			if( min_base_Y > base_Y )
				min_base_Y = base_Y;
			if( max_rms_m < rms_m )
				min_base_Y = rms_m;

			if( max_epselon_roll < fDAVHRR.error_roll )
				max_epselon_roll = fDAVHRR.error_roll;
			if( max_epselon_pitch < fDAVHRR.error_pitch )
				max_epselon_pitch = fDAVHRR.error_pitch;
			if( max_epselon_yaw < fDAVHRR.error_yaw )
				max_epselon_yaw = fDAVHRR.error_yaw;

			double difyaw = fabs( diff_yaw - fDAVHRR.fRegrCOP->yaw );
			if( max_mod_diff_yaw < difyaw )
				max_mod_diff_yaw = difyaw;

			gChips[i]->fChipType[0] = TDChip::GraundChip;
		}
		if( gChips[i]->fChannels_forCalcShifts[1] && gChips[i]->fChipType[1] == TDChip::GraundChip ) {

			fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
			gChips[i]->fChipType[1] = TDChip::ControlChip;

			fDAVHRR.calculateCorrection_byAutoGCPLineRegres( fACM, true );
			calcResDisp_forAutoGCPs( fACM, fCorResAvgX, fCorResAvgY );
			if( fDAVHRR.fRegrCOP->yaw != fDAVHRR.beta1_y )
				flag_yaw = true;
			else
				flag_yaw = false;
			fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
			fDAVHRR.calculateCorrection_byAutoGCPLineRegres( fCS.fLinRegresCoeff_dX * fCorResAvgX, fCS.fLinRegresCoeff_dY * fCorResAvgY, fACM, flag_yaw );

			calcResDisp_forAutoGCPs( fDAVHRR, fACM, base_X, base_Y, rms_m );
			if( min_base_X > base_X )
				min_base_X = base_X;
			if( min_base_Y > base_Y )
				min_base_Y = base_Y;
			if( max_rms_m < rms_m )
				min_base_Y = rms_m;

			if( max_epselon_roll < fDAVHRR.error_roll )
				max_epselon_roll = fDAVHRR.error_roll;
			if( max_epselon_pitch < fDAVHRR.error_pitch )
				max_epselon_pitch = fDAVHRR.error_pitch;
			if( max_epselon_yaw < fDAVHRR.error_yaw )
				max_epselon_yaw = fDAVHRR.error_yaw;

			double difyaw = fabs( diff_yaw - fDAVHRR.fRegrCOP->yaw );
			if( max_mod_diff_yaw < difyaw )
				max_mod_diff_yaw = difyaw;

			gChips[i]->fChipType[1] = TDChip::GraundChip;
		}

	}
};


void attitudeEstimation( long N, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	min_base_X = 10e+10;
	avg_base_X = .0;
	max_base_X = .0;
	std_dev_base_X = .0;
	min_base_Y = 10e+10;
	avg_base_Y = .0;
	max_base_Y = .0;
	std_dev_base_Y = .0;
	min_rms_m = 10e+10;
	avg_rms_m = .0;
	max_rms_m = .0;
	std_dev_rms_m = .0;
	min_epselon_roll = 10e+10;
	avg_epselon_roll = .0;
	max_epselon_roll = .0;
	std_dev_epselon_roll = .0;
	min_epselon_pitch = 10e+10;
	avg_epselon_pitch = .0;
	max_epselon_pitch = .0;
	std_dev_epselon_pitch = .0;
	min_epselon_yaw = 10e+10;
	avg_epselon_yaw = .0;
	max_epselon_yaw = .0;
	std_dev_epselon_yaw = .0;
	min_mod_diff_yaw = 10e+10;
	avg_mod_diff_yaw = .0;
	max_mod_diff_yaw = .0;
	std_dev_mod_diff_yaw = .0;

	double * base_X = new double[N+1];
	double * base_Y = new double[N+1];
	double * rms_m = new double[N+1];
	double * epselon_roll = new double[N+1];
	double * epselon_pitch = new double[N+1];
	double * epselon_yaw = new double[N+1];
	double * mod_diff_yaw = new double[N+1];

	calcResDisp_forAutoGCPs( fDAVHRR, fACM, base_X[0], base_Y[0], rms_m[0] );
	epselon_roll[0] = fDAVHRR.error_roll;
	epselon_pitch[0] = fDAVHRR.error_pitch;
	epselon_yaw[0] = fDAVHRR.error_yaw;
	mod_diff_yaw[0] = fDAVHRR.fRegrCOP->yaw;

	fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );

	int j, ch;
	//TAufitChipMask::ChipCursor cc( fACM ), Cursor( fACM );
	TCLCursor cc = fACM.createCursor();
	TCLCursor cursor = fACM.createCursor();
	// Выкидываем по одной точке и считаем оценки точности расчёта углов по регрессии.
	for( int i = 1; i <= N; i++ ) {
		j = 1;
		ch = 0;
		for( cc.setToFirst(); cc.isValid(); cc.setToNext() ) {
			TDChip * pchip = cc.elementAt();
			if( pchip->fChipType[0] == TDChip::GraundChip ) {
				if( j++ == i ) {
					pchip->fChipType[0] = TDChip::ControlChip;
					cursor = cc;
					ch = 0;
					break;
				}
			}
			if( pchip->fChannels_forCalcShifts[1] && pchip->fChipType[1] == TDChip::GraundChip ) {
				if( j++ == i ) {
					pchip->fChipType[1] = TDChip::ControlChip;
					cursor = cc;
					ch = 1;
					break;
				}
			}
		}

		fDAVHRR.calculateCorrection_byAutoGCPLineRegres( fACM, true );
		calcResDisp_forAutoGCPs( fACM, fCorResAvgX, fCorResAvgY );
		if( fDAVHRR.fRegrCOP->yaw != fDAVHRR.beta1_y )
			flag_yaw = true;
		else
			flag_yaw = false;
		fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
		fDAVHRR.calculateCorrection_byAutoGCPLineRegres( fCS.fLinRegresCoeff_dX * fCorResAvgX, fCS.fLinRegresCoeff_dY * fCorResAvgY, fACM, flag_yaw );

		calcResDisp_forAutoGCPs( fDAVHRR, fACM, base_X[i], base_Y[i], rms_m[i] );

		epselon_roll[i] = fDAVHRR.error_roll;
		epselon_pitch[i] = fDAVHRR.error_pitch;
		epselon_yaw[i] = fDAVHRR.error_yaw;
		mod_diff_yaw[i] = fabs( mod_diff_yaw[0] - fDAVHRR.fRegrCOP->yaw );

		fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
		//fACM.chipAt( Cursor )->fChipType[ch] = TDChip::GraundChip;
		cursor.elementAt()->fChipType[ch] = TDChip::GraundChip;
	}

	for( int i = 1; i <= N; i++ ) {
		if( min_base_X >= base_X[i] )
			min_base_X = base_X[i];
		if( max_base_X <= base_X[i] )
			max_base_X = base_X[i];
		avg_base_X += base_X[i];
		std_dev_base_X += base_X[i] * base_X[i];

		if( min_base_Y >= base_Y[i] )
			min_base_Y = base_Y[i];
		if( max_base_Y <= base_Y[i] )
			max_base_Y = base_Y[i];
		avg_base_Y += base_Y[i];
		std_dev_base_Y += base_Y[i] * base_Y[i];

		if( min_rms_m >= rms_m[i] )
			min_rms_m = rms_m[i];
		if( max_rms_m <= rms_m[i] )
			max_rms_m = rms_m[i];
		avg_rms_m += rms_m[i];
		std_dev_rms_m += rms_m[i] * rms_m[i];

		if( min_epselon_roll >= epselon_roll[i] )
			min_epselon_roll = epselon_roll[i];
		if( max_epselon_roll <= epselon_roll[i] )
			max_epselon_roll = epselon_roll[i];
		avg_epselon_roll += epselon_roll[i];
		std_dev_epselon_roll += epselon_roll[i] * epselon_roll[i];

		if( min_epselon_pitch >= epselon_pitch[i] )
			min_epselon_pitch = epselon_pitch[i];
		if( max_epselon_pitch <= epselon_pitch[i] )
			max_epselon_pitch = epselon_pitch[i];
		avg_epselon_pitch += epselon_pitch[i];
		std_dev_epselon_pitch += epselon_pitch[i] * epselon_pitch[i];

		if( min_epselon_yaw >= epselon_yaw[i] )
			min_epselon_yaw = epselon_yaw[i];
		if( max_epselon_yaw <= epselon_yaw[i] )
			max_epselon_yaw = epselon_yaw[i];
		avg_epselon_yaw += epselon_yaw[i];
		std_dev_epselon_yaw += epselon_yaw[i] * epselon_yaw[i];

		if( min_mod_diff_yaw >= mod_diff_yaw[i] )
			min_mod_diff_yaw = mod_diff_yaw[i];
		if( max_mod_diff_yaw <= mod_diff_yaw[i] )
			max_mod_diff_yaw = mod_diff_yaw[i];
		avg_mod_diff_yaw += mod_diff_yaw[i];
		std_dev_mod_diff_yaw += mod_diff_yaw[i] * mod_diff_yaw[i];
	}
	std_dev_base_X = sqrt( ( std_dev_base_X - ( ( avg_base_X * avg_base_X ) / double(N) ) ) / double( N - 1 ) );
	avg_base_X = avg_base_X / double(N);
	std_dev_base_Y = sqrt( ( std_dev_base_Y - ( ( avg_base_Y * avg_base_Y ) / double(N) ) ) / double( N - 1 ) );
	avg_base_Y = avg_base_Y / double(N);
	std_dev_rms_m = sqrt( ( std_dev_rms_m - ( ( avg_rms_m * avg_rms_m ) / double(N) ) ) / double( N - 1 ) );
	avg_rms_m = avg_rms_m / double(N);

	std_dev_epselon_roll = sqrt( ( std_dev_epselon_roll - ( ( avg_epselon_roll * avg_epselon_roll ) / double(N) ) ) / double( N - 1 ) );
	avg_epselon_roll = avg_epselon_roll / double(N);
	std_dev_epselon_pitch = sqrt( ( std_dev_epselon_pitch - ( ( avg_epselon_pitch * avg_epselon_pitch ) / double(N) ) ) / double( N - 1 ) );
	avg_epselon_pitch = avg_epselon_pitch / double(N);
	std_dev_epselon_yaw = sqrt( ( std_dev_epselon_yaw - ( ( avg_epselon_yaw * avg_epselon_yaw ) / double(N) ) ) / double( N - 1 ) );
	avg_epselon_yaw = avg_epselon_yaw / double(N);
	std_dev_mod_diff_yaw = sqrt( ( std_dev_mod_diff_yaw - ( ( avg_mod_diff_yaw * avg_mod_diff_yaw ) / double(N) ) ) / double( N - 1 ) );
	avg_mod_diff_yaw = avg_mod_diff_yaw / double(N);

	delete [] base_X;
	delete [] base_Y;
	delete [] rms_m;
	delete [] epselon_roll;
	delete [] epselon_pitch;
	delete [] epselon_yaw;
	delete [] mod_diff_yaw;
};



void calcStatParams_for2SigmaCriterion( TAufitChipMask & fACM ) {
	//TAufitChipMask::ChipCursor cc( fACM );
	TCLCursor cursor = fACM.createCursor();

	fGCPNumber = fACM.numberOfGraundChips();   // Число GCPs.
	if( !fGCPNumber )
		return;

	fResMaxM = .0;
	fResAvgMX = .0;
	fResStdDevMX = .0;
	fResAvgMY = .0;
	fResStdDevMY = .0;
	double m_data;
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			fResAvgMX += fabs( chip->dX[0] );
			fResStdDevMX += chip->dX[0] * chip->dX[0];
			fResAvgMY += fabs( chip->dY[0] );
			fResStdDevMY += chip->dY[0] * chip->dY[0];

			m_data = chip->Modulus( false, 0 );
			if( m_data > fResMaxM )
				fResMaxM = m_data;
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			fResAvgMX += fabs( chip->dX[1] );
			fResStdDevMX += chip->dX[1] * chip->dX[1];
			fResAvgMY += fabs( chip->dY[1] );
			fResStdDevMY += chip->dY[1] * chip->dY[1];

			m_data = chip->Modulus( false, 1 );
			if( m_data > fResMaxM )
				fResMaxM = m_data;
		}
	}

	fResStdDevMX = sqrt( ( fResStdDevMX - ( ( fResAvgMX * fResAvgMX ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
	fResAvgMX = fResAvgMX / fGCPNumber;
	fResStdDevMY = sqrt( ( fResStdDevMY - ( ( fResAvgMY * fResAvgMY ) / fGCPNumber ) ) / ( fGCPNumber - 1 ) );
	fResAvgMY = fResAvgMY / fGCPNumber;
};


bool selection_GraundChips_for_autoGCPs_afterCalcCorrectionMinimiz( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	//char str_buf[100];
	//TAufitChipMask::ChipCursor cursor( fACM );
	TCLCursor cursor = fACM.createCursor();
	unsigned long n = 0, quantity_badGCPs = 0, N = fACM.numberOfGraundChips();
	TCorrectionParams cop( 2 );

	print_LogFile( "\nОтбраковка GCPs критерием 2-сигма по одной реперной точке с наихудшим значением стат-й знач-ти.\n" );
	print_LogFile( "\nЗначения статических параметров по всем autoGCPs до отбраковки по критерию 2-сигма:" );

	calcStatParams_forAutoGCPs( false, true, fACM );
	fDAVHRR.setCorrectionParams( false, cop, fACM, true );
	fCS.fMinParams[4] = 1;
	if( N == 1 )
		fCS.fMinParams[4] = 0;
	if( N == 2 && fACM.duplicateOfGraundChips() )
		fCS.fMinParams[4] = 0;

	fDAVHRR.calculateCorrection_byAutoGCPMinimiz( fCS.fMinMethod, fCS.fMinParams, fACM );
	calcStatParams_forAutoGCPs( false, false, fACM );
	uint8_t bed_ch;
	TDChip * bed_chip;
	do {
		n++;
		quantity_badGCPs = 0;
		double bed_stat_signf = 1000.;
		if( fResMaxM >= 1. && N > 2 ) {
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::GraundChip ) {
					if( fabs(fabs(chip->dX[0]) - fResAvgMX) >= (2. * fResStdDevMX) ||
							fabs(fabs(chip->dY[0]) - fResAvgMY) >= (2. * fResStdDevMY) ) {
						quantity_badGCPs++;
						if( chip->fNormStatSign[0] <= bed_stat_signf ) {
							bed_chip = chip;
							bed_stat_signf = bed_chip->fNormStatSign[0];
							bed_ch = 0;
						}
					}
				}
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
					if( fabs(fabs(chip->dX[1]) - fResAvgMX) >= (2. * fResStdDevMX) ||
							fabs(fabs(chip->dY[1]) - fResAvgMY) >= (2. * fResStdDevMY) ) {
						quantity_badGCPs++;
						if( chip->fNormStatSign[1] <= bed_stat_signf ) {
							bed_chip = chip;
							bed_stat_signf = bed_chip->fNormStatSign[1];
							bed_ch = 1;
						}
					}
				}
			}
		}
		if( quantity_badGCPs > 0 ) {
			fDAVHRR.setCorrectionParams( false, cop, fACM, true );
			bed_chip->fChipType[bed_ch] = TDChip::ControlChip;
			N--;
			fDAVHRR.calculateCorrection_byAutoGCPMinimiz( fCS.fMinMethod, fCS.fMinParams, fACM );
			calcStatParams_for2SigmaCriterion( fACM );
		}
	} while( (fResMaxM >= 1.) && (N > 2) && (quantity_badGCPs > 0) );

	print_LogFile( "\nЗначения статических параметров по всем autoGCPs после отбраковки по критерию 2-сигма:" );
	calcStatParams_forAutoGCPs( false, true, fACM );
	calcStatParams_forAutoGCPs( false, false, fACM );
	fDAVHRR.setCorrectionParams( true, cop, fACM, true );

	N = fACM.numberOfControlChips();
	if( N > 0 ) {
		FILE * fLog = fopen( pathLogFile, "a" );
		fprintf( fLog, "\nотбракованные GCPs по критерию 2-сигма (%lu):\n", N );
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::ControlChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
		}
		fclose( fLog );
	}

	N = fACM.numberOfGraundChips();
	if( N > 0 ) {
		FILE * fLog = fopen( pathLogFile, "a" );
		fprintf( fLog, "\nautoGCPs, неотбракованные критерием 2-сигма (%lu):\n", N );
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
		}
		fclose( fLog );
	}
	if (fResMaxM <= 1. )
		return true;
	else
		return false;
};


bool selection_GraundChips_for_autoGCPs_afterCalcCorrectionLineRegres( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	//char str_buf[100];
	//TAufitChipMask::ChipCursor cursor( fACM );
	TCLCursor cursor = fACM.createCursor();
	unsigned long n = 0, quantity_badGCPs = 0, N = fACM.numberOfGraundChips();
	TCorrectionParams cop( 2 );

	print_LogFile( "\nОтбраковка GCPs критерием 2-сигма по одной реперной точке с наихудшим значением стат-й знач-ти.\n" );
	//    print_LogFile( "\nЗначения статических параметров по всем autoGCPs до отбраковки по критерию 2-сигма:" );

	//    calcStatParams_forAutoGCPs( true, true, fACM );
	fDAVHRR.setCorrectionParams( true, cop, fACM, true );
	fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fACM );
	calcStatParams_for2SigmaCriterion( fACM );
	//    calcStatParams_forAutoGCPs( true, false, fACM );

	uint8_t bed_ch;
	TDChip * bed_chip;
	do {
		n++;
		quantity_badGCPs = 0;
		double bed_stat_signf = 1000000.;
		if( fResMaxM >= 1. && N > 4 ) {
			for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
				TDChip * chip = cursor.elementAt();
				if( chip->fChipType[0] == TDChip::GraundChip ) {
					if( fabs(fabs(chip->dX[0]) - fResAvgMX) >= (2. * fResStdDevMX) ||
							fabs(fabs(chip->dY[0]) - fResAvgMY) >= (2. * fResStdDevMY) ) {
						quantity_badGCPs++;
						if( chip->fNormStatSign[0] <= bed_stat_signf ) {
							bed_chip = chip;
							bed_stat_signf = bed_chip->fNormStatSign[0];
							bed_ch = 0;
						}
					}
				}
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
					if( fabs(fabs(chip->dX[1]) - fResAvgMX) >= (2. * fResStdDevMX) ||
							fabs(fabs(chip->dY[1]) - fResAvgMY) >= (2. * fResStdDevMY) ) {
						quantity_badGCPs++;
						if( chip->fNormStatSign[1] <= bed_stat_signf ) {
							bed_chip = chip;
							bed_stat_signf = bed_chip->fNormStatSign[1];
							bed_ch = 1;
						}
					}
				}
			}
		}
		if( quantity_badGCPs > 0 ) {
			fDAVHRR.setCorrectionParams( true, cop, fACM, true );
			bed_chip->fChipType[bed_ch] = TDChip::ControlChip;
			N--;
			fDAVHRR.calculateCorrection_byAutoGCP_onLineRegres( fACM );
			calcStatParams_for2SigmaCriterion( fACM );
		}
	} while( (fResMaxM >= 1.) && (N > 4) && (quantity_badGCPs > 0) );

	//    print_LogFile( "\nЗначения статических параметров по всем autoGCPs после отбраковки по критерию 2-сигма:" );
	//    calcStatParams_forAutoGCPs( true, true, fACM );
	//    calcStatParams_forAutoGCPs( true, false, fACM );
	//    fCorResAvgX = fResAvgX; fCorResAvgY = fResAvgY;

	fDAVHRR.setCorrectionParams( true, cop, fACM, true );

	N = fACM.numberOfControlChips();
	if( N > 0 ) {
		FILE * fLog = fopen( pathLogFile, "a" );
		fprintf( fLog, "\nотбракованные GCPs по критерию 2-сигма (%lu):\n", N );
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::ControlChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
		}
		fclose( fLog );
	}

	N = fACM.numberOfGraundChips();
	if( N > 0 ) {
		FILE * fLog = fopen( pathLogFile, "a" );
		fprintf( fLog, "\nautoGCPs, неотбракованные критерием 2-сигма (%lu):\n", N );
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 4\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->PercCloudy );
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
				fprintf( fLog, "%ld\t %ld\t %.5f\t %.5f\t 2\t %.7f\t %.3f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->PercCloudy );
		}
		fclose( fLog );
	}
	if (fResMaxM <= 1. )
		return true;
	else
		return false;
};


void selection_GraundChips_for_autoGCPs_forCalcCorrectionLineRegres_onCombination( TDataAVHRR & fDAVHRR, int n, TDChip * chips, TAufitChipMask & fACM, int & num_gcp, double rms ) {
	int ch;
	//double beta0_x, beta0_y, beta1_y; //не используются
	for( int i = 0; i < n; i++ ) {
		if( chips[i].fChipType[0] == TDChip::ControlChip || ( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::ControlChip ) ) {
			if( chips[i].fChipType[0] == TDChip::ControlChip ) {
				ch = 0;
				chips[i].fChipType[0] = TDChip::GraundChip;
				num_gcp++;
			} else {
				if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::ControlChip ) {
					ch = 1;
					chips[i].fChipType[1] = TDChip::GraundChip;
					num_gcp++;
				}
			}

			fDAVHRR.setCorrectionParams( true, TCorrectionParams( 2 ), n, chips, fACM, true );
			fDAVHRR.calculateCorrection_byAutoGCPLineRegres( n, chips, fACM, false );
			calcResDisp_forAutoGCPs( n, chips );
			//             calcLinearRegressionCoefficients( n, chips, beta0_x, beta0_y, beta1_y );
			//             calcResDisp_forAutoGCPs( n, chips, beta0_x, beta0_y, beta1_y );

			if( fResRmsM <= rms ) {
				int quantity_badGCPs = 0;
				for( int j = 0; j < n; j++ ) {
					if( chips[j].fChipType[0] == TDChip::GraundChip ) {
						if( fabs(fabs(chips[j].dX[0]) - fResAvgMX) >= (2. * fResStdDevMX) ||
								fabs(fabs(chips[j].dY[0]) - fResAvgMY) >= (2. * fResStdDevMY) ) {
							quantity_badGCPs++;
						}
					} else {
						if( chips[j].fChannels_forCalcShifts[1] && chips[j].fChipType[1] == TDChip::GraundChip ) {
							if( fabs(fabs(chips[j].dX[1]) - fResAvgMX) >= (2. * fResStdDevMX) ||
									fabs(fabs(chips[j].dY[1]) - fResAvgMY) >= (2. * fResStdDevMY) ) {
								quantity_badGCPs++;
							}
						}
					}
				}
				if( quantity_badGCPs > 0 ) {
					chips[i].fChipType[ch] = TDChip::ControlChip;
					num_gcp--;
				}
			} else {
				chips[i].fChipType[ch] = TDChip::ControlChip;
				num_gcp--;
			}
		}
	}
}


void configuration_for_autoGCPs( bool flag, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) {
	minConfiguration_ofGroundX = 2048L, maxConfiguration_ofGroundX = 0L;
	minConfiguration_ofGroundY = 10000L, maxConfiguration_ofGroundY = 0L;

	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			if( minConfiguration_ofGroundX >= long(chip->X1+chip->ChipSize().x()/2) )
				minConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( minConfiguration_ofGroundY >= long(chip->Y1+chip->ChipSize().y()/2) )
				minConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
			if( maxConfiguration_ofGroundX <= long(chip->X1+chip->ChipSize().x()/2) )
				maxConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( maxConfiguration_ofGroundY <= long(chip->Y1+chip->ChipSize().y()/2) )
				maxConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			if( minConfiguration_ofGroundX >= long(chip->X1+chip->ChipSize().x()/2) )
				minConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( minConfiguration_ofGroundY >= long(chip->Y1+chip->ChipSize().y()/2) )
				minConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
			if( maxConfiguration_ofGroundX <= long(chip->X1+chip->ChipSize().x()/2) )
				maxConfiguration_ofGroundX = long(chip->X1+chip->ChipSize().x()/2);
			if( maxConfiguration_ofGroundY <= long(chip->Y1+chip->ChipSize().y()/2) )
				maxConfiguration_ofGroundY = long(chip->Y1+chip->ChipSize().y()/2);
		}
	}

	FILE * fLog = fopen( pathLogFile, "a" );
	fprintf( fLog, "\n\n********************************************************************************************************************************************\n" );
	fprintf( fLog, "\nконфигурация расположения autoGCPs на изображении:\n( %lu, %lu ) X ( %lu, %lu ) <=> | %lu, %lu |\n",
			 minConfiguration_ofGroundX, minConfiguration_ofGroundY, maxConfiguration_ofGroundX, maxConfiguration_ofGroundY,
			 maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1, maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1 );
	fprintf( fLog, "размеры изображения:\t| 2048, %u |\n", fDAVHRR.fNIP->fScans );
	fprintf( fLog, "база конфигурации autoGCPs:\t{ %.5f, %.5f }\n",
			 double(maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1) / 2048.,
			 double(maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1) / double(fDAVHRR.fNIP->fScans) );
	fprintf( fLog, "\n********************************************************************************************************************************************\n" );
	fclose( fLog );

	if( !flag ) {
		MminConfiguration_ofGroundX = minConfiguration_ofGroundX;
		MmaxConfiguration_ofGroundX = maxConfiguration_ofGroundX;
		MminConfiguration_ofGroundY = minConfiguration_ofGroundY;
		MmaxConfiguration_ofGroundY = maxConfiguration_ofGroundY;
	}
};



void openGraundChips( bool flag_ar, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, int & n ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла с опорными чипами для чтения!!!" );

	print_log( "Чтение файла с заданными опорными чипами." );

	FILE * f;
	if( !(f = fopen( inputFileGCP, "r" )) ) {
		throw ae1;
	}

	char buf[400];
	uint32_t gcpnum = 0;
	uint8_t * ch;
	long * x, * y;
	double * dx, * dy, * critic, * cloudy; //, * dhx, * dhy; //unused

	fgets( buf, 100, f );
	fgets( buf, 100, f );
	fgets( buf, 100, f );
	fgets( buf, 100, f );
	fgets( buf, 100, f );
	fgets( buf, 100, f );
	fgets( buf, 100, f );
	//    fgets( buf, 100, f );
	//    fgets( buf, 100, f );
	//    fgets( buf, 100, f );
	fgets( buf, 100, f );
	sscanf( buf, "%u", &gcpnum );
	//    fgets( buf, 100, f );
	//    fgets( buf, 100, f );
	/*
	    if( gcpnum < 4 ){
	        for( int j = 0; j < gcpnum; j++ )
	            fgets( buf, 400, f );
	        gcpnum = 0;
	        fgets( buf, 100, f );
	        fgets( buf, 400, f );
	        fgets( buf, 100, f );
	        fgets( buf, 100, f );
	        fgets( buf, 100, f );
	        sscanf( buf, "%u", &gcpnum );
	        fgets( buf, 100, f );
	        fgets( buf, 100, f );
	    }
	*/
	ch = new uint8_t[gcpnum];
	x = new long[gcpnum];
	y = new long[gcpnum];
	dx = new double[gcpnum];
	dy = new double[gcpnum];
	critic = new double[gcpnum];
	cloudy = new double[gcpnum];
	//    dhx = new double[gcpnum];
	//    dhy = new double[gcpnum];
	float cl;
	for( uint32_t j = 0; j < gcpnum; j++ ) {
		fgets( buf, 400, f );
		//        sscanf( buf, "%d\t %d\t %lf\t %lf\t %d\t %lf\t %lf\t %lf\t %lf\n", &x[j], &y[j], &dx[j], &dy[j], &ch[j], &critic[j], &cloudy[j], &dhx[j], &dhy[j] );
		sscanf( buf, "%ld\t %ld\t %lf\t %lf\t %hhu\t %lf\t %f\n", &x[j], &y[j], &dx[j], &dy[j], &ch[j], &critic[j], &cl );
		cloudy[j] = double( cl );
	}
	fclose( f );

	n = gcpnum;
	if( flag_ar ) {
		p_chips = new TDChip[n];
	}

	char fn[20];
	splitpath( inputFileGCP, 0, 0, fn, 0 );

	double lon_c, lat_c;
	TAstronom fAstronom;
	TCLCursor cursor = fACM.createCursor();
	for( uint32_t i = 0; i < gcpnum; i++ ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( ( abs(x[i] - chip->X1 - chip->ChipSize().x()/2) < 3 ) && ( abs(y[i] - chip->Y1 - chip->ChipSize().y()/2) < 3 ) ) {
				chip->PercCloudy = cloudy[i];
				//                   FILE * fcl = fopen( fn, "a" );
				//                   fprintf( fcl, "%.5f\n", chip->fAngleCenterSanny );
				//                   fclose( fcl );
				fDAVHRR.fSRef->xy2ll( double(chip->X1 + chip->ChipSize().x()/2), double(chip->Y1 + chip->ChipSize().y()/2), &lon_c, &lat_c );
				lon_c = -lon_c;
				chip->fAngleCenterSanny = fAstronom.shcrds( fAstronom.julian( fDAVHRR.fNIP->fYear, fDAVHRR.fNIP->fYearTime + 1 ), lon_c, lat_c );
				if( ch[i] == 2 ) {
					chip->fChannels_forCalcShifts[1] = true;
					//                       chip->fChipType[1] = TDChip::ControlChip;
					chip->fChipType[1] = TDChip::GraundChip;
					chip->autoDisplacements( 1, dx[i], dy[i] );
					chip->fNormStatSign[1] = critic[i];
					//                       chip->handDisplacements( 1, dhx[i], dhy[i] );
					if( flag_ar )
						p_chips[i].inicil( 1, chip );
				}
				if( ch[i] == 4 ) {
					chip->fChannels_forCalcShifts[0] = true;
					//                       chip->fChipType[0] = TDChip::ControlChip;
					chip->fChipType[0] = TDChip::GraundChip;
					chip->autoDisplacements( 0, dx[i], dy[i] );
					chip->fNormStatSign[0] = critic[i];
					//                       chip->handDisplacements( 0, dhx[i], dhy[i] );
					if( flag_ar )
						p_chips[i].inicil( 0, chip );
				}
				break;
			}
		}
	}

	delete [] ch;
	delete [] x;
	delete [] y;
	delete [] dx;
	delete [] dy;
	delete [] critic;
	delete [] cloudy;
	//    delete [] dhx;
	//    delete [] dhy;
};


void openSourceGraundChips( bool flag_ar, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, int & n ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла с опорными чипами для чтения!!!" );

	print_log( "Чтение файла с заданными опорными чипами." );

	FILE * f;
	if( !(f = fopen( inputFileGCP, "r" )) ) {
		throw ae1;
	}

	char buf[400];
	uint32_t gcpnum = 0;
	uint8_t * ch;
	long * x, * y;
	double * dx, * dy, * critic, * cloudy; //, * dhx, * dhy; - unused

	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	fgets( buf, 400, f );
	sscanf( buf, "%u", &gcpnum );
	fgets( buf, 100, f );
	fgets( buf, 100, f );

	if( gcpnum < 4 ) {
		for( uint32_t j = 0; j < gcpnum; j++ )
			fgets( buf, 400, f );
		gcpnum = 0;
		fgets( buf, 400, f );
		fgets( buf, 400, f );
		fgets( buf, 400, f );
		fgets( buf, 400, f );
		fgets( buf, 400, f );
		sscanf( buf, "%u", &gcpnum );
		fgets( buf, 100, f );
		fgets( buf, 100, f );
	}

	ch = new uint8_t[gcpnum];
	x = new long[gcpnum];
	y = new long[gcpnum];
	dx = new double[gcpnum];
	dy = new double[gcpnum];
	critic = new double[gcpnum];
	cloudy = new double[gcpnum];

	float cl;
	for( uint32_t j = 0; j < gcpnum; j++ ) {
		fgets( buf, 400, f );
		sscanf( buf, "%ld\t %ld\t %lf\t %lf\t %hhu\t %lf\t %f\n", &x[j], &y[j], &dx[j], &dy[j], &ch[j], &critic[j], &cl );
		cloudy[j] = double( cl );
	}
	fclose( f );

	n = gcpnum;
	if( flag_ar ) {
		p_chips = new TDChip[n];
	}

	double lon_c, lat_c;
	TAstronom fAstronom;
	TCLCursor cursor = fACM.createCursor();
	for( uint32_t i = 0; i < gcpnum; i++ ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( ( abs(x[i] - chip->X1 - chip->ChipSize().x()/2) < 3 ) && ( abs(y[i] - chip->Y1 - chip->ChipSize().y()/2) < 3 ) ) {
				chip->PercCloudy = cloudy[i];
				fDAVHRR.fSRef->xy2ll( double(chip->X1 + chip->ChipSize().x()/2), double(chip->Y1 + chip->ChipSize().y()/2), &lon_c, &lat_c );
				lon_c = -lon_c;
				chip->fAngleCenterSanny = fAstronom.shcrds( fAstronom.julian( fDAVHRR.fNIP->fYear, fDAVHRR.fNIP->fYearTime + 1 ), lon_c, lat_c );
				if( ch[i] == 2 ) {
					chip->fChannels_forCalcShifts[1] = true;
					chip->fChipType[1] = TDChip::ControlChip;
					chip->autoDisplacements( 1, dx[i], dy[i] );
					chip->fNormStatSign[1] = critic[i];
					if( flag_ar )
						p_chips[i].inicil( 1, chip );
				}
				if( ch[i] == 4 ) {
					chip->fChannels_forCalcShifts[0] = true;
					chip->fChipType[0] = TDChip::ControlChip;
					chip->autoDisplacements( 0, dx[i], dy[i] );
					chip->fNormStatSign[0] = critic[i];
					if( flag_ar )
						p_chips[i].inicil( 0, chip );
				}
				break;
			}
		}
	}

	delete [] ch;
	delete [] x;
	delete [] y;
	delete [] dx;
	delete [] dy;
	delete [] critic;
	delete [] cloudy;
};


void save_autoGCP( bool flag_mr, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи autoGCPs!!!" );

	unsigned long n = fACM.numberOfGraundChips();   // Число autoGCP.
	if( n <= 0 )
		return; // Нельзя записывать пустой набор autoGCP.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], gcpfilepath[MAX_PATH];
	#warning ацкое зло, но думать ломы
	splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
	//strlwr( fname );

	//int l = strlen( fname ); - unused

	// Конструирование полного имени файла GCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fTargetDataDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fTargetDataDir.c_str())  , drive, dir, 0, 0 );
	else {
		getcwd( gcpfilepath, MAX_PATH );
		int t = strlen(gcpfilepath);
		if( gcpfilepath[t-1] != DIRD ) {
			gcpfilepath[t] = DIRD;
			gcpfilepath[t+1] = '\0';
		}
		splitpath( gcpfilepath, drive, dir, 0, 0 );
	}
	makepath( gcpfilepath, drive, dir, fname, "gcp" );

	FILE * f;
	if( !(f = fopen( gcpfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
	fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
	fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
	fprintf( f, "%d\t ; номер витка NORAD\n", fDAVHRR.fISP->fRevNum );
	fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
	fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
	fprintf( f, "%ld\t ; число autoGCP\n", n );

	// Запись autoGCP.
	TCLCursor cursor = fACM.createCursor();

	//TDChip chip;
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip )
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[0], chip->dY[0], 4, chip->fNormStatSign[0], chip->PercCloudy );
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[1], chip->dY[1], 2, chip->fNormStatSign[1], chip->PercCloudy );
	}

	fclose( f );
};


void save_autoGCP( uint8_t passage, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи autoGCPs!!!" );

	unsigned long n = fACM.numberOfGraundChips();   // Число autoGCP.
	if( n <= 0 )
		return; // Нельзя записывать пустой набор autoGCP.

	print_log( "Запись файла с реперными точками." );

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], gcpfilepath[MAX_PATH];

	#warning ацкое зло, но думать ломы
	splitpath(const_cast<char *>(inputFileName.c_str()) , 0, 0, fname, 0 );
	//strlwr( fname );

	if( passage == 2 ) {
		int l = strlen( fname );
		char buf[5], *p = fname + l;
		sprintf( buf, "_ac" );
		strcpy( p, buf );
	}

	// Конструирование полного имени файла GCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fStatDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fStatDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( gcpfilepath, MAX_PATH );
		int t = strlen(gcpfilepath);
		if( gcpfilepath[t-1] != DIRD ) {
			gcpfilepath[t] = DIRD;
			gcpfilepath[t+1] = '\0';
		}
		splitpath( gcpfilepath, drive, dir, 0, 0 );
	}
	//    l = strlen( dir );
	//    p = dir + l;
	//    strcpy( p, output_autoGCPDir );
	makepath( gcpfilepath, drive, dir, fname, "gcp" );

	FILE * f;
	if( !(f = fopen( gcpfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
	fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
	fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
	fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
	fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
	fprintf( f, "%ld\t ; число autoGCP\n", n );

	// Запись autoGCP.
	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip )
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[0], chip->dY[0], 4, chip->fNormStatSign[0], chip->PercCloudy );
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[1], chip->dY[1], 2, chip->fNormStatSign[1], chip->PercCloudy );
	}

	fclose( f );
};


void saveGCP( int pref1, long pref2, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи autoGCPs!!!" );

	unsigned long n = fACM.numberOfGraundChips() + fACM.numberOfControlChips();   // Число GCP.
	if( n <= 0 )
		return; // Нельзя записывать пустой набор GCP.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], gcpfilepath[MAX_PATH];
	#warning ацкое зло, но думать ломы
	splitpath(const_cast<char *>(inputFileName.c_str()), drive, dir, fname, 0 );
	//strlwr( fname );

	int l = strlen( fname );
	char buf[5], *p = fname + l;
	sprintf( buf, "_%d_%ld", pref1, pref2 );
	strcpy( p, buf );

	// Конструирование полного имени файла GCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fGPCXDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fGPCXDir.c_str()), drive, dir, 0, 0 );
	else {
		getcwd( gcpfilepath, MAX_PATH );
		int t = strlen(gcpfilepath);
		if( gcpfilepath[t-1] != DIRD ) {
			gcpfilepath[t] = DIRD;
			gcpfilepath[t+1] = '\0';
		}
		splitpath( gcpfilepath, drive, dir, 0, 0 );
	}
	l = strlen( dir );
	p = dir + l;
	strcpy( p, output_autoGCPDir );
	makepath( gcpfilepath, drive, dir, fname, "gcp" );

	FILE * f;
	if( !(f = fopen( gcpfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
	fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
	fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
	fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
	fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
	fprintf( f, "%ld\t ; число autoGCP\n", n );

	// Запись autoGCP.
	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip )
			fprintf( f, "1\t %ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[0], chip->dY[0], 4, chip->fNormStatSign[0], chip->PercCloudy );
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
			fprintf( f, "1\t %ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[1], chip->dY[1], 2, chip->fNormStatSign[1], chip->PercCloudy );

		if( chip->fChipType[0] == TDChip::ControlChip )
			fprintf( f, "0\t %ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[0], chip->dY[0], 4, chip->fNormStatSign[0], chip->PercCloudy );
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
			fprintf( f, "0\t %ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[1], chip->dY[1], 2, chip->fNormStatSign[1], chip->PercCloudy );

	}

	fclose( f );
};


void save_autoGCP( bool flag_mr, TDataAVHRR & fDAVHRR, int n, TDChip * chips ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи autoGCPs!!!" );

	if( n <= 0 )
		return; // Нельзя записывать пустой набор autoGCP.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], gcpfilepath[MAX_PATH];
	#warning ацкое зло, но думать ломы
	splitpath(const_cast<char *>(inputFileName.c_str()), drive, dir, fname, 0 );
	//strlwr( fname );
	/*
	    int l = strlen( fname );
	    char buf[5], *p = fname + l;
	    if( !flag_mr ) sprintf( buf, "_lr" );
	    else sprintf( buf, "_min" );
	    strcpy( p, buf );
	*/
	// Конструирование полного имени файла GCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fGPCXDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fGPCXDir.c_str()), drive, dir, 0, 0 );
	else {
		getcwd( gcpfilepath, MAX_PATH );
		int t = strlen(gcpfilepath);
		if( gcpfilepath[t-1] != DIRD ) {
			gcpfilepath[t] = DIRD;
			gcpfilepath[t+1] = '\0';
		}
		splitpath( gcpfilepath, drive, dir, 0, 0 );
	}
	//    int l = strlen( dir );
	//    char * p = dir + l;
	//    strcpy( p, output_autoGCPDir );
	makepath( gcpfilepath, drive, dir, fname, "gcp" );

	FILE * f;
	if( !(f = fopen( gcpfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
	fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
	fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
	fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
	fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
	fprintf( f, "%d\t ; число autoGCP\n", n );

	// Запись autoGCP.
	for( int i = 0; i < n; i++ ) {
		if( chips[i].fChipType[0] == TDChip::GraundChip ) {
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chips[i].X1+chips[i].ChipSize().x()/2, chips[i].Y1+chips[i].ChipSize().y()/2,
					 chips[i].dX[0], chips[i].dY[0], 4, chips[i].fNormStatSign[0], chips[i].PercCloudy );
		} else {
			if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip )
				fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
						 chips[i].X1+chips[i].ChipSize().x()/2,chips[i].Y1+chips[i].ChipSize().y()/2,
						 chips[i].dX[1], chips[i].dY[1], 2, chips[i].fNormStatSign[1], chips[i].PercCloudy );
		}
	}

	fclose( f );
};



void save_autoGCP_c( int n_comb, TDataAVHRR & fDAVHRR, int n, TDChip * chips ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи autoGCPs!!!" );

	if( n <= 0 )
		return; // Нельзя записывать пустой набор autoGCP.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], gcpfilepath[MAX_PATH];
	#warning ацкое зло, но думать ломы
	splitpath(const_cast<char *>(inputFileName.c_str()), drive, dir, fname, 0 );
	//strlwr( fname );

	int l = strlen( fname );
	char buf[7], *p = fname + l;
	sprintf( buf, "_%d", n_comb );
	strcpy( p, buf );

	// Конструирование полного имени файла GCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fGPCXDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fGPCXDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( gcpfilepath, MAX_PATH );
		int t = strlen(gcpfilepath);
		if( gcpfilepath[t-1] != DIRD ) {
			gcpfilepath[t] = DIRD;
			gcpfilepath[t+1] = '\0';
		}
		splitpath( gcpfilepath, drive, dir, 0, 0 );
	}
	//    l = strlen( dir );
	//    p = dir + l;
	//    strcpy( p, output_autoGCPDir );
	makepath( gcpfilepath, drive, dir, fname, "gcp" );

	FILE * f;
	if( !(f = fopen( gcpfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
	fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
	fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
	fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
	fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
	fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
	fprintf( f, "%d\t ; число autoGCP\n", n );

	// Запись autoGCP.
	for( int i = 0; i < n; i++ ) {
		if( chips[i].fChipType[0] == TDChip::GraundChip ) {
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chips[i].X1+chips[i].ChipSize().x()/2, chips[i].Y1+chips[i].ChipSize().y()/2,
					 chips[i].dX[0], chips[i].dY[0], 4, chips[i].fNormStatSign[0], chips[i].PercCloudy );
		} else {
			if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip )
				fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
						 chips[i].X1+chips[i].ChipSize().x()/2,chips[i].Y1+chips[i].ChipSize().y()/2,
						 chips[i].dX[1], chips[i].dY[1], 2, chips[i].fNormStatSign[1], chips[i].PercCloudy );
		}
	}

	fclose( f );
};


void open_Correction(  TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, double & navigProbabI, double & navigProbabII, unsigned & NumbGCPs,
					   double & ResRmsM, unsigned & CenterConfig_X,  double & BaseGCPs_X, unsigned & CenterConfig_Y,  double & BaseGCPs_Y ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для чтения коррекции!!!" );

	//    unsigned long n = fACM.numberOfGraundChips();   // Число autoGCP.

	//    if( n <= 0 ) return; // Нельзя записывать коррекцию при пустом наборе autoGCPs.

	/*    char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], corfilepath[MAX_PATH];
	    splitpath( inputFileName, 0, 0, fname, 0 );
	    //strlwr( fname );*/
	// Конструирование полного имени файла с autoGCPs.
	// Проверка существования каталога fTargetDataDir.
	/*	if( check_dir( fCS.fLogDir ) == 0 ) splitpath( fCS.fLogDir, drive, dir, 0, 0 );
		else{
			_fullpath( corfilepath, ".", MAX_PATH );
			strcat( corfilepath, "\\" );
			splitpath( corfilepath, drive, dir, 0, 0 );
		}

	    makepath( corfilepath, drive, dir, fname, "cor" );
	*/
	FILE * f;
	if( !(f = fopen( inputFileCorr, "r" )) ) {
		throw ae1;
	}

	char buf[501];
	fgets( buf, 500, f );

	fgets( buf, 500, f );
	sscanf( buf, "%lf", &(fDAVHRR.fMinCOP->roll) );
	fgets( buf, 500, f );
	sscanf( buf, "%lf", &(fDAVHRR.fMinCOP->pitch) );
	fgets( buf, 500, f );
	sscanf( buf, "%lf", &(fDAVHRR.fMinCOP->yaw) );

	fgets( buf, 100, f );
	fgets( buf, 500, f );
	fgets( buf, 500, f );
	sscanf( buf, "%lf", &navigProbabI );
	fgets( buf, 500, f );
	sscanf( buf, "%lf", &navigProbabII );
	fgets( buf, 500, f );
	sscanf( buf, "%d", &NumbGCPs );
	fgets( buf, 500, f );
	sscanf( buf, "%lf", &ResRmsM );
	fgets( buf, 500, f );
	sscanf( buf, "%lf", &BaseGCPs_X );

	fgets( buf, 100, f );
	fgets( buf, 500, f );
	fgets( buf, 500, f );
	sscanf( buf, "%d", &CenterConfig_X );
	fgets( buf, 500, f );
	sscanf( buf, "%lf", &BaseGCPs_Y );
	fgets( buf, 500, f );
	sscanf( buf, "%d", &CenterConfig_Y );


	/*    fprintf( f, "Значения углов платформы спутника, восстановленных по линейной регрессии:\n" );
	    fprintf( f, "%.14f\t; roll_LR\n", fDAVHRR.fRegrCOP->roll );
	    fprintf( f, "%.14f\t; pitch_LR\n", fDAVHRR.fRegrCOP->pitch );
	    fprintf( f, "%.14f\t; yaw_LR\n", fDAVHRR.fRegrCOP->yaw );
	    fprintf( f, "Значения параметров оценки точности привязки по линейной регрессии:\n" );
	    fprintf( f, "%d\t; numberGCPs\n%.7f\t; resAvgM\n%.7f\t; resStdDevM\n%.7f\t; resRmsM\n%.7f\t; resMaxM\n%.7f\t; baseGCPs_X\n%.7f\t; baseGCPs_Y\n",
	                fGCPNumber, fResAvgM, fResStdDevM, fResRmsM, fResMaxM, fDAVHRR.l_BaseX, fDAVHRR.l_BaseY );
	    fprintf( f, "%.7f\t; corResAvgX\n", fCorResAvgX );
	    fprintf( f, "%.7f\t; corResAvgY\n", fCorResAvgY );
	    fprintf( f, "%.14f\t; max_error_angles_LR\n", fDAVHRR.max_error_angles );
	    fprintf( f, "%.14f\t; error_roll_LR\n", fDAVHRR.error_roll );
	    fprintf( f, "%.14f\t; error_pitch_LR\n", fDAVHRR.error_pitch );
	    fprintf( f, "%.14f\t; error_yaw_LR\n", fDAVHRR.error_yaw );
	*/
	fclose( f );
}


void save_Correction( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM,
					  double navigProbabilityI, double navigProbabilityII, unsigned centerConfig_X, unsigned centerConfig_Y ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи коррекции!!!" );

	unsigned long n = fACM.numberOfGraundChips();   // Число autoGCP.

	if( n <= 0 )
		return; // Нельзя записывать коррекцию при пустом наборе autoGCPs.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], corfilepath[MAX_PATH];
	#warning ацкое зло, но думать ломы
	splitpath(const_cast<char *>(inputFileName.c_str()), 0, 0, fname, 0 );
	//strlwr( fname );

	// Конструирование полного имени файла с autoGCPs.
	// Проверка существования каталога fSourceDataDir.
	if( check_dir( fCS.fSourceDataDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fSourceDataDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( corfilepath, MAX_PATH );
		int t = strlen(corfilepath);
		if( corfilepath[t-1] != DIRD ) {
			corfilepath[t] = DIRD;
			corfilepath[t+1] = '\0';
		}
		splitpath( corfilepath, drive, dir, 0, 0 );
	}
	makepath( corfilepath, drive, dir, fname, "cor" );

	FILE * f;
	if( !(f = fopen( corfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "Праметры коррекции (satellite attitude & orbital parametrs correction):\n" );
	fprintf( f, "%d\t; NORAD orbit - номер витка телеграммы\n", fDAVHRR.fISP->fRevNum );
	fprintf( f, "%.14f\t; roll - угол крена\n", fDAVHRR.fRegrCOP->roll );
	fprintf( f, "%.14f\t; pitch - угол тангажа\n", fDAVHRR.fRegrCOP->pitch );
	fprintf( f, "%.14f\t; yaw - угол рысканья\n", fDAVHRR.fRegrCOP->yaw );
	fprintf( f, "%.14f\t; dn0 - среднее движение\n", fDAVHRR.fRegrCOP->dn0 );
	fprintf( f, "%.14f\t; di0 - наклонение\n", fDAVHRR.fRegrCOP->di0 );
	fprintf( f, "%.14f\t; draan0 - восходящий узел\n", fDAVHRR.fRegrCOP->draan0 );
	fprintf( f, "%.14f\t; dm0 - средняя аномалия\n", fDAVHRR.fRegrCOP->dm0 );

	fprintf( f, "\nЗначения параметров оценки точности привязки (navigate precision):\n" );
	fprintf( f, "%.1f\t; navigProbabilityI\n%.1f\t; navigProbabilityII\n%ld\t; numberGCPs\n%.5f\t; resRmsM\n%.5f\t; baseGCPs_X\n",
			 navigProbabilityI, navigProbabilityII, fGCPNumber, fResRmsM, fDAVHRR.l_BaseX );

	fprintf( f, "\nЗначения параметров конфигурации реперных точек (configuration of GCPs):\n" );
	fprintf( f, "%d\t; centerConfig_X\n%.5f\t; baseGCPs_Y\n%d\t; centerConfig_Y\n",
			 centerConfig_X, fDAVHRR.l_BaseY, centerConfig_Y );

	fprintf( f, "\nЗначения параметров остаточных невязок в реперных точках (residual navigation errors):\n" );
	fprintf( f, "%.5f\t; resAvgM\n%.5f\t; resStdDevM\n%.5f\t; resMaxM\n",
			 fResAvgM, fResStdDevM, fResMaxM );
	fprintf( f, "%.5f\t; fResAvgMX\n%.5f\t; fResRmsX\n%.5f\t; fResMaxMX\n%.5f\t; fResAvgMY\n%.5f\t; fResRmsY\n%.5f\t; fResMaxMY\n",
			 fResAvgMX, fResRmsX, fResMaxMX, fResAvgMY, fResRmsY, fResMaxMY );

	fprintf( f, "\nЗначения корректируемых параметров, связанных с размером пиксела:\n" );
	fprintf( f, "%.14f\t; corResAvgX\n", fCorResAvgX );
	fprintf( f, "%.14f\t; corResAvgY\n", fCorResAvgY );
	fclose( f );
}


void save_Correction( TDataAVHRR & fDAVHRR, int n, TDChip * chips ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи коррекции!!!" );

	if( n <= 0 )
		return; // Нельзя записывать коррекцию при пустом наборе autoGCPs.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], corfilepath[MAX_PATH];
	#warning ацкое зло, но думать ломы
	splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
	//strlwr( fname );
	// Конструирование полного имени файла с autoGCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fSourceDataDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fSourceDataDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( corfilepath, MAX_PATH );
		int t = strlen(corfilepath);
		if( corfilepath[t-1] != DIRD ) {
			corfilepath[t] = DIRD;
			corfilepath[t+1] = '\0';
		}
		splitpath( corfilepath, drive, dir, 0, 0 );
	}
	int l = strlen( dir );
	char * p = dir + l;
	strcpy( p, output_autoGCPDir );
	makepath( corfilepath, drive, dir, fname, "cor" );

	FILE * f;
	if( !(f = fopen( corfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "Значения углов платформы спутника, восстановленных минимизацией:\n" );
	fprintf( f, "%.14f\t; roll_M\n", fDAVHRR.fMinCOP->roll );
	fprintf( f, "%.14f\t; pitch_M\n", fDAVHRR.fMinCOP->pitch );
	fprintf( f, "%.14f\t; yaw_M\n", fDAVHRR.fMinCOP->yaw );
	fprintf( f, "Значения углов платформы спутника, восстановленных по линейной регрессии:\n" );
	fprintf( f, "%.14f\t; roll_LR\n", fDAVHRR.fRegrCOP->roll );
	fprintf( f, "%.14f\t; pitch_LR\n", fDAVHRR.fRegrCOP->pitch );
	fprintf( f, "%.14f\t; yaw_LR\n", fDAVHRR.fRegrCOP->yaw );
	fprintf( f, "Значения параметров оценки точности привязки по линейной регрессии:\n" );
	fprintf( f, "%ld\t; numberGCPs\n%.7f\t; resAvgM\n%.7f\t; resStdDevM\n%.7f\t; resRmsM\n%.7f\t; resMaxM\n%.7f\t; baseGCPs_X\n%.7f\t; baseGCPs_Y\n",
			 fGCPNumber, fResAvgM, fResStdDevM, fResRmsM, fResMaxM, fDAVHRR.l_BaseX, fDAVHRR.l_BaseY );
	fprintf( f, "%.7f\t; corResAvgX\n", fCorResAvgX );
	fprintf( f, "%.7f\t; corResAvgY\n", fCorResAvgY );
	fprintf( f, "%.14f\t; max_error_angles_LR\n", fDAVHRR.max_error_angles );
	fprintf( f, "%.14f\t; error_roll_LR\n", fDAVHRR.error_roll );
	fprintf( f, "%.14f\t; error_pitch_LR\n", fDAVHRR.error_pitch );
	fprintf( f, "%.14f\t; error_yaw_LR\n", fDAVHRR.error_yaw );
	fclose( f );
}


void save_Correction_c( int n_comb, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи коррекции!!!" );

	unsigned long n = fACM.numberOfGraundChips();   // Число autoGCP.

	if( n <= 0 )
		return; // Нельзя записывать коррекцию при пустом наборе autoGCPs.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], corfilepath[MAX_PATH];
	splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
	//strlwr( fname );

	int l = strlen( fname );
	char buf[5], *p = fname + l;
	sprintf( buf, "_%d", n_comb );
	strcpy( p, buf );

	// Конструирование полного имени файла с autoGCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fGPCXDir .c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fGPCXDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( corfilepath, MAX_PATH );
		int t = strlen(corfilepath);
		if( corfilepath[t-1] != DIRD ) {
			corfilepath[t] = DIRD;
			corfilepath[t+1] = '\0';
		}
		splitpath( corfilepath, drive, dir, 0, 0 );
	}
	//    l = strlen( dir );
	//    p = dir + l;
	//    strcpy( p, output_autoGCPDir );
	makepath( corfilepath, drive, dir, fname, "cor" );

	FILE * f;
	if( !(f = fopen( corfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "Значения углов платформы спутника, восстановленных минимизацией:\n" );
	fprintf( f, "%.14f\t; roll_M\n", fDAVHRR.fMinCOP->roll );
	fprintf( f, "%.14f\t; pitch_M\n", fDAVHRR.fMinCOP->pitch );
	fprintf( f, "%.14f\t; yaw_M\n", fDAVHRR.fMinCOP->yaw );
	fprintf( f, "Значения углов платформы спутника, восстановленных по линейной регрессии:\n" );
	fprintf( f, "%.14f\t; roll_LR\n", fDAVHRR.fRegrCOP->roll );
	fprintf( f, "%.14f\t; pitch_LR\n", fDAVHRR.fRegrCOP->pitch );
	fprintf( f, "%.14f\t; yaw_LR\n", fDAVHRR.fRegrCOP->yaw );
	fprintf( f, "Значения параметров оценки точности привязки по линейной регрессии:\n" );
	fprintf( f, "%ld\t; numberGCPs\n%.7f\t; resAvgM\n%.7f\t; resStdDevM\n%.7f\t; resRmsM\n%.7f\t; resMaxM\n%.7f\t; baseGCPs_X\n%.7f\t; baseGCPs_Y\n",
			 fGCPNumber, fResAvgM, fResStdDevM, fResRmsM, fResMaxM, fDAVHRR.l_BaseX, fDAVHRR.l_BaseY );
	fprintf( f, "%.7f\t; corResAvgX\n", fCorResAvgX );
	fprintf( f, "%.7f\t; corResAvgY\n", fCorResAvgY );
	fprintf( f, "%.14f\t; max_error_angles_LR\n", fDAVHRR.max_error_angles );
	fprintf( f, "%.14f\t; error_roll_LR\n", fDAVHRR.error_roll );
	fprintf( f, "%.14f\t; error_pitch_LR\n", fDAVHRR.error_pitch );
	fprintf( f, "%.14f\t; error_yaw_LR\n", fDAVHRR.error_yaw );
	fclose( f );
}


void save_Correction_c( int n_comb, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM,
						double navigProbabilityI, double navigProbabilityII, unsigned centerConfig_X, unsigned centerConfig_Y ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи коррекции!!!" );

	unsigned long n = fACM.numberOfGraundChips();   // Число autoGCP.

	if( n <= 0 )
		return; // Нельзя записывать коррекцию при пустом наборе autoGCPs.

	//    print_log( "Запись файла с параметрами корррекции." );

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], corfilepath[MAX_PATH];
	splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
	//strlwr( fname );

	int l = strlen( fname );
	char buf[5], *p = fname + l;
	sprintf( buf, "_%d", n_comb );
	strcpy( p, buf );

	// Конструирование полного имени файла с autoGCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fStatDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fStatDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( corfilepath, MAX_PATH );
		int t = strlen(corfilepath);
		if( corfilepath[t-1] != DIRD ) {
			corfilepath[t] = DIRD;
			corfilepath[t+1] = '\0';
		}
		splitpath( corfilepath, drive, dir, 0, 0 );
	}
	//    int l = strlen( dir );
	//    char * p = dir + l;
	//    strcpy( p, output_autoGCPDir );
	makepath( corfilepath, drive, dir, fname, "cor" );

	FILE * f;
	if( !(f = fopen( corfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "Значения углов положения платформы спутника (satellite attitude):\n" );
	fprintf( f, "%.14f\t; roll_LR\n", fDAVHRR.fRegrCOP->roll );
	fprintf( f, "%.14f\t; pitch_LR\n", fDAVHRR.fRegrCOP->pitch );
	fprintf( f, "%.14f\t; yaw_LR\n", fDAVHRR.fRegrCOP->yaw );

	fprintf( f, "\nЗначения параметров оценки точности привязки (navigate precision):\n" );
	fprintf( f, "%.1f\t; navigProbabilityI\n%.1f\t; navigProbabilityII\n%ld\t; numberGCPs\n%.5f\t; resRmsM\n%.5f\t; baseGCPs_X\n",
			 navigProbabilityI, navigProbabilityII, fGCPNumber, fResRmsM, fDAVHRR.l_BaseX );

	fprintf( f, "\nЗначения параметров конфигурации реперных точек (configuration of GCPs):\n" );
	fprintf( f, "%d\t; centerConfig_X\n%.5f\t; baseGCPs_Y\n%d\t; centerConfig_Y\n",
			 centerConfig_X, fDAVHRR.l_BaseY, centerConfig_Y );

	fprintf( f, "\nЗначения параметров остаточных невязок в реперных точках (residual navigation errors):\n" );
	fprintf( f, "%.5f\t; resAvgM\n%.5f\t; resStdDevM\n%.5f\t; resMaxM\n",
			 fResAvgM, fResStdDevM, fResMaxM );
	fprintf( f, "%.5f\t; fResAvgMX\n%.5f\t; fResRmsX\n%.5f\t; fResMaxMX\n%.5f\t; fResAvgMY\n%.5f\t; fResRmsY\n%.5f\t; fResMaxMY\n",
			 fResAvgMX, fResRmsX, fResMaxMX, fResAvgMY, fResRmsY, fResMaxMY );

	fprintf( f, "\nЗначения корректируемых параметров, связанных с размером пиксела:\n" );
	fprintf( f, "%.14f\t; corResAvgX\n", fCorResAvgX );
	fprintf( f, "%.14f\t; corResAvgY\n", fCorResAvgY );
	fclose( f );
}


void save_Correction_c( int n_comb, TDataAVHRR & fDAVHRR, int n, TDChip * chips ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи коррекции!!!" );

	if( n <= 0 )
		return; // Нельзя записывать коррекцию при пустом наборе autoGCPs.

	char drive[MAX_DRIVE], dir[MAX_DIR+100], fname[MAX_FNAME], corfilepath[MAX_PATH];
	splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
	//strlwr( fname );

	int l = strlen( fname );
	char buf[5], *p = fname + l;
	sprintf( buf, "_%d", n_comb );
	strcpy( p, buf );

	// Конструирование полного имени файла с autoGCPs.
	// Проверка существования каталога fTargetDataDir.
	if( check_dir( fCS.fGPCXDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fGPCXDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( corfilepath, MAX_PATH );
		int t = strlen(corfilepath);
		if( corfilepath[t-1] != DIRD ) {
			corfilepath[t] = DIRD;
			corfilepath[t+1] = '\0';
		}
		splitpath( corfilepath, drive, dir, 0, 0 );
	}
	l = strlen( dir );
	p = dir + l;
	strcpy( p, output_autoGCPDir );
	makepath( corfilepath, drive, dir, fname, "cor" );

	FILE * f;
	if( !(f = fopen( corfilepath, "w" )) ) {
		throw ae1;
	}

	fprintf( f, "Значения углов платформы спутника, восстановленных минимизацией:\n" );
	fprintf( f, "%.14f\t; roll_M\n", fDAVHRR.fMinCOP->roll );
	fprintf( f, "%.14f\t; pitch_M\n", fDAVHRR.fMinCOP->pitch );
	fprintf( f, "%.14f\t; yaw_M\n", fDAVHRR.fMinCOP->yaw );
	fprintf( f, "Значения углов платформы спутника, восстановленных по линейной регрессии:\n" );
	fprintf( f, "%.14f\t; roll_LR\n", fDAVHRR.fRegrCOP->roll );
	fprintf( f, "%.14f\t; pitch_LR\n", fDAVHRR.fRegrCOP->pitch );
	fprintf( f, "%.14f\t; yaw_LR\n", fDAVHRR.fRegrCOP->yaw );
	fprintf( f, "Значения параметров оценки точности привязки по линейной регрессии:\n" );
	fprintf( f, "%ld\t; numberGCPs\n%.7f\t; resAvgM\n%.7f\t; resStdDevM\n%.7f\t; resRmsM\n%.7f\t; resMaxM\n%.7f\t; baseGCPs_X\n%.7f\t; baseGCPs_Y\n",
			 fGCPNumber, fResAvgM, fResStdDevM, fResRmsM, fResMaxM, fDAVHRR.l_BaseX, fDAVHRR.l_BaseY );
	fprintf( f, "%.7f\t; corResAvgX\n", fCorResAvgX );
	fprintf( f, "%.7f\t; corResAvgY\n", fCorResAvgY );
	fprintf( f, "%.14f\t; max_error_angles_LR\n", fDAVHRR.max_error_angles );
	fprintf( f, "%.14f\t; error_roll_LR\n", fDAVHRR.error_roll );
	fprintf( f, "%.14f\t; error_pitch_LR\n", fDAVHRR.error_pitch );
	fprintf( f, "%.14f\t; error_yaw_LR\n", fDAVHRR.error_yaw );
	fclose( f );
}


void save_intervGCP( bool flag_rejct, const char * sign_record, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи статистики!!!" );

	TDChip::ChipType chip_type = TDChip::GraundChip;
	unsigned long n;
	if( !flag_rejct ) {
		n = fACM.numberOfGraundChips();
		chip_type = TDChip::GraundChip;
	} else {
		n = fACM.numberOfControlChips();
		chip_type = TDChip::ControlChip;
	}
	if( n <= 0 )
		return; // Нельзя записывать при пустом наборе GCP.

	FILE * f;
	// Дата.
	int month, date;
	dayToDate( fDAVHRR.fNIP->fYear, int(fDAVHRR.fNIP->fYearTime), &month, &date );

	char pathStatFileGCP[MAX_PATH], drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME];
	splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
	//strlwr( fname );

	int l = strlen( fname );
	char buf[5], *p = fname + l;
	sprintf( buf, "_%s", sign_record );
	strcpy( p, buf );

	// Конструирование полного имени файла статистики.
	// Проверка существования каталога fStatDir.
	if( check_dir( fCS.fStatDir.c_str() ) == 0 )
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(fCS.fStatDir.c_str()) , drive, dir, 0, 0 );
	else {
		getcwd( pathStatFileGCP, MAX_PATH );
		int t = strlen(pathStatFileGCP);
		if( pathStatFileGCP[t-1] != DIRD ) {
			pathStatFileGCP[t] = DIRD;
			pathStatFileGCP[t+1] = '\0';
		}
		splitpath( pathStatFileGCP, drive, dir, 0, 0 );
	}
	makepath( pathStatFileGCP, drive, dir, fname, "gcp" );

	if( !flagCreateFiles ) {
		if( !(f = fopen( pathStatFileGCP, "w" )) )
			throw ae1;
		switch( number_algorithm ) {
		case 1:
			fprintf( f, "морфологический\t ; алгоритм расчёта параметров GCP\n" );
			break;
		case 2:
			fprintf( f, "шаблонный\t ; алгоритм расчёта параметров GCP\n" );
			break;
		}

		fprintf( f, "%02d.%02d.%d\t ; Дата\n", date+1, month+1, fDAVHRR.fNIP->fYear );
		fprintf( f, "%.5f\t ; угол восхождения Солнца над горизонтом для центрального пиксела изображения (в градусах)\n", fDAVHRR.fHeighSanny_overHorizonDegree );
		fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
		fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
		fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
		fprintf( f, "%d\t ; номер витка NORAD\n", fDAVHRR.fISP->fRevNum );
		fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
		fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
		fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
		fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
		fprintf( f, "%ld\t ; число промежуточных реперных точек.\n", n );
	} else {
		if( !(f = fopen( pathStatFileGCP, "a" )) )
			throw ae1;
		fprintf( f, "\n%ld\t ; число промежуточных реперных точек.\n", n );
	}
	if( !flagMaskCloudyClear ) {
		fprintf( f, "Без фильтрации облачности.\n\n" );
	} else {
		fprintf( f, "С фильтрацией облачности.\n\n" );
	}

	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->Ball == 6 )
			continue;
		if( chip->fChipType[0] == chip_type ) {
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[0], chip->dY[0], 4, chip->fNormStatSign[0], chip->PercCloudy );
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == chip_type ) {
			fprintf( f, "%ld\t %ld\t %.7f\t %.7f\t %d\t %.10f\t %.3f\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->dX[1], chip->dY[1], 2, chip->fNormStatSign[1], chip->PercCloudy );
		}
	}

	fprintf( f, "\n<***********************************************************************************************************************************************************************>\n\n" );
	fclose( f );
};


void save_StatMaxGCP( bool flag_rejct, uint8_t nf, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи статистики!!!" );

	TDChip::ChipType chip_type = TDChip::GraundChip;
	unsigned long n;
	if( !flag_rejct ) {
		n = fACM.numberOfGraundChips();
		chip_type = TDChip::GraundChip;
	} else {
		n = fACM.numberOfControlChips();
		chip_type = TDChip::ControlChip;
	}

	if( n <= 0 )
		return; // Нельзя записывать при пустом наборе GCP.

	FILE * f;
	if( !flagCreateFiles && nf == 1 ) {
		// Дата.
		int month, date;
		dayToDate( fDAVHRR.fNIP->fYear, int(fDAVHRR.fNIP->fYearTime), &month, &date );

		char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME];
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
		//strlwr( fname );

		// Конструирование полного имени файла статистики.
		// Проверка существования каталога fStatDir.
		if( check_dir( fCS.fStatDir.c_str() ) == 0 )
			#warning ацкое зло, но думать ломы
			splitpath(const_cast<char *>(fCS.fStatDir.c_str()) , drive, dir, 0, 0 );
		else {
			getcwd( pathStatFileMax, MAX_PATH );
			int t = strlen(pathStatFileMax);
			if( pathStatFileMax[t-1] != DIRD ) {
				pathStatFileMax[t] = DIRD;
				pathStatFileMax[t+1] = '\0';
			}
			splitpath( pathStatFileMax, drive, dir, 0, 0 );
		}
		makepath( pathStatFileMax, drive, dir, fname, "sma" );

		if( !(f = fopen( pathStatFileMax, "w" )) )
			throw ae1;

		fprintf( f, "%02d.%02d.%d\t ; Дата\n", date+1, month+1, fDAVHRR.fNIP->fYear );
		fprintf( f, "%.5f\t ; угол восхождения Солнца над горизонтом для центрального пиксела изображения (в градусах)\n", fDAVHRR.fHeighSanny_overHorizonDegree );
		fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
		fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
		fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
		fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
		fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
		fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
		fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
		fprintf( f, "%ld\t ; число реперных точек.\n", n );
	} else {
		if( !(f = fopen( pathStatFileMax, "a" )) )
			throw ae1;
		fprintf( f, "\n%ld\t ; число реперных точек.\n", n );
	}
	if( !flagMaskCloudyClear ) {
		fprintf( f, "Без фильтрации облачности.\n\n" );
	} else {
		fprintf( f, "С фильтрацией облачности.\n\n" );
	}

	double alfa;
	unsigned n0 = 0, nn0 = 0, nns0 = 0, n1 = 0, nn1 = 0, nns1 = 0;
	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->Ball == 6 )
			continue;
		if( chip->fChipType[0] == chip_type ) {
			if( fabs( chip->fExtremAlgOptimum[0] - chip->fAbsExtremAlgOptimum[0] ) > 0.001 ) {
				alfa = anglescan( chip->X1+chip->ChipSize().x()/2, fDAVHRR.fBlk0.b0.satId );
				fprintf( f, "4\t %ld\t %ld\t\n%.3f\t %.3f\t %.3f\n%.5f\t %.5f\t %.7f\t %.5f\n%.5f\t %.5f\t %.7f\t %.5f\n%.14f\t %.14f %lu\t %.7f\t %.7f\t %.7f\t %.7f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->fAngleCenterSanny, alfa, chip->PercCloudy,
						 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->fExtremAlgOptimum[0],
						 chip->dXAbs[0], chip->dYAbs[0], chip->fAbsNormStatSign[0], chip->fAbsExtremAlgOptimum[0],
						 chip->psyCryt[0], chip->Ver[0], chip->numberDegreeofFreedom[0], chip->avgSea[0], chip->dyspSea[0], chip->avgLand[0], chip->dyspLand[0] );
				nn0++;
				if( chip->fAbsNormStatSign[0] >= fCS.fStatisticSignificanceThreshold )
					nns0++;
			} else {
				n0++;
			}
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == chip_type ) {
			if( fabs( chip->fExtremAlgOptimum[1] - chip->fAbsExtremAlgOptimum[1] ) > 0.001 ) {
				alfa = anglescan( chip->X1+chip->ChipSize().x()/2, fDAVHRR.fBlk0.b0.satId );
				fprintf( f, "2\t %ld\t %ld\t\n%.3f\t %.3f\t %.3f\n%.5f\t %.5f\t %.7f\t %.5f\n%.5f\t %.5f\t %.7f\t %.5f\n%.14f\t %.14f %lu\t %.7f\t %.7f\t %.7f\t %.7f\n",
						 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
						 chip->fAngleCenterSanny, alfa, chip->PercCloudy,
						 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->fExtremAlgOptimum[1],
						 chip->dXAbs[1], chip->dYAbs[1], chip->fAbsNormStatSign[1], chip->fAbsExtremAlgOptimum[1],
						 chip->psyCryt[1], chip->Ver[1], chip->numberDegreeofFreedom[1], chip->avgSea[1], chip->dyspSea[1], chip->avgLand[1], chip->dyspLand[1] );
				nn1++;
				if( chip->fAbsNormStatSign[1] >= fCS.fStatisticSignificanceThreshold )
					nns1++;
			} else {
				n1++;
			}
		}
	}
	fprintf( f, "\n<++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>\n" );
	fprintf( f, "Количество точек с равными максимумами по 4 каналу:\t%d\n", n0 );
	fprintf( f, "Количество точек с неравными максимумами по 4 каналу:\t%d\n", nn0 );
	fprintf( f, "Количество точек с неравными максимумами по 4 каналу, но у которых стат.знач-ть > порога:\t%d\n", nns0 );
	if( fDAVHRR.fChannels_forCalcGCPs[1] ) {
		fprintf( f, "Количество точек с равными максимумами по 2 каналу:\t%d\n", n1 );
		fprintf( f, "Количество точек с неравными максимумами по 2 каналу:\t%d\n", nn1 );
		fprintf( f, "Количество точек с неравными максимумами по 2 каналу, но у которых стат.знач-ть > порога:\t%d\n", nns1 );
	}
	fprintf( f, "<++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>\n" );
	fclose( f );
};


void saveStatisticaContour( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия файла для записи статистики!!!" );

	unsigned long n = fACM.numberOfGraundChipsStat();   // Число autoGCP.
	if( n <= 0 )
		return; // Нельзя записывать при пустом наборе autoGCP.

	FILE * f;
	if( !flagCreateFiles ) {
		// Дата.
		int month, date;
		dayToDate( fDAVHRR.fNIP->fYear, int(fDAVHRR.fNIP->fYearTime), &month, &date );

		char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME];
		#warning ацкое зло, но думать ломы
		splitpath(const_cast<char *>(inputFileName.c_str()) , drive, dir, fname, 0 );
		//strlwr( fname );

		int l = strlen( fname );
		char buf[5], *p = fname + l;
		sprintf( buf, "_%d", number_algorithm );
		strcpy( p, buf );

		// Конструирование полного имени файла статистики.
		// Проверка существования каталога fLogDir.
		if( check_dir( fCS.fStatDir.c_str() ) == 0 )
			#warning ацкое зло, но думать ломы
			splitpath(const_cast<char *>(fCS.fStatDir.c_str()) , drive, dir, 0, 0 );
		else {
			getcwd( pathStatisticaFile, MAX_PATH );
			int t = strlen(pathStatisticaFile);
			if( pathStatisticaFile[t-1] != DIRD ) {
				pathStatisticaFile[t] = DIRD;
				pathStatisticaFile[t+1] = '\0';
			}
			splitpath( pathStatisticaFile, drive, dir, 0, 0 );
		}
		makepath( pathStatisticaFile, drive, dir, fname, "stat" );

		if( !(f = fopen( pathStatisticaFile, "w" )) ) {
			throw ae1;
		}
		switch( number_algorithm ) {
		case 1:
			fprintf( f, "морфологический\t ; алгоритм расчёта параметров GCP\n" );
			break;
		case 2:
			fprintf( f, "шаблонный\t ; алгоритм расчёта параметров GCP\n" );
			break;
		}

		fprintf( f, "%02d.%02d.%d\t ; Дата\n", date+1, month+1, fDAVHRR.fNIP->fYear );
		fprintf( f, "%.5f\t ; угол восхождения Солнца над горизонтом для центрального пиксела изображения (в градусах)\n", fDAVHRR.fHeighSanny_overHorizonDegree );
		fprintf( f, "%u\t ; id спутника\n", fDAVHRR.fBlk0.b0.satId );
		fprintf( f, "%s\t ; имя спутника\n", fDAVHRR.satName );
		fprintf( f, "%u\t ; номер витка\n", fDAVHRR.fBlk0.b0.revNum );
		fprintf( f, "%u\t ; число сканов\n", fDAVHRR.fNIP->fScans );
		fprintf( f, "%ld\t ; max ошибки географ. привязки по строкам\n", fCS.fMaxNavigationErrorLine );
		fprintf( f, "%ld\t ; max ошибки географ. привязки по столбцам\n", fCS.fMaxNavigationErrorColumn );
		fprintf( f, "%ld\t ; подробность маски и изображения\n", fCS.fMaskScale );
		fprintf( f, "%ld\t ; число чипов, имеющих контур без резких изменений в направлении.\n", n );
	} else {
		if( !(f = fopen( pathStatisticaFile, "a" )) ) {
			throw ae1;
		}
		fprintf( f, "\n%ld\t ; число чипов, имеющих контур без резких изменений в направлении.\n", n );
	}
	if( !flagMaskCloudyClear ) {
		fprintf( f, "Без фильтрации облачности.\n\n" );
	} else {
		fprintf( f, "С фильтрацией облачности.\n\n" );
	}

	double alfa;
	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->Ball != 6 )
			continue;
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			alfa = anglescan( chip->X1+chip->ChipSize().x()/2, fDAVHRR.fBlk0.b0.satId );
			fprintf( f, "4\t %ld\t %ld\n%.3f\t %.3f\t %.3f\n%.5f\t %.5f\t %.7f\t %.5f\n%.5f\t %.5f\t %.7f\t %.5f\n%.14f\t %.14f %lu\t %.7f\t %.7f\t %.7f\t %.7f\n\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->fAngleCenterSanny, alfa, chip->PercCloudy,
					 chip->dX[0], chip->dY[0], chip->fNormStatSign[0], chip->fExtremAlgOptimum[0],
					 chip->dXAbs[0], chip->dYAbs[0], chip->fAbsNormStatSign[0], chip->fAbsExtremAlgOptimum[0],
					 chip->psyCryt[0], chip->Ver[0], chip->numberDegreeofFreedom[0], chip->avgSea[0], chip->dyspSea[0], chip->avgLand[0], chip->dyspLand[0] );
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			alfa = anglescan( chip->X1+chip->ChipSize().x()/2, fDAVHRR.fBlk0.b0.satId );
			fprintf( f, "2\t %ld\t %ld\n%.3f\t %.3f\t %.3f\n%.5f\t %.5f\t %.7f\t %.5f\n%.5f\t %.5f\t %.7f\t %.5f\n%.14f\t %.14f %lu\t %.7f\t %.7f\t %.7f\t %.7f\n\n",
					 chip->X1+chip->ChipSize().x()/2, chip->Y1+chip->ChipSize().y()/2,
					 chip->fAngleCenterSanny, alfa, chip->PercCloudy,
					 chip->dX[1], chip->dY[1], chip->fNormStatSign[1], chip->fExtremAlgOptimum[1],
					 chip->dXAbs[1], chip->dYAbs[1], chip->fAbsNormStatSign[1], chip->fAbsExtremAlgOptimum[1],
					 chip->psyCryt[1], chip->Ver[1], chip->numberDegreeofFreedom[1], chip->avgSea[1], chip->dyspSea[1], chip->avgLand[1], chip->dyspLand[1] );
		}
	}

	fprintf( f, "\n<***********************************************************************************************************************************************************************>\n\n" );
	fclose( f );
};


// ----------------------------------------------------------------------------------

TDataAVHRR :: TDataAVHRR( const string& inputFilePath, bool flag_load ) :

fISP( 0 ),
fNIP( 0 ),
fMinCOP( 0 ),
fRegrCOP( 0 ),
fIniCOP( 0 ),
fSRef( 0 ),
fCloudyMask( 0 ),
fHeighSanny_overHorizonDegree(.0),
beta0_y(.0), beta1_y(.0), SS_y(.0), difX_y(.0),
stand_dev_est_beta0_y(.0), stand_dev_est_beta1_y(.0),
SR(.0), SS_x(.0),
sumX2_y(0), l_BaseX(.0), l_BaseY(.0),
error_roll(.0), error_pitch(.0), error_yaw(.0), max_error_angles(.0) {
	satName[0] = 0;

	for( int i=0; i<5; i++ ) {
		fka[i] = 0;
		fkb[i] = 0;
		fmaxPixValue[i] = 0;
		fCalibrData[i] = 0;
		fAvailableChannels[i] = false;
	}

	fChannels_forCalcGCPs[0] = true;
	fChannels_forCalcGCPs[1] = false;

	fDataFilePath[0] = 0;
	if( !inputFilePath.empty() ) {  // если программа запущена в обычном режиме и был указан файл данных
		strncpy( fDataFilePath, inputFilePath.c_str(), MAX_PATH-1 );
		fDataFilePath[MAX_PATH-1] = '\0';
	}

	try {
		prepareToLoadNewFile( flag_load );
		if( !flag_load )
			loadCalibrFiles();
	} catch( TAccessExc ae ) {
		errorMessage( ae.what() );
	} catch( TParamExc pe ) {
		errorMessage( pe.what() );
	}
}


TDataAVHRR :: ~TDataAVHRR() {
	delete fIR;
	delete fSRef;
	delete fISP;
	delete fNIP;
	delete fIniCOP;
	delete fMinCOP;
	delete fRegrCOP;

	if( fCloudyMask != 0 )
		delete [] fCloudyMask;

	for( int i=0; i<5; i++ )
		if( fAvailableChannels[i] )
			delete [] fCalibrData[i];
}



void TDataAVHRR::prepareToLoadNewFile( bool flag_load ) throw ( TAccessExc, TParamExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка доступа к исходному файлу данных!!!" );
	TParamExc pe1( 1, "ERROR: Недопустимый формат исходного файла данных!!!" );

	TBlk0_HRPT * bh = (TBlk0_HRPT *)&fBlk0;
	//    fLostScans = bh->lostFrameNum;

	// Чтение 0-блока.
	FILE * f;
	if( (f = fopen( fDataFilePath, "rb" )) == 0 || fread( (char*)&fBlk0, 1, 512, f ) != 512 ) {
		fclose( f );
		throw ae1;
	}
	fclose( f );

	if( fBlk0.b0.formatType != 0xff || fBlk0.b0.dataType1 != 1 || fBlk0.b0.dataType2 != 1 ) {
		throw pe1;
	}

	if( strcmp( fBlk0.b0.satName, "NOAA" ) == 0 && (fBlk0.b0.satId & 0xffff) == 0 ) {  // файл формата 1999 года
		sprintf( satName, "NOAA %u", uint32_t( fBlk0.b0.satId ) >> 16 );
	} else {   // файл формата 2000 года
		TSatInfoTable st;
		if( st.setToSatelliteWithId( fBlk0.b0.satId ) )
			strcpy( satName, st.satName() );
		else
			strcpy( satName, "unknown" );
	}

	// Инициализация fChannels_forCalcGCPs и fAvailableChannels.
	unsigned long mask = bh->frameMask;
	for( int i=0; i<5; i++ )
		fAvailableChannels[i] = (mask & ( 32768 >> i )) ? true : false;

	fISP = new TIniSatParams( fBlk0 );
	fNIP = new TNOAAImageParams( fBlk0 );
	fIniCOP = new TCorrectionParams( fBlk0 );
	fMinCOP = new TCorrectionParams( 2 );
	fRegrCOP = new TCorrectionParams( 2 );
	fSRef = new TStraightReferencer( *fISP, *fNIP, *fRegrCOP );
	fIR = new TInverseReferencer( *fISP, *fNIP, *fRegrCOP );

	double lon, lat;

	fSRef->xy2ll( 0., fNIP->fScans / 2., &lon, &lat );
	lon = -lon;
	TAstronom fAstronom;
	double heighSanny_1 = fAstronom.shcrds( fAstronom.julian( fNIP->fYear, fNIP->fYearTime + 1 ), lon, lat );
	fSRef->xy2ll( 2047., fNIP->fScans / 2., &lon, &lat );
	lon = -lon;
	double heighSanny_2 = fAstronom.shcrds( fAstronom.julian( fNIP->fYear, fNIP->fYearTime + 1 ), lon, lat );
	if( heighSanny_1 < heighSanny_2 )
		heighSanny_1 = heighSanny_2;

	fSRef->xy2ll( 1024., fNIP->fScans / 2., &lon, &lat );
	lon = -lon;
	fHeighSanny_overHorizonDegree = fAstronom.shcrds( fAstronom.julian( fNIP->fYear, fNIP->fYearTime + 1 ), lon, lat );

	if( heighSanny_1 <= 5. ) {
		fAvailableChannels[0] = false;
		fAvailableChannels[1] = false;
	} else {
		if( fAvailableChannels[1] )
			fChannels_forCalcGCPs[1] = true;
	}

	if( !flag_load ) {
		for( int i=0; i<5; i++ ) {
			// выделение памяти под калиброванные данные каналов
			fCalibrData[i] = fAvailableChannels[i] ? new short [2048*fNIP->fScans] : 0;
		}

		fCloudyMask = new char [2048*fNIP->fScans];
		memset( fCloudyMask, 0, 2048 * fNIP->fScans );
	} else {
		for( int i=0; i<5; i++ )
			fAvailableChannels[i] = false;
	}
}


void TDataAVHRR::loadCalibrFiles() throw( TAccessExc, TParamExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка доступа к файлу калиброванных данных!!!" );
	TParamExc pe1( 1, "ERROR: Недопустимый формат файла калиброванных данных!!!" );

	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], path[MAX_PATH];
	splitpath( fDataFilePath, drive, dir, fname, 0 );

	makepath( path, drive, dir, fname, 0 );

	int l = strlen( path );
	char *p = path + l;

	TBlk0 fblk0;
	TBlk0_AVHRR * fBlk0_Calibr = reinterpret_cast<TBlk0_AVHRR *>(reinterpret_cast<char *>(&fblk0));

	FILE * f;
	for( int i = 0; i < 5; i++ ) {
		if( fAvailableChannels[i] ) {
			switch( i ) {
			case 0:
				strcpy( p, "_1.clb" );
				break;
			case 1:
				strcpy( p, "_2.clb" );
				break;
			case 2:
				strcpy( p, "_3.clb" );
				break;
			case 3:
				strcpy( p, "_4.clb" );
				break;
			case 4:
				strcpy( p, "_5.clb" );
				break;
			}

			if( ( f = fopen( path, "rb" ) ) == 0 || fread( (char*)&fblk0, 1, 512, f ) != 512 )
				throw ae1;
			if( fblk0.b0.formatType != 0xff || fblk0.b0.dataType1 != 2 || fblk0.b0.dataType2 != 1 )
				throw pe1;
			if( fBlk0_Calibr->processLevel != 0x00000001 )
				throw pe1;

			fka[i] = fBlk0_Calibr->ka;
			fkb[i] = fBlk0_Calibr->kb;
			fmaxPixValue[i] = fBlk0_Calibr->maxPixelValue;

			fseek( f, sizeof(TBlk0), SEEK_SET );
			fread( fCalibrData[i], 2, fNIP->fScans * 2048, f );
			fclose( f );
		}
	}
}


void TDataAVHRR::calculateCorrection_byAutoGCPMinimiz( int fMinMethod, int fMinParams[COR_PARAM_NUM], TAufitChipMask & fACM ) {
	TOrbitCorrector oc( *fISP, *fNIP, *fMinCOP );
	oc.calcCorrection( true, fACM, fMinMethod, fMinParams );
	setCorrectionParams( false, oc.correction(), fACM );
}


void TDataAVHRR::calculateCorrection_byAutoGCP_onLineRegres( TAufitChipMask & fACM ) {
	TOrbitCorrector oc( *fISP, *fNIP, *fRegrCOP );
	oc.calcCorrectionAngles_onLinearRegression( fACM, flag_yaw );
	beta1_y = oc.beta1_y;
	setCorrectionParams( true, oc.correction(), fACM );
}


void TDataAVHRR::calculateCorrection_byAutoGCPLineRegres( TAufitChipMask & fACM, bool flag_precision ) {
	TOrbitCorrector oc( *fISP, *fNIP, *fRegrCOP );
	oc.calcCorrection_onLinearRegressionCoefficients( true, flag_precision, fACM, false );
	beta1_y = oc.beta1_y;
	setCorrectionParams( true, oc.correction(), fACM );
	if( flag_precision ) {
		SR = oc.SR;
		beta0_y = oc.beta0_y;
		SS_y = oc.SS_y;
		difX_y = oc.difX_y;
		sumX2_y = oc.sumX2_y;
		stand_dev_est_beta0_y = oc.stand_dev_est_beta0_y;
		stand_dev_est_beta1_y = oc.stand_dev_est_beta1_y;
		SS_x = oc.SS_x;
		l_BaseX = ( maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1 ) / 2048.;
		l_BaseY = ( maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1 ) / double( fNIP->fScans );
		error_yaw = ( oc.error_yaw );// / ( 1.5 * l_BaseX );
		error_roll = oc.error_roll;
		error_pitch = oc.error_pitch;
		max_error_angles = error_roll;
		if( max_error_angles < error_pitch )
			max_error_angles = error_pitch;
		if( max_error_angles < error_yaw )
			max_error_angles = error_yaw;
	}
}


void TDataAVHRR::calculateCorrection_byAutoGCPLineRegres( int n, TDChip * chips, TAufitChipMask & fACM, bool flag_precision ) {
	TOrbitCorrector oc( *fISP, *fNIP, *fRegrCOP );
	oc.calcCorrection_onLinearRegressionCoefficients( true, flag_precision, n, chips, fACM, false );
	beta1_y = oc.beta1_y;
	setCorrectionParams( true, oc.correction(), n, chips, fACM );
	if( flag_precision ) {
		SR = oc.SR;
		beta0_y = oc.beta0_y;
		SS_y = oc.SS_y;
		difX_y = oc.difX_y;
		sumX2_y = oc.sumX2_y;
		stand_dev_est_beta0_y = oc.stand_dev_est_beta0_y;
		stand_dev_est_beta1_y = oc.stand_dev_est_beta1_y;
		SS_x = oc.SS_x;
		l_BaseX = ( maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1 ) / 2048.;
		l_BaseY = ( maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1 ) / double( fNIP->fScans );
		error_yaw = ( oc.error_yaw );// / ( 1.5 * l_BaseX );
		error_roll = oc.error_roll;
		error_pitch = oc.error_pitch;
		max_error_angles = error_roll;
		if( max_error_angles < error_pitch )
			max_error_angles = error_pitch;
		if( max_error_angles < error_yaw )
			max_error_angles = error_yaw;
	}
}


void TDataAVHRR::calculateCorrection_byAutoGCP_onLineRegres( double cor_avg_dX, double cor_avg_dY, TAufitChipMask & fACM, bool flag_yaw ) {
	setCorrectionParams( true, TCorrectionParams( 2 ), fACM, true );
	TOrbitCorrector oc( *fISP, *fNIP, *fRegrCOP );
	oc.cor_avg_dX = cor_avg_dX;
	oc.cor_avg_dY = cor_avg_dY;
	oc.lr_yaw = beta1_y;
	oc.calcCorrectionAngles_onLinearRegression( fACM, flag_yaw );
	setCorrectionParams( true, oc.correction(), fACM );
}


void TDataAVHRR::calculateCorrection_byAutoGCPLineRegres( double cor_avg_dX, double cor_avg_dY, TAufitChipMask & fACM, bool flag_yaw ) {
	TOrbitCorrector oc( *fISP, *fNIP, *fRegrCOP );
	oc.cor_avg_dX = cor_avg_dX;
	oc.cor_avg_dY = cor_avg_dY;
	oc.lr_yaw = beta1_y;
	oc.calcCorrection_onLinearRegressionCoefficients( true, true, fACM, flag_yaw );
	setCorrectionParams( true, oc.correction(), fACM );
	*fMinCOP = *fRegrCOP;
}


void TDataAVHRR::calculateCorrection_byAutoGCPLineRegres( double cor_avg_dX, double cor_avg_dY, int n, TDChip * chips, TAufitChipMask & fACM, bool flag_yaw ) {
	TOrbitCorrector oc( *fISP, *fNIP, *fRegrCOP );
	oc.cor_avg_dX = cor_avg_dX;
	oc.cor_avg_dY = cor_avg_dY;
	oc.lr_yaw = beta1_y;
	oc.calcCorrection_onLinearRegressionCoefficients( true, true, n, chips, fACM, flag_yaw );
	setCorrectionParams( true, oc.correction(), n, chips, fACM );

	SR = oc.SR;
	beta0_y = oc.beta0_y;
	beta1_y = oc.beta1_y;
	SS_y = oc.SS_y;
	difX_y = oc.difX_y;
	sumX2_y = oc.sumX2_y;
	stand_dev_est_beta0_y = oc.stand_dev_est_beta0_y;
	stand_dev_est_beta1_y = oc.stand_dev_est_beta1_y;
	SS_x = oc.SS_x;
	l_BaseX = ( maxConfiguration_ofGroundX - minConfiguration_ofGroundX + 1 ) / 2048.;
	l_BaseY = ( maxConfiguration_ofGroundY - minConfiguration_ofGroundY + 1 ) / double( fNIP->fScans );
	error_yaw = ( oc.error_yaw );// / ( 1.5 * l_BaseX );
	error_roll = oc.error_roll;
	error_pitch = oc.error_pitch;
	max_error_angles = error_roll;
	if( max_error_angles < error_pitch )
		max_error_angles = error_pitch;
	if( max_error_angles < error_yaw )
		max_error_angles = error_yaw;
}


void TDataAVHRR::setCorrectionParams( bool flag, const TCorrectionParams & cop, TAufitChipMask & fACM, bool flag_ah ) {
	if( !flag )
		*fMinCOP = cop;
	else
		*fRegrCOP = cop;
	fSRef->setCorrectionParams( cop );
	fACM.setCorrectionParams( flag_ah, cop );
}


void TDataAVHRR::setCorrectionParams( bool flag, const TCorrectionParams & cop,  int n, TDChip * chips, TAufitChipMask & fACM, bool flag_ah ) {
	if( !flag )
		*fMinCOP = cop;
	else
		*fRegrCOP = cop;
	fSRef->setCorrectionParams( cop );
	fACM.setCorrectionParams( flag_ah, cop, n, chips );
}


void TDataAVHRR::saveCorToA0() throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка доступа к исходному файлу данных!!!" );

	print_log( "Запись параметров корррекции в файл." );

	if( access( fDataFilePath, F_OK|R_OK|W_OK ) != 0 ) {		// если файл данных имеет атрибут "read only"
		//if( chmod( fDataFilePath, S_IREAD | S_IWRITE ) != 0 )
		throw ae1;
	}

	TBlk0_HRPT blk0;
	int hdr = 0;
	struct stat fstat;
	struct utimbuf utb;

	if( stat( fDataFilePath, &fstat ) != 0 ) throw ae1;
	utb.actime = fstat.st_atime;
	utb.modtime = fstat.st_mtime;

	//HFILE fh;
	//FILESTATUS3 fs;
	//ulong action;
	//ulong actual;

	if(( hdr = open( fDataFilePath,O_RDONLY ) ) == -1 ) throw ae1;
	read( hdr, &blk0, sizeof(blk0) );
	close( hdr );
	// //DosOpen( fDataFilePath, &fh, &action, 0L, 0L, OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
	// //		 OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY, 0L );
	// // Запоминаем время последней записи в файл.
	//DosQueryFileInfo( fh, FIL_STANDARD, &fs, sizeof( FILESTATUS3 ) );
	// // Здесь же читаем 0-блок.
	//DosRead( fh, (char*)&b, 512, &actual );
	//DosClose( fh );

	blk0.corVersion = fRegrCOP->fVersion;
	blk0.corTBUSTime = int16_t( fRegrCOP->fTBUSTime * 1000. );
	blk0.corTime = int16_t( fRegrCOP->fTime * 1000. );
	blk0.corRoll = fRegrCOP->roll;
	blk0.corPitch = fRegrCOP->pitch;
	blk0.corYaw = fRegrCOP->yaw;

	if( (hdr = open( fDataFilePath, O_WRONLY )) == -1 ) throw ae1;
	write( hdr, &blk0, sizeof(blk0) );
	lseek( hdr, (off_t)0, SEEK_END );
	close( hdr );
	utime( fDataFilePath, &utb );
	// // Запись в файл измененного 0-блока c восстановлением времени записи в файл.
	//DosOpen( fDataFilePath, &fh, &action, 0L, 0L, OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
	//		 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_WRITEONLY, 0L );
	//DosWrite( fh, (char*)&b, 512, &actual );
	//DosSetFileInfo( fh, FIL_STANDARD, &fs, sizeof( FILESTATUS3 ) );
	//DosClose( fh );
}
