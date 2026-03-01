# libbase components

## pandargs

### Description:

pandargs is header-only utility tool that helps to parse command line arguments. It supports several argument types:
- integer
- double
- boolean
- string
- uint32_t
- uint64_t
- list
- compound

The more detail description of each type is in "Usage" section below.

Source code of pandargs contains in `utils/pandargs.h` file.

### Usage:

pandargs API consists of two major entities: template class `PandArg`, which represents an argument and `PandArgParser` class, which represents a parser.

Instead of `PandArg`, `PandArgCompound` can be used, which serves for creation of the compound arguments.
It inherits `PandArg<bool>`.

#### Arguments

To create an argument, it's template constructor should be called. Here is an instance:

```c++
                              // argument name | default value | argument description
    ark::PandArg<bool>   pab("bool",           false,        "Sample boolean argument");
                              // argument name | argument description     | sub-arguments
    PandArgCompound        arg("compound",      "Sample boolean argument", {&sub_bool_arg, &sub_int_arg, &sub_double_arg, &sub_string_arg});
```

Constructor can accept: 
- 3 parameters: argument name, default value, description.
- 4 parameters for single list: argument name, default value, description, delimiter.
- 5 parameters for integer args: argument name, default value, description, min value, max value
- PandArgCompound accepts three parameters: argument name, description and list of sub-arguments

There is description for them:
- Argument name, is a name, which will appear in a command line.
- Default value is a value argument will have regardless was it parsed or not.
- Argument description will be used to form a help message.
- Delimiter is a character or string that separates the different value if the single argument list
- Min value is the number that the integer argument cannot be less than
- Max value is the number that the integer argument cannot be greater than
- List of sub-arguments has type `std::initializer_list<PandArgBase *>`, and it accepts any non-compound arguments.
  Sub-arguments should not be added to the parser via `PandArgParser::Add`.

Template parameter is an argument type. Following values could be passed:
- `int` for integer argument
- `double` for double argument
- `bool` for boolean argument
- `std::string` for string argument
- `uint64_t` for uint64_t argument
- `uint32_t` for uint32_t argument
- `arg_list_t` for list argument

`arg_list_t` is a type, declared in `pandargs.h` under `panda` namespace, which is an alias for `std::vector<std::string>` type.

`PandArg` provides following public API:
- `PandArgType GetType()` - returns type of an argument
- `std::string GetName()` - returns name of an argument
- `std::string GetDesc()` - returns description of an argument
- `T GetValue()` - returns value of an argument depending on it's type
- `T GetDefaultValue()` - returns default value of an argument
- `void SetValue(T val)` - set value of an argument
- `ResetDefaultValue()` - set value back to default one

#### Argument types
There are three global argument types in pandargs:
- regular arguments
- tail arguments
- remainder arguments

Regular arguments are typical non-positional arguments like ```--arg=1```
Tail arguments are positional arguments, which should be introduced with help of parser API
Remainder arguments are arguments that come after trailing `--`. All of them are plain std::vector of std::string

Compound argument is a boolean argument, which can contains sub-arguments, followed by symbol `:`,
e.g: `--compiler-dump:folder=ir_dump,compact`. Sub-arguments follow the same rules as the regular arguments
and can be any type of argument. Compound arguments implicitly have default value `false` and user can't change it.

To access compound arguments from `C++`, regular convention should be used, for accessing sub-arguments, method name
contains name of the compound argument plus name of the sub-argument.
E.g. `--compiler-dump:compact`, here, to access `compact` sub-arguments `[Is|Get]CompilerDumpCompact()` should be used.

#### Parser

