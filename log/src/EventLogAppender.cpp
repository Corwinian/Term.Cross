//
// $Log: EventLogAppender.cpp,v $
// Revision 1.2  2002-01-05 11:03:00-06  wsenn
// fixed copyright year
//
// Revision 1.1  2002-01-05 10:44:35-06  wsenn
// added copyright and permission notice
//
// Revision 1.0  2002-01-05 09:32:38-06  wsenn
// Initial revision
//
//
// $Id: EventLogAppender.cpp,v 1.2 2002-01-05 11:03:00-06 wsenn Exp wsenn $
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

#include <iostream>
#include <string>
#include "EventLogAppender.hpp"

using namespace std;

#ifdef unix
/*
7 LOG_DEBUG     The message is only for debugging purposes.
6 LOG_INFO      The message is purely informational.
4 LOG_WARNING   The message is a warning.
3 LOG_ERR       The message describes an error.
2 LOG_CRIT      The message states a critical condition.

debug	=	100,
info	=	200,
warning	=	300,
error	=	400,
fatal	=	500
*/
int getIntPriority(nsCLog::eSeverity pri){
	switch(pri) {
		case nsCLog::debug:
			return LOG_DEBUG;
			break;
		case nsCLog::info:
			return LOG_INFO;
			break;
		case nsCLog::warning:
			return LOG_WARNING;
			break;
		case nsCLog::error:
			return LOG_ERR;
			break;
		case nsCLog::fatal:
			return LOG_CRIT;
			break;
		default:
			return LOG_DEBUG;
			break;
	}
}
#endif

//default constructor
CEventLogAppender::CEventLogAppender() : CLogAppender(){
	bIsOpen = false;
}

//commonly used constructor
CEventLogAppender::CEventLogAppender(nsCLog::eSeverity threshold,
	string strFormat) : CLogAppender(threshold, strFormat) {
	bIsOpen = false;
}

//destructor
CEventLogAppender::~CEventLogAppender() {
	if(true == bIsOpen) {
		deRegisterEventLog();
	}
}

bool CEventLogAppender::registerEventLog(string source) {
	mStrSource = source;
#if defined WIN32
	if(NULL == (hEventLogHandle = RegisterEventSource(NULL, mStrSource.c_str()))) {
		return false;
	}
#elif defined unix
	if(!bIsOpen){
		setlogmask(LOG_UPTO(getIntPriority(mThreshold)));
		openlog(mStrSource.c_str(),LOG_NDELAY,LOG_LOCAL1);
	}
#endif
	bIsOpen = true;

	return true;
}

bool CEventLogAppender::deRegisterEventLog() {
	if(bIsOpen == false
#ifdef WIN32
	 || NULL == hEventLogHandle
#endif
	){
		return false;
	}

#ifdef WIN32
	DeregisterEventSource(hEventLogHandle);
	hEventLogHandle = NULL;
#elif defined unix
	closelog();
#endif
	bIsOpen = false;

	return true;
}

bool CEventLogAppender::writeLog(nsCLog::eSeverity severity, string message) {
	if(severity < mThreshold) {
		return true;
	}

	if(false == bIsOpen) {
		if(!registerEventLog(mStrSource)) {
			return false;
		}
	}
	string outstring;

	if(false == CLogAppender::formatMessage(severity, message, outstring)) {
		return false;
	}

	
#if defined WIN32
	const char * pStr;
	char * pOutString;
	
	pOutString = new char[outstring.length() + 1];
	
	strcpy(pOutString, outstring.c_str());
	
	pStr = pOutString;

	//TODO: с этой фй надо разобраться
	if(! ReportEvent(hEventLogHandle,
			EVENTLOG_ERROR_TYPE,
			1500,
			0,
			NULL,
			1,
			0,
			(LPCTSTR *) &pStr,
			NULL)) {
		return false;
	}
#elif defined unix
	syslog(getIntPriority(severity),"%s",outstring.c_str());
#endif
	return true;
}
