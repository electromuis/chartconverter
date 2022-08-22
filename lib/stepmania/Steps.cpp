/* This stores a single note pattern for a song.
 *
 * We can have too much data to keep everything decompressed as NoteData, so most
 * songs are kept in memory compressed as SMData until requested.  NoteData is normally
 * not requested casually during gameplay; we can move through screens, the music
 * wheel, etc. without touching any NoteData.
 *
 * To save more memory, if data is cached on disk, read it from disk on demand.  Not
 * all Steps will have an associated file for this purpose.  (Profile edits don't do
 * this yet.)
 *
 * Data can be on disk (always compressed), compressed in memory, and uncompressed in
 * memory. */
#include "global.h"
#include "Steps.h"
#include "Song.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "NoteData.h"
#include "GameManager.h"
#include "NoteDataUtil.h"
#include <algorithm>

/* register DisplayBPM with StringConversion */
#include "EnumHelper.h"

// For hashing hart keys - Mina
#include "CryptManager.h"

static const char *DisplayBPMNames[] =
{
	"Actual",
	"Specified",
	"Random",
};
XToString( DisplayBPM );

Steps::Steps(Song *song): m_StepsType(StepsType_Invalid), m_pSong(song),
	m_pNoteData(new NoteData), m_bNoteDataIsFilled(false), 
	m_sNoteDataCompressed(""), m_sFilename(""), m_bSavedToDisk(false), m_iHash(0),
	m_sDescription(""), m_sChartStyle(""), 
	m_Difficulty(Difficulty_Invalid), m_iMeter(0),
	m_bAreCachedRadarValuesJustLoaded(false),
	m_sCredit(""), displayBPMType(DISPLAY_BPM_ACTUAL),
	specifiedBPMMin(0), specifiedBPMMax(0) {}

Steps::~Steps()
{
}

void Steps::GetDisplayBpms( DisplayBpms &AddTo ) const
{
	if( this->GetDisplayBPM() == DISPLAY_BPM_SPECIFIED )
	{
		AddTo.Add( this->GetMinBPM() );
		AddTo.Add( this->GetMaxBPM() );
	}
	else
	{
		float fMinBPM, fMaxBPM;
		this->GetTimingData()->GetActualBPM( fMinBPM, fMaxBPM );
		AddTo.Add( fMinBPM );
		AddTo.Add( fMaxBPM );
	}
}

unsigned Steps::GetHash() const
{
	if( m_iHash )
		return m_iHash;
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled )
			return 0; // No data, no hash.
		NoteDataUtil::GetSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed );
	}
	m_iHash = GetHashForString( m_sNoteDataCompressed );
	return m_iHash;
}

bool Steps::IsNoteDataEmpty() const
{
	return this->m_sNoteDataCompressed.empty();
}

void Steps::SetNoteData( const NoteData& noteDataNew )
{
	*m_pNoteData = noteDataNew;
	m_bNoteDataIsFilled = true;
	
	m_sNoteDataCompressed = RString();
	m_iHash = 0;
}

void Steps::GetNoteData( NoteData& noteDataOut ) const
{
	Decompress();

	if( m_bNoteDataIsFilled )
	{
		noteDataOut = *m_pNoteData;
	}
	else
	{
		noteDataOut.ClearAll();
		noteDataOut.SetNumTracks( GAMEMAN->GetStepsTypeInfo(m_StepsType).iNumTracks );
	}
}

NoteData Steps::GetNoteData() const
{
	NoteData tmp;
	this->GetNoteData( tmp );
	return tmp;
}

void Steps::SetSMNoteData( const RString &notes_comp_ )
{
	m_pNoteData->Init();
	m_bNoteDataIsFilled = false;

	m_sNoteDataCompressed = notes_comp_;
	m_iHash = 0;
}

/* XXX: this function should pull data from m_sFilename, like Decompress() */
void Steps::GetSMNoteData( RString &notes_comp_out ) const
{
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled ) 
		{
			/* no data is no data */
			notes_comp_out = "";
			return;
		}

		NoteDataUtil::GetSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed );
	}

	notes_comp_out = m_sNoteDataCompressed;
}

float Steps::PredictMeter() const
{
	float pMeter = 0.775f;

	const float RadarCoeffs[NUM_RadarCategory] =
	{
		10.1f, 5.27f,-0.905f, -1.10f, 2.86f,
		0,0,0,0,0,0,0,0
	};
	const RadarValues &rv = GetRadarValues( PLAYER_1 );
	for( int r = 0; r < NUM_RadarCategory; ++r )
		pMeter += rv[r] * RadarCoeffs[r];

	const float DifficultyCoeffs[NUM_Difficulty] =
	{
		-0.877f, -0.877f, 0, 0.722f, 0.722f, 0
	};
	pMeter += DifficultyCoeffs[this->GetDifficulty()];

	// Init non-radar values
	const float SV = rv[RadarCategory_Stream] * rv[RadarCategory_Voltage];
	const float ChaosSquare = rv[RadarCategory_Chaos] * rv[RadarCategory_Chaos];
	pMeter += -6.35f * SV;
	pMeter += -2.58f * ChaosSquare;
	if (pMeter < 1) pMeter = 1;
	return pMeter;
}

