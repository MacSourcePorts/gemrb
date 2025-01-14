# game/app specific values
export APP_VERSION="0.9"
export ICONSDIR="artwork"
export ICONSFILENAME="gemrb"
export PRODUCT_NAME="gemrb"
export EXECUTABLE_NAME="gemrb"
export PKGINFO="APPLGRB"
export COPYRIGHT_TEXT="Infinity Engine © 1998 Bioware. All rights reserved."

#constants
source ../MSPScripts/constants.sh

export HIGH_RESOLUTION_CAPABLE="true"
export BUNDLE_ID="org.gemrb.gemrb"

rm -rf ${BUILT_PRODUCTS_DIR}

# create makefiles with cmake, perform builds with make
rm -rf ${X86_64_BUILD_FOLDER}
mkdir ${X86_64_BUILD_FOLDER}
cd ${X86_64_BUILD_FOLDER}
cmake \
-DDISABLE_WERROR=1 \
-DCMAKE_OSX_ARCHITECTURES=x86_64  \
-DCMAKE_BUILD_TYPE=Release  \
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.12  \
-DCMAKE_PREFIX_PATH=/usr/local  \
-DCMAKE_INSTALL_PREFIX=/usr/local  \
-DPYTHON_LIBRARY=/usr/local/Frameworks/Python.framework/Versions/3.9/lib/libpython3.9.dylib  \
-DPYTHON_INCLUDE_DIR=/usr/local/Frameworks/Python.framework/Versions/3.9/include/python3.9  \
..
make
install_name_tool -change @loader_path/../Frameworks/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib gemrb/${PRODUCT_NAME}.app/Contents/PlugIns/x86_64/SDLVideo.so
install_name_tool -change @loader_path/../Frameworks/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib gemrb/${PRODUCT_NAME}.app/Contents/PlugIns/x86_64/SDLAudio.so
install_name_tool -change @loader_path/../Frameworks/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib gemrb/${PRODUCT_NAME}.app/Contents/PlugIns/x86_64/OpenALAudio.so
# codesign --force --deep -s - ./gemrb/${PRODUCT_NAME}.app
cp -a gemrb/${PRODUCT_NAME}.app .
# mv gemrb/${PRODUCT_NAME}.app ${PRODUCT_NAME}.app

cd ..
rm -rf ${ARM64_BUILD_FOLDER}
mkdir ${ARM64_BUILD_FOLDER}
cd ${ARM64_BUILD_FOLDER}
cmake  \
-DDISABLE_WERROR=1  \
-DCMAKE_OSX_ARCHITECTURES=arm64  \
-DCMAKE_BUILD_TYPE=Release  \
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.12  \
-DCMAKE_PREFIX_PATH=/opt/Homebrew  \
-DCMAKE_INSTALL_PREFIX=/opt/Homebrew  \
-DPYTHON_LIBRARY=/opt/Homebrew/Frameworks/Python.framework/Versions/3.9/lib/libpython3.9.dylib  \
-DPYTHON_INCLUDE_DIR=/opt/Homebrew/Frameworks/Python.framework/Versions/3.9/include/python3.9  \
..
make
install_name_tool -change @loader_path/../Frameworks/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib gemrb/${PRODUCT_NAME}.app/Contents/PlugIns/arm64/SDLVideo.so
install_name_tool -change @loader_path/../Frameworks/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib gemrb/${PRODUCT_NAME}.app/Contents/PlugIns/arm64/SDLAudio.so
install_name_tool -change @loader_path/../Frameworks/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib gemrb/${PRODUCT_NAME}.app/Contents/PlugIns/arm64/OpenALAudio.so
# codesign --force --deep -s - ./gemrb/${PRODUCT_NAME}.app
cp -a gemrb/${PRODUCT_NAME}.app .
# mv gemrb/${PRODUCT_NAME}.app ${PRODUCT_NAME}.app

cd ..

# create the app bundle
"../MSPScripts/build_app_bundle.sh" skiplipo

source ../MSPScripts/signing_values.local

lipo "${X86_64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}" "${ARM64_BUILD_FOLDER}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}" -output "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_FOLDER_PATH}/${EXECUTABLE_NAME}" -create

cp -a "${X86_64_BUILD_FOLDER}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/." "${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}"

mkdir -p "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/demo"
cp -a "${X86_64_BUILD_FOLDER}/${CONTENTS_FOLDER_PATH}/demo/." "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/demo"

