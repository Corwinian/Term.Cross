#include <math.h>

#define TINY 1.0e-20;

int ludcmp( double *a, int n, int *indx, double *d ) {
	int i,j,k,mi;
	*d = 1.;
	for( k=0; k<n-1; k++ ) {
		/* поиск k-перестановки */
		mi = k;
		for( j=k+1; j<n; j++ )
			if( fabs(a[k*n+mi]) < fabs(a[k*n+j]) )
				mi = j;
		/* осуществление k-перестановки */
		if( mi != k ) {
			for( i=0; i<n; i++ ) {
				double t = a[i*n+k];
				a[i*n+k] = a[i*n+mi];
				a[i*n+mi] = t;
			}
			*d = -(*d);
		}
		indx[k] = mi;
		indx[mi] = k;
		/* вычисление k-строки матрицы U */
		for( j=k+1; j<n; j++ )
			a[k*n+j] /= a[k*n+k];
		/* вычисление (k+1)-столбца матрицы L */
		for( i=k+1; i<n; i++ ) {
			for( j=k+1; j<n; j++ ) {
				a[i*n+j] -= a[i*n+k]*a[k*n+j];
			}
		}

	}
	return 0;
//	    int i,imax,j,k;
//	    double big,dum,sum,temp;
//	 
//	    double *vv = new double[n];
//	 
//	    *d=1.0;
//	    for (i=0;i<n;i++) {
//	        big=0.0;
//	        for (j=0;j<n;j++) if( (temp=fabs(a[i*n+j])) > big) big=temp;
//	        if (big == 0.0){ delete vv; return 1; }
//	        vv[i] = 1.0/big;
//	    }
//	 
//	    for (j=0;j<n;j++) {
//	        for (i=0;i<j;i++) {
//	            sum=a[i*n+j];
//	            for (k=0;k<i;k++) sum -= a[i*n+k]*a[k*n+j];
//	            a[i*n+j]=sum;
//	        }
//	        big=0.0;
//	        for(i=j;i<n;i++) {
//	            sum = a[i*n+j];
//	            for(k=0;k<j;k++) sum -= a[i*n+k]*a[k*n+j];
//	            a[i*n+j] = sum;
//	            if( (dum=vv[i]*fabs(sum)) >= big) {
//	                big=dum;
//	                imax=i;
//	            }
//	        }
//	 
//	// Меняем местами строки матрицы.
//	        if (j != imax) {
//	            for(k=0;k<n;k++){
//	                dum=a[imax*n+k];
//	                a[imax*n+k]=a[j*n+k];
//	                a[j*n+k]=dum;
//	            }
//	            *d = -(*d);
//	            vv[imax]=vv[j];
//	        }
//	        indx[j]=imax;
//	 
//	        if (a[j*n+j] == 0.0) a[j*n+j]=TINY;
//	        if( j != n-1 ){
//	            dum = 1.0/(a[j*n+j]);
//	            for(i=j+1;i<n;i++) a[i*n+j] *= dum;
//	        }
//	    }
//	 
//	    delete vv;
//	    return 0;
}
