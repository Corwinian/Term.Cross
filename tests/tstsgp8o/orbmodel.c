/*--------------------------------------------------------------------------
	ORBMODEL.C
	Вся орбитальная модель ( sgp8 ), оформленная в виде одного файла.
	Предназначена для изготовления *.obj в различных 32-битных
	операционных системах с помощью различных компиляторов.
	Date: 2 december 2003
	Date: 6 august 2006
		Исправлены определения глобальных констант;
		Исправлена ошибка в SGP8 ( знак перед последним членом y5 ).
--------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include "f34_uni.h"
#include "orbmod2.h"

/*---------------------------------------------------------------------
	TIniSatParams_1
	Функция заполняет структуру TIniSatParams данными
	NORAD-телеграммы из 0-блока нового формата.
	Прототип:
		void TIniSatParams_1( TIniSatParams* isp, struct TBlk0_uni* b );
	Параметры:
	isp	Указатель на инициализируемую структуру.
	b	Указатель на структуру 0-блока нового формата
		с заполненными полями NORAD.
---------------------------------------------------------------------*/
void TIniSatParams_1( TIniSatParams* isp, struct TBlk0_uni* b )
{
	isp->year = b->ep_year;
	isp->day = b->ep_day;
	isp->n0 = b->n0;
	isp->bstar = b->bstar;
	isp->i0 = b->i0;
	isp->raan = b->raan;
	isp->e0 = b->e0;
	isp->w0 = b->w0;
	isp->m0 = b->m0;
}

/*-------------------------------------------------------------------------
	TNOAAImageParams_1
	Функция, инициализирующая поля структуры TNOAAImageParams
	по 0-блоку нового формата.
        Вычисление поля ascendFlag производится при помощи орбитальной 
	модели,	для чего используется внутренняя структура isp.
	Прототип:
	void TNOAAImageParams_1( TNOAAImageParams* nip, struct TBlk0_uni* blk0 );
	Параметры:
	nip	Инициализируемая структура.
	blk0	Указатель на 0-блок нового формата. 0-блок должен
		содержать корректную информацию NORAD.
---------------------------------------------------------------------------*/
void TNOAAImageParams_1( TNOAAImageParams* nip, struct TBlk0_uni* b )
{
	TSatParams sat;
	TIniSatParams isp;

	nip->year = b->s_year;
	nip->yearTime = (double)b->s_day + (double)b->s_time / 86400000.0;
	nip->scans = b->frame_c + b->frame_lost;
	TIniSatParams_1 ( &isp, b );
	iniSGP8( &sat, &isp, nip->year, nip->yearTime, 0 );
	SGP8( &sat, &isp, 0.0 );
	nip->ascendFlag = ( sat.v[2] > 0.0 )? 1: 0;
}

