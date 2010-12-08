// $Log: LogDriver.cpp,v $
// Revision 1.6  2002-01-05 11:02:57-06  wsenn
// fixed copyright year
//
// Revision 1.5  2002-01-05 10:44:30-06  wsenn
// added copyright and permission notice
//
// Revision 1.4  2002-01-05 09:35:27-06  wsenn
// added file log appender and event log appender
//
// Revision 1.3  2001-11-14 21:38:03-06  wsenn
// shortened up a little
//
// Revision 1.2  2001-11-13 19:28:25-06  wsenn
// added CConsoleLogAppender and changed the vector to a vector of pointers
// for downstream polymorphism
//
// Revision 1.1  2001-11-11 14:16:52-06  wsenn
// first pass
//
//
// $Id: LogDriver.cpp,v 1.6 2002-01-05 11:02:57-06 wsenn Exp wsenn $
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
#include <cstdlib>
#include "Log.hpp"


int main(int argc, char* argv[])
{

	CLog myLog("LogDriver");
	CConsoleLogAppender * pConsoleLogAppender = new CConsoleLogAppender(nsCLog::info);
	if(false == myLog.addAppender(pConsoleLogAppender)) {
		cerr << "error adding appender" << endl;
		exit(1);
	}

	CFileLogAppender * pFileLogAppender = new CFileLogAppender(nsCLog::warning, "test.dat");
	if(false == myLog.addAppender(pFileLogAppender)) {
		cerr << "error adding appender" << endl;
		exit(1);
	}

	CEventLogAppender * pEventLogAppender = new CEventLogAppender(nsCLog::debug);
	if(false == myLog.addAppender(pEventLogAppender)) {
		cerr << "error adding appender" << endl;
		exit(1);
	}

	myLog.writeLog(nsCLog::info, "This is my test log message");

	return 0;
}
