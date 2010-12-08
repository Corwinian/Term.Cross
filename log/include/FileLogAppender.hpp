// $Log: FileLogAppender.h,v $
// Revision 1.3  2002-01-05 11:01:59-06  wsenn
// fixed year of copyright
//
// Revision 1.2  2002-01-05 10:44:28-06  wsenn
// added copyright and permission notice
//
// Revision 1.1  2002-01-05 09:34:13-06  wsenn
// edited a comment
//
// Revision 1.0  2002-01-04 19:52:50-06  wsenn
// renamed from file.h and edited to suit
//
// Revision 1.0  2002-01-04 19:44:16-06  wsenn
// added to logdriver project
//
// Revision 1.1  2001-11-15 08:00:26-06  wsenn
// wrote the class itself
//
// Revision 1.0  2001-11-15 07:19:34-06  wsenn
// initial checkin
//
//
// $Id: FileLogAppender.h,v 1.3 2002-01-05 11:01:59-06 wsenn Exp wsenn $
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

#ifndef __FILELOGAPPENDER_H
#define __FILELOGAPPENDER_H

#include <string>
#include <fstream>

#include <Utility.hpp>
#include <LogAppender.hpp>

using namespace std;

class CFileLogAppender : public CLogAppender {
public:
	//default constructor
	CFileLogAppender();

	//commonly used constructor
	CFileLogAppender(nsCLog::eSeverity threshold, string strFileName, bool append = true,
		string strFormat = string("%d %t | %s | %e | %m"))throw ( string );

	//destructor
	~CFileLogAppender();

	//open method
	bool openFile(string strFileName, bool append = true) throw ( string );

	//close method
	bool closeFile();

	//writeLine method (don't need no stinkin' read method!)
	virtual bool writeLog(nsCLog::eSeverity, string message);

private:
	string mStrFileName;
	ofstream * pStream;
	bool bIsOpen;
};

#endif
