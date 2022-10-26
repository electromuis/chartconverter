#include "global.h"

#include <tomcrypt.h>

#include "CryptManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "CryptHelpers.h"

CryptManager::CryptManager()
{
}

CryptManager::~CryptManager()
{
}

RString CryptManager::GetMD5ForString( RString sData )
{
	unsigned char digest[16];

	int iHash = register_hash( &md5_desc );

	hash_state hash;
	hash_descriptor[iHash].init( &hash );
	hash_descriptor[iHash].process( &hash, (const unsigned char *) sData.data(), sData.size() );
	hash_descriptor[iHash].done( &hash, digest );

	return RString( (const char *) digest, sizeof(digest) );
}

RString CryptManager::GetSHA1ForString( RString sData )
{
	unsigned char digest[20];

	int iHash = register_hash( &sha1_desc );

	hash_state hash;
	hash_descriptor[iHash].init( &hash );
	hash_descriptor[iHash].process( &hash, (const unsigned char *) sData.data(), sData.size() );
	hash_descriptor[iHash].done( &hash, digest );

	return RString( (const char *) digest, sizeof(digest) );
}

RString CryptManager::GetSHA256ForString( RString sData )
{
	unsigned char digest[32];

	int iHash = register_hash( &sha256_desc );

	hash_state hash;
	hash_descriptor[iHash].init( &hash );
	hash_descriptor[iHash].process( &hash, (const unsigned char *) sData.data(), sData.size() );
	hash_descriptor[iHash].done( &hash, digest );

	return RString( (const char *) digest, sizeof(digest) );
}




/*
 * (c) 2004-2007 Chris Danford, Glenn Maynard
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
