// $Log: Utility.h,v $
// Revision 1.3  2002-01-05 11:01:57-06  wsenn
// fixed year of copyright
//
// Revision 1.2  2002-01-05 10:44:32-06  wsenn
// added copyright and permission notice
//
// Revision 1.1  2001-11-13 19:28:25-06  wsenn
// added CConsoleLogAppender and changed the vector to a vector of pointers
// for downstream polymorphism
//
// Revision 1.0  2001-11-11 15:35:27-06  wsenn
// Initial revision
//
// Revision 1.0  2001-11-11 14:16:39-06  wsenn
// Initial revision
//
//
// $Id: Utility.h,v 1.3 2002-01-05 11:01:57-06 wsenn Exp wsenn $
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

#ifndef __UTILITY_H
#define __UTILITY_H

//#pragma warning( disable : 4786)

#include <string>

using namespace std;

namespace nsCLog {
	enum eSeverity {
		unknown =	-1,
		dump	=	0,
		debug	=	100,
		info	=	200,
		warning	=	300,
		error	=	400,
		fatal	=	500
	};
	
//	enum eBits {
//		KB = 1024,
//		MB = 1048576
//	};
	
	extern const string strDump;
	extern const string strDebug;
	extern const string strInfo;
	extern const string strWarn;
	extern const string strError;
	extern const string strFatal;
	extern const string strUnknown;

	string getThresholdAsString(nsCLog::eSeverity threshold);
	nsCLog::eSeverity getThresholdFromString(string threshold);
};
#endif
