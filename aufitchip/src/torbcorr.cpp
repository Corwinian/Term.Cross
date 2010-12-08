/*-------------------------------------------------------------------------
   torbcorr.cpp
-------------------------------------------------------------------------*/
#include <tc_config.h>
#include <nrc.hpp>
#include <aufit_mask.hpp>
#include <torbcorr.hpp>
TOrbitCorrector * oc;
int gcp_num;
double * lon0, * lat0, * centr_gcp_x, * centr_gcp_y;
int var_flags[COR_PARAM_NUM];// = { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
double var_weights[COR_PARAM_NUM] = { 1., 1., 1e-2, 1e-2, 1e-2, 1e-7, 1e-3, 1e-3, 1e-6, 1e-3, 1e-3 };
double var_bounds[COR_PARAM_NUM] = { 3., 3., 0.02, 0.02, 0.02, .0000001, .001, .001, .000001, .001, .001 };
bool fAutoHandFlag = true;

double minimized_fun( double * x ) ;

TOrbitCorrector::TOrbitCorrector( const TIniSatParams& isp, const TNOAAImageParams& nip, const TCorrectionParams &cop ) :
fISP( isp ),
fNIP( nip ),
fCOP( cop ),
beta0_y(.0), beta1_y(.0), SS_y(.0), difX_y(.0),
SR(.0), sumX2_y(0), SS_x(.0),
stand_dev_est_beta0_y(.0), stand_dev_est_beta1_y(.0),
error_roll(.0), error_pitch(.0), error_yaw(.0),
cor_avg_dX(.0), cor_avg_dY (.0), lr_yaw(.0)
{
	if( fCOP.fVersion == 0 )
		fCOP.fVersion = 2; // если коррекция файла не производилась, используем для коррекции SGP4; иначе пользуемся той моделью, которая была
	oc = this;
}


const TCorrectionParams & TOrbitCorrector::correction() const {
	return fCOP;
}


void TOrbitCorrector::calcCorrection( bool flag, TAufitChipMask & fACM, int min_method, int * min_params ) {
	int i=0, j;
	fAutoHandFlag = flag;

	memcpy( var_flags, min_params, sizeof( int ) * COR_PARAM_NUM );

	gcp_num = 0;

	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip )
			gcp_num++;
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
			gcp_num++;
	}
	if( gcp_num == 0 )
		return;

	lon0 = new double [ gcp_num ];
	lat0 = new double [ gcp_num ];
	centr_gcp_x = new double [ gcp_num ];
	centr_gcp_y = new double [ gcp_num ];

	// Перевод опорных точек в географические координаты.
	TStraightReferencer * sr = new TStraightReferencer( fISP, fNIP, fCOP );
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( fAutoHandFlag ) {
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				centr_gcp_x[i] = double( chip->X1 + chip->ChipSize().x()/2 );
				centr_gcp_y[i] = double( chip->Y1 + chip->ChipSize().y()/2 );
				sr->xy2ll( centr_gcp_x[i] - chip->dX[0], centr_gcp_y[i] - chip->dY[0], lon0 + i, lat0 + i );
				i++;
			}
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				centr_gcp_x[i] = double( chip->X1 + chip->ChipSize().x()/2 );
				centr_gcp_y[i] = double( chip->Y1 + chip->ChipSize().y()/2 );
				sr->xy2ll( centr_gcp_x[i] - chip->dX[1], centr_gcp_y[i] - chip->dY[1], lon0 + i, lat0 + i );
				i++;
			}
		} else {
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				centr_gcp_x[i] = double( chip->X1 + chip->ChipSize().x()/2 );
				centr_gcp_y[i] = double( chip->Y1 + chip->ChipSize().y()/2 );
				sr->xy2ll( centr_gcp_x[i] - chip->dhX[0], centr_gcp_y[i] - chip->dhY[0], lon0 + i, lat0 + i );
				i++;
			}
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				centr_gcp_x[i] = double( chip->X1 + chip->ChipSize().x()/2 );
				centr_gcp_y[i] = double( chip->Y1 + chip->ChipSize().y()/2 );
				sr->xy2ll(centr_gcp_x[i] - chip->dhX[1], centr_gcp_y[i] - chip->dhY[1], lon0 + i, lat0 + i );
				i++;
			}
		}
	}
	delete sr;

	//-----------------------------------------------------------------------------
	// собственно минимизация
	//-----------------------------------------------------------------------------
	double x[COR_PARAM_NUM];
	memset( x, 0, sizeof( x ) );
	double eps = 1.e-8;
	int iter = 2000;
	int fun_c;
	double fx;
	int nvar = 0;

	// подсчитываем число выбранных для минимизации переменных
	for( i = 0; i < COR_PARAM_NUM; i++ ) {
		nvar += min_params[i];
	}

	if( min_method == 1 ) {      // Powell
		double xi[COR_PARAM_NUM * COR_PARAM_NUM];   // начальное направление поиска
		memset( xi, 0, sizeof( xi ) );
		for( i = 0; i < nvar; i++ ) {
			xi[i*nvar + i] = .1;
		}
		powell( x, xi, nvar, eps, &iter, &fx, minimized_fun );
	}

	if( min_method == 2 ) {      // Nelder-Mead
		min_nelder_mead_1( nvar, minimized_fun, x, .1, &eps, &iter, &fun_c, &fx, 0 );
	}

	// "раздергиваем" вектор x по выбранным для минимизации параметрам
	TCorrectionParamsAsArray * pcor = (TCorrectionParamsAsArray *)&fCOP;
	j = 0;
	for( i = 0; i < COR_PARAM_NUM; i++ ) {
		if( min_params[i] )
			pcor->params[i] += var_weights[i] * x[j++];
	}

	delete [] lat0;
	delete [] lon0;

	delete [] centr_gcp_x;
	delete [] centr_gcp_y;
}