mkdir -p "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}"
echo lipo "${X86_64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.0.9.0-git.dylib" "${ARM64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.0.9.0-git.dylib" -output "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.0.9.0-git.dylib" -create
lipo "${X86_64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.0.9.0-git.dylib" "${ARM64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.0.9.0-git.dylib" -output "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.0.9.0-git.dylib" -create
cp -R "${X86_64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.dylib" "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/libgemrb_core.dylib"
echo lipo "${X86_64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libSDL2-2.0.0.dylib" "${ARM64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libSDL2-2.0.0.dylib" -output "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/libSDL2-2.0.0.dylib" -create
lipo "${X86_64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libSDL2-2.0.0.dylib" "${ARM64_BUILD_FOLDER}/${FRAMEWORKS_FOLDER_PATH}/libSDL2-2.0.0.dylib" -output "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/libSDL2-2.0.0.dylib" -create

mkdir -p "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/PlugIns/x86_64"
cp -a "${X86_64_BUILD_FOLDER}/${CONTENTS_FOLDER_PATH}/PlugIns/x86_64/." "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/PlugIns/x86_64"

mkdir release/gemrb.app/Contents/Frameworks/x86_64

cp -a /usr/local/opt/python@3.9/Frameworks/Python.framework release/gemrb.app/Contents/Frameworks/x86_64
echo codesign --force timestamp --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/Current
codesign --force timestamp --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/Current

install_name_tool -change /usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/Python @executable_path/../Frameworks/x86_64/Python.framework/Versions/3.9/Python release/gemrb.app/Contents/PlugIns/x86_64/GUIScript.so

cp /usr/local/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib release/gemrb.app/Contents/Frameworks/x86_64
dylibbundler -od -b -x release/gemrb.app/Contents/Frameworks/x86_64/libSDL2_mixer-2.0.0.dylib -d release/gemrb.app/Contents/MacOS/libs-x86_64 -p @executable_path/libs-x86_64/
install_name_tool -change /usr/local/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib @executable_path/../Frameworks/x86_64/libSDL2_mixer-2.0.0.dylib release/gemrb.app/Contents/PlugIns/x86_64/SDLAudio.so

install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbisfile.3.dylib @executable_path/libs-x86_64/libvorbisfile.3.dylib release/gemrb.app/Contents/PlugIns/x86_64/OGGReader.so

cp /usr/local/opt/libpng/lib/libpng16.16.dylib release/gemrb.app/Contents/MacOS/libs-x86_64
install_name_tool -change /usr/local/opt/libpng/lib/libpng16.16.dylib @executable_path/libs-x86_64/libpng16.16.dylib release/gemrb.app/Contents/PlugIns/x86_64/PNGImporter.so

cp /usr/local/opt/freetype/lib/libfreetype.6.dylib release/gemrb.app/Contents/MacOS/libs-x86_64
install_name_tool -change /usr/local/opt/freetype/lib/libfreetype.6.dylib @executable_path/libs-x86_64/libfreetype.6.dylib release/gemrb.app/Contents/PlugIns/x86_64/TTFImporter.so
install_name_tool -change /usr/local/opt/libpng/lib/libpng16.16.dylib @executable_path/libs-x86_64/libpng16.16.dylib release/gemrb.app/Contents/MacOS/libs-x86_64/libfreetype.6.dylib

mkdir -p "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/PlugIns/arm64"
cp -a "${ARM64_BUILD_FOLDER}/${CONTENTS_FOLDER_PATH}/PlugIns/arm64/." "${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/PlugIns/arm64"

mkdir release/gemrb.app/Contents/Frameworks/arm64

cp -a /opt/homebrew/opt/python@3.9/Frameworks/Python.framework release/gemrb.app/Contents/Frameworks/arm64
echo codesign --force timestamp --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/Current
codesign --force timestamp --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/Current

install_name_tool -change /opt/homebrew/opt/python@3.9/Frameworks/Python.framework/Versions/3.9/Python @executable_path/../Frameworks/arm64/Python.framework/Versions/3.9/Python release/gemrb.app/Contents/PlugIns/arm64/GUIScript.so

cp /opt/homebrew/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib release/gemrb.app/Contents/Frameworks/arm64
dylibbundler -od -b -x release/gemrb.app/Contents/Frameworks/arm64/libSDL2_mixer-2.0.0.dylib -d release/gemrb.app/Contents/MacOS/libs-arm64 -p @executable_path/libs-arm64/
install_name_tool -change /opt/homebrew/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib @executable_path/../Frameworks/arm64/libSDL2_mixer-2.0.0.dylib release/gemrb.app/Contents/PlugIns/arm64/SDLAudio.so

