#include "global.h"
#include "RageUtil.h"
#include "RageLog.h"

unsigned char g_UpperCase[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF,
};

unsigned char g_LowerCase[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF,
};

RString Basename( const RString &sDir )
{
	size_t iEnd = sDir.find_last_not_of( "/\\" );
	if( iEnd == sDir.npos )
		return RString();

	size_t iStart = sDir.find_last_of( "/\\", iEnd );
	if( iStart == sDir.npos )
		iStart = 0;
	else
		++iStart;

	return sDir.substr( iStart, iEnd-iStart+1 );
}

/* Return all but the last named component of dir:
 *
 * a/b/c -> a/b/
 * a/b/c/ -> a/b/
 * c/ -> ./
 * /foo -> /
 * / -> /
 */
RString Dirname( const RString &dir )
{
	// Special case: "/" -> "/".
	if( dir.size() == 1 && dir[0] == '/' )
		return "/";

	int pos = dir.size()-1;
	// Skip trailing slashes.
	while( pos >= 0 && dir[pos] == '/' )
		--pos;

	// Skip the last component.
	while( pos >= 0 && dir[pos] != '/' )
		--pos;

	if( pos < 0 )
		return "./";

	return dir.substr(0, pos+1);
}

void FixSlashesInPlace( RString &sPath )
{
	for( unsigned i = 0; i < sPath.size(); ++i )
		if( sPath[i] == '\\' )
			sPath[i] = '/';
}

bool BeginsWith( const RString &sTestThis, const RString &sBeginning )
{
	ASSERT( !sBeginning.empty() );
	return sTestThis.compare( 0, sBeginning.length(), sBeginning ) == 0;
}

bool EndsWith( const RString &sTestThis, const RString &sEnding )
{
	ASSERT( !sEnding.empty() );
	if( sTestThis.size() < sEnding.size() )
		return false;
	return sTestThis.compare( sTestThis.length()-sEnding.length(), sEnding.length(), sEnding ) == 0;
}

void CollapsePath( RString &sPath, bool bRemoveLeadingDot )
{
	RString sOut;
	sOut.reserve( sPath.size() );

	size_t iPos = 0;
	size_t iNext;
	for( ; iPos < sPath.size(); iPos = iNext )
	{
		// Find the next slash.
		iNext = sPath.find( '/', iPos );
		if( iNext == RString::npos )
			iNext = sPath.size();
		else
			++iNext;

		/* Strip extra slashes, but don't remove slashes from the beginning of the string. */
		if( iNext - iPos == 1 && sPath[iPos] == '/' )
		{
			if( !sOut.empty() )
				continue;
		}

		// If this is a dot, skip it.
		if( iNext - iPos == 2 && sPath[iPos] == '.' && sPath[iPos+1] == '/' )
		{
			if( bRemoveLeadingDot || !sOut.empty() )
				continue;
		}

		// If this is two dots,
		if( iNext - iPos == 3 && sPath[iPos] == '.' && sPath[iPos+1] == '.' && sPath[iPos+2] == '/' )
		{
			/* If this is the first path element (nothing to delete),
			 * or all we have is a slash, leave it. */
			if( sOut.empty() || (sOut.size() == 1 && sOut[0] == '/') )
			{
				sOut.append( sPath, iPos, iNext-iPos );
				continue;
			}

			// Search backwards for the previous path element.
			size_t iPrev = sOut.rfind( '/', sOut.size()-2 );
			if( iPrev == RString::npos )
				iPrev = 0;
			else
				++iPrev;

			// If the previous element is also .., leave it.
			bool bLastIsTwoDots = (sOut.size() - iPrev == 3 && sOut[iPrev] == '.' && sOut[iPrev+1] == '.' );
			if( bLastIsTwoDots )
			{
				sOut.append( sPath, iPos, iNext-iPos );
				continue;
			}

			sOut.erase( iPrev );
			continue;
		}

		sOut.append( sPath, iPos, iNext-iPos );
	}

	sOut.swap( sPath );
}

