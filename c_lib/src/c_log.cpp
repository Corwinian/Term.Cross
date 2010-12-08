/*-----------------------------------------------------------------------------
    c_log.cpp
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <tc_config.h>
#include "c_lib/c_log.hpp"
#include "c_lib/c_misc.hpp"

TLog::TLog( const char * file_name, const char * title, MessageLevel level, MessageDisplay display, int append ) throw ( TAccessExc ):
		fLevel( level ),
fDisplay( display ) {
	FILE * f;
	time_t ltime;
	struct tm * local_time;

	int l = strlen(file_name);

	fFileName = new char(l+1);
	fFileName[l] = '\0';	//NULL terminated string

	strncpy( fFileName, file_name, l );

	if( (f = fopen( fFileName, append == 0 ? "w" : "a" )) == NULL ) {        // с указанным файлом невозможно работать
		//throw TAccessExc( 1, "TLog::TLog: ошибка открытия файла" );
		fDisplay = screen;
		f = stderr;
	}

	fprintf( f, "\n\n" );   // отделяем новые записи от уже имеющихся в файле

	if( title )
		fprintf( f, "%s\n", title );    // печать заголовка

	// предваряющее сообщение
	time( &ltime );
	local_time = localtime( &ltime );
	fprintf( f, "Log started at:    %s", asctime( local_time ) );
	fprintf( f, "-------------------------------------------\n\n" );

	fclose( f );
}


TLog::~TLog() {
	FILE * f;
	time_t ltime;
	struct tm * local_time;

	f = fopen( fFileName, "a" );

	// завершающее сообщение
	fprintf( f, "\n-------------------------------------------\n" );
	time( &ltime );
	local_time = localtime( &ltime );
	fprintf( f, "Log finished at:   %s\n\n", asctime( local_time ) );

	fclose( f );
}



int TLog::print( const char * text, MessageLevel priority, MessageDisplay display ) {
	FILE * f;

	if( priority == nothing || priority > fLevel )
		return 1;    // сообщение не напечатано вообще

	// печать в файл
	if( (f = fopen( fFileName, "a" )) == NULL )
		return 2;
	fprintf( f, "%s\n", text );
	fclose( f );

	// печать на экран
	if( display == filescreen && fDisplay == filescreen ) {
		printf( "%s\n", text );
	}

	return 0;
}


int TLog::info( const char * text ) {
	return print( text, inf, file );
}


int TLog::error( const char * text ) {
	return print( text, err, filescreen );
}


int TLog::debug( const char * text ) {
	return print( text, dbg, file );
}


int TLog::dump( const char * buf, uint32_t buf_size, MessageDisplay display ) {
	FILE * f;

	if( fLevel < dmp )
		return 1;   // не печатаем дамп вообще

	// печать в файл
	if( (f = fopen( fFileName, "a" )) == NULL )
		return 2;
	memDump( f, buf, buf_size );
	fclose( f );

	// печать на экран
	if( display == filescreen && fDisplay == filescreen ) {
		memDump( stdout, buf, buf_size );
	}

	return 0;
}


int TLog::dump( const char * text, const char * buf, uint32_t buf_size, MessageDisplay display ) {
	int r;
	if( (r = print( text, dmp, display )) != 0 )
		return r;
	return dump( buf, buf_size, display );
}

