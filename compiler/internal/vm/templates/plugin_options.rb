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

module Common
  module_function

  def plugins
    @plugins
  end

  def assign_data_level(cur_hash, key, cur_data)
    if !cur_data && (cur_data.class == OpenStruct || cur_data.class == Array)
      return
    elsif !cur_data
      #boolean ?
      cur_data = "false"
    end

    if cur_data.class == OpenStruct
      wrap_hash_level(cur_hash, key, cur_data)
    elsif cur_data.class == Array
      wrap_array_level(cur_hash, key, cur_data)
    else
      cur_hash[key] = cur_data.to_s
    end
  end

  def wrap_array_level(cur_hash, key, cur_data)
    if !cur_data || cur_data.class != Array
      return
    end

    cur_data.each do |val|
      assign_data_level(cur_hash, key, val)
    end
  end

  def wrap_hash_level(cur_hash, key, cur_data)
    if !cur_data || cur_data.class != OpenStruct
        return
    end

    if !cur_hash[key]
      cur_hash[key] = Hash.new()
    end

    new_h = cur_data.to_h

    new_h.each do |sub_key, val|
      assign_data_level(cur_hash[key.to_s], sub_key.to_s, val)
    end
  end

  def wrap_data(data)
    @data = data
    @plugins = Hash.new()
    if !data || !data.plugins
      return
    end

    data.plugins.each do |plugin|
      h_plugin = plugin.to_h
      h_plugin_lang = h_plugin.keys.first.to_s
      lang_enum = h_plugin_lang == "JAVA" ? "JAVA_8" : h_plugin_lang
      lang_enum = "panda::panda_file::SourceLang::" + lang_enum
      @plugins[h_plugin_lang] = Hash.new()
      @plugins[h_plugin_lang]["lang_enum"] = lang_enum
      assign_data_level(@plugins, h_plugin_lang, h_plugin.values.first)
    end
  end
end

def Gen.on_require(data)
  Common.wrap_data(data)
end
