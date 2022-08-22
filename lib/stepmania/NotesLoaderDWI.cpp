#include "global.h"
#include "NotesLoaderDWI.h"
#include "RageLog.h"
#include "MsdFile.h"
#include "RageUtil.h"
#include "RageUtil_CharConversions.h"
#include "NoteData.h"
#include "Song.h"
#include "Steps.h"
#include "NotesLoader.h"
#include "Difficulty.h"

#include <map>

Difficulty DwiCompatibleStringToDifficulty( const RString& sDC );

static std::map<int,int> g_mapDanceNoteToNoteDataColumn;

/** @brief The different types of core DWI arrows and pads. */
enum DanceNotes
{
	DANCE_NOTE_NONE = 0,
	DANCE_NOTE_PAD1_LEFT,
	DANCE_NOTE_PAD1_UPLEFT,
	DANCE_NOTE_PAD1_DOWN,
	DANCE_NOTE_PAD1_UP,
	DANCE_NOTE_PAD1_UPRIGHT,
	DANCE_NOTE_PAD1_RIGHT,
	DANCE_NOTE_PAD2_LEFT,
	DANCE_NOTE_PAD2_UPLEFT,
	DANCE_NOTE_PAD2_DOWN,
	DANCE_NOTE_PAD2_UP,
	DANCE_NOTE_PAD2_UPRIGHT,
	DANCE_NOTE_PAD2_RIGHT
};

/**
 * @brief Turn the individual character to the proper note.
 * @param c The character in question.
 * @param i The player.
 * @param note1Out The first result based on the character.
 * @param note2Out The second result based on the character.
 * @param sPath the path to the file.
 */
static void DWIcharToNote( char c, bool isP2, int &note1Out, int &note2Out, const RString &sPath )
{
	switch( c )
	{
	case '0':	note1Out = DANCE_NOTE_NONE;		note2Out = DANCE_NOTE_NONE;		break;
	case '1':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_LEFT;	break;
	case '2':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_NONE;		break;
	case '3':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case '4':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_NONE;		break;
	case '5':	note1Out = DANCE_NOTE_NONE;		note2Out = DANCE_NOTE_NONE;		break;
	case '6':	note1Out = DANCE_NOTE_PAD1_RIGHT;	note2Out = DANCE_NOTE_NONE;		break;
	case '7':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_LEFT;	break;
	case '8':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_NONE;		break;
	case '9':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'A':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_DOWN;	break;
	case 'B':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'C':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_NONE;		break;
	case 'D':	note1Out = DANCE_NOTE_PAD1_UPRIGHT;	note2Out = DANCE_NOTE_NONE;		break;
	case 'E':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_UPLEFT;	break;
	case 'F':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_DOWN;	break;
	case 'G':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_UP;		break;
	case 'H':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'I':	note1Out = DANCE_NOTE_PAD1_LEFT;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	case 'J':	note1Out = DANCE_NOTE_PAD1_DOWN;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	case 'K':	note1Out = DANCE_NOTE_PAD1_UP;		note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	case 'L':	note1Out = DANCE_NOTE_PAD1_UPRIGHT;	note2Out = DANCE_NOTE_PAD1_RIGHT;	break;
	case 'M':	note1Out = DANCE_NOTE_PAD1_UPLEFT;	note2Out = DANCE_NOTE_PAD1_UPRIGHT;	break;
	default:	
			LOG->UserLog( "Song file", sPath, "has an invalid DWI note character '%c'.", c );
			note1Out = DANCE_NOTE_NONE;		note2Out = DANCE_NOTE_NONE;		break;
	}
	
	if(isP2) {
		if( note1Out != DANCE_NOTE_NONE )
			note1Out += 6;
		if( note2Out != DANCE_NOTE_NONE )
			note2Out += 6;
	}
}

/**
 * @brief Determine the note column[s] to place notes.
 * @param c The character in question.
 * @param i The player.
 * @param col1Out The first result based on the character.
 * @param col2Out The second result based on the character.
 * @param sPath the path to the file.
 */
static void DWIcharToNoteCol( char c, bool isP2, int &col1Out, int &col2Out, const RString &sPath )
{
	int note1, note2;
	DWIcharToNote( c, isP2, note1, note2, sPath );

	if( note1 != DANCE_NOTE_NONE )
		col1Out = g_mapDanceNoteToNoteDataColumn[note1];
	else
		col1Out = -1;

	if( note2 != DANCE_NOTE_NONE )
		col2Out = g_mapDanceNoteToNoteDataColumn[note2];
	else
		col2Out = -1;
}

/**
 * @brief Determine if the note in question is a 192nd note.
 *
 * DWI used to use <...> to indicate 1/192nd notes; at some
 * point, <...> was changed to indicate jumps, and `' was used for
 * 1/192nds.  So, we have to do a check to figure out what it really
 * means.  If it contains 0s, it's most likely 192nds; otherwise,
 * it's most likely a jump.  Search for a 0 before the next >: 
 * @param sStepData the step data.
 * @param pos the position of the step data.
 * @return true if it's a 192nd note, false otherwise.
 */
