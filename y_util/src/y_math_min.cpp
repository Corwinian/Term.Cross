/*-----------------------------------------------------------------------------
    y_math_min.cpp
-----------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include <y_util/y_math.hpp>

typedef double * PDOUBLE;

// число отрезков, на которые разбивается диапазон поиска в функции min_down - должно быть четным
#define NR 10

//static double * pool_x;     // координаты точек - pool_size * pool_nvar элементов
//static double * pool_fx;    // значения функции в точках - pool_size элементов
//static int * pool_f;        // флаги занятости элементов пула - pool_size элементов
//static int pool_size;       // число точек, информацию о которых может хранить пул
//static int pool_index;      // начало очереди
//static int pool_nvar;       // размерность пространства переменных

//static void ini_pool( int nvar );
//static void destroy_pool();
//static int search_pool( double * x, double * fx );
//static void add_pool( double * x, double fx );

/*
void ini_pool( int nvar )
{
    pool_nvar = nvar;
    pool_index = 0;
    pool_size = (NR + 1) * nvar * 2;
    pool_x = new double [ pool_size * nvar ];
    pool_fx = new double [ pool_size ];
    pool_f = new int [ pool_size ];
    memset( pool_f, 0, sizeof( int ) * pool_size );
}
 
void destroy_pool()
{
    delete [] pool_x;
    delete [] pool_fx;
    delete [] pool_f;
    pool_x = 0;
    pool_fx = 0;
}
 
int search_pool( double * x, double * fx ){
    int i, k;
    double * px1, * px2;
    double max_d, d;
 
    for( i = 0; i < pool_size; i++ ){
        if( pool_f[i] == 0 ) continue;
 
        max_d = 0.;
        px1 = pool_x + i * pool_nvar;
        px2 = x;
        for( k = 0; k < pool_nvar; k++, px1++, px2++ ){
            d = fabs( *px1 - *px2 );
            if( fabs( *px1 ) > 1e-15 ){
                d /= fabs( *px1 );
            }
            else if( fabs( *px2 ) > 1e-15 ){
                d /= fabs( *px2 );
            }
 
            if( d > max_d ) max_d = d;
        }
 
        if( max_d < 1e-15 ){
            *fx = pool_fx[i];
            return 1;
        }
    }
 
    return 0;
}
 
 
void add_pool( double * x, double fx ){
    memcpy( pool_x + pool_index * pool_nvar, x, sizeof( double ) * pool_nvar );
    pool_fx[ pool_index ] = fx;
    pool_f[ pool_index ] = 1;
 
    if( ++pool_index == pool_size ) pool_index = 0;
}
*/