install_name_tool -change /opt/homebrew/opt/libvorbis/lib/libvorbisfile.3.dylib @executable_path/libs-arm64/libvorbisfile.3.dylib release/gemrb.app/Contents/PlugIns/arm64/OGGReader.so

cp /opt/homebrew/opt/libpng/lib/libpng16.16.dylib release/gemrb.app/Contents/MacOS/libs-arm64
install_name_tool -change /opt/homebrew/opt/libpng/lib/libpng16.16.dylib @executable_path/libs-arm64/libpng16.16.dylib release/gemrb.app/Contents/PlugIns/arm64/PNGImporter.so

cp /opt/homebrew/opt/freetype/lib/libfreetype.6.dylib release/gemrb.app/Contents/MacOS/libs-arm64
install_name_tool -change /opt/homebrew/opt/freetype/lib/libfreetype.6.dylib @executable_path/libs-arm64/libfreetype.6.dylib release/gemrb.app/Contents/PlugIns/arm64/TTFImporter.so
install_name_tool -change /opt/homebrew/opt/libpng/lib/libpng16.16.dylib @executable_path/libs-arm64/libpng16.16.dylib release/gemrb.app/Contents/MacOS/libs-arm64/libfreetype.6.dylib


# TODO: if this works figure out a better way
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/bin/python3.9
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_asyncio.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_bisect.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_blake2.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_bz2.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_cn.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_hk.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_iso2022.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_jp.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_kr.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_tw.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_contextvars.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_crypt.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_csv.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_ctypes_test.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_ctypes.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_curses_panel.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_curses.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_datetime.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_dbm.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_decimal.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_elementtree.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_gdbm.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_hashlib.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_heapq.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_json.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_lsprof.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_lzma.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_md5.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_multibytecodec.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_multiprocessing.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_opcode.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_pickle.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_posixshmem.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_posixsubprocess.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_queue.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_random.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_scproxy.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha1.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha256.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha3.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha512.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_socket.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sqlite3.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_ssl.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_statistics.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_struct.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testbuffer.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testcapi.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testimportmultiple.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testinternalcapi.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testmultiphase.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_uuid.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_xxsubinterpreters.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_xxtestfuzz.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_zoneinfo.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/array.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/audioop.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/binascii.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/cmath.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/fcntl.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/grp.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/math.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/mmap.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/nis.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/parser.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/pyexpat.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/readline.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/resource.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/select.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/syslog.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/termios.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/unicodedata.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/xxlimited.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/zlib.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/arm64/Python.framework/Versions/3.9/Resources/Python.app/Contents/MacOS/Python
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/bin/python3.9
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_asyncio.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_bisect.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_blake2.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_bz2.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_cn.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_hk.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_iso2022.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_jp.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_kr.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_codecs_tw.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_contextvars.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_crypt.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_csv.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_ctypes_test.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_ctypes.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_curses_panel.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_curses.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_datetime.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_dbm.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_decimal.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_elementtree.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_gdbm.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_hashlib.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_heapq.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_json.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_lsprof.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_lzma.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_md5.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_multibytecodec.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_multiprocessing.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_opcode.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_pickle.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_posixshmem.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_posixsubprocess.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_queue.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_random.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_scproxy.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha1.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha256.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha3.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sha512.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_socket.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_sqlite3.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_ssl.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_statistics.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_struct.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testbuffer.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testcapi.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testimportmultiple.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testinternalcapi.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_testmultiphase.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_uuid.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_xxsubinterpreters.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_xxtestfuzz.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/_zoneinfo.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/array.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/audioop.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/binascii.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/cmath.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/fcntl.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/grp.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/math.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/mmap.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/nis.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/parser.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/pyexpat.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/readline.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/resource.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/select.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/syslog.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/termios.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/unicodedata.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/xxlimited.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/lib/python3.9/lib-dynload/zlib.cpython-39-darwin.so
codesign --force --timestamp --options runtime --sign "${SIGNING_IDENTITY}" release/gemrb.app/Contents/Frameworks/x86_64/Python.framework/Versions/3.9/Resources/Python.app/Contents/MacOS/Python

#sign and notarize
export ENTITLEMENTS_FILE="platforms/apple/gemrb.entitlements"
"../MSPScripts/sign_and_notarize.sh" "$1" entitlements