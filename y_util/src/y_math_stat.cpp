/*-----------------------------------------------------------------------------
    y_math_moment.cpp
-----------------------------------------------------------------------------*/
#include <math.h>

#include "y_util/y_math.hpp"


void d_minmax( const double * data, int data_size, double * pmin, double * pmax ) {
	int i;
	double cmin, cmax;
	const double * p = data + 1;

	cmin = cmax = *p;

	for( i = 1; i < data_size; i++, p++ ) {
		if( *p < cmin ) {
			cmin = *p;
		} else if( *p > cmax ) {
			cmax = *p;
		}
	}

	if( pmin )
		*pmin = cmin;
	if( pmax )
		*pmax = cmax;
}


void d_avg( const double * data, int data_size, double * pavg ) {
	int i;
	double avg = 0.;
	const double * p = data + 1;

	for( i = 0; i < data_size; i++ ) {
		avg += *p++;
	}

	*pavg = avg / double( data_size );
}


void d_rms( const double * data, int data_size, double * rms ) {
	int i;

	*rms = 0;
	for( i = 1; i <= data_size; i++ ) {
		*rms += data[i]*data[i];
	}

	*rms = sqrt( *rms / double( data_size ) );
}



void d_moment( const double * data, int n, double * avg, double * var, double * std_dev,
			   double * avg_dev, double * skew, double * curt ) {
	int j;
	double ep = 0.;
	double p;

	*avg = 0.;
	for( j = 1; j <= n; j++ ) {
		*avg += data[j];
	}
	*avg /= double(n);

	*avg_dev = *var = *skew = *curt = 0;
	for( j = 1; j <= n; j++ ) {
		*avg_dev += fabs(data[j]-(*avg));
		ep += data[j];
		*var += (p = data[j]*data[j]);
		*skew += (p *= data[j]);
		*curt += p * data[j];
	}
	*avg_dev /= double(n);

	if( n > 1 ) {
		*var = (*var-ep*ep/double(n))/double(n-1);
		*std_dev = sqrt(*var);
		if( *var != 0. ) {
			*skew /= (double(n)*(*var)*(*std_dev));
			*curt = (*curt)/(double(n)*(*var)*(*var))-3.0;
		} else {
			*skew = *curt = 0;
		}
	} else {
		*var = *std_dev = *skew = *curt = 0;
	}
}