void min_down( int nvar, double (*fun)( double * ), double * x0, double * range, double * eps, int * fun_c, double * fx ) {
	int i, k;
	const int nr = NR;    // число отрезков, на которые разбивается диапазон поиска - должно быть четным
	double * x;
	double * x_min;
	double * x1;
	double * dx;
	double f;
	double dummy_min;           // заведомо не минимальное значение
	double fmin;                // текущий найденный минимум
	double kmin;                // минимум, найденный в очередном поиске по отрезку k-й оси
	int min_index;
	double max_dx;
	int * zoom;         // счетчики сжатий по каждой из координат
	int fun_count = 0;


	//    ini_pool( nvar );

	x = new double [ nvar ];
	x_min = new double [ nvar ];
	x1 = new double [ nvar ];
	dx = new double [ nvar ];
	zoom = new int [ nvar ];

	//    max_dx = 0.;
	for( i = 0; i < nvar; i++ ) {
		x1[i] = x0[i] - range[i];
		dx[i] = range[i] / double( nr / 2 );
		//        if( dx[i] > max_dx ) max_dx = dx[i];
	}

	// инициализация начального положения предполагаемой точки минимума
	min_index = 0;
	memcpy( x_min, x1, sizeof( double ) * nvar );
	dummy_min = fmin = 2. * fabs( fun( x_min ) );   // заведомо не минимальное значение
	fun_count++;

	memset( zoom, 0, sizeof( int ) * nvar );

	do {     // основной цикл поиска - пока максимальный из шагов по координатным осям не станет меньше заданного eps
		k = 0;
		while( k < nvar ) {  // цикл по координатам

			memcpy( x, x_min, sizeof( double ) * nvar );
			x[k] = x1[k];

			// поиск минимума на отрезке k-й координаты
			kmin = dummy_min;   // заведомо не минимальное значение

			for( i = 0; i <= nr; i++, x[k] += dx[k] ) {
				//                if( search_pool( x, &f ) == 0 ){
				f = fun( x );
				fun_count++;
				//                    add_pool( x, f );
				//                }

				if( f < kmin ) {
					kmin = f;
					x_min[k] = x[k];
					min_index = i;
				}
			}

			//            printf( "fmin = %.14f   kmin = %.14f\n", fmin, kmin );
			fmin = kmin;    // имеем право так делать: старый минимум гарантированно не меньше нового, найденного в очередном поиске

			// определяем, что делать с отрезком k-й координаты - растягивать его или сжимать
			// массив zoom нужен для того, чтобы гарантированно не выйти за начальные границы поиска
			if( min_index == 0 /*&& zoom[k] > 0 && x1[k] > x0[k] - range[k] */) {
				//                printf( "растяжение влево\n" );
				// растяжение и сдвиг влево
				x1[k] -= dx[k] * double( nr / 2 );
				dx[k] *= 2.;
				zoom[k]--;
				//                if( x1[k] < x0[k] - range[k] ){     // если в результате смещения центра выходим за левую границу начального диапазона поиска
				//                    x1[k] = x0[k] - range[k];
				//                }
			} else if( min_index == nr /*&& zoom[k] > 0 && x1[k] + dx[k] * double( nr ) < x0[k] + range[k] */) {
				//                printf( "растяжение вправо\n" );
				// растяжение и сдвиг вправо, точка x1[k] остается при этом на месте
				dx[k] *= 2.;
				zoom[k]--;
				//                if( x1[k] + dx[k] * double( nr ) > x0[k] + range[k] ){     // если в результате смещения центра выходим за правую границу начального диапазона поиска
				//                    x1[k] = x0[k] + range[k] - dx[k] * double( nr );
				//                }
			} else {
				// сжатие диапазона k-й координаты с центром в точке минимума
				dx[k] *= .5;
				x1[k] = x_min[k] - dx[k] * double( nr / 2 );

				//                if( x1[k] < x0[k] - range[k] ){     // если в результате смещения центра выходим за левую границу начального диапазона поиска
				//                    x1[k] = x0[k] - range[k];
				//                }
				//                else if( x1[k] + dx[k] * double( nr ) > x0[k] + range[k] ){     // если в результате смещения центра выходим за правую границу начального диапазона поиска
				//                    x1[k] = x0[k] + range[k] - dx[k] * double( nr );
				//                }

				zoom[k]++;      // увеличиваем счетчик сжатий для данной координаты
				k++;            // переход к следующей координате
			}
		}       // цикл по k

		max_dx = 0.;
		for( i = 0; i < nvar; i++ ) {
			if( dx[i] > max_dx )
				max_dx = dx[i];
		}

	} while( max_dx * 2. > *eps );    // умножаю на 2, потому что действительно поиск с этим шагом будет осуществлен в следующей итерации

	//    destroy_pool();

	memcpy( x0, x_min, sizeof( double ) * nvar );
	*eps = max_dx;
	*fun_c = fun_count;
	*fx = fmin;

	delete [] x;
	delete [] x_min;
	delete [] x1;
	delete [] dx;
	delete [] zoom;
}



