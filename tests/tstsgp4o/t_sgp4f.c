/*---------------------------------------------------------------------
	T_SGP4F.C
	Программа тестирует оригинальный код модели SGP4, переписанный
	с FORTRAN'а на C. Как и в программе T_SGP8F, тестирование производится
	для некоторой модельной телеграммы.
	Date: 8 august 2006
---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "f34_uni.h"

/* определения из orbmod2.h */
#define ORB_PI      3.141592653589793238463
#define RD      (180.0/ORB_PI)      /* Множитель перевода радиан в градусы. */
#define DR      (ORB_PI/180.0)      /* Множитель перевода градусов в радианы. */
#define CENTURY	1900

/* константы ( входят в COMMON/C1/ ) */
#define	DE2RA	0.174532925E-1
#define	E6A		1.E-8
//#define	PI		3.14159265
//#define	PIO2	1.57079633
//#define	TWOPI	6.2831853
//#define	X3PIO2	4.71238898
//#define	TOTHRD	0.66666667
#define	PI		ORB_PI
#define PIO2	(PI/2.0)
#define	TWOPI	(PI*2.0)
#define X3PIO2	(PI*1.5)
#define	TW_TH	(2.0/3.0)

#define	Ke		0.743669161E-1
#define	XJ2		1.082616E-3
#define	XJ3		(-0.253881E-5)
#define	XJ4		(-1.65597E-6)
#define	AE		1.0
#define AE2		(AE*AE)
#define AE4		(AE2*AE2)
#define CK2		(0.5*XJ2*AE2)
#define A3COF	(-XJ3/CK2*AE2*AE)
#define CK4		(-0.375*XJ4*AE4)
#define	Ea		6378.135
#define	XMNPDA	1440.0
#define	Q0		120.0
#define	S0		78.0
//#define	RHO		0.15696615

/* прототипы функций */
extern void SGP4F( double );
extern double ACTAN ( double, double );
extern double FMOD2P ( double );
extern void norad_time ( short*, double*, char* p );
extern void TBlk0_ini ( struct TBlk0_uni* b, char* p );
extern double econv ( char* p, char* s );

/* заголовок выходного файла */
struct TBlk0_uni b;

/* модельная телеграмма */
char t_tlg[] = "\
1 88888U          80275.98708465  .00073094  13844-3  66816-4 0     8\r\n\
2 88888  72.8435 115.9689 0086731  52.6988 110.5714 16.05824518   105\r\n\
";

/* переменные блока COMMON/E1/ */
double c_m0;
double c_raan0;
double c_w0;
double c_e0;
double c_i0;
double c_n0;
double XNDT20;
double XNDD60;
double isp_bstar;
double X;
double Y;
double Z;
double XDOT;
double YDOT;
double ZDOT;
double EPOCH;
double DS50;
double Q0MS2T;
double S;

int IFLAG;

/* эталонные координаты и скорости для заданной телеграммы */
double tx[5] = { 2328.97048951,2456.10705566,2567.56195068,2663.09078980,2742.55133057};
double ty[5] = { -5995.22076416,-6071.93853760,-6112.50384522,-6115.48229980,-6079.67144775};
double tz[5] = { 1719.97067261,1222.89727783, 713.96397400, 196.39640427,-326.38095856};
double txdot[5] = { 2.91207230,2.67938992,2.44024599,2.19611958,1.94850229};
double tydot[5] = { -0.98341546,-0.44829041, 0.09810869, 0.65241995, 1.21106251};
double tzdot[5] = { -7.09081703,-7.22879231,-7.31995916,-7.36282432,-7.35619372};

/* массивы отклонений от эталона */
double dx[5], dy[5], dz[5], dxdot[5], dydot[5], dzdot[5];

