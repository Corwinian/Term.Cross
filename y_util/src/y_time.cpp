/*-------------------------------------------------------------------------
    y_time.cpp
-------------------------------------------------------------------------*/
#include "y_util/y_time.hpp"

static int months[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
static int leapMonths[13] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

int isLeapYear( int year ) {
	return ( (year%4==0 && year%100!=0) || year%400==0 ) ? 1 : 0;
}

//--------------------------------------------------------------------------
//  dateToDay1
//  Функция вычисляет номер дня в году по его дате.
//  Прототип:
//  int dateToDay( int year, int month, int date )
//  Параметры:
//  year    Год.
//  month   Месяц (1-based).
//  date    Число (1-based).
//  Возвращаемое значение: номер дня в году (1-based).
//--------------------------------------------------------------------------
int dateToDay1( int year, int month, int date ) {
	int n = date + months[month-1];
	if( month>2 && isLeapYear(year) ) n++;
	return n;
}

//--------------------------------------------------------------------------
//  dayToDate1
//  Функция вычисляет дату дня по его номеру в году.
//  Прототип:
//  void dayToDate( int year, int year_day, int *month, int *date );
//  Парметры:
//  year        Год (полный).
//  year_day    Номер дня в году (1-based).
//  Возвращаемые значения:
//  month       Номер месяца (1-based).
//  date        Дата (1-based).
//--------------------------------------------------------------------------
void dayToDate1( int year, int year_day, int *month, int *date ) {
	int i = 1;
	int *m = isLeapYear(year) ? leapMonths : months;
	while( m[i] < year_day ) i++;
	*month = i;
	*date = year_day - m[i-1];
}

//--------------------------------------------------------------------------
//  dateToDay
//  Функция вычисляет номер дня в году по его дате.
//  Прототип:
//  int dateToDay( int year, int month, int date )
//  Параметры:
//  year    Год.
//  month   Месяц (0-based).
//  date    Число (0-based).
//  Возвращаемое значение: номер дня в году (0-based).
//--------------------------------------------------------------------------
int dateToDay( int year, int month, int date ) {
	int n = date + months[month];
	if( month>1 && isLeapYear(year) ) n++;
	return n;
}

//--------------------------------------------------------------------------
//  dayToDate
//  Функция вычисляет дату дня по его номеру в году.
//  Прототип:
//  void dayToDate( int year, int year_day, int *month, int *date );
//  Парметры:
//  year        Год (полный).
//  year_day    Номер дня в году (0-based).
//  Возвращаемые значения:
//  month       Номер месяца (0-based).
//  date        Дата (0-based).
//--------------------------------------------------------------------------
void dayToDate( int year, int year_day, int *month, int *date ) {
	int *m = isLeapYear(year) ? leapMonths : months;
	int i = 1;
	while( m[i] <= year_day ) i++;
	*month = i-1;
	*date = year_day - m[i-1];
}

double dayTimeToFraction( int h, int m, int s, int t ) {
	return (double(t)/360000. + double(s)/3600. + double(m)/60. + double(h)) / 24.;
}

void dayFractionToTime( double dayFraction, int *hour, int *minute, int *second, int *tic ) {
	dayFraction *= 24.;
	*hour = int( dayFraction );
	dayFraction -= double( *hour );
	dayFraction *= 60.;
	*minute = int( dayFraction );
	dayFraction -= double( *minute );
	dayFraction *= 60.;
	*second = int( dayFraction );
	dayFraction -= double( *second );
	dayFraction *= 100.;
	*tic = int( dayFraction );
}

double timeBetween( int year1, double yearTime1, int year2, double yearTime2 ) {
	int y1,y2;
	double yt1,yt2;
	if( year2 >= year1 ) {
		y2 = year2;
		yt2 = yearTime2;
		y1 = year1;
		yt1 = yearTime1;
	} else {
		y1 = year2;
		yt1 = yearTime2;
		y2 = year1;
		yt2 = yearTime1;
	}
	double t = yt2 - yt1;
	if( y2 > y1 ) {
		t += 365.;
		if( isLeapYear( y1 ) )
			t += 1.;
	}
	return (year2 >= year1 ? t : -t);
}

/*---------------------------------------------------------------------
	dayAdd
	Дополнительная функция к функциям работы с временем типа Year и
	Year_Day. Производит прибавление промежутка add_day, выраженного
	в днях, к моменту времени year, year_day.


	1. Результирующие значения возвращаются в тех же переменных.
	2. Функция производит корректировку года и дня в году, если момент
	времени переходит через год. Параметр add_day может иметь произвольное
	значение больше, меньше или равное 0.0. В последнем случае функция
	просто нормирует заданные значения с учетом перехода через год.
	Корректировка года обязательно нужна при использовании функций
	dayToDate() и mjd1(), так как в них отсутствует проверка на
	переполнение индекса месяца, если количество дней в году больше
	размера года в днях.

	Date: 12 september 2006
---------------------------------------------------------------------*/
void dayAdd( int* year, double* year_day, double add_day ) {
	int j, i;

	/* вычисляем year_day */
	*year_day += add_day;

	/* корректируем 1-based year_day */
	if ( *year_day >= 1.0 ) {
		/* если день года положителен и больше 1, вычитаем из него
		количество дней в текущих годах, пока есть, что вычитать */
		do {
			i = 0;
			j = ( isLeapYear(*year))? leapMonths[12] : months[12];
			if ( *year_day >= (double)(j+1)) {
				(*year_day) -= (double)j;
				(*year)++;
				i = 1;
			}
		} while ( i );
	}
	else {
		/* иначе, если день года меньше 1 или 0, переходим
		к предыдущим годам, пока не получим 1-based */
		do {
			i = 0;
			j = ( isLeapYear((*year)-1))? leapMonths[12] : months[12];
			if ( *year_day < 1.0 ) {
				(*year_day) += (double)j;
				(*year)--;
				i = 1;
			}
		} while ( i );
	}
}
