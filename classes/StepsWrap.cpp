#include "lib/stepmania/global.h"

#include "lib/stepmania/Steps.h"
#include "lib/stepmania/Song.h"
#include "lib/stepmania/CryptManager.h"
#include "lib/stepmania/RageUtil.h"
#include "lib/stepmania/NoteData.h"
#include "lib/stepmania/NoteDataUtil.h"
#include "lib/stepmania/GameConstantsAndTypes.h"
#include "StepsWrap.h"
#include "StepmaniaJS.h"

#include <iostream>

Napi::Object StepsWrap::Create(Napi::Env env, Steps* steps)
{
    Napi::Function func = DefineClass(env, "Song", {
        InstanceAccessor<&StepsWrap::GetMeter>("meter", napi_enumerable),
        InstanceAccessor<&StepsWrap::GetDifficulty>("difficulty", napi_enumerable),
        InstanceAccessor<&StepsWrap::GetCredit>("credit", napi_enumerable),
        InstanceAccessor<&StepsWrap::GetStyle>("style", napi_enumerable),
        
        InstanceMethod<&StepsWrap::GetHash>("GetHash"),
        InstanceMethod<&StepsWrap::GetHashInput>("GetHashInput"),
    }, steps);

    return func.New({});
}

StepsWrap::StepsWrap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<StepsWrap>(info)
{
    this->steps = (Steps*)info.Data();
}

Napi::Value StepsWrap::GetMeter(const Napi::CallbackInfo &info)
{
    return Napi::Number::New(info.Env(), this->steps->GetMeter());
}

Napi::Value StepsWrap::GetDifficulty(const Napi::CallbackInfo &info)
{
    return Napi::String::New(
        info.Env(),
        DifficultyToString(this->steps->GetDifficulty())
    );
}

Napi::Value StepsWrap::GetCredit(const Napi::CallbackInfo &info)
{
    return Napi::String::New(info.Env(), this->steps->GetCredit());
}

Napi::Value StepsWrap::GetStyle(const Napi::CallbackInfo &info)
{
    return Napi::String::New(info.Env(), StepsTypeToString(this->steps->m_StepsType));
}

// Groovestats hashing functions START


static const int BEATS_PER_MEASURE = 4;
static const int ROWS_PER_MEASURE = ROWS_PER_BEAT * BEATS_PER_MEASURE;
static const int divisions[] = {
    1,
    2,
    4,
    8,
    12,
    16,
    24,
    32,
    48,
    64,
    192,
};

static const int numDivisions = 11;

float GSNoteTypeToDivision( int divisionIndex )
{
	switch( divisions[divisionIndex] )
	{
	case 1:	return 1.0f*4;
	case 2:	return 1.0f*2;
	case 4:	return 1.0f;	// quarter notes
	case 8:	return 1.0f/2;	// eighth notes
	case 12:	return 1.0f/3;	// quarter note triplets
	case 16:	return 1.0f/4;	// sixteenth notes
	case 24:	return 1.0f/6;	// eighth note triplets
	case 32:	return 1.0f/8;	// thirty-second notes
	case 48:	return 1.0f/12; // sixteenth note triplets
	case 64:	return 1.0f/16; // sixty-fourth notes
	case 192:	return 1.0f/48; // sixty-fourth note triplets
	default:
		FAIL_M(ssprintf("Unrecognized devision index type: %i", divisionIndex));
	}
}

float GSGetSmallestNoteTypeInRange( const NoteData &n, int iStartIndex, int iEndIndex )
{
    for(int d=0; d<numDivisions; d++)
    {
        int iRowSpacing = lrintf( GSNoteTypeToDivision(d) * ROWS_PER_BEAT );

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
			return GSNoteTypeToDivision(d);	// stop searching. We found the smallest NoteType
    }
	return GSNoteTypeToDivision(0);	// well-formed notes created in the editor should never get here
}

float GSGetSmallestNoteTypeForMeasure( const NoteData &nd, int iMeasureIndex )
{
	const int iMeasureStartIndex = iMeasureIndex * ROWS_PER_MEASURE;
	const int iMeasureEndIndex = (iMeasureIndex+1) * ROWS_PER_MEASURE;

	return GSGetSmallestNoteTypeInRange( nd, iMeasureStartIndex, iMeasureEndIndex );
}

