#include "global.h"
#include "Difficulty.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "Steps.h"

static const char *DifficultyNames[] = {
	"Beginner",
	"Easy",
	"Medium",
	"Hard",
	"Challenge",
	"Edit",
};
XToString( Difficulty );
StringToX( Difficulty );

struct OldStyleStringToDifficultyMapHolder
{
	std::map<RString, Difficulty> conversion_map;
	OldStyleStringToDifficultyMapHolder()
	{
		conversion_map["beginner"]= Difficulty_Beginner;
		conversion_map["easy"]= Difficulty_Easy;
		conversion_map["basic"]= Difficulty_Easy;
		conversion_map["light"]= Difficulty_Easy;
		conversion_map["medium"]= Difficulty_Medium;
		conversion_map["another"]= Difficulty_Medium;
		conversion_map["trick"]= Difficulty_Medium;
		conversion_map["standard"]= Difficulty_Medium;
		conversion_map["difficult"]= Difficulty_Medium;
		conversion_map["hard"]= Difficulty_Hard;
		conversion_map["ssr"]= Difficulty_Hard;
		conversion_map["maniac"]= Difficulty_Hard;
		conversion_map["heavy"]= Difficulty_Hard;
		conversion_map["smaniac"]= Difficulty_Challenge;
		conversion_map["challenge"]= Difficulty_Challenge;
		conversion_map["expert"]= Difficulty_Challenge;
		conversion_map["oni"]= Difficulty_Challenge;
		conversion_map["edit"]= Difficulty_Edit;
	}
};
OldStyleStringToDifficultyMapHolder OldStyleStringToDifficulty_converter;
Difficulty OldStyleStringToDifficulty( const RString& sDC )
{
	RString s2 = sDC;
	s2.MakeLower();
	std::map<RString, Difficulty>::iterator diff=
		OldStyleStringToDifficulty_converter.conversion_map.find(s2);
	if(diff != OldStyleStringToDifficulty_converter.conversion_map.end())
	{
		return diff->second;
	}
	return Difficulty_Invalid;
}

/*
 * (c) 2001-2004 Chris Danford
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
