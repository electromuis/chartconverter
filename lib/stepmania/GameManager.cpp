#include "global.h"
#include "GameManager.h"

GameManager*	GAMEMAN = nullptr;	// global and accessable from anywhere in our program

static const StepsTypeInfo g_StepsTypeInfos[] = {
	// dance
	{ "dance-single",	4,	true,	StepsTypeCategory_Single },
	{ "dance-double",	8,	true,	StepsTypeCategory_Double },
	{ "dance-couple",	8,	true,	StepsTypeCategory_Couple },
	{ "dance-solo",		6,	true,	StepsTypeCategory_Single },
	{ "dance-threepanel",	3,	true,	StepsTypeCategory_Single }, // thanks to kurisu
	{ "dance-routine",	8,	false,	StepsTypeCategory_Routine },
	// pump
	{ "pump-single",	5,	true,	StepsTypeCategory_Single },
	{ "pump-halfdouble",	6,	true,	StepsTypeCategory_Double },
	{ "pump-double",	10,	true,	StepsTypeCategory_Double },
	{ "pump-couple",	10,	true,	StepsTypeCategory_Couple },
	// uh, dance-routine has that one bool as false... wtf? -aj
	{ "pump-routine",	10,	true,	StepsTypeCategory_Routine },
	// kb7
	{ "kb7-single",		7,	true,	StepsTypeCategory_Single },
	// { "kb7-small",		7,	true,	StepsTypeCategory_Single },
	// ez2dancer
	{ "ez2-single",		5,	true,	StepsTypeCategory_Single },	// Single: TL,LHH,D,RHH,TR
	{ "ez2-double",		10,	true,	StepsTypeCategory_Double },	// Double: Single x2
	{ "ez2-real",		7,	true,	StepsTypeCategory_Single },	// Real: TL,LHH,LHL,D,RHL,RHH,TR
	// parapara paradise
	{ "para-single",	5,	true,	StepsTypeCategory_Single },
	// ds3ddx
	{ "ds3ddx-single",	8,	true,	StepsTypeCategory_Single },
	// beatmania
	{ "bm-single5",		6,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-versus5",		6,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-double5",		12,	true,	StepsTypeCategory_Double },	// called "bm" for backward compat
	{ "bm-single7",		8,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-versus7",		8,	true,	StepsTypeCategory_Single },	// called "bm" for backward compat
	{ "bm-double7",		16,	true,	StepsTypeCategory_Double },	// called "bm" for backward compat
	// dance maniax
	{ "maniax-single",	4,	true,	StepsTypeCategory_Single },
	{ "maniax-double",	8,	true,	StepsTypeCategory_Double },
	// technomotion
	{ "techno-single4",	4,	true,	StepsTypeCategory_Single },
	{ "techno-single5",	5,	true,	StepsTypeCategory_Single },
	{ "techno-single8",	8,	true,	StepsTypeCategory_Single },
	{ "techno-double4",	8,	true,	StepsTypeCategory_Double },
	{ "techno-double5",	10,	true,	StepsTypeCategory_Double },
	{ "techno-double8",	16,	true,	StepsTypeCategory_Double },
	// pop'n music
	{ "pnm-five",		5,	true,	StepsTypeCategory_Single },	// called "pnm" for backward compat
	{ "pnm-nine",		9,	true,	StepsTypeCategory_Single },	// called "pnm" for backward compat
	{ "lights-cabinet",	0,	false,	StepsTypeCategory_Single },
	// kickbox mania
	{ "kickbox-human", 4, true, StepsTypeCategory_Single },
	{ "kickbox-quadarm", 4, true, StepsTypeCategory_Single },
	{ "kickbox-insect", 6, true, StepsTypeCategory_Single },
	{ "kickbox-arachnid", 8, true, StepsTypeCategory_Single },
};

StepsType GameManager::StringToStepsType( RString sStepsType )
{
	sStepsType.MakeLower();

	for( int i=0; i<NUM_StepsType; i++ )
		if( g_StepsTypeInfos[i].szName == sStepsType )
			return StepsType(i);

	return StepsType_Invalid;
}

const StepsTypeInfo &GameManager::GetStepsTypeInfo( StepsType st )
{
	ASSERT( ARRAYLEN(g_StepsTypeInfos) == NUM_StepsType );
	ASSERT_M( st < NUM_StepsType, ssprintf("StepsType %d < NUM_StepsType (%d)", st, NUM_StepsType) );
	return g_StepsTypeInfos[st];
}

/*
 * (c) 2001-2006 Chris Danford, Glenn Maynard
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
