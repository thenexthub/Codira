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

require 'ostruct'
require 'delegate'

class String
  def snakecase
    gsub(/::/, '/')
      .gsub(/([A-Z]+)([A-Z][a-z])/, '\1_\2')
      .gsub(/([a-z\d])([A-Z])/, '\1_\2')
      .tr('-', '_')
      .downcase
  end
end

class OpenStruct
  # Versions prior to 2.7.1 lack of `to_h` overload, so define something similar:
  def to_stringhash
    h = {}
    each_pair { |k, v| h[k.to_s] = v }
    h
  end
end

class Option < SimpleDelegator
  def initialize(data)
    super
    if has_sub_options?
      raise "Compound option should not have `type` field, it is always bool" if respond_to?(:type)
      raise "Compound option should not have `default` field, it is always `false``" if respond_to?(:default)
      self[:type] = 'bool'
      self[:default] = 'false'
    end
  end

  def raise(msg)
    super "in '#{name}': #{msg}"
  end

  def fix(options)
    if has_sub_options?
      sub_options.map { |subopt| subopt.fix(options) }
      return
    end
    fix_references(options)
    flatten_lists_of_option_values
    verify
  end

  def fix_references(options)
    if respond_to?(:possible_values) && possible_values.is_a?(String)
      raise "Wrong reference syntax: #{possible_values}" if possible_values[0] != '$'
      p = possible_values[1..-1]
      target = options.find {|x| x.name == p }
      raise "Invalid reference #{possible_values}" if target.nil?
      self["possible_values"] = target["possible_values"]
    end
  end

  def flatten_lists_of_option_values
    if self[:possible_values].is_a? OpenStruct
      self[:possible_values] = possible_values.to_stringhash.flatten(2)
    end
  end

  def verify
    if respond_to? :default
      supported_default_types = [String, Array, TrueClass, FalseClass, Integer, Float]
      if not supported_default_types.member? self[:default].class
        raise "Syntax error, 'default' should be of #{supported_default_types} type, got '#{self[:default].class}'"
      elsif (self[:default].is_a? Array) != (type == 'arg_list_t')
        raise "Syntax error, 'Array'-default should be used with 'arg_list_t'-options"
      end
    end
  end

  def field_name
    n = name.tr('-', '_').tr('.', '_') + '_'
    sub_option? ? (self.parent.field_name + n) : n
  end

  def camel_name
    n = name.split(Regexp.union(['-', '.'])).map(&:capitalize).join
    sub_option? ? (self.parent.camel_name + n) : n
  end

  def view_type
    case type
    when 'std::string'
      'const std::string &'
    when 'arg_list_t'
      'const arg_list_t &'
    else
      type
    end
  end
  
  def getter_signature
    ret_type = view_type
    ret_type += ' ' if ret_type[-1] != '&'
    ret_type + getter_name + '([[maybe_unused]] std::string_view lang = "") const'
  end

  def getter_name
    (type == 'bool' ? 'Is' : 'Get') + camel_name
  end

  def setter_name
    'Set' + camel_name
  end

  def sub_option?
    respond_to?(:parent)
  end

  def has_sub_options?
    respond_to?(:sub_options)
  end

  def has_enum?
    respond_to?(:enum)
  end

  def flat_enum
    raise "'enum' has invalid type" if !enum.is_a?(Array) && !enum.is_a?(OpenStruct)
    return enum if enum.is_a?(Array)
    enum.to_h.values.flatten
  end

  def sub_enums
    return {} if enum.is_a?(Array)
    enum.to_stringhash
  end

  def deprecated?
    respond_to?(:deprecated) && deprecated
  end

  def lang_specific?
    respond_to?(:lang_specific)
  end

  def has_lang_suboptions?
    respond_to?(:lang)
  end

  def lang_field_name(lang)
    "#{lang}.#{name}".tr('-', '_').tr('.', '_') + '_'
  end

  def lang_camel_name(lang)
    n = "#{lang}.#{name}".split(Regexp.union(['-', '.'])).map(&:capitalize).join
    sub_option? ? (self.parent.camel_name + n) : n
  end

  def default_value
    if default_target_specific?
      raise "Syntax error, 'default_target_specific' should be of 'OpenStruct' class, got '#{default_target_specific.class}'" unless default_target_specific.is_a?(OpenStruct)
      res = "ark::default_target_options::GetTargetSpecificOptionValue<std::map<std::string, "
      res += type
      res += ">>("
      res += "std::map<std::string, "
      res += type
      res += ">"
      res += "{"
      default_target_specific.each_pair { |k, v| res += "{#{expand_string k.to_s}, #{v}}," }
      res += "}, ark::default_target_options::GetTargetString)"
    else
      return default_constant_name if need_default_constant
      return '{' + default.map { |e| expand_string(e) }.join(', ') + '}' if type == 'arg_list_t'
      return expand_string(default) if type == 'std::string'
      default
    end
  end

  def full_description
    full_desc = description
    full_desc.prepend("[DEPRECATED] ") if deprecated?
    if defined? possible_values
      full_desc += "." unless full_desc[-1] == "."
      full_desc += ' Possible values: ' + possible_values.inspect
    end
    full_desc += "." unless full_desc[-1] == "."
    if default_target_specific?
      full_desc += " Default (target specific):\n"
      default_target_specific.each_pair{ |k, v| full_desc += " on '#{k.to_s}': #{v}\n" }
    else
      full_desc += ' Default: ' + default.inspect
    end
    Common::to_raw(full_desc)
  end

  def expand_string(s)
    @expansion_map ||= {
      '$ORIGIN' => 'exe_dir_'
    }
    @expansion_map.each do |k, v|
      ret = ""
      if s.include?(k)
        split = s.split(k);
        for i in 1..split.length() - 1
          ret += v + ' + ' + Common::to_raw(split[i])+ ' + '
        end
        return ret.delete_suffix(' + ')
      end
    end
    Common::to_raw(s)
  end

  def default_target_specific?
    respond_to? :default_target_specific
  end

  def need_default_constant
    (type == 'int' || type == 'double' || type == 'uint32_t' || type == 'uint64_t') && !default_target_specific?
  end

  def default_constant_name
    name.tr('-', '_').tr('.', '_').upcase + '_DEFAULT'
  end
