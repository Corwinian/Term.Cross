void lubksb(double *a, int n, int *indx, double *b, double *x){
//	    int i, ii=0, ip, j;
//	    double sum;
//	 
//	    for(i=0;i<n;i++){
//	        ip = indx[i];
//	        sum = b[ip];
//	        b[ip] = b[i];
//	        if( ii ){
//	            for(j=ii;j<=i-1;j++) sum -= a[i*n+j]*b[j];
//	        }
//	        else if( sum ) ii = i;
//	        b[i] = sum;
//	    }
//	 
//	    for(i=n-1;i>=0;i--){
//	        sum = b[i];
//	        for (j=i+1;j<n;j++) sum -= a[i*n+j]*b[j];
//	        b[i] = sum/a[i*n+i];
//	    }
	int i,j;
	/* нахождение решения уравнения Ly = b */
	for( i=0; i<n; i++ ) {
		double t = 0.0;
		for( j=0; j<i; j++ )
			t += x[j]*a[i*n+j];
		x[i] = (b[i]-t) / a[i*n+i];
	}
	/* нахождение решения уравнения Ux = y */
	for( i=n-1; i>=0; i-- ) {
		double t = 0.0;
		for( j=i+1; j<n; j++ )
			t += x[j]*a[i*n+j];
		x[i] -= t;
	}

	for(i=0;i<(n>>1);i++) {
		double t = x[indx[i]];
		x[indx[i]] = x[i];
		x[i] = t;
	}
}
