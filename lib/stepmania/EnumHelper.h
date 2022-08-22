#ifndef ENUM_HELPER_H
#define ENUM_HELPER_H

#include "RageUtil.h"
#include <memory>

#define LuaDeclareType(t)
#define XToLocalizedString(t)
#define LuaXType(t)

/** @brief A general foreach loop for enumerators, going up to a max value. */
#define FOREACH_ENUM_N( e, max, var )	for( e var=(e)0; var<max; enum_add<e>( var, +1 ) )
/** @brief A general foreach loop for enumerators. */
#define FOREACH_ENUM( e, var )	for( e var=(e)0; var<NUM_##e; enum_add<e>( var, +1 ) )

const RString &EnumToString( int iVal, int iMax, const char **szNameArray, unique_ptr<RString> *pNameCache ); // XToString helper

#define XToString(X) \
const RString& X##ToString(X x); \
COMPILE_ASSERT( NUM_##X == ARRAYLEN(X##Names) ); \
const RString& X##ToString( X x ) \
{	\
	static unique_ptr<RString> as_##X##Name[NUM_##X+2]; \
	return EnumToString( x, NUM_##X, X##Names, as_##X##Name ); \
} \
namespace StringConversion { template<> RString ToString<X>( const X &value ) { return X##ToString(value); } }

#define StringToX(X)	\
X StringTo##X(const RString&); \
X StringTo##X( const RString& s ) \
{	\
	for( unsigned i = 0; i < ARRAYLEN(X##Names); ++i )	\
		if( !s.CompareNoCase(X##Names[i]) )	\
			return (X)i;	\
	return X##_Invalid;	\
} \
namespace StringConversion \
{ \
	template<> bool FromString<X>( const RString &sValue, X &out ) \
	{ \
		out = StringTo##X(sValue); \
		return out != X##_Invalid; \
	} \
}

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2004-2006
 * @section LICENSE
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
