/*
*/
#ifndef _CALIBR_HPP_
#define _CALIBR_HPP_

//#include <cmath>
//#include <stdarg.h>
//#include <cstdarg>
//#include <string>

uint64_t t;

#define max(x,y) fmax(x,y)
void albcof(int, int, XML&, TAlbedoCalParams&) throw (int);
void calcLinearConversionParams(double, TTelemetryData&, TCorrParams&, double*, double*);
void calculateLook_up_table(short int*, int, double, double, double, TCorrParams&, double*, double, double, int, double);
void calculateTarget(TInputParams&, TTelemetryData&, T_NFR&, double*, double*) throw (int);
void calibr_processing(TBlk0_AVHRR&, const char*, const char*, const char*, XML&) throw (int);
void construct_file_names();
double find_t(double, double*, double, double, int);
int findMinMaxPositiveValue(short int*, int, short int*, short int*);
double get_temp(int, double, double, double, TCorrParams, double*, double, double, int);
void ini_corrParams(TInputParams&, TCorrParams&) throw (int);
void ini_lut_T2R(T_NFR&, double*, double, double, int);
void ini_nfr(TInputParams&, T_NFR&) throw (int);
short int medh(short int*, int, int);
short int medin(short int*, int, int, int, int*);
void navigationSystemInit(const TBlk0&, TStraightReferencer**, double*) throw (int);
void parseCommandString( int, char * [] ) throw (TException);
int parseStringOfDouble(string& , double*, int, int, string::iterator*) throw (TException);
void proccesAscent(short int*, int, int, TStraightReferencer&, double, double, double);
void processLUT(short int*, short int*, int, short int*);
void readBlk0() throw (int);
void readCalibrDatFile(XML**) throw (int);
void readCfg() throw (TException);
void readDataFile(void*, const char*, int, int) throw (int);
void tabalb(short int*, int, TAlbedoCalParams&, double*, double*, int*);
void telemetry_processing(TTelemetryData&, short int*, int, const char*) throw (int);
double tem2rad(double, T_NFR&);
void verifyBlk0(TBlk0_AVHRR&, const char*) throw (int);
void writeFileNOAA(const char*, TBlk0_AVHRR&, short int*, int) throw (int);
#endif