/*--------------------------------------------------------------------------
	iniSGP8
	Функция, заполняющая постоянную часть структуры TSatParams.
	Параметры baseYear и baseYearTime определяют базовый момент времени.
	Прототип:
	void iniSGP8( TSatParams *sat, TIniSatParams *isp,
		unsigned short baseYear, double baseYearTime, TCorrectionParams *cop )
	Параметры:
	sat		Указатель на заполняемую структуру.
	isp		Прогнозные орбитальные параметры, которые будут
			использоваться для вычислений.
	baseYear	Год ( полный )
	baseYearTime	Время в году, в днях (1-based).
	cop		Указатель на структуру, содержащую значения
			корректирующих параметров. Из всех полей структуры
			используется только fTime.
			Может быть NULL.
--------------------------------------------------------------------------*/
void iniSGP8( TSatParams* this, TIniSatParams* isp, unsigned short baseYear,
        double baseYearTime, TCorrectionParams* cop )
{
	unsigned short j;
	double n, day_frac;
	double a02,e02,dd,df,del,tet,te2,te4;
	double be1,be2,be3,k1,kt,ksi,et1,et2,et3,et4,ps2;
	double b1,b2,b3,c0,c1,c2,c3,d1,d2,d3,d4,d5;

	/* Проведение коррекции времени. */
	if ( cop ) {
		baseYearTime += cop->fTime / 86400.;
		/* Отслеживание перехода через границу года, полагая при этом,
		что коррекция по времени имеет величину порядка секунды. */
		if ( baseYearTime < 1. ) {
			/* Переход через границу года назад. */
			baseYear--;
			baseYearTime += 365.;
			if ( isLeapYear( baseYear )) baseYearTime += 1.;
		}
		else {
			/* Переход через границу года вперед. */
			if ( isLeapYear( baseYear )) {
				if ( baseYearTime >= 367. ) {
					baseYear++;
					baseYearTime -= 366.;
				}
			}
			else {
				if ( baseYearTime >= 366. ) {
					baseYear++;
					baseYearTime -= 365.;
				}
			}
		}
	}
	this->eptim = timeBetween ( isp->year, isp->day,
                baseYear, baseYearTime ) * 1440.;

	this->on0 = isp->n0;
	this->a0 = pow( Ke/this->on0, TW_TH );
	tet = cos( isp->i0 );
	te2 = tet*tet;
	te4 = te2*te2;
	e02 = pow( isp->e0, 2. );
	be2 = 1.0 - e02;
	be1 = sqrt( be2 );
	be3 = be1*be2;
	df = TK2*(3.0*te2-1.0)/be3;
	del = df / pow( this->a0, 2. );
	dd = del*del;
	dd = 134.0*del*dd/81.0 + dd + del/3.0;
	this->a0 *= 1.0-dd;
	del = df / pow( this->a0, 2. );
	this->on0 /= 1.0+del;
	this->a0 /= 1.0-del;

	a02 = pow( this->a0, 2. );
	k1 = this->on0*TK2/(a02*be3);
	this->mf = k1*(3.0*te2 - 1.0);
	k1 /= be1;
	this->wf = k1*(5.0*te2 - 1.0);
	this->omf = -2.0*k1*tet;
	k1 *= K2/(8.0*a02*be3);
	this->ms = k1*(13.0 - 78.0*te2 + 137.0*te4);
	k1 /= be1;
	kt = be2*be2;
	kt = TK4*this->on0/(a02*a02*kt*kt);
	this->ws = k1*(7.0-114.0*te2+395.0*te4) + kt*(3.0-36.0*te2+49.0*te4);
	this->oms = 8.0*k1*tet*(4.0-19.0*te2) + 2.0*kt*tet*(3.0-7.0*te2);

	d1 = this->a0*be2;
	ksi = 1.0/(d1-Ser);
	et1 = isp->e0 * Ser * ksi;
	et2 = et1*et1;
	et3 = et1*et2;
	et4 = et2*et2;
	ps2 = 1.0-et2;
	d5 = ksi/ps2;
	df = 1.0 + e02;
	c0 = d5*d5;
	c0 = c0*c0*sqrt(ps2);
	c0 = isp->bstar * Qms4 * this->on0 * this->a0 * c0/ sqrt(df);
	c1 = TH_TW*this->on0*df*df*c0;
	d1 = d5/d1;
	d2 = 12.0 + 36.0*et2 + 4.5*et4;
	d3 = 15.0*et2 + 2.5*et4;
	d4 = 5.0*et1 + 3.75*et3;
	b1 = K2*(3.0*te2 - 1.0);
	b2 = K2*(te2 - 1.0);
	b3 = A3COF * sin( isp->i0 );
	c2 = d1*d3*b2;
	c3 = d4*d5*b3;
	df = 5.0*isp->e0*et3 + 3.0*et2 + 34.0*e02*et2;
	df += 8.5*e02 + 20.0*isp->e0*et1;
	dd = isp->w0;
	df += d1*d2*b1 + c2*cos(2.0*dd) + c3*sin(dd);
	this->dn = c1*(2.0+df);
	dd = this->dn / this->on0;
	this->de = TW_TH*dd*(isp->e0 - 1.0);

	/* Вычисление this->ov0. */
	day_frac = modf( baseYearTime, &n );
	j = baseYear - 1983;
	n += (double)((j+2)/4) - 1.;
	df = (double)j*365. + n;
	dd = dinpi( 9.242202558e-4 * df );
	d1 = dinpi( 1.638168 - dd - 24.0*3.8397e-5*day_frac );
	dd = dinpi( 0.17202791e-1 * df ) - 7.67945e-5*sin(d1);
	this->ov0 = dinpi( dd + 1.746778922 + 24.0*0.262516145*day_frac );
}

/*--------------------------------------------------------------------------
	dinpi
	Функция рассматривает получаемое значение как угол в радианах и
	возвращает соответствующий ему из интервала [0, 2PI).
--------------------------------------------------------------------------*/
double dinpi( double arc )
{
	return ( arc - floor( arc/(2.*PI)) * 2.* PI );
}

