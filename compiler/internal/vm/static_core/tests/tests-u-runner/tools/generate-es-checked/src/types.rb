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

module ESChecker
    module Types
        def self.value_is_number(r)
            r.kind_of?(Float) ||
                r.kind_of?(Integer) ||
                r.kind_of?(Hash) && r["__kind"] == "num" ||
                ["#__NaN", "#__NegInf", "#__Inf", "#__NaN"].include?(r) ||
                r.kind_of?(String) && /^Double.bitCastFromLong.*/ =~ r
        end

        def self.value_is_string(r)
            return r.kind_of?(String) && !UNMANGLE.has_key?(r) ||
                r.kind_of?(Hash) && r["__kind"] == "str"
        end

        def self.dump(t, buf: "")
            case t[:kind]
            when :array
                buf << "Array<"
                dump t[:el], buf: buf
                buf << ">"
            when :tuple
                buf << "["
                t[:els].each_with_index { |t, i|
                    if i != 0
                        buf << ", "
                    end
                    dump t, buf: buf
                }
                buf << "]"
            when :trivial
                buf << t[:str]
            else
                raise "unknown type kind #{t.kind}"
            end
            buf
        end

        def self.detect(rb_obj, dflt: nil)
            if !dflt.nil?
                return ESChecker::Types::Parser.new(dflt).run
            end
            if [true, false].include? rb_obj
                return { :kind => :trivial, :str => "boolean" }
            end
            if value_is_number(rb_obj)
                return { :kind => :trivial, :str => "number" }
            end
            if value_is_string rb_obj
                return { :kind => :trivial, :str => "string" }
            end
            if rb_obj.kind_of?(Hash) && rb_obj["__kind"] == "bigint"
                return { :kind => :trivial, :str => "bigint" }
            end
            if rb_obj.kind_of?(Array)
                if rb_obj.size == 0
                    return { :kind => :array, :el => { :kind => :trivial, :str => "Object|null|undefined" } }
                end
                typs = rb_obj.map { |r| detect r }
                if typs.all? { |t| t == typs[0] }
                    return { :kind => :array, :el => typs[0] }
                end
            end
            if rb_obj == "#__undefined" || rb_obj == "#__null"
                return { :kind => :trivial, :str => "Any" }
            end
            raise "can't detect type for #{rb_obj}"
        end

        class Parser
            def initialize(str)
                @str = str
                @chrs = str.chars.reverse
            end

            def error(msg: nil)
                raise "can't parse type from '#{@str}' : tail is '#{@chrs.reverse[..100].join}' ; #{msg}"
            end

            def skip_ws
                while @chrs.size != 0 && /\s+/ =~ @chrs[-1]
                    @chrs.pop
                end
            end
            def fetch_expected(s)
                if @chrs[-1] != s
                    error(msg: "expected #{s}")
                end
                @chrs.pop
            end
            def starts_with(s)
                if s.size > @chrs.size
                    return false
                end
                s.each_char.with_index { |c, i|
                    if c != @chrs[-1 - i]
                        return false
                    end
                }
                return true
            end

            def run
                ret = step
                skip_ws
                if @chrs.size != 0
                    error(msg: "unparsed tail")
                end
                return ret
            end

            def parse_tuple_els
                skip_ws
                els = []
                while @chrs.size != 0 && @chrs[-1] != ']'
                    els.push(step)
                    skip_ws
                    if @chrs[-1] == ','
                        @chrs.pop
                    else
                        break
                    end
                    skip_ws
                end
                els
            end

            def parse_trivial
                stck = []
                res = String.new
                while @chrs.size > 0
                    if @chrs[-1] == '<' || @chrs[-1] == '['
                        stck.push @chrs[-1]
                        res << @chrs.pop
                    elsif @chrs[-1] == '>' || @chrs[-1] == ']'
                        if stck.size == 0
                            break
                        end
                        res << @chrs.pop
                        stck.pop
                    elsif stck.size == 0 && @chrs[-1] == ','
                        break
                    else
                        res << @chrs.pop
                    end
                end
                res
            end

            def step
                skip_ws
                if starts_with "Array<"
                    @chrs.pop("Array<".size)
                    el = step
                    skip_ws
                    fetch_expected '>'
                    return { :kind => :array, :el => el }
                elsif starts_with "Iterable<"
                    @chrs.pop("Iterable<".size)
                    el = step
                    skip_ws
                    fetch_expected '>'
                    return { :kind => :array, :el => el }
                elsif starts_with '['
                    @chrs.pop
                    els = parse_tuple_els
                    fetch_expected ']'
                    return { :kind => :tuple, :els => els }
                else
                    res = parse_trivial
                    return { :kind => :trivial, :str => res }
                end
            end
        end
    end
end