RString NormalizeDecimal(float decimal)
{
    return ssprintf("%.3f", decimal);
}

RString StepsWrap::NormallizedChart()
{
    NoteData noteData;
    steps->GetNoteData(noteData);

    // Get note data
	vector<NoteData> parts;
	float fLastBeat = -1.0f;

	NoteDataUtil::SplitCompositeNoteData( noteData, parts );

	for (NoteData &nd : parts)
	{
		NoteDataUtil::InsertHoldTails( nd );
		fLastBeat = max( fLastBeat, nd.GetLastBeat() );
	}

	int iLastMeasure = int( fLastBeat/BEATS_PER_MEASURE );

	RString sRet = "";
	int partNum = 0;
	for (NoteData const &nd : parts)
	{
		if( partNum++ != 0 )
			sRet.append( "&\n" );
		for( int m = 0; m <= iLastMeasure; ++m ) // foreach measure
		{
			if( m )
				sRet.append( "\n,\n" );

            int iRowSpacing = lrintf(GSGetSmallestNoteTypeForMeasure(nd, m) * ROWS_PER_BEAT);
			const int iMeasureStartRow = m * ROWS_PER_MEASURE;
			const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

			for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
			{
                if(r != iMeasureStartRow)
                    sRet.append( 1, '\n' );

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

				
			}
		}
	}

    return sRet;
}

// Groovestats hashing functions END

Napi::Value StepsWrap::GetHashInput(const Napi::CallbackInfo &info)
{
    RString format = (std::string) info[0].As<Napi::String>();

    if(format == "GS2" || format == "GS3") {
        RString hashInput = "";

		try {
        	hashInput.append(NormallizedChart());
		}
		catch (runtime_error e)
		{
			Napi::TypeError::New(info.Env(), e.what())
            	.ThrowAsJavaScriptException();

        	return info.Env().Null();
		}

        // BPM
        auto bpms = steps->m_pSong->m_SongTiming.GetTimingSegments(SEGMENT_BPM);

        for(int i=0; i<bpms.size(); i++) {
            BPMSegment* bpm = (BPMSegment*)bpms[i];
            hashInput.append(NormalizeDecimal(bpm->GetBeat()));
            hashInput.append("=");
            hashInput.append(NormalizeDecimal(bpm->GetBPM()));
            if(i < bpms.size()-1) {
                hashInput.append(";");
            }
        }

        return Napi::String::New(info.Env(), hashInput);
    }
    else {
        Napi::TypeError::New(info.Env(), "Unsupported hash format")
            .ThrowAsJavaScriptException();

        return info.Env().Null();
    }
}

Napi::Value StepsWrap::GetHash(const Napi::CallbackInfo &info)
{
    RString format = (std::string) info[0].As<Napi::String>();

    if(format == "GS2" || format == "GS3") {
        RString hashInput = "";

		try {
        	hashInput.append(NormallizedChart());
		}
		catch (runtime_error e)
		{
			Napi::TypeError::New(info.Env(), e.what())
            	.ThrowAsJavaScriptException();

        	return info.Env().Null();
		}

        // BPM
        auto bpms = steps->m_pSong->m_SongTiming.GetTimingSegments(SEGMENT_BPM);

        for(int i=0; i<bpms.size(); i++) {
            BPMSegment* bpm = (BPMSegment*)bpms[i];
            hashInput.append(NormalizeDecimal(bpm->GetBeat()));
            hashInput.append("=");
            hashInput.append(NormalizeDecimal(bpm->GetBPM()));
            if(i < bpms.size()-1) {
                hashInput.append(";");
            }
        }

        CryptManager crypt;
        RString hash;
        int hashLength = 0;
        if(format == "GS2") {
            hash = BinaryToHex(crypt.GetSHA256ForString(hashInput));
            hashLength = 12;
        }
        else if(format == "GS3") {
            hash = BinaryToHex(crypt.GetSHA1ForString(hashInput));
            hashLength = 16;
        }
        RString hashOut = hash.substr(0, hashLength);

        return Napi::String::New(info.Env(), hashOut);
    }
    else {
        Napi::TypeError::New(info.Env(), "Unsupported hash format")
            .ThrowAsJavaScriptException();

        return info.Env().Null();
    }
}