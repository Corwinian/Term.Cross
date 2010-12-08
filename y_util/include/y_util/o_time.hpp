/*-----------------------------------------------------------------------------
    y_time.hpp
-----------------------------------------------------------------------------*/
#ifndef _O_TIME_HPP_
#define _O_TIME_HPP_

#include <tc_config.h>
#include <c_lib/c_types.hpp>


/*---------------------------------------------------------------------
	MJD1.C
	Функция вычисляет модифицированную юлианскую дату с 15 октября 1582
	года. Идея взята из функции из книги О.Монтенбрук,
	Т.Пфлегер "Астрономия с персональным компьютером", стр.21.
	Переделана под нормальное плавающее время года ( 1-based, как в
	телеграмме norad ).
	Прототип: double mjd1 ( int year, double year_day );
-----------------------------------------------------------------------*/
double mjd1 ( int year, double year_day );

/*--------------------------------------------------------------------------
	DIFF_TIME.C
	Функция для вычисления разности между двумя моментами времени,
	заданными как Year и Year_Day.
	Прототип:	double diff_time ( short year1, double yearDay1,
		unsigned short year2, double yearDay2 );
	где
		year1, yearDay1		Определяют первый момент времени.
		year2, yearDay2		Определяют второй момент времени.
	Возвращаемое значение:
	Интервал между вторым и первым моментом времени, выраженный в днях.
	Результат > 0 , если второй момент времени позже ( больше ) первого.
	Примечание:
		Данная функция практически аналогична функции из orbmodel.c
	timeBetween(), однако здесь входные параметры практически ничем
	не ограничены, поскольку разность двух моментов времени вычисляется
	через модифицированные юлианские даты ( см. sa_date_time(), mjd1()).
	Date : 7 september 2006
--------------------------------------------------------------------------*/
double diff_time ( short year1, double yearDay1, short year2, double yearDay2 );
/*---------------------------------------------------------------------
	GMST.C
	Функция вычисляет среднее звездное время в Гринвиче для заданного
	момента времени UTC, как величину от 0.0 до 1.0 ( т.е. в 24-часовых
	сутках ).
	Будучи переведенной в градусы или радианы умножением на 360 или 2*PI,
	значение данной функции представляет из себя фактически угол между
	направлением на меридиан Гринвича и направлением на точку весеннего
	равноденствия в экваториальной системе координат ECI для заданной даты
	и заданного момента времени UTC.
	Прототип: double gmst ( double mjd_time );
	Примечания:
	1. Заданный момент времени UTC задается как модифицированная
		юлианская дата вместе с собственно временем UTC. Входной параметр
		данной функции может быть получен с помощью функции mjd1.
	2. Возвращаемое функцией значение может быть преобразовано в
		градусы умножением на 360 или в радианы умножением на 2*PI
		или в единицы времени умножением на 24 ( часы ), 1440 ( минуты )
		или 86400 ( секунды ).
	3. Коэффициенты в формуле для звездного времени заданы относительно
		эпохи J2000 ( mjd = 51544.5, jd = 2451545.0 ).
---------------------------------------------------------------------*/
double gmst ( double mjd_time );

#endif