`PandArgParser` provides following public API:
- `bool Add(PandArgBase* arg)` - add an argument to parser. Returns `true` if argument was succsessfully added. `PandArgBase` is a base type for all template argument types
- `bool Parse(int argc, const char* argv[])` - parse arguments. Returns `true` on success. Note: `argv` & `argc` should be passed as is, without any amendments
- `std::string GetErrorString()` - returns error string if error occurred on argument addition or parsing
- `void EnableTail()` - enables positional arguments
- `void DisableTail()` - disables positional arguments
- `bool IsTailEnabled()` - returns `true` if positional arguments enabled
- `bool IsArgSet(PandArgBase* arg)` - returns `true` if an argument was added to a parser
- `bool IsArgSet(const std::string& arg_name)` - returns `true` if an argument with given name was added to a parser
- `bool PushBackTail(PandArgBase* arg)` - add tail argument to the end of tail arguments list. `false` if argument already in a tail list
- `bool PopBackTail()` - remove last argument from tail list
- `void EraseTail()` - remove all arguments from tail list
- `void EnableRemainder()` - enable remainder argument
- `void DisableRemainder()` - disable remainder argument
- `bool IsRemainderEnabled()` - `true` if remainder argument enabled
- `arg_list_t GetRemainder()` - returns remainder argument
- `std::string GetHelpString()` - returns string with all arguments and their description
- `std::string GetRegularArgs()` - returns string with all regular arguments and their values

Tail argument is a sequence of positinal arguments values. Function ```PushBackTail()``` adds an argument to the end of sequence, ```PopBackTail()``` removes the last added argument. Tail arguments may be added to a parser when tail is disabled, but they will be ignored if tail is disabled while parsing

Sample parser usage:
```c++
    ark::PandArgParser pa_parser;
    pa_parser.EnableTail();
    pa_parser.Add(&pab);
    if (!pa_parser.Add(&pab)) {
        std::cout << pa_parser.GetErrorString();
    }
    if (!pa_parser.Parse(argc, argv)) {
        std::cout << pa_parser.GetErrorString();
    }
```

#### Command line arguments convention

- Any non-positional argument should start with `--` (double dash) prefix
- Argument and it's value may be separated either by whitespace (` `) or by equals (`=`) sign
- If tail (positional arguments) enabled, first argument without double dash prefix concidered as a begin of positional arguments sequence
- Positional arguments should be without names or `=` signs, separated by whitespaces
- Boolean argument may be used without a value. This arguments always considered as `true`
- Remainder arguments are all literals that come after trailing `--`
- True values for boolean arguments: **true**, **on**, **1**
- False values for boolean arguments: **false**, **off**, **0**
- for integer arguments it's possible to define value range
- list values must be repeated with arg name or separated by delimiter
- string and list arguments may accept no parameters

Sample command line usage:
```bash
$ ./app --bool # bool is true
$ ./app --bool= # bool is true
$ ./app --bool on --bool1=off # bool is true, bool1 is false
$ ./app --uint32=32 # uint32 is 32
$ ./app --uint64=64 # uint64 is 64
$ ./app --string="a string" # string is "a string"
$ ./app --string "a string" # string is "a string"
$ ./app --string string # string is "string"
$ ./app --string= --int=1 # string is an empty string, int is 1
$ ./app --list=val1 --list=val2 --list=val3 # list argument example
$ ./app --slist=val1:val2:val3 # list argument example
$ ./app --int=0x40 --uint64=0x400 # int is 64, uint64 is 1024
$ ./app --int=1 false -1 "list1 list2 list3" # tail arguments example
$ ./app --double 3.14 --bool off -- arg1 --arg2=1 --arg3=false # remainder arguments example
$ ./app --compound:bool,int=2,double=54.321,string=Hello # Compound argument example
```
In the tail arguments example, `false` is a boolean value, `-1` is integer value and `str1` and `str2` is a string value. List may not be a tail argument. Positional values verified the same way as regular values, difference only that it's names are ignored in an input string, but position matters
In the remainder arguments example, all literals coming after `--` will go to remainder and can be obtained by `GetRemainder()` function

How to add tail arguments:
```c++
    ark::PandArgParser pa_parser;
    pa_parser.EnableTail();
    // now pab will be processed as a positional argument
    if (!pa_parser.PushBackTail(&pab)) {
        std::cout << pa_parser.GetErrorString();
    }
    if (!pa_parser.Parse(argc, argv)) {
        std::cout << pa_parser.GetErrorString();
    }
```
