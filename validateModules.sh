#!/bin/bash
libDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
for lib in $libDir/*; do 
if [[ $lib == *.dylib ]]; then 
echo "Signing $lib and removing from com.apple.quarantine"; 
sudo codesign -s - $lib --timestamp --deep --force
xattr -d com.apple.quarantine $lib
fi; 
done
