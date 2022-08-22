#include "global.h"
#include "Song.h"

#include <time.h>
#include <set>
#include <float.h>

//-Nick12 Used for song file hashing
#include "CryptManager.h"

Song::Song()
{
	m_LoadedFromProfile = ProfileSlot_Invalid;
	m_fVersion = STEPFILE_VERSION_NUMBER;
	m_fMusicSampleStartSeconds = -1;
	m_fMusicSampleLengthSeconds = 0;
	m_fMusicLengthSeconds = 0;
	firstSecond = -1;
	lastSecond = -1;
	specifiedLastSecond = -1;
	m_SelectionDisplay = SHOW_ALWAYS;
	m_bEnabled = true;
	m_DisplayBPMType = DISPLAY_BPM_ACTUAL;
	m_fSpecifiedBPMMin = 0;
	m_fSpecifiedBPMMax = 0;
	m_bIsSymLink = false;
	m_bHasMusic = false;
	m_bHasBanner = false;
	m_bHasBackground = false;
	m_loaded_from_autosave= false;
}

Song::~Song()
{
	for (Steps *s : m_vpSteps)
	{
		SAFE_DELETE( s );
	}
	m_vpSteps.clear();
	for (Steps *s : m_UnknownStyleSteps)
	{
		SAFE_DELETE(s);
	}
	m_UnknownStyleSteps.clear();

	// It's the responsibility of the owner of this Song to make sure
	// that all pointers to this Song and its Steps are invalidated.
}

float Song::GetFirstSecond() const
{
	return this->firstSecond;
}

float Song::GetFirstBeat() const
{
	return this->m_SongTiming.GetBeatFromElapsedTime(this->firstSecond);
}

float Song::GetLastSecond() const
{
	return this->lastSecond;
}

float Song::GetLastBeat() const
{
	return this->m_SongTiming.GetBeatFromElapsedTime(this->lastSecond);
}

float Song::GetSpecifiedLastSecond() const
{
	return this->specifiedLastSecond;
}

float Song::GetSpecifiedLastBeat() const
{
	return this->m_SongTiming.GetBeatFromElapsedTime(this->specifiedLastSecond);
}

void Song::SetFirstSecond(const float f)
{
	this->firstSecond = f;
}

void Song::SetLastSecond(const float f)
{
	this->lastSecond = f;
}

void Song::SetSpecifiedLastSecond(const float f)
{
	this->specifiedLastSecond = f;
}

Steps *Song::CreateSteps()
{
	Steps *pSteps = new Steps(this);
	InitSteps( pSteps );
	return pSteps;
}

void Song::InitSteps(Steps *pSteps)
{
	// TimingData is initially empty (i.e. defaults to song timing)
	pSteps->m_sAttackString = this->m_sAttackString;
	pSteps->SetDisplayBPM(this->m_DisplayBPMType);
	pSteps->SetMinBPM(this->m_fSpecifiedBPMMin);
	pSteps->SetMaxBPM(this->m_fSpecifiedBPMMax);
}

void Song::AddSteps( Steps* pSteps )
{
	// Songs of unknown stepstype are saved as a forwards compatibility feature
	// so that editing a simfile made by a future version that has a new style
	// won't delete those steps. -Kyz
	if(pSteps->m_StepsType != StepsType_Invalid)
	{
		m_vpSteps.push_back( pSteps );
		ASSERT_M( pSteps->m_StepsType < NUM_StepsType, ssprintf("%i", pSteps->m_StepsType) );
		m_vpStepsByType[pSteps->m_StepsType].push_back( pSteps );
	}
	else
	{
		m_UnknownStyleSteps.push_back(pSteps);
	}
}

void Song::GetDisplayBpms( DisplayBpms &AddTo ) const
{
	if( m_DisplayBPMType == DISPLAY_BPM_SPECIFIED )
	{
		AddTo.Add( m_fSpecifiedBPMMin );
		AddTo.Add( m_fSpecifiedBPMMax );
	}
	else
	{
		float fMinBPM, fMaxBPM;
		m_SongTiming.GetActualBPM( fMinBPM, fMaxBPM );
		AddTo.Add( fMinBPM );
		AddTo.Add( fMaxBPM );
	}
}

RString Song::GetDisplayMainTitle() const
{
	return m_sMainTitle;
}

RString Song::GetDisplaySubTitle() const
{
	return m_sSubTitle;
}

RString Song::GetDisplayArtist() const
{
	return m_sArtist;
}

RString Song::GetMainTitle() const
{
	return m_sMainTitle;
}

RString Song::GetDisplayFullTitle() const
{
	RString Title = GetDisplayMainTitle();
	RString SubTitle = GetDisplaySubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

RString Song::GetTranslitFullTitle() const
{
	RString Title = GetTranslitMainTitle();
	RString SubTitle = GetTranslitSubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
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