int min_nelder_mead_1( int nvar, double (*fun)( double * ), double * x, double t, double * eps, int * iter, int * fun_c,
					   double * fx, FILE * logfile ) {
	double * simp_buf;
	PDOUBLE * simp;
	double l, d1, d2, n;
	int min_index;
	int r;
	int i, j;

	simp_buf = new double [ (nvar + 1) * nvar ];
	simp = new PDOUBLE [ nvar + 1 ];
	simp[0] = simp_buf;
	for( i = 1; i <= nvar; i++ ) {
		simp[i] = simp[i-1] + nvar;
	}

	// построение начального многогранника
	l = (t > 0.) ? t : 1000. * (*eps);
	n = nvar;
	d1 = l * ( sqrt( n + 1. ) + n - 1. ) / ( n * sqrt( 2. ) );
	d2 = l * ( sqrt( n + 1. ) - 1. ) / ( n * sqrt( 2. ) );

	memcpy( simp[0], x, sizeof( double ) * nvar );
	for( i = 1; i <= nvar; i++ ) {
		for( j = 0; j < nvar; j++ ) {
			simp[i][j] = d2;
		}
		simp[i][i-1] = d1;

		for( j = 0; j < nvar; j++ )
			simp[i][j] += x[j];
	}

	// собственно Нелдер-Мид
	r = min_nelder_mead( nvar, fun, simp_buf, eps, iter, fun_c, &min_index, fx, logfile );

	// сохраняем результат
	memcpy( x, simp[min_index], sizeof( double ) * nvar );

	delete [] simp;
	delete [] simp_buf;

	return r;
}