static bool Is192( const RString &sStepData, size_t pos )
{
	while( pos < sStepData.size() )
	{
		if( sStepData[pos] == '>' )
			return false;
		if( sStepData[pos] == '0' )
			return true;
		++pos;
	}
	
	return false;
}
/** @brief All DWI files use 4 beats per measure. */
const int BEATS_PER_MEASURE = 4;

/* We prefer the normal names; recognize a number of others, too. (They'll get
 * normalized when written to SMs, etc.) */
Difficulty DwiCompatibleStringToDifficulty( const RString& sDC )
{
	RString s2 = sDC;
	s2.MakeLower();
	if( s2 == "beginner" )			return Difficulty_Beginner;
	else if( s2 == "easy" )		return Difficulty_Easy;
	else if( s2 == "basic" )		return Difficulty_Easy;
	else if( s2 == "light" )		return Difficulty_Easy;
	else if( s2 == "medium" )		return Difficulty_Medium;
	else if( s2 == "another" )		return Difficulty_Medium;
	else if( s2 == "trick" )		return Difficulty_Medium;
	else if( s2 == "standard" )	return Difficulty_Medium;
	else if( s2 == "difficult")	return Difficulty_Medium;
	else if( s2 == "hard" )		return Difficulty_Hard;
	else if( s2 == "ssr" )			return Difficulty_Hard;
	else if( s2 == "maniac" )		return Difficulty_Hard;
	else if( s2 == "heavy" )		return Difficulty_Hard;
	else if( s2 == "smaniac" )		return Difficulty_Challenge;
	else if( s2 == "challenge" )	return Difficulty_Challenge;
	else if( s2 == "expert" )		return Difficulty_Challenge;
	else if( s2 == "oni" )			return Difficulty_Challenge;
	else if( s2 == "edit" )		return Difficulty_Edit;
	else							return Difficulty_Invalid;
}

static StepsType GetTypeFromMode(const RString &mode)
{
	if( mode == "SINGLE" )
		return StepsType_dance_single;
	else if( mode == "DOUBLE" )
		return StepsType_dance_double;
	else if( mode == "COUPLE" )
		return StepsType_dance_couple;
	else if( mode == "SOLO" )
		return StepsType_dance_solo;
	ASSERT_M(0, "Unrecognized DWI notes format " + mode + "!");
	return StepsType_Invalid; // just in case.
}

