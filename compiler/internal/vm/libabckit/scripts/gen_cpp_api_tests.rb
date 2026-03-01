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

require 'erb'

abckit_scripts = File.dirname(__FILE__)
abckit_root = File.expand_path("../", abckit_scripts)
abckit_test = File.join(abckit_root, "/tests/mock/tests/")


def getReturnTypeString(arr)
    str = ""
    arr[1].each do |el|
        str += el + " "
    end
    return str.slice(0, str.length - 1)
end

def getReturnType(arr)
    return arr[1]
end

def getName(arr)
    return arr[0][1]
end

def getClass(arr)
    return arr[0][0]
end

def getFuncArgs(arr)
    return arr[2]
end

def fixCppApiName(foo)
    if getClass(foo) == "DynamicIsa"
        if getName(foo)[0..5] == "Create"
            return ("Icreate" + getName(foo).slice(6, getName(foo).length))
        end
        if getName(foo)[0..2] == "Get"
            return ("Iget" + getName(foo).slice(3, getName(foo).length))
        end
        if getName(foo)[0..2] == "Set"
            return ("Iset" + getName(foo).slice(3, getName(foo).length))
        end
    end
    if getClass(foo) == "BasicBlock"
        return ("BB" + getName(foo))
    end
    if getClass(foo) == "Graph"
        return ("G" + getName(foo))
    end
    if getClass(foo) == "Value"
        return ("Value" + getName(foo))
    end
    if getClass(foo) == "Literal"
        return ("Literal" + getName(foo))
    end
    return getName(foo)
end

def argIsVararg(arg)
    if arg[arg.length - 1].include?("...") || arg[arg.length - 2].include?("...")
        return true
    end
    return false
end

def generateArgsFromHelpers(args)
    default_types = Hash.new
    default_types = {
        default_types = {
            "double" => "DEFAULT_DOUBLE",
            "int" => "DEFAULT_I32",
            "int32_t" => "DEFAULT_I32",
            "uint8_t" => "DEFAULT_U8",
            "uint16_t" => "DEFAULT_U16",
            "uint32_t" => "DEFAULT_U32",
            "uint64_t" => "DEFAULT_U64",
            "bool" => "DEFAULT_BOOL",
            "std::function<void(BasicBlock)>&" => "DEFAULT_CB",
            "std::function<bool(core::Module)>&" => "DEFAULT_CB",
            "std::string" => "DEFAULT_CONST_CHAR",
            "std::string&" => "DEFAULT_CONST_CHAR",
            "std::string_view" => "DEFAULT_CONST_CHAR",
            "std::string_view&" => "DEFAULT_CONST_CHAR",
            "instructions&&" => "abckit\:\:mock\:\:helpers\:\:GetMockInstruction(f)",
            "AbckitIsaApiDynamicConditionCode" => "DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE"
        }
    }
    str = ""
    args.each do |arg|
        str += ""
        if default_types.include?(arg[arg.length - 2])
            str += default_types[arg[arg.length - 2]].to_s + ", "
        elsif argIsVararg(arg)
            str += "abckit\:\:mock\:\:helpers\:\:GetMockInstruction(f)"
            str += ", "
        else
            str += "abckit\:\:mock\:\:helpers\:\:GetMock" + arg[arg.length - 2].delete("&").delete("*").gsub("core\:\:", "Core") + "(f)" + ", "
        end
    end
    return str.length == 0 ? str : str.slice(0, str.rindex(','));
end

def parseArgs(arg_str)
    args_arr = []
    arr = arg_str.split(',')
    arr.each do |el|
        arg = el.split(" ")
        if arg[arg.length - 1][0..1] == '&&'
            arg[arg.length - 2] += '&&'
            arg[arg.length - 1].delete!('&')
        end
        if arg[arg.length - 1][0] == '&'
            arg[arg.length - 2] += '&'
            arg[arg.length - 1].delete!('&')
        end
        if arg[arg.length - 1][0] == '*'
            arg[arg.length - 2] += '*'
            arg[arg.length - 1].delete!('*')
        end
        args_arr << arg
    end
    return args_arr
end

def parseFunction(str, cls)
    arr = []
    name = str.slice(0, str.index('('));
    ret_type = name.slice(0, name.rindex(' '))
    name = name.slice(name.rindex(' '), name.length)
    name.delete! " "

    if name[0..1] == '&&'
        ret_type += '&&'
        name = name.slice(2, name.length)
    end

    if name[0] == '&'
        ret_type += '&'
        name = name.slice(1, name.length)
    end

    if name[0] == '*'
        ret_type += '*'
        name = name.slice(1, name.length)
    end

    ret_type_arr = ret_type.split(" ")
    if ret_type.include? "template"
        s = ""
        i = 0
        while ret_type_arr[0] do
            s += ret_type_arr[0] + " "
            ret_type_arr.delete_at(0)
            if s.include? ">"
                s = s.slice(0, s.length - 1)
                break
            end
        end
        ret_type_arr[0] = s
    end

    arr << [cls, name]
    arr << ret_type_arr
    arr << parseArgs(str.slice(str.index('(') + 1, str.rindex(')') - str.index('(') - 1))

    return arr
end

def parseFile(filePath, cls)
    file = File.open(filePath)
    file_data = file.readlines.map(&:chomp)
    while file_data[0] != "public:" do
        file_data.delete_at(0)
    end
    file_data.delete_at(0)
    file_data.delete_if {|x| x.include? "*/" or x.include? "* "or x.include? "//" or x.include? "/**" or x.empty? }
    file_data.delete_if {|x| x.include?("default")}
    file_data.delete_if {|x| x.include?("operator")}
    if(file_data.include?("protected:"))
        file_data = file_data.slice(0, file_data.index("protected:"))
    elsif (file_data.include?("private:"))
        file_data = file_data.slice(0, file_data.index("private:"))
    elsif (file_data.include?("};"))
        file_data = file_data.slice(0, file_data.index("};"))
    end

    size = file_data.size
    i = 0
    while i < (size - 1) do
        if !file_data[i].include?(";")
            if file_data[i + 1].include? "{"
                file_data[i] += ";"
                while !file_data[i + 1].include? "}" do
                    file_data.delete_at(i + 1)
                    size = size - 1
                end
                file_data.delete_at(i + 1)
                size = size - 1
                next
            end
            if file_data[i].include? "{"
                file_data.delete("{")
                file_data[i] += ";"
                while !file_data[i + 1].include? "}" do
                    file_data.delete_at(i + 1)
                    size = size - 1
                end
                file_data.delete_at(i + 1)
                size = size - 1
                next
            end
            file_data[i] = file_data[i] + file_data[i + 1]
            file_data.delete_at(i + 1)
            size = size - 1
            next
        end
        i = i + 1
    end
    puts file_data

    parsed_functions = []
    i = 0
    while i < file_data.size do
        parsed_functions << parseFunction(file_data[i], cls)
        i = i + 1
    end
    parsed_functions.delete_if {|x| getReturnType(x).empty?}
    parsed_functions.delete_if {|x| getName(x) == getClass(x)}

    parsed_functions.each do |foo|
        puts foo.inspect
    end

    return parsed_functions
end

parsed_functions = []
parsed_functions += parseFile("../include/libabckit/cpp/headers/core/annotation_interface.h", "Annotation")

cpp_tests_erb = File.join(abckit_test, "cpp_api_test.cpp.erb")
testfile_fullpath = File.join(abckit_test, "cpp_api_gtests_gen.cpp")
res = ERB.new(File.read(cpp_tests_erb), nil, "%").result(binding)
File.write(testfile_fullpath, res)