double minimized_fun( double * x ) {
	TStraightReferencer * sr;
	TCorrectionParamsAsArray * pcor;
	double p[COR_PARAM_NUM];
	double lon, lat, dlon, dlat, d = 0.;
	int i, j;

	// "раздергиваем" вектор x по тем параметрам коррекции, которые должны участвовать в минимизации
	j = 0;
	for( i = 0; i < COR_PARAM_NUM; i++ ) {
		p[i] = var_flags[i] ? var_weights[i] * x[j++] : 0.;
	}

	pcor = (TCorrectionParamsAsArray *) &oc->fCOP;
	for( i = 0; i < COR_PARAM_NUM; i++ )
		pcor->params[i] += p[i];

	sr = new TStraightReferencer( oc->fISP, oc->fNIP, oc->fCOP );
	// основной цикл вычисления функции
	for( i = 0; i < gcp_num; i++ ) {
		sr->xy2ll( centr_gcp_x[i], centr_gcp_y[i], &lon, &lat );
		dlon = (lon0[i] - lon) * cos( lat );
		dlat = lat0[i] - lat;
		d += dlon*dlon + dlat*dlat;
	}
	delete sr;

	for( i = 0; i < COR_PARAM_NUM; i++ )
		pcor->params[i] -= p[i];

	d /= double( gcp_num );

	// штрафные добавки за переход за границы допустимых значений параметров
	for( i = 0; i < COR_PARAM_NUM; i++ ) {
		if( fabs( p[i] ) > var_bounds[i] ) {
			d += pow( fabs( p[i] ) - var_bounds[i], 2. );
		}
	}

	return d;
}


