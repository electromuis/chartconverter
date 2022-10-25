#include "lib/stepmania/global.h"
#include "lib/stepmania/Song.h"
#include "SongWrap.h"
#include "StepsWrap.h"
#include "StepmaniaJS.h"

Napi::Object SongWrap::Create(Napi::Env env, std::shared_ptr<Song> song)
{
    Napi::Function func = DefineClass(env, "Song", {
        InstanceAccessor<&SongWrap::GetTitle>("title", napi_enumerable),
        InstanceAccessor<&SongWrap::GetArtist>("artist", napi_enumerable),
        InstanceAccessor<&SongWrap::GetSteps>("steps", napi_enumerable)
    }, &song);

    return func.New({});
}

SongWrap::SongWrap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<SongWrap>(info)
{
    this->song = *(std::shared_ptr<Song>*)info.Data();
}

Napi::Value SongWrap::GetTitle(const Napi::CallbackInfo &info)
{
    return Napi::String::New(info.Env(), this->song->m_sMainTitle);
}

Napi::Value SongWrap::GetArtist(const Napi::CallbackInfo &info)
{
    return Napi::String::New(info.Env(), this->song->m_sArtist);
}

Napi::Value SongWrap::GetSteps(const Napi::CallbackInfo &info)
{
    auto steps = song->GetAllSteps();
    Napi::Array steps_array = Napi::Array::New(info.Env(), steps.size());

    for(int i=0; i<steps.size(); i++) {
        steps_array[i] = StepsWrap::Create(info.Env(), steps[i]);
    }

    return steps_array;
}