/*-----------------------------------------------------------------------------
    c_str.hpp
    Содержит класс TCharString.
-----------------------------------------------------------------------------*/
#ifndef _C_STR_HPP_
#define _C_STR_HPP_


#include <string.h>
#include <c_lib/c_types.hpp>

class TCharString {
public:
	TCharString();
	TCharString( const TCharString & );

	/*
	    Строка из одного символа.
	*/
	TCharString( char );

	TCharString( const char * );

	~TCharString();

	operator char * ();
	operator const char * () const;

	TCharString & clear();

	TCharString & operator = ( const TCharString & );

	TCharString & operator = ( const char * );

	TCharString & operator += ( const TCharString & );

	TCharString & operator += ( const char * );


	TCharString & assign( const TCharString & );

	TCharString & assign( const char * );

	TCharString & append( const TCharString & );

	TCharString & append( const char * );

	TCharString & append( char );

	TCharString & append( int );

	TCharString & append( short );

	TCharString & append( long );

	TCharString & append( float );

	TCharString & append( double );

	TCharString & operator += ( short );

	TCharString & operator += ( int );

	TCharString & operator += ( long );

	TCharString & operator += ( float );

	TCharString & operator += ( double );

	/*
	    Добавление к строке одного символа - оператор введен в класс из соображений эффективности.
	*/
	TCharString & operator += ( char );

	uint32_t length() const;

private:
	char *  buf;
	uint32_t   buf_size;   // размер буфера
	uint32_t   len;        // длина строки: тождественно strlen( buf )
};


inline TCharString::TCharString() :
		buf( new char [128] ),
		buf_size( 128 ),
len( 0 ) {
	buf[0] = 0;
}

inline TCharString::TCharString( const TCharString & s ) :
		buf( new char [s.buf_size] ),
		buf_size( s.buf_size ),
len( s.len ) {
	memcpy( buf, s.buf, len );
	buf[len] = 0;
}

inline TCharString::TCharString( char c ) :
		buf( new char[16] ),
buf_size( 16 ) {
	if( (buf[0] = c) ) {
		len = 1;
		buf[1] = 0;
	} else {
		len = 0;
	}
}

inline TCharString::~TCharString() {
	delete [] buf;
}

inline uint32_t TCharString::length() const {
	return len;
}

inline TCharString::operator char * () {
	return buf;
}

inline TCharString::operator const char * () const {
	return buf;
}

inline TCharString & TCharString::clear() {
	len = 0;
	buf[0] = 0;

	return *this;
}

inline TCharString & TCharString::operator = ( const TCharString & s ) {
	return assign(s);
}

inline TCharString & TCharString::operator = ( const char * s ) {
	return assign(s);
}

inline TCharString & TCharString::operator += ( const TCharString & s ) {
	return append(s);
}


inline TCharString & TCharString::operator += ( const char * s ) {
	return append(s);
}

inline TCharString & TCharString::operator += ( char c ) {
	return append(c);
}

inline TCharString & TCharString::operator += ( short i ) {
	return append(i);
}
inline TCharString & TCharString::operator += ( int i ) {
	return append(i);
}
inline TCharString & TCharString::operator += ( long i ) {
	return append(i);
}
inline TCharString & TCharString::operator += ( float d ) {
	return append(d);
}
inline TCharString & TCharString::operator += ( double d ) {
	return append(d);
}

#endif
