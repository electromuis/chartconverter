#include "global.h"
#include "NotesLoaderSM.h"
#include "NoteTypes.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "Steps.h"
#include "MsdFile.h"
#include "GameManager.h"
#include "RageFile.h"

// Everything from this line to the creation of sm_parser_helper exists to
// speed up parsing by allowing the use of std::map.  All these functions
// are put into a map of function pointers which is used when loading.
// -Kyz
/****************************************************************/
struct SMSongTagInfo
{
	SMLoader* loader;
	Song* song;
	const MsdFile::value_t* params;
	const RString& path;
	vector< pair<float, float> > BPMChanges, Stops;
	SMSongTagInfo(SMLoader* l, Song* s, const RString& p)
		:loader(l), song(s), path(p)
	{}
};

typedef void (*song_tag_func_t)(SMSongTagInfo& info);

// Functions for song tags go below this line. -Kyz
/****************************************************************/
void SMSetTitle(SMSongTagInfo& info)
{
	info.song->m_sMainTitle = (*info.params)[1];
	info.loader->SetSongTitle((*info.params)[1]);
}
void SMSetSubtitle(SMSongTagInfo& info)
{
	info.song->m_sSubTitle = (*info.params)[1];
}
void SMSetArtist(SMSongTagInfo& info)
{
	info.song->m_sArtist = (*info.params)[1];
}
void SMSetTitleTranslit(SMSongTagInfo& info)
{
	info.song->m_sMainTitleTranslit = (*info.params)[1];
}
void SMSetSubtitleTranslit(SMSongTagInfo& info)
{
	info.song->m_sSubTitleTranslit = (*info.params)[1];
}
void SMSetArtistTranslit(SMSongTagInfo& info)
{
	info.song->m_sArtistTranslit = (*info.params)[1];
}
void SMSetGenre(SMSongTagInfo& info)
{
	info.song->m_sGenre = (*info.params)[1];
}
void SMSetCredit(SMSongTagInfo& info)
{
	info.song->m_sCredit = (*info.params)[1];
}
void SMSetBanner(SMSongTagInfo& info)
{
	info.song->m_sBannerFile = (*info.params)[1];
}
void SMSetBackground(SMSongTagInfo& info)
{
	info.song->m_sBackgroundFile = (*info.params)[1];
}
void SMSetCDTitle(SMSongTagInfo& info)
{
	info.song->m_sCDTitleFile = (*info.params)[1];
}
void SMSetMusic(SMSongTagInfo& info)
{
	info.song->m_sMusicFile = (*info.params)[1];
}
void SMSetOffset(SMSongTagInfo& info)
{
	info.song->m_SongTiming.m_fBeat0OffsetInSeconds = StringToFloat((*info.params)[1]);
}
void SMSetBPMs(SMSongTagInfo& info)
{
	info.loader->ParseBPMs(info.BPMChanges, (*info.params)[1]);
}
void SMSetStops(SMSongTagInfo& info)
{
	info.Stops.clear();
	info.loader->ParseStops(info.Stops, (*info.params)[1]);
}
void SMSetDelays(SMSongTagInfo& info)
{
	info.loader->ProcessDelays(info.song->m_SongTiming, (*info.params)[1]);
}
void SMSetTimeSignatures(SMSongTagInfo& info)
{
	info.loader->ProcessTimeSignatures(info.song->m_SongTiming, (*info.params)[1]);
}
void SMSetTickCounts(SMSongTagInfo& info)
{
	info.loader->ProcessTickcounts(info.song->m_SongTiming, (*info.params)[1]);
}
void SMSetSampleStart(SMSongTagInfo& info)
{
	info.song->m_fMusicSampleStartSeconds = HHMMSSToSeconds((*info.params)[1]);
}
void SMSetSampleLength(SMSongTagInfo& info)
{
	info.song->m_fMusicSampleLengthSeconds = HHMMSSToSeconds((*info.params)[1]);
}
void SMSetDisplayBPM(SMSongTagInfo& info)
{
	// #DISPLAYBPM:[xxx][xxx:xxx]|[*];
	if((*info.params)[1] == "*")
	{ info.song->m_DisplayBPMType = DISPLAY_BPM_RANDOM; }
	else
	{
		info.song->m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;
		info.song->m_fSpecifiedBPMMin = StringToFloat((*info.params)[1]);
		if((*info.params)[2].empty())
		{ info.song->m_fSpecifiedBPMMax = info.song->m_fSpecifiedBPMMin; }
		else
		{ info.song->m_fSpecifiedBPMMax = StringToFloat((*info.params)[2]); }
	}
}
void SMSetSelectable(SMSongTagInfo& info)
{
	if((*info.params)[1].EqualsNoCase("YES"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	else if((*info.params)[1].EqualsNoCase("NO"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_NEVER; }
	// ROULETTE from 3.9. It was removed since UnlockManager can serve
	// the same purpose somehow. This, of course, assumes you're using
	// unlocks. -aj
	else if((*info.params)[1].EqualsNoCase("ROULETTE"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	/* The following two cases are just fixes to make sure simfiles that
	 * used 3.9+ features are not excluded here */
	else if((*info.params)[1].EqualsNoCase("ES") || (*info.params)[1].EqualsNoCase("OMES"))
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	else if(StringToInt((*info.params)[1]) > 0)
	{ info.song->m_SelectionDisplay = info.song->SHOW_ALWAYS; }
	else
	{ LOG->UserLog("Song file", info.path, "has an unknown #SELECTABLE value, \"%s\"; ignored.", (*info.params)[1].c_str()); }
}

typedef std::map<RString, song_tag_func_t> song_handler_map_t;

struct sm_parser_helper_t
{
	song_handler_map_t song_tag_handlers;
	// Unless signed, the comments in this tag list are not by me.  They were
	// moved here when converting from the else if chain. -Kyz
	sm_parser_helper_t()
	{
		song_tag_handlers["TITLE"]= &SMSetTitle;
		song_tag_handlers["SUBTITLE"]= &SMSetSubtitle;
		song_tag_handlers["ARTIST"]= &SMSetArtist;
		song_tag_handlers["TITLETRANSLIT"]= &SMSetTitleTranslit;
		song_tag_handlers["SUBTITLETRANSLIT"]= &SMSetSubtitleTranslit;
		song_tag_handlers["ARTISTTRANSLIT"]= &SMSetArtistTranslit;
		song_tag_handlers["GENRE"]= &SMSetGenre;
		song_tag_handlers["CREDIT"]= &SMSetCredit;
		song_tag_handlers["BANNER"]= &SMSetBanner;
		song_tag_handlers["BACKGROUND"]= &SMSetBackground;
		song_tag_handlers["CDTITLE"]= &SMSetCDTitle;
		song_tag_handlers["MUSIC"]= &SMSetMusic;
		song_tag_handlers["OFFSET"]= &SMSetOffset;
		song_tag_handlers["BPMS"]= &SMSetBPMs;
		song_tag_handlers["STOPS"]= &SMSetStops;
		song_tag_handlers["FREEZES"]= &SMSetStops;
		song_tag_handlers["DELAYS"]= &SMSetDelays;
		song_tag_handlers["TIMESIGNATURES"]= &SMSetTimeSignatures;
		song_tag_handlers["TICKCOUNTS"]= &SMSetTickCounts;
		song_tag_handlers["SAMPLESTART"]= &SMSetSampleStart;
		song_tag_handlers["SAMPLELENGTH"]= &SMSetSampleLength;
		song_tag_handlers["DISPLAYBPM"]= &SMSetDisplayBPM;
		song_tag_handlers["SELECTABLE"]= &SMSetSelectable;
	}
};
sm_parser_helper_t sm_parser_helper;
// End sm_parser_helper related functions. -Kyz
/****************************************************************/

void SMLoader::SetSongTitle(const RString & title)
{
	this->songTitle = title;
}

RString SMLoader::GetSongTitle() const
{
	return this->songTitle;
}

float SMLoader::RowToBeat( RString line, const int rowsPerBeat )
{
	RString backup = line;
	Trim(line, "r");
	Trim(line, "R");
	if( backup != line )
	{
		return StringToFloat( line ) / rowsPerBeat;
	}
	else
	{
		return StringToFloat( line );
	}
}

void SMLoader::LoadFromTokens( 
			     RString sStepsType, 
			     RString sDescription,
			     RString sDifficulty,
			     RString sMeter,
			     RString sRadarValues,
			     RString sNoteData,
			     Steps &out
			     )
{

	Trim( sStepsType );
	Trim( sDescription );
	Trim( sDifficulty );
	Trim( sNoteData );

	// LOG->Trace( "Steps::LoadFromTokens(), %s", sStepsType.c_str() );

	// backwards compatibility hacks:
	// HACK: We eliminated "ez2-single-hard", but we should still handle it.
	if( sStepsType == "ez2-single-hard" )
		sStepsType = "ez2-single";

	// HACK: "para-single" used to be called just "para"
	if( sStepsType == "para" )
		sStepsType = "para-single";

	out.m_StepsType = GAMEMAN->StringToStepsType( sStepsType );
	out.m_StepsTypeStr = sStepsType;
	out.SetDescription( sDescription );
	out.SetCredit( sDescription ); // this is often used for both.
	out.SetChartName(sDescription); // yeah, one more for good measure.
	out.SetDifficulty( OldStyleStringToDifficulty(sDifficulty) );

	// Handle hacks that originated back when StepMania didn't have
	// Difficulty_Challenge. (At least v1.64, possibly v3.0 final...)
	if( out.GetDifficulty() == Difficulty_Hard )
	{
		// HACK: SMANIAC used to be Difficulty_Hard with a special description.
		if( sDescription.CompareNoCase("smaniac") == 0 ) 
			out.SetDifficulty( Difficulty_Challenge );

		// HACK: CHALLENGE used to be Difficulty_Hard with a special description.
		if( sDescription.CompareNoCase("challenge") == 0 ) 
			out.SetDifficulty( Difficulty_Challenge );
	}

	if( sMeter.empty() )
	{
		// some simfiles (e.g. X-SPECIALs from Zenius-I-Vanisher) don't
		// have a meter on certain steps. Make the meter 1 in these instances.
		sMeter = "1";
	}
	out.SetMeter( StringToInt(sMeter) );

	out.SetSMNoteData( sNoteData );

	out.TidyUpData();
}

void SMLoader::ParseBPMs( vector< pair<float, float> > &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayBPMChangeExpressions;
	split( line, ",", arrayBPMChangeExpressions );

	for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
	{
		vector<RString> arrayBPMChangeValues;
		split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
		if( arrayBPMChangeValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #BPMs value \"%s\" (must have exactly one '='), ignored.",
				     arrayBPMChangeExpressions[b].c_str() );
			continue;
		}

		const float fBeat = RowToBeat( arrayBPMChangeValues[0], rowsPerBeat );
		const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );
		if( fNewBPM == 0 ) {
			LOG->UserLog("Song file", this->GetSongTitle(),
				     "has a zero BPM; ignored.");
			continue;
		}

		out.push_back( make_pair(fBeat, fNewBPM) );
	}
}

void SMLoader::ParseStops( vector< pair<float, float> > &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayFreezeExpressions;
	split( line, ",", arrayFreezeExpressions );
	
	for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
	{
		vector<RString> arrayFreezeValues;
		split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
		if( arrayFreezeValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #STOPS value \"%s\" (must have exactly one '='), ignored.",
				     arrayFreezeExpressions[f].c_str() );
			continue;
		}

		const float fFreezeBeat = RowToBeat( arrayFreezeValues[0], rowsPerBeat );
		const float fFreezeSeconds = StringToFloat( arrayFreezeValues[1] );
		if( fFreezeSeconds == 0 ) {
			LOG->UserLog("Song file", this->GetSongTitle(),
				     "has a zero-length stop; ignored.");
			continue;
		}

		out.push_back( make_pair(fFreezeBeat, fFreezeSeconds) );
	}
}

// Utility function for sorting timing change data
namespace {
	bool compare_first(pair<float, float> a, pair<float, float> b) {
		return a.first < b.first;
	}
}

// Precondition: no BPM change or stop has 0 for its value (change.second).
//     (The ParseBPMs and ParseStops functions make sure of this.)
// Postcondition: all BPM changes, stops, and warps are added to the out
//     parameter, already sorted by beat.
void SMLoader::ProcessBPMsAndStops(TimingData &out,
		vector< pair<float, float> > &vBPMs,
		vector< pair<float, float> > &vStops)
{
	vector< pair<float, float> >::const_iterator ibpm, ibpmend;
	vector< pair<float, float> >::const_iterator istop, istopend;

	// Current BPM (positive or negative)
	float bpm = 0;
	// Beat at which the previous timing change occurred
	float prevbeat = 0;
	// Start/end of current warp (-1 if not currently warping)
	float warpstart = -1;
	float warpend = -1;
	// BPM prior to current warp, to detect if it has changed
	float prewarpbpm = 0;
	// How far off we have gotten due to negative changes
	float timeofs = 0;

	// Sort BPM changes and stops by beat.  Order matters.
	// TODO: Make sorted lists a precondition rather than sorting them here.
	// The caller may know that the lists are sorted already (e.g. if
	// loaded from cache).
	stable_sort(vBPMs.begin(), vBPMs.end(), compare_first);
	stable_sort(vStops.begin(), vStops.end(), compare_first);

	// Convert stops that come before beat 0.  All these really do is affect
	// where the arrows are with respect to the music, i.e. the song offset.
	// Positive stops subtract from the offset, and negative add to it.
	istop = vStops.begin();
	istopend = vStops.end();
	for (/* istop */; istop != istopend && istop->first < 0; istop++)
	{
		out.m_fBeat0OffsetInSeconds -= istop->second;
	}

	// Get rid of BPM changes that come before beat 0.  Positive BPMs before
	// the chart don't really do anything, so we just ignore them.  Negative
	// BPMs cause unpredictable behavior, so ignore them as well and issue a
	// warning.
	ibpm = vBPMs.begin();
	ibpmend = vBPMs.end();
	for (/* ibpm */; ibpm != ibpmend && ibpm->first <= 0; ibpm++)
	{
		bpm = ibpm->second;
		if (bpm < 0 && ibpm->first < 0)
		{
			LOG->UserLog("Song file", this->GetSongTitle(),
					"has a negative BPM prior to beat 0.  "
					"These cause problems; ignoring.");
		}
	}

	// It's beat 0.  Do you know where your BPMs are?
	if (bpm == 0)
	{
		// Nope.  Can we just use the next BPM value?
		if (ibpm == ibpmend)
		{
			// Nope.
			bpm = 60;
			LOG->UserLog("Song file", this->GetSongTitle(),
					"has no valid BPMs.  Defaulting to 60.");
		}
		else
		{
			// Yep.  Get the next BPM.
			ibpm++;
			bpm = ibpm->second;
			LOG->UserLog("Song file", this->GetSongTitle(),
					"does not establish a BPM before beat 0.  "
					"Using the value from the next BPM change.");
		}
	}
	// We always want to have an initial BPM.  If we start out warping, this
	// BPM will be added later.  If we start with a regular BPM, add it now.
	if (bpm > 0 && bpm <= FAST_BPM_WARP)
	{
		out.AddSegment(BPMSegment(BeatToNoteRow(0), bpm));
	}

	// Iterate over all BPMs and stops in tandem
	while (ibpm != ibpmend || istop != istopend)
	{
		// Get the next change in order, with BPMs taking precedence
		// when they fall on the same beat.
		bool changeIsBpm = istop == istopend || (ibpm != ibpmend && ibpm->first <= istop->first);
		const pair<float, float> & change = changeIsBpm ? *ibpm : *istop;

		// Calculate the effects of time at the current BPM.  "Infinite"
		// BPMs (SM4 warps) imply that zero time passes, so skip this
		// step in that case.
		if (bpm <= FAST_BPM_WARP)
		{
			timeofs += (change.first - prevbeat) * 60/bpm;

			// If we were in a warp and it finished during this
			// timeframe, create the warp segment.
			if (warpstart >= 0 && bpm > 0 && timeofs > 0)
			{
				// timeofs represents how far past the end we are
				warpend = change.first - (timeofs * bpm/60);
				out.AddSegment(WarpSegment(BeatToNoteRow(warpstart),
							warpend - warpstart));

				// If the BPM changed during the warp, put that
				// change at the beginning of the warp.
				if (bpm != prewarpbpm)
				{
					out.AddSegment(BPMSegment(BeatToNoteRow(warpstart), bpm));
				}
				// No longer warping
				warpstart = -1;
			}
		}

		// Save the current beat for the next round of calculations
		prevbeat = change.first;

		// Now handle the timing changes themselves
		if (changeIsBpm)
		{
			// Does this BPM change start a new warp?
			if (warpstart < 0 && (change.second < 0 || change.second > FAST_BPM_WARP))
			{
				// Yes.
				warpstart = change.first;
				prewarpbpm = bpm;
				timeofs = 0;
			}
			else if (warpstart < 0)
			{
				// No, and we aren't currently warping either.
				// Just a normal BPM change.
				out.AddSegment(BPMSegment(BeatToNoteRow(change.first), change.second));
			}
			bpm = change.second;
			ibpm++;
		}
		else
		{
			// Does this stop start a new warp?
			if (warpstart < 0 && change.second < 0)
			{
				// Yes.
				warpstart = change.first;
				prewarpbpm = bpm;
				timeofs = change.second;
			}
			else if (warpstart < 0)
			{
				// No, and we aren't currently warping either.
				// Just a normal stop.
				out.AddSegment(StopSegment(BeatToNoteRow(change.first), change.second));
			}
			else
			{
				// We're warping already.  Stops affect the time
				// offset directly.
				timeofs += change.second;

				// If a stop overcompensates for the time
				// deficit, the warp ends and we stop for the
				// amount it goes over.
				if (change.second > 0 && timeofs > 0)
				{
					warpend = change.first;
					out.AddSegment(WarpSegment(BeatToNoteRow(warpstart),
								warpend - warpstart));
					out.AddSegment(StopSegment(BeatToNoteRow(change.first), timeofs));

					// Now, are we still warping because of
					// the BPM value?
					if (bpm < 0 || bpm > FAST_BPM_WARP)
					{
						// Yep.
						warpstart = change.first;
						// prewarpbpm remains the same
						timeofs = 0;
					}
					else
					{
						// Nope, warp is done.  Add any
						// BPM change that happened in
						// the meantime.
						if (bpm != prewarpbpm)
						{
							out.AddSegment(BPMSegment(BeatToNoteRow(warpstart), bpm));
						}
						warpstart = -1;
					}
				}
			}
			istop++;
		}
	}

	// If we are still warping, we now have to consider the time remaining
	// after the last timing change.
	if (warpstart >= 0)
	{
		// Will this warp ever end?
		if (bpm < 0 || bpm > FAST_BPM_WARP)
		{
			// No, so it ends the entire chart immediately.
			// XXX There must be a less hacky and more accurate way
			// to do this.
			warpend = 99999999.0f;
		}
		else
		{
			// Yes.  Figure out when it will end.
			warpend = prevbeat - (timeofs * bpm/60);
		}
		out.AddSegment(WarpSegment(BeatToNoteRow(warpstart),
					warpend - warpstart));

		// As usual, record any BPM change that happened during the warp
		if (bpm != prewarpbpm)
		{
			out.AddSegment(BPMSegment(BeatToNoteRow(warpstart), bpm));
		}
	}
}

void SMLoader::ProcessDelays( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayDelayExpressions;
	split( line, ",", arrayDelayExpressions );

	for( unsigned f=0; f<arrayDelayExpressions.size(); f++ )
	{
		vector<RString> arrayDelayValues;
		split( arrayDelayExpressions[f], "=", arrayDelayValues );
		if( arrayDelayValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #DELAYS value \"%s\" (must have exactly one '='), ignored.",
				     arrayDelayExpressions[f].c_str() );
			continue;
		}
		const float fFreezeBeat = RowToBeat( arrayDelayValues[0], rowsPerBeat );
		const float fFreezeSeconds = StringToFloat( arrayDelayValues[1] );
		// LOG->Trace( "Adding a delay segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );

		if(fFreezeSeconds > 0.0f)
			out.AddSegment( DelaySegment(BeatToNoteRow(fFreezeBeat), fFreezeSeconds) );
		else
			LOG->UserLog(
				     "Song file",
				     this->GetSongTitle(),
				     "has an invalid delay at beat %f, length %f.",
				     fFreezeBeat, fFreezeSeconds );
	}
}

void SMLoader::ProcessTimeSignatures( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> vs1;
	split( line, ",", vs1 );

	for (RString const &s1 : vs1)
	{
		vector<RString> vs2;
		split( s1, "=", vs2 );

		if( vs2.size() < 3 )
		{
			LOG->UserLog("Song file",
				GetSongTitle(),
				"has an invalid time signature change with %i values.",
				static_cast<int>(vs2.size()) );
			continue;
		}

		const float fBeat = RowToBeat( vs2[0], rowsPerBeat );
		const int iNumerator = StringToInt( vs2[1] );
		const int iDenominator = StringToInt( vs2[2] );

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

		if( iDenominator < 1 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid time signature change with beat %f, iDenominator %i.",
				     fBeat, iDenominator );
			continue;
		}

		out.AddSegment( TimeSignatureSegment(BeatToNoteRow(fBeat), iNumerator, iDenominator) );
	}
}

