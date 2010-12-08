/*-------------------------------------------------------------------------
    c_except.hpp
-------------------------------------------------------------------------*/
#ifndef _C_EXCEPT_HPP_
#define _C_EXCEPT_HPP_

#include <c_lib/c_types.hpp>

class TException {
public:
	TException( int n, const char * s );
	TException( const TException & );
	virtual ~TException();

	TException & operator = ( const TException & );

	int id() const {
		return Id;
	}
	const char * text() const {
		return Text;
	}

private:
	int Id;
	char * Text;
};

class TAccessExc : public TException {
public:
	TAccessExc( int n, const char * s );

};

class TParamExc : public TException {
public:
	TParamExc( int n, const char * s );
};

class TRequestExc : public TException {
public:
	TRequestExc( int n, const char * s );
};

#endif
