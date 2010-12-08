// $Log: Log.h,v $
// Revision 1.6  2002-01-05 11:01:58-06  wsenn
// fixed year of copyright
//
// Revision 1.5  2002-01-05 10:44:34-06  wsenn
// added copyright and permission notice
//
// Revision 1.4  2002-01-05 09:34:38-06  wsenn
// added file log appender and event log appender
//
// Revision 1.3  2001-11-14 21:41:02-06  wsenn
// lowercased method names
//
// Revision 1.2  2001-11-13 19:28:25-06  wsenn
// added CConsoleLogAppender and changed the vector to a vector of pointers
// for downstream polymorphism
//
// Revision 1.1  2001-11-11 14:16:52-06  wsenn
// first pass
//
// Revision 1.0  2001-11-11 13:05:49-06  wsenn
// Initial revision
//
//
// $Id: Log.h,v 1.6 2002-01-05 11:01:58-06 wsenn Exp wsenn $
//
// COPYRIGHT AND PERMISSION NOTICE
//
// Copyright (c) 2002 Will Senn
//
// All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, and/or sell copies of the Software, and to permit persons
// to whom the Software is furnished to do so, provided that the above
// copyright notice(s) and this permission notice appear in all copies of
// the Software and that both the above copyright notice(s) and this
// permission notice appear in supporting documentation.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
// OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
// INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
// FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
// NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Except as contained in this notice, the name of a copyright holder
// shall not be used in advertising or otherwise to promote the sale, use
// or other dealings in this Software without prior written authorization
// of the copyright holder.
//

#ifndef __LOG_H
#define __LOG_H

//#pragma warning( disable : 4786)

#include <string>	//string
#include <vector>	//vector
#include <utility>	//pair

#include <Utility.hpp>
#include <LogAppender.hpp>
#include <ConsoleLogAppender.hpp>
#include <EConsoleLogAppender.hpp>
#include <FileLogAppender.hpp>
#include <EventLogAppender.hpp>

using namespace std;

class CLog {
public:
	//default constructor
	CLog();	//filename will be the source
	
	//commonly used constructor
	CLog(string source);
	
	//destructor
	~CLog();

	//method to add appenders to the vector
	bool addAppender(CLogAppender * appender);

	//method for writing log messages to the appender logs
	bool writeLog(nsCLog::eSeverity severity, string message);

	//method for writing log messages to the appender logs
	bool writeLog(nsCLog::eSeverity severity, const char* message);

	bool dump(string message,const char* buf, unsigned long length);
	bool debug(string message);
	bool info(string message);
	bool warning(string message);
	bool error(string message);
	bool fatal(string message);

	bool dump( const char* message, const char* buf, unsigned long length );
	bool debug( const char* message );
	bool info( const char* message );
	bool warning( const char* message );
	bool error( const char* message );
	bool fatal( const char* message );
	
	string formatDump( string& message, const char* buf, unsigned long length );
	string formatDump( const char* message, const char* buf, unsigned long length );
private:
	vector<CLogAppender *> vecAppender;

	string mStrSource;
};

inline bool CLog::dump( string message, const char* buf, unsigned long length ){
	return writeLog(nsCLog::dump, formatDump(message,buf,length));
}

inline bool CLog::debug( string message ){
	return writeLog(nsCLog::debug, message);
}

inline bool CLog::info( string message ){
	return writeLog(nsCLog::info, message);
}

inline bool CLog::warning( string message ){
	return writeLog(nsCLog::warning, message);
}

inline bool CLog::error( string message ){
	return writeLog(nsCLog::error, message);
}

inline bool CLog::fatal( string message ){
	return writeLog(nsCLog::fatal, message);
}

inline bool CLog::dump( const char* message, const char* buf, unsigned long length  ){
	return writeLog(nsCLog::dump, formatDump(message,buf,length));
}

inline bool CLog::debug( const char* message ){
	return writeLog(nsCLog::debug, message);
}

inline bool CLog::info( const char* message ){
	return writeLog(nsCLog::info, message);
}

inline bool CLog::warning( const char* message ){
	return writeLog(nsCLog::warning, message);
}

inline bool CLog::error( const char* message ){
	return writeLog(nsCLog::error, message);
}

inline bool CLog::fatal( const char* message ){
	return writeLog(nsCLog::fatal, message);
}

#endif
