#include <math.h>

#define JMAX 40

int rtbis( double (*func)(double), double x1, double x2, double xacc,
		   double *root ) {
	int j;
	double dx,f,fmid,xmid,rtb;

	f=(*func)(x1);
	fmid=(*func)(x2);
	if (f*fmid >= 0.0)
		return 1;
	rtb = f < 0.0 ? (dx=x2-x1,x1) : (dx=x1-x2,x2);
	for (j=0;j<JMAX;j++) {
		fmid=(*func)(xmid=rtb+(dx *= 0.5));
		if (fmid <= 0.0)
			rtb=xmid;
		if (fabs(dx) < xacc || fmid == 0.0) {
			*root = rtb;
			return 0;
		}
	}
	return 2;
}
