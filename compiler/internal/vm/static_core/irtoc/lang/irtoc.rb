#!/usr/bin/env ruby

#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

require_relative 'options'
# Parse options in advance, since it may be required during scripts loading
Options.parse

require_relative 'function'
require_relative 'cpp_function'
require_relative 'instructions_data'
require_relative 'output'
require_relative 'ir_generator'
require_relative 'isa'
require 'fileutils'
require 'logger'
require 'optparse'
require 'securerandom'
require 'tempfile'
require 'yaml'

FILE_BEGIN = %{
// THIS FILE WAS GENERATED FOR #{Options.arch.upcase}

#include "irtoc/backend/compilation.h"
#include "compiler/optimizer/ir/ir_constructor.h"
#include "compiler/optimizer/code_generator/relocations.h"
#include "libarkbase/utils/expected.h"
#include "asm_defines.h"
#include "cross_values.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "libarkbase/mem/stack_like_allocator.h"

#ifndef __clang_analyzer__

using namespace ark::compiler;

namespace ark::irtoc \{
// NOLINTBEGIN(readability-identifier-naming)

}

FILE_END = %{
// NOLINTEND(readability-identifier-naming)
\} // namespace ark::irtoc

#endif  // __clang_analyzer__
}

class Irtoc

  attr_reader :functions, :cpp_functions
  attr_writer :compile_for_llvm

  def initialize
    # List of all compiled functions.
    @functions = []
    # Currently processed script file.
    @current_script = nil
    # Macros, that are defined for the current script. Since each macros is just a method in Function class, we need
    # to remove all these methods after script is done. Because further scripts should not see macros, defined in
    # previous one.
    @macros = []
    # Macros that also generate corresponding IR Builder methods
    @builder_macros = []
    @compile_for_llvm = false
  end

  (0..31).each { |i| define_method("r#{i}") { i } }
  { :rax => 0,
    :rcx => 1,
    :rdx => 2,
    :rbx => 11,
    :rsp => 10,
    :rbp => 9,
    :rsi => 6,
    :rdi => 7}.each { |name, value| define_method(name) { value } }

  def function(name, **kwargs, &block)
    func = Function.new name, **kwargs, compile_for_llvm: @compile_for_llvm, &block
    @functions << func unless func.mode.nil?
    func.compile
  end

  def cpp_function(name, &block)
    func = CppFunction.new(name)
    func.instance_eval(&block)
    (@cpp_functions ||= []) << func
  end

  def macro(name, &block)
    raise "Macro can't start with capital letter" if name.to_s.start_with? /[A-Z]/
    Function.define_method(name.to_sym, &block)
    @macros << name
  end

  def scoped_macro(name, &block)
    raise "Macro can't start with capital letter" if name.to_s.start_with? /[A-Z]/
    Function.define_method(name.to_sym) do |*args|
      @locals.push({})
      @unresolved.push({})
      @labels.push({})
      if args.size != block.arity && block.arity >= 0 || args.size < -(block.arity + 1) && block.arity < 0
        raise ArgumentError.new("Macro #{name} wrong number of arguments (given #{args.size}, expected #{block.arity})")
      end
      res = instance_exec(*args, &block)
      @locals.pop()
      @unresolved.pop()
      @labels.pop()
      res
    end
    @macros << name
  end

  def remove_macros
    @macros.each do |name|
      Function.remove_method name
    end
    @macros.clear
  end

  def include_relative(filename)
    fixed_filename = "#{File.basename(filename)}.fixed"
    filename = "#{File.dirname(@current_script)}/#{filename}"
    preprocess(filename, fixed_filename)
    self.instance_eval File.open(fixed_filename).read, filename
  end

  def preprocess_line(line)
    line.gsub(/(\w+) *:= *([^>])/, 'let :\1, \2').gsub(/\} *Else/, "}; Else").gsub(/%(\w+)/, 'LiveIn(regmap[:\1])')
  end

  def read_plugin(filename)
    return "" if Options.plugins.empty?
    output = ""
    Options.plugins[filename].each do |full_filename|
      File.open(full_filename).readlines.each do |line|
        output += preprocess_line(line)
      end
    end
    output
  end

  def preprocess(filename, output_filename)
    tmp_filename = "#{output_filename}.#{SecureRandom.uuid}.tmp"
    File.open(tmp_filename, "w") do |file|
      File.open(filename).readlines.each do |line|
        line_matches = line.match(/include_plugin '(.*)'/)
        if line_matches
          filename = line_matches.captures[0]
          file.puts read_plugin(filename)
        else
          file.puts preprocess_line(line)
        end
      end
    end
    FileUtils.mv tmp_filename, output_filename, force: true
  end

  def run(input_file)
    @current_script = input_file
    fixed_filename = "#{File.basename(input_file)}.fixed"
    preprocess(input_file, fixed_filename)
    begin
      data = File.open(fixed_filename).read
      self.instance_eval data, fixed_filename
    rescue SyntaxError => e
      puts "========== Begin #{fixed_filename} =========="
      puts data
      puts "========== End #{fixed_filename} =========="
      raise
    end
  end