void Steps::TidyUpData()
{
	// Don't set the StepsType to dance single if it's invalid.  That just
	// causes unrecognized charts to end up where they don't belong.
	// Leave it as StepsType_Invalid so the Song can handle it specially.  This
	// is a forwards compatibility feature, so that if a future version adds a
	// new style, editing a simfile with unrecognized Steps won't silently
	// delete them. -Kyz
	if( m_StepsType == StepsType_Invalid )
	{
		LOG->Warn("Detected steps with unknown style '%s' in '%s'", m_StepsTypeStr.c_str(), m_pSong->m_sSongFileName.c_str());
	}
	else if(m_StepsTypeStr == "")
	{
		m_StepsTypeStr= GAMEMAN->GetStepsTypeInfo(m_StepsType).szName;
	}

	if( GetDifficulty() == Difficulty_Invalid )
		SetDifficulty( StringToDifficulty(GetDescription()) );

	if( GetDifficulty() == Difficulty_Invalid )
	{
		if(	 GetMeter() == 1 )	SetDifficulty( Difficulty_Beginner );
		else if( GetMeter() <= 3 )	SetDifficulty( Difficulty_Easy );
		else if( GetMeter() <= 6 )	SetDifficulty( Difficulty_Medium );
		else				SetDifficulty( Difficulty_Hard );
	}

	if( GetMeter() < 1) // meter is invalid
		SetMeter( int(PredictMeter()) );
}

void Steps::Decompress() const
{
	const_cast<Steps *>(this)->Decompress();
}

bool stepstype_is_kickbox(StepsType st)
{
	return st == StepsType_kickbox_human || st == StepsType_kickbox_quadarm ||
		st == StepsType_kickbox_insect || st == StepsType_kickbox_arachnid;
}

void Steps::Decompress()
{
	if( m_bNoteDataIsFilled )
		return;	// already decompressed


	if( !m_sFilename.empty() && m_sNoteDataCompressed.empty() )
	{
		this->GetSMNoteData( m_sNoteDataCompressed );
	}

	if( m_sNoteDataCompressed.empty() )
	{
		/* there is no data, do nothing */
	}
	else
	{
		// load from compressed
		bool bComposite = GAMEMAN->GetStepsTypeInfo(m_StepsType).m_StepsTypeCategory == StepsTypeCategory_Routine;
		m_bNoteDataIsFilled = true;
		m_pNoteData->SetNumTracks( GAMEMAN->GetStepsTypeInfo(m_StepsType).iNumTracks );

		NoteDataUtil::LoadFromSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed, bComposite );
	}
}

void Steps::Compress() const
{
	// Always leave lights data uncompressed.
	if( this->m_StepsType == StepsType_lights_cabinet && m_bNoteDataIsFilled )
	{
		m_sNoteDataCompressed = RString();
		return;
	}

	if( !m_sFilename.empty() )
	{
		/* We have a file on disk; clear all data in memory.
		 * Data on profiles can't be accessed normally (need to mount and time-out
		 * the device), and when we start a game and load edits, we want to be
		 * sure that it'll be available if the user picks it and pulls the device.
		 * Also, Decompress() doesn't know how to load .edits. */
		m_pNoteData->Init();
		m_bNoteDataIsFilled = false;

		/* Be careful; 'x = ""', m_sNoteDataCompressed.clear() and m_sNoteDataCompressed.reserve(0)
		 * don't always free the allocated memory. */
		m_sNoteDataCompressed = RString();
		return;
	}

	// We have no file on disk. Compress the data, if necessary.
	if( m_sNoteDataCompressed.empty() )
	{
		if( !m_bNoteDataIsFilled )
			return; /* no data is no data */
		NoteDataUtil::GetSMNoteDataString( *m_pNoteData, m_sNoteDataCompressed );
	}

	m_pNoteData->Init();
	m_bNoteDataIsFilled = false;
}

void Steps::CopyFrom( Steps* pSource, StepsType ntTo, float fMusicLengthSeconds )	// pSource does not have to be of the same StepsType
{
	m_StepsType = ntTo;
	m_StepsTypeStr= GAMEMAN->GetStepsTypeInfo(ntTo).szName;
	NoteData noteData;
	pSource->GetNoteData( noteData );
	noteData.SetNumTracks( GAMEMAN->GetStepsTypeInfo(ntTo).iNumTracks );
	m_Timing = pSource->m_Timing;
	this->m_pSong = pSource->m_pSong;
	this->m_sAttackString = pSource->m_sAttackString;
	this->SetNoteData( noteData );
	this->SetDescription( pSource->GetDescription() );
	this->SetDifficulty( pSource->GetDifficulty() );
	this->SetMeter( pSource->GetMeter() );
}

