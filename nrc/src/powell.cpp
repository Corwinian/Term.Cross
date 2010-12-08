#include <math.h>

#include "nrc.hpp"

#define ITMAX 200

/*
 * powell
 * Нахождение безусловного локального минимума функции нескольких
 * переменных.
 * p Начальное приближение.
 * xi Матрица размером n*n, задающая начальные(?) направления поиска.
 *     Конкретно, каждая строка этой матрицы есть вектор, вдоль которого при
 *     первой(?) итерации осуществляется поиск. Хорошим выходом является
 *     задание единичной матрицы.
 * n Размерность пространства.
 * ftol Требуемая относительная точность.
 * iter Число проделанных итераций.
 * fret Значение функции в найденной точке.
 * func Минимизируемая функция.
 */
void powell( double p[], double *xi, int n, double ftol, int *iter, double *fret, double (*func)(double []) ) {
	int i,ibig,j;
	double del,fp,fptt,t;

	double * pt = new double[n];
	double * ptt = new double[n];
	double * xit = new double[n];

	*fret=(*func)(p);
	for (j=0;j<n;j++)
		pt[j]=p[j];
	for (*iter=0;;++(*iter)) {
		fp=(*fret);
		ibig=0;
		del=0.0;
		for (i=0;i<n;i++) {
			for (j=0;j<n;j++)
				xit[j]=xi[i*n+j];
			fptt=(*fret);
			linmin(p,xit,n,fret,func);
			if (fabs(fptt-(*fret)) > del) {
				del=fabs(fptt-(*fret));
				ibig=i;
			}
		}
		if( 2.0*fabs(fp-(*fret)) <= ftol*(fabs(fp)+fabs(*fret)) || *iter == ITMAX ) {
			delete [] xit;
			delete [] ptt;
			delete [] pt;
			return;
		}
		for (j=0;j<n;j++) {
			ptt[j]=2.0*p[j]-pt[j];
			xit[j]=p[j]-pt[j];
			pt[j]=p[j];
		}
		fptt=(*func)(ptt);
		if (fptt < fp) {
			t = 2.*(fp-2.*(*fret)+fptt) * nrc_sqr(fp-(*fret)-del) - del*nrc_sqr(fp-fptt);
			if (t < 0.) {
				linmin(p,xit,n,fret,func);
				for (j=0;j<n;j++) {
					xi[ibig*n+j]=xi[(n-1)*n+j];
					xi[(n-1)*n+j]=xit[j];
				}
			}
		}
	}
}