static int UnicodeDoUpper( char *p, size_t iLen, const unsigned char pMapping[256] )
{
	// Note: this has problems with certain accented characters. -aj
	wchar_t wc = L'\0';
	unsigned iStart = 0;
	if( !utf8_to_wchar(p, iLen, iStart, wc) )
		return 1;

	wchar_t iUpper = wc;
	if( wc < 256 )
		iUpper = pMapping[wc];
	if( iUpper != wc )
	{
		RString sOut;
		wchar_to_utf8( iUpper, sOut );
		if( sOut.size() == iStart )
			memcpy( p, sOut.data(), sOut.size() );
		else
			WARN( ssprintf("UnicodeDoUpper: invalid character at \"%s\"", RString(p,iLen).c_str()) );
	}

	return iStart;
}

float StringToFloat( const RString &sString )
{
	float fOut = std::strtof(sString, nullptr);
	if (!isfinite(fOut))
	{
		fOut = 0.0f;
	}
	return fOut;
}

bool StringToFloat( const RString &sString, float &fOut )
{
	char *endPtr = nullptr;

	fOut = std::strtof(sString, &endPtr);
	return sString.size() && *endPtr == '\0' && isfinite(fOut);
}

void Trim( RString &sStr, const char *s )
{
	RString::size_type b = 0, e = sStr.size();
	while( b < e && strchr(s, sStr[b]) )
		++b;
	while( b < e && strchr(s, sStr[e-1]) )
		--e;
	sStr.assign( sStr.substr(b, e-b) );
}

int StringToInt( const std::string& str, std::size_t* pos, int base, int exceptVal )
{
  try
  {
    return std::stoi(str, pos, base);
  }
  catch (const std::invalid_argument & e) {
    LOG->Warn( "stoi(%s): %s", str.c_str(), e.what() );
  }
  catch (const std::out_of_range & e) {
    LOG->Warn( "stoi(%s): %s", str.c_str(), e.what() );
  }
  return exceptVal;
}

RString join( const RString &sDeliminator, const vector<RString> &sSource)
{
	if( sSource.empty() )
		return RString();

	RString sTmp;
	size_t final_size= 0;
	size_t delim_size= sDeliminator.size();
	for(size_t n= 0; n < sSource.size()-1; ++n)
	{
		final_size+= sSource[n].size() + delim_size;
	}
	final_size+= sSource.back().size();
	sTmp.reserve(final_size);

	for( unsigned iNum = 0; iNum < sSource.size()-1; iNum++ )
	{
		sTmp += sSource[iNum];
		sTmp += sDeliminator;
	}
	sTmp += sSource.back();
	return sTmp;
}

RString join( const RString &sDelimitor, vector<RString>::const_iterator begin, vector<RString>::const_iterator end )
{
	if( begin == end )
		return RString();

	RString sRet;
	size_t final_size= 0;
	size_t delim_size= sDelimitor.size();
	for(vector<RString>::const_iterator curr= begin; curr != end; ++curr)
	{
		final_size+= curr->size();
		if(curr != end)
		{
			final_size+= delim_size;
		}
	}
	sRet.reserve(final_size);

	while( begin != end )
	{
		sRet += *begin;
		++begin;
		if( begin != end )
			sRet += sDelimitor;
	}

	return sRet;
}

RString ssprintf( const char *fmt, ...)
{
	va_list	va;
	va_start(va, fmt);
	return vssprintf(fmt, va);
}

#define FMT_BLOCK_SIZE		2048 // # of bytes to increment per try