void TOrbitCorrector::calcCorrectionAngles_onLinearRegression( TAufitChipMask & fACM, bool flag_yaw ) {
	gcp_num = 0;

	//-----------------------------------------------------------------------------
	// собственно получение коэффициетов линейной регссии
	//-----------------------------------------------------------------------------
	double lr_x, lr_y, sum_x = .0, summ_x = .0, sum_y = .0, sum_x2 = .0, sum_xy = .0;
	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
			lr_y = -1. * chip->dY[0] - cor_avg_dY;
			sum_x += lr_x;
			sum_y += lr_y;
			sum_x2 += lr_x * lr_x;
			sum_xy += lr_x * lr_y;
			gcp_num++;
			summ_x += -1. * chip->dX[0] - cor_avg_dX;
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
			lr_y = -1. * chip->dY[1] - cor_avg_dY;
			sum_x += lr_x;
			sum_y += lr_y;
			sum_x2 += lr_x * lr_x;
			sum_xy += lr_x * lr_y;
			gcp_num++;
			summ_x += -1. * chip->dX[1] - cor_avg_dX;
		}
	}

	if( lr_yaw != 0. ) {
		beta1_y = lr_yaw;
		beta0_y = ( sum_y - beta1_y * sum_x ) / double(gcp_num);
	} else {
		beta1_y = ( gcp_num * sum_xy - sum_x * sum_y ) / ( gcp_num * sum_x2 - sum_x * sum_x );
		beta0_y = ( sum_y - beta1_y * sum_x ) / double(gcp_num);
	}
	sumX2_y = (unsigned long)sum_x2;

	int fBaseYear = fNIP.fYear;
	double fBaseYearTime = fNIP.fYearTime; // отодвигаемся назад на время одного скана.
	if( fBaseYearTime < 0 ) {               // на всякий случай проверяем переход назад через границу года.
		fBaseYearTime += isLeapYear( --fBaseYear ) ? 365. : 364.;
	}
	TOrbitalModel om( fISP, fBaseYear, fBaseYearTime, TCorrectionParams( 2 ) );

	om.model( ( fNIP.fScans/2. + sum_y/gcp_num ) / ( 360. * 1440. ) );

	double ER = 6371.11;
	SR = om.R - ER;

	fCOP.roll = (1.1*summ_x) / (SR * gcp_num);
	if( flag_yaw || fabs(beta1_y) > 0.02 ) {
		fCOP.yaw = .0;
		fCOP.pitch = (1.1*sum_y) / (SR * gcp_num);
	} else {
		fCOP.pitch = (1.1*beta0_y) / SR;
		fCOP.yaw = beta1_y;
	}
}


