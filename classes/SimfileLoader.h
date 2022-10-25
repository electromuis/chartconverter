#pragma once

#include <napi.h>

class SimfileLoader: public Napi::ObjectWrap<SimfileLoader> {
public:
   // Init function for exports
   static Napi::Object Init(Napi::Env env, Napi::Object exports);

   Napi::Value LoadString(const Napi::CallbackInfo &info);

   SimfileLoader(const Napi::CallbackInfo &info);
private:
   static Napi::FunctionReference constructor;
};