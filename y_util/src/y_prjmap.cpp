/*-------------------------------------------------------------------------
    y_prjmap.cpp
-------------------------------------------------------------------------*/
#include <math.h>
#include <c_lib/c_const.hpp>
#include "y_util/y_prjmap.hpp"


TProjMapper::TProjMapper( ProjType t, double lon, double lat, double lon_size, double lat_size, double lon_res, double lat_res ) :
pt( t ) {
	if( pt == mrt ) {
		double m1 = log( tan( .5*lat + .25*PI ) );
		double m2 = log( tan( .5*(lat + lat_size) + .25*PI ) );
		size_y = long( (m2 - m1) / lat_res + .5 ) + 1;
		lat_a = double(size_y - 1) / (m2 - m1);
		lat_b = -lat_a * m1;
		size_x = long( lon_size / lon_res + .5 ) + 1;
		lon_a = double(size_x - 1) / lon_size;
		lon_b = -lon_a * lon;
	} else {
		size_y = long( lat_size / lat_res + .5 ) + 1;
		lat_a = double(size_y - 1) / lat_size;
		lat_b = -lat_a * lat;
		size_x = long( lon_size / lon_res + .5 ) + 1;
		lon_a = double(size_x - 1) / lon_size;
		lon_b = -lon_a * lon;
	}
}

double TProjMapper::lat( uint32_t scan ) const {
	return pt == mrt ?
		   2. * atan( exp( (double(scan) - lat_b) / lat_a ) ) - PI / 2. :
		   (double(scan) - lat_b) / lat_a;
}

double TProjMapper::lon( uint32_t column ) const {
	return (double(column) - lon_b) / lon_a;
}

uint32_t TProjMapper::scan( double lat ) const {
	return (uint32_t)(pt == mrt ?
				   log( tan( .5 * lat + .25 * PI ) ) * lat_a + lat_b + 1e-12 :  /* добавочка нужна для компенсации погрешности счёта */
				   lat * lat_a + lat_b + 1e-12);
}

double TProjMapper::dScan( double lat ) const {
	return (uint32_t)(pt == mrt ?
				   log( tan( .5 * lat + .25 * PI ) ) * lat_a + lat_b :
				   lat * lat_a + lat_b);
}

uint32_t TProjMapper::column( double lon ) const {
	return (uint32_t)(lon * lon_a + lon_b + 1e-12); /* добавочка нужна для компенсации погрешности счёта */
}

double TProjMapper::dColumn( double lon ) const {
	return lon * lon_a + lon_b;
}

//
//TMercTrans::TMercTrans( double lat, double lat_size, double lat_res ){
//    double m1 = log( tan( .5*lat + .25*PI ) );
//    double m2 = log( tan( .5*(lat + lat_size) + .25*PI ) );
//    double m_res = log( tan( .5*(lat + .5*lat_size + lat_res) + .25*PI ) ) - log( tan( .5*(lat + .5*lat_size) + .25*PI ) );
//    fScanNum = long( (m2-m1) / m_res + .5 ) + 1;
//    fA = (fScanNum - 1) / (m2 - m1);
//    fB = -fA * m1;
//}
//