void TOrbitCorrector::calcCorrection_onLinearRegressionCoefficients( bool flag, bool flag_precision, TAufitChipMask & fACM, bool flag_yaw ) {
	fAutoHandFlag = flag;
	gcp_num = 0;

	//-----------------------------------------------------------------------------
	// собственно получение коэффициетов линейной регссии
	//-----------------------------------------------------------------------------
	double lr_x, lr_y, sum_x = .0, summ_x = .0, sum_y = .0, sum_x2 = .0, sum_xy = .0;
	TCLCursor cursor = fACM.createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( fAutoHandFlag ) {
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
				lr_y = -1. * chip->dY[0] - cor_avg_dY;
				sum_x += lr_x;
				sum_y += lr_y;
				sum_x2 += lr_x * lr_x;
				sum_xy += lr_x * lr_y;
				gcp_num++;
				summ_x += -1. * chip->dX[0] - cor_avg_dX;
			}
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
				lr_y = -1. * chip->dY[1] - cor_avg_dY;
				sum_x += lr_x;
				sum_y += lr_y;
				sum_x2 += lr_x * lr_x;
				sum_xy += lr_x * lr_y;
				gcp_num++;
				summ_x += -1. * chip->dX[1] - cor_avg_dX;
			}
		} else {
			if( chip->fChipType[0] == TDChip::GraundChip ) {
				lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
				lr_y = -1. * chip->dhY[0] - cor_avg_dY;
				sum_x += lr_x;
				sum_y += lr_y;
				sum_x2 += lr_x * lr_x;
				sum_xy += lr_x * lr_y;
				gcp_num++;
				summ_x += -1. * chip->dhX[0] - cor_avg_dX;
			}
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
				lr_y = -1. * chip->dhY[1] - cor_avg_dY;
				sum_x += lr_x;
				sum_y += lr_y;
				sum_x2 += lr_x * lr_x;
				sum_xy += lr_x * lr_y;
				gcp_num++;
				summ_x += -1. * chip->dhX[1] - cor_avg_dX;
			}
		}
	}

	if( lr_yaw != 0. ) {
		beta1_y = lr_yaw;
		beta0_y = ( sum_y - beta1_y * sum_x ) / double(gcp_num);
	} else {
		beta1_y = ( gcp_num * sum_xy - sum_x * sum_y ) / ( gcp_num * sum_x2 - sum_x * sum_x );
		beta0_y = ( sum_y - beta1_y * sum_x ) / double(gcp_num);
	}
	sumX2_y = (unsigned long)sum_x2;

	int fBaseYear = fNIP.fYear;
	double fBaseYearTime = fNIP.fYearTime; // - 1./(360.*1440.);   // отодвигаемся назад на время одного скана
	if( fBaseYearTime < 0 ) {                            // на всякий случай проверяем переход назад через границу года
		fBaseYearTime += isLeapYear( --fBaseYear ) ? 365. : 364.;
	}
	TOrbitalModel om( fISP, fBaseYear, fBaseYearTime, TCorrectionParams() );

	om.model( ( fNIP.fScans/2. + sum_y/gcp_num ) / ( 360. * 1440. ) );

	//    double lon, lat;
	//    sr->xy2ll( fNIP.fScans/2. + summ_y/n, 1023.5 + summ_x/n, &lon, &lat );

	double ER = 6371.11;
	SR = om.R - ER;

	fCOP.roll = (1.1*summ_x) / (SR * gcp_num);
	if( ( gcp_num == 1) || ( gcp_num == 2 && fACM.duplicateOfGraundChips() ) || flag_yaw || fabs(beta1_y) > 0.015 ) {
		fCOP.yaw = .0;
		fCOP.pitch = (1.1*sum_y) / (SR * gcp_num);
	} else {
		fCOP.pitch = (1.1*beta0_y) / SR;
		fCOP.yaw = beta1_y;
	}

	if( flag_precision ) {
		if( fabs(beta1_y) > 0.015 ) {
			fCOP.yaw = .0;
			fCOP.pitch = (1.1*sum_y) / (SR * gcp_num);
		}
		lr_x = .0;
		lr_y = .0;
		double std_dev_dY = .0;
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( fAutoHandFlag ) {
				if( chip->fChipType[0] == TDChip::GraundChip ) {
					lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chip->dY[0] - cor_avg_dY;
					SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
					difX_y += pow( lr_x - sum_x/gcp_num, 2. );
					std_dev_dY += pow( chip->dY[0] + cor_avg_dY, 2. );
					SS_x += pow( chip->dX[0] + cor_avg_dX, 2. );
				}
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
					lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chip->dY[1] - cor_avg_dY;
					SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
					difX_y += pow( lr_x - sum_x/gcp_num, 2. );
					std_dev_dY += pow( chip->dY[1] + cor_avg_dY, 2. );
					SS_x += pow( chip->dX[1] + cor_avg_dX, 2. );
				}
			} else {
				if( chip->fChipType[0] == TDChip::GraundChip ) {
					lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chip->dhY[0] - cor_avg_dY;
					SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
					difX_y += pow( lr_x - sum_x/gcp_num, 2. );
					std_dev_dY += pow( chip->dhY[0] + cor_avg_dY, 2. );
					SS_x += pow( chip->dhX[0] + cor_avg_dX, 2. );
				}
				if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
					lr_x = double(chip->X1 + chip->ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chip->dhY[1] - cor_avg_dY;
					SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
					difX_y += pow( lr_x - sum_x/gcp_num, 2. );
					std_dev_dY += pow( chip->dhY[1] + cor_avg_dY, 2. );
					SS_x += pow( chip->dhX[1] + cor_avg_dX, 2. );
				}
			}
		}

		if( gcp_num > 2 ) {
			SS_y /= gcp_num - 2;
			stand_dev_est_beta0_y = pow( (sumX2_y * SS_y) / (gcp_num * difX_y), .5 );
			stand_dev_est_beta1_y = pow( (SS_y / difX_y), .5 );
			if( fabs(beta1_y) <= 0.015 ) {
				error_yaw = stand_dev_est_beta1_y;
				error_pitch = (stand_dev_est_beta0_y*1.1) / SR;
			}
		}
		if( gcp_num > 1 ) {
			SS_x = pow( (SS_x - (summ_x*summ_x)/gcp_num)/(gcp_num - 1), .5);
			error_roll = (SS_x*1.1) / SR;
			if( fCOP.yaw != beta1_y )
				error_pitch = (pow( (std_dev_dY - (sum_y*sum_y)/gcp_num)/(gcp_num - 1), .5)*1.1) / SR;
		}
	}
}

