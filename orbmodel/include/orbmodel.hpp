/*-------------------------------------------------------------------------
	orbmodel.hpp
	Орбитальная модель ( sgp8 и sgp4 )
	Автор: Эпштейн Ю.
	ВНИМАНИЕ! В отличие от канонической реализации модели и телеграмм NORAD
	данная реализация использует 0-based день в году.
	Date: 12 ноября 2005
	Date: 7 august 2006
		Исправлены определения глобальных констант;
		Исправлена ошибка в SGP8 ( знак перед последним членом y5 ).
		Исправлена ошибка в TIniSatParams::TIniSatParams( const char * file_name, const char * sat_name )
			Неправильная работа с годом для даты >= 2000 г.
-------------------------------------------------------------------------*/
#ifndef _ORBMODEL_HPP_
#define _ORBMODEL_HPP_

#include <tc_config.h>
#include <c_lib.hpp>

#define COR_PARAM_NUM   11      // число параметров коррекции
#pragma pack(1)
struct TCorrectionParamsAsArray {
	uint32_t version;
	double params[COR_PARAM_NUM];
};
#pragma pack()

/*-------------------------------------------------------------------------
    TCorrectionParams
    Класс для представления набора параметров коррекции географической
    привязки.
-------------------------------------------------------------------------*/
#pragma pack(1)
class TCorrectionParams {
public:

	// Конструирование по данным в 0-блоке.
	// Если коррекция в 0-блоке отсутствует (поле fVersion == 0), поле fVersion структуры инициализируется значением 2, остальные поля - 0.
	TCorrectionParams( const TBlk0 & blk0 );

	TCorrectionParams( uint32_t ver = 0,
					   double t = 0, double tbus_t = 0,
					   double roll_angle = 0, double pitch_angle = 0, double yaw_angle = 0,
					   double dn0 = 0, double di0 = 0, double draan0 = 0, double de0 = 0, double dw0 = 0, double dm0 = 0 );

	friend int operator==( const TCorrectionParams &c1, const TCorrectionParams &c2 );
	friend int operator!=( const TCorrectionParams &c1, const TCorrectionParams &c2 );

	/*
	    Возвращаемое значение:
	    1   значения всех корректирующих параметров равны 0 (значение поля fVersion не учитывается)
	    0   в противном случае
	*/
	int isZero() const;

	// 0 - отсутствие коррекции
	// 1 - коррекция вычислена с использованием орбитальной модели SGP8
	// 2 - коррекция вычислена с использованием орбитальной модели SGP4
	uint32_t fVersion;

	// коррекция времени начала сеанса
	double fTime;       // В секундах.
	double fTBUSTime;   // В секундах.
	// коррекция ориентации платформы спутника, в радианах
	double roll;
	double pitch;
	double yaw;
	// коррекция прогнозных орбитальных параметров
	double dn0;     // среднее движение
	double di0;     // наклонение
	double draan0;  // восходящий узел
	double de0;     // эксцентриситет
	double dw0;     // аргумент перигея
	double dm0;     // средняя аномалия
};
#pragma pack()

//--------------------------------------------------------------------------
//  TIniSatParams
//  Прогнозные орбитальные параметры.
//--------------------------------------------------------------------------
class TIniSatParams {
public:
	TIniSatParams( const TBlk0 & );
	TIniSatParams( const TOldBlk0 & );
	//--------------------------------------------------------------------------
	//  Инициализация текстом телеграммы TLE, уже прочитанным в буфер. Буфер
	//  должен содержать только текст телеграммы, без имени спутника перед ним.
	//--------------------------------------------------------------------------
	TIniSatParams( const char * );
	//--------------------------------------------------------------------------
	//  Инициализация из файла телеграмм TLE, получаемого по E-Mail.
	//  TIniSatParams( const char * fileName, const char *satName )
	//  Параметры:
	//  satName     Задает имя спутника. Например, "NOAA 12".
	//              Максимальный размер - 32 символа, включая '\0'.
	//  Исключения:
	//  TAccessExc( 1, "Ошибка доступа к файлу TLE" )
	//  TRequestExc( 1, "Файл TLE не содержит телеграммы для данного спутника" )
	//-------------------------------------------------------------------------
	TIniSatParams( const char *, const char * ) throw ( TAccessExc, TRequestExc );

