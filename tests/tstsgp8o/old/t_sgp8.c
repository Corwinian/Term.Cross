/*---------------------------------------------------------------------
	T_SGP8.C
	�ணࠬ�� ��� ���஢���� �ࡨ⠫쭮� ������ SGP8.
	� ����⢥ ��� �ᯮ������ ⥫��ࠬ�� ��� ����ࠪ⭮��
	��⭨��, �ਢ������� � ���㬥�� spacetrk.pdf. ��� ��
	�ਢ����� १����� ��⮢�� ���⮢, � ����묨 �����
	�ࠢ���� १����� ���� �� ������ �ணࠬ��.
	Date: 1 august 2006
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "f34_uni.h"
#include "orbmod2.h"
//#include <conio.h>

/* ���⨯� �㭪権 */
extern short isLeapYear( short );
extern short dateToDay( short, short, short );
extern void dayToDate( short, short, short*, short* );
extern double timeBetween( unsigned short, double, unsigned short, double );
extern void norad_time ( short*, double*, char* p );
extern void TBlk0_ini ( struct TBlk0_uni* b, char* p );
extern double econv ( char* p, char* s );

/* ��������� ��室���� 䠩�� */
struct TBlk0_uni b;

/* ��६���� �ࡨ⠫쭮� ������ */
TIniSatParams isp;
TSatParams sat;

/* ��⮢�� ⥫��ࠬ�� */
char t_tlg[] = "\
1 88888U          80275.98708465  .00073094  13844-3  66816-4 0     8\r\n\
2 88888  72.8435 115.9689 0086731  52.6988 110.5714 16.05824518   105\r\n\
";

/* �⠫���� ���न���� � ᪮��� ��� �������� ⥫��ࠬ�� */
double tx[5] = { 2328.87265015, 2456.04577637, 2567.68383789, 2663.49508667, 2743.29238892 };
double ty[5] = { -5995.21289063, -6071.90490722, -6112.40881348, -6115.18182373, -6078.90783691 };
double tz[5] = { 1720.04884338, 1222.84086609,713.29282379, 194.62816810, -329.73434067 };
double txdot[5] = { 2.91210661, 2.67936245, 2.43992555, 2.19525236, 1.94680957 };
double tydot[5] = { -0.98353850, -0.44820847, 0.09893919, 0.65453661, 1.21500109 };
double tzdot[5] = { -7.09081554, -7.22888553, -7.32018769, -7.36308974, -7.35625595 };

/* ���ᨢ� �⪫������ �� �⠫��� */
double dx[5], dy[5], dz[5], dxdot[5], dydot[5], dzdot[5];

/* ��६���� ��� ࠡ��� �㭪権 �६��� */
static char* month_name[13] = { "Invalid", "January", "February", "March",
	"April", "May", "June", "July", "August", "September",
	"October", "November", "December" };
static short months[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273,
304, 334, 365 };
static short leapMonths[13] = { 0, 31, 60, 91, 121, 152, 182, 213, 244,
274, 305, 335, 366 };

int main()
{
	int i, j;

	/* ���樠�����㥬 ���� norad � 0-����� */
	memset ((void*)(&b), 0, sizeof( struct TBlk0_uni ));
	b.formatType = 0xff;
	TBlk0_ini ( &b, t_tlg );

	j = strlen ( t_tlg );
	for ( i = 0; i < j; i++ ) {
		if ( t_tlg[i] == 0x0d ) continue;
		else if ( t_tlg[i] == 0x0a ) printf ( "\n" );
		else printf ( "%1c", t_tlg[i] );
	}

	/* �ࡨ⠫�� ��ࠬ���� �� 0-����� */
	printf ( "\nReference number: %ld\n", b.ref_num );
	printf ( "Ephem set number: %d\n", b.set_num );
	printf ( "Ephem type: %d\n", b.ephem );
	printf ( "Year: %d\n", b.ep_year );
	printf ( "Day: %12.8lf\n", b.ep_day );
	printf ( "Average motion n0 ( rad/s ): %12.10lf ( %11.9lf 1/day )\n", b.n0, b.n0*4.0*RD );
	printf ( "BSTAR Drag term: %12.10lf\n", b.bstar );
	printf ( "Orbit declination i0 ( rad ): %12.10lf ( %8.4lf deg )\n", b.i0, b.i0*RD );
	printf ( "Right accending raan ( rad ): %12.10lf ( %8.4lf deg )\n", b.raan, b.raan*RD );
	printf ( "Excentricity e0: %9.7lf\n", b.e0 );
	printf ( "Perigelium arg w0 ( rad ): %12.10lf ( %8.4lf deg )\n", b.w0, b.w0*RD );
	printf ( "Average anomaly m0 ( rad ): %12.10lf ( %8.4lf deg )\n\n", b.m0, b.m0*RD );

	/* ���樠������ �������� isp */
	TIniSatParams_1 ( &isp, &b );
	/* ���樠������ ������ �� epoch time */
	iniSGP8 ( &sat, &isp, b.ep_year, b.ep_day, 0 );
	/* �����뢠�� ������ �� ��⪨ ���। � 蠣�� � 6 �ᮢ */
	for ( j = 0, i = 0; j < 1441; j += 360, i++ ) {
		SGP8( &sat, &isp, (double)j );
		printf ( "%4d %13.7lf %13.7lf %13.7lf %10.7lf %10.7lf %10.7lf\n",
			j, sat.r[0], sat.r[1], sat.r[2], sat.v[0]/60.0, sat.v[1]/60.0, sat.v[2]/60.0 );

		/* ��室�� �⪫������ �� �⠫����� ���祭�� */
		dx[i] = sat.r[0] - tx[i];
		dy[i] = sat.r[1] - ty[i];
		dz[i] = sat.r[2] - tz[i];
		dxdot[i] = sat.v[0]/60.0 - txdot[i];
		dydot[i] = sat.v[1]/60.0 - tydot[i];
		dzdot[i] = sat.v[2]/60.0 - tzdot[i];
	}

	/* �ᯥ��뢠�� �⪫������ �� �⠫����� ���祭�� */
	printf ( "\n�⪫������ �� �⠫���:\n" );
	for ( i = 0; i < 5; i++ ) {
		printf ( "%4d %11.8lf %11.8lf %11.8lf %11.8lf %11.8lf %11.8lf\n",
			i*360, dx[i], dy[i], dz[i], dxdot[i], dydot[i], dzdot[i] );
	}

	return 0;
}