void TOrbitCorrector::calcCorrection_onLinearRegressionCoefficients( bool flag, bool flag_precision, int n, TDChip * chips, TAufitChipMask & fACM, bool flag_yaw ) {
	fAutoHandFlag = flag;
	gcp_num = 0;

	//-----------------------------------------------------------------------------
	// собственно получение коэффициетов линейной регссии
	//-----------------------------------------------------------------------------
	double lr_x, lr_y, sum_x = .0, summ_x = .0, sum_y = .0, sum_x2 = .0, sum_xy = .0;
	for( int i = 0; i < n; i++ ) {
		if( fAutoHandFlag ) {
			if( chips[i].fChipType[0] == TDChip::GraundChip ) {
				lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
				lr_y = -1. * chips[i].dY[0] - cor_avg_dY;
				sum_x += lr_x;
				sum_y += lr_y;
				sum_x2 += lr_x * lr_x;
				sum_xy += lr_x * lr_y;
				gcp_num++;
				summ_x += -1. * chips[i].dX[0] - cor_avg_dX;
			} else {
				if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
					lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chips[i].dY[1] - cor_avg_dY;
					sum_x += lr_x;
					sum_y += lr_y;
					sum_x2 += lr_x * lr_x;
					sum_xy += lr_x * lr_y;
					gcp_num++;
					summ_x += -1. * chips[i].dX[1] - cor_avg_dX;
				}
			}
		} else {
			if( chips[i].fChipType[0] == TDChip::GraundChip ) {
				lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
				lr_y = -1. * chips[i].dhY[0] - cor_avg_dY;
				sum_x += lr_x;
				sum_y += lr_y;
				sum_x2 += lr_x * lr_x;
				sum_xy += lr_x * lr_y;
				gcp_num++;
				summ_x += -1. * chips[i].dhX[0] - cor_avg_dX;
			} else {
				if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
					lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chips[i].dhY[1] - cor_avg_dY;
					sum_x += lr_x;
					sum_y += lr_y;
					sum_x2 += lr_x * lr_x;
					sum_xy += lr_x * lr_y;
					gcp_num++;
					summ_x += -1. * chips[i].dhX[1] - cor_avg_dX;
				}
			}
		}
	}

	if( lr_yaw != 0. ) {
		beta1_y = lr_yaw;
		beta0_y = ( sum_y - beta1_y * sum_x ) / double(gcp_num);
	} else {
		beta1_y = ( gcp_num * sum_xy - sum_x * sum_y ) / ( gcp_num * sum_x2 - sum_x * sum_x );
		beta0_y = ( sum_y - beta1_y * sum_x ) / double(gcp_num);
	}
	sumX2_y = (unsigned long)sum_x2;

	int fBaseYear = fNIP.fYear;
	double fBaseYearTime = fNIP.fYearTime; // - 1./(360.*1440.);   // отодвигаемся назад на время одного скана
	if( fBaseYearTime < 0 ) {                            // на всякий случай проверяем переход назад через границу года
		fBaseYearTime += isLeapYear( --fBaseYear ) ? 365. : 364.;
	}
	TOrbitalModel om( fISP, fBaseYear, fBaseYearTime, TCorrectionParams() );

	om.model( ( fNIP.fScans/2. + sum_y/gcp_num ) / ( 360. * 1440. ) );

	//    double lon, lat;
	//    sr->xy2ll( fNIP.fScans/2. + summ_y/n, 1023.5 + summ_x/n, &lon, &lat );

	double ER = 6371.11;
	SR = om.R - ER;

	fCOP.roll = (1.1*summ_x) / (SR * gcp_num);
	if( ( gcp_num == 1) || ( gcp_num == 2 && fACM.duplicateOfGraundChips( n, chips ) ) || flag_yaw || fabs(beta1_y) > 0.015 ) {
		fCOP.yaw = .0;
		fCOP.pitch = (1.1*sum_y) / (SR * gcp_num);
	} else {
		fCOP.pitch = (1.1*beta0_y) / SR;
		fCOP.yaw = beta1_y;
	}

	if( flag_precision ) {
		if( fabs(beta1_y) > 0.015 ) {
			fCOP.yaw = .0;
			fCOP.pitch = (1.1*sum_y) / (SR * gcp_num);
		}
		lr_x = .0;
		lr_y = .0;
		double std_dev_dY = .0;
		for( int i = 0; i < n; i++ ) {
			if( fAutoHandFlag ) {
				if( chips[i].fChipType[0] == TDChip::GraundChip ) {
					lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chips[i].dY[0] - cor_avg_dY;
					SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
					difX_y += pow( lr_x - sum_x/gcp_num, 2. );
					std_dev_dY += pow( chips[i].dY[0] + cor_avg_dY, 2. );
					SS_x += pow( chips[i].dX[0] + cor_avg_dX, 2. );
				} else {
					if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
						lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
						lr_y = -1. * chips[i].dY[1] - cor_avg_dY;
						SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
						difX_y += pow( lr_x - sum_x/gcp_num, 2. );
						std_dev_dY += pow( chips[i].dY[1] + cor_avg_dY, 2. );
						SS_x += pow( chips[i].dX[1] + cor_avg_dX, 2. );
					}
				}
			} else {
				if( chips[i].fChipType[0] == TDChip::GraundChip ) {
					lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
					lr_y = -1. * chips[i].dhY[0] - cor_avg_dY;
					SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
					difX_y += pow( lr_x - sum_x/gcp_num, 2. );
					std_dev_dY += pow( chips[i].dhY[0] + cor_avg_dY, 2. );
					SS_x += pow( chips[i].dhX[0] + cor_avg_dX, 2. );
				} else {
					if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
						lr_x = double(chips[i].X1 + chips[i].ChipSize().x()/2) - 1023.5;
						lr_y = -1. * chips[i].dhY[1] - cor_avg_dY;
						SS_y += pow( (lr_y - (beta1_y * lr_x) - beta0_y), 2. );
						difX_y += pow( lr_x - sum_x/gcp_num, 2. );
						std_dev_dY += pow( chips[i].dhY[1] + cor_avg_dY, 2. );
						SS_x += pow( chips[i].dhX[1] + cor_avg_dX, 2. );
					}
				}
			}
		}

		if( gcp_num > 2 ) {
			SS_y /= gcp_num - 2;
			stand_dev_est_beta0_y = pow( (sumX2_y * SS_y) / (gcp_num * difX_y), .5 );
			stand_dev_est_beta1_y = pow( (SS_y / difX_y), .5 );
			if( fabs(beta1_y) <= 0.015 ) {
				error_yaw = stand_dev_est_beta1_y;
				error_pitch = (stand_dev_est_beta0_y*1.1) / SR;
			}
		}
		if( gcp_num > 1 ) {
			SS_x = pow( (SS_x - (summ_x*summ_x)/gcp_num)/(gcp_num - 1), .5);
			error_roll = (SS_x*1.1) / SR;
			if( fCOP.yaw != beta1_y )
				error_pitch = (pow( (std_dev_dY - (sum_y*sum_y)/gcp_num)/(gcp_num - 1), .5)*1.1) / SR;
		}
	}
}
