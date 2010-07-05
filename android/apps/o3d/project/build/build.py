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

  source_files = [
      classgen_path + 'ParseElement.c',
      classgen_path + 'ClassGenGlue.cpp',
      classgen_path + 'ClassGenerator.cpp',
      classgen_output_path + 'classgen_lexer.c',
      classgen_output_path + 'classgen_parser.c'
    ]
  Cc(b, source_files, classgen_output_path)
  Link(b, source_files, classgen_output_path, 'ClassGen')

  # Run ClassGen to generate header files.
  classgen_exe = classgen_output_path + 'ClassGen'
  system_path = project_path + '/jni/game/system/'
  math_path = project_path + '/jni/game/math/'
  game_path = project_path + '/jni/game/game/'
  meta_files = [
      system_path + 'BoxCollisionSystem.meta', 
      system_path + 'CollisionSystem.meta', 
      system_path + 'MainLoop.meta', 
      system_path + 'ProfileSystem.meta', 
      system_path + 'RenderableObject.meta', 
      system_path + 'Renderer.meta', 
      system_path + 'System.meta', 
      system_path + 'TimeSystem.meta', 
      system_path + 'TimeSystemPosix.meta', 
      math_path + "Box.meta",  
      game_path + "AnimationComponent.meta", 
      game_path + "CollisionComponent.meta", 
      game_path + "PhysicsComponent.meta", 
      game_path + "CollisionPairSystem.meta", 
      game_path + "ComponentList.meta", 
      game_path + "GameComponent.meta", 
      game_path + "GameObject.meta", 
      game_path + "GameObjectSystem.meta", 
      game_path + "GravityComponent.meta", 
      game_path + "LinearMotionComponent.meta", 
      game_path + "MovementComponent.meta", 
      game_path + "PlayerAnimationComponent.meta", 
      game_path + "PlayerMotionComponent.meta", 
      game_path + "RenderComponent.meta", 
      
    ]

  header_path = project_path + '/bin/headers/'
  for file in meta_files:
    ClassGen(b, classgen_exe, file, header_path)

# Convenience functions

def MakeOutputFile(input_file, output_path, extension):
  """Generates the name of an output file."""
  (base_name, ext) = os.path.splitext(os.path.basename(input_file))
  output_file = output_path + base_name + extension
  if output_path[len(output_path) - 1] != '/':
    output_file = output_path + '/' + base_name + extension

  return output_file


# Build rules

def Lex(builder, input_file, output_path):
  """Builds the input file with Lex."""
  output_file = MakeOutputFile(input_file, output_path, '.c')

  builder.ExecuteIf(
    ['flex',  '--outfile=' + output_file, input_file],
    [input_file],
    [output_file])


def Yacc(builder, input_file, output_path):
  """Builds the input file with Yacc."""
  output_file = MakeOutputFile(input_file, output_path, '.c')

  builder.ExecuteIf(
    ['yacc',  '-d', '-v', '--output=' + output_file, input_file],
    [input_file],
    [output_file])


def Cc(builder, input_files, output_path):
  """Compiles the list of input files with gcc.  File paths are used as include paths."""
  input_paths = set()
  for file in input_files:
    in_path = os.path.abspath(os.path.dirname(file))
    input_paths.add(in_path)

  include_paths = ('-I' + (' -I'.join(input_paths))).split(' ')

  for file in input_files:
    output_file = MakeOutputFile(file, output_path, '.o')

    args = ['gcc','-c', file, '-o', output_file]
    args.extend(include_paths)
    builder.ExecuteIf(
      args,
      [file],
      [output_file])

def Link(builder, input_files, output_path, ouput_name):
  """Links the input object files into an executable."""
  output_files = []
  for file in input_files:
    output_file = MakeOutputFile(file, output_path, '.o')
    output_files.append(output_file)

  output_executable = output_path + ouput_name
  args = ['g++','-o', output_executable]
  args.extend(output_files)
  builder.ExecuteIf(
    args,
    output_files,
    [output_executable])


def ClassGen(builder, classgen_path, input_file, output_path):
  """Builds the input file with ClassGen."""
  output_file = MakeOutputFile(input_file, output_path, '.h')
  
  builder.ExecuteIf(\
    [classgen_path, input_file, output_file], \
    [classgen_path, input_file], \
    [output_file], \
    )

if __name__ == '__main__':
  main(sys.argv[1:])