RString vssprintf( const char *szFormat, va_list argList )
{
	RString sStr;

#if defined(WIN32)
	char *pBuf = nullptr;
	int iChars = 1;
	int iUsed = 0;
	int iTry = 0;

	do
	{
		// Grow more than linearly (e.g. 512, 1536, 3072, etc)
		iChars += iTry * FMT_BLOCK_SIZE;
		pBuf = (char*) _alloca( sizeof(char)*iChars );
		iUsed = vsnprintf( pBuf, iChars-1, szFormat, argList );
		++iTry;
	} while( iUsed < 0 );

	// assign whatever we managed to format
	sStr.assign( pBuf, iUsed );
#else
	static bool bExactSizeSupported;
	static bool bInitialized = false;
	if( !bInitialized )
	{
		/* Some systems return the actual size required when snprintf
		 * doesn't have enough space.  This lets us avoid wasting time
		 * iterating, and wasting memory. */
		char ignore;
		bExactSizeSupported = ( snprintf( &ignore, 0, "Hello World" ) == 11 );
		bInitialized = true;
	}

	if( bExactSizeSupported )
	{
		va_list tmp;
		va_copy( tmp, argList );
		char ignore;
		int iNeeded = vsnprintf( &ignore, 0, szFormat, tmp );
		va_end(tmp);

		char *buf = new char[iNeeded + 1];
		std::fill(buf, buf + iNeeded + 1, '\0');
		vsnprintf( buf, iNeeded+1, szFormat, argList );
		RString ret(buf);
		delete [] buf;
		return ret;
	}

	int iChars = FMT_BLOCK_SIZE;
	int iTry = 1;
	for (;;)
	{
		// Grow more than linearly (e.g. 512, 1536, 3072, etc)
		char *buf = new char[iChars];
		std::fill(buf, buf + iChars, '\0');
		int used = vsnprintf( buf, iChars - 1, szFormat, argList );
		if ( used == -1 )
		{
			iChars += ( ++iTry * FMT_BLOCK_SIZE );
		}
		else
		{
			/* OK */
			sStr.assign(buf, used);
		}

		delete [] buf;
		if (used != -1)
		{
			break;
		}
	}
#endif
	return sStr;
}

void MakeUpper( char *p, size_t iLen )
{
	char *pStart = p;
	char *pEnd = p + iLen;
	while( p < pEnd )
	{
		// Fast path:
		if( likely( !(*p & 0x80) ) )
		{
			if( unlikely(*p >= 'a' && *p <= 'z') )
				*p += 'A' - 'a';
			++p;
			continue;
		}

		int iRemaining = iLen - (p-pStart);
		p += UnicodeDoUpper( p, iRemaining, g_UpperCase );
	}
}

void MakeLower( char *p, size_t iLen )
{
	char *pStart = p;
	char *pEnd = p + iLen;
	while( p < pEnd )
	{
		// Fast path:
		if( likely( !(*p & 0x80) ) )
		{
			if( unlikely(*p >= 'A' && *p <= 'Z') )
				*p -= 'A' - 'a';
			++p;
			continue;
		}

		int iRemaining = iLen - (p-pStart);
		p += UnicodeDoUpper( p, iRemaining, g_LowerCase );
	}
}

void UnicodeUpperLower( wchar_t *p, size_t iLen, const unsigned char pMapping[256] )
{
	wchar_t *pEnd = p + iLen;
	while( p != pEnd )
	{
		if( *p < 256 )
			*p = pMapping[*p];
		++p;
	}
}

void MakeUpper( wchar_t *p, size_t iLen )
{
	UnicodeUpperLower( p, iLen, g_UpperCase );
}

void MakeLower( wchar_t *p, size_t iLen )
{
	UnicodeUpperLower( p, iLen, g_LowerCase );
}

float HHMMSSToSeconds( const RString &sHHMMSS )
{
	vector<RString> arrayBits;
	split( sHHMMSS, ":", arrayBits, false );

	while( arrayBits.size() < 3 )
		arrayBits.insert(arrayBits.begin(), "0" );	// pad missing bits

	float fSeconds = 0;
	fSeconds += StringToInt( arrayBits[0] ) * 60 * 60;
	fSeconds += StringToInt( arrayBits[1] ) * 60;
	fSeconds += StringToFloat( arrayBits[2] );

	return fSeconds;
}