end

def main
  abort "YAML description file is not specified" unless Options.instructions_yaml
  abort "ISA YAML file is not specified" unless Options.isa
  abort "ISAPI file is not specified" unless Options.isapi

  InstructionsData.setup Options.instructions_yaml
  Function.setup
  IRInstruction.setup
  ISA.setup Options.isa, Options.isapi

  require Options.commonapi

  functions = []
  cpp_functions = []
  llvm_functions = []

  Options.input_files.each do |input_file|
    irtoc = Irtoc.new
    irtoc.run(input_file)
    functions += irtoc.functions
    cpp_functions += irtoc.cpp_functions if irtoc.cpp_functions
  end
  # Compile each function for LLVM
  Options.input_files.each do |input_file|
    irtoc = Irtoc.new
    irtoc.compile_for_llvm = true
    irtoc.run(input_file)
    llvm_functions += irtoc.functions
  end

  if Options.ir_api == 'ir-constructor'
    raise 'Provide one --output file to generate intepreter handler IR for stock compiler. Or provide two --output files delimited by \':\' to generate IR for stock compiler (the first file), and llvm compiler (the second file)' unless Options.output_files.size <= 2
    Output.setup Options.output_files[0]
    Output.println(FILE_BEGIN)
    functions.reject(&:enable_builder).each do |function|
      function.emit_ir ""
    end
    Output.println(FILE_END)
    if Options.output_files.size == 2
      Output.setup Options.output_files[1]
      interp_llvm_functions = llvm_functions.reject(&:enable_builder).select do |f|
        f.mode.include?(:Interpreter) || f.mode.include?(:InterpreterEntry)
      end
      unless interp_llvm_functions.empty?
        Output.println(FILE_BEGIN)
        interp_llvm_functions.each do |function|
          function.emit_ir '_LLVM'
        end
        Output.println(FILE_END)
      end
    end
  elsif Options.ir_api == 'ir-builder'
    builder_functions = functions.select(&:enable_builder)
    Output.setup Options.output_files[0]
    Output.println('#include "optimizer/ir/graph.h"')
    Output.println('#include "optimizer/ir/basicblock.h"')
    Output.println('#include "optimizer/ir/inst.h"')
    Output.println('#include "asm_defines.h"')
    Output.println('namespace ark::compiler {')
    Output.println('// NOLINTBEGIN(readability-identifier-naming)')
    builder_functions.each(&:generate_builder)
    Output.println('// NOLINTEND(readability-identifier-naming)')
    Output.println('} // namespace ark::compiler')
  elsif Options.ir_api == 'ir-inline'
    Output.setup Options.output_files[0]
    Output << '#include "optimizer/ir/graph.h"'
    Output << '#include "optimizer/ir/basicblock.h"'
    Output << '#include "optimizer/ir/inst.h"'
    Output << "#include \"asm_defines.h\"\n"
    Output << "namespace ark::compiler {\n"
    Output << "// NOLINTBEGIN(readability-identifier-naming)\n"
    cpp_functions.each { |func| func.generate_ir(GeneratorIrInline.new) }
    Output << "// NOLINTEND(readability-identifier-naming)\n"
    Output << '}  // namespace ark::compiler'
  else
    abort 'Should be unreachable: Unknown IR API'
  end

  validation = Hash[functions.select(&:validation).map { |f| [f.name, f.validation] } ]
  File.open("validation.yaml", "w") do |file|
    file.write(validation.to_yaml)
  end
end

main