int main()
{
	int i;
	double TS, TF, DELT, TSINCE;

	/* инициализируем поля norad в 0-блоке */
	memset ((void*)(&b), 0, sizeof( struct TBlk0_uni ));
	b.formatType = 0xff;
	TBlk0_ini ( &b, t_tlg );

	for ( i = 0; i < strlen ( t_tlg ); i++ ) {
		if ( t_tlg[i] == 0x0d ) continue;
		else if ( t_tlg[i] == 0x0a ) printf ( "\n" );
		else printf ( "%1c", t_tlg[i] );
	}

	/* орбитальные параметры из 0-блока */
	printf ( "\nReference number: %ld\n", b.ref_num );
	printf ( "Ephem set number: %d\n", b.set_num );
	printf ( "Ephem type: %d\n", b.ephem );
	printf ( "Year: %d\n", b.ep_year );
	printf ( "Day: %12.8lf\n", b.ep_day );
	printf ( "Average motion n0 ( rad/s ): %12.10lf ( %11.9lf 1/day )\n", b.n0, b.n0*4.0*RD );
	printf ( "BSTAR Drag term: %12.10lf\n", b.bstar );
	printf ( "Orbit declination i0 ( rad ): %12.10lf ( %8.4lf deg )\n", b.i0, b.i0*RD );
	printf ( "Right accending raan ( rad ): %12.10lf ( %8.4lf deg )\n", b.raan, b.raan*RD );
	printf ( "Excentricity e0: %9.7lf\n", b.e0 );
	printf ( "Perigelium arg w0 ( rad ): %12.10lf ( %8.4lf deg )\n", b.w0, b.w0*RD );
	printf ( "Average anomaly m0 ( rad ): %12.10lf ( %8.4lf deg )\n\n", b.m0, b.m0*RD );

	/* инициализация переменных старой программы по переменным 0-блока */
	c_raan0 = b.raan;
	c_w0 = b.w0;
	c_m0 = b.m0;
	c_i0 = b.i0;
	c_n0 = b.n0;
	isp_bstar = b.bstar;
	c_e0 = b.e0;
	// WTF?
	XNDD60 = 0.0;
	XNDT20 = 0.0;

	Q0MS2T = pow(((Q0-S0)*AE/Ea), 4.0); //Вычисляемая константа!
	S=AE*(1.+S0/Ea); //Ышо одна!

	TS = 0.0;
	TF = 1440.0;
	DELT = 360.0;
	IFLAG = 1;

	for ( TSINCE = TS, i = 0; TSINCE <= TF; TSINCE += DELT, i++ ) {

		SGP4F( TSINCE );

		X *= Ea/AE;
		Y *= Ea/AE;
		Z *= Ea/AE;
		XDOT *= Ea/AE*XMNPDA/86400.0;
		YDOT *= Ea/AE*XMNPDA/86400.0;
		ZDOT *= Ea/AE*XMNPDA/86400.0;

		printf ( "%4.0lf%15.8lf%15.8lf%15.8lf%12.8lf%12.8lf%12.8lf\n",
			TSINCE,X,Y,Z,XDOT,YDOT,ZDOT );

		/* находим отклонения от эталонных значений */
		dx[i] = X - tx[i];
		dy[i] = Y - ty[i];
		dz[i] = Z - tz[i];
		dxdot[i] = XDOT - txdot[i];
		dydot[i] = YDOT - tydot[i];
		dzdot[i] = ZDOT - tzdot[i];
	}

	/* распечатываем отклонение от эталонных значений */
	printf ( "\nОтклонения от эталона:\n" );
	for ( i = 0; i < 5; i++ ) {
		printf ( "%4d %11.8lf %11.8lf %11.8lf %11.8lf %11.8lf %11.8lf\n",
			i*360, dx[i], dy[i], dz[i], dxdot[i], dydot[i], dzdot[i] );
	}

	return 0;
}

