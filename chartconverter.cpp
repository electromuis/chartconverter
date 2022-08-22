#include <iostream>
#include <string>

#include "lib/stepmania/NotesLoaderSM.h"
#include "lib/stepmania/NotesWriterJson.h"
#include "lib/stepmania/NotesWriterDWI.h"
#include "lib/stepmania/Song.h"
#include "lib/stepmania/RageFileDriverDirect.h"
#include "lib/stepmania/RageFileDriverMemory.h"
#include "lib/stepmania/RageUtil.h"

using namespace std;

int main(int argc, char** argv)
{
	if(argc < 2) {		
		cout << "No input file specified" << endl;
		return -1;
	}

	Song song;
	SMLoader loader;
	RageFile inputFile;

	{
		RString input = argv[1];
		RString dir = Dirname(input);
		RString fname = "/" + input.substr(dir.length());
		RageFileDriverDirect directDriver(dir);
		
		inputFile.Open(directDriver, fname, RageFile::READ);
	}

	if(!loader.LoadFromSimfile(inputFile, song)) {
		cout << "Failed to read " << argv[1] << endl;
		return -1;
	}

	RageFile outputFile;
	RageFileDriver* fileDriver;

	if(argc > 2) {
		RString output = argv[2];
		RString dir = Dirname(output);
		RString fname = "/" + output.substr(dir.length());
		fileDriver = new RageFileDriverDirect(dir);
		outputFile.Open(*fileDriver, fname, RageFile::WRITE);
	}
	else {
		fileDriver = new RageFileDriverMem();
		outputFile.Open(*fileDriver, "tmp", RageFile::WRITE);
	}

	RString dataOut;
	if(!NotesWriterDWI::Write(outputFile, song)) {
		cout << "Failed to write " << argv[1] << endl;
		return -1;
	}

	if(argc <= 2) {
		outputFile.Open(*fileDriver, "tmp", RageFile::READ);
		RString FileString;
		FileString.reserve( outputFile.GetFileSize() );
		int iBytesRead = outputFile.Read( FileString );
		cout << FileString;
	}

	delete fileDriver;

	return 0;
}