RString SecondsToHHMMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	RString sReturn = ssprintf( "%02d:%02d:%02d", iMinsDisplay/60, iMinsDisplay%60, iSecsDisplay );
	return sReturn;
}

RString SecondsToMMSSMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	RString sReturn = ssprintf( "%02d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

RString SecondsToMSSMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	RString sReturn = ssprintf( "%01d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

RString SecondsToMMSSMsMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 1000 );
	RString sReturn = ssprintf( "%02d:%02d.%03d", iMinsDisplay, iSecsDisplay, min(999,iLeftoverDisplay) );
	return sReturn;
}

RString SecondsToMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	RString sReturn = ssprintf( "%01d:%02d", iMinsDisplay, iSecsDisplay);
	return sReturn;
}

RString SecondsToMMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	RString sReturn = ssprintf( "%02d:%02d", iMinsDisplay, iSecsDisplay);
	return sReturn;
}

template <class S>
static int DelimitorLength( const S &Delimitor )
{
	return Delimitor.size();
}

static int DelimitorLength( char Delimitor )
{
	return 1;
}

static int DelimitorLength( wchar_t Delimitor )
{
	return 1;
}

template <class S, class C>
void do_split( const S &Source, const C Delimitor, vector<S> &AddIt, const bool bIgnoreEmpty )
{
	/* Short-circuit if the source is empty; we want to return an empty vector if
	 * the string is empty, even if bIgnoreEmpty is true. */
	if( Source.empty() )
		return;

	size_t startpos = 0;

	do {
		size_t pos;
		pos = Source.find( Delimitor, startpos );
		if( pos == Source.npos )
			pos = Source.size();

		if( pos-startpos > 0 || !bIgnoreEmpty )
		{
			/* Optimization: if we're copying the whole string, avoid substr; this
			 * allows this copy to be refcounted, which is much faster. */
			if( startpos == 0 && pos-startpos == Source.size() )
				AddIt.push_back(Source);
			else
			{
				const S AddRString = Source.substr(startpos, pos-startpos);
				AddIt.push_back(AddRString);
			}
		}

		startpos = pos+DelimitorLength(Delimitor);
	} while ( startpos <= Source.size() );
}

void split( const RString &sSource, const RString &sDelimitor, vector<RString> &asAddIt, const bool bIgnoreEmpty )
{
	if( sDelimitor.size() == 1 )
		do_split( sSource, sDelimitor[0], asAddIt, bIgnoreEmpty );
	else
		do_split( sSource, sDelimitor, asAddIt, bIgnoreEmpty );
}

void split( const wstring &sSource, const wstring &sDelimitor, vector<wstring> &asAddIt, const bool bIgnoreEmpty )
{
	if( sDelimitor.size() == 1 )
		do_split( sSource, sDelimitor[0], asAddIt, bIgnoreEmpty );
	else
		do_split( sSource, sDelimitor, asAddIt, bIgnoreEmpty );
}

/* Use:

RString str="a,b,c";
int start = 0, size = -1;
for(;;)
{
	do_split( str, ",", start, size );
	if( start == str.size() )
		break;
	str[start] = 'Q';
}

*/

template <class S>
void do_split( const S &Source, const S &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	if( size != -1 )
	{
		// Start points to the beginning of the last delimiter. Move it up.
		begin += size+Delimitor.size();
		begin = min( begin, len );
	}

	size = 0;

	if( bIgnoreEmpty )
	{
		// Skip delims.
		while( begin + Delimitor.size() < Source.size() &&
			!Source.compare( begin, Delimitor.size(), Delimitor ) )
			++begin;
	}

	/* Where's the string function to find within a substring?
	 * C++ strings apparently are missing that ... */
	size_t pos;
	if( Delimitor.size() == 1 )
		pos = Source.find( Delimitor[0], begin );
	else
		pos = Source.find( Delimitor, begin );
	if( pos == Source.npos || (int) pos > len )
		pos = len;
	size = pos - begin;
}

