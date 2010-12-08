/*-----------------------------------------------------------------------------
    torbmod.cpp
-----------------------------------------------------------------------------*/
#include "orbconst.h"

#include <math.h>
#include <orbmodel.hpp>
#include <y_util.hpp>

#ifdef DEBUG
//#include <conio>
//#include <iostream>
//#include <string>
//using namespace std;

#include <stdio.h>
#include <strings.h>
#endif
//double dinpi( double );

TOrbitalModel::TOrbitalModel( const TIniSatParams& iniSatParams, int baseYear, double baseYearTime, const TCorrectionParams & c ) :
		fIniBaseYear( baseYear ),
		fIniBaseYearTime( baseYearTime ),
		fBaseYear( baseYear ),
		fBaseYearTime( baseYearTime ),
		isp( iniSatParams ),
		cop( TCorrectionParams() ),
		fModel( SGP8 ) 
{
	ov0 = ovenAngle( fBaseYear, fBaseYearTime );
	ini_sgp8();
	setCorrectionParams( c );
#ifdef DEBUG
	//printf( "PI: %12.10lf \n", PI );
#endif
}

void TOrbitalModel::setCorrectionParams( const TCorrectionParams & c ) {
	if( cop == c )
		return;

	cop.roll = c.roll;
	cop.pitch = c.pitch;
	cop.yaw = c.yaw;

	cop.dm0 = c.dm0;
	cop.draan0 = c.draan0;

	// все параметры, которые требуют реинициализации орбитальной модели, оказались равны
	if( cop.fVersion == c.fVersion && cop.fTime == c.fTime && cop.fTBUSTime == c.fTBUSTime &&
			cop.di0 == c.di0 && cop.de0 == c.de0 && cop.dn0 == c.dn0 && cop.dw0 == c.dw0  ) {
		return;
	}

	// нужно увеличить время с проверкой на переход через границу года и пересчитать угол на Овна
	if( cop.fTime != c.fTime || cop.fTBUSTime != c.fTBUSTime && (cop.fVersion == 1 || cop.fVersion == 2) ) {
		fBaseYear = fIniBaseYear;
		fBaseYearTime = fIniBaseYearTime + (cop.fTime + cop.fTBUSTime)/86400.;
		// Отслеживание перехода через границу года. Полагаем, что коррекция по времени имеет величину порядка секунды.
		if( fBaseYearTime < 0. ) {       // переход через границу года назад
			fBaseYear--;
			fBaseYearTime += 364.;
			if( isLeapYear( fBaseYear ) )
				fBaseYearTime += 1.;
		} else {                           // отслеживание перехода через границу года вперед
			if( isLeapYear( fBaseYear ) ) {
				if( fBaseYearTime >= 366. ) {
					fBaseYear++;
					fBaseYearTime -= 366.;
				}
			} else {
				if( fBaseYearTime >= 365. ) {
					fBaseYear++;
					fBaseYearTime -= 365.;
				}
			}
		}

		ov0 = ovenAngle( fBaseYear, fBaseYearTime );    // пересчет угла на Овна с учетом проведенной коррекции времени
	}

	cop.fTime = c.fTime;
	cop.fTBUSTime = c.fTBUSTime;

	if( cop.fVersion != c.fVersion || cop.di0 != c.di0 || cop.de0 != c.de0 || cop.dn0 != c.dn0 || cop.dw0 != c.dw0 ) {
		cop.fVersion = c.fVersion;
		cop.di0 = c.di0;
		cop.de0 = c.de0;
		cop.dn0 = c.dn0;
		cop.dw0 = c.dw0;

		switch( cop.fVersion ) {
		case 0:
			fModel = SGP8;
			break;
		case 1:
			fModel = SGP8;
			break;
		case 2:
			fModel = SGP4;
			break;
		default:
			fModel = SGP4;
			break;
		}

		switch( fModel ) {
		case SGP4:
			ini_sgp4();
			break;
		case SGP8:
			ini_sgp8();
			break;
		}
	}
}

