#include "global.h"
#include "NoteDataUtil.h"
#include "NoteData.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "Song.h"
#include "RadarValues.h"
#include "TimingData.h"
#include <utility>

// TODO: Remove these constants that aren't time signature-aware
static const int BEATS_PER_MEASURE = 4;
static const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;

NoteType NoteDataUtil::GetSmallestNoteTypeForMeasure( const NoteData &nd, int iMeasureIndex )
{
	const int iMeasureStartIndex = iMeasureIndex * ROWS_PER_MEASURE;
	const int iMeasureEndIndex = (iMeasureIndex+1) * ROWS_PER_MEASURE;

	return NoteDataUtil::GetSmallestNoteTypeInRange( nd, iMeasureStartIndex, iMeasureEndIndex );
}

NoteType NoteDataUtil::GetSmallestNoteTypeInRange( const NoteData &n, int iStartIndex, int iEndIndex )
{
	// probe to find the smallest note type
	FOREACH_ENUM(NoteType, nt)
	{
		float fBeatSpacing = NoteTypeToBeat( nt );
		int iRowSpacing = lrintf( fBeatSpacing * ROWS_PER_BEAT );

		bool bFoundSmallerNote = false;
		// for each index in this measure
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( n, i, iStartIndex, iEndIndex )
		{
			if( i % iRowSpacing == 0 )
				continue;	// skip
			
			if( !n.IsRowEmpty(i) )
			{
				bFoundSmallerNote = true;
				break;
			}
		}

		if( bFoundSmallerNote )
			continue;	// searching the next NoteType
		else
			return nt;	// stop searching. We found the smallest NoteType
	}
	return NoteType_Invalid;	// well-formed notes created in the editor should never get here
}

