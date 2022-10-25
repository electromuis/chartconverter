#include "SimfileLoader.h"

#include "lib/stepmania/global.h"
#include "lib/stepmania/Song.h"
#include "StepmaniaJS.h"
#include "SongWrap.h"

Napi::FunctionReference SimfileLoader::constructor;

SimfileLoader::SimfileLoader(const Napi::CallbackInfo &info) : Napi::ObjectWrap<SimfileLoader>(info)
{

}

Napi::Object SimfileLoader::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(env, "SimfileLoader", {
        InstanceMethod<&SimfileLoader::LoadString>("LoadString")
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("SimfileLoader", func);
    return exports;
}

Napi::Value SimfileLoader::LoadString(const Napi::CallbackInfo &info)
{
    std::shared_ptr<Song> song = std::make_shared<Song>();

    RString data = (std::string) info[0].As<Napi::String>();
    RString format = (std::string) info[1].As<Napi::String>();

    if(!ParseSimfile(format, data, *song)) {
        Napi::TypeError::New(info.Env(), "Parsing simfile failed")
            .ThrowAsJavaScriptException();

        return info.Env().Null();
    }

    Napi::Object newInstance = SongWrap::Create(info.Env(), song);
    Napi::EscapableHandleScope scope(info.Env());

    return scope.Escape(napi_value(newInstance));
}