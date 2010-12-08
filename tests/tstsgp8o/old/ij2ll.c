/*-------------------------------------------------------------------------
	ij2ll.c
-------------------------------------------------------------------------*/
#include <math.h>
#include <string.h>
#include "f34_uni.h"
#include "orbmod2.h"
#include "vec_fun.h"

static double last_scan; // Номер последнего скана, с которым мы работали.

static double myp[3][3];
static double mr[3][3];

// Матрица преобразования координат, заданных в с.к. сканера, в координаты
// в неподвижной геоцентрической с.к. Третий столбец не используется,
// посему - не вычисляется.
static double ma[3][2];

static double sin_ov, cos_ov;

static double C;
static double b[3];

static TIniSatParams isp;
static TSatParams sat;

static void ini_ij2ll_s( double );
static void ij2ll_s( double, double *, double * );


void iniIJ2LL( TIniSatParams * ISP, TNOAAImageParams * nip, TCorrectionParams * cop )
{
	int i,j,k;
	double roll = ( cop )? cop->roll : 0.;
	double pitch = ( cop )? cop->pitch : 0.;
	double yaw = ( cop )? cop->yaw : 0.;

	/* заполнение массивов заменено, так как компилятор Watcom не понимает
	закомментированные конструкции */
	double mp[3][3];
	double my[3][3];

//	double mp[3][3] = {
//		{ cos( pitch ), 0., -sin( pitch ) },
//		{ 0., 1., 0. },
//		{ sin( pitch ), 0., cos( pitch ) } };
//	double my[3][3] = {
//		{ 1., 0., 0. },
//		{ 0., cos( yaw ), -sin( yaw ) },
//		{ 0., sin( yaw ), cos( yaw ) } };

	mp[0][0] = cos( pitch );
	mp[0][1] = 0.0;
	mp[0][2] = -sin( pitch );
	mp[1][0] = 0.0;
	mp[1][1] = 1.0;
	mp[1][2] = 0.0;
	mp[2][0] = sin( pitch );
	mp[2][1] = 0.0;
	mp[2][2] = cos( pitch );

	my[0][0] = 1.0;
	my[0][1] = 0.0;
	my[0][2] = 0.0;
	my[1][0] = 0.0;
	my[1][1] = cos( yaw );
	my[1][2] = -sin( yaw );
	my[2][0] = 0.0;
	my[2][1] = sin( yaw );
	my[2][2] = cos( yaw );


	last_scan = -1;

	memcpy ((void*)&isp, (void*)ISP, sizeof( TIniSatParams ));
	iniSGP8( &sat, &isp, nip->year, nip->yearTime, cop );

	mr[0][0] = mr[1][1] = cos( roll );
	mr[1][0] = sin( roll );
	mr[0][1] = -mr[1][0];
	mr[0][2] = mr[2][0] = mr[1][2] = mr[2][1] = 0.;
	mr[2][2] = 1.;

	for(i=0;i<3;i++){
		for(j=0;j<3;j++){
			double t=0.;
			for(k=0;k<3;k++) t+=my[i][k]*mp[k][j];
			myp[i][j] = t;
		}
	}
}


void IJ2LL( double x, double y, double *lon, double *lat )
{
        if( y != last_scan ){
                last_scan = y;
                ini_ij2ll_s( last_scan );
        }
        ij2ll_s( x, lat, lon );
}


void ini_ij2ll_s( double scan )
{
	double msi[3][3];
	double r[3];
	double v[3];
	double d[3];
	double mal[3][3];
	double mm1[3][3];
	double mm2[3][3];
	double t;
	double cosa, sina;
	int m,n,k;

	SGP8( &sat, &isp, AVHRR_SCAN_TIME*scan );
	vec_copy( r, sat.r );
	r[0] /= Ea2; r[1] /= Ea2; r[2] /= Eb2;
	vec_add( r, r, b );
	C = scal_mul( r, sat.r ) - 1.0;
	sin_ov = sin( sat.ov );
	cos_ov = cos( sat.ov );

	/* Вычисление матрицы msi. */
	vec_num_mul( sat.r, -1./vec_len(sat.r), msi[0] );
	vec_num_mul( sat.v, 1./vec_len(sat.v), v );
	vec_norm( vec_mul( v, msi[0], msi[1] ) );
	vec_mul( msi[0], msi[1], msi[2] );

	/* Вычисление угла a. */
	cosa = scal_mul( msi[2], v );
	sina = sqrt( 1. - cosa*cosa );
	if( sina > 1.e-10 ){
		vec_mul( msi[2], v, d );
		if( scal_mul( d, msi[1] ) < 0. ) sina = -sina;
	}
	else{ sina = 0.; cosa = 1.; }
	mal[0][0] = cosa; mal[0][1] = 0; mal[0][2] = sina;
	mal[1][0] = 0; mal[1][1] = 1; mal[1][2] = 0;
	mal[2][0] = -sina; mal[2][1] = 0; mal[2][2] = cosa;

	/* Вычисление матрицы ma, maT = msi * myp * malT * mr * ma. */
	for(m=0;m<3;m++){
		for(n=0;n<3;n++){
			t = 0;
			for(k=0;k<3;k++) t+=mr[m][k]*mal[k][n];
			mm1[m][n] = t;
		}
	}
	for(m=0;m<3;m++){
		for(n=0;n<3;n++){
			t = 0;
			for(k=0;k<3;k++) t += mal[k][m]*mm1[k][n];
			mm2[m][n] = t;
		}
	}
	for(m=0;m<3;m++){
		for(n=0;n<3;n++){
			t = 0;
			for(k=0;k<3;k++) t += myp[m][k]*mm2[k][n];
			mm1[m][n]=t;
		}
	}
	for(m=0;m<3;m++){
		for(n=0;n<2;n++){
			t = 0;
			for(k=0;k<3;k++) t+=msi[k][m]*mm1[k][n];
			ma[m][n] = t;
		}
	}
}


