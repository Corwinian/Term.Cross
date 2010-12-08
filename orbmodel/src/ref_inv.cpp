//-------------------------------------------------------------------------
//  ref_inv.cpp
//-------------------------------------------------------------------------
#include <math.h>
#include <y_util.hpp>
#include "orbconst.h"
#include <orbmodel.hpp>

struct TStoreElement {
	double scan_n[3];   // Нормаль плоскости сканирования.
	double scan_d;      // Свободный член уравнения плоскости сканирования.
	double sat_pos[3];  // Положение спутника.
	double sub_p[3];    // Подспутниковая точка.
	double dir_v[3];    // Вектор, перпендикулярный плоскости радиус-вектора и вектора скорости. Нужен для определения знака угла.
	double sin_dov,cos_dov; // Синус и косинус угла на Овна для каждого скана.
};


TInverseReferencer::TInverseReferencer( const TIniSatParams& isp, const TNOAAImageParams& nip, const TCorrectionParams& cop ) :
		fNIP( nip ),
fCOP( cop ) {
	fBaseYear = fNIP.fYear;
	fBaseYearTime = fNIP.fYearTime - 1./(360.*1440.);   // отодвигаемся назад на время одного скана
	if( fBaseYearTime < 0 ) {                            // на всякий случай проверяем переход назад через границу года
		fBaseYearTime += isLeapYear( --fBaseYear ) ? 365. : 364.;
	}

	fOM = new TOrbitalModel( isp, fBaseYear, fBaseYearTime, fCOP );

	maxScanAngle = fNIP.satId == TSatInfoTable::sat_id_noaa_16 ? NOAA16_MAX_SCAN_ANGLE*DR : NOAA_MAX_SCAN_ANGLE*DR;

	se = new TStoreElement[fNIP.fScans+2];
	ini_ll2xy();
}


TInverseReferencer::~TInverseReferencer() {
	delete [] se;
	delete fOM;
}


void TInverseReferencer::setCorrectionParams( const TCorrectionParams & c ) {
	if( fCOP == c )
		return;

	fCOP = c;
	fOM->setCorrectionParams( fCOP );
	ini_ll2xy();
}


