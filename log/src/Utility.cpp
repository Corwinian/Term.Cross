// $Log: Utility.cpp,v $
// Revision 1.4  2002-01-05 11:02:56-06  wsenn
// fixed copyright year
//
// Revision 1.3  2002-01-05 10:44:29-06  wsenn
// added copyright and permission notice
//
// Revision 1.2  2001-11-14 21:42:04-06  wsenn
// moved strings to the namespace
//
// Revision 1.1  2001-11-13 19:28:25-06  wsenn
// added CConsoleLogAppender and changed the vector to a vector of pointers
// for downstream polymorphism
//
// Revision 1.0  2001-11-12 21:09:37-06  wsenn
// Initial revision
//
//
// $Id: Utility.cpp,v 1.4 2002-01-05 11:02:56-06 wsenn Exp wsenn $
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

#include "Utility.hpp"
#ifdef DEBUG
#include <stdio.h>
#endif


namespace nsCLog {
	
	const string strDump = string("dump");
	const string strDebug = string("debug");
	const string strInfo = string("info");
	const string strWarn = string("warn");
	const string strError = string("error");
	const string strFatal = string("fatal");
	const string strUnknown = string("unknown");

	string getThresholdAsString(nsCLog::eSeverity threshold) {
		switch(threshold) {
			case nsCLog::dump:
				return nsCLog::strDump;
				break;
			case nsCLog::debug:
				return nsCLog::strDebug;
				break;
			case nsCLog::info:
				return nsCLog::strInfo;
				break;
			case nsCLog::warning:
				return nsCLog::strWarn;
				break;
			case nsCLog::error:
				return nsCLog::strError;
				break;
			case nsCLog::fatal:
				return nsCLog::strFatal;
				break;
			default:
				return nsCLog::strUnknown;
				break;
		}
	}

	nsCLog::eSeverity getThresholdFromString(string threshold) {
		if(nsCLog::strDump.compare(threshold)==0){
			return nsCLog::dump;
		}else if(nsCLog::strDebug.compare(threshold)==0){
			return nsCLog::debug;
		}else if(nsCLog::strInfo.compare(threshold)==0){
			return nsCLog::info;
		}else if(nsCLog::strWarn.compare(threshold)==0){
			return nsCLog::warning;
		}else if(nsCLog::strError.compare(threshold)==0){
			return nsCLog::error;
		}else if(nsCLog::strFatal.compare(threshold)==0){
			return nsCLog::fatal;
		}else return nsCLog::unknown;
	}

}
