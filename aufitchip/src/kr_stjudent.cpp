/*----------------------------------------------------------------------------------------------
    kr_stjudent.cpp
    В этом файле собраны функции для проверки статистических гипотез.
-----------------------------------------------------------------------------------------------*/
#include <math.h>
#include <assert.h>
#include <kr_stjudent.hpp>

TChecking_of_StatisticalHypothesises :: TChecking_of_StatisticalHypothesises()
{

}


TChecking_of_StatisticalHypothesises :: ~TChecking_of_StatisticalHypothesises()
{

}


double TChecking_of_StatisticalHypothesises :: verificationLevel_of_CriterionStjudent( double col_sea, double sr_sea, double disp_sea, double col_land, double sr_land, double disp_land )
{
    double chisl = fabs( sr_land - sr_sea ) * pow( col_sea * col_land * ( col_sea + col_land - 2  ), .5 );
    double znam = ( disp_sea * ( col_sea - 1 ) + disp_land * ( col_land - 1 ) ) * ( col_sea + col_land );

    if( znam <= 0. ) return ( -1000. );
    return ( chisl / pow( znam, .5 ) );
}


double TChecking_of_StatisticalHypothesises :: verificationNormalLevel_of_CriterionStjudent( double col_sea, double sr_sea, double disp_sea, double col_land, double sr_land, double disp_land )
{
    double chisl = fabs( sr_land - sr_sea ) * pow( col_sea * col_land, .5 );
    double znam = ( disp_sea * ( col_sea - 1 ) + disp_land * ( col_land - 1 ) ) * ( col_sea + col_land );

    if( znam <= 0. ) return ( -1000. );
    return ( chisl / pow( znam, .5 ) );
}


double TChecking_of_StatisticalHypothesises :: probability_of_DistributionStjudent( double x, long idf )
{
    double ret_val, r__1;
    double dasb, tval;
    long idfm2, i__1, i1, l;
    double a, b, p;
    double f1, f2, p1, da, df, sb, az, fz, dsb, sum, fkm1;

    double d__ = 0.0;

    if( idf <= 0 ){
        ret_val = 1.0e50;
        return ret_val;
    }

    df = idf;
    tval = x;
    i1 = idf - ( idf / 2 << 1 );
    a = tval / sqrt( df );
    b = df / ( df + tval * tval );
    sb = sqrt( b );
    da = d__ * a;
    dsb = d__ * sb;
    dasb = a * dsb;
    p1 = prob2_of_DistributionStjudent( dasb );
    f2 = a * sb * exp( dsb * -0.5 * dsb ) * p1 * 0.3989423;
    f1 = b * ( da * f2 + a * 0.1591549 * exp( d__ * (-0.5) * d__ ) );
    sum = 0.0;

    if( idf == 1 ){
        r__1 = 0.7071068 * dsb;
        p1 = prob1_of_DistributionStjudent( r__1 ) * 0.5;
        p = prob3_of_DistributionStjudent( dsb, a, 0.000001 );
        ret_val = p1 + ( p + sum ) * 2.0;
        return ret_val;
    }

    if( i1 <= 0 ) sum = f2;
    else sum = f1;

    if( idf > 4 ){
        idfm2 = idf - 2;
        az = 1.0;
        fz = 2.0;
        i__1 = idfm2;

        for( l = 2; l <= i__1; l += 2 ){
            fkm1 = fz - 1.0;
            f2 = b * ( da * az * f1 + f2 ) * fkm1 / fz;
            az = 1.0 / ( az * fkm1 );
            f1 = b * ( da * az * f2 + f1 ) * fz / ( fz + 1.0 );

            if( i1 <= 0 ) sum += f2;
            else sum += f1;

            az = 1.0 / ( az * fz );
            fz += 2.0;
        }
    }

    if( i1 <= 0 ){
        r__1 = 0.7071068 * d__;
        p1 = prob1_of_DistributionStjudent( r__1 ) * 0.5;
        ret_val = p1 + sum * 2.506628;
    }
    else{
        r__1 = 0.7071068 * dsb;
        p1 = prob1_of_DistributionStjudent( r__1 ) * 0.5;
        p = prob3_of_DistributionStjudent( dsb, a, 0.000001 );
        ret_val = p1 + ( p + sum ) * 2.0;
    }

    return ret_val;
}


double TChecking_of_StatisticalHypothesises :: prob1_of_DistributionStjudent( double x )
{
    double s = 0.0, t = fabs( x );

    if( t <= 6.5 )
        s = exp( -t * t ) * ( ( ( ( ( ( t * 0.56419 + 6.802899 ) * t +
            38.71143 ) * t + 131.1266 ) * t + 278.5978 ) * t + 355.969 ) * t
            + 224.1828 ) / ( ( ( ( ( ( ( t + 12.05784 ) * t + 69.11384 ) * t +
            238.4503 ) * t + 527.5538 ) * t + 741.5214 ) * t + 608.9322 ) * t
            + 224.1828 );

    if( x < 0.0 ) s = 2.0 - s;

    return s;
}


