#!/bin/bash

# Post build step. Pass in Release or Debug on command line

cp -rf output/$1/obj/DevDriverAPI/libDevDriverAPI.so $1/DevDriverAPI/lib
cp -rf ../../source/DevDriverAPI/DevDriverAPI.h $1/DevDriverAPI/include

