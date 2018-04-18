#! /usr/bin/python
#
# Simple script to update a set of common directories that are needed as dependencies of the current project
# Usage:
#   FetchDependencies.py [latest]
#
# If "latest" is specified, the latest commit will be checked out.
# Otherwise, the repos will be updated to the commit specified in the "gitMapping" table.
# If the required commit in the "gitMapping" is None, the repo will be updated to the latest commit.

import os
import subprocess
import sys

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# When running this script on Windows (and not under cygwin), we need to set the Shell=True argument to Popen and similar calls
# Without this option, Jenkins builds fail to find the correct version of git
SHELLARG = False
# The environment variable SHELL is only set for Cygwin or Linux
SHELLTYPE = os.environ.get('SHELL')
if ( SHELLTYPE == None ):
    # running on windows under default shell
    SHELLARG = True

# Print the version of git being used. This also confirms that the script can find git
try:
     subprocess.call(["git","--version"], shell=SHELLARG)
except OSError:
    # likely to be due to inability to find git command
    print "Error calling command: git --version"

# Calculate the root of the git server - all dependencies should be retrieved from the same server
gitURL = subprocess.check_output(["git", "-C", scriptRoot, "remote", "get-url", "origin"], shell=SHELLARG)
# Strip everything after the last '/' from the URL to retrieve the root
gitRoot = gitURL.rsplit('/',1)[0] + "/"
    
# If cloning from github - use the master branch - otherwise use amd-master
gitBranch = "amd-master"
if "github" in gitURL:
    gitBranch = "master"

print "\nFetching dependencies from: " + gitRoot + " - using branch: " + gitBranch + "\n"
    
# Define a set of dependencies that exist as separate git projects. The parameters are:
# "git repo name"  : ["Directory for clone relative to this script",  "commit hash to checkout (or None for top of tree)"

gitMapping = {
    # Lib.
    "common-lib-amd-ADL.git"              : ["../../Common/Lib/AMD/ADL",           "master"],
    # QtCommon
    "QtCommon"                            : ["../../QtCommon",                     None]
}

# for each dependency - test if it has already been fetched - if not, then fetch it, otherwise update it to top of tree
for key in gitMapping:
    # convert path to OS specific format
    # add script directory to path
    tmppath = os.path.join(scriptRoot, gitMapping[key][0])
    # clean up path, collapsing any ../ and converting / to \ for Windows
    path = os.path.normpath(tmppath)
    source = gitRoot + key

    reqdCommit = gitMapping[key][1]
    # reqdCommit may be "None" - or user may override commit via command line. In this case, use tip of tree
    if((len(sys.argv) != 1 and sys.argv[1] == "latest") or reqdCommit is None):
        reqdCommit = gitBranch

    print "\nChecking out commit: " + reqdCommit + " for " + key

    if os.path.isdir(path):
        # directory exists - get latest from git using pull
        print "Directory " + path + " exists. \n\tUsing 'git pull' to get latest from " + source
        sys.stdout.flush()
        try:
            subprocess.check_call(["git", "-C", path, "pull", "origin", reqdCommit], shell=SHELLARG)
        except subprocess.CalledProcessError as e:
            print ("'git pull' failed with returncode: %d\n" % e.returncode)
            sys.exit(1)
        sys.stderr.flush()
        sys.stdout.flush()
    else:
        # directory doesn't exist - clone from git
        print "Directory " + path + " does not exist. \n\tUsing 'git clone' to get latest from " + source
        sys.stdout.flush()
        try:
            subprocess.check_call(["git", "-C", scriptRoot, "clone", source, path, "--branch", reqdCommit], shell=SHELLARG)
        except subprocess.CalledProcessError as e:
            print ("'git clone' failed with returncode: %d\n" % e.returncode)
            sys.exit(1)
        sys.stderr.flush()
        sys.stdout.flush()