/*---------------------------------------------------------------------
	SGP4F.C
	Переписанная с FORTRAN'а функция SGP4.
	Переменные блока COMMON/E1/ объявлены глобальными.
	Переменные блока COMMON/C1/ вместо переменных заменены
	определениями #define, кроме Q0MS2T и S.
	Параметр функции IFLAG объявлен глобальным.
---------------------------------------------------------------------*/
void SGP4F( double TSINCE )
{
	/* все локальные переменные в силу свойств фортрана объявлены
	как static */
	static double a0;
	static double teta;
	static double teta2;
	static double teta4;
	static double X3THM1;
	static double e02;
	static double beta;
	static double beta2;
	static double beta3;
	static double del;
	static double AO;
	static double DELO;
	static double XNODP;
	static double AODP;
	static double S4;
	static double QOMS24;
	static double PERIGE;
	static double PINVSQ;
	static double TSI;
	static double ETA;
	static double ETASQ;
	static double EETA;
	static double PSISQ;
	static double COEF;
	static double COEF1;
	static double C1;
	static double C2;
	static double C3;
	static double C4;
	static double C5;
	static double SINIO;
	static double A3OVK2;
	static double X1MTH2;
	static double TEMP1;
	static double TEMP2;
	static double TEMP3;
	static double XMDOT;
	static double X1M5TH;
	static double OMGDOT;
	static double XHDOT1;
	static double XNODOT;
	static double OMGCOF;
	static double XMCOF;
	static double XNODCF;
	static double T2COF;
	static double XLCOF;
	static double AYCOF;
	static double DELMO;
	static double SINMO;
	static double X7THM1;
	static double C1SQ;
	static double D2;
	static double TEMP;
	static double D3;
	static double D4;
	static double T3COF;
	static double T4COF;
	static double T5COF;
	static double XMDF;
	static double OMGADF;
	static double XNODDF;
	static double OMEGA;
	static double XMP;
	static double TSQ;
	static double XNODE;
	static double TEMPA;
	static double TEMPE;
	static double TEMPL;
	static double DELOMG;
	static double DELM;
	static double TCUBE;
	static double TFOUR;
	static double A;
	static double E;
	static double XL;
	static double BETA;
	static double XN;
	static double AXN;
	static double XLL;
	static double AYNL;
	static double XLT;
	static double AYN;
	static double CAPU;
	static double SINEPW;
	static double COSEPW;
	static double TEMP4;
	static double TEMP5;
	static double TEMP6;
	static double EPW;
	static double ECOSE;
	static double ESINE;
	static double ELSQ;
	static double PL;
	static double R;
	static double RDOT;
	static double RFDOT;
	static double BETAL;
	static double COSU;
	static double SINU;
	static double U;
	static double SIN2U;
	static double COS2U;
	static double RK;
	static double UK;
	static double XNODEK;
	static double XINCK;
	static double RDOTK;
	static double RFDOTK;
	static double SINUK;
	static double COSUK;
	static double SINIK;
	static double COSIK;
	static double SINNOK;
	static double COSNOK;
	static double XMX;
	static double XMY;
	static double UX;
	static double UY;
	static double UZ;
	static double VX;
	static double VY;
	static double VZ;

	static int ISIMP;
//	int I;

	if ( IFLAG == 0 ) goto m100;

/*	RECOVER ORIGINAL MEAN MOTION (XNODP) AND SEMIMAJOR AXIS (AODP)
	FROM INPUT ELEMENTS	*/

	teta = cos (c_i0);
	teta2 = teta*teta;
	teta4 = teta2*teta2;

	e02 = c_e0*c_e0;
	beta2 = 1.-e02;
	beta = sqrt(beta2);
	beta3=beta*beta2;

	a0 = pow ((Ke/c_n0),TW_TH );

	X3THM1 = (3.*teta2-1.);

//	DEL1 = 1.5*CK2*X3THM1/(a0*a0*beta*beta2);
	del = 1.5*CK2*(3.*teta2-1.)/(a0*a0*beta3);

	AO = a0*(1.-del*(.5*TW_TH+del*(1.+134./81.*del)));

//	DELO = 1.5*CK2*X3THM1/(AO*AO*beta*beta2);
	DELO = 1.5*CK2*(3.*teta2-1.)/(AO*AO*beta*beta2);

	XNODP = c_n0/(1.+DELO);
	AODP = AO/(1.-DELO);

/*	INITIALIZATION	*/

/*	FOR PERIGEE LESS THAN 220 KILOMETERS, THE ISIMP FLAG IS SET AND
	THE EQUATIONS ARE TRUNCATED TO LINEAR VARIATION IN SQRT A AND
	QUADRATIC VARIATION IN MEAN ANOMALY. ALSO, THE C3 TERM, THE
	DELTA OMEGA TERM, AND THE DELTA M TERM ARE DROPPED.	*/

	ISIMP = 0;
	if ((AODP*(1.-c_e0)/AE) < (220./Ea+AE)) ISIMP = 1;

/*	FOR PERIGEE BELOW 156 KM, THE VALUES OF
	S AND Q0MS2T ARE ALTERED	*/

	S4 = S;
	QOMS24 = Q0MS2T;
	PERIGE = (AODP*(1.-c_e0)-AE)*Ea;
	if (PERIGE >= 156.) goto m10;
	S4 = PERIGE-78.;
	if (PERIGE > 98.) goto m9;
	S4 = 20.;
m9:	;
	QOMS24 = pow (((120.-S4)*AE/Ea),4.0);
	S4 = S4/Ea+AE;
m10:	;
	PINVSQ = 1./(AODP*AODP*beta2*beta2);
	TSI = 1./(AODP-S4);
	ETA = AODP*c_e0*TSI;
	ETASQ = ETA*ETA;
	EETA = c_e0*ETA;
	PSISQ = fabs(1.-ETASQ);
	COEF = QOMS24* pow (TSI,4.0);
	COEF1 = COEF/ pow (PSISQ,3.5);
	C2 = COEF1*XNODP*(AODP*(1.+1.5*ETASQ+EETA*(4.+ETASQ))+.75*CK2*TSI/PSISQ*X3THM1*(8.+3.*ETASQ*(8.+ETASQ)));
	C1 = isp_bstar*C2;
	SINIO = sin (c_i0);
	A3OVK2 = -XJ3/CK2* pow (AE,3.0);
	C3 = COEF*TSI*A3OVK2*XNODP*AE*SINIO/c_e0;
	X1MTH2 = 1.-teta2;
	C4 = 2.*XNODP*COEF1*AODP*beta2*(ETA*
		(2.+.5*ETASQ)+c_e0*(.5+2.*ETASQ)-2.*CK2*TSI/
		(AODP*PSISQ)*(-3.*X3THM1*(1.-2.*EETA+ETASQ*
		(1.5-.5*EETA))+.75*X1MTH2*(2.*ETASQ-EETA*
		(1.+ETASQ))* cos(2.*c_w0)));
	C5 = 2.*COEF1*AODP*beta2*(1.+2.75*(ETASQ+EETA)+EETA*ETASQ);
	//THETA4 = teta2*teta2;
	TEMP1 = 3.*CK2*PINVSQ*XNODP;
	TEMP2 = TEMP1*CK2*PINVSQ;
	TEMP3 = 1.25*CK4*PINVSQ*PINVSQ*XNODP;
	XMDOT = XNODP+.5*TEMP1*beta*X3THM1+.0625*TEMP2*beta*
		(13.-78.*teta2+137.*teta4);
	X1M5TH = 1.-5.*teta2;
	OMGDOT = -.5*TEMP1*X1M5TH+.0625*TEMP2*(7.-114.*teta2+
		395.*teta4)+TEMP3*(3.-36.*teta2+49.*teta4);
	XHDOT1 = -TEMP1*teta;
	XNODOT = XHDOT1+(.5*TEMP2*(4.-19.*teta2)+2.*TEMP3*(3.-
		7.*teta2))*teta;
	OMGCOF = isp_bstar*C3* cos(c_w0);
	XMCOF = -TW_TH*COEF*isp_bstar*AE/EETA;
	XNODCF = 3.5*beta2*XHDOT1*C1;
	T2COF = 1.5*C1;
	XLCOF = .125*A3OVK2*SINIO*(3.+5.*teta)/(1.+teta);
	AYCOF = .25*A3OVK2*SINIO;
	DELMO = pow ((1.+ETA* cos(c_m0)),3.0);
	SINMO = sin (c_m0);
	X7THM1 = 7.*teta2-1.;
	if (ISIMP == 1) goto m90;
	C1SQ = C1*C1;
	D2 = 4.*AODP*TSI*C1SQ;
	TEMP = D2*TSI*C1/3.;
	D3 = (17.*AODP+S4)*TEMP;
	D4 = .5*TEMP*AODP*TSI*(221.*AODP+31.*S4)*C1;
	T3COF = D2+2.*C1SQ;
	T4COF = .25*(3.*D3+C1*(12.*D2+10.*C1SQ));
	T5COF = .2*(3.*D4+12.*C1*D3+6.*D2*D2+15.*C1SQ*(
		2.*D2+C1SQ));
m90:	;
	IFLAG = 0;

/*	UPDATE FOR SECULAR GRAVITY AND ATMOSPHERIC DRAG	*/

m100:	;
	XMDF = c_m0+XMDOT*TSINCE;
	OMGADF = c_w0+OMGDOT*TSINCE;
	XNODDF = c_raan0+XNODOT*TSINCE;
	OMEGA = OMGADF;
	XMP = XMDF;
	TSQ = TSINCE*TSINCE;
	XNODE = XNODDF+XNODCF*TSQ;
	TEMPA = 1.-C1*TSINCE;
	TEMPE = isp_bstar*C4*TSINCE;
	TEMPL = T2COF*TSQ;
	if (ISIMP == 1) goto m110;
	DELOMG = OMGCOF*TSINCE;
	DELM = XMCOF*( pow ((1.+ETA* cos(XMDF)),3.0) -DELMO);
	TEMP = DELOMG+DELM;
	XMP = XMDF+TEMP;
	OMEGA = OMGADF-TEMP;
	TCUBE = TSQ*TSINCE;
	TFOUR = TSINCE*TCUBE;
	TEMPA = TEMPA-D2*TSQ-D3*TCUBE-D4*TFOUR;
	TEMPE = TEMPE+isp_bstar*C5*( sin(XMP)-SINMO);
	TEMPL = TEMPL+T3COF*TCUBE+
		TFOUR*(T4COF+TSINCE*T5COF);
m110:	;
	A = AODP*TEMPA*TEMPA;
	E = c_e0-TEMPE;
	XL = XMP+OMEGA+XNODE+XNODP*TEMPL;
	BETA = sqrt(1.-E*E);
	XN = Ke/pow(A,1.5);

/*	LONG PERIOD PERIODICS */

	AXN = E* cos(OMEGA);
	TEMP = 1./(A*BETA*BETA);
	XLL = TEMP*XLCOF*AXN;
	AYNL = TEMP*AYCOF;
	XLT = XL+XLL;
	AYN = E* sin(OMEGA)+AYNL;

/*	SOLVE KEPLERS EQUATION */

	CAPU = FMOD2P(XLT-XNODE);
	TEMP2 = CAPU;
	while ( 1 ) {
		SINEPW = sin(TEMP2);
		COSEPW = cos(TEMP2);
		TEMP3 = AXN*SINEPW;
		TEMP4 = AYN*COSEPW;
		TEMP5 = AXN*COSEPW;
		TEMP6 = AYN*SINEPW;
		EPW = (CAPU-TEMP4+TEMP3-TEMP2)/(1.-TEMP5-TEMP6)+TEMP2;
		if ( fabs(EPW-TEMP2) <= E6A) break;
		TEMP2 = EPW;
	}

/*	SHORT PERIOD PRELIMINARY QUANTITIES */

m140:	;
	ECOSE = TEMP5+TEMP6;
	ESINE = TEMP3-TEMP4;
	ELSQ = AXN*AXN+AYN*AYN;
	TEMP = 1.-ELSQ;
	PL = A*TEMP;
	R = A*(1.-ECOSE);
	TEMP1 = 1./R;
	RDOT = Ke* sqrt(A)*ESINE*TEMP1;
	RFDOT = Ke* sqrt(PL)*TEMP1;
	TEMP2 = A*TEMP1;
	BETAL = sqrt(TEMP);
	TEMP3 = 1./(1.+BETAL);
	COSU = TEMP2*(COSEPW-AXN+AYN*ESINE*TEMP3);
	SINU = TEMP2*(SINEPW-AYN-AXN*ESINE*TEMP3);
	U = ACTAN(SINU,COSU);
	SIN2U = 2.*SINU*COSU;
	COS2U = 2.*COSU*COSU-1.;
	TEMP = 1./PL;
	TEMP1 = CK2*TEMP;
	TEMP2 = TEMP1*TEMP;

/*	UPDATE FOR SHORT PERIODICS */

	RK = R*(1.-1.5*TEMP2*BETAL*X3THM1)+.5*TEMP1*X1MTH2*COS2U;
	UK = U-.25*TEMP2*X7THM1*SIN2U;
	XNODEK = XNODE+1.5*TEMP2*teta*SIN2U;
	XINCK = c_i0+1.5*TEMP2*teta*SINIO*COS2U;
	RDOTK = RDOT-XN*TEMP1*X1MTH2*SIN2U;
	RFDOTK = RFDOT+XN*TEMP1*(X1MTH2*COS2U+1.5*X3THM1);

/*	ORIENTATION VECTORS */

	SINUK = sin (UK);
	COSUK = cos (UK);
	SINIK = sin (XINCK);
	COSIK = cos (XINCK);
	SINNOK = sin (XNODEK);
	COSNOK = cos (XNODEK);
	XMX = -SINNOK*COSIK;
	XMY = COSNOK*COSIK;
	UX = XMX*SINUK+COSNOK*COSUK;
	UY = XMY*SINUK+SINNOK*COSUK;
	UZ = SINIK*SINUK;
	VX = XMX*COSUK-COSNOK*SINUK;
	VY = XMY*COSUK-SINNOK*SINUK;
	VZ = SINIK*COSUK;

/*	POSITION AND VELOCITY	*/

	X = RK*UX;
	Y = RK*UY;
	Z = RK*UZ;
	XDOT = RDOTK*UX+RFDOTK*VX;
	YDOT = RDOTK*UY+RFDOTK*VY;
	ZDOT = RDOTK*UZ+RFDOTK*VZ;
}

