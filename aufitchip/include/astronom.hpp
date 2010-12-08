/*-------------------------------------------*\
 Данный файл содержит функции необходмые для
 определения положения Солнца в заданный момент
 времени относительно точки на поверхности Земли.
 Функции, представленные здесь, являются
 урезанными си-версиями некоторых функций
 входящих в библиотеку astronom.lib
\*-------------------------------------------*/

#ifndef _ASTRONOM_HPP_
#define _ASTRONOM_HPP_


class TAstronom
{
public:
// Прототипы функций
    double shcrds( double, double, double );
    void shcrds( double, double, double , double * , double * );
    double gsidtj( double );
    double eclipt( double );
    double gsidaj( double );
    double delpsi( double );
    void sunpos( double, double *, double * );
    double kplreq( double, double, double );
    double pert3( double );
    double julian( int, double );
};


#endif