	unsigned fRevNum; //TODO: wtf 'unsigned'? is this an unsigned int?
	unsigned fSetNum;
	int    fEpochYear;
	double fEpochYearTime;  // Полное время в году, выраженное в днях (0-based).
	double n0;       // Mean motion.
	double bstar;    // BSTAR drag term.
	double i0;       // Inclination.
	double raan;     // Right ascension of ascending node.
	double e0;       // Eccentricity.
	double w0;       // Argument of perigee.
	double m0;       // Mean anomaly.
};

/*-------------------------------------------------------------------------
    TNOAAImageParams
-------------------------------------------------------------------------*/
class TNOAAImageParams {
public:
	TNOAAImageParams( const TBlk0 & );

	TSatInfoTable::NoradSatId   satId;      // идентификатор спутника NORAD
//	uint32_t   fYear;          // Полный год.
	int   fYear;          // Полный год.
	double  fYearTime;      // Полное время в году, выраженное в днях (0-based).
	uint32_t   fScans;         // Число сканов в снимке.
	int     fAscendFlag;    // false - нисходящий, true - восходящий
};

enum OrbitalModel { SGP4, SGP8 };
/*-------------------------------------------------------------------------
    class TOrbitalModel
-------------------------------------------------------------------------*/
class TOrbitalModel {
public:
	//--------------------------------------------------------------------------
	//  Производится инициализация переменных, или не зависящих от времени, или
	//  зависящих только от базового момента времени ( baseYear и baseYearTime ).
	//  Параметры, вычисляемые для конкретного момента времени, остаются
	//  неопределенными. Для их вычисления необходимо воспользоваться функцией
	//  model.
	//  Параметры:
	//  baseYear        Год, включая век: 1997, а не 97.
	//  baseYearTime    Время в году, выраженное в днях (0-based).
	//  ВНИМАНИЕ !!! Параметры baseYear и baseYearTime должны задавать время,
	//  как можно ближе лежащее к тем моментам времени, для которых в дальнейшем
	//  будет вызываться функция model.
	//
	//    c.fVersion:
	//    0   SGP8
	//    1   SGP8
	//    2   SGP4
	//
	//--------------------------------------------------------------------------
	TOrbitalModel( const TIniSatParams&, int baseYear, double baseYearTime, const TCorrectionParams& c = TCorrectionParams() );

	/*
	    c.fVersion:
	    0   SGP8
	    1   SGP8
	    2   SGP4
	*/
	void setCorrectionParams( const TCorrectionParams& c );

	// Расчитывает орбитальные параметры на момент времени, заданный относительно базового.
	// Параметры:
	// rel_time      Относительное время в днях ( может быть отрицательным ).
	void model( double rel_time );
	// Устанавливает текущую модель и сбрасывает параметры коррекции.
	void setModel( OrbitalModel model );

	// время без учета коррекции
	int fIniBaseYear;
	double fIniBaseYearTime;

	// время с учетом коррекции
	int     fBaseYear;
	double  fBaseYearTime;
	// Момент времени, на который сейчас просчитана орбитальная модель. Выражено относительно fBaseYear+fBaseYearTime в днях.
	double  fRelTime;

	//-------------------------------------------------------------------------
	// Параметры, вычисляемые для конкретного момента времени.
	//-------------------------------------------------------------------------
	double r[3];    // Радиус-вектор в неподвижной с.к.
	double v[3];    // Скорость в неподвижной с.к.
	double m;       // Средняя аномалия.
	double E;       // Эксцентрическая аномалия.
	double f;       // Истинная аномалия.
	double ov;      // Угол на Овна.
	double om;      // Восходящий узел.
	double e;       // Eccentricity.
	double n;       // Среднее движение.
	double w;       // Аргумент перигея.
	double R;       // расстояние от центра Земли до ИСЗ
	double i;       // наклонение орбиты
	double a;       // большая полуось орбиты

	//-------------------------------------------------------------------------
	//  Параметры, инициализируемые конструктором.
	//-------------------------------------------------------------------------
	// SGP4, SGP8
	TIniSatParams isp;
	TCorrectionParams cop;

	double ov0;     // угол на Овна
	double n0;      // Original mean motion.
	double a0;      // Semimajor axis.

	// SGP8
	double dn;      // Производная по времени среднего движения.
	double de;      // Производная по времени эксцентриситета.
	double omf, oms;
	double wf, ws;
	double mf, ms;

	// SGP4
	double c1, c2, c3, c4, c5;
	double d2, d3, d4;
	double ksi, beta, etta, teta;
	double dmdt;
	double dwdt;
	double domdt;

private:

