#include <napi.h>
#include <stdexcept>

#include "lib/stepmania/global.h"

#include "lib/stepmania/NotesLoaderDWI.h"
#include "lib/stepmania/NotesLoaderJson.h"
#include "lib/stepmania/NotesLoaderSM.h"
#include "lib/stepmania/NotesLoaderSMA.h"
#include "lib/stepmania/NotesLoaderSSC.h"

#include "lib/stepmania/NotesWriterDWI.h"
#include "lib/stepmania/NotesWriterJson.h"
#include "lib/stepmania/NotesWriterSM.h"
#include "lib/stepmania/NotesWriterSSC.h"

#include "lib/stepmania/Song.h"
#include "lib/stepmania/RageFileDriverMemory.h"
#include "lib/stepmania/RageUtil.h"
#include "lib/stepmania/RageFile.h"
#include "lib/stepmania/NotesLoader.h"


using namespace Napi;

bool ParseSimfile(RString type, RString data, Song& song)
{
    NotesLoaderBase* loader;

    if(type == "dwi") {
        loader = new NotesLoaderDWI();
    }
    else if(type == "json") {
        loader = new NotesLoaderJson();
    }
    else if(type == "sm") {
        loader = new SMLoader();
    }
    else if(type == "sma") {
        loader = new SMALoader();
    }
    else if(type == "ssc") {
        loader = new SSCLoader();
    }
    else {
        return false;
    }

    RageFileDriverMem fileDriver;

    RageFile inFile;
    inFile.Open(fileDriver, "tmpIn", RageFile::WRITE);
    inFile.Write(data);
    inFile.Open(fileDriver, "tmpIn", RageFile::READ);

    bool result = loader->LoadFromSimfile(inFile, song);
    delete loader;
    
    if(!result)
        return false;

    return true;
}

RString WriteSimfile(RString type, Song& song)
{
    NotesWriterBase* writer;

    if(type == "dwi") {
        writer = new NotesWriterDWI();
    }
    else if(type == "json") {
        writer = new NotesWriterJson();
    }
    else if(type == "sm") {
        writer = new NotesWriterSM();
    }
    else if(type == "ssc") {
        writer = new NotesWriterSSC();
    }
    else {
        throw std::invalid_argument("Output format not supported");
    }

    RageFileDriverMem fileDriver;
    RageFile outFile;
    outFile.Open(fileDriver, "tmpOut", RageFile::WRITE);

    bool result = writer->Write(outFile, song);
    delete writer;

    if(!result)
        throw std::invalid_argument("Converting failed");

    outFile.Open(fileDriver, "tmpOut", RageFile::READ);
    RString FileString;
    FileString.reserve( outFile.GetFileSize() );
    int iBytesRead = outFile.Read( FileString );

    return FileString;
}

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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "ConvertChart"), Napi::Function::New(env, ConvertChart));
    return exports;
}

NODE_API_MODULE(addon, Init)