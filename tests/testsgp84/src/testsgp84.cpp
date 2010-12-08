
/*
*/

#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <config.h>
#include <c_lib.hpp>
#include <y_util.hpp>
#include <orbmodel.hpp>

//#define PI      3.141592653589793238463
//#define RD      (180.0/PI)      /* Множитель перевода радиан в градусы. */
//#define DR      (PI/180.0)      /* Множитель перевода градусов в радианы. */

/*double dinpi( double arc ) {
	return (arc - floor(arc/(2.*PI))*2.*PI);
}
*/
double ovenAngle1( int year, double year_time ) ;
double ovenAngle2( int baseYear, double baseYearTime ) ;
double ovenAngle3( int baseYear, double baseYearTime ) ;

/* тестовая телеграмма */
char t_tlg[] = "\
1 88888U          80275.98708465  .00073094  13844-3  66816-4 0     8\r\n\
2 88888  72.8435 115.9689 0086731  52.6988 110.5714 16.05824518   105\r\n\
";
/*
char t_tlg[] = "\
1 88888U          80275.98708465  .00073094  13844-3  66816-4 0     8\r\n\
2 88888  72.8435 115.9689 0086731  52.6988 110.5714 16.05824518   105\r\n\
";
*/

/* эталонные координаты и скорости для заданной телеграммы */
double* tx;
double* ty;
double* tz;
double* txdot;
double* tydot;
double* tzdot;

/* эталонные координаты и скорости SGP8 для заданной телеграммы */
double tx_sgp8[5] = { 2328.87265015, 2456.04577637, 2567.68383789, 2663.49508667, 2743.29238892 };
double ty_sgp8[5] = { -5995.21289063, -6071.90490722, -6112.40881348, -6115.18182373, -6078.90783691 };
double tz_sgp8[5] = { 1720.04884338, 1222.84086609,713.29282379, 194.62816810, -329.73434067 };
double txdot_sgp8[5] = { 2.91210661, 2.67936245, 2.43992555, 2.19525236, 1.94680957 };
double tydot_sgp8[5] = { -0.98353850, -0.44820847, 0.09893919, 0.65453661, 1.21500109 };
double tzdot_sgp8[5] = { -7.09081554, -7.22888553, -7.32018769, -7.36308974, -7.35625595 };

/* эталонные координаты и скорости SGP4 для заданной телеграммы */
double tx_sgp4[5] = { 2328.97048951,2456.10705566,2567.56195068,2663.09078980,2742.55133057};
double ty_sgp4[5] = { -5995.22076416,-6071.93853760,-6112.50384522,-6115.48229980,-6079.67144775};
double tz_sgp4[5] = { 1719.97067261,1222.89727783, 713.96397400, 196.39640427,-326.38095856};
double txdot_sgp4[5] = { 2.91207230,2.67938992,2.44024599,2.19611958,1.94850229};
double tydot_sgp4[5] = { -0.98341546,-0.44829041, 0.09810869, 0.65241995, 1.21106251};
double tzdot_sgp4[5] = { -7.09081703,-7.22879231,-7.31995916,-7.36282432,-7.35619372};

/* массивы отклонений от эталона */
double dx[5], dy[5], dz[5], dxdot[5], dydot[5], dzdot[5];