/*--------------------------------------------------------------------------
        ij2ll_s
        Локальные переменные.
        R               Длина вектора спутник -> сканируемая точка.
        d               Направление спутник -> сканируемая точка в инерци-
                        альной с.к.
        e               Сканируемая точка в инерциальной системе координат.
        ecf             Сканируемая точка в системе координат, связанной с
                        вращением Земли.
        A, B            Квадратичный и линейный коэфф. уравнения для R.
--------------------------------------------------------------------------*/
void ij2ll_s( double x, double *lat, double *lon )
{
	double R, A, B;
	double d[3];
	double e[3], ecf[2];
	double theta = (1024. - x)*STEP_ANGLE;
	int i;

	/* заполнение массивов заменено, так как компилятор Watcom не понимает
	закомментированные конструкции */
	double dpqs[2];
//	double dpqs[2] = { cos(theta), -sin(theta) };
	dpqs[0] = cos(theta);
	dpqs[1] = -sin(theta);

	for(i=0;i<3;i++) d[i] = ma[i][0]*dpqs[0] + ma[i][1]*dpqs[1];

	/* Решение квадратного уравнения для R. */
	A = (d[0]*d[0] + d[1]*d[1])/Ea2 + d[2]*d[2]/Eb2;
	B = scal_mul( b, d );
	R = -(B + sqrt( B*B - 4.0*A*C ))/(A+A);

	/* Радиус-вектор точки поверхности в неподвижной с.к. */
	vec_add( sat.r, vec_num_mul( d, R, d ), e );

	/* Переход к системе, связанной с вращением Земли. */
	ecf[0] = e[0]*cos_ov + e[1]*sin_ov;
	ecf[1] = e[1]*cos_ov - e[0]*sin_ov;

	/* Находим геодезические широту и долготу. */
	*lat = atan2( e[2], (1.0-Ee2)*sqrt(ecf[0]*ecf[0]+ecf[1]*ecf[1]) );
	*lon = atan2( ecf[1], ecf[0] );
}

/*--------------------------------------------------------------------------
	vec_mul
--------------------------------------------------------------------------*/
double * vec_mul( double *a, double *b, double *c )
{
    double x1,x2,y1,y2,z1,z2;

    x1 = a[1]*b[2];
    x2 = a[2]*b[1];
    y1 = a[0]*b[2];
    y2 = a[2]*b[0];
    z1 = a[0]*b[1];
    z2 = a[1]*b[0];
    c[0] = x1 - x2;
    c[1] = y2 - y1;
    c[2] = z1 - z2;
    return c;
}

/*--------------------------------------------------------------------------
    SCAL_MUL
--------------------------------------------------------------------------*/
double  scal_mul( double *a, double *b )
{
    return ( a[0]*b[0] + a[1]*b[1] + a[2]*b[2] );
}

/*--------------------------------------------------------------------------
    VEC_LEN
--------------------------------------------------------------------------*/
double  vec_len( double *a )
{
    return sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );
}

/*--------------------------------------------------------------------------
    VEC_NORM
--------------------------------------------------------------------------*/
void    vec_norm( double *a )
{
    double l = vec_len(a);
    a[0] /= l; a[1] /= l; a[2] /= l;
}

/*--------------------------------------------------------------------------
    VEC_COPY
--------------------------------------------------------------------------*/
void    vec_copy( double *dst, double *src )
{
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
}

/*--------------------------------------------------------------------------
    VEC_ADD
--------------------------------------------------------------------------*/
double * vec_add( double *a, double *b, double *r )
{
    double *t = r;
    *t++ = *a++ + *b++;
    *t++ = *a++ + *b++;
    *t++ = *a++ + *b++;
    return r;
}

/*--------------------------------------------------------------------------
    VEC_SUB
--------------------------------------------------------------------------*/
double * vec_sub( double *a, double *b, double *r )
{
    double *t = r;
    *t++ = *a++ - *b++;
    *t++ = *a++ - *b++;
    *t++ = *a++ - *b++;
    return r;
}

/*--------------------------------------------------------------------------
    VEC_NUM_MUL
--------------------------------------------------------------------------*/
double * vec_num_mul( double *a, double num, double * r )
{
    double *t = r;
    *t++ = *a++ *  num;
    *t++ = *a++ *  num;
    *t++ = *a++ *  num;
    return r;
}