/*---------------------------------------------------------------------
	ACTAN
	Специфическая функция, вызываемая из SGP4F и SGP8F.
	При переписывании сохранен исходный алгоритм.
	Переменные блока COMMON/C2/ объявлены как #define.
---------------------------------------------------------------------*/
double ACTAN ( double SINX, double COSX )
{
	double TEMP;
	double ret_cod = 0.0;

	if ( COSX == 0.0 ) goto m5;
	if ( COSX > 0.0 ) goto m1;
	ret_cod = PI;
	goto m7;
m1:	;
	if ( SINX == 0.0 ) goto m8;
	if ( SINX > 0.0 ) goto m7;
	ret_cod = TWOPI;
	goto m7;
m5:	;
	if ( SINX == 0.0 ) goto m8;
	if ( SINX > 0.0 ) goto m6;
	ret_cod = X3PIO2;
	goto m8;
m6:	;
	ret_cod = PIO2;
	goto m8;
m7:	;
	TEMP = SINX/COSX;
	ret_cod += atan( TEMP );
m8:	;
	return ret_cod;
}

/*---------------------------------------------------------------------
	FMOD2P
	Специфическая функция, вызываемая из SGP8F.
	При переписывании сохранен исходный алгоритм.
	Переменные блока COMMON/C2/ объявлены как #define.
---------------------------------------------------------------------*/
double FMOD2P( double X )
{
	int i;
	double ret_cod;

	ret_cod = X;
	i = (int)(ret_cod/TWOPI);
	ret_cod -= (double)i * TWOPI;
	if ( ret_cod < 0.0 ) ret_cod += TWOPI;
	return ret_cod;
}