end

class Event < SimpleDelegator
  def method_name  
    'Event' + name.split('-').map(&:capitalize).join
  end  

  def args_list
    args = ''
    delim = ''
    fields.each do |field|
      args.concat(delim, field.type, ' ', field.name)           
      delim = ', '       
    end
    return args
  end
  
  def print_line
    qoute = '"'
    line = 'events_file_'
    delim = ' << '
    fields.each do |field|        
      line.concat(delim, qoute, field.name, qoute) 
      delim = ' << ":" << ' 
      line.concat(delim, field.name)  
      delim = ' << "," << '    
    end 
    return line
  end  
   
end

module Common
  module_function

  def create_sub_options(option, options)
    return unless option.has_sub_options?
    raise "Only boolean option can have sub options: #{option.name}" unless option.type == 'bool'
    sub_options = []
    option.sub_options.each_with_object(sub_options) do |sub_option, sub_options|
      sub_option[:parent] = option
      sub_options << Option.new(sub_option)
    end
    options.concat sub_options
    option[:sub_options] = sub_options
  end

  def options
    @options ||= @data.options.each_with_object([]) do |option, options|
      option_hash = option.to_h
      new_option = nil
      if option_hash.include?(:lang)
        lang_spec_option = option_hash.clone
        lang_spec_option.delete(:lang)
        lang_spec_option[:lang_specific] = true
        option.lang.each do |lang|
          lang_spec_option[:description] = "#{option.description}. Only for #{lang}"
          lang_spec_option[:name] = "#{lang}.#{option.name}"
          new_option = Option.new(OpenStruct.new(lang_spec_option))
          create_sub_options(new_option, options)
          options << new_option
        end
        main_option = option_hash.clone
        main_option[:name] = "#{option.name}"
        new_option = Option.new(OpenStruct.new(main_option))
      else
        new_option = Option.new(option)
      end
      create_sub_options(new_option, options)
      options << new_option
      options.last.fix(@data.options)
    end
    @options
  end

  def events
    @data.events.map do |op|
      Event.new(op)
    end
  end

  def module
    @data.module
  end

  def to_raw(s)
    'R"(' + s + ')"'
  end

  def wrap_data(data)
    @data = data
  end
end

def Gen.on_require(data)
  Common.wrap_data(data)
end