/*
	TOrbitalModel::setModel
	Устанавливает текущую орбитальную модель и сбрасывает параметры коррекции.
	Параметры:
		TOrbitalModel::OrbitalModel model - орбитальная модель (SGP8 || SGP4)
	19 января 2009
*/
void TOrbitalModel::setModel( /*TOrbitalModel::*/OrbitalModel model ){
	fModel = model;
	cop = TCorrectionParams();
	fBaseYear = fIniBaseYear;
	fBaseYearTime = fIniBaseYearTime;
	ov0 = ovenAngle( fBaseYear, fBaseYearTime );
#ifdef DEBUG
		if (model == SGP4){
			printf("select sgp4\n");
		}
		else printf("select sgp8\n");
#endif
	switch( fModel ) {
		case SGP4:
			ini_sgp4();
			break;
		case SGP8:
			ini_sgp8();
			break;
	}
}

void TOrbitalModel::model( double rel_time ) {
	switch( fModel ) {
	case SGP4:
		sgp4( rel_time );
		break;
	case SGP8:
		sgp8( rel_time );
		break;
	}
}

void TOrbitalModel::ini_sgp4f() {
}

void TOrbitalModel::ini_sgp4() {
#ifdef DEBUG
	printf("ini_sgp4()\n");
#endif
	double c_i0 = isp.i0 + cop.di0;
	double c_e0 = isp.e0 + cop.de0;
	double c_n0 = isp.n0 + cop.dn0;
	double c_w0 = isp.w0 + cop.dw0;

	teta = cos( c_i0 );
	double teta2 = teta*teta;
	double teta3 = teta2*teta;
	double teta4 = teta3*teta;

	double e02 = c_e0*c_e0;

	double beta2 = 1.0 - e02;
	beta = sqrt( beta2 );
	double beta3 = beta*beta2;

	{
		a0 = pow( Ke/c_n0, TW_TH );
		double df = TK2*(3.0*teta2-1.0)/beta3;
		double del = df/(a0*a0);
		double dd = del*del;
		dd = 134.0*del*dd/81.0 + dd + del/3.0;
		a0 *= 1.0-dd;
		del = df/(a0*a0);
		n0 = c_n0 / (1.0+del);
		a0 /= 1.0-del;
	}
	//double a02 = a0*a0;

	ksi = 1. / (a0 - Ser);
	etta = a0 * c_e0 * ksi;
	double etta2 = etta * etta;
	double etta3 = etta * etta2;
	double etta4 = etta2 * etta2;

	{
		double t1 = Qms4 * pow(ksi,4) * pow(1.-etta2,-3.5);
		c2 = t1 * n0 * (a0 * (1. + 1.5*etta2 + 4.*c_e0*etta + c_e0*etta3) + 1.5*K2*ksi/(1.-etta2)*(-.5 + 1.5*teta2)*(8. + 24.*etta2 + 3.*etta4));
		c1 = isp.bstar * c2;
		c3 = Qms4 * pow(ksi,5) * A30K2 * n0 * Ea * sin(c_i0)/c_e0;
		t1 *= 2. * a0 * beta2;
		c4 = t1 * n0 * ( (2.*etta*(1.+c_e0*etta) + .5*(c_e0+etta3)) - 2.*K2*ksi/(a0*(1.-etta2)) *
						 (3.*(1.-3.*teta3)*(1. + 1.5*etta2 - 2.*c_e0*etta - .5*c_e0*etta3) + .75*(1-teta2)*(2.*etta2 - c_e0*etta - c_e0*etta3)*cos(2.*c_w0)) );
		c5 = t1 * (1. + 11.*.25*etta*(etta+c_e0) + c_e0*etta3);
	}

	d2 = 4.*a0*ksi*c1*c1;
	d3 = 4./3.*a0*pow(ksi,2)*(17.*a0+Ser)*pow(c1,3);
	d4 = 2./3.*a0*pow(ksi,3)*(221.*a0+31.*Ser)*pow(c1,4);

	// вычисление dmdt, dwdt, domdt
	{
		double t1 = K2 / (pow(a0,2)*pow(beta,3));
		double t2 = pow(K2,2) / (pow(a0,4)*pow(beta,7));
		double t3 = K4 / (pow(a0,4)*pow(beta,8));
		dmdt = (1. + t1*1.5*(-1.+3.*teta2) + t2*3.*(13.-78.*teta2+137.*teta4)/16.)*n0;
		dwdt = (-t1*1.5*(1.-5.*teta2)/beta + t2*3.*(7.-114.*teta2+395.*teta4)/(16.*beta) + t3*1.25*(3.-36.*teta2+49.*teta4))*n0;
		domdt = (-t1*3.*teta/beta + t2*1.5*(4.*teta-19.*teta3)/beta + t3*2.5*teta*(3.-7.*teta2))*n0;
	}
}

