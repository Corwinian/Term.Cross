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
double f1dim(double x, int ncom, double *pcom, double *xicom,
			 double (*func)(double[]) ) {
	double *xt = new double[ncom];
	for(int j=0;j<ncom;j++)
		xt[j]=pcom[j]+x*xicom[j];
	double f = func(xt);
	delete xt;
	return f;
}