double TChecking_of_StatisticalHypothesises :: prob2_of_DistributionStjudent( double x )
{
    return ( prob1_of_DistributionStjudent( -0.7071067 * x ) * 0.5 );
}


double TChecking_of_StatisticalHypothesises :: prob3_of_DistributionStjudent( double y, double z__, double eps )
{
    double aeps, hsqb, bexp, d__, f, g;

    double t = 0.0, ahsqb, a4, b4, d1, g1, d2;

    double ta, ab4, a4b4, ber, asq, ter, sum, ep1 = eps;

    double a = fabs( z__ ), b = fabs( y );

    if( eps == 0.0 ) ep1 = 1e-6;

    if( a == 0.0 ) return t;

    ta = atan( a );

    if( a * b >= 4.0 ){
        t = 0.1591549 * ( ta + atan( 1.0 / a ) ) - ( prob2_of_DistributionStjudent( b ) - 0.5 ) * 0.5;
        if( z__ < 0.0 ) t = -t;
        return t;
    }

    hsqb = b * 0.5 * b;

    if( hsqb > 88.72283 ) return t;

    bexp = exp( -hsqb );
    asq = a * a;
    a4 = asq * asq;
    b4 = hsqb * hsqb;
    a4b4 = a4 * b4;
    ahsqb = a * hsqb;
    ab4 = a * b4 * 0.5;
    f = 1.0;
    sum = 0.0;
    g = 3.0;

    do{
        g1 = 3.0;
        ber = 0.0;
        ter = ab4;

        ber += ter;
        while( ter > (ber * ep1) ){
//            ber += ter;
//            if( ter <= ber * ep1 ) break;
            ter = ter * hsqb / g1;
            g1 += 1.0;
            ber += ter;
        }

        d1 = ( ber + ahsqb ) / f;
        d2 = ber * asq / ( f + 2.0 );
        d__ = d1 - d2;
        sum += d__;
        t = ta - sum * bexp;
        aeps = ep1 * t;
        ahsqb = ahsqb * a4b4 / ( ( g - 1.0 ) * g );
        ab4 = ab4 * a4b4 / ( ( g + 1.0 ) * g );
        f += 4.0;
        g += 2.0;

    } while( d2 * bexp >= aeps );

    t *= 0.1591549;

    if( z__ < 0.0 ) t = -t;

    return t;
}




#define LGM_LIM         7
/* Implementation dependent const used to increase
 * convergence in logGamma and gammaDF.
 *      May be changed when porting functions to
 *      computers with different float/double lengths.
 */

double GammaDF::logGamma(double x)
/*
 * Compute natural logarithm of Gamma(x)
 *      using the asymptotic Sterling's expansion.
 * See Abramowitz & Stegun,
 *      Handbook of Mathematical Functions, 1964 [6.1.41]
 * The first 20 terms give the result with 50 digits.
 * If x <= 0, assert() is called to indicate error.
 */
{
   long double static c[20] =
   {
      /* Asymtotic expansion coefficients             */
      1.0 / 12.0, -1.0 / 360.0, 1.0 / 1260.0, -1.0 / 1680.0, 1.0 / 1188.0,
      -691.0 / 360360.0, 1.0 / 156.0, -3617.0 / 122400.0, 43867.0 / 244188.0,
      -174611.0 / 125400.0, 77683.0 / 5796.0, -236364091.0 / 1506960.0,
      657931.0 / 300.0, -3392780147.0 / 93960.0, 1723168255201.0 / 2492028.0,
      -7709321041217.0 / 505920.0, 151628697551.0 / 396.0,
      -26315271553053477373.0 / 2418179400.0, 154210205991661.0 / 444.0,
      - 261082718496449122051.0 / 21106800.0
   };


   double x2, presum, sum, den, z;
   int  i;

   assert(x > 0);                      /* Negative argument: Error!         */

   if (x == 1 || x == 2)
      return 0;

   for (z = 0; x < LGM_LIM; x += 1)    /* Increase argument if necessary.   */
      z += log(x);

   den = x;
   x2 = x * x;                         /* Compute the asymptotic expansion  */
   presum = (x - 0.5) * log(x) - x + 0.9189385332046727417803297364;
   for (i = 0; i < 20; i++) {
      sum = presum + c[i] / den;
      if (sum == presum) break;
      den = den * x2;
      presum = sum;
   }
   return sum - z;                     /* Fit the increased argument if any  */

}/*logGamma*/



static const double zero = 0.0;
static const double one = 1.0;
static const double two = 2.0;

GammaDF::GammaDF(double shape, double scale):
    a(shape), shape(shape), scale(scale), lga(logGamma(shape))
 {
   assert(shape > 0 && scale > 0);
 }


double GammaDF::series(double x)
// См. Abramowitz & Stegun,
//      Handbook of Mathematical Functions, 1964 [6.5.29]
//  М.Абрамовиц, И.Стиган
//      Справочник по специальным функциям (М: Мир, 1979)
//
// Для вычисления неполной гамма функции P(a,x)
// используется ее разложение в ряд Тейлора.
//
{
   double sum, prev_sum, term, aa = a;
   long i = 0;

   term = sum = one / a;
   do {
      aa += one;
      term *= x / aa;
      prev_sum = sum; sum += term;
      i++;
   } while(fabs(prev_sum) != fabs(sum));
   return sum *= exp(-x + a * log(x) - lga);
}/* incGamma_series */

