ADD_SUBDIRECTORY( core )

# GUIScripts and override go directly in the bundle
SET(RESOURCE_LIST "GUIScripts" "override" "unhardcoded" "../demo")
IF (OPENGL_BACKEND)
	LIST(APPEND RESOURCE_LIST "plugins/SDLVideo/Shaders")
ENDIF()
SET(RESOURCE_FILES "")

FOREACH(RESOURCE_DIR_NAME ${RESOURCE_LIST})
	SET(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_DIR_NAME}")
	IF (APPLE)
		FILE(GLOB CHILDREN RELATIVE "${RESOURCE_DIR}" ${RESOURCE_DIR}/*)
		FOREACH(CHILD ${CHILDREN})
			IF(IS_DIRECTORY "${RESOURCE_DIR}/${CHILD}")
				# be wary of .DS_Store files. glob them out by using [A-Za-z]*.*
				FILE(GLOB RESOURCE_DIR_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${RESOURCE_DIR}/${CHILD}/[A-Za-z]*.*")
				SET_SOURCE_FILES_PROPERTIES(
					${RESOURCE_DIR_FILES}
					PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${RESOURCE_DIR_NAME}/${CHILD}
				)
				SET(RESOURCE_FILES ${RESOURCE_FILES} ${RESOURCE_DIR_FILES})
			ELSEIF(NOT ${CHILD} MATCHES "\\.DS_Store")
				SET_SOURCE_FILES_PROPERTIES(
					"${RESOURCE_DIR}/${CHILD}"
					PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${RESOURCE_DIR_NAME}
				)
				SET(RESOURCE_FILES ${RESOURCE_FILES} "${RESOURCE_DIR_NAME}/${CHILD}")
			ENDIF()
		ENDFOREACH()
	ELSE (APPLE)
		INSTALL(DIRECTORY ${RESOURCE_DIR} DESTINATION "${DATA_DIR}")
	ENDIF (APPLE)
ENDFOREACH()

IF(APPLE) #application bundle generation
	# icon
	SET_SOURCE_FILES_PROPERTIES(
		../artwork/gemrb.icns
		PROPERTIES MACOSX_PACKAGE_LOCATION Resources
	)

	# default values
	SET_SOURCE_FILES_PROPERTIES(
		../apple/defaults.plist
		PROPERTIES MACOSX_PACKAGE_LOCATION Resources
	)

	SET(MACOSX_BUNDLE_ICON_FILE gemrb)
	SET(PRODUCT_NAME "GemRB") # for info.plist
	
	ADD_EXECUTABLE(gemrb MACOSX_BUNDLE
					../platforms/apple/CocoaWrapper/mac/CocoaWrapper.mm
					../platforms/apple/AppleLogger.mm
					../platforms/apple/EmptyStringTransformer.m
					../artwork/gemrb.icns
					../platforms/apple/defaults.plist
					${RESOURCE_FILES})

	# we already link to Cocoa and we need to modify this with the ‘dylib_file’ option for non-framework SDL builds
	message(STATUS "Starting SDL_LIBRARY and cmake path: '${SDL_LIBRARY}' (${libdir})")
	set(OLD_SDL_LIB ${SDL_LIBRARY})
	STRING(REPLACE "-framework Cocoa" "" SDL_LIBRARY ${SDL_LIBRARY})
	if(${OLD_SDL_LIB} STREQUAL ${SDL_LIBRARY} AND SDL_BACKEND STREQUAL "SDL2")
		set(SDL_BASENAME "libSDL2-2.0.0.dylib")
		set(SDL_LIBRARY "${libdir}/${SDL_BASENAME}")
		message(STATUS "Using path set by SDL2 itself: ${SDL_LIBRARY}")
	else()
		GET_FILENAME_COMPONENT(SDL_LIBRARY ${SDL_LIBRARY} REALPATH)
		GET_FILENAME_COMPONENT(SDL_BASENAME ${SDL_LIBRARY} NAME)
	endif()

	SET(BUNDLE_FRAMEWORK_PATH "${CMAKE_BINARY_DIR}/gemrb/${PROJECT_NAME}.app/Contents/Frameworks")
					
	MESSAGE(STATUS "Copying SDL: ${SDL_LIBRARY}")
	FILE(COPY ${SDL_LIBRARY} DESTINATION ${BUNDLE_FRAMEWORK_PATH} FILE_PERMISSIONS OWNER_READ OWNER_WRITE)

	SET(SDL_LIBRARY "${BUNDLE_FRAMEWORK_PATH}/${SDL_BASENAME}")
	
	IF(NOT SDL_BASENAME MATCHES ".framework")
		# change the "install name" of the copied SDL
		ADD_CUSTOM_COMMAND(TARGET gemrb PRE_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id "@loader_path/../Frameworks/${SDL_BASENAME}" "${SDL_LIBRARY}")
	ENDIF()

	SET(BUNDLE_LOADER "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/MacOS/GemRB") # BUNDLE_LOADER is used by plugins needing to extend the cocoa wrapper (SDLVideo).
	STRING(APPEND CMAKE_MODULE_LINKER_FLAGS " -bundle_loader \"${BUNDLE_LOADER}\"")
ELSE(APPLE)
	ADD_SUBDIRECTORY( docs )
	ADD_SUBDIRECTORY( tests )

	SET(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../platforms")

	IF(WIN32)
		ADD_EXECUTABLE(gemrb ${PLATFORM_DIR}/windows/GemRB.cpp ${PLATFORM_DIR}/windows/Win32Console.cpp ${PLATFORM_DIR}/windows/window.rc)
	ELSEIF(VITA)
		ADD_EXECUTABLE(gemrb ${PLATFORM_DIR}/vita/GemRB.cpp ${PLATFORM_DIR}/vita/VitaLogger.cpp)
	ELSEIF(ANDROID)
		ADD_EXECUTABLE(gemrb ${PLATFORM_DIR}/android/GemRB.cpp ${PLATFORM_DIR}/android/AndroidLogger.cpp)
	ELSE(WIN32)
		ADD_EXECUTABLE(gemrb GemRB.cpp )
	ENDIF()

	INSTALL( FILES "${CMAKE_CURRENT_BINARY_DIR}/GemRB.cfg.sample" GemRB.cfg.noinstall.sample DESTINATION ${EXAMPLE_CONF_DIR} )
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/GemRB.cfg.sample" RENAME GemRB.cfg DESTINATION ${EXAMPLE_CONF_DIR})
ENDIF(APPLE)

# we need to add gemrb target before processing plugins or COCOA plugins don't work right.
ADD_SUBDIRECTORY( plugins )

# create a compilable icon from a bmp
FUNCTION(create_icon file output filename)
	FILE(READ ${file} filedata HEX)
	# convert hex data for C compatibility
	STRING(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
	FILE(WRITE "${PROJECT_BINARY_DIR}/${output}" "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")
ENDFUNCTION()
create_icon("${CMAKE_SOURCE_DIR}/artwork/gemrb.bmp" "gemrb-icon.h" "gemrb_icon")

SET_TARGET_PROPERTIES(gemrb PROPERTIES INSTALL_RPATH ${LIB_DIR})
INCLUDE_DIRECTORIES( ${SDL_INCLUDE_DIR} )
IF(WIN32)
 TARGET_LINK_LIBRARIES(gemrb gemrb_core)
ELSE(WIN32)
 IF(APPLE)
	# custom info.plist
	SET(PRODUCT_BUNDLE_IDENTIFIER "org.gemrb.gemrb")
	SET(EXECUTABLE_NAME "GemRB")
	SET_TARGET_PROPERTIES(gemrb PROPERTIES BUNDLE True MACOSX_BUNDLE_INFO_PLIST ../platforms/apple/osx/Info.plist OUTPUT_NAME GemRB)

	# compile nib
	ADD_CUSTOM_COMMAND (TARGET gemrb POST_BUILD 
                      COMMAND ${IBTOOL} --errors --warnings --notices --output-format human-readable-text 
                              --compile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/GemRB.nib 
                            ${CMAKE_CURRENT_SOURCE_DIR}/../platforms/apple/CocoaWrapper/mac/nibs/GemRB.xib
                      COMMENT "Compiling GemRB.xib")

	if (STATIC_LINK)
		TARGET_LINK_LIBRARIES(gemrb
			  ${COCOA_LIBRARY_PATH} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT}
			 gemrb_core ${plugins}  -Wl,-all_load)
	else (STATIC_LINK)
		TARGET_LINK_LIBRARIES(gemrb gemrb_core
			  ${COCOA_LIBRARY_PATH} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
	endif (STATIC_LINK)
 ELSE(APPLE)
  if (STATIC_LINK)
	  TARGET_LINK_LIBRARIES(gemrb ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT}
		  -Wl,--whole-archive gemrb_core ${plugins} -Wl,--no-whole-archive)
  else (STATIC_LINK)
	  TARGET_LINK_LIBRARIES(gemrb gemrb_core ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
  endif (STATIC_LINK)
 ENDIF(APPLE)
ENDIF(WIN32)

# preconfigure the sample config with the selected paths
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/GemRB.cfg.sample.in"
  "${CMAKE_CURRENT_BINARY_DIR}/GemRB.cfg.sample"
  IMMEDIATE @ONLY
)

INSTALL( TARGETS gemrb DESTINATION ${BIN_DIR} )

# optional script to help deploy dependencies when building with windows.
IF(VCPKG_AUTO_DEPLOY)
	INCLUDE(${CMAKE_SOURCE_DIR}/platforms/windows/vcpkg_deps.cmake)
ENDIF()

IF(VITA)
	INCLUDE(${CMAKE_SOURCE_DIR}/platforms/vita/vita_build_vpk.cmake)
ENDIF(VITA)
