#ifndef _ANGLESCAN_HPP_
    #define _ANGLESCAN_HPP_


#define ANGLE_SCAN_NOAA_12_14_15_17 55.37135

#define ANGLE_SCAN_NOAA_16 55.25


double anglescan( long x, uint32_t id )
{
    if( id != uint32_t(TSatInfoTable::sat_id_noaa_16) ) return ( abs( 1024 - x ) * ( ANGLE_SCAN_NOAA_12_14_15_17 / 1023.5 ) );
    else return ( abs( 1024 - x ) * ( ANGLE_SCAN_NOAA_16 / 1023.5 ) );
}

#endif