/*---------------------------------------------------------------------
	�㭪樨 ��� �८�ࠧ������ �६��� � ���
---------------------------------------------------------------------*/
short isLeapYear( short year )
{
        if( (year%4==0 && year%100!=0) || year%400==0 ) return 1;
        return 0;
}

short dateToDay( short year, short month, short date )
{
	short n = date + months[month-1];
	if( month>2 && isLeapYear(year) ) n++;
	return n;
}

void dayToDate( short year, short year_day, short *month, short *date )
{
	short i = 1;
	short *m = isLeapYear(year) ? leapMonths : months;
	while( m[i] < year_day ) i++;
	*month = i;
	*date = year_day - m[i-1];
}
/*--------------------------------------------------------------------------
	timeBetween
	�㭪�� ��� ���᫥��� ���ࢠ�� ����� ���� �����⠬� �६���.
	���⨯:
	double timeBetween ( unsigned short year1, double yearTime1,
			unsigned short year2, double yearTime2 );
		year1, yearTime1	��।����� ���� ������ �६���.
		year2, yearTime2	��।����� ��ன ������ �६���.
	�����頥��� ���祭��:
	���ࢠ� ����� ���� � ���� �����⮬ �६���, ��ࠦ���� � ����.
	������� > 0 , �᫨ ��ன ������ �६��� ����� ��ࢮ��.
	�������� !!! ��ࠬ���� year1 � year2 ������ 㤮���⢮���� �᫮���: 
	fabs(year2 - year1) <= 1, �.�. �㭪�� ����⠭� �� ��࠭�祭���
	�ਬ������.
--------------------------------------------------------------------------*/
double timeBetween( unsigned short year1, double yearTime1, unsigned short year2, double yearTime2 )
{
	unsigned short y1,y2;
	double yt1,yt2;
	double t;

	if ( year2 >= year1 ) {
		y2 = year2;
		yt2 = yearTime2;
		y1 = year1;
		yt1 = yearTime1;
	}
	else {
		y1 = year2;
		yt1 = yearTime2;
		y2 = year1;
		yt2 = yearTime1;
	}
	t = yt2 - yt1;
	if ( y2 > y1 ) {
		t += 365.0;
		if ( isLeapYear( y1 )) t += 1.0;
	}
	return (( year2 >= year1 )? t : -t);
}

