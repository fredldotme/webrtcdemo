#!/bin/bash

set -e

cd build/aarch64-linux-gnu/app/libwebrtc/

rm -rf webrtc/stamp/webrtc-src-build webrtc/stamp/webrtc-src-configure webrtc/stamp/webrtc-src-done webrtc/build lib/ include
cd webrtc/src
git add webrtc/
git status
git diff --cached > ~/Projects/open/webrtcdemo/3rdparty/libwebrtc/patches/2.patch
git diff --cached > ./patches/2.patch
cd build
git reset --hard HEAD
cd ..
git reset --hard HEAD
cd ../../
