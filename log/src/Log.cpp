// $Log: Log.cpp,v $
// Revision 1.5  2002-01-05 11:03:00-06  wsenn
// fixed copyright year
//
// Revision 1.4  2002-01-05 10:44:31-06  wsenn
// added copyright and permission notice
//
// Revision 1.3  2001-11-14 21:40:44-06  wsenn
// lowercased method names
//
// Revision 1.2  2001-11-13 19:28:25-06  wsenn
// added CConsoleLogAppender and changed the vector to a vector of pointers
// for downstream polymorphism
//
// Revision 1.1  2001-11-11 14:16:52-06  wsenn
// first pass
//
// Revision 1.0  2001-11-11 13:05:48-06  wsenn
// Initial revision
//
//
// $Id: Log.cpp,v 1.5 2002-01-05 11:03:00-06 wsenn Exp wsenn $
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

#include <string.h>
#include <string>
#include <vector>

using namespace std;

#include "Log.hpp"

//default constructor
CLog::CLog() {
	mStrSource = string(__FILE__);
}

//commonly used constructor
CLog::CLog(string source) {
	if(source.empty()) {
		mStrSource = string(__FILE__);
	}
	else {
		mStrSource = source;
	}
}

//destructor
CLog::~CLog() {
}

//method to add appenders to the vector
bool CLog::addAppender( CLogAppender* pAppender ) {
	pAppender->setSource(mStrSource);
	vecAppender.push_back(pAppender);

	return true;
}

bool CLog::writeLog( nsCLog::eSeverity severity, string message ) {
	if(severity == nsCLog::unknown) return false;
	
	vector<CLogAppender *>::iterator vec_iter;

	//for each appender in the vector, call it's writelog methoc
	for(vec_iter = vecAppender.begin(); vec_iter != vecAppender.end(); vec_iter++) {
		if(false == (*vec_iter)->writeLog(severity, message)) {
			return false;
		}
	}

	return true;
}

bool CLog::writeLog( nsCLog::eSeverity severity, const char* message ) {
	if(severity == nsCLog::unknown) return false;

	string msg = string(message);
	vector<CLogAppender *>::iterator vec_iter;
	//for each appender in the vector, call it's writelog methoc
	for(vec_iter = vecAppender.begin(); vec_iter != vecAppender.end(); vec_iter++) {
		if(false == (*vec_iter)->writeLog(severity, msg)) {
			return false;
		}
	}

	return true;
}

string CLog::formatDump( string& message, const char* buf, unsigned long length ){
    char output[256];
    char offsetFormatString[16];
    int offset_length;        //размер строки смещения
    string retStr(message);

    if( length <= 0xffff )
        offset_length = 4;
    else if( length <= 0xffffff )
        offset_length = 6;
    else
        offset_length = 8;

    sprintf( offsetFormatString, "%%0%xd", offset_length );
    retStr.append("\n");
	
    for( unsigned long i = 0; i < length; i += 16 ){

    	//печатаем смещение.
    	int pos = sprintf( output, offsetFormatString, i );

        output[pos++] = 0x20;

        //печатаем 16-тиричное представление
        for( unsigned long j = 0; j < 16; j++ ){
            if( j % 4 == 0 ) output[pos++] = 0x20;     //
            if( i + j < length ){            //
                pos += sprintf( output + pos, "%02x", (0xff & buf[i+j]) );
            }
            else{
                output[pos++] = 0x20;
                output[pos++] = 0x20;
            }
        }

		output[pos++] = 0x20;
        //печатаем символьное представление, управляющие символы замещаем '.'
        for( unsigned long j = 0; j < 16; j++ ){
            if( i+j < length && buf[i+j]!=0 && buf[i+j]!=0x7 && buf[i+j]!=0xa && buf[i+j]!=0xd && buf[i+j]!=0x9 && buf[i+j]!=0x8 && buf[i+j]!=0x1b )
                output[pos++] = buf[i+j];
            else if( i+j >= length ) break;
            else
                output[pos++] = '.';
        }
        output[pos++] = '\n';
        output[pos++] = '\0';

        retStr.append(output);//.append('\n');
    }
    return retStr;
}

string CLog::formatDump( const char* message, const char* buf, unsigned long length ){
	string msg = string(message);
	return formatDump(msg,buf,length);
}
