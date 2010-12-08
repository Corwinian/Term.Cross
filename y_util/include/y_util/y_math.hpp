/*-----------------------------------------------------------------------------
    y_math.hpp
-----------------------------------------------------------------------------*/
#ifndef _Y_MATH_HPP_
#define _Y_MATH_HPP_

#include <stdio.h>
#include <tc_config.h>

// Копирование вектора.
void v_cpy( double * dst, const double * src );

// Сложение векторов.
void v_add( const double *a, const double *b, double *r );

// Вычитание векторов.
void v_sub( const double *a, const double *b, double *r );

// Умножение вектора на число.
void v_mul( const double * a, double n, double * r );

// Вычисление длины вектора.
double v_len( const double *a );

/*
    Вычисление скалярного произведения.
*/
double v_smul( const double * a, const double * b );


/*
    Вычисление угла между векторами.
*/
double v_angle( const double * a, const double * b );


/*
    Нормирование вектора. Массив b может совпадать с массивом a.
*/
void v_norm( const double * a, double * b );

/*
    Вычисление векторного произведения. Массив c НЕ должен совпадать ни с одним
    из массивов a или b.
*/
void v_vmul( const double * a, const double * b, double * c );


/*
    Вычисление вектора нормали плоскости векторов a и b. Массив c НЕ должен
    совпадать ни с одним из массивов a или b.
*/
void v_ort( const double * a, const double * b, double * c );


double mgi240( const double * a, const double * b, const double * c );


/*
    Нахождение минимума и максимума.
 
    ВНИМАНИЕ !!! Индексация массива начинается с 1. Таким образом, массив data должен иметь размер data_size+1
    (или, если p - указатель на обычный массив C, передавать указатель p-1).
 
    pmin, pmax  Могут быть 0.
*/
void d_minmax( const double * data, int data_size, double * pmin, double * pmax );


/*
    Вычисление среднего.
 
    ВНИМАНИЕ !!! Индексация массива начинается с 1. Таким образом, массив data должен иметь размер data_size+1
    (или, если p - указатель на обычный массив C, передавать указатель p-1).
*/
void d_avg( const double * data, int data_size, double * pavg );


/*
    Вычисление rms (root mean square).
 
    ВНИМАНИЕ !!! Индексация массива начинается с 1. Таким образом, массив data должен иметь размер data_size+1
    (или, если p - указатель на обычный массив C, передавать указатель p-1).
*/
void d_rms( const double * data, int data_size, double * rms );


/*
    Вычисление моментов.
 
    ВНИМАНИЕ !!! Индексация массива начинается с 1. Таким образом, массив data должен иметь размер data_size+1
    (или, если p - указатель на обычный массив C, передавать указатель p-1).
 
    data        Массив данных.
    data_size   >=1
    avg         среднее
    var         дисперсия
    std_dev     стандартное отклонение = sqrt(var)
    avg_dev     среднее отклонение
*/
void d_moment( const double * data, int data_size, double * avg, double * var, double * std_dev,
			   double * avg_dev, double * skew, double * curt );



/*
    Поиск минимума функции нескольких переменных методом деформируемого многогранника (Нелдера-Мида).
 
    Входные параметры:
    nvar            Число переменных, >=2.
    fun             Минимизируемая функция. В качестве входного параметра она должна принимать указатель на массив размера nvar.
    ini_simplex     Начальный многогранник - массив размером (nvar+1)*nvar элементов. Организация массива - (nvar+1) векторов по nvar элементов.
    eps             Точность, по достижении которой поиск нужно прекратить - расстояние между "максимальной" и "минимальной" вершинами
                    конечного многогранника.
    iter            Число итераций, по достижении которого поиск следует прекратить. Может быть задан 0, тогда используется внутренняя
                    константа.
    logfile         Файл протокола. Может быть равен NULL, тогда протокол вестись не будет.
 
    Завершение поиска происходит по достижении iter или eps.
 
    Выходные параметры:
    ini_simplex     Конечный многогранник.
    eps             Достигнутая точность.
    iter            Число проделанных итераций.
    fun_c           Число вызовов функции.
    min_index       Индекс "минимальной" вершины многогранника (точка минимума).
    min_value       Значение функции в точке минимума.
 
    Возвращаемое значение:
    0   ок, выход произошел по достижению eps
    1   выход произошел по достижению максимального числа итераций
 
    Описание метода:
    Д.Химмельблау "Прикладное нелинейное программирование", М. Мир, 1975, стр.163.
*/
int min_nelder_mead( int nvar, double (*fun)( double * ), double * ini_simplex, double * eps, int * iter, int * fun_c,
					 int * min_index, double * min_value, FILE * logfile );


/*
    "Обертка" для функции min_nelder_mead. В отличие от нее, не требует от пользователя задания начального многранника, а строит его
    сама, используя в качестве одной из вершин начальную точку поиска.
 
    Входные параметры:
    nvar            Число переменных.
    fun             Минимизируемая функция. В качестве входного параметра она должна принимать указатель на массив размера nvar.
    x               Начальная точка поиска -  массив размером nvar элементов.
    t               Длина ребра начального многогранника. Если задать 0, будет использована величина 1000*eps.
    eps             Точность, по достижении которой поиск нужно прекратить.
    iter            Число итераций, по достижении которого поиск следует прекратить. Может быть задан 0, тогда используется внутренняя
                    константа.
    logfile         Файл протокола. Может быть равен NULL, тогда протокол вестись не будет.
 
    Завершение поиска происходит по достижении iter или eps.
 
    Выходные параметры:
    x               Найденная точка минимума.
    eps             Достигнутая точность.
    iter            Число проделанных итераций.
    fun_c           Число вызовов функции.
    fx              Значение функции в точке минимума.
*/
int min_nelder_mead_1( int nvar, double (*fun)( double * ), double * x, double t, double * eps, int * iter, int * fun_c,
					   double * fx, FILE * logfile );


/*
    Минимизация методом покоординатного_спуска-перебора.
*/
void min_down( int nvar, double (*fun)( double * ), double * x, double * r, double * eps, int * fun_c, double * fx );


#endif
