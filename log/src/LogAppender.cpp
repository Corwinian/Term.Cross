// $Log: LogAppender.cpp,v $
// Revision 1.4  2002-01-05 11:02:58-06  wsenn
// fixed copyright year
//
// Revision 1.3  2002-01-05 10:44:30-06  wsenn
// added copyright and permission notice
//
// Revision 1.2  2001-11-14 21:39:25-06  wsenn
// added formatMessage method
// and setSource/getSource methods
//
// Revision 1.1  2001-11-13 19:28:26-06  wsenn
// added CConsoleLogAppender and changed the vector to a vector of pointers
// for downstream polymorphism
//
// Revision 1.0  2001-11-11 14:16:39-06  wsenn
// Initial revision
//
//
// $Id: LogAppender.cpp,v 1.4 2002-01-05 11:02:58-06 wsenn Exp wsenn $
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

#include <string>
#include <ctime>
#include <cstdio>

#include "LogAppender.hpp"

using namespace std;

CLogAppender::CLogAppender() {
	mThreshold = nsCLog::info;
	mStrSource = "unknown";
	mStrFormat = "%d_%t| %s | %e | %m";	//"mmddyyyy_hhmmss.mss: severity | source | message"
}

//commonly used constructor
CLogAppender::CLogAppender(nsCLog::eSeverity threshold, string strFormat) {
	mThreshold = threshold;
	mStrSource = "unknown";
	mStrFormat = strFormat;
}

//destructor defined in header
CLogAppender::~CLogAppender() {
}

bool CLogAppender::formatMessage(nsCLog::eSeverity severity, string& message, string& outstring) {
	//string outstring = mStrFormat;

	string::size_type position;
//	nDatePosition,
//		nTimePosition,
//		nSeverityPosition,
//		nSourcePosition,
//		nMessagePosition;
	
    struct tm *today;
    time_t ltime;

	char datestr[80];
	char timestr[80];

	time( &ltime );
	today = localtime( &ltime );

	//outstring = mStrFormat;
	outstring.assign(mStrFormat);
	if(string::npos != (position = outstring.find("%d"))) {
		//gotta date tag at nDatePosition
		sprintf(datestr, "%04d-%02d-%02d",
			today->tm_year + 1900,
			today->tm_mon + 1,
			today->tm_mday);
		outstring.replace(position, 2, datestr);
	}
	if(string::npos != (position = outstring.find("%t"))) {
		//gotta time tag at nDatePosition
		sprintf(timestr, "%02d:%02d:%02d",
			today->tm_hour,
			today->tm_min,
			today->tm_sec);
		outstring.replace(position, 2, timestr);
	}
	if(string::npos != (position = outstring.find("%e"))) {
		//gotta severity tag at nDatePosition
		string strSeverity;
		switch(severity) {
			case nsCLog::dump:
				strSeverity = "DUMP   ";
				break;
			case nsCLog::debug:
				strSeverity = "DEBUG  ";
				break;
			case nsCLog::info:
				strSeverity = "INFO   ";
				break;
			case nsCLog::warning:
				strSeverity = "WARN   ";
				break;
			case nsCLog::error:
				strSeverity = "ERROR  ";
				break;
			case nsCLog::fatal:
				strSeverity = "FATAL  ";
				break;
			default:
				strSeverity = "UNKNOWN";
				break;
		}
		outstring.replace(position, 2, strSeverity);
	}
	if(string::npos != (position = outstring.find("%s"))) {
		//gotta source tag at nDatePosition
		outstring.replace(position, 2, mStrSource);
	}
	if(string::npos != (position = outstring.find("%m"))) {
		//gotta message tag at nDatePosition
		outstring.replace(position, 2, message);
	}

	return true;
}

//accessors
string CLogAppender::getSource() {
	return mStrSource;
}

//mutators
bool CLogAppender::setSource(string & source) {
	mStrSource = source;
	return true;
}
