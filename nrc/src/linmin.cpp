#include "nrc.hpp"

#define TOL 2.0e-4

static int ncom;
static double *pcom, *xicom;
static double (*nrfunc)(double[]);

static double f( double );

/*
 * linmin
 * Минимизация функции вдоль луча в n-мерном пространстве.
 * p Начало луча. На выходе - найденная точка.
 * xi Направление луча. На выходе - вектор между началом луча и найденной
 * точкой
 * n Размерность пространства.
 * fret Значение в точке минимума.
 * func Минимизируемая функция.
 */
void linmin(double p[], double xi[], int n, double *fret,
			double (*func)(double [])) {
	int j;
	double xx,xmin,fx,fb,fa,bx,ax;

	ncom = n;
	pcom = new double[n];
	xicom = new double[n];
	nrfunc = func;
	for (j=0;j<n;j++) {
		pcom[j]=p[j];
		xicom[j]=xi[j];
	}
	ax=0.0;
	xx=1.0;
	mnbrak( &ax, &xx, &bx, &fa, &fx, &fb, f );
	*fret = brent( ax, xx, bx, f, TOL, &xmin );
	for (j=0;j<n;j++) {
		xi[j] *= xmin;
		p[j] += xi[j];
	}
	delete xicom;
	delete pcom;
}

double f( double x ) {
	return f1dim( x, ncom, pcom, xicom, nrfunc );
}
