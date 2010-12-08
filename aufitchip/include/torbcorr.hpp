
/*-------------------------------------------------------------------------
   torbcorr.hpp
-------------------------------------------------------------------------*/

#ifndef _TORBCORR_HPP_
    #define _TORBCORR_HPP_

#include <tc_config.h>
#include <orbmodel.hpp>

class TOrbitCorrector {
    friend double minimized_fun( double * );
public:
    TOrbitCorrector( const TIniSatParams &, const TNOAAImageParams &, const TCorrectionParams & );

/*
    Вычисление коррекции производится кумулятивно по отношению к коррекции, указанной при создании объекта.

    Параметры:
    min_method      1 - Powell, 2 - Nelder-Mead
    min_params      Указатель на массив размером COR_PARAM_NUM (см. orbmodel.hpp) - флаги параметров, участвующих в минимизации.
*/
    void calcCorrection( bool flag, TAufitChipMask & fACM, int min_method, int * min_params );

    void calcCorrectionAngles_onLinearRegression( TAufitChipMask & fACM, bool flag_yaw );

    void calcCorrection_onLinearRegressionCoefficients( bool flag, bool flag_precision, TAufitChipMask & fACM, bool flag_yaw );

    void calcCorrection_onLinearRegressionCoefficients( bool flag, bool flag_precision, int n, TDChip * chips, TAufitChipMask & fACM, bool flag_yaw );

    const TCorrectionParams & correction() const;

    double SR;
    double beta0_y, beta1_y, SS_y, difX_y;
    unsigned long sumX2_y;
    double stand_dev_est_beta0_y, stand_dev_est_beta1_y;
    double SS_x;
    double error_roll, error_pitch, error_yaw;

    double cor_avg_dX, cor_avg_dY, lr_yaw;
private:
    TIniSatParams fISP;
    TNOAAImageParams fNIP;
    TCorrectionParams fCOP;
};

#endif
