#!/bin/bash

##
# for MING, ensure that we have .exe files in src and samples
os=$(uname)
if [ "${os:0:4}" == "MING" ]; then
	cp ../src/.libs/*.exe     ../src/
	cp ../src/.libs/*.dll     ../src/
	cp ../samples/.libs/*.exe ../samples/
fi

# That's all Folks!
##
