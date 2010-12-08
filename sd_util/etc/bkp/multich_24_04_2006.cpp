/*-----------------------------------------------------------------------------
    multich.cpp
-------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <c_lib.hpp>
#include <orbmodel.hpp>
#include <Log.hpp>

#include "auto_ptr.hpp"
#include "astronom.hpp"

#ifndef LOG_FORMAT
#define LOG_FORMAT "%d %t %e %m"
#endif

char useMsg[] = "\n\
�ᯮ�짮�����: multich [��樨] ���_���䨣��樨 䠩�_������\n\
䠩�_������     ��ᯠ������� � �⪠���஢���� 䠩� �⢥�⮣� ������.\n\
*** ᯥ���᪨� ��権 �⨫�� �� ����� ***\n\
��樨, �ࠢ���騥 ������ ᮮ�饭�� � log-䠩�:\n\
-llevel={dump,debug,info,warning,error,fatal} �஢��� �뢮�� ᮮ�饭��.\n\
-lerr           �뢮���� ᮮ�饭�� � �⠭����� ��⮪ �訡��.\n\
-lout           �뢮���� ᮮ�饭�� � �⠭����� ��⮪ �뢮��.\n\
-lfile=<name>   �뢮���� ᮮ�饭�� � 䠩� � ������ name,\n\
                䠩� ᮧ������ ������\n\
-lappend=<name> �뢮���� ᮮ�饭�� � 䠩� � ������ name,\n\
                ᮮ�饭�� �����뢠���� � ����� 䠩��\n";

// ���祭�� ��६�����  �।� TERM_ROOT
char * term_root = ".";

/*
	��樨 �� ���������� ��ப� ����� ����� ��᮪�� �ਮ��� 祬 ��樨 �� 䠩�� ���䨣��権.
*/

nsCLog::eSeverity logLevel = nsCLog::info;
bool useStdErr = false;
bool useStdOut = false;
string logFileName = string();
bool append = false;
CLog *logfile;

char exeFileName[MAX_PATH];    // argv[0]
char cfgName[MAX_FNAME];

// "nxxxxxx_3.clb", "nxxxxxx_4.clb", "nxxxxxx_5.clb"
char inputFileName[3][MAX_PATH];
char outputFileName[MAX_PATH];

// ��ࠬ���� �⬮��୮� ���४樨
struct TMultichParams {
	// ���� ���宦����� �� �����
	// �᫨ 㣮� ���宦����� �� ����� ��� ������ �窨 ����� angle_day --- ��⠥� �� ����,
	// �᫨ 㣮� ���宦����� �� ����� ��� ������ �窨 ����� ����� angle_night --- ��⠥� �� ����.
	double angle_day;
	double angle_night;

	// ��ࠬ���� 䨫���樨
	double min_delta_day;  // �᫨ T_4 - T_5 \not\in [min_delta_day, max_delta_day]
	double max_delta_day;  // ��� ��⠥� ����ﭭ��

	double min_delta_night; // �᫨ T_4 - T_5 \not\in [min_delta_night, max_delta_night]
	double max_delta_night; // ��� ��⠥� ����ﭭ��
	// ��ࠬ���� ���४樨
	double a_day[4];
	double a_night[4];
};

const int lost_point = -4;      // �⨬ ���祭��� ���� ��������� ����ﭭ� ���ᥫ�
const double KELVIN0 = 273.16;

char msg[64000];                      // ���� ��� ᮮ�饭�� ����

#include "multich.hpp"

