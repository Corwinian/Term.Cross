/*-------------------------------------------------------------------------
    c_misc.cpp
-------------------------------------------------------------------------*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include <tc_config.h>
#include "c_lib/c_misc.hpp"
#include "c_lib/c_types.hpp"

#if defined HAVE_DRIVE
#include <direct.h>
#endif

int isFloatingPoint( const char * s ) {
	while( isspace( *s ) )
		s++;
	if( *s == '+' || *s == '-' )
		s++;

	int state = 0;
	for(;;) {
		int c = *s++;
		switch( state ) {
		case 0:
			if( isdigit(c) )
				state = 3;
			else if( c == '.' )
				state = 1;
			else if( c == 'e' || c == 'E' )
				state = 5;
			else
				return 0;
			break;
		case 1:
			if( isdigit(c) )
				state = 2;
			else
				return 0;
			break;
		case 2:
			if( !isdigit(c) ) {
				if( c == 'e' || c == 'E' )
					state = 5;
				else if( c == 0 )
					return 1;
				else
					return 0;
			}
			break;
		case 3:
			if( !isdigit(c) ) {
				if( c == '.' )
					state = 4;
				else if( c == 'e' || c == 'E' )
					state = 5;
				else if( c == 0 )
					return 1;
				else
					return 0;
			}
			break;
		case 4:
			if( !isdigit(c) ) {
				if( c == 'e' || c == 'E' )
					state = 5;
				else if( c == 0 )
					return 1;
				else
					return 0;
			}
			break;
		case 5:
			if( isdigit(c) )
				state = 7;
			else if( c == '+' || c == '-' )
				state = 6;
			else
				return 0;
			break;
		case 6:
			if( isdigit(c) )
				state = 7;
			else
				return 0;
			break;
		case 7:
			if( !isdigit(c) ) {
				return c == 0 ? 1 : 0;
			}
			break;
		}
	}
}

uint32_t padOn32BitBound( uint32_t value ) {
	return ((value + 3) & 0xFFFFFFFC);
}

int check_dir( const char * path ) {
	int l;
	if( !(l = strlen( path )) )
		return 0;
	char s[l];
	strcpy( s, path );
	if( s[l-1] == DIRD )
		s[l-1] = '\0';

#if defined HAVE_DRIVE

	if( strlen( s ) == 2 && s[1] == ':' && isalpha( s[0] ) ) {   // если путь - имя диска
		s[0] = toupper( s[0] );
		int cur_drv = _getdrive();
		int result = _chdrive( 1 + s[0] - 'A' );
		_chdrive( cur_drv );
		return (result == 0 ? 0 : 1);
	}
#endif

	struct stat st;
	return ( stat( s, &st ) == 0 && (st.st_mode & S_IFDIR) ? 0 : 1 );
}

void memDump( FILE * f, const char * b, uint32_t l ) {
	char output[80];
	char offset_format_string[10];

	uint32_t offset_length;        // число символов для печати смещения
	if( l <= 0xffff )
		offset_length = 4;
	else if( l <= 0xffffff )
		offset_length = 6;
	else
		offset_length = 8;
	sprintf( offset_format_string, "%%0%x", offset_length );

	output[37 + offset_length] = 0;     // ASCIIZ

	for( uint32_t i = 0; i < l; i += 16 ) {
		// печать смещения
		int pos = sprintf( output, offset_format_string, i );

		output[pos++] = 0x20;

		// печать шестнадцатиричного дампа
		for( uint32_t j = 0; j < 16; j++ ) {
			if( j % 4 == 0 )
				output[pos++] = 0x20;     // разделяем четверки байт пробелами
			if( i + j < l ) {            // если не дошли до конца буфера
				pos += sprintf( output + pos, "%02x", (uint32_t)b[i+j] );
			} else {
				output[pos++] = 0x20;
				output[pos++] = 0x20;
			}
		}
		fprintf( f, "%s  ", output );

		// печать символьного дампа
		for( uint32_t j = 0; j < 16; j++ ) {
			if( i+j < l && b[i+j]!=0 && b[i+j]!=0x7 && b[i+j]!=0xa && b[i+j]!=0xd && b[i+j]!=0x9 && b[i+j]!=0x8 && b[i+j]!=0x1b )
				fputc( b[i+j], f );
			else
				fputc( '.', f );
		}

		fprintf( f, "\n" );
	}
}

int splitpath(char* path, char* drive, char* dir, char* name, char* ext) {
	if(!path)
		return -1;
	char* drvEnd;
	char* pEnd;
	char* nEnd;
	char* tmp;
	drvEnd = pEnd = nEnd = path;
	for(tmp = path; *tmp != '\0'; tmp++) {
		switch(*tmp) {
		case '\\':
		case '/':
			pEnd = tmp+1;
			break;
		case '.':
			nEnd = tmp;
			break;
		case ':':
			drvEnd = tmp+1;
			break;
		}
	}
	if( pEnd < drvEnd )
		pEnd = drvEnd;
	if( nEnd < pEnd )
		nEnd = tmp;

	if(drive) {
		memcpy(drive, path, (size_t) (drvEnd - path));
		drive[drvEnd - path] = '\0';
	}

	if(dir) {
		memcpy(dir, drvEnd, (size_t) (pEnd - drvEnd));
		dir[pEnd - drvEnd] = '\0';
	}

	if(name) {
		memcpy(name, pEnd, (size_t) (nEnd - pEnd));
		name[nEnd - pEnd] = '\0';
	}

	if(ext) {
		if(*nEnd == '.')
			nEnd++;
		memcpy(ext, nEnd, (size_t) (tmp - nEnd));
		ext[tmp - nEnd] = '\0';
	}

	return 0;
}

int makepath( char* path, const char* drive, const char* dir, const char* name, const char* ext ) {
	char* tmp = path;
	if(drive) {
		strcpy(tmp,drive);
		tmp+=strlen(drive);
	}
	if(dir) {
		strcpy(tmp,dir);
		tmp+=strlen(dir);
	}
	if(name) {
		if(tmp > path && (*(tmp-1)!= '/') && (*(tmp-1)!= '\\') ){
			*tmp = DIRD; *(++tmp)='\0';
		}
//		if(tmp > path){
//			if((*(tmp-1)!= '/')&&(*(tmp-1)!= '\\')){
//				*tmp = DIRD; *(++tmp)='\0';
//			}
//		}
		strcpy(tmp,name);
		tmp+=strlen(name);
	}
	if(ext) {
		if( ext[0] != '.' && strlen(name) > 0 ) {
			*tmp = '.';
			tmp++;
		}
		strcpy(tmp,ext);
	}

	return 0;
}
