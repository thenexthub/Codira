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

TypeInfo = Struct.new(:name, :value, :type, :unwrap, :wrap, keyword_init: true) do
    def initialize(*)
        super
        self.wrap ||= 'RefType'
        self.type ||= name
    end
end

@type_infos = [
    TypeInfo.new(name: 'String', value: '"abc"', unwrap: 'String'),
    TypeInfo.new(name: 'boolean', value: 'true', unwrap: 'U1', wrap: 'U1'),
    TypeInfo.new(name: 'byte', value: '127', unwrap: 'F64', wrap: 'I8'),
    TypeInfo.new(name: 'short', value: '32767', unwrap: 'F64', wrap: 'I16'),
    TypeInfo.new(name: 'int', value: '2147483647', unwrap: 'F64', wrap: 'I32'),
    TypeInfo.new(name: 'long', value: '9223372036854775', unwrap: 'F64', wrap: 'I64'),
    TypeInfo.new(name: 'char', value: '65535', unwrap: 'F64', wrap: 'U16'),
    TypeInfo.new(name: 'double', value: '12.34', unwrap: 'F64', wrap: 'F64'),
    TypeInfo.new(name: 'float', value: '12.34', unwrap: 'F64', wrap: 'F32'),
    TypeInfo.new(name: 'int_array', value: '[1, 2]', type: 'int[]'),
    TypeInfo.new(name: 'String_array', value: '["ab", "cd"]', type: 'String[]'),
    TypeInfo.new(name: 'const_object', value: 'const_obj', type: 'EtsClass'),
    TypeInfo.new(name: 'object', value: 'create_obj()', type: 'EtsClass | null'),
    TypeInfo.new(name: 'null_object', value: 'create_null_obj()', type: 'EtsClass | null'),
]
