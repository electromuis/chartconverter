#include "global.h"
#include "RageLog.h"
#include "RageUtil.h"
#include <iostream>

#include <ctime>
#if defined(_WINDOWS)
#include <windows.h>
#endif
#include <map>

RageLog* LOG;		// global and accessible from anywhere in the program


RageLog::RageLog()
{
		
}

RageLog::~RageLog()
{
	
}

void RageLog::Trace( const char *fmt, ... )
{
	va_list	va;
	va_start( va, fmt );
	RString sBuff = vssprintf( fmt, va );
	va_end( va );

	cout << sBuff << std::endl;
}

/* Use this for more important information; it'll always be included
 * in crash dumps. */
void RageLog::Info( const char *fmt, ... )
{
	va_list	va;
	va_start( va, fmt );
	RString sBuff = vssprintf( fmt, va );
	va_end( va );

	cout << sBuff << std::endl;
}

void RageLog::Warn( const char *fmt, ... )
{
	va_list	va;
	va_start( va, fmt );
	RString sBuff = vssprintf( fmt, va );
	va_end( va );

	cout << sBuff << std::endl;
}

void RageLog::Time(const char *fmt, ...)
{
	va_list	va;
	va_start(va, fmt);
	RString sBuff = vssprintf(fmt, va);
	va_end(va);

	cout << sBuff << std::endl;
}

void RageLog::UserLog( const RString &sType, const RString &sElement, const char *fmt, ... )
{
	va_list va;
	va_start( va, fmt );
	RString sBuf = vssprintf( fmt, va );
	va_end( va );
	
	if( !sType.empty() )
		sBuf = ssprintf( "%s \"%s\" %s", sType.c_str(), sElement.c_str(), sBuf.c_str() );
	
	cout << sBuf << std::endl;
}

#define NEWLINE "\n"

void ShowWarningOrTrace( const char *file, int line, const char *message, bool bWarning )
{
	/* Ignore everything up to and including the first "src/". */
	const char *temp = strstr( file, "src/" );
	if( temp )
		file = temp + 4;

	void (RageLog::*method)(const char *fmt, ...) = bWarning ? &RageLog::Warn : &RageLog::Trace;

	if( LOG )
		(LOG->*method)( "%s:%i: %s", file, line, message );
	else
		fprintf( stderr, "%s:%i: %s\n", file, line, message );
}


/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
