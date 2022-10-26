#include <napi.h>
#include <stdexcept>

#include "StepmaniaJS.h"
#include "lib/stepmania/Song.h"
#include "lib/stepmania/Steps.h"
#include "classes/SimfileLoader.h"


// ConvertChart(formatIn, formatOut, dataIn)
Napi::Value ConvertChart(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Wrong number of arguments")
            .ThrowAsJavaScriptException();

        return env.Null();
    }

    if (!info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
            return env.Null();

    }

    // Read js function arguments
    RString formatIn = (std::string) info[0].As<Napi::String>();
    RString formatOut = (std::string) info[1].As<Napi::String>();
    RString dataIn = (std::string) info[2].As<Napi::String>();

    Song song;
    if(!ParseSimfile(formatIn, dataIn, song)) {
        Napi::TypeError::New(env, "Parsing simfile failed")
            .ThrowAsJavaScriptException();

        return env.Null();
    }

    RString dataOut = WriteSimfile(formatOut, song);

    return Napi::String::New(env, dataOut);
}

Napi::Value HashChart(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Wrong number of arguments")
            .ThrowAsJavaScriptException();

        return env.Null();
    }

    if (!info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
            return env.Null();

    }

    // Read js function arguments
    RString formatIn = (std::string) info[0].As<Napi::String>();
    RString formatOut = (std::string) info[1].As<Napi::String>();
    RString dataIn = (std::string) info[2].As<Napi::String>();

    Song song;
    if(!ParseSimfile(formatIn, dataIn, song)) {
        Napi::TypeError::New(env, "Parsing simfile failed")
            .ThrowAsJavaScriptException();

        return env.Null();
    }

    auto steps = song.GetAllSteps();
    Napi::Array hash_array = Napi::Array::New(env, steps.size());

    for(int i=0; i<steps.size(); i++) {
        RString hash = HashSimfile(formatOut, *steps[i]);
        hash_array[i] = Napi::String::New(env, hash);
    }

    return hash_array;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    SimfileLoader::Init(env, exports);

    exports.Set(Napi::String::New(env, "ConvertChart"), Napi::Function::New(env, ConvertChart));
    
    return exports;
}

NODE_API_MODULE(addon, Init)