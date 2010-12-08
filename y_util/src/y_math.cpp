/*-------------------------------------------------------------------------
    y_math.cpp
-------------------------------------------------------------------------*/
#include <math.h>

#include <c_lib.hpp>

#include "y_util/y_math.hpp"

/*--------------------------------------------------------------------------
    V_LEN
--------------------------------------------------------------------------*/
double v_len( const double *a ) {
	return sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );
}


/*--------------------------------------------------------------------------
    V_CPY
--------------------------------------------------------------------------*/
void v_cpy( double * dst, const double * src ) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}


/*--------------------------------------------------------------------------
    V_MUL
--------------------------------------------------------------------------*/
void v_mul( const double * a, double n, double * r ) {
	r[0] = a[0] * n;
	r[1] = a[1] * n;
	r[2] = a[2] * n;
}


/*--------------------------------------------------------------------------
    V_ADD
--------------------------------------------------------------------------*/
void v_add( const double *a, const double *b, double *r ) {
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
}

/*--------------------------------------------------------------------------
    V_SUB
--------------------------------------------------------------------------*/
void v_sub( const double *a, const double *b, double *r ) {
	r[0] = a[0] - b[0];
	r[1] = a[1] - b[1];
	r[2] = a[2] - b[2];
}


double v_smul( const double * a, const double * b ) {
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}


double v_angle( const double * a, const double * b ) {
	double s = v_smul( a, a ) * v_smul( b, b );
	if( s != 0. )
		return acos( v_smul( a, b ) / sqrt( s ) );
	else
		return 0.;
}


void v_norm( const double * a, double * b ) {
	double r = v_len( a );
	if( r != 0. ) {
		b[0] = a[0] / r;
		b[1] = a[1] / r;
		b[2] = a[2] / r;
	}
}


void v_vmul( const double * a, const double * b, double * c ) {
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];
}


void v_ort( const double * a, const double * b, double * c ) {
	v_vmul( a, b, c );
	v_norm( c, c );
}


double mgi240( const double * a, const double * b, const double * c ) {
	double v1[3], v2[3], v3[3];
	v_ort( c, b, v1 );
	v_ort( a, b, v2 );
	double angle = v_angle( v1, v2 );
	v_ort( v1, v2, v3 );
	if( v_smul( v3, b ) > 0. )
		angle = 2.*PI - angle;
	return angle;
}
