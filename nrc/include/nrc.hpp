//-------------------------------------------------------------------------
// NRC.HPP
//-------------------------------------------------------------------------
#ifndef _NRC_
#define _NRC_

#include <math.h>

/*
 * f1dim
 * Функция используется другими функциями библиотеки. Вычисляет значение
 * функции в точке, отстоящей от другой в заданном направлении
 * на заданном расстоянии.
 * x Расстояние ( со знаком ).
 * n Размерность пространства.
 * pcom Базовая точка.
 * xicom Вектор направления ( не обязательно единичный, однако внутри
 * функции нормировка не производится ).
 */
double f1dim(double x, int n, double *pcom, double *xicom, double (*func)(double[]) );

/*
 * brent
 * Нахождение минимума функции методом Брента ( Brent ).
 * ax Левая граница интервала. ???
 * bx Точка в интервале. ???
 * cx Права граница интервала. ???
 * f Минимизируемая функция.
 * tol Требуемая точность.
 * xmin Найденный минимум.
 */
double brent(double ax, double bx, double cx, double (*f)(double), double tol, double *xmin);

/*
 * dfridr
 * Вычисление производной методом Риддера ( Ridder ).
 * func Дифференцируемая функция.
 * x Точка.
 * h Начальный шаг.
 * err Выход - оценка погрешности.
 */
double dfridr( double (*func)(double), double x, double h, double *err );

/*
 * linmin
 * Минимизация функции вдоль луча в n-мерном пространстве.
 * p Начало луча. На выходе - найденная точка.
 * xi Направление луча. На выходе - вектор между началом луча и найденной
 * точкой
 * n Размерность пространства.
 * fret Значение в точке минимума.
 * func Минимизируемая функция.
 */
void linmin(double p[], double xi[], int n, double *fret, double (*func)(double []));

void  mnbrak(double *ax, double *bx, double *cx, double *fa, double *fb,
			double *fc, double (*func)(double) );

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
void  powell( double p[], double *xi, int n, double ftol, int *iter,
			double *fret, double (*func)(double []));


inline double nrc_sqr(double a) { return a*a; }
inline double nrc_max(double a,double b) { return (a>b)?a:b; }
inline double nrc_sign(double a,double b) { return (b>0.)?fabs(a):-fabs(a); }

/*
    rtbis
    Поиск корня методом бисекции.
    Returns:
    0   OK
    1   Функция принимает на концах указанного интервала значения
        одинакового знака.
    2   Слишком большое число итераций.
*/
int rtbis( double (*func)(double), double x1, double x2, double xacc, double *root );

/*
    Функция производит LU-разложение матрицы a.
    Возвращаемое значение:
    0   OK
    1   Матрица вырождена.
*/
int ludcmp( double *a, int n, int *indx, double *d );
void lubksb( double *a, int n, int *indx, double *b, double *x );

#endif