/*---------------------------------------------------------------------
	NORAD_TIME.C
	Функция представляет из себя частичную инициализацию параметров
	NORAD из телеграммы, расположенной в памяти. Она производит получение
	лишь двух параметров NORAD: года и времени от начала года в сутках.
	Функция используется как отдельно для определения времени телеграммы
	и сравнения с другими временами, так и в составе функции TBlk0_ini
	при полной инициализации 0-блока из телеграммы NORAD.
	Прототип: void norad_time ( short* year, double* time,
			char* p );
	где	year	указатель на переменную, в которую будет записан
			год прогноза;
		time	указатель на переменную, в которую будет записано
			полное время прогноза в сутках от начала года
			[1.0-365.99(366.99)] ( аналогично телеграмме );
		p	указатель на первую строку телеграммы.
---------------------------------------------------------------------*/
void norad_time ( short* year, double* time, char* p )
{
	short i;
	char s[16];

	/* Год */
	memcpy ( s, p+18, 2 );
	s[2] = (char)0;
	i = (short) atoi( s );
	/* переход через 2000 год */
	if ( i < 57 ) i += 100;
	*year = i + CENTURY;

	/* День и его дробная часть */
	memcpy ( s, p+20, 12 );
	s[12] = (char)0;
	*time = atof( s );
}

