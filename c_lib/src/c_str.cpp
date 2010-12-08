/*-----------------------------------------------------------------------------
	c_str.cpp
-----------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include <tc_config.h>
#include "c_lib/c_str.hpp"

TCharString::TCharString( const char * s ):
len( strlen( s ) ){
	buf_size =
			len < 16 ? 32 :
			len < 128 ? 256 : len + 256;
	buf = new char [ buf_size ];
	memcpy( buf, s, len );
	buf[len] = 0;
}

TCharString & TCharString::assign( const TCharString & s ){
	len = s.len;

	if( len < buf_size ){ // в этом случае увеличивать размер нашего буфера не надо
		memcpy( buf, s.buf, len );
		buf[len] = 0;
	}else{
		delete [] buf;
		buf_size = s.buf_size;
		buf = new char [ buf_size ];
		memcpy( buf, s.buf, len );
		buf[len] = 0;
	}

	return *this;
}

TCharString & TCharString::assign( const char * s ){
	len = strlen( s );

	if( len < buf_size ){ // в этом случае увеличивать размер нашего буфера не надо
		memcpy( buf, s, len );
		buf[len] = 0;
	}else{
		delete [] buf;
		buf_size =
				len < 16 ? 32 :
				len < 128 ? 256 : len + 256;
		buf = new char [ buf_size ];
		memcpy( buf, s, len );
		buf[len] = 0;
	}

	return *this;
}

TCharString & TCharString::append( const TCharString & s ){
	uint32_t l = len + s.len;

	if( l < buf_size ){
		memcpy( buf + len, s.buf, s.len );
	}else{ // нужно увеличить размер буфера
		buf_size =
				l < 16 ? 32 :
				l < 128 ? 256 : l + 256;
		char * p = new char [ buf_size ];
		memcpy( p, buf, len );
		memcpy( p + len, s.buf, s.len );
		delete [] buf;
		buf = p;
	}

	len = l;
	buf[len] = 0;

	return *this;
}

TCharString & TCharString::append( const char * s ){
	uint32_t slen = strlen( s );
	uint32_t l = len + slen;

	if( l < buf_size ){
		memcpy( buf + len, s, slen );
	}else{ // нужно увеличить размер буфера
		buf_size =
				l < 16 ? 32 :
				l < 128 ? 256 : l + 256;
		char * p = new char [ buf_size ];
		memcpy( p, buf, len );
		memcpy( p + len, s, slen );
		delete [] buf;
		buf = p;
	}

	len = l;
	buf[len] = 0;

	return *this;
}

TCharString & TCharString::append( char c ){
	if( len + 1 == buf_size ){ // нужно увеличить размер буфера
		buf_size += 256;
		char * p = new char [ buf_size ];
		memcpy( p, buf, len );
		delete [] buf;
		buf = p;
	}

	buf[len++] = c;
	buf[len] = 0;

	return *this;
}

TCharString & TCharString::append( short i ){
	return append( (long)i );
}

TCharString & TCharString::append( int i ){
	return append( (long)i );
}

TCharString & TCharString::append( long i ){
	static char buff[64];
	sprintf( buff, "%ld", i );
	return append( buff );
}

TCharString & TCharString::append( float e ){
	return append(e);
}

TCharString & TCharString::append( double e ){
	static char buff[128];
	sprintf( buff, "%e", e );
	return append( buff );
}
