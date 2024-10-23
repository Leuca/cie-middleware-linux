#!/usr/bin/env bash
#
#
SDK_PATH=./cie_sign_sdk
CIE_PATH=./cie-pkcs11
LIBCIE_PATH=./libcie-pkcs11
GIT=$(which git)
CMAKE=$(which cmake)

# Remove pre-compiled static-libs
if [ -d $SDK_PATH/Dependencies ]; then
	rm -rf $SDK_PATH/Dependencies
fi

# Remove CryptoPP headers
if [ -d $SDK_PATH/src/cryptopp ]; then
	rm -rf $SDK_PATH/src/cryptopp
fi
if [ -d $CIE_PATH/Cryptopp ]; then
	rm -rf $CIE_PATH/Cryptopp
fi

# Merge cie_sign_sdk source with cie-pkcs11 source
mkdir -p $LIBCIE_PATH
rm -f $SDK_PATH/CMakeLists.txt
rm -f $SDK_PATH/README.mk
rm -f $SDK_PATH/src/Util/UUC*
cp -r $SDK_PATH/* $LIBCIE_PATH/
cp $CIE_PATH/keys.h $LIBCIE_PATH/include/
rm -f $CIE_PATH/keys.h
rm -f $CIE_PATH/*.a
rm -f $CIE_PATH/Util/funccallinfo.cpp
cp -f $CIE_PATH/Util/UUCStringTable.cpp $LIBCIE_PATH/src/
cp -f $CIE_PATH/Util/UUCStringTable.h $LIBCIE_PATH/include/
cp -f $CIE_PATH/Util/UUCHashtable.hpp $LIBCIE_PATH/include/
rm -f $CIE_PATH/Util/UUC*
cp -rf $CIE_PATH/* $LIBCIE_PATH/src/
rm -f $LIBCIE_PATH/src/Sign/definitions.h

# Restore git repository
if [ "$GIT" != "" ]; then
	$GIT restore .
else
	echo "Warning: git not found"
fi

# Prepare build with CMake
if [ "$CMAKE" != "" ]; then
	$CMAKE \
		-S "." \
		-B "build" \
		-DCMAKE_C_FLAGS_RELEASE:STRING="-DNDEBUG" \
		-DCMAKE_CXX_FLAGS_RELEASE:STRING="-DNDEBUG" \
		-DCMAKE_Fortran_FLAGS_RELEASE:STRING="-DNDEBUG" \
		-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
		-DBUILD_SHARED_LIBS:BOOL=ON
else
	echo "Error: CMake not found"
	exit 1
fi

# Build with CMake
$CMAKE --build "build" -j$(nproc) --verbose