int main( int argc, char * argv[] ) {

	if( argc < 3 ) {
		printf( "%s", useMsg );
		return 0;
	}

	char * t;
	if(NULL!=(t = getenv("TERM_ROOT"))){
		term_root = strdup(t);
	}else{
		fprintf( stderr, "��६����� TERM_ROOT �� ��⠭������, �㤥� �ᯮ�짮��� ⥪�騩 ����.\n" );
	}

	strcpy( exeFileName, argv[0] );
	strcpy( inputFileName[0], argv[argc-1] );
	strcpy( cfgName, argv[argc-2] );

	try {
		readCfg();
		parseCommandString( argc, argv );
	} catch (TException e) {
		fprintf( stderr, "%s\n", e.text() );
		exit(-1);
	}

	logfile = new CLog("multich.exe");
	if(useStdOut) {
		//logfile.addAppender(new ConsoleLogAppender(logLevel,"%s | %e | %m"));
		//logfile->addAppender(new CConsoleLogAppender(logLevel, "%e | %m"));
		logfile->addAppender(new CConsoleLogAppender(logLevel, LOG_FORMAT));
	}
	if(useStdErr) {
		//logfile.addAppender(new EConsoleLogAppender(logLevel,"%s | %e | %m"));
		//logfile->addAppender(new EConsoleLogAppender(logLevel, "%e | %m"));
		logfile->addAppender(new EConsoleLogAppender(logLevel, LOG_FORMAT));
	}
	if(!logFileName.empty()) {
		try {
			//logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, "%s | %e | %m"));
			logfile->addAppender(new CFileLogAppender(logLevel, logFileName, append, LOG_FORMAT));
		} catch(string e) {
			fprintf(stderr,e.c_str());
			exit(1);
		}
	}

	TAutoPtr<CLog> aLogFile( logfile );

	logfile->info( string("multich.exe - ") + inputFileName[0] );

	construct_file_names();

	TBlk0_AVHRR in4_blk0;
	TBlk0_AVHRR in5_blk0;
	TBlk0_AVHRR out_blk0;

	if( readBlk0( inputFileName[1], in4_blk0 ) != 0 ||
			readBlk0( inputFileName[2], in5_blk0 ) ) {
		return 1;
	}

	if( verifyBlk0( in4_blk0, inputFileName[1] ) != 0 ||
			verifyBlk0( in5_blk0, inputFileName[2] ) != 0 ) {
		return 1;
	}

	logfile->info( "�⥭�� 䠩�� ����஥� (multich.dat)" );
	int satId = in4_blk0.b0.satId;
	TMultichParams mParams;
	if( readDatFile( satId, mParams ) != 0 ) {
		return 1;
	}

	int length = in4_blk0.totalFrameNum*in4_blk0.totalPixNum;
	short *in4_data = new short[length];
	short *in5_data = new short[length];
	short *out_data = new short[length];

	logfile->info( "�⥭�� 䠩�� ������ 4-�� ������" );
	if( readData( in4_data, inputFileName[1], length*sizeof(short), 512 ) != 0 ) {
		return 1;
	}

	logfile->info( "�⥭�� 䠩�� ������ 5-�� ������" );
	if( readData( in5_data, inputFileName[2], length*sizeof(short), 512 ) != 0 ) {
		return 1;
	}

	logfile->info( "����஥��� ᪮�४�஢����� ⥬������" );
	if( multich_processing( mParams,
							in4_blk0, in4_data,
							in5_blk0, in5_data,
							out_blk0, out_data ) != 0 ) {
		return 1;
	}


	logfile->info( "������ ᪮�४�஢����� ⥬������" );
	writeData( out_data, out_blk0, outputFileName, sizeof(short)*length );

	delete out_data;
	delete in5_data;
	delete in4_data;

	logfile->info( "" );
	logfile->info( "ࠡ�� �ணࠬ�� �����襭� �ᯥ譮 !" );
	return 0;
}