/*---------------------------------------------------------------------
	NORAD_TIME.C
	�㭪�� �।�⠢��� �� ᥡ� ������ ���樠������ ��ࠬ��஢
	NORAD �� ⥫��ࠬ��, �ᯮ�������� � �����. ��� �ந������ ����祭��
	���� ���� ��ࠬ��஢ NORAD: ���� � �६��� �� ��砫� ���� � ��⪠�.
	�㭪�� �ᯮ������ ��� �⤥�쭮 ��� ��।������ �६��� ⥫��ࠬ��
	� �ࠢ����� � ��㣨�� �६�����, ⠪ � � ��⠢� �㭪樨 TBlk0_ini
	�� ������ ���樠����樨 0-����� �� ⥫��ࠬ�� NORAD.
	���⨯: void norad_time ( short* year, double* time,
			char* p );
	���	year	㪠��⥫� �� ��६�����, � ������ �㤥� ����ᠭ
			��� �ண����;
		time	㪠��⥫� �� ��६�����, � ������ �㤥� ����ᠭ�
			������ �६� �ண���� � ��⪠� �� ��砫� ����
			[1.0-365.99(366.99)] ( �������筮 ⥫��ࠬ�� );
		p	㪠��⥫� �� ����� ��ப� ⥫��ࠬ��.
---------------------------------------------------------------------*/
void norad_time ( short* year, double* time, char* p )
{
	short i;
	char s[16];

	/* ��� */
	memcpy ( s, p+18, 2 );
	s[2] = (char)0;
	i = (short) atoi( s );
	/* ���室 �१ 2000 ��� */
	if ( i < 57 ) i += 100;
	*year = i + CENTURY;

	/* ���� � ��� �஡��� ���� */
	memcpy ( s, p+20, 12 );
	s[12] = (char)0;
	*time = atof( s );
}

/*---------------------------------------------------------------------
	�㭪�� �ந������ ����祭�� ��ࠬ��஢ NORAD �� ⥫��ࠬ�� TLE.
	��� �����뢠�� ��ࠬ���� NORAD � 0-���� �� ���� �����, ��
	�⮬ ��⠥���, �� 㪠��⥫� ᯮ��樮��஢�� �� ���� ᨬ���
	��ࢮ� ��ப� ⥫��ࠬ��. ����⨥ � �����⨥ 䠩��, � ⠪��
	�⥭�� �� 䠩�� �ந�⥪��� ᭠�㦨 ������ �㭪樨. �஬� ⮣�,
	�㭪�� �����뢠�� � 0-���� �� ⮫쪮 �㦭� ��� SGP8 ��ࠬ����,
	�� � ��㣨� ��ࠬ���� ( ���ਬ��, ����� ���୮�� ��⪠ ).
	���⨯:
		void TBlk0_ini ( struct TBlk0_uni* b, char* p );
	���	b	㪠��⥫� �� �������� 0-�����;
		p	㪠��⥫� �� ����� ��ப� ⥫��ࠬ��;
---------------------------------------------------------------------*/
void TBlk0_ini ( struct TBlk0_uni* b, char* p )
{
	char s[16];
	short year;
	double day;

	/* ����砥� ��� � �६� */
	norad_time ( &year, &day, p );
	b->ep_year = year;
	b->ep_day = day;

	/* BSTAR drag term. */
	b->bstar = econv ( p + 53, s );

	/* ����樮���㥬 p �� ��砫� ��ன ��ப� ⥫��ࠬ�� */
	p = strchr ( p, 0x0a );
	p ++;

	/* ���������� */
	memcpy ( s, p+8, 8 );
	s[8] = (char)0;
	b->i0 = atof( s ) * DR;
	/* ���室�騩 㧥� */
	memcpy ( s, p+17, 8 );
	s[8] = (char)0;
	b->raan = atof( s ) * DR;
	/* ���業����� */
	memcpy ( s+1, p+26, 7 );
	s[0] = (char)'.';
	s[8] = (char)0;
	b->e0 = atof( s );
	/* ��ਣ�� */
	memcpy ( s, p+34, 8 );
	s[8] = (char)0;
	b->w0 = atof( s ) * DR;
	/* �।��� �������� */
	memcpy ( s, p+43, 8 );
	s[8] = (char)0;
	b->m0 = atof( s ) * DR;
	/* �।��� �������� */
	memcpy ( s, p+52, 11 );
	s[11] = (char)0;
	b->n0 = atof( s ) * 0.25 * DR;	//4.36332313e-3;

	/* ����� ��⪠ */
	memcpy ( s, p+63, 5 );
	s[5] = (char)0;
	b->ref_num = atol( s );
}

/*---------------------------------------------------------------------
	ECONV.C
	��㦥���� �㭪�� ��� �८�ࠧ������ ᨬ���쭮� ��ப� ����
	nnnnnnn�mmm � .nnnnnnnE�mmm, � ��⥬ � double
	Modification date: 8 august 2006
		�㭪�� �� ��ࠡ��뢠�� ���� ��। �����ᮩ -> ��ࠢ����.
---------------------------------------------------------------------*/
double econv ( char* p, char* s )
{
	char* t = s;

	/* �ய�᪠�� �஡��� */
	while( *p == ' ' ) p++;
	/* ��ନ஢���� ����� ������� */
	if ( *p == '-' || *p == '+' ) *t++ = *p++;
	/* ��ନ஢���� ������� */
	*t++ = '.';
	while( *p != '+' && *p != '-' ) *t++ = *p++;
	/* ��ନ஢���� ���浪� */
	*t++ = 'e';
	while( *p != ' ' ) *t++ = *p++;
	*t = (char)0;	
	return ( atof( s ));
}
