#include "global.h"
#include "NotesLoaderSMA.h"
#include "GameManager.h"
#include "MsdFile.h"
#include "NoteTypes.h"
#include "NotesLoaderSM.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "Steps.h"

void SMALoader::ProcessMultipliers( TimingData &out, const int iRowsPerBeat, const RString sParam )
{
	vector<RString> arrayMultiplierExpressions;
	split( sParam, ",", arrayMultiplierExpressions );
	
	for( unsigned f=0; f<arrayMultiplierExpressions.size(); f++ )
	{
		vector<RString> arrayMultiplierValues;
		split( arrayMultiplierExpressions[f], "=", arrayMultiplierValues );
		unsigned size = arrayMultiplierValues.size();
		if( size < 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #MULTIPLIER value \"%s\" (must have at least one '='), ignored.",
				     arrayMultiplierExpressions[f].c_str() );
			continue;
		}
		const float fComboBeat = RowToBeat( arrayMultiplierValues[0], iRowsPerBeat );
		const int iCombos = StringToInt( arrayMultiplierValues[1] ); // always true.
		// hoping I'm right here: SMA files can use 6 values after the row/beat.
		const int iMisses = (size == 2 || size == 4 ?
							 iCombos : 
							 StringToInt(arrayMultiplierValues[2]));
		out.AddSegment( ComboSegment(BeatToNoteRow(fComboBeat), iCombos, iMisses) );
	}
}

void SMALoader::ProcessBeatsPerMeasure( TimingData &out, const RString sParam )
{
	vector<RString> vs1;
	split( sParam, ",", vs1 );
	
	for (RString const &s1 : vs1)
	{
		vector<RString> vs2;
		split( s1, "=", vs2 );

		if( vs2.size() < 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid beats per measure change with %i values.",
				     static_cast<int>(vs2.size()) );
			continue;
		}
		const float fBeat = StringToFloat( vs2[0] );
		const int iNumerator = StringToInt( vs2[1] );

		if( fBeat < 0 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid time signature change with beat %f.",
				     fBeat );
			continue;
		}
		if( iNumerator < 1 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid time signature change with beat %f, iNumerator %i.",
				     fBeat, iNumerator );
			continue;
		}

		out.AddSegment( TimeSignatureSegment(BeatToNoteRow(fBeat), iNumerator) );
	}
}

void SMALoader::ProcessSpeeds( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> vs1;
	split( line, ",", vs1 );

	for (std::vector<RString>::const_iterator s1 = vs1.begin(); s1 != vs1.end(); ++s1)
	{
		vector<RString> vs2;
		vs2.clear(); // trying something.
		RString loopTmp = *s1;
		Trim( loopTmp );
		split( loopTmp, "=", vs2 );
		
		if( vs2.size() == 2 ) // First one always seems to have 2.
		{
			// Aldo_MX: 4 is the default value in SMA, although SM5 requires 0 for the first segment :/
			vs2.push_back(s1 == vs1.begin() ? "0" : "4");
		}
		
		if( vs2.size() < 3 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with %i values.",
				     static_cast<int>(vs2.size()) );
			continue;
		}

		const float fBeat = RowToBeat( vs2[0], rowsPerBeat );

		RString backup = vs2[2];
		Trim(vs2[2], "s");
		Trim(vs2[2], "S");

		const float fRatio = StringToFloat( vs2[1] );
		const float fDelay = StringToFloat( vs2[2] );

		SpeedSegment::BaseUnit unit = ((backup != vs2[2]) ?
			SpeedSegment::UNIT_SECONDS : SpeedSegment::UNIT_BEATS);


		if( fBeat < 0 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with beat %f.",
				     fBeat );
			continue;
		}

		if( fDelay < 0 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with beat %f, length %f.",
				     fBeat, fDelay );
			continue;
		}

		out.AddSegment( SpeedSegment(BeatToNoteRow(fBeat), fRatio, fDelay, unit) );
	}
}


/**
 * @file
 * @author Aldo Fregoso, Jason Felds (c) 2009-2011
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
