#! /bin/bash

set -e

cp vlc-android/vlc/build-android-arm-linux-androideabi/ndk/libs/armeabi-v7a/*.so Assets/VLCUnity/Plugins/Android/armeabi-v7a
cp vlc-android/libvlc/jni/libs/armeabi-v7a/libvlcjni.so Assets/VLCUnity/Plugins/Android/armeabi-v7a