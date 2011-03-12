#ifndef _STRFORMAT_
#define _STRFORMAT_


#include <string>
#include <ostream>
#include <sstream>



std::string strformat ( std::string& format , ... );
std::string strformat ( char const * format , ... );

std::ostream& streamformat ( std::ostream& out, std::string& format , ... );
std::ostream& streamformat ( std::ostream& out, char const * format , ... );

std::string& strformat2 ( std::string& dst, std::string& format , ... );
std::string& strformat2 ( std::string& dst, char const * format , ... );

int strscanf ( std::string& src, std::string& format , ... );
int strscanf ( std::string& src, char const * format , ... );

#endif
