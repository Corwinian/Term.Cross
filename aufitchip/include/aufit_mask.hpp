/*----------------------------------------------------------------------------------------------------------
    aufit_mask.hpp
    Класс реализующий алгоритм нахождения смещений заданного чипа в изображении относительно его местонахождения в маске.
----------------------------------------------------------------------------------------------------------*/

#ifndef _AUFIT_CHIP_MASK_HPP_
    #define _AUFIT_CHIP_MASK_HPP_

#include <tc_config.h>
#include <orbmodel.hpp>
#include <tdchip.hpp>
#include <y_util.hpp>

#define THICKNESS 3


class TAufitChipMask {
public:

    TAufitChipMask( char fMaskChipBaseFileDir[MAX_PATH], long scale, long navigation_error_line, long navigation_error_column,
                    TNOAAImageParams * nip, TIniSatParams * isp, TStraightReferencer * sr, TInverseReferencer * ir, const TCorrectionParams & cop,
                    long nrfc, char * mask_cloudy, bool channel1, short * calibr_data1, short max_value1, double Ka1, double Kb1,
                    bool channel2, short * calibr_data2, short max_value2, double Ka2, double Kb2 );

    virtual ~TAufitChipMask();

    friend void print_log( const char * );

	friend void errorMessage( const char * );

    void resultMask_to_pcx( char[MAX_PATH], TDChip*, unsigned long, uint8_t ) throw( TAccessExc );

    void resultContour_to_pcx(  char[MAX_PATH], TDChip*, unsigned long, uint8_t ) throw( TAccessExc );

    bool result_MaskorContourChips_to_pcx( uint8_t option_chips_to_pcx,  char fGPCXDir[MAX_PATH], char fDataFilePath[MAX_PATH] ) throw( TAccessExc );

    bool result_MaskorContour_GraundChipsorautoGCPs_to_pcx( bool graund_or_gcp, bool mask_or_contour,  char fGPCXDir[MAX_PATH], char fDataFilePath[MAX_PATH] ) throw( TAccessExc );

    void settingAvgShifts( long, double, double );

    void settingShiftsSearchZone( long, long );

    bool calculateShiftGraundChips( uint8_t );

    void calculateShiftChip_Templet_Quant2080( TDChip * );

    void calculateShiftChip_Morphological_MaxAverage( unsigned, TDChip * );

    void calc_land_sea_Quant2080_Levels( uint8_t );

    void calc_land_sea_QuantMedian_Levels( uint8_t );

    void calc_land_sea_Average_Levels( uint8_t );

    bool verification_parametrsGraundChips( bool );

    void verification_parametrsGraundChip( unsigned, TDChip * );

    void verification_parametrsGraundChip( unsigned, uint8_t, TDChip * );

    bool verification_parametrsGraundChipsAbs( bool );
    void verification_parametrsGraundChipAbs( unsigned, uint8_t, TDChip * );

    // Курсор для работы со списком чипов.
    //class ChipCursor {
    //friend class TAufitChipMask;
    //public:
    //    ChipCursor ( const TAufitChipMask & );
    //    ChipCursor ( const ChipCursor & );
    //    bool
    //        setToFirst(),
    //        setToLast(),
    //        setToNext(),
    //        setToPrevious(),
    //        isValid();
    //    void invalidate();
    //private:
    //    TChipList::Cursor fChipCursor;
    //};

    //friend class ChipCursor;
	TCLCursor createCursor();

    unsigned duplicateOfGCPs() const;

    bool duplicateOfGraundChips() const;

    bool duplicateOfGraundChips( int, TDChip * ) const;

    unsigned long numberOfChips() const;

    unsigned long numberOfStatChips() const;

    unsigned long numberOfReperChips( TDChip::ChipType ) const;

    unsigned long numberOfGraundChipsStat() const;

    unsigned long numberOfGraundChips() const;

    unsigned long numberOfControlChips() const;

    //TDChip * chipAt( const ChipCursor & cc ) const;

    void allGraundChips( TDChip * chip ) const;

    void setControlToCheckChips();

    void setGraundToCheckChips();

    void setControlToGraundChips();

    void setGraundToControlChips();

    void setAllToGraundChips();

    void setAll6ToCheckChips();

    void setCorrectionParams_forControlChips();

    void setCorrectionParams( bool, const TCorrectionParams & );

    void setCorrectionParams( bool, const TCorrectionParams &, int, TDChip * );

    void set_flagMaskCloudyClear( bool flag ){ flagMaskCloudClear = flag; };

    TDChip::ChipType fChipType;

    TProjMapper* fPM;      // Данные проекции маски.

private:

    void loadDataMask( char fMaskFileDir[MAX_PATH] ) throw( TAccessExc );

    void iniChipList( char fBaseChipsFileDir[MAX_PATH] ) throw( TAccessExc, TParamExc );

    void buildMaskChip( TDChip * );

    bool calc_shift_Templet_1( uint8_t, long &, long & );

    bool calc_shift_Templet_2( uint8_t, long &, long & );

    bool calcCostFunction_Templet( uint8_t, long, long );

    double calc_shift_Morphological_1( uint8_t, long &, long &, bool, double &, long &, long & );

    double calc_shift_Morphological_2( uint8_t, double, long &, long &, bool, double &, long &, long & );

    double calcCostFunction_Morphological_MaxAverage_onContour( uint8_t pch, long shift_x, long shift_y );

    void hist_build_m( unsigned long *, uint8_t, long, long, long, long, short, short );

    void hist_build_m( unsigned long *, unsigned long *, uint8_t, long, long, long, long, short, short );

    void data_find_min_max( short *, short *, uint8_t, long, long, long, long );

    void hist_calc_q( short *, double, unsigned long *, short, short );

    bool check_GraundChipOnBeatenPixels( uint8_t pch, TDChip * chip );

    void removeAllChips();

    void calculateAutoGraundChip( uint8_t, TDChip * );
    void calculateHandGraundChip( uint8_t, TDChip * );


    TNOAAImageParams * fNIP;
    TIniSatParams * fISP;
    TStraightReferencer * fSR;
    TInverseReferencer * fIR;
    TCorrectionParams fCOP;

    TMaskFileHdr * fMFH;     // Нулевой блок файла маски.
    char * fDataMask;        // Данные маски всего региона.

    char * fCloudyMask;
    bool flagMaskCloudClear;

    short * calibrData[2];   // Данные AVHRR, калиброванные.
    bool fChannels_forCalcGCPs[2]; // По каким каналам идёт расчёт autoGCPs

    short maxPixValue[2];
    double ka[2], kb[2];     // Коэфициенты пересчета для альбедо и температуры.

    char * maskBuf;        // Буфер для маска фрагмента, с учётом её подробности.

    long fragmX1, fragmY1, fragmX2, fragmY2;  // Координаты чипа на изображении, включительно.
    long fragmWidth, fragmHeight;            // Размер фрагмента чипа в изображении в пикселах изображения.
    long maskWidth, maskHeight;             // Размер фрагмента чипа в маске в пикселах маски, с учётом её подробности.

    double landPixValue[2], seaPixValue[2];  // Значение уровней суши и моря для реперного участка изображения.

    double clearPixels, minCost00;

    long maxShiftLine, maxShiftColumn;
    long maskScale;

    double avg_largeChips_dX, avg_largeChips_dY;
    long quantity_largeChips;

    long fNumberRegionsforFiltrCloudy;

    TChipList * fChipList;
};

#endif
