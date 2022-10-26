#ifndef NotesLoaderSM_H
#define NotesLoaderSM_H

#include "NotesLoader.h"
#include "GameConstantsAndTypes.h"

class Song;
class Steps;
class TimingData;
class RageFile;

/**
 * @brief The highest allowable speed before Warps come in.
 *
 * This was brought in from StepMania 4's recent betas. */
const float FAST_BPM_WARP = 9999999.f;

/** @brief The maximum file size for edits. */
const int MAX_EDIT_STEPS_SIZE_BYTES		= 60*1024;	// 60KB

/** @brief Reads a Song from an .SM file. */
struct SMLoader : public NotesLoaderBase
{
	SMLoader() : fileExt(".sm"), songTitle() {}
	
	SMLoader(RString ext) : fileExt(ext), songTitle() {}
	
	virtual ~SMLoader() {}

	virtual bool LoadFromSimfile(RageFile& f, Song &out );

	/**
	 * @brief Retrieve the relevant notedata from the simfile.
	 * @param path the path where the simfile lives.
	 * @param out the Steps we are loading the data into. */
	virtual bool LoadNoteDataFromSimfile(RageFile& f, Steps &out );
	
	/**
	 * @brief Parse BPM Changes data from a string.
	 * @param out the vector to put the data in.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ParseBPMs(vector< pair<float, float> > &out,
	               const RString line,
	               const int rowsPerBeat = -1);
	/**
	 * @brief Process the BPM Segments from the string.
	 * @param out the TimingData being modified.
	 * @param vBPMChanges the vector of BPM Changes data. */
	void ProcessBPMs(TimingData & out,
	                 const vector< pair<float, float> > &vBPMChanges);
	/**
	 * @brief Parse Stops data from a string.
	 * @param out the vector to put the data in.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ParseStops(vector< pair<float, float> > &out,
	                const RString line,
	                const int rowsPerBeat = -1);
	/**
	 * @brief Process the Stop Segments from the data.
	 * @param out the TimingData being modified.
	 * @param vStops the vector of Stops data. */
	void ProcessStops(TimingData & out,
	                  const vector< pair<float, float> > &vStops);
	/**
	 * @brief Process BPM and stop segments from the data.
	 * @param out the TimingData being modified.
	 * @param vBPMs the vector of BPM changes.
	 * @param vStops the vector of stops. */
	void ProcessBPMsAndStops(TimingData &out,
			vector< pair<float, float> > &vBPMs,
			vector< pair<float, float> > &vStops);
	/**
	 * @brief Process the Delay Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ProcessDelays(TimingData & out,
			  const RString line,
			  const int rowsPerBeat = -1);
	/**
	 * @brief Process the Time Signature Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ProcessTimeSignatures(TimingData & out,
			   const RString line,
			   const int rowsPerBeat = -1);
	/**
	 * @brief Process the Tickcount Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	void ProcessTickcounts(TimingData & out,
				   const RString line,
				   const int rowsPerBeat = -1);
	
	/**
	 * @brief Process the Speed Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	virtual void ProcessSpeeds(TimingData & out,
				   const RString line,
				   const int rowsPerBeat = -1);
	
	virtual void ProcessCombos(TimingData & /* out */,
				   const RString line,
				   const int /* rowsPerBeat */ = -1) {}
	
	/**
	 * @brief Process the Fake Segments from the string.
	 * @param out the TimingData being modified.
	 * @param line the string in question.
	 * @param rowsPerBeat the number of rows per beat for this purpose. */
	virtual void ProcessFakes(TimingData & out,
				  const RString line,
				  const int rowsPerBeat = -1);
	
	
	/**
	 * @brief Convert a row value to the proper beat value.
	 * 
	 * This is primarily used for assistance with converting SMA files.
	 * @param line The line that contains the value.
	 * @param rowsPerBeat the number of rows per beat according to the original file.
	 * @return the converted beat value. */
	float RowToBeat(RString line, const int rowsPerBeat);
	
protected:
	/**
	 * @brief Process the different tokens we have available to get NoteData.
	 * @param stepsType The current StepsType.
	 * @param description The description of the chart.
	 * @param difficulty The difficulty (in words) of the chart.
	 * @param meter the difficulty (in numbers) of the chart.
	 * @param radarValues the calculated radar values.
	 * @param noteData the note data itself.
	 * @param out the Steps getting the data. */
	virtual void LoadFromTokens(RString sStepsType, 
				    RString sDescription,
				    RString sDifficulty,
				    RString sMeter,
				    RString sRadarValues,
				    RString sNoteData,
				    Steps &out);
	
	/**
	 * @brief Retrieve the file extension associated with this loader.
	 * @return the file extension. */
	RString GetFileExtension() const { return fileExt; }

	std::vector<RString> GetSongDirFiles(const RString &sSongDir);

public:
	// SetSongTitle and GetSongTitle changed to public to allow the functions
	// used by the parser helper to access them. -Kyz
	/**
	 * @brief Set the song title.
	 * @param t the song title. */
	virtual void SetSongTitle(const RString & title);
	
	/**
	 * @brief Get the song title.
	 * @return the song title. */
	virtual RString GetSongTitle() const;
	
private:
	/** @brief The file extension in use. */
	const RString fileExt;
	/** @brief The song title that is being processed. */
	RString songTitle;

	std::vector<RString> m_SongDirFiles;
};

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
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