void TInverseReferencer::ini_ll2xy() {
	double tt, dov; //t,
	// Транспонированная матрица пересчета координат из надирной с.к. в неподвижную геоцентрическую.
	double msi[3][3];
	// Транспонированная матрица перехода от с.к. сканера к неподвижной геоцентрической.
	double ma[3][3];

	int i,m,n,k; //,year

	double roll = fCOP.fVersion == 0 ? 0 : fCOP.roll;
	double pitch = fCOP.fVersion == 0 ? 0 : fCOP.pitch;
	double yaw = fCOP.fVersion == 0 ? 0 : fCOP.yaw;

	double myp[3][3];
	{
		double my[3][3] = {
							{ 1., 0., 0. },
							{ 0., cos( yaw ), -sin( yaw ) },
							{ 0., sin( yaw ), cos( yaw ) } };
		double mp[3][3] = {
							{ cos( pitch ), 0., -sin( pitch ) },
							{ 0., 1., 0. },
							{ sin( pitch ), 0., cos( pitch ) } };
		for( m=0; m<3; m++ ) {
			for( n=0; n<3; n++ ) {
				double t = 0.;
				for( k=0; k<3; k++ )
					t += my[m][k]*mp[k][n];
				myp[m][n] = t;
			}
		}
	}
	double mr[3][3] = {
						  { cos( roll ), -sin( roll ), 0. },
						  { sin( roll ), cos( roll ), 0. },
						  { 0., 0., 1. } };

	// Инициализация орбитальной модели.
	/*
	    tt = fNIP.fYearTime - 1./(360.*1440.);
	    year = fNIP.fYear;
	    if( tt < 0 ){
	        tt += 364.;
	        year--;
	        if( isLeapYear(year) ) tt += 1.;
	    }
	*/

	tt = dov = 0.;
	for( i = 0; i < (int)fNIP.fScans + 2; i++, tt += 1./(360.*1440.), dov += D_OV ) {
		se[i].sin_dov = sin( dov );
		se[i].cos_dov = cos( dov );
		fOM->model( tt );
		v_cpy( se[i].sat_pos, fOM->r );
		// Вычисление матрицы msi.
		v_mul( fOM->r, -1./v_len(fOM->r), msi[0] );
		double v[3];
		v_norm( fOM->v, v );
		v_vmul( v, msi[0], msi[1] );
		v_norm( msi[1], msi[1] );
		v_vmul( msi[0], msi[1], msi[2] );
		// Вычисление угла a.
		double cosa = v_smul( msi[2], v );
		if( cosa > 1. )
			cosa = 1.;  // этот код необходим из-за погрешностей машинных вычислений
		double sina = sqrt( 1. - cosa*cosa );
		if( sina > 1.e-10 ) {
			double d[3];
			v_vmul( msi[2], v, d );
			if( v_smul( d, msi[1] ) < 0. )
				sina = -sina;
		} else {
			sina = 0.;
			cosa = 1.;
		}
		double mal[3][3] = {
							{ cosa, 0., sina },
							{ 0., 1., 0. },
							{ -sina, 0., cosa } };
		// Вычисление матрицы ma, maT = msi * myp * malT * mr * ma.
		double mm1[3][3];
		for( m=0; m<3; m++) {
			for( n=0; n<3; n++) {
				double t = 0.;
				for( k=0; k<3; k++)
					t += mr[m][k]*mal[k][n];
				mm1[m][n] = t;
			}
		}
		double mm2[3][3];
		for(m=0;m<3;m++) {
			for(n=0;n<3;n++) {
				double t=0.;
				for(k=0;k<3;k++)
					t += mal[k][m]*mm1[k][n];
				mm2[m][n] = t;
			}
		}
		for(m=0;m<3;m++) {
			for(n=0;n<3;n++) {
				double t = 0.;
				for(k=0;k<3;k++)
					t += myp[m][k]*mm2[k][n];
				mm1[m][n]=t;
			}
		}
		for(m=0;m<3;m++) {
			for(n=0;n<3;n++) {
				double t=0.;
				for(k=0;k<3;k++)
					t+=msi[k][m]*mm1[k][n];
				ma[n][m] = t;
			}
		}
		v_cpy( se[i].dir_v, ma[1] );
		v_cpy( se[i].scan_n, ma[2] );
		se[i].scan_d = -v_smul( se[i].sat_pos, se[i].scan_n );

		// Вычисление подспутниковой точки.
		{
			double A,B,C,R,b[3];
			double r[3] = { fOM->r[0]/Ea2, fOM->r[1]/Ea2, fOM->r[2]/Eb2 };
			v_add(r,r,b);
			C = v_smul(r,fOM->r)-1.;
			A = (ma[0][0]*ma[0][0] + ma[0][1]*ma[0][1])/Ea2 + ma[0][2]*ma[0][2]/Eb2;
			B = v_smul( b, ma[0] );
			R = -(B + sqrt( B*B - 4.*A*C ))/(A+A);
			v_mul( ma[0], R, r );
			v_add( fOM->r, r, se[i].sub_p );
		}
	}
	prev_lat = 100.;    // невозможное значение
}


