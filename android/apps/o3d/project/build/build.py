#!/bin/python
#
# Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""build system for assets"""

import build_system
import sys
from optparse import OptionParser


def main(argv):
  """This is the main function."""
  parser = OptionParser()
  parser.add_option(
      "--no-execute", action="store_true",
      help="does not execute any external commands")
  parser.add_option(
      "--force", action="store_true",
      help="forces building regardless of dates.")
  parser.add_option(
      "-v", "--verbose", action="store_true",
      help="prints more output.")
  parser.add_option(
      "-d", "--debug", action="store_true",
      help="turns on debugging.")

  (options, args) = parser.parse_args(args=argv)

  b = build_system.BuildSystem(
      options.verbose, options.force, not options.no_execute, options.debug)

  # The first array is args
  # The second array is the source files to check
  # The third array is the destination files expected to be built.
  b.ExecuteIf(['cp', 'src1.txt', 'dst1.txt'], ['src1.txt'], ['dst1.txt'])

if __name__ == '__main__':
  main(sys.argv[1:])