	//enum OrbitalModel { SGP4, SGP8 };
	/*
	    Инициализируется конструктором в зависимости от значения поля fVersion параметра cop.
	    По умолчанию (fVersion == 0) используется модель sgp8.
	*/
	OrbitalModel fModel;

	void ini_sgp4();
	void ini_sgp4f();
	void ini_sgp8();

	void sgp4( double rel_time );
	void sgp4f( double rel_time );
	void sgp8( double rel_time );

	static double ovenAngle( int year, double year_time );
};

//-------------------------------------------------------------------------
//  TInverseReferencer
//-------------------------------------------------------------------------
struct TStoreElement;
class TInverseReferencer {
public:
	//TInverseReferencer( const TIniSatParams&, const TNOAAImageParams&, const TCorrectionParams& );
	TInverseReferencer( const TIniSatParams&, const TNOAAImageParams&, const TCorrectionParams& cop = TCorrectionParams() );
	~TInverseReferencer();

	void setCorrectionParams( const TCorrectionParams& );

	//--------------------------------------------------------------------------
	//  ll2xy
	//  Функция решает задачу определения координат (x,y) пиксела снимка
	//  AVHRR, соответствующего географической точке с координатами (lon,lat),
	//  т.е. обратную задачу географической привязки.
	//  Параметры:
	//  lon, lat    Задают географические координаты точки.
	//              -PI <= lon <= PI
	//              -PI/2 <= lat <= PI/2
	//  x, y        Указатели на переменные под результат.
	//  Возвращаемое значение:
	//  0               Точка с указанными координатами не принадлежит снимку.
	//  1               Преобразование (lon,lat)->(x,y) проведено успешно.
	//--------------------------------------------------------------------------
	int ll2xy( double lon, double lat, double *x, double *y );
	int ll2xy( double lon, double lat, int *x, int *y );

private:
	void ini_ll2xy();

	TNOAAImageParams fNIP;
	TCorrectionParams fCOP;
	TOrbitalModel * fOM;

	int fBaseYear;
	double fBaseYearTime;

	double ER, VR;
	double gc_lat, sin_lat, cos_lat, prev_lat;
	TStoreElement * se;

	double maxScanAngle;    // максимальный угол сканирования, выраженный в радианах (центр крайнего пиксела)
};

/*-------------------------------------------------------------------------
    TStraightReferencer
-------------------------------------------------------------------------*/
class TStraightReferencer {
public:
	TStraightReferencer( const TIniSatParams&, const TNOAAImageParams&, const TCorrectionParams& cop = TCorrectionParams() );
	~TStraightReferencer();

	void setCorrectionParams( const TCorrectionParams& );

	/*-------------------------------------------------------------------------
	    Функция решает задачу определения координат (lon,lat) географической
	    точки, соответствующей пикселу снимка AVHRR с координатами (x,y), т.е.
	    прямую задачу географической привязки.
	    Параметры:
	    x               Столбец точки снимка.
	    y               Строка точки снимка.
	    lat, lon        Через эти параметры будет возвращены географические
	                    координаты, соответствующие точке снимка.
	                    -PI < lon <= PI
	                    -PI/2 <= lat <= PI/2
	    r               Необязательный параметр. На выходе: радиус-вектор точки
	                    земной поверхности в с.к., связанной с вращением Земли.
	    Целочисленная версия функции решает прямую задачу географической привязки
	    для центра пиксела с указанными координатами.
	-------------------------------------------------------------------------*/
	void xy2ll( double x, double y, double *lon, double *lat, double * r = 0 );
	void xy2ll( int x, int y, double *lon, double *lat, double * r = 0 );

private:
	void ini_xy2ll_seq( double );
	void xy2ll_seq( double, double *, double *, double * r );

	void ini_matrices( const TCorrectionParams & );    // инициализация матриц myp, mr, ma

	TNOAAImageParams fNIP;
	TCorrectionParams fCOP;
	TOrbitalModel * fOM;

	double fLastScan;
	double sin_ov, cos_ov;
	double myp[3][3];
	double mr[3][3];
	double ma[3][2]; // Матрица преобразования координат, заданных в с.к. сканера, в координаты в неподвижной геоцентрической с.к. Третий столбец не используется.
	double C, b[3];

	double maxScanAngle;    // максимальный угол сканирования, выраженный в радианах (центр крайнего пиксела)
};

//  Функция рассматривает получаемое значение как угол в радианах и
//  возвращает соответствующий ему из интервала [0, 2PI).
double dinpi( double arc );

#endif
