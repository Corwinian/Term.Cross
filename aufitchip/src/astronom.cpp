
#include <math.h>
#include <astronom.hpp>

#define DAY_TIME_IN_MILISECUNDS   86400000 // Количество мсек. в одних сутках.



/*-------------------------------------------*\
 SHCRDS.C
 Определение местных горизонтальных координат Солнца
 ВХОД:
 jd - юлианская дата
 l - долгота места наблюдения (рад)
 fi - широта места наблюдения (рад)

 Функция возвращает высоту видимого Солнца над
 горизонтом (град)
\*-------------------------------------------*/

double TAstronom::shcrds( double jd, double l, double fi)
{
	double st;
	double alfa;
	double delta;
	double hh;
	double sh;
	double pi=3.14159265358979;
	double raddeg;

	raddeg=pi/180.0;

	st = gsidtj( jd );
	sunpos(jd, &alfa, &delta );
	hh = 2 * pi * st - l - alfa;
	sh = sin( fi ) * sin( delta ) + cos( fi ) * cos( delta ) * cos( hh );
	return( asin( sh ) / raddeg );
}


/*-------------------------------------------*\
 SHCRDS.C
 Определение местных горизонтальных координат Солнца
 Более полная версия чем предыдущая.
 ВХОД:
 jd - юлианская дата
 l - долгота места наблюдения (рад)
 fi - широта места наблюдения (рад)

 Функция записывает высоту видимого Солнца над
 горизонтом (град) по адресу h, и его азимут
 по адресу a.
\*-------------------------------------------*/

void TAstronom::shcrds( double jd, double l, double fi, double* h, double* a )
{
	double st;
	double alfa;
	double delta;
	double hh;
	double sh;
	const double pi=3.14159265358979;
	const double raddeg=pi/180.0;
	double x;
	double y;


	st = gsidtj( jd );
	sunpos(jd, &alfa, &delta );
	hh = 2 * pi * st - l - alfa;
	x=cos(hh)*sin(fi)-tan(delta)*cos(fi);
	y=sin(hh);
	*a=atan2(y,x)/raddeg;
	sh = sin( fi ) * sin( delta ) + cos( fi ) * cos( delta ) * cos( hh );
	// переход от астрономического азимута к геодезическому
	*a += 180.;
	*h =asin(sh)/raddeg;
}


/*-------------------------------------------*\
 GSIDTI.C
 вычисление истинного звездного времени на гринвическом
 меридиане.
 Функции подается юлианская дата на которую вычисляется
 звездное время
     (целая+дробная часть)
 Функция возвращает истинное звездное время на меридиане
 Гринвича (в долях суток)
\*-------------------------------------------*/

double TAstronom::gsidtj( double jd ){
	double st;
	double jd0;
	double eps;
	double dpsi;
	double pi = 3.14159265358979;
	jd0 = jd - fmod( jd - 0.5 , 1.0 );  /* полночь */
	st =  gsidaj( jd );
	eps = eclipt( jd0 );
	dpsi = delpsi( jd0 );
	st += cos(eps) * dpsi / ( 2 * pi );
	return( st );
}


/*-------------------------------------------*\
 ECLIPT.C
 вычисление среднего наклона эклиптики
 вход:
 юлианская дата на которую вычисляется наклон эклиптики
     (целая+дробная часть)
 Функция возвращает наклон эклиптики к экватору (рад)
\*-------------------------------------------*/

double TAstronom::eclipt(double jd){
	double p[4] = { 0.000000503,
			-0.00000164,
			-0.0130125,
			23.452294 };
	int i;
	double t;
	double deg = 1.745329251994330e-2; /* 1 degree = deg radians */
	double eps;

	t = ( jd - 2415020.0 ) / 36525.;
	eps = 0.0 ;
	for( i = 0; i < 4; i++) eps = eps * t + p[i] ;
	eps *= deg;
	return( eps );
}


/*-------------------------------------------*\
 GSIDAJ.C
 вычисление среднего звездного времени на гринвическом
 меридиане
 вход
 jd - юлианская дата на которую вычисляется звездное время
      (целая+дробная часть)
 Функция возращает среднее звездное время на меридиане
 Гринвича (в долях суток)
\*-------------------------------------------*/

