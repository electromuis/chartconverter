#pragma once

#include <napi.h>
#include <memory>

class Song;

class SongWrap : public Napi::ObjectWrap<SongWrap> {
public:
    static Napi::Object Create(Napi::Env env, std::shared_ptr<Song> song);
    SongWrap(const Napi::CallbackInfo& info);

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Value FromString(const Napi::CallbackInfo& info);

    Napi::Value GetTitle(const Napi::CallbackInfo &info);
    Napi::Value GetArtist(const Napi::CallbackInfo &info);
    Napi::Value GetSteps(const Napi::CallbackInfo &info);

protected:
    std::shared_ptr<Song> song;
};