void readCfg() throw (TException) {
	char path[MAX_PATH];
	const char * s;
	TCfg* cfg = NULL;

	if((NULL == strchr(cfgName,'\\'))&&
			(NULL == strchr(cfgName,'/'))&&
			(NULL == strchr(cfgName,'.'))) {
		strcpy(path,term_root);
		int t = strlen(path);
		/* �᫨ ����, � ���� ������塞 ࠧ����⥫� */
		if(path[t-1]!='/'||path[t-1]!='\\'){
			path[t] = DIRD; path[t+1] = '\0';
		}
#if DIRD == '/'
		strcat( path, "cfg/" );
#else
		strcat( path, "cfg\\" );
#endif
		strcat( path, cfgName );
		strcat( path, ".cfg" );
	} else {
		strcpy(path,cfgName);
	}

	cfg = new TCfg( path );

	try {
		s = cfg->getValue( "LOG_LEVEL" );           /* �����⨬� ���祭��: dump,debug,info,warning,error,fatal */
		if( nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(string(s))))
			logLevel = nsCLog::info;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_STDERR" );           //
		if('1' == *s)
			useStdErr = true;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_STDOUT" );           //
		if('1' == *s)
			useStdOut = true;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_APPEND" );           //
		logFileName = s;
		append = true;
	} catch(...) {}
	try {
		s = cfg->getValue( "LOG_FILE" );         /* ��� 䠩�� */
		logFileName = s;
		append = false;
	} catch(...) {}

	delete cfg;
}

void construct_file_names() {
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], fext[MAX_EXT]; //, s[_MAX_FNAME];
	splitpath( inputFileName[0], drive, dir, fname, fext );
	int l = strlen(fname) - 1;

	fname[l] = '3';
	makepath( inputFileName[0], drive, dir, fname, "clb" );
	fname[l] = '4';
	makepath( inputFileName[1], drive, dir, fname, "clb" );
	fname[l] = '5';
	makepath( inputFileName[2], drive, dir, fname, "clb" );
	fname[l] = 'm';
	makepath( outputFileName, drive, dir, fname, "mch" );

	logfile->debug( "䠩�� ������:" );
	//  logfile->debug( inputFileName[0] );   // ���� ��⨩ ����� � ��� �� �ᯮ������
	logfile->debug( inputFileName[1] );
	logfile->debug( inputFileName[2] );

	logfile->debug( "䠩� १����:" );
	logfile->debug( outputFileName );
}

void parseCommandString( int argc, char* argv[] ) throw (TException) {
	for( int i = 1; i < argc; i++ ) {
		char * s = argv[i];
		if( *s == '-' ) {
			s++;
			if( *s == 'l' ) {
				s++;
				if(!strcmp(s,"err"))
					useStdErr = true;
				else if(!strcmp(s,"out"))
					useStdOut = true;
				else if(!strncmp(s,"file=",5)) {
					s+=5;
					if(strlen(s)==0)
						throw TException(100, "�������⨬�� ���祭�� ��ࠬ��� -lfile=<file name>");
					logFileName.assign((char*)s);
					append = false;
				} else if(!strncmp(s,"append=",7)) {
					s+=7;
					if(strlen(s)==0)
						throw TException(100, "�������⨬�� ���祭�� ��ࠬ��� -lappend=<file name>");
					logFileName.assign((char*)s);
					append = true;
				} else if(!strncmp(s,"level=",6)) {
					s+=6;
					if(nsCLog::unknown == (logLevel = nsCLog::getThresholdFromString(string(s))))
						throw TException(100, "�������⨬�� ���祭�� ��ࠬ��� -llevel=<message level>");
				} else {
					sprintf(msg,"��������� ��ࠬ��� %s",argv[i]);
					throw TException(100, msg);
				}
			} else {
				/* 㪠���� �������⭠� ���� */
				sprintf(msg,"��������� ��ࠬ��� %s",argv[i]);
				throw TException(100, msg);
			}
		}
	}
}

int readBlk0( const char* name, TBlk0_AVHRR &b0 ) {
	FILE* f = fopen( name, "rb" );
	if( f == 0 ) {
		sprintf( msg, "�訡�� ������ 䠩�� %s", name );
		logfile->error( msg );
		return 1;
	}
	if( fread( &b0, sizeof(b0), 1, f ) != 1 ) {
		sprintf( msg, "�訡�� �⥭�� 䠩�� %s", name );
		logfile->error( msg );
		return 1;
	}
	fclose( f );
	return 0;
}

int verifyBlk0( TBlk0_AVHRR &blk0, const char * name = 0 ) {
	if( blk0.b0.formatType != 0xFF ) {
		if( name != 0 ) {
			sprintf( msg, "���ࠢ���� �ଠ� �㫥���� ����� 䠩�� %s", inputFileName[1] );
			logfile->error( msg );
		} else {
			logfile->error( "���ࠢ���� �ଠ� �㫥���� �����" );
		}
		return 1;
	}
	return 0;
}


