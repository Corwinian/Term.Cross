/*-------------------------------------------------------------------------
    c_except.hpp
-------------------------------------------------------------------------*/
#ifndef _C_EXCEPT_HPP_
#define _C_EXCEPT_HPP_

#include <c_lib/c_types.hpp>
#include <string>
#include <exception>

class TException : public std::exception
{
public:
	
	TException( int n, const std::string s ):Id( n ), Text(s){}
	TException( const TException & e)	:Id( e.Id ), Text(e.Text){}

    ~TException() throw() { Text.std::string::~string();}
	TException & operator = ( const TException &);
	
	int id() const {	return Id;}
	const std::string  text() const {	return Text;}
	const char* what() const throw() {return Text.c_str();}
	
private:
	int Id;
	std::string Text;
};

class TAccessExc : public TException {
public:
	TAccessExc( int n, const std::string s ):TException( n, s ) {}
};

class TParamExc : public TException {
public:
	TParamExc( int n, const std::string s ):TException( n, s ) {}
};

class TRequestExc : public TException {
public:
	TRequestExc( int n, const std::string s ):TException( n, s ) {}
};


#endif
