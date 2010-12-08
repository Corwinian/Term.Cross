// $Log: FileLogAppender.cpp,v $
// Revision 1.3  2002-01-05 11:02:58-06  wsenn
// fixed copyright year
//
// Revision 1.2  2002-01-05 10:44:32-06  wsenn
// added copyright and permission notice
//
// Revision 1.1  2002-01-05 09:36:02-06  wsenn
// modified the writelog method and added a comment
//
// Revision 1.0  2002-01-04 19:52:49-06  wsenn
// renamed from file.cpp
//
// Revision 1.0  2002-01-04 19:44:18-06  wsenn
// added to logdriver project
//
// Revision 1.1  2001-11-15 07:59:34-06  wsenn
// fleshed out the methods
//
// Revision 1.0  2001-11-15 07:19:28-06  wsenn
// initial checkin
//
//
// $Id: FileLogAppender.cpp,v 1.3 2002-01-05 11:02:58-06 wsenn Exp wsenn $
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

#include <fstream>
#include <string>

#include "FileLogAppender.hpp"

using namespace std;

//default constructor
CFileLogAppender::CFileLogAppender() : CLogAppender(){
	pStream = NULL;
	bIsOpen = false;
}

//commonly used constructor
CFileLogAppender::CFileLogAppender(nsCLog::eSeverity threshold,	string strFileName, bool append, string strFormat) throw ( string )
	: CLogAppender(threshold, strFormat) 
{
	pStream = NULL;
	pStream = new ofstream(strFileName.c_str(), (append?(ios::app):(ios::out))); //|(ios::in)
	//pStream = new ofstream(strFileName.c_str(), ios::app|ios::out);
	if(!pStream) {
		//exit(1);
		throw strFileName+" open error";
	}
	pStream->setf(ios::unitbuf);	//turn off buffering
	//pStream->seekp(ios::end);
	bIsOpen = true;
}

//destructor
CFileLogAppender::~CFileLogAppender() {
	if(NULL != pStream) {
		pStream->close();
		delete pStream;
		pStream = NULL;
	}
	bIsOpen = false;
}

//open method
bool CFileLogAppender::openFile(string strFileName, bool append)  throw ( string ){
	bool bResult = false;
	
	if(false == bIsOpen) {
		if(NULL == pStream) {
			//pStream = new ofstream(strFileName.c_str(), append?(ios::app):(ios::out));
			pStream = new ofstream(strFileName.c_str(), (append?(ios::app):(ios::out))); //|(ios::in)
			//pStream = new ofstream(strFileName.c_str(), ios::out);
			if(!pStream) {
				throw strFileName+" open error";
			}
			pStream->setf(ios::unitbuf);	//turn off buffering
			//pStream->seekp(ios::end);
			bIsOpen = true;
			bResult = true;
		}
		else {
			pStream->open(strFileName.c_str(), (append?(ios::app):(ios::out))|(ios::in));
			//pStream->open(strFileName.c_str(), ios::out);
			if(!pStream) {
				throw strFileName+" open error";
			}
			pStream->setf(ios::unitbuf);	//turn off buffering
			//pStream->seekp(ios::end);
			bIsOpen = true;
			bResult = true;
		}
	}
	else {
		bResult = false;
	}
	
	return bResult;
}

//close method
bool CFileLogAppender::closeFile() {
	bool bResult = false;
	
	if(true == bIsOpen) {
		if(NULL == pStream) {
			bIsOpen = false;
			bResult = false;
		}
		else {
			pStream->close();
			bIsOpen = false;
			bResult = true;
		}
	}

	return bResult;
}

//writeLine method (don't need no stinkin' read method!)
bool CFileLogAppender::writeLog(nsCLog::eSeverity severity,	string message) {
	bool bResult = false;
	
	if(severity < mThreshold) {
		return true;
	}
	
	string outstring;
	if(false == CLogAppender::formatMessage(severity, message, outstring)) {
		return false;
	}
	
	*pStream << outstring << "\n";
	if(!pStream) {
		bResult = false;
	}
	else {
		bResult = true;
	}

	return bResult;
}