/*--------------------------------------------------------------------------
	SGP8
	Производит вычисление орбитальных элементов по орбитальной модели
	SGP8 на заданный момент времени относительно базового, указанного
	при вызове iniSGP8.
	Прототип:
	void SGP8( TSatParams* sat, TIniSatParams* isp, double relTime )
	Параметры:
	sat	Указатель на структуру, во вторую часть которой (см. описание
		структуры) будут помещены результаты вычислений.
	isp	Указатель на заполненную структуру прогнозных орбитальных
		параметров. Ее содержимое должно совпадать с тем, что
		было при вызове функции iniSGP8.
	relTime	Время, для которого будут просчитаны орбитальные элементы.
		Задается в минутах относительно базового.
--------------------------------------------------------------------------*/
void SGP8( TSatParams *this, TIniSatParams *isp, double relTime )
{
	double  tt,z1,a,df,dd,sf,cf,u,R,rd,rf,dr,drd,di,drf,du,dla,lam,y4,y5,
		be1,be2,su,cu,s2u,c2u,ux,uy, uz,vx,vy,vz,si,ci,si05,ci05,ci2;

	this->reltim = relTime;
	/* tt - промежуток времени между текущим моментом времени и временем
	прогнозных орбитальных параметров; выражено в минутах */
	tt = relTime + this->eptim;
	this->ov = dinpi(this->ov0 + D_OV*(relTime/AVHRR_SCAN_TIME));

	z1 = this->dn * tt * tt / 2.0;
	df = SE_TH*z1 / this->on0 + tt;
	di = this->omf*df + this->oms*tt;
	this->raan = dinpi(isp->raan + di);
	di = this->wf*df + this->ws*tt;
	this->w = dinpi(isp->w0 + di);          /* Аргумент перигея. */
	dd = this->mf*df + this->ms*tt + z1;
	si = dinpi(this->on0*tt);
	this->m = dinpi(isp->m0 + si + dd);     /* Средняя аномалия. */
	this->n = this->on0 + this->dn*tt;      /* Среднее движение. */
	this->e = isp->e0 + this->de*tt;        /* Эксцентриситет. */

	/* Вычисление эксцентрической аномалии по средней. */
	this->E = this->m + this->e*(sin(this->m) + this->e*sin(2.*this->m)/2.);
	do {
		df = ((this->m - this->E) + this->e*sin(this->E))/(1.0 - this->e*cos(this->E));
		this->E += df;
	} while ( fabs(df) > 1.e-8 );

	a = pow( Ke/this->n, TW_TH );
	be2 = 1. - pow( this->e, 2. );
	be1 = sqrt(be2);
	df = 1. - this->e*cos(this->E);
	sf = be1*sin(this->E)/df;
	cf = (cos(this->E) - this->e) / df;
	this->f = (sf >= 0.) ? acos(cf) : 2.*PI-acos(cf);   // Истинная аномалия.
	u = dinpi(this->f + this->w);

	su = sin(u);
	cu = cos(u);
	s2u = sin(u+u);
	c2u = cos(u+u);
	si = sin( isp->i0 );
	ci = cos( isp->i0 );
	ci2 = ci*ci;
	dd = 1.0 + this->e*cf;
	R = a*be2/dd;
	df = this->n * a;
	rd = df * this->e * sf / be1;
	rf = df*a*be1/R;
	df = a*be2;
/*	6 august 2006, Громов А.В.: Константа K2 */
	y4 = 0.5 * K2 / df;
	y5 = 1.0 - ci2;
	ux = K3*si;
	dr = y4*(y5*c2u + 3.0 - 9.0*ci2) - ux*su;
	lam = a/R;
	dla = lam*lam;
	drd = -this->n * dla * (2.0*y4*y5*s2u + ux*cu);
	y4 = y4/df;
	y5 = K3/df;
	di = ci*(3.0*y4*si*c2u - y5*this->e*sin(this->w));
	drf = this->n*(a*lam*si*di/ci - dla*dr);
	lam = 1.0 - 5.0*ci2;
	dla = lam - 2.0*ci2;
	be1 = (this->f - this->m) + this->e * sf;
	du = y4*(0.5*dla*s2u - 3.0*lam*be1);
	df = isp->i0 / 2.0;
	si05 = sin(df);
	ci05 = cos(df);
	df = this->e * cos(this->w);
	dd = (dd+1.0)*cu;
	du -= y5*(si*dd + 0.5*ci2*df/(si05*ci05));
	dla = y4*(0.5*(dla + 6.0*ci)*s2u - 3.0*(lam + 2.0*ci)*be1) + y5*si*(df*ci/(1.0 + ci) - dd);
	R = (R + dr)*Ea;
	rd = (rd + drd)*Ea;
	rf = (rf + drf)*Ea;
/*	6 august 2006, Громов А.В.
	Исправлена ошибка: знак перед последним членом y5
*/
	y4 = si05*su + cu*si05*du + 0.5*su*ci05*di;
	y5 = si05*cu - su*si05*du + 0.5*cu*ci05*di;
	lam = dinpi(u + this->raan + dla);
	su = sin(lam);
	cu = cos(lam);

	df = sqrt(1.0 - y4*y4 - y5*y5);

	dd = 2.0*y4;
	ux = dd*(y5*su - y4*cu) + cu;
	uy = -dd*(y5*cu + y4*su) + su;
	uz = dd*df;

	dd = 2.0*y5;
	vx = dd*(y5*su - y4*cu) - su;
	vy = -dd*(y5*cu + y4*su) + cu;
	vz = dd*df;

	this->v[0] = rd*ux + rf*vx;
	this->v[1] = rd*uy + rf*vy;
	this->v[2] = rd*uz + rf*vz;

	this->r[0] = R*ux;
	this->r[1] = R*uy;
	this->r[2] = R*uz;
}
