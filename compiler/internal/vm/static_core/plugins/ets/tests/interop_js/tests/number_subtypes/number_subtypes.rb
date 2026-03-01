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

class NumberTypeDescription
    def initialize(name, min, max, sum, suffix)
        @name, @min, @max, @sum, @suffix = name, min, max, sum, suffix
    end

    def getName
        @name
    end

    def getMin
        @min
    end

    def getMax
        @max
    end

    def getSum
        @sum
    end

    def getSuffix
      @suffix
    end
end

@integral_primitives = [
    NumberTypeDescription.new("byte", -128, 127, -128 + 127, ""),
    NumberTypeDescription.new("short", -32768, 32767, -32768 + 32767, ""),
    NumberTypeDescription.new("int", -2147483648, 2147483647, -2147483648 + 2147483647, ""),
    # double-long precision loss
    # NumberTypeDescription.new("long", -9223372036854775808, 9223372036854775, 0),
    NumberTypeDescription.new("long", -9223372036854775, 9223372036854775, -9223372036854775 + 9223372036854775, ""),
    # char will map to string when pass to dynamic world
    # NumberTypeDescription.new("char", 0, 65535, 0 + 65535),
]

@float_primitives = [
    NumberTypeDescription.new("float", 0.111, 2.71, 0.111 + 2.71, "f"),
    NumberTypeDescription.new("double", 0.01, 3.14, 0.01 + 3.14, ""),
]
