/*-----------------------------------------------------------------------------
    hprj.hpp
-----------------------------------------------------------------------------*/
//#define INCL_DOSPROCESS
//#include <os2.h>

void parseCommandString( int, char * [] ) throw ( TException );
void readCfg() throw ( TException );
void readInputFile() throw ( TException );
void buildProj() throw ( TException );
void saveProjFile() throw ( TException );
void atExit();
void correctPixels();
void constructProjBlk0();