void TOrbitalModel::ini_sgp8() {
#ifdef DEBUG
	printf("ini_sgp8()\n");
#endif
	double a02,dd,df,del,k1,kt,ksi,et1,et2,et3,et4,ps2,b1,b2,b3,c0,c1,c2,c3,d1,d2,d3,d4,d5;

	double c_i0 = isp.i0 + cop.di0;
	double c_e0 = isp.e0 + cop.de0;
	double c_n0 = isp.n0 + cop.dn0;
	double c_w0 = isp.w0 + cop.dw0;

	i = c_i0; // в модели SGP8 наклонение орбиты считается постоянным

	n0 = c_n0;
	a0 = pow( Ke/n0, TW_TH );
	double teta = cos( c_i0 );
	double teta2 = teta*teta;
	double teta4 = teta2*teta2;
	double e02 = c_e0*c_e0;
	double be2 = 1.0 - e02;
	double be1 = sqrt( be2 );
	double be3 = be1*be2;
	df = TK2*(3.0*teta2-1.0)/be3;
	del = df/(a0*a0);
	dd = del*del;
	dd = 134.0*del*dd/81.0 + dd + del/3.0;
	a0 *= 1.0-dd;

	del = df/(a0*a0);
	n0 /= 1.0+del;
	a0 /= 1.0-del;
	a02 = a0*a0;

	k1 = n0*TK2/(a02*be3);
	mf = k1*(3.0*teta2 - 1.0);
	k1 /= be1;
	wf = k1*(5.0*teta2 - 1.0);
	omf = -2.0*k1*teta;
	k1 *= K2/(8.0*a02*be3);
	ms = k1*(13.0 - 78.0*teta2 + 137.0*teta4);
	k1 /= be1;
	kt = be2*be2;
	kt = TK4*n0/(a02*a02*kt*kt);
	ws = k1*(7.0-114.0*teta2+395.0*teta4) + kt*(3.0-36.0*teta2+49.0*teta4);
	oms = 8.0*k1*teta*(4.0-19.0*teta2) + 2.0*kt*teta*(3.0-7.0*teta2);

	d1 = a0*be2;
	ksi = 1.0/(d1-Ser);
	et1 = c_e0 * Ser * ksi;
	et2 = et1*et1;
	et3 = et1*et2;
	et4 = et2*et2;
	ps2 = 1.0-et2;
	d5 = ksi/ps2;
	df = 1.0 + e02;
	c0 = d5*d5;
	c0 = c0*c0*sqrt(ps2);
	c0 = isp.bstar * Qms4 * n0 * a0 * c0 / sqrt(df);
	c1 = 1.5*n0*df*df*c0;
	d1 = d5/d1;
	d2 = 12.0 + 36.0*et2 + 4.5*et4;
	d3 = 15.0*et2 + 2.5*et4;
	d4 = 5.0*et1 + 3.75*et3;
	b1 = K2*(3.0*teta2 - 1.0);
	b2 = K2*(teta2 - 1.0);
	b3 = A30K2 * sin( c_i0 );
	c2 = d1*d3*b2;
	c3 = d4*d5*b3;
	df = 5.0*c_e0*et3 + 3.0*et2 + 34.0*e02*et2;
	df = 8.5*e02 + 20.0*c_e0*et1 + df;
	dd = c_w0;
	df += d1*d2*b1 + c2*cos(2.0*dd) + c3*sin(dd);
	dn = c1*(2.0+df);
	dd = dn/n0;
	de = TW_TH*dd*(c_e0 - 1.0);
}

void TOrbitalModel::sgp4f( double rel_time ) {
}