int main(int argc, char** argv){

	if(argc < 2){
		printf("%s <sgp4|sgp8|oven>\n",argv[0]);
		exit(0);
	}
	/* переменные орбитальной модели */
//	TIniSatParams isp;
//	TSatParams sat;

	if(strcasecmp("oven",argv[1])==0){
		int year = 2009;
		double year_date[2] = { 012.49515798, 350.4 };
		for(int i = 0; i < 2; i++)
			printf(" %lf \n %lf \n %lf \n ", ovenAngle1(year,year_date[i] - 1.), ovenAngle2(year,year_date[i]), ovenAngle3(year,year_date[i]));
		exit(0);
	}else if (strcasecmp("sgp4",argv[1])==0 || strcasecmp("sgp8",argv[1])==0){

		/* заголовок выходного файла */
		//TBlk0 b;

		TIniSatParams isp(t_tlg);
		/* орбитальные параметры из 0-блока */
		printf ( "\nReference number: %d\n", isp.fRevNum );
		printf ( "Ephem set number: %d\n", isp.fSetNum );
		//printf ( "Ephem type: %d\n", isp.ephem );
		printf ( "Year: %d\n", isp.fEpochYear );
		printf ( "Day: %12.8lf\n", isp.fEpochYearTime );
		printf ( "Average motion n0 ( rad/s ): %12.10lf ( %11.9lf 1/day )\n", isp.n0, isp.n0*4.0*RD );
		printf ( "BSTAR Drag term: %12.10lf\n", isp.bstar );
		printf ( "Orbit declination i0 ( rad ): %12.10lf ( %8.4lf deg )\n", isp.i0, isp.i0*RD );
		printf ( "Right accending raan ( rad ): %12.10lf ( %8.4lf deg )\n", isp.raan, isp.raan*RD );
		printf ( "Excentricity e0: %9.7lf\n", isp.e0 );
		printf ( "Perigelium arg w0 ( rad ): %12.10lf ( %8.4lf deg )\n", isp.w0, isp.w0*RD );
		printf ( "Average anomaly m0 ( rad ): %12.10lf ( %8.4lf deg )\n\n", isp.m0, isp.m0*RD );

	/* инициализация модели, по умолчанию SGP8  */
		TOrbitalModel om( isp, isp.fEpochYear, isp.fEpochYearTime );

		int i,j;
		if (strcasecmp("sgp4",argv[1])==0){
			printf("select sgp4\n");
			tx = tx_sgp4;
			ty = ty_sgp4;
			tz = tz_sgp4;
			txdot = txdot_sgp4;
			tydot = tydot_sgp4;
			tzdot = tzdot_sgp4;
			om.setModel(SGP4);
		} else {
			printf("select sgp8\n");
			tx = tx_sgp8;
			ty = ty_sgp8;
			tz = tz_sgp8;
			txdot = txdot_sgp8;
			tydot = tydot_sgp8;
			tzdot = tzdot_sgp8;
			om.setModel(SGP8);
		}

		printf(" Version %ld\n fTime %lf\n fTBUSTime %lf\n roll %lf\n pitch %lf\n yaw %lf\n dn0 %lf\n di0 %lf\n draan %lf\n de0 %lf\n dw0 %lf\n dm0 %lf\n",
			om.cop.fVersion,
			// коррекция времени начала сеанса
			om.cop.fTime,       // В секундах.
			om.cop.fTBUSTime,   // В секундах.
			// коррекция ориентации платформы спутника, в радианах
			om.cop.roll,
			om.cop.pitch,
			om.cop.yaw,
			// коррекция прогнозных орбитальных параметров
			om.cop.dn0,     // среднее движение
			om.cop.di0,     // наклонение
			om.cop.draan0,  // восходящий узел
			om.cop.de0,     // эксцентриситет
			om.cop.dw0,     // аргумент перигея
			om.cop.dm0     // средняя аномалия
		);

		for ( j = 0, i = 0; j < 1441; j += 360, i++ ) {
			om.model( ((double)j)/1440 );
			printf ( "%4d %13.7lf %13.7lf %13.7lf %10.7lf %10.7lf %10.7lf\n",
				j, om.r[0], om.r[1], om.r[2], om.v[0]/60.0, om.v[1]/60.0, om.v[2]/60.0 );

			/* находим отклонения от эталонных значений */
			dx[i] = om.r[0] - tx[i];
			dy[i] = om.r[1] - ty[i];
			dz[i] = om.r[2] - tz[i];
			dxdot[i] = om.v[0]/60.0 - txdot[i];
			dydot[i] = om.v[1]/60.0 - tydot[i];
			dzdot[i] = om.v[2]/60.0 - tzdot[i];
		}

		/* распечатываем отклонение от эталонных значений */
		printf ( "\nОтклонения от эталона:\n" );
		for ( i = 0; i < 5; i++ ) {
			printf ( "%4d %11.8lf %11.8lf %11.8lf %11.8lf %11.8lf %11.8lf\n",
				i*360, dx[i], dy[i], dz[i], dxdot[i], dydot[i], dzdot[i] );
		}

		return 0;
	} else {
		printf("%s <sgp4|sgp8|oven>\n",argv[0]);
		exit(0);
	}
}

double ovenAngle1( int year, double year_time ) {
	int n = (int)year_time;
	int j = year - 1983;
	double day_frac = year_time - double(n);
	n += (j+2)/4;
	double df = double( 365*j + n );
	double dd = dinpi( 0.17202791e-1 * df ) - 7.67945e-5*sin( dinpi( 1.638168 - dinpi( 9.242202558e-4 * df ) - 24.0*3.8397e-5*day_frac ) );
	return dinpi( dd + 1.746778922 + 24.0*0.262516145*day_frac );
}

double ovenAngle2( int baseYear, double baseYearTime ) {
	double day_frac, n, df, dd, d1;
	int j;
	day_frac = modf( baseYearTime, &n );
	j = baseYear - 1983;
	n += (double)((j+2)/4) - 1.;
	df = (double)j*365. + n;
	dd = dinpi( 9.242202558e-4 * df );
	d1 = dinpi( 1.638168 - dd - 24.0*3.8397e-5*day_frac );
	dd = dinpi( 0.17202791e-1 * df ) - 7.67945e-5*sin(d1);
	return  dinpi( dd + 1.746778922 + 24.0*0.262516145*day_frac );
}

double ovenAngle3( int baseYear, double baseYearTime ) {
	double dd = mjd1 ((int)baseYear, baseYearTime );
	return gmst( dd ) * 2.0 * PI;
}