double TAstronom::gsidaj( double jd ){
	double jd0;
	double st0;
	double t;

	/* находим звездное время на 0h UT */
	jd0 = jd - fmod( jd - 0.5 , 1.0 );	/* полночь */
	t = ( jd0 - 2415020.0 ) / 36525. ;
	st0 = 0.276919398 + 100.0021359 * t + 0.000001075 * t * t ;
	st0 = fmod( st0, 1.0 );
	/* находим звездное время на момент jd */
	return( st0 + ( jd - jd0 ) * 1.002737908 );
}


/*-------------------------------------------*\
 DELPCI.C
 вычисление нутации в долготе
 вход
 юлианская дата на которую вычисляется нутация
     (целая+дробная часть)
 Функция возвращает нутацию в долготе (рад)
\*-------------------------------------------*/

double TAstronom::delpsi( double jd ){
	double t;
	double l;
	double lp;
	double m;
	double mp;
	double w;
	double deg = 1.745329251994330e-2;
	double sec = 4.848136811095360e-6;
	double dpsi;

	t = ( jd - 2415020.0 ) / 36525.;
	/* средняя долгота Солнца */
	l = ( 279.6967 + 36000.7689 * t + 0.000303 * t * t ) * deg;
	/* средняя долгота Луны */
	lp = ( 270.4342 + 481267.8831 * t + 0.001133 * t * t ) * deg;
	/* средняя аномалия Солнца */
	m = ( 358.4758 + 35999.0498 * t + 0.000150 * t * t ) * deg;
	/* средняя аномалия Луны */
	mp = ( 296.1046 + 477198.8491 * t + 0.009192 * t * t ) * deg;
	/* долгота восходящего узла Луны */
	w = ( 259.1833 - 1934.1420 * t + 0.002078 * t * t) * deg;

	dpsi =  - ( 17.2327 + 0.01737 * t ) * sin( w )
		- ( 1.2729 + 0.00013 * t ) * sin( 2 * l )
		+ 0.2088 * sin( 2 * w )
		- 0.2037 * sin( 2 * lp )
		+ ( 0.1261 - 0.00031 * t ) * sin( m )
		+ 0.0675 * sin( mp )
		- ( 0.0497 - 0.00012 * t ) * sin( 2 * l + m )
		- 0.0342 * sin( 2 * lp - w )
		- 0.0261 * sin( 2 * lp + mp )
		+ 0.214 * sin( 2 * l - m )
		- 0.0149 * sin( 2 * l - 2 * lp + mp )
		+ 0.0124 * sin( 2 * l - w )
		+ 0.0114 * sin( 2 * lp - mp );
	/* перевод из секунд дуги в радианы */
	dpsi *= sec;
	return( dpsi );
}


/*-------------------------------------------*\
 SUNPOS.C
 Опеделение видимого положения Солнца и уравнения
 времени на момент jd

 Вызов:
 sunpos( double jd, double* alfa, double* delta )
 где jd - юлианская дата, на которую определяются
     координаты Солнца (целая+дробная часть)
 alfa - прямое восхождение Солнца (рад)
 delta - склонение Солнца (рад)
	отнесенные к истинному равноденствию даты
 Функция модефицирует переменные alfa и delta
\*-------------------------------------------*/

