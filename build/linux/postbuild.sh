#!/bin/bash

# Post build step. Pass in Release or Debug on command line

cp -rf output/$1/obj/RGP_API/libRGP_API.so $1/RGP_API/lib
