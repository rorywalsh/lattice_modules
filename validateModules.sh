#!/bin/bash
for lib in *; do 
if [ $lib == *.dylib ]; then 
echo "Signing $lib and removing from com.apple.quarantine"; 
sudo codesign -s - $lib --timestamp --deep --force
xattr -d com.apple.quarantine $lib
fi; 
done
