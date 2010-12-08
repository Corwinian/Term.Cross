/*-----------------------------------------------------------------------------
    c_sattbl.cpp
-----------------------------------------------------------------------------*/
#include <string.h>

#include <tc_config.h>
#include "c_lib/c_sattbl.hpp"

typedef struct {
	uint32_t id;
	char name[13];
	char prefix[9];
}
TSatInfo;


static TSatInfo satInfo[NUMBER_OF_SATELLITES] = {
			{ 15427, "NOAA 9", "n9" },
			{ 16969, "NOAA 10", "n0" },
			{ 19531, "NOAA 11", "n1" },
			{ 21263, "NOAA 12", "n2" },
			{ 23455, "NOAA 14", "n4" },
			{ 25338, "NOAA 15", "n5" },
			{ 26536, "NOAA 16", "n6" },
			{ 27453, "NOAA 17", "n7" },
			{ 28654, "NOAA 18", "n8" },
			{ 33591, "NOAA 19", "n9" },  // dublicate with noaa-9 !!!
			{ 23522, "GMS 5", "g5" },
			{ 24883, "ORBVIEW 2", "ss"  },
			{ 24834, "FENGYUN 2A", "fa" },
			{ 26382, "FENGYUN 2B", "fb" },
			{ 20788, "FENGYUN 1B", "fb" },
			{ 25730, "FENGYUN 1C", "fc" },
			{ 27431, "FENGYUN 1D", "fd" }
		};


int TSatInfoTable::setToFirst() {
	fIndex = 0;
	return 1;
}


int TSatInfoTable::setToNext() {
	if( fIndex == NOWHERE )
		return 0;
	if( ++fIndex == numberOfSatellites() ) {
		fIndex = NOWHERE;
		return 0;
	}
	return 1;
}


int TSatInfoTable::isValid() const {
	return (fIndex != NOWHERE);
}


int TSatInfoTable::setToSatelliteWithId( uint32_t id ) {
	for( uint32_t i=0; i<numberOfSatellites(); i++ ) {
		if( satInfo[i].id == id ) {
			fIndex = i;
			return 1;
		}
	}
	fIndex = NOWHERE;
	return 0;
}


int TSatInfoTable::setToSatelliteWithName( const char * name ) {
	for( uint32_t i=0; i<numberOfSatellites(); i++ ) {
		if( strcmp( satInfo[i].name, name ) == 0 ) {
			fIndex = i;
			return 1;
		}
	}
	fIndex = NOWHERE;
	return 0;
}


uint32_t TSatInfoTable::satId() const throw( TRequestExc ) {
	if( fIndex == NOWHERE )
		throw TRequestExc( 1, "TSatInfoTable::satId()" );
	return satInfo[fIndex].id;
}


const char * TSatInfoTable::satName() const throw( TRequestExc ) {
	if( fIndex == NOWHERE )
		throw TRequestExc( 1, "TSatInfoTable::satName()" );
	return satInfo[fIndex].name;
}


const char * TSatInfoTable::fileNamePrefix() const throw( TRequestExc ) {
	if( fIndex == NOWHERE )
		throw TRequestExc( 1, "TSatInfoTable::fileNamePrefix()" );
	return satInfo[fIndex].prefix;
}



uint32_t TSatInfoTable::satId( uint32_t i ) const throw( TRequestExc ) {
	if( i >= NUMBER_OF_SATELLITES )
		throw TRequestExc( 1, "TSatInfoTable::satId( uint32_t )" );
	return satInfo[i].id;
}


const char * TSatInfoTable::satName( uint32_t i ) const throw( TRequestExc ) {
	if( i >= NUMBER_OF_SATELLITES )
		throw TRequestExc( 1, "TSatInfoTable::satName( uint32_t )" );
	return satInfo[i].name;
}


const char * TSatInfoTable::fileNamePrefix( uint32_t i ) const throw( TRequestExc ) {
	if( i >= NUMBER_OF_SATELLITES )
		throw TRequestExc( 1, "TSatInfoTable::fileNamePrefix( uint32_t )" );
	return satInfo[i].prefix;
}


uint32_t TSatInfoTable::numberOfSatellites() {
	return NUMBER_OF_SATELLITES;
}


int TSatInfoTable::containsInfoForSatellite( uint32_t sat_id ) {
	for( uint32_t i=0; i<numberOfSatellites(); i++ ) {
		if( satInfo[i].id == sat_id )
			return 1;
	}
	return 0;
}
