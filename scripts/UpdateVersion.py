#! /usr/bin/python
#
# Simple script to add the build number and build date to the version strings used by DevDriverTools

import fileinput
import re
import sys
import os
import stat
import time
from datetime import date

usageString = "Usage: UpdateVersion.py <rootDir> <buildNumber> <optional build name>\n"

# Check for correct number of arguments
if ((len(sys.argv) != 3) and (len(sys.argv) != 4)):
    print usageString
    exit(1)

# Assign arguments to variables
rootDir = sys.argv[1]    
buildNumber = sys.argv[2]

if (len(sys.argv) == 4):
    buildName = "(" + sys.argv[3] + ")"
else:
    buildName = ""
    
# Validate arguments
if (os.path.isdir(rootDir) == False):
    print ("\nError: directory \"" + rootDir + "\" does not exist\n")
    print usageString
    exit(1)

# Save todays date
today = date.today()

# Edit Version.h
versionFilePath = os.path.join(rootDir, os.path.normpath("source/Common/Version.h"))

if (os.path.isfile(versionFilePath) == False):
    print ("\nError: file \"" + versionFilePath + "\" does not exist\n")
    print usageString
    exit(1)

# Iterate over file making regular expression substitutions
for line in fileinput.input(versionFilePath, inplace=True):
    # replace all references to build number
    line = re.sub(r'(.*DEV_DRIVER_TOOLS_BUILD_NUMBER\s+)\d+(\D.*)', r'\g<1>' + buildNumber + '\g<2>', line.rstrip())

    # update build using todays date
    line = re.sub(r'(.*DEV_DRIVER_TOOLS_BUILD_DATE_STRING.*").*(".*)', r'\g<1>' + str(today.month) + "/" + str(today.day) + "/" + str(today.year) + '\g<2>', line.rstrip())

    # if optional build name is provided - then add to the application name and version string
    m = re.match('(.*DEV_DRIVER_TOOLS_BUILD_SUFFIX\s+")(.*)(".*)', line.rstrip())
    if (m):
        line = m.group(1).rstrip() + buildName + m.group(3)

    print line
    # echo to stderr (can't use stdout as it is already captured by fileinput module)
    sys.stderr.write(line + "\n")

