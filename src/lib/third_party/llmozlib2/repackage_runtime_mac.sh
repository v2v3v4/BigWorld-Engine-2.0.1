#!/bin/bash

# This script adds or updates libllmozlib2.dylib in mozilla-universal-darwin.tgz.
sandbox_location="$1"

xcodebuild -project llmozlib2.xcodeproj -target llmozlib2 -configuration 'Release' build

if [ -f build/Release/libllmozlib2.dylib ]; then
	echo "libllmozlib2.dylib found"
else
	echo "libllmozlib2.dylib not found, please build the Release target in llmozlib.xcodeproj"
	exit 1
fi

if [ -d "$sandbox_location" ]; then
	echo "Using sandbox at $sandbox_location"
else
	echo "Sandbox $sandbox_location not found, exiting."
	exit 1
fi


# exit on any error
set -e

echo "Unpacking mozilla-universal-darwin-original.tgz..."

# expand the runtime archive
rm -rf mozilla-universal-darwin
mkdir -p mozilla-universal-darwin
(cd mozilla-universal-darwin && tar -zxf ../mozilla-universal-darwin-original.tgz)

echo "Adding libllmozlib2.dylib..."

# copy in the latest built version of llmozlib2
cp ./build/Release/libllmozlib2.dylib mozilla-universal-darwin

echo "Removing unneeded pieces..."

# remove bits of the mozilla runtime that we don't use
rm -rf mozilla-universal-darwin/Info.plist
rm -rf mozilla-universal-darwin/Resources
rm -rf mozilla-universal-darwin/run-mozilla.sh
rm -rf mozilla-universal-darwin/updater.app
rm -rf mozilla-universal-darwin/xpicleanup
rm -rf mozilla-universal-darwin/xulrunner
rm -rf mozilla-universal-darwin/xulrunner-bin

echo "Creating mozilla-universal-darwin.tgz..."

# tar up the runtime and llmozlib2 
rm -f ./mozilla-universal-darwin.tgz
tar -zcf ./mozilla-universal-darwin.tgz \
	-C mozilla-universal-darwin . 

#rm -rf mozilla-universal-darwin

#echo '###############################'
#echo 'please copy mozilla-universal-darwin.tgz to indra/newview in your sandbox.'

echo "Copying mozilla-universal-darwin.tgz to $sandbox_location/indra/newview..."
cp mozilla-universal-darwin.tgz $sandbox_location/indra/newview/

echo "Copying stub library to $sandbox_location/libraries/universal-darwin/..."
cp build/Release_stub/libllmozlib2.dylib $sandbox_location/libraries/universal-darwin/lib_debug/libllmozlib2.dylib
cp build/Release_stub/libllmozlib2.dylib $sandbox_location/libraries/universal-darwin/lib_release/libllmozlib2.dylib

echo "done!"

