/*-----------------------------------------------------------------------------
    hunp.hpp
-----------------------------------------------------------------------------*/
//#define INCL_DOSPROCESS
//#include <os2.h>

void parseCommandString( int, char * [] ) throw ( TException );
void readCfg() throw ( TException );
void construct_file_names();
void construct_blk0();
void atExit();