void TAstronom::sunpos( double jd, double* alfa, double * delta)
{
	double t;
	double dt;
	double l;
	double m;
	double e;
	double s;
	double omega;
	double sv;
	double eps;
	double v;
	double pi = 3.14159265358979;
	double raddeg;
	double tol = 1.0e-8;

	raddeg = pi / 180.0;

	/* время в Юлианских столетиях от эпохи 1900 январь 0.5 */
	t = ( jd - 2415020.0 ) / 36525.0;
	/* разница между эфемеридным и всемирным временем */
	dt = 0.41 + 1.2053 * t + 0.4992 * t * t;	/* (мин, прибл. знач.) */
	/* время в Юлианских эфемеридных столетиях от эпохи 1900 январь 0.5 */
	t += dt / (24.0 * 60.0 * 36525.0)  ;
	/* средняя геометрическая долгота Солнца, отсчитываемая от
	   средней точки равноденствия на дату jd (рад) */
	l = ( 279.69668 + 36000.76892 * t + 0.0003025 * t * t );
	l = fmod( l, 360.0) * raddeg;
	/* средняя аномалия Солнца (рад) */
	m = 358.47583 + 35999.04975 * t - 0.000150 * t * t - 0.0000033 * t * t * t ;
	m = fmod( m, 360.0 ) * raddeg;	/*  переводим в радианы */
	/* эксцентриситет орбиты Земли */
	e = 0.01675104 - 0.0000418 * t - 0.000000126 * t * t;
	v = kplreq( tol, m, e );
	s = l + v - m;
	if( s < 0.0 ) s += 2 * pi;
	s += pert3( t );

	/* s есть истинная геометрическя долгота, отнесенная к среднему
	   равноденствию даты jd. Если требуется видимая долгота
	   отнесенная к истинному равноденствию даты jd, то s нужно
	   исправить за нутацию и аберрацию */
	omega = ( 259.18 - 1934.142 * t ) * raddeg;  /* рад  */
	sv = s - ( 0.00569 + 0.00479 * sin( omega ) ) * raddeg; /* рад */

	/* наклон эклиптики (рад) */
	eps = eclipt( t );
	/* добавка к eps для вычисления видимого положения Солнца */
	eps += 0.00256 * cos( omega ) * raddeg;
	/* прямое восхождение Солнца (рад) */
	*alfa = atan2( cos( eps ) * sin( sv ), cos( sv ) );
	if( *alfa < 0.0 ) *alfa += 2 * pi;
	/* склонение Солнца (рад) */
	*delta = asin( sin( eps ) * sin( sv ) );
}


/*-------------------------------------------*\
 KPLREQ.C
 решение уравнения Кеплера
 Параметры:
 tol - точность решения (рад)
 m - средняя аномалия (рад)
 e - эксцентриситет
 Функция возвращает значение истинной аномалии
\*-------------------------------------------*/

double TAstronom::kplreq( double tol, double m, double e )
{
	double d = 1.0;
	double ea;

	ea = m;
	while( fabs( d ) > tol ){
		d = m + e * sin( ea ) - ea;
		d /= 1.0 - e * cos( ea ) ;
		ea += d;
	}

	return( 2.0 * atan( sqrt( ( 1.0 + e ) / ( 1 - e ) ) * tan( ea / 2.0 ) )	);
}


/*-------------------------------------------*\
 PERT3.C
 вычисление возмущений Солнца в долготе
 вход:
 t - время в юлианских столетиях от эпохи 1900, январь, 0.5
 Функция возвращает возмущение в долготе.
\*-------------------------------------------*/

double TAstronom::pert3( double t ){
	double pi = 3.14159265358979;
	double raddeg;
	double a;
	double b;
	double c;
	double d;
	double e;
	double dl;

	raddeg = pi / 180.0;
	a = ( 153.23 * 22518.7541 * t ) * raddeg;
	b = ( 216.57 + 45037.5082 * t ) * raddeg;
	c = ( 312.69 + 32964.3577 * t ) * raddeg;
	d = ( 350.74 + 445267.1142 *t - 0.00144 * t * t ) * raddeg;
	e = ( 231.19 + 20.20 * t ) * raddeg;
	dl = 0.00134 * cos( a ) + 0.00154 * cos( b ) + 0.00200 * cos( c )
		+ 0.00179 * sin( d ) + 0.00178 * sin( e );
	dl *= raddeg;
	return( dl );
}


/*------------------------------------------------*\
 вычисление юлианского дня по григорианской календарной дате
 вход
 yy - год
 d - номер дня в году (целая+дробная часть,
 	нумерация начинается с единицы)
 выход
 юлианская дата
\*------------------------------------------------*/

double TAstronom::julian( int yy, double d )
{
	int y,m,a,b;
	y = yy - 1;
	m = 13;
	a = y / 100;
	b = 2 - a + a / 4;
	return( (double)((int)( 365.25 * (double) y ) + (int)( 30.6001 * (double) ( m + 1 ) ) ) + d + 1720994.5 + (double) b );
}
