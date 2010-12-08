#ifndef _MULTICH_HPP_
#define _MULTICH_HPP_
//int createLog();
void readCfg() throw (TException);
//int checkInputFileNameFormat();
void construct_file_names();
void parseCommandString(int, char**) throw (TException);
int readBlk0(const char*, TBlk0_AVHRR&);
int verifyBlk0(TBlk0_AVHRR&, const char*);
int readDatFile(int, TMultichParams&);
static int strIsEmpty(const char*);
static int strParse(const char*, int&, TMultichParams&);
int multich_processing(const TMultichParams&, TBlk0_AVHRR&, short int*, TBlk0_AVHRR&, short int*, TBlk0_AVHRR&, short int*, TBlk0_AVHRR&, short int*);
int readData(void*, const char*, int, int);
double calcAngle(int, int, double, TStraightReferencer&);
int writeData(void*, TBlk0_AVHRR&, const char*, int);
double calcSatHeight(TOrbitalModel&, int);
double calcAthm(double, double);
#endif
