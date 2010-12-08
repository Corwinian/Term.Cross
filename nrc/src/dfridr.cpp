#include <math.h>

#include "nrc.hpp"

#define CON 1.4
#define CON2 (CON*CON)
#define BIG 1.0e30
#define NTAB 10
#define SAFE 2.0

/*
 * dfridr
 * Вычисление производной методом Риддера ( Ridder ).
 * func Дифференцируемая функция.
 * x Точка.
 * h Начальный шаг.
 * err Выход - оценка погрешности.
 */
double dfridr( double (*func)(double), double x, double h, double *err ) {
	int i,j;
	double errt, fac, hh, *a, ans = .0;

	a = new double [NTAB * NTAB];
	hh = h;
	a[0]=((*func)(x+hh)-(*func)(x-hh))/(2.0*hh);
	*err = BIG;
	for( i = 1; i < NTAB; i++ ) {
		hh /= CON;
		a[i]=((*func)(x+hh)-(*func)(x-hh))/(2.0*hh);
		fac = CON2;
		for( j = 1 ; j <= i; j++ ) {
			a[NTAB*j + i] = (a[NTAB*(j-1)+i]*fac-a[NTAB*(j-1)+i-1])/(fac-1.0);
			fac = CON2*fac;
			errt = nrc_max(fabs( a[NTAB*j+i]-a[NTAB*(j-1)+i]),
						   fabs(a[NTAB*j+i]-a[NTAB*(j-1)+i-1]) );
			if (errt <= *err) {
				*err = errt;
				ans = a[NTAB*j+i];
			}
		}
		if( fabs(a[(NTAB+1)*i]-a[(NTAB+1)*(i-1)]) >= SAFE*(*err) )
			break;
	}
	delete a;
	return ans;
}
