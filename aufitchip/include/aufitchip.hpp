/*-------------------------------------------------------------------------
    aufitchip.hpp
-------------------------------------------------------------------------*/

#ifndef _AUFITCHIP_HPP_
    #define _AUFITCHIP_HPP_

#include <tc_config.h>
#include <orbmodel.hpp>
#include <aufit_mask.hpp>
#include <filtrcloudy.hpp>
/*--------------------------------------------------------------------------------
    TConfigSetting - структура, содержащая конфигурационные установки программы
---------------------------------------------------------------------------------*/

struct TConfigSetting
{
    char fSourceDataDir[MAX_PATH];          // местонахождение необработанных файлов данных - .a0 вместе с .clb
	char fTargetDataDir[MAX_PATH];	         // каталог, в который следует переносить файлы с GCPs
    char fGPCXDir[MAX_PATH];   		     // местонахождение grayscale-PCX результатов совмещения опорных чипов
    char fLogDir[MAX_PATH];    		     // местонахождение log-файлов
    char fStatDir[MAX_PATH];    		     // местонахождение log-файлов статистики
    char fMaskandChipBaseDir[MAX_PATH];     // местонахождение файлов маски и базы чипов
    char ParametrsFiltrCloudyDir[MAX_PATH]; // местонахождение конфигурационных файлов параметров для фильтрации облачности
    int fMinMethod;                     // используемый метод минимизации: 1 - Powell, 2 - Nelder-Mead
/*
    0   time
    1   TBUS time
    2   roll
    3   pitch
    4   yaw
    5   n0
    6   i0
    7   raan0
    8   e0
    9   w0
    10  m0
*/
    int fMinParams[COR_PARAM_NUM];          // флаги параметров, по которым необходимо осуществлять минимизацию штрафной функции
    long fMaxNavigationErrorLine;           // максимально возможная ошибка первичной географической привязки по строке в пикселах.
    long fMaxNavigationErrorColumn;         // максимально возможная ошибка первичной географической привязки по столбцу в пикселах.
    long fMaskScale;                        // во сколько раз маска подробней изображения
    uint8_t fFiltrCloudyforChips;              // флаг для фильтрации облачности для чипов изображения: 0 - не фильтровать; 1 - фильтровать, но не отбраковывать чипы по порогу fMaxPercentageCloudy; 2 - фильтровать и отбраковывать чипы по порогу fMaxPercentageCloudy
    double fMaxPercentageCloudy;            // порог отбраковки чипов после процедуры фильтрации облачности по кол-ву отфильтрованных пикселов (включая и поисковую зону) в процентах
    uint8_t fRepeatCalcShiftsforChips;         // флаг для повторного выполнения вычисления смещений чипов изображения, используя средние оценки полученные на первом этапе вычислений: 0 - не выполнять второй этап вычислений; 1 - выполнять второй этап вычислений
    double fStatisticSignificanceThreshold; // порог отбраковки опорных чипов по нормализованному значению критерия статистической значимости для 2-го канала AVHRR (день) и 4-го канала AVHRR (ночь).
    double fLinRegresCoeff_dX;              // коэффициент для поправки параметров линейной регрессии смещения по столбцу
    double fLinRegresCoeff_dY;              // коэффициент для поправки параметров линейной регрессии смещения по строке
	bool juhbla;
}; // End of struct TConfigSetting.



/*-----------------------------------------------------------------------------
    TDataAVHRR - класс данных AVHHRR
-----------------------------------------------------------------------------*/
class TDataAVHRR {
public:
/*
    file_path   Полный путь входного файла данных.
*/
    TDataAVHRR( const char * file_path, bool flag_load );

    virtual ~TDataAVHRR();

    friend void print_log( const char * );

	friend void errorMessage( const char * );

    void prepareToLoadNewFile( bool flag_load ) throw ( TAccessExc, TParamExc );

    void loadCalibrFiles() throw( TAccessExc, TParamExc );

    void calculateCorrection_byAutoGCPMinimiz( int fMinMethod, int fMinParams[COR_PARAM_NUM], TAufitChipMask & fACM );

    void calculateCorrection_byAutoGCP_onLineRegres( TAufitChipMask & fACM );

    void calculateCorrection_byAutoGCPLineRegres( TAufitChipMask & fACM, bool flag_precision );

    void calculateCorrection_byAutoGCPLineRegres( int, TDChip *, TAufitChipMask & fACM, bool flag_precision );

    void calculateCorrection_byAutoGCP_onLineRegres( double cor_avg_dX, double cor_avg_dY, TAufitChipMask & fACM, bool flag_yaw );

