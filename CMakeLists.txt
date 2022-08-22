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
	lib/stepmania/NotesLoaderSM.cpp
	lib/stepmania/NotesLoaderSMA.cpp
	lib/stepmania/NotesLoaderSSC.cpp
	lib/stepmania/NotesLoaderDWI.cpp
	lib/stepmania/NotesLoaderSM.cpp

	lib/stepmania/NotesWriterJson.cpp
	lib/stepmania/NotesWriterDWI.cpp
)

if (CMAKE_JS_VERSION)
	add_library(chartconverterjs SHARED
		${SHARED_SOURCES}
		binding.cpp
		${CMAKE_JS_SRC}
	)

	message("Tom: ${TOMCRYPT_LIBRARIES}")
	set_target_properties(chartconverterjs PROPERTIES PREFIX "" SUFFIX ".node")
	target_link_libraries(chartconverterjs PRIVATE ${CMAKE_JS_LIB} ${TOMCRYPT_LIBRARIES})

	execute_process(COMMAND node -p "require('node-addon-api').include"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE NODE_ADDON_API_DIR
	)
	# strip `"` and `\n` from the output above
	string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
	string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

	add_compile_definitions(chartconverterjs PRIVATE
		CMAKE_POWERED
		HAVE_SNPRINTF
	)

	include_directories(${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
else()
	add_executable(chartconverter
		${SHARED_SOURCES}
		chartconverter.cpp
	)

	add_compile_definitions(chartconverter PRIVATE
		CMAKE_POWERED
		HAVE_SNPRINTF
	)

	target_link_libraries(chartconverter PUBLIC ${TOMCRYPT_LIBRARIES})
endif()
