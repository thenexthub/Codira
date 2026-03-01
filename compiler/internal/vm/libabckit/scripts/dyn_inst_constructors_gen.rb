# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

require 'ostruct'
require 'yaml'

scripts_dir = File.dirname(__FILE__)
Dir.chdir(scripts_dir)

isa = YAML.load_file('../../isa/isa.yaml')['groups']
insts = []

def process_inst_params(inst_params)
  inst_params.map! { |param|
    # param can be either "some_id" or "v:in:any"
    param_options = param.split(":")
    processed_param = OpenStruct.new
    processed_param.name = param_options[0][/[a-z_]*/]
    processed_param.direction = param_options[1] ? param_options[1] : "in"
    param = processed_param
  }
end

isa.each { |isa_title|
  isa_title['instructions'].each { |isa_inst|
    inst = OpenStruct.new
    inst.opcode, *inst.params = isa_inst['sig'].split(' ')
    inst.opcode = inst.opcode.split(".").map!{ |str| str.capitalize }.join
    process_inst_params(inst.params)
    inst.ic = isa_inst['properties'] ?
      isa_inst['properties'].any?{
        |str| str.include? "ic_slot" or  str.include? "jit_ic_slot"
      } : false
    inst.inacc = isa_inst['acc'] ?
        (isa_inst['acc'] =~ /inout:/ or isa_inst['acc'] =~ /in:/): false
    insts.append(inst)
  }
}

dyn_opcodes = File.open("../include/libabckit/c/isa/isa_dynamic.h").read.split()
  .select{ |str|
    str.start_with?("ABCKIT_ISA_API_DYNAMIC_OPCODE_")
  }.map!{ |str|
    str.delete_prefix("ABCKIT_ISA_API_DYNAMIC_OPCODE_").chop!.split("_").map! { |str|
      str.capitalize
    }.join
  }

declaration_num = 0
# tmp workaround: isa.yaml does not have If opcode, but opcodes.h does
declaration_num += 1
puts "--> declarations"
insts.each { |inst|
  next unless dyn_opcodes.include? inst.opcode
  declaration_num += 1

  indent = ' ' * 4
  inst_input = imm_input = 0

  declaration = indent + "AbckitInst *(*Icreate#{inst.opcode})(\n"
  indent += ' ' * 4
  declaration += indent  + "/* in */ AbckitGraph *graph"

  if inst.inacc
    declaration += ",\n" + indent
    declaration += "/* in */ AbckitInst *acc"
  end

  inst.params.each { |param|
    # case when first inst param is imm number of ic_slot
    next imm_input += 1 if inst.ic and param.name == "imm" and imm_input == 0

    declaration += ",\n" + indent
    declaration += "/* #{param.direction} */ "

    case param.name
    when "v"
      declaration += "AbckitInst *input#{inst_input}"
      inst_input += 1
    when "method_id"
      declaration += "AbckitCoreFunction *method"
    when "literalarray_id"
      declaration += "AbckitLiteralArray *literalArray"
    when "string_id"
      declaration += "AbckitString *string"
    when "imm"
      case inst.opcode
      when "Getmodulenamespace", "WideGetmodulenamespace"
        declaration += "AbckitCoreModule *md"
      when "Ldexternalmodulevar", "WideLdexternalmodulevar"
        declaration += "AbckitCoreImportDescriptor *id"
      when "Ldlocalmodulevar", "WideLdlocalmodulevar", "Stmodulevar", "WideStmodulevar"
        declaration += "AbckitCoreExportDescriptor *ed"
      else
        declaration += "uint64_t imm#{inst.ic ? imm_input - 1 : imm_input}"
      end
      imm_input += 1
    else
      puts "Unknown type of inst param"
      exit 1
    end
  }
  declaration += ");"
  puts declaration
  puts "\n"
}

if declaration_num != dyn_opcodes.length()
  puts "Missed some declarations"
  puts "total printed declarations: #{declaration_num}"
  puts "total dynamic opcodes: #{dyn_opcodes.length()}"
  dyn_opcodes.each { |opcode|
    next unless insts.index { |inst| inst.opcode == opcode }.nil?
    puts "No declaration for #{opcode} opcode"
  }
  exit 1
end


puts "--> definitions"
insts.each { |inst|
  next unless dyn_opcodes.include? inst.opcode

  inst_input = imm_input = 0

  definition = "extern \"C\" AbckitInst *Icreate#{inst.opcode}("
  definition += "AbckitGraph *graph"

  if inst.inacc
    definition += ", AbckitInst *acc"
  end

  inst.params.each { |param|
    # case when first inst param is imm number of ic_slot
    next imm_input += 1 if inst.ic and param.name == "imm" and imm_input == 0

    case param.name
    when "v"
      definition += ", AbckitInst *input#{inst_input}"
      inst_input += 1
    when "method_id"
      definition += ", AbckitCoreFunction *method"
    when "literalarray_id"
      definition += ", AbckitLiteralArray *literalArray"
    when "string_id"
      definition += ", AbckitString *string"
    when "imm"
      case inst.opcode
      when "Getmodulenamespace", "WideGetmodulenamespace"
        definition += ", AbckitCoreModule *md"
      when "Ldexternalmodulevar", "WideLdexternalmodulevar"
        definition += ", AbckitCoreImportDescriptor *id"
      when "Ldlocalmodulevar", "WideLdlocalmodulevar", "Stmodulevar", "WideStmodulevar"
        definition += ", AbckitCoreExportDescriptor *ed"
      else
        definition += ", uint64_t imm#{inst.ic ? imm_input - 1 : imm_input}"
      end
      imm_input += 1
    else
      puts "Unknown type of inst param"
      exit 1
    end
  }
  indent = ' ' * 4
  definition += ")\n{\n";
  definition += indent + "LIBABCKIT_UNIMPLEMENTED\n";
  definition += "}\n";
  puts definition
  puts "\n"
}

puts "--> dyn instruction constructor names"
insts.each { |inst|
  next unless dyn_opcodes.include? inst.opcode
  constructor_name = "Icreate#{inst.opcode},"
  puts constructor_name
}