int readDatFile( int satId, TMultichParams & mParams ) {
	char path[MAX_PATH];
	static char buf[2048];

	strcpy(path,term_root);
	int t = strlen(path);
	/* �᫨ ����, � ���� ������塞 ࠧ����⥫� */
	if(path[t-1]!='/'||path[t-1]!='\\'){
		path[t] = DIRD; path[t+1] = '\0';
	}
#if DIRD == '/'
		strcat( path, "data/multich.dat" );
#else
		strcat( path, "data\\multich.dat" );
#endif
	logfile->debug( "䠩� multich.dat:" );
	logfile->debug( path );
	logfile->debug( "" );

	FILE * f = fopen( path, "r" );
	if( f == 0 ) {
		sprintf( msg, "�訡�� ������ 䠩�� %s", path );
		logfile->error( msg );
		return 1;
	}

	int line = 0;
	int res = 2;
	int id;
	while ( fgets( buf, sizeof(buf), f ) && res == 2 ) {
		line++;
		if( !strIsEmpty(buf) ) {
			if( strParse(buf, id, mParams ) != 0 ) {
				res = 1;
			} else if( id == satId ) {
				res = 0;
			}
		}
	}

	if( res == 2 ) {
		sprintf( msg, "� 䠩�� %s ���������� ����� �� ��⭨�� %d", path, satId );
		logfile->error( msg );
	} else if( res == 1 ) {
		sprintf( msg, "�訡�� ࠧ��� ��ப� %d 䠩�� %s", line, path );
		logfile->error( msg );
	}
	fclose(f);

	return res;
}

static int strIsEmpty( const char * buf ) {
	const char * p = buf;
	while( *p && isspace(*p) )
		p++;
	if( *p == '\0' || *p == '#' )
		return 1;
	return 0;
}

static int strParse( const char * buf, int& satId, TMultichParams & params ) {
	int id;
	int ns = sscanf( buf, "\
					 %d \
					 %lf %lf\
					 %lf %lf\
					 %lf %lf\
					 %lf %lf %lf %lf\
					 %lf %lf %lf %lf",
					 &id,
					 &params.angle_day,
					 &params.angle_night,
					 &params.min_delta_day,
					 &params.max_delta_day,
					 &params.min_delta_night,
					 &params.max_delta_night,
					 &params.a_day[0], &params.a_day[1], &params.a_day[2], &params.a_day[3],
					 &params.a_night[0], &params.a_night[1], &params.a_night[2], &params.a_night[3] );

	if( ns == 15 ) {
		sprintf( msg, "satId = %d", id );
		logfile->debug( msg );
		sprintf( msg, "angle_day = %lf    angle_night = %lf", params.angle_day, params.angle_night );
		logfile->debug( msg );
		sprintf( msg, "min_delta_day = %lf     max_delta_day = %lf", params.min_delta_day, params.max_delta_day );
		logfile->debug( msg );
		sprintf( msg, "min_delta_night = %lf   max_delta_night = %lf", params.min_delta_night, params.max_delta_night );
		logfile->debug( msg );
		logfile->debug( " SST = B_1 (T_11) + B_2 (T_11-T_12) + B_3 (T_11-T_12) (sec(theta)-1) - B_4" );
		sprintf( msg, "day   B_1 = %lf   B_2 = %lf   B_3 = %lf   B_4 = %lf ",
				 params.a_day[0], params.a_day[1], params.a_day[2], params.a_day[3] );
		logfile->debug( msg );
		sprintf( msg, "night B_1 = %lf   B_2 = %lf   B_3 = %lf   B_4 = %lf ",
				 params.a_night[0], params.a_night[1], params.a_night[2], params.a_night[3] );
		logfile->debug( msg );
		satId = id;
		return 0;
	}
	return 1;
}


