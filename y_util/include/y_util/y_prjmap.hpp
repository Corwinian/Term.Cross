/*-------------------------------------------------------------------------
    y_prjmap.hpp
-------------------------------------------------------------------------*/
#ifndef _Y_PRJMAP_HPP_
#define _Y_PRJMAP_HPP_

#include <tc_config.h>
#include <c_lib/c_types.hpp>


class TProjMapper {
public:
	enum ProjType { mrt = 1, eqd };

	// все параметры - в радианах
	TProjMapper( ProjType t, double lon, double lat, double lon_size, double lat_size, double lon_res, double lat_res );

	uint32_t sizeX() const {
		return size_x;
	}
	uint32_t sizeY() const {
		return size_y;
	}

	// узнать долготу по номеру столбца
	// радианы
	// Внимание !!! Если проекция проходит через 180-меридиан, т.е. левый край находится в восточном полушарии, а правый - в
	// западном, то для столбцов, попадающих в западное полушарие, будут возвращаться значения >PI.
	double lon( uint32_t column ) const;
	// узнать широту по номеру скана
	// радианы
	double lat( uint32_t scan ) const;

	// узнать номер скана по широте
	// lat  широта в радианах
	uint32_t scan( double lat ) const;
	// возвращает неокруглённое значение
	double dScan( double lat ) const;

	// узнамть номер столбца по долготе
	// lon  долгота в радианах
	// Внимание !!! Если проекция проходит через 180-меридиан, т.е. левый край находится в восточном полушарии, а правый - в
	// западном, то для получения номеров столбцов, попадающих в западное полушарие, должны указываться значения lon>PI.
	uint32_t column( double lon ) const;
	// возвращает неокруглённое значение
	double dColumn( double lon ) const;

private:
	ProjType pt;
	double lat_a, lat_b;
	double lon_a, lon_b;
	uint32_t size_x, size_y;
};


#endif