static void LoadFromSMNoteDataStringWithPlayer( NoteData& out, const RString &sSMNoteData, int start,
						int len, PlayerNumber pn, int iNumTracks )
{
	/* Don't allocate memory for the entire string, nor per measure. Instead, use the in-place
	 * partial string split twice. By maintaining begin and end pointers to each measure line
	 * we can perform this without copying the string at all. */
	int size = -1;
	const int end = start + len;
	vector<pair<const char *, const char *> > aMeasureLines;
	for( unsigned m = 0; true; ++m )
	{
		/* XXX Ignoring empty seems wrong for measures. It means that ",,," is treated as
		 * "," where I would expect most people would want 2 empty measures. ",\n,\n,"
		 * would do as I would expect. */
		split( sSMNoteData, ",", start, size, end, true ); // Ignore empty is important.
		if( start == end )
		{
			break;
		}
		// Partial string split.
		int measureLineStart = start, measureLineSize = -1;
		const int measureEnd = start + size;

		aMeasureLines.clear();
		for(;;)
		{
			// Ignore empty is clearly important here.
			split( sSMNoteData, "\n", measureLineStart, measureLineSize, measureEnd, true );
			if( measureLineStart == measureEnd )
			{
				break;
			}
			//RString &line = sSMNoteData.substr( measureLineStart, measureLineSize );
			const char *beginLine = sSMNoteData.data() + measureLineStart;
			const char *endLine = beginLine + measureLineSize;

			while( beginLine < endLine && strchr("\r\n\t ", *beginLine) )
				++beginLine;
			while( endLine > beginLine && strchr("\r\n\t ", *(endLine - 1)) )
				--endLine;
			if( beginLine < endLine ) // nonempty
				aMeasureLines.push_back( pair<const char *, const char *>(beginLine, endLine) );
		}

		for( unsigned l=0; l<aMeasureLines.size(); l++ )
		{
			const char *p = aMeasureLines[l].first;
			const char *const beginLine = p;
			const char *const endLine = aMeasureLines[l].second;

			const float fPercentIntoMeasure = l/(float)aMeasureLines.size();
			const float fBeat = (m + fPercentIntoMeasure) * BEATS_PER_MEASURE;
			const int iIndex = BeatToNoteRow( fBeat );

			int iTrack = 0;
			while( iTrack < iNumTracks && p < endLine )
			{
				TapNote tn;
				char ch = *p;

				switch( ch )
				{
				case '0': tn = TAP_EMPTY;				break;
				case '1': tn = TAP_ORIGINAL_TAP;			break;
				case '2':
				case '4':
				// case 'N': // minefield
					tn = ch == '2' ? TAP_ORIGINAL_HOLD_HEAD : TAP_ORIGINAL_ROLL_HEAD;
					/*
					// upcoming code for minefields -aj
					switch(ch)
					{
					case '2': tn = TAP_ORIGINAL_HOLD_HEAD; break;
					case '4': tn = TAP_ORIGINAL_ROLL_HEAD; break;
					case 'N': tn = TAP_ORIGINAL_MINE_HEAD; break;
					}
					*/

					/* Set the hold note to have infinite length. We'll clamp
					 * it when we hit the tail. */
					tn.iDuration = MAX_NOTE_ROW;
					break;
				case '3':
				{
					// This is the end of a hold. Search for the beginning.
					int iHeadRow;
					if( !out.IsHoldNoteAtRow( iTrack, iIndex, &iHeadRow ) )
					{
						int n = intptr_t(endLine) - intptr_t(beginLine);
						LOG->Warn( "Unmatched 3 in \"%.*s\"", n, beginLine );
					}
					else
					{
						out.FindTapNote( iTrack, iHeadRow )->second.iDuration = iIndex - iHeadRow;
					}

					// This won't write tn, but keep parsing normally anyway.
					break;
				}
				//				case 'm':
				// Don't be loose with the definition.  Use only 'M' since
				// that's what we've been writing to disk.  -Chris
				case 'M': tn = TAP_ORIGINAL_MINE;			break;
				// case 'A': tn = TAP_ORIGINAL_ATTACK;			break;
				case 'K': tn = TAP_ORIGINAL_AUTO_KEYSOUND;		break;
				case 'L': tn = TAP_ORIGINAL_LIFT;			break;
				case 'F': tn = TAP_ORIGINAL_FAKE;			break;
				// case 'I': tn = TAP_ORIGINAL_ITEM;			break;
				default: 
					/* Invalid data. We don't want to assert, since there might
					 * simply be invalid data in an .SM, and we don't want to die
					 * due to invalid data. We should probably check for this when
					 * we load SM data for the first time ... */
					// FAIL_M("Invalid data in SM");
					tn = TAP_EMPTY;
					break;
				}

				p++;
				// We won't scan past the end of the line so these are safe to do.
#if 0
				// look for optional attack info (e.g. "{tipsy,50% drunk:15.2}")
				if( *p == '{' )
				{
					p++;

					char szModifiers[256] = "";
					float fDurationSeconds = 0;
					if( sscanf( p, "%255[^:]:%f}", szModifiers, &fDurationSeconds ) == 2 )	// not fatal if this fails due to malformed data
					{
						tn.type = TapNoteType_Attack;
						tn.sAttackModifiers = szModifiers;
		 				tn.fAttackDurationSeconds = fDurationSeconds;
					}

					// skip past the '}'
					while( p < endLine )
					{
						if( *(p++) == '}' )
							break;
					}
				}
#endif

				// look for optional keysound index (e.g. "[123]")
				if( *p == '[' )
				{
					p++;
					int iKeysoundIndex = 0;
					if( 1 == sscanf( p, "%d]", &iKeysoundIndex ) )	// not fatal if this fails due to malformed data
		 				tn.iKeysoundIndex = iKeysoundIndex;

					// skip past the ']'
					while( p < endLine )
					{
						if( *(p++) == ']' )
							break;
					}
				}

#if 0
				// look for optional item name (e.g. "<potion>"),
				// where the name in the <> is a Lua function defined elsewhere
				// (Data/ItemTypes.lua, perhaps?) -aj
				if( *p == '<' )
				{
					p++;

					// skip past the '>'
					while( p < endLine )
					{
						if( *(p++) == '>' )
							break;
					}
				}
#endif

				/* Optimization: if we pass TAP_EMPTY, NoteData will do a search
				 * to remove anything in this position.  We know that there's nothing
				 * there, so avoid the search. */
				if( tn.type != TapNoteType_Empty && ch != '3' )
				{
					tn.pn = pn;
					out.SetTapNote( iTrack, iIndex, tn );
				}

				iTrack++;
			}
		}
	}

	// Make sure we don't have any hold notes that didn't find a tail.
	for( int t=0; t<out.GetNumTracks(); t++ )
	{
		NoteData::iterator begin = out.begin( t );
		NoteData::iterator lEnd = out.end( t );
		while( begin != lEnd )
		{
			NoteData::iterator next = Increment( begin );
			const TapNote &tn = begin->second;
			if( tn.type == TapNoteType_HoldHead && tn.iDuration == MAX_NOTE_ROW )
			{
				int iRow = begin->first;
				LOG->UserLog( "", "", "While loading .sm/.ssc note data, there was an unmatched 2 at beat %f", NoteRowToBeat(iRow) );
				out.RemoveTapNote( t, begin );
			}

			begin = next;
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::LoadFromSMNoteDataString( NoteData &out, const RString &sSMNoteData_, bool bComposite )
{
	// Load note data
	RString sSMNoteData;
	RString::size_type iIndexCommentStart = 0;
	RString::size_type iIndexCommentEnd = 0;
	RString::size_type origSize = sSMNoteData_.size();
	const char *p = sSMNoteData_.data();

	sSMNoteData.reserve( origSize );
	while( (iIndexCommentStart = sSMNoteData_.find("//", iIndexCommentEnd)) != RString::npos )
	{
		sSMNoteData.append( p, iIndexCommentStart - iIndexCommentEnd );
		p += iIndexCommentStart - iIndexCommentEnd;
		iIndexCommentEnd = sSMNoteData_.find( "\n", iIndexCommentStart );
		iIndexCommentEnd = (iIndexCommentEnd == RString::npos ? origSize : iIndexCommentEnd+1);
		p += iIndexCommentEnd - iIndexCommentStart;
	}
	sSMNoteData.append( p, origSize - iIndexCommentEnd );

	// Clear notes, but keep the same number of tracks.
	int iNumTracks = out.GetNumTracks();
	out.Init();
	out.SetNumTracks( iNumTracks );

	if( !bComposite )
	{
		LoadFromSMNoteDataStringWithPlayer( out, sSMNoteData, 0, sSMNoteData.size(),
						    PLAYER_INVALID, iNumTracks );
		return;
	}

	int start = 0, size = -1;

	vector<NoteData> vParts;
	FOREACH_PlayerNumber( pn )
	{
		// Split in place.
		split( sSMNoteData, "&", start, size, false );
		if( unsigned(start) == sSMNoteData.size() )
			break;
		vParts.push_back( NoteData() );
		NoteData &nd = vParts.back();

		nd.SetNumTracks( iNumTracks );
		LoadFromSMNoteDataStringWithPlayer( nd, sSMNoteData, start, size, pn, iNumTracks );
	}
	CombineCompositeNoteData( out, vParts );
	out.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::InsertHoldTails( NoteData &inout )
{
	for( int t=0; t < inout.GetNumTracks(); t++ )
	{
		NoteData::iterator begin = inout.begin(t), end = inout.end(t);

		for( ; begin != end; ++begin )
		{
			int iRow = begin->first;
			const TapNote &tn = begin->second;
			if( tn.type != TapNoteType_HoldHead )
				continue;

			TapNote tail = tn;
			tail.type = TapNoteType_HoldTail;

			/* If iDuration is 0, we'd end up overwriting the head with the tail
			 * (and invalidating our iterator). Empty hold notes aren't valid. */
			ASSERT( tn.iDuration != 0 );

			inout.SetTapNote( t, iRow + tn.iDuration, tail );
		}
	}
}

void NoteDataUtil::GetSMNoteDataString( const NoteData &in, RString &sRet )
{
	// Get note data
	vector<NoteData> parts;
	float fLastBeat = -1.0f;

	SplitCompositeNoteData( in, parts );

	for (NoteData &nd : parts)
	{
		InsertHoldTails( nd );
		fLastBeat = max( fLastBeat, nd.GetLastBeat() );
	}

	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	sRet = "";
	int partNum = 0;
	for (NoteData const &nd : parts)
	{
		if( partNum++ != 0 )
			sRet.append( "&\n" );
		for( int m = 0; m <= iLastMeasure; ++m ) // foreach measure
		{
			if( m )
				sRet.append( 1, ',' );
			sRet += ssprintf("  // measure %d\n", m);

			NoteType nt = GetSmallestNoteTypeForMeasure( nd, m );
			int iRowSpacing;
			if( nt == NoteType_Invalid )
				iRowSpacing = 1;
			else
				iRowSpacing = lrintf( NoteTypeToBeat(nt) * ROWS_PER_BEAT );
			// (verify first)
			// iRowSpacing = BeatToNoteRow( NoteTypeToBeat(nt) );

			const int iMeasureStartRow = m * ROWS_PER_MEASURE;
			const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

			for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
			{
				for( int t = 0; t < nd.GetNumTracks(); ++t )
				{
					const TapNote &tn = nd.GetTapNote(t, r);
					char c;
					switch( tn.type )
					{
					case TapNoteType_Empty:			c = '0'; break;
					case TapNoteType_Tap:			c = '1'; break;
					case TapNoteType_HoldHead:
						switch( tn.subType )
						{
						case TapNoteSubType_Hold:	c = '2'; break;
						case TapNoteSubType_Roll:	c = '4'; break;
						//case TapNoteSubType_Mine:	c = 'N'; break;
						default:
							FAIL_M(ssprintf("Invalid tap note subtype: %i", tn.subType));
						}
						break;
					case TapNoteType_HoldTail:		c = '3'; break;
					case TapNoteType_Mine:			c = 'M'; break;
					case TapNoteType_Attack:			c = 'A'; break;
					case TapNoteType_AutoKeysound:	c = 'K'; break;
					case TapNoteType_Lift:			c = 'L'; break;
					case TapNoteType_Fake:			c = 'F'; break;
					default: 
						c = '\0';
						FAIL_M(ssprintf("Invalid tap note type: %i", tn.type));
					}
					sRet.append( 1, c );

					if( tn.type == TapNoteType_Attack )
					{
						sRet.append( ssprintf("{%s:%.2f}", tn.sAttackModifiers.c_str(),
								      tn.fAttackDurationSeconds) );
					}
					// hey maybe if we have TapNoteType_Item we can do things here.
					if( tn.iKeysoundIndex >= 0 )
						sRet.append( ssprintf("[%d]",tn.iKeysoundIndex) );
				}

				sRet.append( 1, '\n' );
			}
		}
	}
}

void NoteDataUtil::SplitCompositeNoteData( const NoteData &in, vector<NoteData> &out )
{
	if( !in.IsComposite() )
	{
		out.push_back( in );
		return;
	}

	FOREACH_PlayerNumber( pn )
	{
		out.push_back( NoteData() );
		out.back().SetNumTracks( in.GetNumTracks() );
	}

	for( int t = 0; t < in.GetNumTracks(); ++t )
	{
		for( NoteData::const_iterator iter = in.begin(t); iter != in.end(t); ++iter )
		{
			int row = iter->first;
			TapNote tn = iter->second;
			unsigned index = 0;

			ASSERT_M( index < NUM_PlayerNumber, ssprintf("We have a note not assigned to a player. The note in question is on beat %f, column %i.", NoteRowToBeat(row), t + 1) );
			tn.pn = PLAYER_INVALID;
			out[index].SetTapNote( t, row, tn );
		}
	}
}

void NoteDataUtil::CombineCompositeNoteData( NoteData &out, const vector<NoteData> &in )
{
	for (NoteData const &nd : in)
	{
		const int iMaxTracks = min( out.GetNumTracks(), nd.GetNumTracks() );

		for( int track = 0; track < iMaxTracks; ++track )
		{
			for( NoteData::const_iterator i = nd.begin(track); i != nd.end(track); ++i )
			{
				int row = i->first;
				if( out.IsHoldNoteAtRow(track, i->first) )
					continue;
				if( i->second.type == TapNoteType_HoldHead )
					out.AddHoldNote( track, row, row + i->second.iDuration, i->second );
				else
					out.SetTapNote( track, row, i->second );
			}
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}


void NoteDataUtil::LoadTransformedSlidingWindow( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	// reset all notes
	out.Init();
	
	if( in.GetNumTracks() > iNewNumTracks )
	{
		// Use a different algorithm for reducing tracks.
		LoadOverlapped( in, out, iNewNumTracks );
		return;
	}

	out.SetNumTracks( iNewNumTracks );

	if( in.GetNumTracks() == 0 )
		return;	// nothing to do and don't AV below

	int iCurTrackOffset = 0;
	int iTrackOffsetMin = 0;
	int iTrackOffsetMax = abs( iNewNumTracks - in.GetNumTracks() );
	int bOffsetIncreasing = true;

	int iLastMeasure = 0;
	int iMeasuresSinceChange = 0;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, r )
	{
		const int iMeasure = r / ROWS_PER_MEASURE;
		if( iMeasure != iLastMeasure )
			++iMeasuresSinceChange;

		if( iMeasure != iLastMeasure && iMeasuresSinceChange >= 4 ) // adjust sliding window every 4 measures at most
		{
			// See if there is a hold crossing the beginning of this measure
			bool bHoldCrossesThisMeasure = false;

			for( int t=0; t<in.GetNumTracks(); t++ )
			{
				if( in.IsHoldNoteAtRow( t, r-1 ) &&
				    in.IsHoldNoteAtRow( t, r ) )
				{
					bHoldCrossesThisMeasure = true;
					break;
				}
			}

			// adjust offset
			if( !bHoldCrossesThisMeasure )
			{
				iMeasuresSinceChange = 0;
				iCurTrackOffset += bOffsetIncreasing ? 1 : -1;
				if( iCurTrackOffset == iTrackOffsetMin  ||  iCurTrackOffset == iTrackOffsetMax )
					bOffsetIncreasing ^= true;
				CLAMP( iCurTrackOffset, iTrackOffsetMin, iTrackOffsetMax );
			}
		}

		iLastMeasure = iMeasure;

		// copy notes in this measure
		for( int t=0; t<in.GetNumTracks(); t++ )
		{
			int iOldTrack = t;
			int iNewTrack = (iOldTrack + iCurTrackOffset) % iNewNumTracks;
			TapNote tn = in.GetTapNote( iOldTrack, r );
			tn.pn= PLAYER_INVALID;
			out.SetTapNote( iNewTrack, r, tn );
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}

void PlaceAutoKeysound( NoteData &out, int row, TapNote akTap )
{
	int iEmptyTrack = -1;
	int iEmptyRow = row;
	int iNewNumTracks = out.GetNumTracks();
	bool bFoundEmptyTrack = false;
	int iRowsToLook[3] = {0, -1, 1};
	
	for( int j = 0; j < 3; j ++ )
	{
		int r = iRowsToLook[j] + row;
		if( r < 0 )
			continue;
		for( int i = 0; i < iNewNumTracks; ++i )
		{
			if ( out.GetTapNote(i, r) == TAP_EMPTY && !out.IsHoldNoteAtRow(i, r) )
			{
				iEmptyTrack = i;
				iEmptyRow = r;
				bFoundEmptyTrack = true;
				break;
			}
		}
		if( bFoundEmptyTrack )
			break;
	}
	
	if( iEmptyTrack != -1 )
	{
		akTap.type = TapNoteType_AutoKeysound;
		out.SetTapNote( iEmptyTrack, iEmptyRow, akTap );
	}
}

void NoteDataUtil::LoadOverlapped( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	out.SetNumTracks( iNewNumTracks );

	/* Keep track of the last source track that put a tap into each destination track,
	 * and the row of that tap. Then, if two rows are trying to put taps into the
	 * same row within the shift threshold, shift the newcomer source row. */
	int LastSourceTrack[MAX_NOTE_TRACKS];
	int LastSourceRow[MAX_NOTE_TRACKS];
	int DestRow[MAX_NOTE_TRACKS];

	for( int tr = 0; tr < MAX_NOTE_TRACKS; ++tr )
	{
		LastSourceTrack[tr] = -1;
		LastSourceRow[tr] = -MAX_NOTE_ROW;
		DestRow[tr] = tr;
		wrap( DestRow[tr], iNewNumTracks );
	}

	const int ShiftThreshold = BeatToNoteRow(1);

	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, row )
	{
		for( int iTrackFrom = 0; iTrackFrom < in.GetNumTracks(); ++iTrackFrom )
		{
			TapNote tnFrom = in.GetTapNote( iTrackFrom, row );
			if( tnFrom.type == TapNoteType_Empty || tnFrom.type == TapNoteType_AutoKeysound )
				continue;
			tnFrom.pn= PLAYER_INVALID;

			// If this is a hold note, find the end.
			int iEndIndex = row;
			if( tnFrom.type == TapNoteType_HoldHead )
				iEndIndex = row + tnFrom.iDuration;

			int &iTrackTo = DestRow[iTrackFrom];
			if( LastSourceTrack[iTrackTo] != iTrackFrom )
			{
				if( iEndIndex - LastSourceRow[iTrackTo] < ShiftThreshold )
				{
					/* This destination track is in use by a different source
					 * track. Use the least-recently-used track. */
					for( int DestTrack = 0; DestTrack < iNewNumTracks; ++DestTrack )
						if( LastSourceRow[DestTrack] < LastSourceRow[iTrackTo] )
							iTrackTo = DestTrack;
				}

				// If it's still in use, then we just don't have an available track.
				if( iEndIndex - LastSourceRow[iTrackTo] < ShiftThreshold )
				{
					// If it has a keysound, put it in autokeysound track.
					if( tnFrom.iKeysoundIndex >= 0 )
					{
						TapNote akTap = tnFrom;
						PlaceAutoKeysound( out, row, akTap );
					}
					continue;
				}
			}

			LastSourceTrack[iTrackTo] = iTrackFrom;
			LastSourceRow[iTrackTo] = iEndIndex;
			out.SetTapNote( iTrackTo, row, tnFrom );
			if( tnFrom.type == TapNoteType_HoldHead )
			{
				const TapNote &tnTail = in.GetTapNote( iTrackFrom, iEndIndex );
				out.SetTapNote( iTrackTo, iEndIndex, tnTail );
			}
		}
		
		// find empty track for autokeysounds in 2 next rows, so you can hear most autokeysounds
		for( int iTrackFrom = 0; iTrackFrom < in.GetNumTracks(); ++iTrackFrom )
		{
			const TapNote &tnFrom = in.GetTapNote( iTrackFrom, row );
			if( tnFrom.type != TapNoteType_AutoKeysound )
				continue;
			
			PlaceAutoKeysound( out, row, tnFrom );
		}
	}
	out.RevalidateATIs(vector<int>(), false);
}

int FindLongestOverlappingHoldNoteForAnyTrack( const NoteData &in, int iRow )
{
	int iMaxTailRow = -1;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		const TapNote &tn = in.GetTapNote( t, iRow );
		if( tn.type == TapNoteType_HoldHead )
			iMaxTailRow = max( iMaxTailRow, iRow + tn.iDuration );
	}

	return iMaxTailRow;
}

// For every row in "in" with a tap or hold on any track, enable the specified tracks in "out".
void LightTransformHelper( const NoteData &in, NoteData &out, const vector<int> &aiTracks )
{
	for( unsigned i = 0; i < aiTracks.size(); ++i )
		ASSERT_M( aiTracks[i] < out.GetNumTracks(), ssprintf("%i, %i", aiTracks[i], out.GetNumTracks()) );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS( in, r )
	{
		/* If any row starts a hold note, find the end of the hold note, and keep searching
		 * until we've extended to the end of the latest overlapping hold note. */
		int iHoldStart = r;
		int iHoldEnd = -1;
		for(;;)
		{
			int iMaxTailRow = FindLongestOverlappingHoldNoteForAnyTrack( in, r );
			if( iMaxTailRow == -1 )
			{
				break;
			}
			iHoldEnd = iMaxTailRow;
			r = iMaxTailRow;
		}

		if( iHoldEnd != -1 )
		{
			// If we found a hold note, add it to all tracks.
			for( unsigned i = 0; i < aiTracks.size(); ++i )
			{
				int t = aiTracks[i];
				out.AddHoldNote( t, iHoldStart, iHoldEnd, TAP_ORIGINAL_HOLD_HEAD );
			}
			continue;
		}

		if( in.IsRowEmpty(r) )
			continue;

		// Enable every track in the output.
		for( unsigned i = 0; i < aiTracks.size(); ++i )
		{
			int t = aiTracks[i];
			out.SetTapNote( t, r, TAP_ORIGINAL_TAP );
		}
	}
}

// For every track enabled in "in", enable all tracks in "out".
void NoteDataUtil::LoadTransformedLights( const NoteData &in, NoteData &out, int iNewNumTracks )
{
	// reset all notes
	out.Init();

	out.SetNumTracks( iNewNumTracks );

	vector<int> aiTracks;
	for( int i = 0; i < out.GetNumTracks(); ++i )
		aiTracks.push_back( i );

	LightTransformHelper( in, out, aiTracks );
}

struct recent_note
{
	int row;
	int track;
	recent_note()
		:row(0), track(0) {}
	recent_note(int r, int t)
		:row(r), track(t) {}
};

// CalculateRadarValues has to delay some stuff until a row ends, but can
// only detect a row ending when it hits the next note.  There isn't a note
// after the last row, so it also has to do the delayed stuff after exiting
// its loop.  So this state structure exists to be passed to a function that
// can be called from both places to do the work.  If this were Lua,
// DoRowEndRadarCalc would be a nested function. -Kyz
struct crv_state
{
	bool judgable;
	// hold_ends tracks where currently active holds will end, which is used
	// to count the number of hands. -Kyz
	vector<int> hold_ends;
	// num_holds_on_curr_row saves us the work of tracking where holds started
	// just to keep a jump of two holds from counting as a hand.
	int num_holds_on_curr_row;
	int num_notes_on_curr_row;

	crv_state()
		:judgable(false), num_holds_on_curr_row(0), num_notes_on_curr_row(0)
	{}
};

static void DoRowEndRadarCalc(crv_state& state, RadarValues& out)
{
	if(state.judgable)
	{
		if(state.num_notes_on_curr_row + (state.hold_ends.size() -
				state.num_holds_on_curr_row) >= 3)
		{
			++out[RadarCategory_Hands];
		}
	}
}

void NoteDataUtil::RemoveHoldNotes( NoteData &in, int iStartIndex, int iEndIndex )
{
	// turn all the HoldNotes into TapNotes
	for( int t=0; t<in.GetNumTracks(); ++t )
	{
		NoteData::TrackMap::iterator begin, end;
		in.GetTapNoteRangeInclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNoteType_HoldHead ||
				begin->second.subType != TapNoteSubType_Hold )
				continue;
			begin->second.type = TapNoteType_Tap;
		}
	}
	in.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::ChangeRollsToHolds( NoteData &in, int iStartIndex, int iEndIndex )
{
	for( int t=0; t<in.GetNumTracks(); ++t )
	{
		NoteData::TrackMap::iterator begin, end;
		in.GetTapNoteRangeInclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNoteType_HoldHead ||
				begin->second.subType != TapNoteSubType_Roll )
				continue;
			begin->second.subType = TapNoteSubType_Hold;
		}
	}
	in.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::ChangeHoldsToRolls( NoteData &in, int iStartIndex, int iEndIndex )
{
	for( int t=0; t<in.GetNumTracks(); ++t )
	{
		NoteData::TrackMap::iterator begin, end;
		in.GetTapNoteRangeInclusive( t, iStartIndex, iEndIndex, begin, end );
		for( ; begin != end; ++begin )
		{
			if( begin->second.type != TapNoteType_HoldHead ||
				begin->second.subType != TapNoteSubType_Hold )
				continue;
			begin->second.subType = TapNoteSubType_Roll;
		}
	}
	in.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveSimultaneousNotes( NoteData &in, int iMaxSimultaneous, int iStartIndex, int iEndIndex )
{
	// Remove tap and hold notes so no more than iMaxSimultaneous buttons are being held at any
	// given time.  Never touch data outside of the range given; if many hold notes are overlapping
	// iStartIndex, and we'd have to change those holds to obey iMaxSimultaneous, just do the best
	// we can without doing so.
	if( in.IsComposite() )
	{
		// Do this per part.
		vector<NoteData> vParts;
		
		SplitCompositeNoteData( in, vParts );
		for (NoteData &nd : vParts)
			RemoveSimultaneousNotes( nd, iMaxSimultaneous, iStartIndex, iEndIndex );
		in.Init();
		in.SetNumTracks( vParts.front().GetNumTracks() );
		CombineCompositeNoteData( in, vParts );
	}
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( in, r, iStartIndex, iEndIndex )
	{
		set<int> viTracksHeld;
		in.GetTracksHeldAtRow( r, viTracksHeld );

		// remove the first tap note or the first hold note that starts on this row
		int iTotalTracksPressed = in.GetNumTracksWithTapOrHoldHead(r) + viTracksHeld.size();
		int iTracksToRemove = max( 0, iTotalTracksPressed - iMaxSimultaneous );
		for( int t=0; iTracksToRemove>0 && t<in.GetNumTracks(); t++ )
		{
			const TapNote &tn = in.GetTapNote(t,r);
			if( tn.type == TapNoteType_Tap || tn.type == TapNoteType_HoldHead )
			{
				in.SetTapNote( t, r, TAP_EMPTY );
				iTracksToRemove--;
			}
		}
	}
	in.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveJumps( NoteData &inout, int iStartIndex, int iEndIndex )
{
	RemoveSimultaneousNotes( inout, 1, iStartIndex, iEndIndex );
}

void NoteDataUtil::RemoveHands( NoteData &inout, int iStartIndex, int iEndIndex )
{
	RemoveSimultaneousNotes( inout, 2, iStartIndex, iEndIndex );
}

void NoteDataUtil::RemoveQuads( NoteData &inout, int iStartIndex, int iEndIndex )
{
	RemoveSimultaneousNotes( inout, 3, iStartIndex, iEndIndex );
}

void NoteDataUtil::RemoveSpecificTapNotes(NoteData &inout, TapNoteType tn, int iStartIndex, int iEndIndex)
{
	for(int t=0; t<inout.GetNumTracks(); t++)
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE(inout, t, r, iStartIndex, iEndIndex)
		{
			if(inout.GetTapNote(t,r).type == tn)
			{
				inout.SetTapNote( t, r, TAP_EMPTY );
			}
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveMines(NoteData &inout, int iStartIndex, int iEndIndex)
{
	RemoveSpecificTapNotes(inout, TapNoteType_Mine, iStartIndex, iEndIndex);
}

void NoteDataUtil::RemoveLifts(NoteData &inout, int iStartIndex, int iEndIndex)
{
	RemoveSpecificTapNotes(inout, TapNoteType_Lift, iStartIndex, iEndIndex);
}

void NoteDataUtil::RemoveFakes(NoteData &inout, TimingData const& timing_data, int iStartIndex, int iEndIndex)
{
	RemoveSpecificTapNotes(inout, TapNoteType_Fake, iStartIndex, iEndIndex);
	for(int t=0; t<inout.GetNumTracks(); t++)
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE(inout, t, r, iStartIndex, iEndIndex)
		{
			if(!timing_data.IsJudgableAtRow(r))
			{
				inout.SetTapNote( t, r, TAP_EMPTY );
			}
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllButOneTap( NoteData &inout, int row )
{
	if(row < 0) return;

	int track;
	for( track = 0; track < inout.GetNumTracks(); ++track )
	{
		if( inout.GetTapNote(track, row).type == TapNoteType_Tap )
			break;
	}

	track++;

	for( ; track < inout.GetNumTracks(); ++track )
	{
		NoteData::iterator iter = inout.FindTapNote( track, row );
		if( iter != inout.end(track) && iter->second.type == TapNoteType_Tap )
			inout.RemoveTapNote( track, iter );
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllButPlayer( NoteData &inout, PlayerNumber pn )
{
	for( int track = 0; track < inout.GetNumTracks(); ++track )
	{
		NoteData::iterator i = inout.begin( track );
		
		while( i != inout.end(track) )
		{
			if( i->second.pn != pn && i->second.pn != PLAYER_INVALID )
				inout.RemoveTapNote( track, i++ );
			else
				++i;
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::InsertIntelligentTaps( 
	NoteData &inout, 
	int iWindowSizeRows, 
	int iInsertOffsetRows, 
	int iWindowStrideRows, 
	bool bSkippy, 
	int iStartIndex,
	int iEndIndex )
{
	ASSERT( iInsertOffsetRows <= iWindowSizeRows );
	ASSERT( iWindowSizeRows <= iWindowStrideRows );

	bool bRequireNoteAtBeginningOfWindow = !bSkippy;
	bool bRequireNoteAtEndOfWindow = true;

	/* Start on a multiple of fBeatInterval. */
	iStartIndex = Quantize( iStartIndex, iWindowStrideRows );

	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, i, iStartIndex, iEndIndex )
	{
		// Insert a beat in the middle of every fBeatInterval.
		if( i % iWindowStrideRows != 0 )
			continue;	// even beats only

		int iRowEarlier = i;
		int iRowLater = i + iWindowSizeRows;
		int iRowToAdd = i + iInsertOffsetRows;
		// following two lines have been changed because the behavior of treating hold-heads
		// as different from taps doesn't feel right, and because we need to check
		// against TAP_ADDITION with the BMRize mod.
		if( bRequireNoteAtBeginningOfWindow )
			if( inout.GetNumTapNonEmptyTracks(iRowEarlier)!=1 || inout.GetNumTracksWithTapOrHoldHead(iRowEarlier)!=1 )
				continue;
		if( bRequireNoteAtEndOfWindow )
			if( inout.GetNumTapNonEmptyTracks(iRowLater)!=1 || inout.GetNumTracksWithTapOrHoldHead(iRowLater)!=1 )
				continue;
		// there is a 4th and 8th note surrounding iRowBetween
		
		// don't insert a new note if there's already one within this interval
		bool bNoteInMiddle = false;
		for( int t = 0; t < inout.GetNumTracks(); ++t )
			if( inout.IsHoldNoteAtRow(t, iRowEarlier+1) )
				bNoteInMiddle = true;
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, j, iRowEarlier+1, iRowLater-1 )
			bNoteInMiddle = true;
		if( bNoteInMiddle )
			continue;

		// add a note deterministically somewhere on a track different from the two surrounding notes
		int iTrackOfNoteEarlier = -1;
		bool bEarlierHasNonEmptyTrack = inout.GetTapFirstNonEmptyTrack( iRowEarlier, iTrackOfNoteEarlier );
		int iTrackOfNoteLater = -1;
		inout.GetTapFirstNonEmptyTrack( iRowLater, iTrackOfNoteLater );
		int iTrackOfNoteToAdd = 0;
		if( bSkippy  &&
			iTrackOfNoteEarlier != iTrackOfNoteLater &&   // Don't make skips on the same note
			bEarlierHasNonEmptyTrack )
		{
			iTrackOfNoteToAdd = iTrackOfNoteEarlier;
		}
		else if( abs(iTrackOfNoteEarlier-iTrackOfNoteLater) >= 2 )
		{
			// try to choose a track between the earlier and later notes
			iTrackOfNoteToAdd = min(iTrackOfNoteEarlier,iTrackOfNoteLater)+1;
		}
		else if( min(iTrackOfNoteEarlier,iTrackOfNoteLater)-1 >= 0 )
		{
			// try to choose a track just to the left
			iTrackOfNoteToAdd = min(iTrackOfNoteEarlier,iTrackOfNoteLater)-1;
		}
		else if( max(iTrackOfNoteEarlier,iTrackOfNoteLater)+1 < inout.GetNumTracks() )
		{
			// try to choose a track just to the right
			iTrackOfNoteToAdd = max(iTrackOfNoteEarlier,iTrackOfNoteLater)+1;
		}

		inout.SetTapNote(iTrackOfNoteToAdd, iRowToAdd, TAP_ADDITION_TAP);
	}
	inout.RevalidateATIs(vector<int>(), false);
}
#if 0
class TrackIterator
{
public:
	TrackIterator();

	/* If called, iterate only over [iStart,iEnd]. */
	void SetRange( int iStart, int iEnd )
	{
	}

	/* If called, pay attention to iTrack only. */
	void SetTrack( iTrack );

	/* Extend iStart and iEnd to include hold notes overlapping the boundaries.  Call SetRange()
	 * and SetTrack() first. */
	void HoldInclusive();

	/* Reduce iStart and iEnd to exclude hold notes overlapping the boundaries.  Call SetRange()
	 * and SetTrack() first. */
	void HoldExclusive();

	/* If called, keep the iterator around.  This results in much faster iteration.  If used,
	 * ensure that the current row will always remain valid.  SetTrack() must be called first. */
	void Fast();

	/* Retrieve an iterator for the current row.  SetTrack() must be called first (but Fast()
	 * does not). */
	TapNote::iterator Get();

	int GetRow() const { return m_iCurrentRow; }
	bool Prev();
	bool Next();

private:
	int m_iStart, m_iEnd;
	int m_iTrack;

	bool m_bFast;

	int m_iCurrentRow;

	NoteData::iterator m_Iterator;

	/* m_bFast only: */
	NoteData::iterator m_Begin, m_End;
};

bool TrackIterator::Next()
{
	if( m_bFast )
	{
		if( m_Iterator == XXX )
			;

	}

}

TrackIterator::TrackIterator()
{
	m_iStart = 0;
	m_iEnd = MAX_NOTE_ROW;
	m_iTrack = -1;
}
#endif

void NoteDataUtil::AddMines( NoteData &inout, int iStartIndex, int iEndIndex )
{
	// Change whole rows at a time to be tap notes.  Otherwise, it causes
	// major problems for our scoring system. -Chris

	int iRowCount = 0;
	int iPlaceEveryRows = 6;
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		iRowCount++;

		// place every 6 or 7 rows
		// XXX: What is "6 or 7" derived from?  Can we calculate that in a way
		// that won't break if ROWS_PER_MEASURE changes?
		if( iRowCount>=iPlaceEveryRows )
		{
			for( int t=0; t<inout.GetNumTracks(); t++ )
				if( inout.GetTapNote(t,r).type == TapNoteType_Tap )
					inout.SetTapNote(t,r,TAP_ADDITION_MINE);
			
			iRowCount = 0;
			if( iPlaceEveryRows == 6 )
				iPlaceEveryRows = 7;
			else
				iPlaceEveryRows = 6;
		}
	}

	// Place mines right after hold so player must lift their foot.
	for( int iTrack=0; iTrack<inout.GetNumTracks(); ++iTrack )
	{
		FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( inout, iTrack, r, iStartIndex, iEndIndex )
		{
			const TapNote &tn = inout.GetTapNote( iTrack, r );
			if( tn.type != TapNoteType_HoldHead )
				continue;

			int iMineRow = r + tn.iDuration + BeatToNoteRow(0.5f);
			if( iMineRow < iStartIndex || iMineRow > iEndIndex )
				continue;

			// Only place a mines if there's not another step nearby
			int iMineRangeBegin = iMineRow - BeatToNoteRow( 0.5f ) + 1;
			int iMineRangeEnd = iMineRow + BeatToNoteRow( 0.5f ) - 1;
			if( !inout.IsRangeEmpty(iTrack, iMineRangeBegin, iMineRangeEnd) )
				continue;
		
			// Add a mine right after the hold end.
			inout.SetTapNote( iTrack, iMineRow, TAP_ADDITION_MINE );

			// Convert all notes in this row to mines.
			for( int t=0; t<inout.GetNumTracks(); t++ )
				if( inout.GetTapNote(t,iMineRow).type == TapNoteType_Tap )
					inout.SetTapNote(t,iMineRow,TAP_ADDITION_MINE);

			iRowCount = 0;
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::Echo( NoteData &inout, int iStartIndex, int iEndIndex )
{
	// add 8th note tap "echos" after all taps
	int iEchoTrack = -1;

	const int rows_per_interval = BeatToNoteRow( 0.5f );
	iStartIndex = Quantize( iStartIndex, rows_per_interval );

	/* Clamp iEndIndex to the last real tap note.  Otherwise, we'll keep adding
	 * echos of our echos all the way up to MAX_TAP_ROW. */
	iEndIndex = min( iEndIndex, inout.GetLastRow() )+1;

	// window is one beat wide and slides 1/2 a beat at a time
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		if( r % rows_per_interval != 0 )
			continue;	// 8th notes only

		const int iRowWindowBegin = r;
		const int iRowWindowEnd = r + rows_per_interval*2;

		const int iFirstTapInRow = inout.GetFirstTrackWithTap(iRowWindowBegin);
		if( iFirstTapInRow != -1 )
			iEchoTrack = iFirstTapInRow;

		if( iEchoTrack==-1 )
			continue;	// don't lay

		// don't insert a new note if there's already a tap within this interval
		bool bTapInMiddle = false;
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r2, iRowWindowBegin+1, iRowWindowEnd-1 )
			bTapInMiddle = true;
		if( bTapInMiddle )
			continue;	// don't lay


		const int iRowEcho = r + rows_per_interval;
		{
			set<int> viTracks;
			inout.GetTracksHeldAtRow( iRowEcho, viTracks );

			// don't lay if holding 2 already
			if( viTracks.size() >= 2 )
				continue;	// don't lay
			
			// don't lay echos on top of a HoldNote
			if( find(viTracks.begin(),viTracks.end(),iEchoTrack) != viTracks.end() )
				continue;	// don't lay
		}

		inout.SetTapNote( iEchoTrack, iRowEcho, TAP_ADDITION_TAP );
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::Planted( NoteData &inout, int iStartIndex, int iEndIndex )
{
	ConvertTapsToHolds( inout, 1, iStartIndex, iEndIndex );
}
void NoteDataUtil::Floored( NoteData &inout, int iStartIndex, int iEndIndex )
{
	ConvertTapsToHolds( inout, 2, iStartIndex, iEndIndex );
}
void NoteDataUtil::Twister( NoteData &inout, int iStartIndex, int iEndIndex )
{
	ConvertTapsToHolds( inout, 3, iStartIndex, iEndIndex );
}
void NoteDataUtil::ConvertTapsToHolds( NoteData &inout, int iSimultaneousHolds, int iStartIndex, int iEndIndex )
{
	// Convert all taps to freezes.
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		int iTrackAddedThisRow = 0;
		for( int t=0; t<inout.GetNumTracks(); t++ )
		{
			if( iTrackAddedThisRow > iSimultaneousHolds )
				break;

			if( inout.GetTapNote(t,r).type == TapNoteType_Tap )
			{
				// Find the ending row for this hold
				int iTapsLeft = iSimultaneousHolds;

				int r2 = r+1;
				bool addHold = true;
				FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, next_row, r+1, iEndIndex )
				{
					r2 = next_row;

					// If there are two taps in a row on the same track, 
					// don't convert the earlier one to a hold.
					if( inout.GetTapNote(t,r2).type != TapNoteType_Empty )
					{
						addHold = false;
						break;
					}

					set<int> tracksDown;
					inout.GetTracksHeldAtRow( r2, tracksDown );
					inout.GetTapNonEmptyTracks( r2, tracksDown );
					iTapsLeft -= tracksDown.size();
					if( iTapsLeft == 0 )
						break;	// we found the ending row for this hold
					else if( iTapsLeft < 0 )
					{
						addHold = false;
						break;
					}
				}

				if (!addHold)
				{
					continue;
				}

				// If the steps end in a tap, convert that tap
				// to a hold that lasts for at least one beat.
				if( r2 == r+1 )
					r2 = r+BeatToNoteRow(1);

				inout.AddHoldNote( t, r, r2, TAP_ORIGINAL_HOLD_HEAD );
				iTrackAddedThisRow++;
			}
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::SnapToNearestNoteType( NoteData &inout, NoteType nt1, NoteType nt2, int iStartIndex, int iEndIndex )
{
	// nt2 is optional and should be NoteType_Invalid if it is not used

	float fSnapInterval1 = NoteTypeToBeat( nt1 );
	float fSnapInterval2 = 10000; // nothing will ever snap to this.  That's what we want!
	if( nt2 != NoteType_Invalid )
		fSnapInterval2 = NoteTypeToBeat( nt2 );

	// iterate over all TapNotes in the interval and snap them
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, iOldIndex, iStartIndex, iEndIndex )
	{
		int iNewIndex1 = Quantize( iOldIndex, BeatToNoteRow(fSnapInterval1) );
		int iNewIndex2 = Quantize( iOldIndex, BeatToNoteRow(fSnapInterval2) );

		bool bNewBeat1IsCloser = abs(iNewIndex1-iOldIndex) < abs(iNewIndex2-iOldIndex);
		int iNewIndex = bNewBeat1IsCloser? iNewIndex1 : iNewIndex2;

		for( int c=0; c<inout.GetNumTracks(); c++ )
		{
			TapNote tnNew = inout.GetTapNote(c, iOldIndex);
			if( tnNew.type == TapNoteType_Empty )
				continue;

			inout.SetTapNote(c, iOldIndex, TAP_EMPTY);

			if( tnNew.type == TapNoteType_Tap && inout.IsHoldNoteAtRow(c, iNewIndex) )
				continue; // HoldNotes override TapNotes

			if( tnNew.type == TapNoteType_HoldHead )
			{
				/* Quantize the duration.  If the result is empty, just discard the hold. */
				tnNew.iDuration = Quantize( tnNew.iDuration, BeatToNoteRow(fSnapInterval1) );
				if( tnNew.iDuration == 0 )
					continue;

				/* We might be moving a hold note downwards, or extending its duration
				 * downwards.  Make sure there isn't anything else in the new range. */
				inout.ClearRangeForTrack( iNewIndex, iNewIndex+tnNew.iDuration+1, c );
			}
			
			inout.SetTapNote( c, iNewIndex, tnNew );
		}
	}
	inout.RevalidateATIs(vector<int>(), false);
}

struct ValidRow
{
	StepsType st;
	bool bValidMask[MAX_NOTE_TRACKS];
};
#define T true
#define f false
const ValidRow g_ValidRows[] = 
{
	{ StepsType_dance_double, { T,T,T,T,f,f,f,f } },
	{ StepsType_dance_double, { f,T,T,T,T,f,f,f } },
	{ StepsType_dance_double, { f,f,f,T,T,T,T,f } },
	{ StepsType_dance_double, { f,f,f,f,T,T,T,T } },
	{ StepsType_pump_double, { T,T,T,T,T,f,f,f,f,f } },
	{ StepsType_pump_double, { f,f,T,T,T,T,T,T,f,f } },
	{ StepsType_pump_double, { f,f,f,f,f,T,T,T,T,T } },
};
#undef T
#undef f

void NoteDataUtil::RemoveStretch( NoteData &inout, StepsType st, int iStartIndex, int iEndIndex )
{
	vector<const ValidRow*> vpValidRowsToCheck;
	for( unsigned i=0; i<ARRAYLEN(g_ValidRows); i++ )
	{
		if( g_ValidRows[i].st == st )
			vpValidRowsToCheck.push_back( &g_ValidRows[i] );
	}

	// bail early if there's nothing to validate against
	if( vpValidRowsToCheck.empty() )
		return;

	// each row must pass at least one valid mask
	FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( inout, r, iStartIndex, iEndIndex )
	{
		// only check rows with jumps
		if( inout.GetNumTapNonEmptyTracks(r) < 2 )
			continue;

		bool bPassedOneMask = false;
		for( unsigned i=0; i<vpValidRowsToCheck.size(); i++ )
		{
			const ValidRow &vr = *vpValidRowsToCheck[i];
			if( NoteDataUtil::RowPassesValidMask(inout,r,vr.bValidMask) )
			{
				bPassedOneMask = true;
				break;
			}
		}

		if( !bPassedOneMask )
			RemoveAllButOneTap( inout, r );
	}
	inout.RevalidateATIs(vector<int>(), false);
}

bool NoteDataUtil::RowPassesValidMask( NoteData &inout, int row, const bool bValidMask[] )
{
	for( int t=0; t<inout.GetNumTracks(); t++ )
	{
		if( !bValidMask[t] && inout.GetTapNote(t,row).type != TapNoteType_Empty )
			return false;
	}

	return true;
}

void NoteDataUtil::ConvertAdditionsToRegular( NoteData &inout )
{
	for( int t=0; t<inout.GetNumTracks(); t++ )
		FOREACH_NONEMPTY_ROW_IN_TRACK( inout, t, r )
			if( inout.GetTapNote(t,r).source == TapNoteSource_Addition )
			{
				TapNote tn = inout.GetTapNote(t,r);
				tn.source = TapNoteSource_Original;
				inout.SetTapNote( t, r, tn );
			}
	inout.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::Scale( NoteData &nd, float fScale )
{
	ASSERT( fScale > 0 );
	
	NoteData ndOut;
	ndOut.SetNumTracks( nd.GetNumTracks() );
	
	for( int t=0; t<nd.GetNumTracks(); t++ )
	{
		for( NoteData::const_iterator iter = nd.begin(t); iter != nd.end(t); ++iter )
		{
			TapNote tn = iter->second;
			int iNewRow      = lrintf( fScale * iter->first );
			int iNewDuration = lrintf( fScale * (iter->first + tn.iDuration) );
			tn.iDuration = iNewDuration;
			ndOut.SetTapNote( t, iNewRow, tn );
		}
	}
	
	nd.swap( ndOut );
	nd.RevalidateATIs(vector<int>(), false);
}

/* XXX: move this to an appropriate place, same place as NoteRowToBeat perhaps? */
static inline int GetScaledRow( float fScale, int iStartIndex, int iEndIndex, int iRow )
{
	if( iRow < iStartIndex )
		return iRow;
	else if( iRow > iEndIndex )
		return iRow + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) );
	else
		return lrintf( (iRow - iStartIndex) * fScale ) + iStartIndex;
}

void NoteDataUtil::ScaleRegion( NoteData &nd, float fScale, int iStartIndex, int iEndIndex )
{
	ASSERT( fScale > 0 );
	ASSERT( iStartIndex < iEndIndex );
	ASSERT( iStartIndex >= 0 );
	
	NoteData ndOut;
	ndOut.SetNumTracks( nd.GetNumTracks() );
	
	for( int t=0; t<nd.GetNumTracks(); t++ )
	{
		for( NoteData::const_iterator iter = nd.begin(t); iter != nd.end(t); ++iter )
		{
			TapNote tn = iter->second;
			int iNewRow      = GetScaledRow( fScale, iStartIndex, iEndIndex, iter->first );
			int iNewDuration = GetScaledRow( fScale, iStartIndex, iEndIndex, iter->first + tn.iDuration ) - iNewRow;
			tn.iDuration = iNewDuration;
			ndOut.SetTapNote( t, iNewRow, tn );
		}
	}
	
	nd.swap( ndOut );
	nd.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::InsertRows( NoteData &nd, int iStartIndex, int iRowsToAdd )
{
	ASSERT( iRowsToAdd >= 0 );

	NoteData temp;
	temp.SetNumTracks( nd.GetNumTracks() );
	temp.CopyRange( nd, iStartIndex, MAX_NOTE_ROW );
	nd.ClearRange( iStartIndex, MAX_NOTE_ROW );
	nd.CopyRange( temp, 0, MAX_NOTE_ROW, iStartIndex + iRowsToAdd );		
	nd.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::DeleteRows( NoteData &nd, int iStartIndex, int iRowsToDelete )
{
	ASSERT( iRowsToDelete >= 0 );

	NoteData temp;
	temp.SetNumTracks( nd.GetNumTracks() );
	temp.CopyRange( nd, iStartIndex + iRowsToDelete, MAX_NOTE_ROW );
	nd.ClearRange( iStartIndex, MAX_NOTE_ROW );
	nd.CopyRange( temp, 0, MAX_NOTE_ROW, iStartIndex );		
	nd.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllTapsOfType( NoteData& ndInOut, TapNoteType typeToRemove )
{
	/* Be very careful when deleting the tap notes. Erasing elements from maps using
	 * iterators invalidates only the iterator that is being erased. To that end,
	 * increment the iterator before deleting the elment of the map.
	 */
	for( int t=0; t<ndInOut.GetNumTracks(); t++ )
	{
		for( NoteData::iterator iter = ndInOut.begin(t); iter != ndInOut.end(t); )
		{
			if( iter->second.type == typeToRemove )
				ndInOut.RemoveTapNote( t, iter++ );
			else
				++iter;
		}
	}
	ndInOut.RevalidateATIs(vector<int>(), false);
}

void NoteDataUtil::RemoveAllTapsExceptForType( NoteData& ndInOut, TapNoteType typeToKeep )
{
	/* Same as in RemoveAllTapsOfType(). */
	for( int t=0; t<ndInOut.GetNumTracks(); t++ )
	{
		for( NoteData::iterator iter = ndInOut.begin(t); iter != ndInOut.end(t); )
		{
			if( iter->second.type != typeToKeep )
				ndInOut.RemoveTapNote( t, iter++ );
			else
				++iter;
		}
	}
	ndInOut.RevalidateATIs(vector<int>(), false);
}

int NoteDataUtil::GetMaxNonEmptyTrack( const NoteData& in )
{
	for( int t=in.GetNumTracks()-1; t>=0; t-- )
		if( !in.IsTrackEmpty( t ) )
			return t;
	return -1;
}

bool NoteDataUtil::AnyTapsAndHoldsInTrackRange( const NoteData& in, int iTrack, int iStart, int iEnd )
{
	if( iStart >= iEnd )
		return false;

	// for each index we crossed since the last update:
	FOREACH_NONEMPTY_ROW_IN_TRACK_RANGE( in, iTrack, r, iStart, iEnd )
	{
		switch( in.GetTapNote( iTrack, r ).type )
		{
		case TapNoteType_Empty:
		case TapNoteType_Mine:
			continue;
		default:
			return true;
		}
	}

	if( in.IsHoldNoteAtRow( iTrack, iEnd ) )
		return true;

	return false;
}

/* Find the next row that either starts a TapNote, or ends a previous one. */
bool NoteDataUtil::GetNextEditorPosition( const NoteData& in, int &rowInOut )
{
	int iOriginalRow = rowInOut;
	bool bAnyHaveNextNote = in.GetNextTapNoteRowForAllTracks( rowInOut );

	int iClosestNextRow = rowInOut;
	if( !bAnyHaveNextNote )
		iClosestNextRow = MAX_NOTE_ROW;

	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		int iHeadRow;
		if( !in.IsHoldHeadOrBodyAtRow(t, iOriginalRow, &iHeadRow) )
			continue;

		const TapNote &tn = in.GetTapNote( t, iHeadRow );
		int iEndRow = iHeadRow + tn.iDuration;
		if( iEndRow == iOriginalRow )
			continue;

		bAnyHaveNextNote = true;
		ASSERT( iEndRow < MAX_NOTE_ROW );
		iClosestNextRow = min( iClosestNextRow, iEndRow );
	}

	if( !bAnyHaveNextNote )
		return false;

	rowInOut = iClosestNextRow;
	return true;
}

bool NoteDataUtil::GetPrevEditorPosition( const NoteData& in, int &rowInOut )
{
	int iOriginalRow = rowInOut;
	bool bAnyHavePrevNote = in.GetPrevTapNoteRowForAllTracks( rowInOut );

	int iClosestPrevRow = rowInOut;
	for( int t=0; t<in.GetNumTracks(); t++ )
	{
		int iHeadRow = iOriginalRow;
		if( !in.GetPrevTapNoteRowForTrack(t, iHeadRow) )
			continue;

		const TapNote &tn = in.GetTapNote( t, iHeadRow );
		if( tn.type != TapNoteType_HoldHead )
			continue;

		int iEndRow = iHeadRow + tn.iDuration;
		if( iEndRow >= iOriginalRow )
			continue;

		bAnyHavePrevNote = true;
		ASSERT( iEndRow < MAX_NOTE_ROW );
		iClosestPrevRow = max( iClosestPrevRow, iEndRow );
	}

	if( !bAnyHavePrevNote )
		return false;

	rowInOut = iClosestPrevRow;
	return true;
}


unsigned int NoteDataUtil::GetTotalHoldTicks( NoteData* nd, const TimingData* td )
{
	unsigned int ret = 0;
	// Last row must be included. -- Matt
	int end = nd->GetLastRow()+1;
	vector<TimingSegment*> segments = td->GetTimingSegments( SEGMENT_TICKCOUNT );
	// We start with the LAST TimingSegment and work our way backwards.
	// This way we can continually update end instead of having to lookup when
	// the next segment starts.
	for(int i = segments.size() - 1; i >= 0; i--)
	{
		TickcountSegment *ts = (TickcountSegment*) segments[i];
		if( ts->GetTicks() > 0)
		{
			// Jump to each point where holds would tick and add the number of holds there to ret.
			for(int j = ts->GetRow(); j < end; j += ROWS_PER_BEAT / ts->GetTicks() )
				// 1 tick per row.
				if( nd->GetNumTracksHeldAtRow(j) > 0 )
					ret++;
		}
		end = ts->GetRow();
	}
	return ret;
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
