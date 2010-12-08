/*-------------------------------------------------------------------------
    orbconst.h
    Определения внутренних констант библиотеки ORBMODEL.
	21 янв 2009
		Из c_lib/include/c_lib/c_const.h перенесены параметры эллипсоида
		и вместо "const double" определены через #define
		Эксцентриситет и rвадрат эксцентриситета теперь вычисляются находу,
		а не забиты константами, как ранее.
-------------------------------------------------------------------------*/
#ifndef _ORBCONST_H_
#define _ORBCONST_H_

#define NOAA_MAX_SCAN_ANGLE     55.37135
#define NOAA16_MAX_SCAN_ANGLE   55.25

/* Параметры эллипсоида Земли. Совпадают с WGS72. */
//#define Ea      6378.135        /* Большая полуось земного эллипсоида WGS72. */
//#define Eb      6356.75052      /* Малая полуось земного эллипсоида WGS72. */
/* Параметры эллипсоида Земли. Совпадают с WGS84. */
#define Ea 6378.137             /* Большая полуось земного эллипсоида. WGS84*/
#define Eb 6356.7523142         /* Малая полуось земного эллипсоида. WGS84*/
#define Ea2     (Ea*Ea)
#define Eb2     (Eb*Eb)
//#define Ee2 = 6.69437999014e-3;    /* Квадрат эксцентриситета. */
#define Ee2     ((Ea2 - Eb2) / Ea2)  /* Квадрат эксцентриситета. */
//#define Ee = 8.1819190842622e-2;   /* Эксцентриситет. */
#define Ee      sqrt( Ee2 )          /* Эксцентриситет. */

#define D_OV    1.2153526424e-5
//#define SIN_DOV 1.21535264237e-5
#define SIN_DOV sin(D_OV)
//#define COS_DOV 9.999999999261458e-1
#define COS_DOV cos(D_OV)

#define TW_TH   (2./3.)
#define TH_TW   1.5
#define SE_TH   (7./3.)

/*
//by Epshtein
#define Ke      0.743669161e-1
#define Qms4    1.88027916e-9
#define K2      5.413080e-4
#define TK2     8.11962e-4
#define K3      1.172535e-3
#define TK4     7.76235937e-7
#define K4      (TK4*.8)
#define Ser     1.01222928
*/
#define A30K2   4.69014e-3

#define Ke      0.743669161e-1
#define	XJ2		1.082616e-3
#define	XJ3		(-0.253881e-5)
#define	XJ4		(-1.65597e-6)
#define K2      (0.5*XJ2)		/* 5.413080e-4	*/
#define TK2     (TH_TW*K2)		/* 8.11962e-4	*/
#define A3COF	(-XJ3/K2)
#define K3      (0.25*A3COF)	/* 1.172535e-3	*/
#define TK4     (-15.0/32.0*XJ4)	/* 7.76235937e-7 */
#define K4      (TK4*.8)
#define	Q0		120.0
#define	S0		78.0
//#define	XKMPER	6378.135
#define	XKMPER	6378.137
//#define Qms4    1.88027916e-9
//#define Ser     1.01222928
#define Q0MS2T1	((Q0-S0)/XKMPER)
#define Q0MS2T2	(Q0MS2T1*Q0MS2T1)
#define Qms4	(Q0MS2T2*Q0MS2T2)
#define Ser		(1.0+S0/XKMPER)
#define CENTURY	1900

#endif