/*---------------------------------------------------------------------
	Функция производит получение параметров NORAD из телеграммы TLE.
	Она записывает параметры NORAD в 0-блок из буфера памяти, при
	этом считается, что указатель спозиционирован на первый символ
	первой строки телеграммы. Открытие и закрытие файла, а также
	чтение из файла проистекает снаружи данной функции. Кроме того,
	функция записывает в 0-блок не только нужные для SGP8 параметры,
	но и другие параметры ( например, номер опорного витка ).
	Прототип:
		void TBlk0_ini ( struct TBlk0_uni* b, char* p );
	где	b	указатель на структуру 0-блока;
		p	указатель на первую строку телеграммы;
---------------------------------------------------------------------*/
void TBlk0_ini ( struct TBlk0_uni* b, char* p )
{
	char s[16];
	short year;
	double day;

	/* получаем год и время */
	norad_time ( &year, &day, p );
	b->ep_year = year;
	b->ep_day = day;

	/* BSTAR drag term. */
	b->bstar = econv ( p + 53, s );

	/* Позиционируем p на начало второй строки телеграммы */
	p = strchr ( p, 0x0a );
	p ++;

	/* Наклонение */
	memcpy ( s, p+8, 8 );
	s[8] = (char)0;
	b->i0 = atof( s ) * DR;
	/* Восходящий узел */
	memcpy ( s, p+17, 8 );
	s[8] = (char)0;
	b->raan = atof( s ) * DR;
	/* Эксцентриситет */
	memcpy ( s+1, p+26, 7 );
	s[0] = (char)'.';
	s[8] = (char)0;
	b->e0 = atof( s );
	/* Перигей */
	memcpy ( s, p+34, 8 );
	s[8] = (char)0;
	b->w0 = atof( s ) * DR;
	/* Средняя аномалия */
	memcpy ( s, p+43, 8 );
	s[8] = (char)0;
	b->m0 = atof( s ) * DR;
	/* Среднее движение */
	memcpy ( s, p+52, 11 );
	s[11] = (char)0;
	b->n0 = atof( s ) * 0.25 * DR;	//4.36332313e-3;

	/* номер витка */
	memcpy ( s, p+63, 5 );
	s[5] = (char)0;
	b->ref_num = atol( s );
}

/*---------------------------------------------------------------------
	ECONV.C
	Служебная функция для преобразования символьной строки вида
	nnnnnnnёmmm в .nnnnnnnEёmmm, а затем в double
---------------------------------------------------------------------*/
double econv ( char* p, char* s )
{
	char* t = s;

	/* Пропускаем пробелы */
	while( *p == ' ' ) p++;
	/* Формирование мантиссы */
	*t++ = '.';
	while( *p != '+' && *p != '-' ) *t++ = *p++;
	/* Формирование порядка */
	*t++ = 'e';
	while( *p != ' ' ) *t++ = *p++;
	*t = (char)0;	
	return ( atof( s ));
}
