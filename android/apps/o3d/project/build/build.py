#!/usr/bin/python
#
# Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""build system for assets"""

import build_system
import os
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
  # b.ExecuteIf(['cp', 'src1.txt', 'dst1.txt'], ['src1.txt'], ['dst1.txt'])
  

  project_path = os.path.abspath(sys.path[0] + '/../')
  output_path = project_path + '/bin/host'
  
  # Build Classgen
  classgen_path = project_path + '/tools/ClassGen/'
  classgen_output_path = output_path + '/ClassGen/'
  
  Lex(b, classgen_path + 'classgen_lexer.l', classgen_output_path)
  Yacc(b, classgen_path + 'classgen_parser.y', classgen_output_path)
  
  source_files = [\
      classgen_path + 'ParseElement.c', \
      classgen_path + 'ClassGenGlue.cpp', \
      classgen_path + 'ClassGenerator.cpp', \
      classgen_output_path + 'classgen_lexer.c', \
      classgen_output_path + 'classgen_parser.c' \
    ]
  Cc(b, source_files, classgen_output_path)
  Link(b, source_files, classgen_output_path, 'ClassGen')

# Convenience functions

def MakeOutputFile(input_file, output_path, extension):
  (base_name, ext) = os.path.splitext(os.path.basename(input_file))
  output_file = output_path + base_name + extension
  if output_path[len(output_path) - 1] != '/':
    output_file = output_path + '/' + base_name + extension
    
  return output_file


# Build rules

def Lex(builder, input_file, output_path):
  output_file = MakeOutputFile(input_file, output_path, '.c')
  
  builder.ExecuteIf(\
    ['flex',  '--outfile=' + output_file, input_file], \
    [input_file], \
    [output_file], \
    )
    
    
def Yacc(builder, input_file, output_path):
  output_file = MakeOutputFile(input_file, output_path, '.c')
  
  builder.ExecuteIf(\
    ['yacc',  '-d', '-v', '--output=' + output_file, input_file], \
    [input_file], \
    [output_file], \
    )
    
    
def Cc(builder, input_files, output_path):
  input_paths = set()
  for file in input_files:
    in_path = os.path.abspath(os.path.dirname(file))
    input_paths.add(in_path)
    
  include_paths = ('-I' + (' -I'.join(input_paths))).split(' ')
    
  for file in input_files:
    output_file = MakeOutputFile(file, output_path, '.o')
  
    args = ['gcc','-c', file, '-o', output_file]
    args.extend(include_paths)
    builder.ExecuteIf(\
      args, \
      [file], \
      [output_file], \
      )

def Link(builder, input_files, output_path, ouput_name):
  output_files = []
  for file in input_files:
    output_file = MakeOutputFile(file, output_path, '.o')
    output_files.append(output_file)
    
  output_executable = output_path + ouput_name
  args = ['g++','-o', output_executable]
  args.extend(output_files)
  builder.ExecuteIf(\
    args, \
    output_files, \
    [output_executable], \
    )


if __name__ == '__main__':
  main(sys.argv[1:])

