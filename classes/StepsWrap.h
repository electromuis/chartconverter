#pragma once

#include <napi.h>
#include <memory>

class Steps;

class StepsWrap : public Napi::ObjectWrap<StepsWrap> {
public:
    static Napi::Object Create(Napi::Env env, Steps* steps);
    StepsWrap(const Napi::CallbackInfo& info);

    Napi::Value GetMeter(const Napi::CallbackInfo &info);
    Napi::Value GetDifficulty(const Napi::CallbackInfo &info);
    Napi::Value GetCredit(const Napi::CallbackInfo &info);
    Napi::Value GetStyle(const Napi::CallbackInfo &info);
    Napi::Value GetHash(const Napi::CallbackInfo &info);
    Napi::Value GetHashInput(const Napi::CallbackInfo &info);

protected:
    RString NormallizedChart();
    Steps* steps;
};