double GammaDF::fraction(double x)
// См. Abramowitz & Stegun,
//      Handbook of Mathematical Functions, 1964 [6.5.31]
//  М.Абрамовиц, И.Стиган
//  Справочник по специальным функциям (М: Мир, 1979)
//
// Для вычисления неполной гамма функции P(a,x)
// используется ее разложение в цепную дробь Лежандра
//
//  P(a,x)=exp(-x +x*ln(a))*CF/logGamma(a),
//
//  где
//
//        1    1-a   1   2-a   2
//  CF = ---  ----- --- ----- --- ....
//       x+    1+   x+   1+   x+
//
//  Используются подходящие дроби CF(n) = A(n) / B(n)
//
//  где
//        A(n) = (s(n) * A(n-1) + r(n) * A(n-2)) * factor
//        B(n) = (s(n) * B(n-1) + r(n) * B(n-2)) * factor
//  причем
//        A(-1) = 1, B(-1) = 0, A(0) = s(0), B(0) = 1.
//
//  Здесь
//
//        s(0) = 0, s(1) = x, r(0) = 0, r(1) = 1,
//
//  так что
//
//        A(1) = one * factor, B(1) = r * factor
//
//  и, следовательно,
//
//        r(i) = k - a  if i = 2k,   k > 0
//        r(i) = k      if i = 2k+1,
//        s(i) = 1      if i = 2k,
//        s(i) = x      if i = 2k+1
//
//  factor - шкалирующий множитель
//
{
   double old_sum=zero, factor=one;
   double A0=zero, A1=one, B0=one, B1=x;
   double sum=one/x, z=zero, ma=zero-a, rfact;

   do {
      z += one;
      ma += one;
      /* two steps of recurrence replace A's & B's */
      A0 = (A1 + ma * A0) * factor;	/* i even */
      B0 = (B1 + ma * B0) * factor;
      rfact = z * factor;
      A1 = x * A0 + rfact * A1;	/* i odd, A0 already rescaled */
      B1 = x * B0 + rfact * B1;
      if (B1) {
	 factor = one / B1;
	 old_sum = sum;
	 sum = A1 * factor;
      }/* if */
   } while (fabs(sum) != fabs(old_sum));
   return exp(-x + a * log(x) - lga) * sum;
}/*fraction*/

double GammaDF::value(double x)
// Вычисляется Gamma(x|a):
//      вероятность того, что случайная величина,
//      подчиняющаяся центральному гамма-распределению с параметром 'a',
//      меньше или равна 'x'.
{
   x *= scale;
   if(x <= zero)
      return zero;            /* НЕ ошибка! */
   else if(x < (a + one))
      return series(x);
   else
      return one - fraction(x);
}/*value*/

double GammaDF::inv(double p)
// Ищет такое значение 'x', для которого Gamma(x|a) = p,
//      т.е. равна 'p' вероятность того, что случайная величина,
//      подчиняющаяся центральному гамма-распределению с параметром 'a',
//      меньше или равна 'x'.
{
   double fx, l = 0, r = 1, x;

   if (p == 0) return 0;
   assert(p > 0 && p < 1);

   for(l=0, r=a/2; value(r) < p; r+=a/2) l=r;
   x=(l+r)/2;
   do {
      fx = value(x);
      if (fx > p) r = x; else
      if (fx < p) l = x; else
         break;
      x = (l + r)* 0.5;
    } while ((l!=x) && (r!=x));
    return x;

}/*inv*/


/****************************************************/
/*                 Нормальное распределение         */
/****************************************************/

double TChecking_of_StatisticalHypothesises :: normalDF(double x)
/* Вычисляет вероятность того, что случайная величина,
 * подчиняющаяся стандартному нормальному распределению,
 * принимает значение, не превосходящее x.
 */
{
   double dfg;
   if (x == zero)
      return 0.5;
   GammaDF g(0.5);
   dfg = g.value(x * x / 2) / 2;
   return 0.5 + (x > zero ? dfg : -dfg);

}/*normalDF*/

double TChecking_of_StatisticalHypothesises :: inv_normalDF(double level)
/* Вычисляется квантиль уровня level,
 * который, согласно определению, является корнем уравнения
 *		N(x) = level,
 * где N(x) - стандартное нормальное распределение.
 * Решение уклоняется от точного значения не более, чем на 0.00045.
 * Конечно, значение level должно быть заключено между 0 и 1.
 */
{
   double q, t;

   assert((level > zero) && (level < one));
   t = level < 0.5 ? level : one - level;
   t = sqrt(-2 * log(t));
   q = t - ((0.010328 * t + 0.802853) * t + 2.515517) /
      (((0.001308 * t + 0.189269) * t + 1.432788) * t + 1);
   return level > 0.5 ? q : -q;
}/*inv_normalDF*/
