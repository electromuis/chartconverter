#include "global.h"
#include <cerrno>
#include <cstring>
#include "NotesWriterSM.h"
#include "GameManager.h"
#include "NoteTypes.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "Steps.h"

RString SmEscape( const RString &sUnescaped )
{
	return SmEscape( sUnescaped.c_str(), sUnescaped.size() );
}

RString SmEscape( const char *cUnescaped, int len )
{
	RString answer = "";
	for( int i = 0; i < len; ++i )
	{
		// Other characters we could theoretically escape:
		// NotesWriterSM.cpp used to claim ',' should be escaped, but there was no explanation why
		// '#' is both a control character and a valid part of a parameter.  The only way for there to be
		//   any confusion is in a misformatted .sm file, though, so it is unnecessary to escape it.
		if( cUnescaped[i] == '/' && i + 1 < len && cUnescaped[i + 1] == '/' )
		{
			answer += "\\/\\/";
			++i; // increment here so we skip both //s
			continue;
		}
		if( cUnescaped[i] == '\\' || cUnescaped[i] == ':' || cUnescaped[i] == ';' )
		    answer += "\\";
		answer += cUnescaped[i];
	}
	return answer;
}

bool NotesWriterSM::Write( RageFile& f, const Song &out )
{
	vector<Steps*> steps = out.GetAllSteps();

	return Write(f, out, steps);
}

/**
 * @brief Write out the common tags for .SM files.
 * @param f the file in question.
 * @param out the Song in question. */
static void WriteGlobalTags( RageFile &f, const Song &out )
{
	const TimingData &timing = out.m_SongTiming;
	f.PutLine( ssprintf( "#TITLE:%s;", SmEscape(out.m_sMainTitle).c_str() ) );
	f.PutLine( ssprintf( "#SUBTITLE:%s;", SmEscape(out.m_sSubTitle).c_str() ) );
	f.PutLine( ssprintf( "#ARTIST:%s;", SmEscape(out.m_sArtist).c_str() ) );
	f.PutLine( ssprintf( "#TITLETRANSLIT:%s;", SmEscape(out.m_sMainTitleTranslit).c_str() ) );
	f.PutLine( ssprintf( "#SUBTITLETRANSLIT:%s;", SmEscape(out.m_sSubTitleTranslit).c_str() ) );
	f.PutLine( ssprintf( "#ARTISTTRANSLIT:%s;", SmEscape(out.m_sArtistTranslit).c_str() ) );
	f.PutLine( ssprintf( "#GENRE:%s;", SmEscape(out.m_sGenre).c_str() ) );
	f.PutLine( ssprintf( "#CREDIT:%s;", SmEscape(out.m_sCredit).c_str() ) );
	f.PutLine( ssprintf( "#BANNER:%s;", SmEscape(out.m_sBannerFile).c_str() ) );
	f.PutLine( ssprintf( "#BACKGROUND:%s;", SmEscape(out.m_sBackgroundFile).c_str() ) );
	f.PutLine( ssprintf( "#CDTITLE:%s;", SmEscape(out.m_sCDTitleFile).c_str() ) );
	f.PutLine( ssprintf( "#MUSIC:%s;", SmEscape(out.m_sMusicFile).c_str() ) );
	f.PutLine( ssprintf( "#OFFSET:%.6f;", out.m_SongTiming.m_fBeat0OffsetInSeconds ) );
	f.PutLine( ssprintf( "#SAMPLESTART:%.6f;", out.m_fMusicSampleStartSeconds ) );
	f.PutLine( ssprintf( "#SAMPLELENGTH:%.6f;", out.m_fMusicSampleLengthSeconds ) );
	float specBeat = out.GetSpecifiedLastBeat();
	if( specBeat > 0 )
		f.PutLine( ssprintf("#LASTBEATHINT:%.6f;", specBeat) );

	f.Write( "#SELECTABLE:" );
	switch(out.m_SelectionDisplay)
	{
		default:
			FAIL_M(ssprintf("Invalid selection display: %i", out.m_SelectionDisplay));
		case Song::SHOW_ALWAYS:	f.Write( "YES" );		break;
			//case Song::SHOW_NONSTOP:	f.Write( "NONSTOP" );	break;
		case Song::SHOW_NEVER:		f.Write( "NO" );		break;
	}
	f.PutLine( ";" );

	switch( out.m_DisplayBPMType )
	{
		case DISPLAY_BPM_ACTUAL:
			// write nothing
			break;
		case DISPLAY_BPM_SPECIFIED:
			if( out.m_fSpecifiedBPMMin == out.m_fSpecifiedBPMMax )
				f.PutLine( ssprintf( "#DISPLAYBPM:%.6f;", out.m_fSpecifiedBPMMin ) );
			else
				f.PutLine( ssprintf( "#DISPLAYBPM:%.6f:%.6f;",
									 out.m_fSpecifiedBPMMin, out.m_fSpecifiedBPMMax ) );
			break;
		case DISPLAY_BPM_RANDOM:
			f.PutLine( ssprintf( "#DISPLAYBPM:*;" ) );
			break;
		default:
			break;
	}


	f.Write( "#BPMS:" );
	const vector<TimingSegment *> &bpms = timing.GetTimingSegments(SEGMENT_BPM);
	for( unsigned i=0; i<bpms.size(); i++ )
	{
		const BPMSegment *bs = ToBPM(bpms[i]);

		f.PutLine( ssprintf( "%.6f=%.6f", bs->GetBeat(), bs->GetBPM() ) );
		if( i != bpms.size()-1 )
			f.Write( "," );
	}
	f.PutLine( ";" );

	const vector<TimingSegment *> &stops = timing.GetTimingSegments(SEGMENT_STOP);
	const vector<TimingSegment *> &delays = timing.GetTimingSegments(SEGMENT_DELAY);

	map<float, float> allPauses;
	const vector<TimingSegment *> &warps = timing.GetTimingSegments(SEGMENT_WARP);
	unsigned wSize = warps.size();
	if( wSize > 0 )
	{
		for( unsigned i=0; i < wSize; i++ )
		{
			const WarpSegment *ws = static_cast<WarpSegment *>(warps[i]);
			int iRow = ws->GetRow();
			float fBPS = 60 / out.m_SongTiming.GetBPMAtRow(iRow);
			float fSkip = fBPS * ws->GetLength();
			allPauses.insert(std::pair<float, float>(ws->GetBeat(), -fSkip));
		}
	}

	for( unsigned i=0; i<stops.size(); i++ )
	{
		const StopSegment *fs = ToStop( stops[i] );
		// Handle warps on the same row by summing the values.  Not sure this
		// plays out the same. -Kyz
		map<float, float>::iterator already_exists= allPauses.find(fs->GetBeat());
		if(already_exists != allPauses.end())
		{
			already_exists->second+= fs->GetPause();
		}
		else
		{
			allPauses.insert(pair<float, float>(fs->GetBeat(), fs->GetPause()));
		}
	}
	// Delays can't be negative: thus, no effect.
	for (TimingSegment const *ss : delays)
	{
		float fBeat = NoteRowToBeat( ss->GetRow()-1 );
		float fPause = ToDelay(ss)->GetPause();
		map<float, float>::iterator already_exists= allPauses.find(fBeat);
		if(already_exists != allPauses.end())
		{
			already_exists->second+= fPause;
		}
		else
		{
			allPauses.insert(pair<float,float>(fBeat, fPause));
		}
	}

	f.Write( "#STOPS:" );
	vector<RString> stopLines;
	for (std::pair<float const &, float const &> ap : allPauses)
	{
		stopLines.push_back(ssprintf("%.6f=%.6f", ap.first, ap.second));
	}
	f.PutLine(join(",\n", stopLines));

	f.PutLine( ";" );
}