/*
 
 
1. Задаем координаты вершин начального многогранника, вычисляем
целевые функции в вершинах многогранника.
 
2. Вычисляем индексы h и l, а также целевые функции максимальной
и минимальной вершин многогранника f(h) и f(l) ( реально не вычисляем,
а просто находим среди текущих ).
3. Находим координаты центроида.
4. Находим координаты точки отражения и вычисляем функцию в ней f(x3).
 
5. Если f(x3) < f(l),
	5.1. Находим координаты точки растяжения и вычисляем функцию в ней f(x4).
	5.2. Если f(x4) < f(l),
		5.2.1. Заменяем h <- x4.
		5.2.2. Переходим к пункту 7.
	5.3. Иначе,
		5.3.1. Заменяем h <- x3.
		5.3.2. Переходим к пункту 7.
6. Иначе, ( если f(x3) >= f(l) )
	6.1. Находим вторую по величине после h точку многогранника s и
	целевую функцию в ней f(s).
	6.2. Если f(x3) > f(s),
		6.2.1. Если f(x3) <= f(h), заменяем h <- x3.
		6.2.2. Находим координаты точки сжатия и вычисляем функцию в ней f(x5).
		6.2.3. Если f(x5) < f(h),
			6.2.3.1. Заменяем h <- x5.
			6.2.3.2. Переходим к пункту 7.
		6.2.4. Иначе, ( f(x5) >= f(h) )
			6.2.4.1. Производим операцию уменьшения от точки l в 2 раза.
			6.2.4.2. Переходим к пункту 7.
	6.3. Иначе, ( f(x3) <= f(s) )
		6.3.1. Заменяем h <- x3.
		6.3.2. Переходим к пункту 7.
 
7. Вычисляем условие окончания цикла.
8. Если условие не выполняется, переход к пункту 2.
9. Иначе выход.
 
*/
int min_nelder_mead( int nvar, double (*fun)( double * ), double * ini_simplex,
					 double * eps, int * iter, int * fun_c, int * min_index, double * min_value, FILE * f ) {
	const double alpha = 1.0;
	const double beta = 0.5;
	const double gamma = 2.0;

	// деформация симплекса производится в самом массиве ini_simplex

	PDOUBLE * simp;     // для удобства: массив указателей, размечающих ini_simplex на nvar+1 вектор по nvar елементов
	double * array;     // общий массив под fsimp, x2, x3, x4, x5
	double * fsimp;     // значения функции в вершинах симплекса - nvar+1 элемента
	double * x2;        // центроид симплекса без точки h
	double * x3;        // точка отражения
	double * x4;        // точка растяжения
	double * x5;        // точка сжатия
	double fx3, fx4, fx5;   // значения функции в точках отражения, растяжения, сжатия
	double e;

	int i, j;

	int h;	/* индекс вершины с максимальной целевой функцией */
	int l;	/* индекс вершины с минимальной целевой функцией */
	int s;	/* индекс вершины с второй после максимальной целевой функцией */

	int iter_c = 0;
	int max_iter = (*iter > 0) ? *iter : 200;

	int func_c = 0;

	if( f )
		fprintf( f, "\nmin_melder_mead\n" );

	simp = new PDOUBLE [nvar+1];
	array = new double [nvar+1 + nvar*4];
	// размечаем массив array
	fsimp = array;
	x2 = fsimp + nvar+1;
	x3 = x2 + nvar;
	x4 = x3 + nvar;
	x5 = x4 + nvar;

	simp[0] = ini_simplex;
	fsimp[0] = fun( simp[0] );
	func_c++;

	for( i = 1; i <= nvar; i++ ) {
		simp[i] = simp[i-1] + nvar;
		fsimp[i] = fun( simp[i] );
		func_c++;
	}

	/* инициализация четырехгранника ( диагональный тетраэдр ) */
	/*
		simp[0].roll = init_roll;
		simp[0].pitch = init_pitch;
		simp[0].yaw = init_yaw;
		simp[1].roll = init_roll;
		simp[1].pitch = init_pitch + step_pitch;
		simp[1].yaw = init_yaw + step_yaw;
		simp[2].roll = init_roll + step_roll;
		simp[2].pitch = init_pitch;
		simp[2].yaw = init_yaw + step_yaw;
		simp[3].roll = init_roll + step_roll;
		simp[3].pitch = init_pitch + step_pitch;
		simp[3].yaw = init_yaw;
	*/

	// главный цикл поиска
	do {

		if( f ) {
			fprintf( f, "\niter_c: %d  func_c: %d\n", iter_c, func_c );
			for( i = 0; i <= nvar; i++ ) {
				fprintf( f, "simp[%d] =\n", i );
				for( j = 0; j < nvar; j++ ) {
					fprintf( f, "%.14f ", simp[i][j] );
				}
				fprintf( f, "\nf( simp[%d] ) = %.14f\n", i, fsimp[i] );
			}
		}

		/* определяем индексы минимальной и максимальной вершин */
		l = h = 0;
		for ( i = 1; i <= nvar; i++ ) {
			if ( fsimp[i] > fsimp[h] ) {
				h = i;
			} else if( fsimp[i] <= fsimp[l] ) {
				l = i;
			}
		}

		if( f )
			fprintf( f, "l = %d  h = %d\n", l, h );

		/* определяем координаты центроида ( центра грани четырехгранника, противоположной точке с индексом h ) */
		if( f )
			fprintf( f, "x2 =\n" );
		memset( x2, 0, sizeof( double ) * nvar );
		for( i = 0; i <= nvar; i++ ) {
			if( i != h ) {
				for( j = 0; j < nvar; j++ ) {
					x2[j] += simp[i][j];
				}
			}
		}
		for( j = 0; j < nvar; j++ ) {
			x2[j] /= double( nvar );
			if( f )
				fprintf( f, "%.14f ", x2[j] );
		}

		/* отражение: проецируем точку h через центроид с коэффициентом отражения alpha */
		if( f )
			fprintf( f, "\nx3 =\n" );
		for( j = 0; j < nvar; j++ ) {
			x3[j] = x2[j] + alpha * ( x2[j] - simp[h][j] );
			if( f )
				fprintf( f, "%.14f ", x3[j] );
		}
		fx3 = fun( x3 );
		if( f )
			fprintf( f, "\nf( x3 ) = %.14f\n", fx3 );
		func_c++;

		/*
		    проверка f ( x3 ):
		    если целевая функция в точке отражения меньше целевых функций всех вершин четырехгранника
		    ( меньше функции вершины с минимальным значением )
		*/
		if ( fx3 < fsimp[l] ) {
			/* производим растяжение: растягиваем вектор x3-x2 в gamma раз */
			if( f )
				fprintf( f, "растяжение\nx4 =\n" );
			for( j = 0; j < nvar; j++ ) {
				x4[j] = x2[j] + gamma * ( x3[j] - x2[j] );
				if( f )
					fprintf( f, "%.14f ", x4[j] );
			}
			fx4 = fun( x4 );
			if( f )
				fprintf( f, "\nf( x4 ) = %.14f\n", fx4 );
			func_c++;

			/* если целевая функция в точке растяжения меньше целевых функций
			всех вершин четырехгранника ( меньше функции вершины с минимальным
			значением ) */
			if ( fx4 < fsimp[l] ) {
				/* заменяем точку h на x4 */
				if( f )
					fprintf( f, "x4 -> h\n" );
				memcpy( simp[h], x4, sizeof( double ) * nvar );
				fsimp[h] = fx4;
			} else {
				/* иначе заменяем точку h на x3 */
				if( f )
					fprintf( f, "x3 -> h\n" );
				memcpy( simp[h], x3, sizeof( double ) * nvar );
				fsimp[h] = fx3;
			}
		}
		/* если целевая функция в точке отражения больше функции вершины с минимальным значением ) */
		else {
			/* находим вторую по величине целевой функции после максимальной точку четырехгранника ( индекс s ) */
			s = l;
			for ( i = 0; i <= nvar; i++ ) {
				if( i == h )
					continue;
				if( fsimp[i] >= fsimp[s] ) {
					s = i;
				}
			}
			if( f )
				fprintf( f, "s = %d\n", s );

			/* если целевая функция в точке отражения больше f(s) */
			if ( fx3 > fsimp[s] ) {
				if ( fx3 <= fsimp[h] ) {
					/* заменяем точку h на x3 */
					if( f )
						fprintf( f, "x3 -> h\n" );
					memcpy( simp[h], x3, sizeof( double ) * nvar );
					fsimp[h] = fx3;
				}

				/* производим сжатие: сжимаем вектор xh-x2 в beta раз */
				if( f )
					fprintf( f, "сжатие\nx5 =\n" );
				for( j = 0; j < nvar; j++ ) {
					x5[j] = x2[j] + beta * ( simp[h][j] - x2[j] );
					if( f )
						fprintf( f, "%.14f ", x5[j] );
				}
				fx5 = fun( x5 );
				if( f )
					fprintf( f, "\nf( x5 ) = %.14f\n", fx5 );
				func_c++;

				/* если функция в точке сжатия меньше максимальной */
				if ( fx5 < fsimp[h] ) {
					/* заменяем точку h на x5 */
					if( f )
						fprintf( f, "x5 -> h\n" );
					memcpy( simp[h], x5, sizeof( double ) * nvar );
					fsimp[h] = fx5;
				} else {
					/* производим редукцию: уменьшаем все ребра четырехгранника, выходящие из xl, в 2 раза и пересчитываем
					            целевую функцию для каждой из измененных вершин */
					if( f )
						fprintf( f, "редукция\n" );
					for( i = 0; i <= nvar; i++ ) {
						if( i == l )
							continue;

						for( j = 0; j < nvar; j++ ) {
							simp[i][j] = simp[l][j] + .5 * ( simp[i][j] - simp[l][j] );
						}

						fsimp[i] = fun( simp[i] );
						func_c++;
					}
				}

			} else { /* fx3 <= fsimp[s] */
				/* заменяем точку h на x3 */
				if( f )
					fprintf( f, "x3 -> h\n" );
				memcpy( simp[h], x3, sizeof( double ) * nvar );
				fsimp[h] = fx3;
			}
		}

		/* вычисляем условие окончания цикла: расстояние от минимальной до максимальной точки */
		e = 0.;
		for( j = 0; j < nvar; j++ ) {
			e += (simp[h][j] - simp[l][j]) * (simp[h][j] - simp[l][j]);
		}
		e = sqrt( e );
		if( f )
			fprintf( f, "e = %.14f\n", e );

		iter_c++;

	} while( e > *eps && iter_c < max_iter ); // условие окончания поиска: по достижении требуемой точности или максимального числа итераций

	// запись результатов
	*min_index = l;
	*min_value = fsimp[l];
	*iter = iter_c;
	*fun_c = func_c;
	*eps = e;
	if( f ) {
		fprintf( f, "\nresult:\n" );
		fprintf( f, "l = %d\n", l );
		fprintf( f, "f(l) = %.14f\n", fsimp[l] );
		fprintf( f, "iter_c = %d\n", iter_c );
		fprintf( f, "func_c = %d\n", func_c );
		fprintf( f, "e = %.14f\n", e );
	}

	delete [] array;
	delete [] simp;

	return (iter_c < max_iter) ? 0 : 1;
}