int multich_processing( const TMultichParams &mParams,
						TBlk0_AVHRR &in4_blk0, short *in4_data,
						TBlk0_AVHRR &in5_blk0, short *in5_data,
						TBlk0_AVHRR &out_blk0, short *out_data
					  ) {
	int lines = in4_blk0.totalFrameNum;
	int cols = in4_blk0.totalPixNum;
	int length = lines*cols;

	float *chmFloat = new float [length];
	double *theta = new double[cols];

	// ��� ��।������ 㣫� ���宦����� �� ᮫��
	TNOAAImageParams NOAAImageParams( (TBlk0&)in4_blk0 );
	TCorrectionParams corrParams((TBlk0&)in4_blk0);
	double julian_date = julian(NOAAImageParams.fYear, NOAAImageParams.fYearTime + 1);
	TIniSatParams iniSatParams( (TBlk0&)in4_blk0);
	TOrbitalModel orbitalModel(iniSatParams, NOAAImageParams.fYear, NOAAImageParams.fYearTime + 1, corrParams );
	TStraightReferencer straightReferencer(iniSatParams, NOAAImageParams, corrParams );

	double avarage_h = calcSatHeight( orbitalModel, lines );
	sprintf(msg, "�।��� ���� ��⭨�� �� ᭨���: %.2lf �������஢.", avarage_h);
	logfile->debug(msg);

	logfile->debug("");
	logfile->debug("�⬮��ୠ� ����");
	logfile->debug("  ���.      㣮�.           m");
	// �����뢠�� ��ᥪ���� 㣫�� ������� ����� ᪠��஢����
	for (int i = 0; i < cols; i++) {
		double fi = fabs(54 * DR * ((2.*i)/cols-1));
		theta[i] = calcAthm( fi, avarage_h );
		{
			sprintf(msg, "%6d %10lf %10lf", i, fi, theta[i] );
			logfile->debug(msg);
		}
	}

	//
	// ����⢥��� ��ࠡ�⪠
	//
	for( int i = 0; i < lines; i++ ) {
		int col1 = 0;
		int col2 = 128;
		double angle1 = calcAngle( i, col1, julian_date, straightReferencer );   // ���� ���宦����� �� �����
		double angle2 = calcAngle( i, col2, julian_date, straightReferencer );   // ���� ���宦����� �� �����
		for( int j = 0; j < cols; j++ ) {        // ���� �� ��ப� ᭨���
			int pix = i*cols+j;
			double angle;   // 㣮� ���宦����� �� �����
			if( j > col2 ) {
				col1 = col2;
				col2 += 128;
				if( col2 >= cols )
					col2 = cols - 1;
				angle1 = angle2;
				angle2 = calcAngle( i, col2, julian_date, straightReferencer );
			}
			if( j == col1 ) {
				angle = angle1;
			} else if( j == col2 ) {
				angle = angle2;
			} else {
				angle = angle1 + (angle2 - angle1)*double(j-col1)/double(col2-col1);
			}

			if ( in4_data[pix] < 0 ) {
				out_data[pix] = in4_data[pix];
				continue;
			}

			// ����뢠�� �।��� ࠧ���� �⢥�⮣�-��⮣� �������
			double delta45 = 0;
			int n = 0;
			for( int i1 = -1; i1 <=1; i1++ ) {
				for( int j1 = -1; j1 <= 1; j1++ ) {
					if( i + i1 >= 0 && i + i1 < lines &&
							j + j1 >= 0 && j + j1 < cols ) {
						int l_pix = cols*(i+i1) + (j+j1);
						if( in4_data[l_pix] >= 0 && in5_data[l_pix] >= 0 ) {
							double T4 = in4_data[l_pix]*in4_blk0.ka + in4_blk0.kb;
							double T5 = in5_data[l_pix]*in5_blk0.ka + in5_blk0.kb;
							delta45 += (T4-T5);
							n++;
						}
					}
				}
			}

			if( n == 0 ) {
				out_data[pix] = lost_point;
				continue;
			}
			delta45 /= double(n);

			if( angle > mParams.angle_day ) {    // ��ࠡ�⪠ ��� ������� �祪
				if ((delta45 < mParams.min_delta_day) ||
						(delta45 > mParams.max_delta_day)) {
					out_data[pix] = lost_point;
				} else {
					double t4 = in4_data[pix] * in4_blk0.ka + in4_blk0.kb;
					double a =
						(t4 + KELVIN0) * mParams.a_day[0] +
						delta45 * mParams.a_day[1] +
						delta45 * mParams.a_day[2] * theta[j] -
						mParams.a_day[3];
					chmFloat[pix] = a;
					out_data[pix] = 1;
				}
			} else if( angle < mParams.angle_night ) {    // ��ࠡ�⪠ ��� ����� �祪
				if ((delta45 < mParams.min_delta_night) ||
						(delta45 > mParams.max_delta_night)) {
					out_data[pix] = lost_point;
				} else {
					double t4 = in4_data[pix] * in4_blk0.ka + in4_blk0.kb;
					double a =
						(t4 + KELVIN0) * mParams.a_night[0] +
						delta45 * mParams.a_night[1] +
						delta45 * mParams.a_night[2] * theta[j] -
						mParams.a_night[3];
					chmFloat[pix] = a;
					out_data[pix] = 1;
				}
			} else {      // �窨, �ᯮ������� ����� �����묨 � ���묨,
				// �� �ய�᪠��, � �ਤ��� �� �஬������ ���祭��
				if( (delta45 < mParams.min_delta_night) ||
						(delta45 < mParams.min_delta_day)   ||
						(delta45 > mParams.max_delta_night) ||
						(delta45 > mParams.max_delta_day) ) {
					out_data[pix] = lost_point;
				} else {
					double t4 = in4_data[pix] * in4_blk0.ka + in4_blk0.kb;
					double a_day =
						(t4 + KELVIN0) * mParams.a_day[0] +
						delta45 * mParams.a_day[1] +
						delta45 * mParams.a_day[2] * theta[j] -
						mParams.a_day[3];
					double a_night =
						(t4 + KELVIN0) * mParams.a_night[0] +
						delta45 * mParams.a_night[1] +
						delta45 * mParams.a_night[2] * theta[j] -
						mParams.a_night[3];
					double k1 = (angle - mParams.angle_night) / (mParams.angle_day - mParams.angle_night);
					double k2 = (mParams.angle_day - angle) / (mParams.angle_day - mParams.angle_night);
					double a = a_day * k1 + a_night * k2;
					chmFloat[pix] = a;
					out_data[pix] = 1;
				}
			}
		}
	}
	//
	// ��ࠡ�⪠ �����襭�
	//

	// ��।������ ��ࠬ��஢ �࠭���� ������
	double minT = 0;
	double maxT = 0;
	int flag = 1;
	for( int i = 0; i < lines; i++ ) {
		for( int j = 0; j < cols; j++ ) {
			int pix = i * cols + j;
			if( out_data[pix] >= 0 ) {
				if( flag || chmFloat[pix] < minT ) {
					flag = 0;
					minT = chmFloat[pix];
				}
			}
			if( flag || chmFloat[pix] > maxT ) {
				flag = 0;
				maxT = chmFloat[pix];
			}
		}
	}
	if( flag ) {
		logfile->error( "�� �窨 䠩�� ��䨫��஢���" );
		delete chmFloat;
		delete theta;
		return 1;
	}

	sprintf( msg, "�������쭠� ����祭��� ⥬������ %lf", minT );
	logfile->debug( msg );
	sprintf( msg, "���ᨬ��쭠� ����祭��� ⥬������ %lf", maxT );
	logfile->debug( msg );

	// �������� ������ �㫥���� �����
	out_blk0 = in4_blk0;
	out_blk0.ka = in4_blk0.ka;
	out_blk0.kb = floor(minT);
	out_blk0.processLevel |= 2;

	// ���������� ����� ������
	int maxValue = 0;
	for( int i = 0; i < lines; i++ ) {
		for( int j = 0; j < cols; j++ ) {
			int pix = i * cols + j;
			if( out_data[pix] >= 0 ) {
				out_data[pix] =
					int(floor((chmFloat[pix] - out_blk0.kb)/out_blk0.ka + 0.5));
				if( out_data[pix] > maxValue )
					maxValue = out_data[pix];
			}
		}
	}
	out_blk0.maxPixelValue = maxValue;
	delete chmFloat;
	delete theta;
	return 0;
}