void split( const RString &Source, const RString &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );
}

void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );
}

void split( const RString &Source, const RString &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
}

void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
}

RString BinaryToHex( const void *pData_, size_t iNumBytes )
{
	const unsigned char *pData = (const unsigned char *) pData_;
	RString s;
	for( size_t i=0; i<iNumBytes; i++ )
	{
		unsigned val = pData[i];
		s += ssprintf( "%02x", val );
	}
	return s;
}

RString BinaryToHex( const RString &sString )
{
	return BinaryToHex( sString.data(), sString.size() );
}

void CRC32( unsigned int &iCRC, const void *pVoidBuffer, size_t iSize )
{
	static unsigned tab[256];
	static bool initted = false;
	if( !initted )
	{
		initted = true;
		const unsigned POLY = 0xEDB88320;

		for( int i = 0; i < 256; ++i )
		{
			tab[i] = i;
			for( int j = 0; j < 8; ++j )
			{
				if( tab[i] & 1 )
					tab[i] = (tab[i] >> 1) ^ POLY;
				else
					tab[i] >>= 1;
			}
		}
	}

	iCRC ^= 0xFFFFFFFF;

	const char *pBuffer = (const char *) pVoidBuffer;
	for( unsigned i = 0; i < iSize; ++i )
		iCRC = (iCRC >> 8) ^ tab[(iCRC ^ pBuffer[i]) & 0xFF];

	iCRC ^= 0xFFFFFFFF;
}

unsigned int GetHashForString( const RString &s )
{
	unsigned crc = 0;
	CRC32( crc, s.data(), s.size() );
	return crc;
}

int utf8_get_char_len( char p )
{
	if( !(p & 0x80) ) return 1; /* 0xxxxxxx - 1 */
	if( !(p & 0x40) ) return 1; /* 10xxxxxx - continuation */
	if( !(p & 0x20) ) return 2; /* 110xxxxx */
	if( !(p & 0x10) ) return 3; /* 1110xxxx */
	if( !(p & 0x08) ) return 4; /* 11110xxx */
	if( !(p & 0x04) ) return 5; /* 111110xx */
	if( !(p & 0x02) ) return 6; /* 1111110x */
	return 1; /* 1111111x */
}

bool utf8_to_wchar( const char *s, size_t iLength, unsigned &start, wchar_t &ch )
{
	if( start >= iLength )
		return false;

	int len = utf8_get_char_len( s[start] );

	if( start+len > iLength )
	{
		// We don't have room for enough continuation bytes. Return error.
		start += len;
		ch = L'?';
		return false;
	}

	switch( len )
	{
	case 1:
		ch = (s[start+0] & 0x7F);
		break;
	case 2:
		ch = ( (s[start+0] & 0x1F) << 6 ) |
		       (s[start+1] & 0x3F);
		break;
	case 3:
		ch = ( (s[start+0] & 0x0F) << 12 ) |
		     ( (s[start+1] & 0x3F) << 6 ) |
		       (s[start+2] & 0x3F);
		break;
	case 4:
		ch = ( (s[start+0] & 0x07) << 18 ) |
		     ( (s[start+1] & 0x3F) << 12 ) |
		     ( (s[start+2] & 0x3F) << 6 ) |
		     (s[start+3] & 0x3F);
		break;
	case 5:
		ch = ( (s[start+0] & 0x03) << 24 ) |
		     ( (s[start+1] & 0x3F) << 18 ) |
		     ( (s[start+2] & 0x3F) << 12 ) |
		     ( (s[start+3] & 0x3F) << 6 ) |
		     (s[start+4] & 0x3F);
		break;

	case 6:
		ch = ( (s[start+0] & 0x01) << 30 ) |
		     ( (s[start+1] & 0x3F) << 24 ) |
		     ( (s[start+2] & 0x3F) << 18 ) |
		     ( (s[start+3] & 0x3F) << 12) |
		     ( (s[start+4] & 0x3F) << 6 ) |
		     (s[start+5] & 0x3F);
		break;

	}

	start += len;
	return true;
}

