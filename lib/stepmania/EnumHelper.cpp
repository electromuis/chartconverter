#include "global.h"
#include "EnumHelper.h"
#include "RageUtil.h"
#include "RageLog.h"

// szNameArray is of size iMax; pNameCache is of size iMax+2.
const RString &EnumToString( int iVal, int iMax, const char **szNameArray, unique_ptr<RString> *pNameCache )
{
	if( unlikely(pNameCache[0].get() == nullptr) )
	{
		for( int i = 0; i < iMax; ++i )
		{
			unique_ptr<RString> ap( new RString( szNameArray[i] ) );
			pNameCache[i] = move(ap);
		}

		unique_ptr<RString> ap( new RString );
		pNameCache[iMax+1] = move(ap);
	}

	// iMax+1 is "Invalid".  iMax+0 is the NUM_ size value, which can not be converted
	// to a string.
	// Maybe we should assert on _Invalid?  It seems better to make 
	// the caller check that they're supplying a valid enum value instead of 
	// returning an inconspicuous garbage value (empty string). -Chris
	if (iVal < 0)
		FAIL_M(ssprintf("Value %i cannot be negative for enums! Enum hint: %s", iVal, szNameArray[0]));
	if (iVal == iMax)
		FAIL_M(ssprintf("Value %i cannot be a string with value %i! Enum hint: %s", iVal, iMax, szNameArray[0]));
	if (iVal > iMax+1)
		FAIL_M(ssprintf("Value %i is past the invalid value %i! Enum hint: %s", iVal, iMax, szNameArray[0]));
	return *pNameCache[iVal];
}
/*
 * (c) 2006 Glenn Maynard
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
