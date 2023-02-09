cmake_minimum_required(VERSION 3.13.4)

project(chartconverter)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Modules)
find_package(TomCrypt REQUIRED)

include(lib/Configure.cmake)

include_directories(
	lib/jsoncpp
	lib/pcre
	lib/src/headers
	${TOMCRYPT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(SHARED_SOURCES
	StepmaniaJS.cpp

	lib/jsoncpp/jsoncpp.cpp

	lib/stepmania/RageFile.cpp
	lib/stepmania/RageFileBasic.cpp
	lib/stepmania/RageFileDriver.cpp
	lib/stepmania/RageFileDriverMemory.cpp
	lib/stepmania/RageFileDriverDirect.cpp
	lib/stepmania/RageFileDriverDirectHelpers.cpp
	lib/stepmania/RageUtil_FileDB.cpp
	
	lib/stepmania/RageUtil.cpp
	lib/stepmania/RageLog.cpp
	lib/stepmania/GameManager.cpp
	
	lib/stepmania/Steps.cpp
	lib/stepmania/Song.cpp
	lib/stepmania/TimingData.cpp
	lib/stepmania/TimingSegments.cpp
	lib/stepmania/NoteData.cpp
	lib/stepmania/NoteDataUtil.cpp
	lib/stepmania/Difficulty.cpp
	lib/stepmania/EnumHelper.cpp
	lib/stepmania/global.cpp
	lib/stepmania/NoteTypes.cpp
	lib/stepmania/GameConstantsAndTypes.cpp
	lib/stepmania/CryptHelpers.cpp
	lib/stepmania/CryptManager.cpp
	lib/stepmania/PlayerNumber.cpp
	lib/stepmania/RadarValues.cpp
	lib/stepmania/RageUtil_CachedObject.cpp
	lib/stepmania/MsdFile.cpp
	lib/stepmania/RageUtil_CharConversions.cpp
	lib/stepmania/JsonUtil.cpp
	
	lib/stepmania/NotesLoader.cpp
	lib/stepmania/NotesLoaderDWI.cpp
	lib/stepmania/NotesLoaderJson.cpp
	lib/stepmania/NotesLoaderSM.cpp
	lib/stepmania/NotesLoaderSMA.cpp
	lib/stepmania/NotesLoaderSSC.cpp

	lib/stepmania/NotesWriterDWI.cpp
	lib/stepmania/NotesWriterJson.cpp
	lib/stepmania/NotesWriterSM.cpp
	lib/stepmania/NotesWriterSSC.cpp
	
	
)

if (NOT CMAKE_JS_VERSION)
	error("JS libs not found")
endif()

add_library(StepmaniaJS SHARED
	Bindings.cpp
	
	classes/SimfileLoader.cpp
	classes/SongWrap.cpp
	classes/StepsWrap.cpp

	${CMAKE_JS_SRC}
	${SHARED_SOURCES}
)

message("Tom: ${TOMCRYPT_LIBRARIES}")
set_target_properties(StepmaniaJS PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(StepmaniaJS PRIVATE ${CMAKE_JS_LIB} ${TOMCRYPT_LIBRARIES})

execute_process(COMMAND node -p "require('node-addon-api').include"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE NODE_ADDON_API_DIR
)
# strip `"` and `\n` from the output above
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

add_compile_definitions(StepmaniaJS PRIVATE
	CMAKE_POWERED
	HAVE_SNPRINTF
)

include_directories(${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})