void TOrbitalModel::sgp4( double rel_time ) {
#ifdef DEBUG
	printf("sgp4()\n");
#endif
	double c_m0 = isp.m0 + cop.dm0;
	double c_w0 = isp.w0 + cop.dw0;
	double c_e0 = isp.e0 + cop.de0;
	double c_i0 = isp.i0 + cop.di0;
	double c_raan0 = isp.raan + cop.draan0;

	fRelTime = rel_time;
	// время от телеграммы (мин.)
	double tt = 1440.*(fRelTime + timeBetween( isp.fEpochYear, isp.fEpochYearTime, fBaseYear, fBaseYearTime ));
	double tt2 = tt*tt;
	double tt3 = tt2*tt;
	double tt4 = tt3*tt;
	double tt5 = tt4*tt;

	ov = dinpi( ov0 + D_OV*fRelTime*1440.*360. );

	double mdf = c_m0 + dmdt*tt;
	double wdf = c_w0 + dwdt*tt;
	double omdf = c_raan0 + domdt*tt;
	double del_w = isp.bstar*c3*cos(c_w0)*tt;
	double del_m = -2./3.*Qms4*isp.bstar*pow(ksi,4)*Ea/(c_e0*etta)*(pow(1.+etta*cos(mdf),3) - pow(1.+etta*cos(c_m0),3));
	double mp = dinpi( mdf + del_w + del_m );
	// аргумент перигея
	w = dinpi( wdf - del_w - del_m );
	// восходящий узел
	om = dinpi( omdf - 10.5*n0*K2*teta*c1*tt2/(a0*a0*beta*beta) );
	// эксцентриситет
	e = c_e0 - isp.bstar*c4*tt - isp.bstar*c5*(sin(mp) - sin(c_m0));
	// большая полуось
	a = a0*pow( 1. - c1*tt - d2*tt2 - d3*tt3 - d4*tt4, 2 );
	double beta2 = 1. - e*e;
	n = Ke / pow( a, 1.5 );
	double l = mp + w + om + n0 * (1.5*c1*tt2 + (d2+2.*c1*c1)*tt3 + .25*(3.*d3 + 12.*c1*d2 + 10.*pow(c1,3))*tt4 + .2*(3.*d4 + 12.*c1*d3 + 6.*d2*d2 + 30.*c1*c1*d2 + 15.*pow(c1,4))*tt5);

	double axn = e * cos(w);
	double aynl = A30K2 * .25 * sin(c_i0) / (a * beta2);
	double ayn = e*sin(w) + aynl;
	double ll = aynl * .5 * axn * (3.+5.*teta) / (1.+teta);
	double lt = l + ll;

	// итеративное вычисление ew
	double ew = lt - om;
	{
		double U = ew;
		double dew;
		do {
			dew = (U - ayn*cos(ew) + axn*sin(ew) - ew) / (1. - ayn*sin(ew) - axn*cos(ew));
			ew += dew;
		} while( fabs(dew) > 1e-8 );
	}

	double ecose = axn*cos(ew) + ayn*sin(ew);
	double esine = axn*sin(ew) - ayn*cos(ew);
	double el2 = axn*axn + ayn*ayn;
	double pl = a * (1. - el2);
	R = a * (1. - ecose);
	double drdt = Ke * sqrt(a) * esine / R;
	double rdfdt = Ke * sqrt(pl) / R;
	double cosu, sinu;
	{
		double t = esine / (1. + sqrt(1 - el2));
		cosu = a * (cos(ew) - axn + ayn*t) / R;
		sinu = a * (sin(ew) - ayn - axn*t) / R;
	}
	double u = (sinu >= 0) ? acos(cosu) : 2.*PI - acos(cosu);

	double delr, delu, delom, deli, deldrdt, delrdfdt;
	{
		double sin2u = sin(u+u);
		double cos2u = cos(u+u);
		double t1 = K2 / pl;
		double t2 = t1 / pl;
		double teta2 = teta*teta;
		delr = .5 * t1 * (1. - teta2) * cos2u;
		delu = -.25 * t2 * (7.*teta2 - 1.) * sin2u;
		delom = 1.5 * t2 * teta * sin2u;
		deli = 1.5 * t2 * teta * sin(c_i0) * cos2u;
		deldrdt = -t1 * n * (1. - teta2) * sin2u;
		delrdfdt = t1 * n * ((1. - teta2) * cos2u - 1.5 * (1. - 3.*teta2));
	}

	u += delu;
	om += delom;
	i = c_i0 + deli;
	R *= 1. - 1.5 * K2 * sqrt(1.-el2) * (3.*teta*teta-1.) / (pl*pl);
	R = (R + delr) * Ea;
	drdt = (drdt + deldrdt) * Ea;
	rdfdt = (rdfdt + deldrdt) * Ea;

	cosu = cos(u);
	sinu = sin(u);

	double ux = -sin(om)*cos(i)*sinu + cos(om)*cosu;
	double uy = cos(om)*cos(i)*sinu + sin(om)*cosu;
	double uz = sin(i)*sinu;

	double vx = -sin(om)*cos(i)*cosu - cos(om)*sinu;
	double vy = cos(om)*cos(i)*cosu - sin(om)*sinu;
	double vz = sin(i)*cosu;

	r[0] = R*ux;
	r[1] = R*uy;
	r[2] = R*uz;

	v[0] = drdt*ux + rdfdt*vx;
	v[1] = drdt*uy + rdfdt*vy;
	v[2] = drdt*uz + rdfdt*vz;
}

