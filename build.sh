#!/bin/bash

set -e
set -x

srcdir=$ROOT
blddir=$BUILD_DIR
insdir=$INSTALL_DIR

cd $blddir


# WebRTC
export PATH=$PATH:$blddir/depot_tools
export VPYTHON_BYPASS="manually managed python not supported by chrome operations"
export DEPOT_TOOLS_UPDATE=0

#wget http://launchpadlibrarian.net/534747115/libc6_2.23-0ubuntu11.3_amd64.deb
#dpkg -x libc6_2.23-0ubuntu11.3_amd64.deb ./
#export LD_OVERRIDE="$blddir/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2"
#export LD_LIBRARY_PATH_OVERRIDE="$blddir/lib:$blddir/usr/lib"
export GYP_DEFINES="target_arch=$ARCH"

if [ ! -d $blddir/libwebrtc ]; then
	mkdir $blddir/libwebrtc
fi
cd $blddir/libwebrtc
cmake $srcdir/3rdparty/libwebrtc
make -j8


# libyuv
if [ ! -d $blddir/libyuv ]; then
	mkdir $blddir/libyuv
fi
cd $blddir/libyuv
cmake \
	-DCMAKE_INSTALL_PREFIX=$blddir/libyuv \
	$srcdir/3rdparty/libyuv
make -j8
make install

if [ ! -d $insdir/lib/$ARCH_TRIPLET ]; then
	mkdir -p $insdir/lib/$ARCH_TRIPLET
fi
cp -a $blddir/libyuv/lib*.so* $insdir/lib/$ARCH_TRIPLET/


# QWebRTC
if [ ! -d $blddir/qwebrtc ]; then
	mkdir $blddir/qwebrtc
fi
cd $blddir/qwebrtc
cmake \
	-DCMAKE_CXX_FLAGS="-isystem $blddir/libwebrtc/include -isystem $blddir/libyuv/include -L$blddir/libwebrtclib -L$blddir/libyuv -Wno-deprecated-declarations -Wl,-rpath-link,$blddir/libwebrtc/lib" \
	-DCMAKE_C_FLAGS="-isystem $blddir/libwebrtc/include -isystem $blddir/libyuv/include -L$blddir/libwebrtc/lib -L$blddir/libyuv -Wno-deprecated-declarations -Wl,-rpath-link,$blddir/libwebrtc/lib" \
	-DCMAKE_LD_FLAGS="-L$blddir/libwebrtc/lib -L$blddir/libyuv/lib" \
	-DCMAKE_LIBRARY_PATH="$blddir/libwebrtc/lib:$blddir/libyuv/lib" \
	-DWebRTCOutDir="$blddir/libwebrtc/webrtc/src/out/Release/obj/webrtc/" \
	-DWebRTCLibDir="$blddir/libwebrtc/webrtc/src/out/Release/obj/webrtc/" \
		$srcdir/3rdparty/qwebrtc
make -j8
make install
if [ ! -d $insdir/lib/$ARCH_TRIPLET ]; then
	mkdir -p $insdir/lib/$ARCH_TRIPLET
fi
cp -a $blddir/qwebrtc/lib*.so* $insdir/lib/$ARCH_TRIPLET/


# Main project
if [ ! -d $blddir/project ]; then
	mkdir $blddir/project
fi
cd $blddir/project
cmake \
	-DCMAKE_CXX_FLAGS="-isystem $blddir/libwebrtc/include -isystem $blddir/qwebrtc/include -isystem $blddir/libyuv/include -L$blddir/libwebrtc -L$blddir/qwebrtc -L$blddir/libyuv -Wno-deprecated-declarations -Wl,-rpath-link,$blddir/qwebrtc" \
	-DCMAKE_C_FLAGS="-isystem $blddir/libwebrtc/include -isystem $blddir/qwebrtc/include -isystem $blddir/libyuv/include -L$blddir/libwebrtc -L$blddir/qwebrtc -L$blddir/libyuv -Wno-deprecated-declarations -Wl,-rpath-link,$blddir/qwebrtc" \
	-DCMAKE_LD_FLAGS="-L$blddir/qwebrtc -L$blddir/libyuv/lib" \
	-DCMAKE_LIBRARY_PATH="$blddir/qwebrtc:$blddir/libyuv/lib" \
	-DCMAKE_INSTALL_PREFIX=$insdir \
		$srcdir
make -j8
make install