/*-------------------------------------------------------------------------
	c_except.cpp
-------------------------------------------------------------------------*/
#include <string.h>

#include <tc_config.h>
#include "c_lib/c_except.hpp"

TException & TException::operator = ( const TException &e){

	Text = e.Text;
	Id = e.Id;
	return *this;
}