void TOrbitalModel::sgp8( double rel_time ) {
#ifdef DEBUG
	printf("sgp8()\n");
#endif
	double tt,dd,sf,cf,u,rd,rf,dr,drd,drf,du,dla,lam,y4,y5,be1,be2,su,cu,s2u,c2u,si,ci,si05,ci05;

	double c_m0 = isp.m0 + cop.dm0;
	double c_w0 = isp.w0 + cop.dw0;
	double c_e0 = isp.e0 + cop.de0;
	double c_i0 = isp.i0 + cop.di0;
	double c_raan0 = isp.raan + cop.draan0;

	fRelTime = rel_time;
	tt = 1440.*(fRelTime + timeBetween( isp.fEpochYear, isp.fEpochYearTime, fBaseYear, fBaseYearTime ));
	ov = dinpi( ov0 + D_OV*fRelTime*1440.*360. );
	{
		double t2 = .5 * dn * tt * tt;
		double t1 = SE_TH * t2 / n0 + tt;
		om = dinpi(c_raan0 + omf*t1 + oms*tt);
		w = dinpi(c_w0 + wf*t1 + ws*tt);// Аргумент перигея.
		m = dinpi(c_m0 + n0*tt + mf*t1 + ms*tt + t2);// Средняя аномалия.
	}
	n = n0 + dn*tt; // Среднее движение.
	e = c_e0 + de*tt;
	double E = m + e*(sin(m) + e*sin(2.0*m)/2.0);
	// Вычисление эксцентрической аномалии по средней.
	{
		double dE;
		do {
			dE = ((m-E) + e*sin(E))/(1.0 - e*cos(E));
			E += dE;
		} while( fabs(dE) > 1.0e-8 );
	}

	a = pow( Ke/n, TW_TH );
	be2 = 1. - e*e;
	be1 = sqrt(be2);
	{
		double t1 = 1. - e*cos(E);
		sf = be1*sin(E)/t1;
		cf = (cos(E)-e)/t1;
	}
	double f = (sf >= 0) ? acos(cf) : 2.*PI-acos(cf);   // Истинная аномалия.
	u = dinpi(f+w);

	su = sin(u);
	cu = cos(u);
	s2u = sin(u+u);
	c2u = cos(u+u);
	si = sin( c_i0 );
	ci = cos( c_i0 );
	dd = 1.0 + e*cf;
	R = a*be2/dd;
	rd = n*a*e*sf/be1;
	rf = n*a*a*be1/R;
	{
		double t1 = a*be2;
		y4 = 2.70654e-4 / t1;
		y5 = 1.0 - ci*ci;
		double t = K3*si;
		dr = y4*(y5*c2u + 3.0 - 9.0*ci*ci) - t*su;
		lam = a/R;
		dla = lam*lam;
		drd = -n*dla*(2.0*y4*y5*s2u + t*cu);
		y4 = y4 / t1;
		y5 = K3 / t1;
	}
	double di = ci*(3.0*y4*si*c2u - y5*e*sin(w));
	drf = n*(a*lam*si*di/ci - dla*dr);
	lam = 1.0 - 5.0*ci*ci;
	dla = lam - 2.0*ci*ci;
	be1 = (f-m) + e*sf;
	du = y4*(0.5*dla*s2u - 3.0*lam*be1);
	si05 = sin(.5 * c_i0);
	ci05 = cos(.5 * c_i0);
	dd = (dd+1.0)*cu;
	du -= y5*(si*dd + 0.5*ci*ci*e*cos(w)/(si05*ci05));
	dla = y4*(0.5*(dla + 6.0*ci)*s2u - 3.0*(lam + 2.0*ci)*be1) + y5*si*(e*cos(w)*ci/(1.0 + ci) - dd);
	R = (R + dr)*Ea;
	rd = (rd + drd)*Ea;
	rf = (rf + drf)*Ea;
	//07-08-2006
	//Исправлен знак перед последним членом y5
	//y4 = si05*su + (cu*si05*du + 0.5*su*ci05*di);
	//y5 = si05*cu - (su*si05*du + 0.5*cu*ci05*di);
	y4 = si05*su + cu*si05*du + 0.5*su*ci05*di;
	y5 = si05*cu - su*si05*du + 0.5*cu*ci05*di;
	lam = dinpi(u+om+dla);
	su = sin(lam);
	cu = cos(lam);

	double ux, uy, uz, vx, vy, vz;
	{
		double t1 = sqrt(1.0 - y4*y4 - y5*y5);
		double t2 = 2.0*y4;
		double t3 = 2.0*y5;
		ux = t2*(y5*su - y4*cu) + cu;
		uy = -t2*(y5*cu + y4*su) + su;
		uz = t2*t1;
		vx = t3*(y5*su - y4*cu) - su;
		vy = -t3*(y5*cu + y4*su) + cu;
		vz = t3*t1;
	}

	v[0] = rd*ux + rf*vx;
	v[1] = rd*uy + rf*vy;
	v[2] = rd*uz + rf*vz;

	r[0] = R*ux;
	r[1] = R*uy;
	r[2] = R*uz;
}