    void calculateCorrection_byAutoGCPLineRegres( double cor_avg_dX, double cor_avg_dY, TAufitChipMask & fACM, bool flag_yaw );

    void calculateCorrection_byAutoGCPLineRegres( double cor_avg_dX, double cor_avg_dY, int, TDChip *, TAufitChipMask & fACM, bool flag_yaw );

    void setCorrectionParams( bool flag, const TCorrectionParams & cop, TAufitChipMask & fACM, bool flag_ah = true );

    void setCorrectionParams( bool flag, const TCorrectionParams & cop,  int, TDChip *, TAufitChipMask & fACM, bool flag_ah = true );

    void saveCorToA0() throw( TAccessExc );

    char fDataFilePath[MAX_PATH]; // Полное имя входного файла.

    char satName[16];

    unsigned fLostScans;

    double fHeighSanny_overHorizonDegree; // Высота Солнца над горизонтом (в градусах) для центрального пиксела снимка.

    bool fChannels_forCalcGCPs[2]; // По каким каналам идёт расчёт autoGCPs
    bool fAvailableChannels[5];

    short * fCalibrData[5];                 // Калиброванные данные 1,2,3,4,5 каналов AVHRR.

    double fka[5];
    double fkb[5];
    uint16_t fmaxPixValue[5];

    TIniSatParams * fISP;
    TNOAAImageParams * fNIP;
    TCorrectionParams * fMinCOP;            // Текущая коррекция. Если поле fVersion==0, коррекция отсутствует (вычисленная минимизацией).
    TCorrectionParams * fRegrCOP;           // Текущая коррекция. Если поле fVersion==0, коррекция отсутствует (вычислення по линейной регрессии).
    TCorrectionParams * fIniCOP;            // Коррекция, которая была в 0-блоке входного файла при его загрузке.
    TStraightReferencer * fSRef;
    TInverseReferencer * fIR;

    char * fCloudyMask;  // маска облачности изображения.

    TBlk0 fBlk0;

    double SR;
    double beta0_y, beta1_y, SS_y, difX_y;
    unsigned long sumX2_y;
    double stand_dev_est_beta0_y, stand_dev_est_beta1_y;
    double SS_x;
    double error_roll, error_pitch, error_yaw, max_error_angles;
    double l_BaseX, l_BaseY;

private:

// время последнего изменения загруженного сейчас файла
	//FDATE fCurFileDate;
	//FTIME fCurFileTime;
	//FDATE fPrevFileDate;
	//FTIME fPrevFileTime;

};// End of class TDataAVHRR.


int parseCommandString( int , char * [] );
void loadCfgSet( char * );
void filtrCloudyChips( TAufitChipMask & fACM, TFiltrCloudy & fFC );
void print_LogFile( const char * message_text, bool flag_createFile = true, bool flag_2ch = false ) throw( TAccessExc );
void print_LogStatisticSeriesFiles( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM );
void selection_Chips_for_GraundChips( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM );
void _calculateShifts_forGraundChips( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM );
void calculateShifts_forGraundChips( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM );
void save_intervGCP( bool flag_rejct, const char * sign_record, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc );
bool selection_GraundChips_for_autoGCPs_afterCalcCorrectionLineRegres( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM );
void calcResDisp_forAutoGCPs( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, double & base_x, double & base_y, double & res_m );
void calcResDisp_forAutoGCPs( TAufitChipMask & fACM, double & res_avg_dx, double & res_avg_dy );
void calcResDisp_forAutoGCPs( TAufitChipMask & fACM );
void calcConfigBases_forAutoGCPs( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, unsigned & centerConfig_X, double & base_X, unsigned & centerConfig_Y, double & base_Y );
void calcStatParams_forAutoGCPs( bool flagMin, bool flagstd, TAufitChipMask & fACM );
void navigation_probabilityPrecision( double base_X, double res_navigRMS, long namberGCPs, double & navigProbI, double & navigProbII );
void printFileLog(  TDataAVHRR & fDAVHRR, long centerConfig_X, long centerConfig_Y, double navigProbabilityI, double navigProbabilityII );
void save_Correction( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM, double navigProbabilityI, double navigProbabilityII, unsigned centerConfig_X, unsigned centerConfig_Y ) throw( TAccessExc );
void save_autoGCP( bool flag_mr, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc );
void saveStatisticaContour( TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc );
void selection_GraundChips_for_GCPs( bool flag_passage, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM );
void save_StatMaxGCP( bool flag_rejct, uint8_t nf, TDataAVHRR & fDAVHRR, TAufitChipMask & fACM ) throw( TAccessExc );
#endif