void wchar_to_utf8( wchar_t ch, RString &out )
{
	if( ch < 0x80 ) { out.append( 1, (char) ch ); return; }

	int cbytes = 0;
	if( ch < 0x800 ) cbytes = 1;
	else if( ch < 0x10000 )    cbytes = 2;
	else if( ch < 0x200000 )   cbytes = 3;
	else if( ch < 0x4000000 )  cbytes = 4;
	else cbytes = 5;

	{
		int shift = cbytes*6;
		const int init_masks[] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		out.append( 1, (char) (init_masks[cbytes-1] | (ch>>shift)) );
	}

	for( int i = 0; i < cbytes; ++i )
	{
		int shift = (cbytes-i-1)*6;
		out.append( 1, (char) (0x80 | ((ch>>shift)&0x3F)) );
	}
}

static inline bool is_utf8_continuation_byte( char c )
{
	return (c & 0xC0) == 0x80;
}

bool utf8_to_wchar_ec( const RString &s, unsigned &start, wchar_t &ch )
{
	if( start >= s.size() )
		return false;

	if( is_utf8_continuation_byte( s[start] ) || /* misplaced continuation byte */
		(s[start] & 0xFE) == 0xFE ) /* 0xFE, 0xFF */
	{
		start += 1;
		return false;
	}

	int len = utf8_get_char_len( s[start] );

	const int first_byte_mask[] = { -1, 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

	ch = wchar_t(s[start] & first_byte_mask[len]);

	for( int i = 1; i < len; ++i )
	{
		if( start+i >= s.size() )
		{
			/* We expected a continuation byte, but didn't get one. Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}

		char byte = s[start+i];
		if( !is_utf8_continuation_byte(byte) )
		{
			/* We expected a continuation byte, but didn't get one. Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}
		ch = (ch << 6) | (byte & 0x3F);
	}

	bool bValid = true;
	{
		unsigned c1 = (unsigned) s[start] & 0xFF;
		unsigned c2 = (unsigned) s[start+1] & 0xFF;
		int c = (c1 << 8) + c2;
		if( (c & 0xFE00) == 0xC000 ||
		    (c & 0xFFE0) == 0xE080 ||
		    (c & 0xFFF0) == 0xF080 ||
		    (c & 0xFFF8) == 0xF880 ||
		    (c & 0xFFFC) == 0xFC80 )
	    {
		    bValid = false;
	    }
	}

	if( ch == 0xFFFE || ch == 0xFFFF )
		bValid = false;

	start += len;
	return bValid;
}

bool utf8_is_valid( const RString &s )
{
	for( unsigned start = 0; start < s.size(); )
	{
		wchar_t ch;
		if( !utf8_to_wchar_ec( s, start, ch ) )
			return false;
	}
	return true;
}

void TrimLeft( RString &sStr, const char *s )
{
	int n = 0;
	while( n < int(sStr.size()) && strchr(s, sStr[n]) )
		n++;

	sStr.erase( sStr.begin(), sStr.begin()+n );
}

void TrimRight( RString &sStr, const char *s )
{
	int n = sStr.size();
	while( n > 0 && strchr(s, sStr[n-1]) )
		n--;

	/* Delete from n to the end. If n == sStr.size(), nothing is deleted;
	 * if n == 0, the whole string is erased. */
	sStr.erase( sStr.begin()+n, sStr.end() );
}

/*
 * Copyright (c) 2001-2005 Chris Danford, Glenn Maynard
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
