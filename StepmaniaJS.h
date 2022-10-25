#pragma once

#include "lib/stepmania/global.h"

class Song;
class Steps;

bool ParseSimfile(RString type, RString data, Song& song);
RString WriteSimfile(RString type, Song& song);
RString HashSimfile(RString type, Steps& chart);