void SMLoader::ProcessTickcounts( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayTickcountExpressions;
	split( line, ",", arrayTickcountExpressions );

	for( unsigned f=0; f<arrayTickcountExpressions.size(); f++ )
	{
		vector<RString> arrayTickcountValues;
		split( arrayTickcountExpressions[f], "=", arrayTickcountValues );
		if( arrayTickcountValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #TICKCOUNTS value \"%s\" (must have exactly one '='), ignored.",
				     arrayTickcountExpressions[f].c_str() );
			continue;
		}

		const float fTickcountBeat = RowToBeat( arrayTickcountValues[0], rowsPerBeat );
		int iTicks = clamp(atoi( arrayTickcountValues[1] ), 0, ROWS_PER_BEAT);

		out.AddSegment( TickcountSegment(BeatToNoteRow(fTickcountBeat), iTicks) );
	}
}

void SMLoader::ProcessSpeeds( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> vs1;
	split( line, ",", vs1 );

	for (RString const &s1 : vs1)
	{
		vector<RString> vs2;
		split( s1, "=", vs2 );

		if( vs2[0] == 0 && vs2.size() == 2 ) // First one always seems to have 2.
		{
			vs2.push_back("0");
		}

		if( vs2.size() == 3 ) // use beats by default.
		{
			vs2.push_back("0");
		}

		if( vs2.size() < 4 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an speed change with %i values.",
				     static_cast<int>(vs2.size()) );
			continue;
		}

		const float fBeat = RowToBeat( vs2[0], rowsPerBeat );
		const float fRatio = StringToFloat( vs2[1] );
		const float fDelay = StringToFloat( vs2[2] );

		// XXX: ugly...
		int iUnit = StringToInt(vs2[3]);
		SpeedSegment::BaseUnit unit = (iUnit == 0) ?
			SpeedSegment::UNIT_BEATS : SpeedSegment::UNIT_SECONDS;

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

void SMLoader::ProcessFakes( TimingData &out, const RString line, const int rowsPerBeat )
{
	vector<RString> arrayFakeExpressions;
	split( line, ",", arrayFakeExpressions );

	for( unsigned b=0; b<arrayFakeExpressions.size(); b++ )
	{
		vector<RString> arrayFakeValues;
		split( arrayFakeExpressions[b], "=", arrayFakeValues );
		if( arrayFakeValues.size() != 2 )
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid #FAKES value \"%s\" (must have exactly one '='), ignored.",
				     arrayFakeExpressions[b].c_str() );
			continue;
		}

		const float fBeat = RowToBeat( arrayFakeValues[0], rowsPerBeat );
		const float fSkippedBeats = StringToFloat( arrayFakeValues[1] );

		if(fSkippedBeats > 0)
			out.AddSegment( FakeSegment(BeatToNoteRow(fBeat), fSkippedBeats) );
		else
		{
			LOG->UserLog("Song file",
				     this->GetSongTitle(),
				     "has an invalid Fake at beat %f, beats to skip %f.",
				     fBeat, fSkippedBeats );
		}
	}
}