/**
 * @brief Turn a vector of lines into a single line joined by newline characters.
 * @param lines the list of lines to join.
 * @return the joined lines. */
static RString JoinLineList( vector<RString> &lines )
{
	for( unsigned i = 0; i < lines.size(); ++i )
		TrimRight( lines[i] );

	/* Skip leading blanks. */
	unsigned j = 0;
	while( j < lines.size() && lines.size() == 0 )
		++j;

	return join( "\r\n", lines.begin()+j, lines.end() );
}

/**
 * @brief Retrieve the notes from the #NOTES tag.
 * @param song the Song in question.
 * @param in the Steps in question.
 * @return the #NOTES tag. */
static RString GetSMNotesTag( const Song &song, const Steps &in )
{
	vector<RString> lines;

	lines.push_back( "" );
	// Escape to prevent some clown from making a comment of "\r\n;"
	lines.push_back( ssprintf("//---------------%s - %s----------------",
							  in.m_StepsTypeStr.c_str(), SmEscape(in.GetDescription()).c_str()) );
	lines.push_back( song.m_vsKeysoundFile.empty() ? "#NOTES:" : "#NOTES2:" );
	lines.push_back( ssprintf( "     %s:", in.m_StepsTypeStr.c_str() ) );
	RString desc = in.GetChartName();
	lines.push_back( ssprintf( "     %s:", SmEscape(desc).c_str() ) );
	lines.push_back( ssprintf( "     %s:", DifficultyToString(in.GetDifficulty()).c_str() ) );
	lines.push_back( ssprintf( "     %d:", in.GetMeter() ) );

	vector<RString> asRadarValues;
	// OpenITG simfiles use 11 radar categories.
	int categories = 11;
	FOREACH_PlayerNumber( pn )
	{
		const RadarValues &rv = in.GetRadarValues( pn );
		// Can't use the foreach anymore due to flexible radar lines.
		for( RadarCategory rc = (RadarCategory)0; rc < categories;
			 enum_add<RadarCategory>( rc, 1 ) )
		{
			asRadarValues.push_back( ssprintf("%.6f", rv[rc]) );
		}
	}
	lines.push_back( ssprintf( "     %s:", join(",",asRadarValues).c_str() ) );

	RString sNoteData;
	in.GetSMNoteData( sNoteData );

	split( sNoteData, "\n", lines, true );
	lines.push_back( ";" );

	return JoinLineList( lines );
}

bool NotesWriterSM::Write( RageFile &f, const Song &out, const vector<Steps*>& vpStepsToSave )
{
	WriteGlobalTags( f, out );

	for (Steps const *pSteps : vpStepsToSave)
	{
		RString sTag = GetSMNotesTag( out, *pSteps );
		f.PutLine( sTag );
	}
	if( f.Flush() == -1 )
		return false;

	return true;
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
