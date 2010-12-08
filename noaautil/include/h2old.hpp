/*-----------------------------------------------------------------------------
    h2old.hpp
-----------------------------------------------------------------------------*/
//#define INCL_DOSPROCESS
//#include <os2.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <c_lib.hpp>
#include <Log.hpp>
#include <y_util.hpp>

void parseCommandString( int, char * [] ) throw ( TException );
void readCfg() throw ( TException );
void readInputFile() throw ( TException );
void constructOutputFileName();
void constructOldBlk0();
void calculateOutputData();
void saveOutputFile() throw ( TException );
void atExit();