static NoteData ParseNoteData(RString &step1, RString &step2,
			      Steps &out, const RString &path)
{
	g_mapDanceNoteToNoteDataColumn.clear();
	switch( out.m_StepsType )
	{
		case StepsType_dance_single:
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
			break;
		case StepsType_dance_double:
		case StepsType_dance_couple:
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 1;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 2;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 3;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_LEFT] = 4;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_DOWN] = 5;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_UP] = 6;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD2_RIGHT] = 7;
			break;
		case StepsType_dance_solo:
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_LEFT] = 0;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPLEFT] = 1;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_DOWN] = 2;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UP] = 3;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_UPRIGHT] = 4;
			g_mapDanceNoteToNoteDataColumn[DANCE_NOTE_PAD1_RIGHT] = 5;
			break;
			DEFAULT_FAIL( out.m_StepsType );
	}
	
	NoteData newNoteData;
	newNoteData.SetNumTracks( g_mapDanceNoteToNoteDataColumn.size() );
	
	for( int pad=0; pad<2; pad++ )		// foreach pad
	{
		RString sStepData;
		switch( pad )
		{
			case 0:
				sStepData = step1;
				break;
			case 1:
				if( step2 == "" )	// no data
					continue;	// skip
				sStepData = step2;
				break;
				DEFAULT_FAIL( pad );
		}
		
		sStepData.Replace("\n", "");
		sStepData.Replace("\r", "");
		sStepData.Replace("\t", "");
		sStepData.Replace(" ", "");
		
		double fCurrentBeat = 0;
		double fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
		
		for( size_t i=0; i<sStepData.size(); )
		{
			char c = sStepData[i++];
			switch( c )
			{
					// begins a series
				case '(':
					fCurrentIncrementer = 1.0/16 * BEATS_PER_MEASURE;
					break;
				case '[':
					fCurrentIncrementer = 1.0/24 * BEATS_PER_MEASURE;
					break;
				case '{':
					fCurrentIncrementer = 1.0/64 * BEATS_PER_MEASURE;
					break;
				case '`':
					fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
					break;
					
					// ends a series
				case ')':
				case ']':
				case '}':
				case '\'':
				case '>':
					fCurrentIncrementer = 1.0/8 * BEATS_PER_MEASURE;
					break;
					
				default:	// this is a note character
				{
					if( c == '!' )
					{
						LOG->UserLog(
							     "Song file",
							     path,
							     "has an unexpected character: '!'." );
						continue;
					}
					
					bool jump = false;
					if( c == '<' )
					{
						/* Arr.  Is this a jump or a 1/192 marker? */
						if( Is192( sStepData, i ) )
						{
							fCurrentIncrementer = 1.0/192 * BEATS_PER_MEASURE;
							break;
						}
						
						/* It's a jump.
						 * We need to keep reading notes until we hit a >. */
						jump = true;
						i++;
					}
					
					const int iIndex = BeatToNoteRow( (float)fCurrentBeat );
					i--;
					do {
						c = sStepData[i++];
						
						if( jump && c == '>' )
							break;
						
						int iCol1, iCol2;
						DWIcharToNoteCol(
								 c,
								 false,
								 iCol1,
								 iCol2,
								 path );
						
						if( iCol1 != -1 )
							newNoteData.SetTapNote(iCol1,
									       iIndex,
									       TAP_ORIGINAL_TAP);
						if( iCol2 != -1 )
							newNoteData.SetTapNote(iCol2,
									       iIndex,
									       TAP_ORIGINAL_TAP);
						
						if(i>=sStepData.length())
						{
							break;
							//we ran out of data
							//while looking for the ending > mark
						}
						
						if( sStepData[i] == '!' )
						{
							i++;
							const char holdChar = sStepData[i++];
							
							DWIcharToNoteCol(holdChar,
									 false,
									 iCol1,
									 iCol2,
									 path );
							
							if( iCol1 != -1 )
								newNoteData.SetTapNote(iCol1,
										       iIndex,
										       TAP_ORIGINAL_HOLD_HEAD);
							if( iCol2 != -1 )
								newNoteData.SetTapNote(iCol2,
										       iIndex,
										       TAP_ORIGINAL_HOLD_HEAD);
						}
					}
					while( jump );
					fCurrentBeat += fCurrentIncrementer;
				}
					break;
			}
		}
	}
	
	/* Fill in iDuration. */
	for( int t=0; t<newNoteData.GetNumTracks(); ++t )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK( newNoteData, t, iHeadRow )
		{
			TapNote tn = newNoteData.GetTapNote( t, iHeadRow  );
			if( tn.type != TapNoteType_HoldHead )
				continue;
			
			int iTailRow = iHeadRow;
			bool bFound = false;
			while( !bFound && newNoteData.GetNextTapNoteRowForTrack(t, iTailRow) )
			{
				const TapNote &TailTap = newNoteData.GetTapNote( t, iTailRow );
				if( TailTap.type == TapNoteType_Empty )
					continue;

				newNoteData.SetTapNote( t, iTailRow, TAP_EMPTY );
				tn.iDuration = iTailRow - iHeadRow;
				newNoteData.SetTapNote( t, iHeadRow, tn );
				bFound = true;
			}
			
			if( !bFound )
			{
				/* The hold was never closed.  */
				LOG->UserLog("Song file",
					     path,
					     "failed to close a hold note in \"%s\" on track %i", 
					     DifficultyToString(out.GetDifficulty()).c_str(),
					     t);
				
				newNoteData.SetTapNote( t, iHeadRow, TAP_EMPTY );
			}
		}
	}
	
	ASSERT( newNoteData.GetNumTracks() > 0 );
	return newNoteData;
}

/**
 * @brief Look through the notes tag to extract the data.
 * @param sMode the steps type.
 * @param sDescription the difficulty.
 * @param sNumFeet the meter.
 * @param sStepData1 the guaranteed step data.
 * @param sStepData2 used if sMode is double or couple.
 * @param out the step data.
 * @param sPath the path to the file.
 * @return the success or failure of the operation.
 */
static bool LoadFromDWITokens( 
	RString sMode, 
	RString sDescription,
	RString sNumFeet,
	RString sStepData1, 
	RString sStepData2,
	Steps &out,
	const RString &sPath )
{
	out.m_StepsType = GetTypeFromMode(sMode);

	// if the meter is empty, force it to 1.
	if( sNumFeet.empty() )
		sNumFeet = "1";

	out.SetMeter(StringToInt(sNumFeet));

	out.SetDifficulty( DwiCompatibleStringToDifficulty(sDescription) );

	out.SetNoteData( ParseNoteData(sStepData1, sStepData2, out, sPath) );

	out.TidyUpData();

	return true;
}

/**
 * @brief Turn the DWI style timestamp into a compatible time for our system.
 *
 * This value can be in either "HH:MM:SS.sssss", "MM:SS.sssss", "SSS.sssss"
 * or milliseconds.
 * @param arg1 Either hours, minutes, or seconds, depending on other args.
 * @param arg2 Either minutes or seconds, depending on other args.
 * @param arg3 Seconds if not empty.
 * @return the proper timestamp.
 */
static float ParseBrokenDWITimestamp( const RString &arg1, const RString &arg2, const RString &arg3 )
{
	if( arg1.empty() )
		return 0;

	/* 1+ args */
	if( arg2.empty() )
	{
		/* If the value contains a period, treat it as seconds; otherwise ms. */
		if( arg1.find_first_of(".") != arg1.npos )
			return StringToFloat( arg1 );
		else
			return StringToFloat( arg1 ) / 1000.f;
	}

	/* 2+ args */
	if( arg3.empty() )
		return HHMMSSToSeconds( arg1+":"+arg2 );

	/* 3+ args */
	return HHMMSSToSeconds( arg1+":"+arg2+":"+arg3 );
}


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
