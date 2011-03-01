#ifndef _FILTR_HPP_
#define _FILTR_HPP_
void parseCommandString(int, char**, TFiltrParams&) throw (TException);
//int createLog(TLog**);
void readCfg(Config&, TFiltrParams&) throw (TException);
void readFiltrParams(const Setting&, TFiltrParams&, int satId = 0 ) throw (TException);
int checkInputFileNameFormat();
void construct_file_names();
int read_and_verify_Blk0s(TFiltrParams&);
int chan2_is_need(TFiltrParams&);
void chan2_disable(TFiltrParams&);
int chan3_is_need(TFiltrParams&);
void chan3_disable(TFiltrParams&);
int chan4_is_need(TFiltrParams&);
int chan5_is_need(TFiltrParams&);
int readBlk0(const char*, TBlk0_AVHRR&);
int verifyBlk0(TBlk0_AVHRR&, const char*);
int readData(TFiltrParams&, int);
int readDataFile(void*, const char*, int, int);
int writeFileNOAA(const char*, TBlk0_AVHRR&, short int*, int);
TStraightReferencer* navigationSystemInit(const TBlk0&, double*) throw (TRequestExc);
double angle(int, int, TStraightReferencer&, double);
void printStats(int*);
int correctData(short int*, TBlk0_AVHRR&);
int isVisualChannel(TBlk0_AVHRR&);
#endif