int readData( void* data, const char* fName, int length, int blk0Len = 0 ) {
	FILE * f = fopen( fName, "rb" );
	if( f == 0 ) {
		sprintf( msg, "�訡�� ������ 䠩�� %s", fName );
		logfile->error( msg );
		return 1;
	}
	if( fseek( f, blk0Len, SEEK_SET ) != 0 ) {
		sprintf( msg, "�訡�� ����樮��஢���� � 䠩�� %s", fName );
		logfile->error( msg );
		return 1;
	}
	if( fread( data, length, 1, f ) != 1 ) {
		sprintf( msg, "�訡�� �⥭�� 䠩�� %s", fName );
		logfile->error( msg );
		return 1;
	}
	fclose(f);
	return 0;
}


/*----------------------------------------*\
 ANGLE.C
 �㭪�� �����⢫�� ���᫥��� 㣫�
 ᪫������ ����� ��� �窨 ��室���� ᭨���
 (� �ࠤ���).
 �맮�:
 angle( nscan, col )
 nscan - ����� ��ப�
 col - ����� ���ᥫ� � ��ப�
 ������ ���� �ந��樠����஢��� ��������
 ��६���� julian_date � satParams.
 �஬� �⮣� ��࠭�� �맢��� �㭪�� ini_sgp8
\*----------------------------------------*/
double calcAngle(int nscan, int col, double date, TStraightReferencer & r ) {
	double lat;
	double lon;
	double fi;

	r.xy2ll(col, nscan, &lon, &lat);
	if ((lon >= 0.) && (lon <= PI))
		lon = -lon;
	fi = shcrds( date, lon, lat);
	return (fi);
}


