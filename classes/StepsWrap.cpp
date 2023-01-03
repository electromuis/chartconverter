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
#include <vector>
#include <cmath>

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

RString NormalizeDecimal(float decimal, RString format)
{


	// V2 goes down
	if(format == "GS2") {
		decimal = floor(decimal * 10000.0f) / 10000.0f;
	}

	// V3 goes up
	if(format == "GS3") {
		decimal = ceil(decimal * 10000.0f) / 10000.0f;
	}

	//decimal = ceil(decimal * 1000.0f) / 1000.0f;
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

			NoteType nt = NoteDataUtil::GetSmallestNoteTypeForMeasure( nd, m );
			int iRowSpacing = BeatToNoteRow( NoteTypeToBeat(nt) );

        	const int iMeasureStartRow = m * ROWS_PER_MEASURE;
			const int iMeasureLastRow = (m+1) * ROWS_PER_MEASURE - 1;

			std::vector<int> rowIndexes;

			for( int r=iMeasureStartRow; r<=iMeasureLastRow; r+=iRowSpacing )
			{
				rowIndexes.push_back(r);
			}

			bool minimal = false;
			while(!minimal && rowIndexes.size() % 2 == 0)
			{
				bool all_zeroes = true;
				for(int i=1; i<rowIndexes.size(); i+=2)
				{
					if(!nd.IsRowEmpty(rowIndexes.at(i))) {
						all_zeroes = false;
						break;
					}
				}

				if(all_zeroes) {
					std::vector<int> newIndexes;
					
					for( int ri=0; ri<rowIndexes.size(); ri += 2 )
					{
						newIndexes.push_back(rowIndexes.at(ri));
					}
					rowIndexes = newIndexes;
				} else {
					minimal = true;
				}
			}

			//cout << "Rows: " << rowIndexes.size() << std::endl;

			for( int ri=0; ri<rowIndexes.size(); ri++ )
			{
				int r = rowIndexes.at(ri);
                
				if(ri > 0)
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
            hashInput.append(NormalizeDecimal(bpm->GetBeat(), format));
            hashInput.append("=");
            hashInput.append(NormalizeDecimal(bpm->GetBPM(), format));
            if(i < bpms.size()-1) {
                hashInput.append(",\n");
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
            hashInput.append(NormalizeDecimal(bpm->GetBeat(), format));
            hashInput.append("=");
            hashInput.append(NormalizeDecimal(bpm->GetBPM(), format));
            if(i < bpms.size()-1) {
                hashInput.append(",");
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