bool SMLoader::LoadNoteDataFromSimfile( RageFile& f, Steps &out )
{
	RString path = f.GetPath();

	MsdFile msd;
	if( !msd.ReadFile( f, true ) )  // unescape
	{
		LOG->UserLog( "Song file", path, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}
	
	for (unsigned i = 0; i<msd.GetNumValues(); i++)
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();
		
		// The only tag we care about is the #NOTES tag.
		if( sValueName=="NOTES" || sValueName=="NOTES2" )
		{
			if( iNumParams < 7 )
			{
				LOG->UserLog("Song file",
					     path,
					     "has %d fields in a #NOTES tag, but should have at least 7.",
					     iNumParams );
				continue;
			}
			
			RString stepsType = sParams[1];
			RString description = sParams[2];
			RString difficulty = sParams[3];

			// HACK?: If this is a .edit fudge the edit difficulty
			if(path.Right(5).CompareNoCase(".edit") == 0) difficulty = "edit";

			Trim(stepsType);
			Trim(description);
			Trim(difficulty);
			// Remember our old versions.
			if (difficulty.CompareNoCase("smaniac") == 0)
			{
				difficulty = "Challenge";
			}
			
			/* Handle hacks that originated back when StepMania didn't have
			 * Difficulty_Challenge. TODO: Remove the need for said hacks. */
			if( difficulty.CompareNoCase("hard") == 0 )
			{
				/* HACK: Both SMANIAC and CHALLENGE used to be Difficulty_Hard.
				 * They were differentiated via aspecial description.
				 * Account for the rogue charts that do this. */
				// HACK: SMANIAC used to be Difficulty_Hard with a special description.
				if (description.CompareNoCase("smaniac") == 0 ||
					description.CompareNoCase("challenge") == 0) 
					difficulty = "Challenge";
			}
			
			if(!(out.m_StepsType == GAMEMAN->StringToStepsType( stepsType ) &&
			     out.GetDescription() == description &&
			     (out.GetDifficulty() == StringToDifficulty(difficulty) ||
				  out.GetDifficulty() == OldStyleStringToDifficulty(difficulty))))
			{
				continue;
			}
			
			RString noteData = sParams[6];
			Trim( noteData );
			out.SetSMNoteData( noteData );
			out.TidyUpData();
			return true;
		}
	}
	return false;
}