double TOrbitalModel::ovenAngle( int year, double year_time ) {
	//Todo необходимо перейти на правильное равноденствие J2000
	int n = (int)year_time;
	int j = year - 1983;
	double day_frac = year_time - double(n);
	n += (j+2)/4;
	double df = double( 365*j + n );
	double dd = dinpi( 0.17202791e-1 * df ) - 7.67945e-5*sin( dinpi( 1.638168 - dinpi( 9.242202558e-4 * df ) - 24.0*3.8397e-5*day_frac ) );
	return dinpi( dd + 1.746778922 + 24.0*0.262516145*day_frac );

//	/* Вычисление this->ov0 */
//	day_frac = modf( baseYearTime, &n );
//	j = baseYear - 1983;
//	n += (double)((j+2)/4) - 1.;
//	df = (double)j*365. + n;
//	dd = dinpi( 9.242202558e-4 * df );
//	d1 = dinpi( 1.638168 - dd - 24.0*3.8397e-5*day_frac );
//	dd = dinpi( 0.17202791e-1 * df ) - 7.67945e-5*sin(d1);
//	this->ov0 = dinpi( dd + 1.746778922 + 24.0*0.262516145*day_frac );

/* 15 august 2006, Громов А.В. */
	/* вычисление направления на Овен, то есть звездного времени
	Гринвича для заданного момента времени UTC заменено на новые
	функции, вычисляющие его относительно эпохи J2000 */
//	dd = mjd1 ((int)baseYear, baseYearTime );
//	this->ov0 = gmst( dd ) * 2.0 * PI;
}

//  Функция рассматривает получаемое значение как угол в радианах и
//  возвращает соответствующий ему из интервала [0, 2PI).
double dinpi( double arc ) {
	return (arc - floor(arc/(2.*PI))*2.*PI);
}

