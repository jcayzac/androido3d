# This script pipes diff and merge output from Mercurial to FileMerge.
# see: http://mercurial.selenic.com/wiki/ExtdiffExtension and
# http://mercurial.selenic.com/wiki/MacOSXFileMerge
# opendiff returns immediately, without waiting for FileMerge to exit.
# Piping the output makes opendiff wait for FileMerge.
opendiff "$@" | cat