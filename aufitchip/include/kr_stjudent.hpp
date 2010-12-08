/*----------------------------------------------------------------------------------------------------------
    kr_stjudent.hpp
    Класс, реализация которого помогает выполнять проверку статистических гипотез.
----------------------------------------------------------------------------------------------------------*/
#ifndef _KR_STJUDENT_HPP_
    #define _KR_STJUDENT_HPP_



class TChecking_of_StatisticalHypothesises {
public:

    TChecking_of_StatisticalHypothesises();

    ~TChecking_of_StatisticalHypothesises();

    double probability_of_DistributionStjudent( double, long );

    double verificationLevel_of_CriterionStjudent( double, double, double, double, double, double );

    double verificationNormalLevel_of_CriterionStjudent( double, double, double, double, double, double );

    /* Вычисляет вероятность того, что случайная величина,
     * подчиняющаяся стандартному нормальному распределению,
     * принимает значение, не превосходящее x.
     */
    double normalDF(double x);

    /* Вычисляется квантиль уровня level,
     * который, согласно определению, является корнем уравнения
     *		N(x) = level,
     * где N(x) - стандартное нормальное распределение.
     * Решение уклоняется от точного значения не более, чем на 0.00045.
     * Конечно, значение level должно быть заключено между 0 и 1.
     */
    double inv_normalDF(double level);

private:

    double prob1_of_DistributionStjudent( double );
    double prob2_of_DistributionStjudent( double );
    double prob3_of_DistributionStjudent( double, double, double );

}; // end of class TChecking_of_StatisticalHypothesises;


class GammaDF {
    public:
       GammaDF(double shape, double scale=1);
       double value(double x);    // Функция распределения Gamma(x|shape,scale)
       double inv(double p);      // Обратная функция: Gamma(x|shape,scale)=p
       /*
        * Вычисляет натуральный логарифм полной гамма-функции Gamma(x)
        */
       double logGamma(double x);

    private:
       double a, shape, scale, lga;
       double fraction(double x);
       double series(double x);
 };


#endif
