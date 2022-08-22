#include <napi.h>

#include "lib/stepmania/NotesLoaderSM.h"
#include "lib/stepmania/NotesWriterJson.h"
#include "lib/stepmania/NotesWriterDWI.h"
#include "lib/stepmania/Song.h"
#include "lib/stepmania/RageFileDriverMemory.h"
#include "lib/stepmania/RageUtil.h"
#include "lib/stepmania/RageFile.h"


using namespace Napi;

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

    RageFileDriverMem fileDriver;

    // Parse input chart
    RageFile inFile;
    inFile.Open(fileDriver, "tmpIn", RageFile::WRITE);
    inFile.Write(dataIn);
    inFile.Open(fileDriver, "tmpIn", RageFile::READ);

    SMLoader loader;
    Song song;
    loader.LoadFromSimfile(inFile, song);

    // Write output chart
    RageFile outFile;
    outFile.Open(fileDriver, "tmpOut", RageFile::WRITE);
    NotesWriterJson::WriteSong(outFile, song, true);
    outFile.Open(fileDriver, "tmpOut", RageFile::READ);

    RString FileString;
    FileString.reserve( outFile.GetFileSize() );
    int iBytesRead = outFile.Read( FileString );

    return Napi::String::New(env, FileString);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "ConvertChart"), Napi::Function::New(env, ConvertChart));
  return exports;
}

NODE_API_MODULE(addon, Init)