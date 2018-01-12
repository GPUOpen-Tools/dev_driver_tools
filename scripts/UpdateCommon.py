#! /usr/bin/python
#
# Simple script to update a set of common directories that are needed as dependencies of the current project

import os
import subprocess

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# The address to internal git.amd.com repositories.
internalGitRoot = "ssh://gerritgit/DevTools/ec/"

# Define a set of AMD-internal dependencies that exist as separate git projects.
internalGitMapping = {
    "QtCommon.git"              : ["../../QtCommon/", "b671e5dadc9da629b1fbd17c41fc069440ee4bf2"],
}

# for each dependency - test if it has already been fetched - if not, then fetch it, otherwise update it to top of tree
def PullDependencies(root, mapping):
    for key in mapping:
        # convert path to OS specific format
        # add script directory to path
        print("mapping key: %s"%key)
        tmppath = os.path.join(scriptRoot, mapping[key][0])
        # clean up path, collapsing any ../ and converting / to \ for Windows
        path = os.path.normpath(tmppath)
        source = root + key

        if os.path.isdir(path):
            # directory exists - get latest from git
            print("\nDirectory %s exists, using 'git pull' to get latest from %s"%(path, source))
            p = subprocess.Popen(["git","pull"], cwd=path)
            p.wait();
        else:
            # directory doesn't exist - clone from git
            print("\nDirectory %s does not exist, using 'git clone' to get latest from %s"%(path, source))
            p = subprocess.Popen(["git","clone",source,path])
            p.wait();
            p = subprocess.Popen(["git","reset","--hard",mapping[key][1]], cwd=path)
            p.wait()		

# Pull in dependencies from internal git.amd.com repositories.
PullDependencies(internalGitRoot, internalGitMapping)