int TInverseReferencer::ll2xy( double lon, double lat, double *x, double *y ) {
	double t1[3], t2[3];
	double sigma;
	int u1, u2, um;
	double r1[3], r2[3], rm[3];
	double r0[3];
	double s1, s2, sm;
	double xx, yy;

	u1 = 0;
	u2 = fNIP.fScans+1;
	/* Переход к геоцентрической широте. */
	if( prev_lat != lat ) {
		prev_lat = lat;
		gc_lat = atan( tan( lat ) * (1-Ee2) );
		sin_lat = sin( gc_lat );
		cos_lat = cos( gc_lat );
		ER = Ea * sqrt( 1 - Ee2*sin_lat*sin_lat );
		VR = ER * cos_lat * D_OV;
	}
	r1[0] = r0[0] = ER * cos_lat * cos( lon + fOM->ov0 );
	r1[1] = r0[1] = ER * cos_lat * sin( lon + fOM->ov0 );
	rm[2] = r2[2] = r1[2] = r0[2] = ER * sin_lat;
	r2[0] = r0[0]*se[u2].cos_dov - r0[1]*se[u2].sin_dov;
	r2[1] = r0[1]*se[u2].cos_dov + r0[0]*se[u2].sin_dov;
	if( (s1 = v_smul(r1,se[u1].scan_n) + se[u1].scan_d) <= 0 )
		return 0;
	if( (s2 = v_smul(r2,se[u2].scan_n) + se[u2].scan_d) >= 0 )
		return 0;

	//--------------------------------------------------------------------------
	// Пристрелка перед дихотомией. Мы получаем u1 <= uu <= u2, где uu -
	// истинный скан, причем u1 и u2 достаточно мало отличаются от него.
	//--------------------------------------------------------------------------
	// Ищем приближение для u1.
	u1 += int(s1/(VR+1.1));
	r1[0] = r0[0]*se[u1].cos_dov - r0[1]*se[u1].sin_dov;
	r1[1] = r0[1]*se[u1].cos_dov + r0[0]*se[u1].sin_dov;
	s1 = v_smul( r1, se[u1].scan_n ) + se[u1].scan_d;
	// Ищем приближение для u2.
	u2 -= int(fabs(s2)/(VR+1.1));
	r2[0] = r0[0]*se[u2].cos_dov - r0[1]*se[u2].sin_dov;
	r2[1] = r0[1]*se[u2].cos_dov + r0[0]*se[u2].sin_dov;
	s2 = v_smul( r2, se[u2].scan_n ) + se[u2].scan_d;

	//--------------------------------------------------------------------------
	// Цикл точного определения скана при помощи дихотомического поиска.
	//--------------------------------------------------------------------------
	while( u2 - u1 > 1 ) {
		um = (u1 + u2) >> 1;
		rm[0] = r0[0]*se[um].cos_dov - r0[1]*se[um].sin_dov;
		rm[1] = r0[1]*se[um].cos_dov + r0[0]*se[um].sin_dov;
		sm = v_smul( rm, se[um].scan_n ) + se[um].scan_d;
		if( sm >= 0 ) {
			u1 = um;
			s1 = sm;
			r1[0] = rm[0];
			r1[1] = rm[1];
		} else {
			u2 = um;
			s2 = sm;
			r2[0] = rm[0];
			r2[1] = rm[1];
		}
	}

	if( u1 == 0 || u1 == (int)fNIP.fScans+1 )
		return 0;
	//TODO А нормально ли это приводить ulong к int
	yy = double(u1)  + s1 / (s1 + fabs(s2));
	//--------------------------------------------------------------------------
	// Определение позиции в скане.
	//--------------------------------------------------------------------------
	// Проецируем точку на плоскость сканирования.
	r1[0] -= s1*se[u1].scan_n[0];
	r1[1] -= s1*se[u1].scan_n[1];
	r1[2] -= s1*se[u1].scan_n[2];
	// Определяем угол.
	v_sub( r1, se[u1].sat_pos, t1 );
	v_sub( se[u1].sub_p, se[u1].sat_pos, t2 );
	sigma = acos( v_smul(t1,t2) / sqrt(v_smul(t1,t1)*v_smul(t2,t2)) );
	xx = sigma/(maxScanAngle / 1023.5);
	if( xx > 1024. )
		return 0;
	//  смотрим, в какую сторону от направления на подспутниковую точку надо откладывать угол
	//  при этом надир проходит через точку 1024.0, центром первого пиксела скана является точка 0.5,
	//  центром последнего пиксела скана является точка 2047.5
	v_sub( r1, se[u1].sub_p, t1 );
	if( v_smul( t1, se[u1].dir_v ) >= 0 )
		xx += 1024.;
	else
		xx = 1024. - xx;

	*x = xx;
	*y = yy - 1.;
	return 1;
}


int TInverseReferencer::ll2xy( double lon, double lat, int *x, int *y ) {
	double xx, yy;
	int r = ll2xy( lon, lat, &xx, &yy );
	if( r == 1 ) {
		*x = int(xx);
		*y = int(yy);
	}
	return r;
}
