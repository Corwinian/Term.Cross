// $Log: ConsoleLogAppender.h,v $
// Revision 1.3  2002-01-05 11:02:00-06  wsenn
// fixed year of copyright
//
// Revision 1.2  2002-01-05 10:44:28-06  wsenn
// added copyright and permission notice
//
// Revision 1.1  2001-11-14 21:40:20-06  wsenn
// changed order of format specifiers
//
// Revision 1.0  2001-11-13 19:27:32-06  wsenn
// Initial revision
//
//
// $Id: ConsoleLogAppender.h,v 1.3 2002-01-05 11:02:00-06 wsenn Exp wsenn $
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

#ifndef __CONSOLELOGAPPENDER_H
#define __CONSOLELOGAPPENDER_H

#include <string>

#include <Utility.hpp>
#include <LogAppender.hpp>

using namespace std;

class CConsoleLogAppender : public CLogAppender {
public:
	//default constructor
	CConsoleLogAppender();
	
	//commonly used constructor
	CConsoleLogAppender(nsCLog::eSeverity threshold, string strFormat = string("%d %t | %s | %e | %m"));

	//destructor
	virtual ~CConsoleLogAppender();

	//inherited and overriden
	virtual bool writeLog(nsCLog::eSeverity severity, string message);
};

#endif