void Steps::CreateBlank( StepsType ntTo )
{
	m_StepsType = ntTo;
	m_StepsTypeStr= GAMEMAN->GetStepsTypeInfo(ntTo).szName;
	NoteData noteData;
	noteData.SetNumTracks( GAMEMAN->GetStepsTypeInfo(ntTo).iNumTracks );
	this->SetNoteData( noteData );
}

void Steps::SetDifficultyAndDescription( Difficulty dc, RString sDescription )
{
	m_Difficulty = dc;
	m_sDescription = sDescription;
	if( GetDifficulty() == Difficulty_Edit )
		MakeValidEditDescription( m_sDescription );
}

void Steps::SetCredit( RString sCredit )
{
	m_sCredit = sCredit;
}

void Steps::SetChartStyle( RString sChartStyle )
{
	m_sChartStyle = sChartStyle;
}

bool Steps::MakeValidEditDescription( RString &sPreferredDescription )
{
	if( int(sPreferredDescription.size()) > MAX_STEPS_DESCRIPTION_LENGTH )
	{
		sPreferredDescription = sPreferredDescription.Left( MAX_STEPS_DESCRIPTION_LENGTH );
		return true;
	}
	return false;
}

void Steps::SetMeter( int meter )
{
	m_iMeter = meter;
}

const TimingData *Steps::GetTimingData() const
{
	return m_Timing.empty() ? &m_pSong->m_SongTiming : &m_Timing;
}

bool Steps::HasSignificantTimingChanges() const
{
	const TimingData *timing = GetTimingData();
	if( timing->HasStops() || timing->HasDelays() || timing->HasWarps() ||
		timing->HasSpeedChanges() || timing->HasScrollChanges() )
		return true;

	if( timing->HasBpmChanges() )
	{
		// check to see if these changes are significant.
		DisplayBpms bpms;
		m_pSong->GetDisplayBpms(bpms);
		if (bpms.GetMax() - bpms.GetMin() > 3.000f)
			return true;
	}

	return false;
}

const RString& Steps::GetMusicFile() const
{
	return m_MusicFile;
}

void Steps::SetMusicFile(const RString& file)
{
	m_MusicFile= file;
}

void Steps::SetCachedRadarValues( const RadarValues v[NUM_PLAYERS] )
{
	copy( v, v + NUM_PLAYERS, m_CachedRadarValues );
	m_bAreCachedRadarValuesJustLoaded = true;
}

RString Steps::GenerateChartKey()
{
	ChartKey = this->GenerateChartKey(*m_pNoteData, this->GetTimingData());
	return ChartKey;
}
RString Steps::GetChartKey()
{
	if (ChartKey.empty()) {
		this->Decompress();
		ChartKey = this->GenerateChartKey(*m_pNoteData, this->GetTimingData());
		this->Compress();
	}
	return ChartKey;
}
RString Steps::GenerateChartKey(NoteData &nd, TimingData *td)
{
	RString k = "";
	RString o = "";
	float bpm;
	nd.LogNonEmptyRows();
	std::vector<int>& nerv = nd.GetNonEmptyRowVector();


	RString firstHalf = "";
	RString secondHalf = "";

#pragma omp parallel sections
	{
#pragma omp section
		{
			for (size_t r = 0; r < nerv.size() / 2; r++) {
				int row = nerv[r];
				for (int t = 0; t < nd.GetNumTracks(); ++t) {
					const TapNote &tn = nd.GetTapNote(t, row);
					std::ostringstream os;
					os << tn.type;
					firstHalf.append(os.str());
				}
				bpm = td->GetBPMAtRow(row);
				std::ostringstream os;
				os << static_cast<int>(bpm + 0.374643f);
				firstHalf.append(os.str());
			}
		}

#pragma omp section
		{
			for (size_t r = nerv.size() / 2; r < nerv.size(); r++) {
				int row = nerv[r];
				for (int t = 0; t < nd.GetNumTracks(); ++t) {
					const TapNote &tn = nd.GetTapNote(t, row);
					std::ostringstream os;
					os << tn.type;
					secondHalf.append(os.str());
				}
				bpm = td->GetBPMAtRow(row);
				std::ostringstream os;
				os << static_cast<int>(bpm + 0.374643f);
				firstHalf.append(os.str());
			}
		}
	}
	k = firstHalf + secondHalf;

	//ChartKeyRecord = k;
	o.append("X");	// I was thinking of using "C" to indicate chart.. however.. X is cooler... - Mina
	o.append(BinaryToHex(CryptManager::GetSHA1ForString(k)));
	return o;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard, David Wilson
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
