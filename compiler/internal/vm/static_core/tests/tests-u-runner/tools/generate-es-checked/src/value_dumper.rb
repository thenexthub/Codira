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

require_relative 'types'

module ESChecker
    UNMANGLE = {"#__NaN" => "NaN", "#__null" => "null", "#__Inf" => "Infinity", "#__NegInf" => "-Infinity", "#__undefined" => "undefined"}
    class ValueDumper
        def initialize(conf)
            @name = 0
            @conf = conf
        end

        def parse_type(str)
            ESChecker::Types::Parser.new(str).run
        end

        def unmangle(x)
            toh = Proc.new { |i, off|
                ((i >> (off * 8)) % 256).to_s(16).rjust(2, '0')
            }
            if x.kind_of? Array
                x.map { |e| unmangle e }
            elsif x.kind_of? Hash
                case x["__kind"]
                when "str"
                    res = x["data"].map { |c| "\\u#{toh.(c, 1)}#{toh.(c, 0)}" }.join
                    "\"#{res}\""
                when "num"
                    "Double.bitCastFromLong(#{x["data"]} as long) /* #{x["dbg"]} */"
                when "bigint"
                    "#{x["data"]}n"
                else
                    raise "unknown kind for #{x}"
                end
            elsif x.kind_of? String
                old = x
                x = UNMANGLE[x] || x
                if x != old
                    x
                else
                    x.inspect
                end
            elsif x.kind_of?(Float)
                hex = [x].pack('G').split('').map { |ds| ds[0] }.map { |a| sprintf("%02x", a.ord) }.join()
                "Double.bitCastFromLong(0x#{hex}) /* #{x} */"
            else
                UNMANGLE[x] || x
            end
        end

        def get_array_type_from(x)
            match = /^\s*(Array|Iterable)<(?<elem>.*)>\s*$/.match(x)
            if match
                match[:elem]
            else
                nil
            end
        end

        def dump_value(buf, rb_obj, exp_type)
            case exp_type[:kind]
            when :array, :iterable
                buf << "Array.of<" << ESChecker::Types::dump(exp_type[:el]) << ">("
                rb_obj.each_with_index { |v, i|
                    if i != 0
                        buf << ", "
                    end
                    dump_value(buf, v, exp_type[:el])
                }
                buf << ")"
            when :tuple
                buf << "["
                rb_obj.zip(exp_type[:els]).each_with_index { |v, i|
                    if i != 0
                        buf << ", "
                    end
                    dump_value buf, v[0], v[1]
                }
                buf << "] as " << ESChecker::Types::dump(exp_type)
            when :trivial
                buf << "#{rb_obj} as #{exp_type[:str]}"
            else
                raise "unknown type kind #{exp_type.kind}"
            end
        end

        def dump(rb_obj, is_self: false)
            typ = if is_self
                ESChecker::Types::detect(rb_obj, dflt: @conf.self_type)
            else
                ESChecker::Types::detect(rb_obj, dflt: @conf.ret_type)
            end
            rb_obj = unmangle(rb_obj)
            @buf = String.new
            dump_value(@buf, rb_obj, typ)
            r = @buf
            @buf = String.new
            r
        end
    end
end