bool SMLoader::LoadFromSimfile( RageFile& f, Song &out )
{
	//LOG->Trace( "Song::LoadFromSMFile(%s)", sPath.c_str() );

	RString sPath = f.GetPath();

	MsdFile msd;
	if( !msd.ReadFile( f, true ) )  // unescape
	{
		LOG->UserLog( "Song file", sPath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}

	out.m_SongTiming.m_sFile = sPath;
	out.m_sSongFileName = sPath;

	SMSongTagInfo reused_song_info(&*this, &out, sPath);

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();

		reused_song_info.params= &sParams;
		song_handler_map_t::iterator handler=
			sm_parser_helper.song_tag_handlers.find(sValueName);
		if(handler != sm_parser_helper.song_tag_handlers.end())
		{
		/* Don't use GetMainAndSubTitlesFromFullTitle; that's only for heuristically
		 * splitting other formats that *don't* natively support #SUBTITLE. */
			handler->second(reused_song_info);
		}
		else if(sValueName == "NOTES" || sValueName == "NOTES2")
		{
			if(iNumParams < 7)
			{
				LOG->UserLog( "Song file", sPath, "has %d fields in a #NOTES tag, but should have at least 7.", iNumParams );
				continue;
			}

			Steps* pNewNotes = out.CreateSteps();
			LoadFromTokens( 
				sParams[1], 
				sParams[2], 
				sParams[3], 
				sParams[4], 
				sParams[5], 
				sParams[6],
				*pNewNotes);

			pNewNotes->SetFilename(sPath);
			out.AddSteps( pNewNotes );
		}
		else if(sValueName == "LYRICSPATH" || sValueName == "BGCHANGES" || sValueName == "KEYSOUNDS")
		{
			;
		}
		else
		{
			LOG->UserLog("Song file", sPath, "has an unexpected value named \"%s\".", sValueName.c_str());
		}
	}

	// Turn negative time changes into warps
	ProcessBPMsAndStops(out.m_SongTiming, reused_song_info.BPMChanges, reused_song_info.Stops);

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