int writeData( void* data, TBlk0_AVHRR & blk0, const char* fName, int length ) {
	FILE * f = fopen( fName, "wb" );
	if( f == 0 ) {
		sprintf( msg, "�訡�� ������ 䠩�� %s", fName );
		logfile->error( msg );
		return 1;
	}
	if( fwrite( &blk0, sizeof(TBlk0_AVHRR), 1, f ) != 1 ||
			fwrite( data, length, 1, f ) != 1 ) {
		sprintf( msg, "�訡�� ����� � 䠩� %s", fName );
		logfile->error( msg );
		return 1;
	}
	fclose(f);
	return 0;
}

double calcSatHeight( TOrbitalModel &o, int scans ) {
	double a = 0;
	int n = 0;
	for (int i = 0; i < scans; i += 10) {
		o.model(i / 6.0 / (60. * 60. * 24.));
		a += sqrt(o.r[0] * o.r[0] + o.r[1] * o.r[1] + o.r[2] * o.r[2]);
		n++;
	}
	a /= n;      // ������ avarage_h --- ����ﭨ� �� 業�� ����� �� ��⭨��
	return a - 6370.;   // 6370. --- ࠤ��� �����
}

/*
 ����� �⬮��୮� �����
 � ��⮬ ��ਢ����� ������ �����孮��
 sec(theta) = 1/cos(theta)
 cos(theta) = sqrt(1 - sin^2(theta))
 sin(theta) = sin(fi)*(1+avarage_h/R)	(�� ��㫥 ᨭ�ᮢ)
 */
double calcAthm( double fi, double avarage_h ) {
	const double R = 6370.;      // ������ �����
	//const double h = 820.;      // �।��� ���� ��⭨�� ��� ��ਧ��⮬
	//    double sinAzimut = (1. + avarage_h / R) * sin(fi);
	//    return  1. /sqrt(1. - sinAzimut * sinAzimut) - 1.;
	double sinAzimut = (1. + avarage_h / R) * sin(fi);
	return  1. /sqrt(1. - sinAzimut * sinAzimut) - 1.;
}
