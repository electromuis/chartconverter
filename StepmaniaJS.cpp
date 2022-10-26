#include "StepmaniaJS.h"

#include <iostream>
#include <string>

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
#include "lib/stepmania/Steps.h"
#include "lib/stepmania/RageFileDriverDirect.h"
#include "lib/stepmania/RageFileDriverMemory.h"
#include "lib/stepmania/RageUtil.h"
#include "lib/stepmania/CryptManager.h"

using namespace std;

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