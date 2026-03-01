# Codira Formatter User Guide

## Introduction

`CODEFMT (Codira Formatter)` is an automated code formatting tool developed based on the Codira language programming specifications.

## Usage Instructions

Command line usage: `codefmt [option] file [option] file`

`codefmt -h` displays help information and option descriptions

```text
Usage:
     codefmt -f fileName [-o fileName] [-l start:end]
     codefmt -d fileDir [-o fileDir]
Options:
   -h            Show usage
                     eg: codefmt -h
   -v            Show version
                     eg: codefmt -v
   -f            Specifies the file to be formatted. The value can be a relative or absolute path.
                     eg: codefmt -f test.code
   -d            Specifies the directory containing files to be formatted. The value can be a relative or absolute path.
                     eg: codefmt -d test/
   -o <value>    Output destination. For single file formatting, '-o' is followed by the output filename (supports relative/absolute paths).
                 For directory formatting, a path must be specified after -o (supports relative/absolute paths).
                     eg: codefmt -f a.code -o ./fmta.code
                     eg: codefmt -d ~/testsrc -o ./testout
   -c <value>    Specifies the formatting configuration file (supports relative/absolute paths).
                 If the specified config file cannot be read, codefmt will attempt to read the default config file from CODIRA_HOME.
                 If the default config also fails, built-in configurations will be used.
                     eg: codefmt -f a.code -c ./config/cangjie-format.toml
                     eg: codefmt -d ~/testsrc -c ~/home/project/config/cangjie-format.toml
   -l <region>   Only formats specified line ranges in the input file (only valid with single file mode).
                 Format: [start:end] where 'start' and 'end' are line numbers (1-based).
                     eg: codefmt -f a.code -o ./fmta.code -l 1:25
```

### File Formatting

`codefmt -f`

- Format and overwrite source file (supports relative/absolute paths):

```shell
codefmt -f ../../../test/uilang/Thread.code
```

- Option `-o` creates a new `.code` file with formatted output (supports relative/absolute paths):

```shell
codefmt -f ../../../test/uilang/Thread.code -o ../../../test/formated/Thread.code
```

### Directory Formatting

`codefmt -d`

- Option `-d` specifies the Codira source directory to scan and format (supports relative/absolute paths):

```shell
codefmt -d test/              // Relative path

codefmt -d /home/xxx/test     // Absolute path
```

- Option `-o` specifies output directory (can be existing or new). Note MAX_PATH limitations:
  - Windows: typically ≤260 characters
  - Linux: recommended ≤4096 characters

```shell
codefmt -d test/ -o /home/xxx/testout

codefmt -d /home/xxx/test -o ../testout/

codefmt -d testsrc/ -o /home/../testout   // Error if source directory doesn't exist
```

### Configuration File

`codefmt -c`

- Option `-c` allows custom formatting configuration:

```shell
codefmt -f a.code -c ./cangjie-format.toml
```

Default cangjie-format.toml configuration (also used as built-in defaults):

```toml
# indent width
indentWidth = 4 # Range: [0, 8]

# line length limit
linelimitLength = 120 # Range: [1, 120]

# line break type
lineBreakType = "LF" # "LF" or "CRLF"

# allow Multi-line Method Chain when it's level equal or greater than multipleLineMethodChainLevel
allowMultiLineMethodChain = false

# if allowMultiLineMethodChain is true,
# and method chain's level ≥ multipleLineMethodChainLevel,
# method chain will be formatted to multi-line.
# e.g. A.b().c() level=2, A.b().c().d() level=3
# ObjectA.b().c().d().e().f() =>
# ObjectA
#     .b()
#     .c()
#     .d()
#     .e()
#     .f()
multipleLineMethodChainLevel = 5 # Range: [2, 10]

# allow Multi-line Method Chain when exceeding linelimitLength
multipleLineMethodChainOverLineLength = true
```

> **Note:**
>
> If custom config fails, reads default `cangjie-format.toml` from CODIRA_HOME.
> If default config also fails, uses built-in configurations.
> For any failed config option, uses built-in default for that option.

### Partial Formatting

`codefmt -l`

- Option `-l` formats only specified line ranges (only works with single file mode `-f`):

```shell
codefmt -f a.code -o .code -l 10:25 // Formats only lines 10-25
```

## Formatting Rules

- Source files should contain (in order) copyright, package, import, and top-level elements, separated by blank lines.

【Correct Example】

```cangjie
// Part 1: Copyright
/*
 * Copyright (c) [Year of First Pubication]-[Year of Latest Update]. [Company Name]. All rights reserved.
 */

// Part 2: Package declaration
package com.myproduct.mymodule

// Part 3: Imports
import std.collection.HashMap   // Standard library

// Part 4: Public elements
public class ListItem <: Component {
    // CODE
}

// Part 5: Internal elements
class Helper {
    // CODE
}
```

> **Note:**
>
> The formatter doesn't enforce blank lines after copyright, but preserves one if present.

- Consistent 4-space indentation:

【Correct Example】

```cangjie
class ListItem {
    var content: Array<Int64> // Correct: 4-space indent relative to class
    init(
        content: Array<Int64>, // Correct: 4-space indent for parameters
        isShow!: Bool = true,
        id!: String = ""
    ) {
        this.content = content
    }
}
```

- K&R brace style for non-empty blocks:

【Correct Example】

```cangjie
enum TimeUnit { // Correct: Brace on same line with space
    Year | Month | Day | Hour
} // Correct: Closing brace on new line

class A { // Correct: Brace on same line with space
    var count = 1
}

func fn(a: Int64): Unit { // Correct: Brace on same line with space
    if (a > 0) { // Correct: Brace on same line with space
    // CODE
    } else { // Correct: } else on same line
        // CODE
    } // Correct: Closing brace on new line
}

// Lambda functions
let add = {
    base: Int64, bonus: Int64 => // Correct: K&R style for lambda
    print("Correct news")
    base + bonus
}
```

- Spacing around keywords and important elements (per G.FMT.10):

【Correct Example】

```cangjie
var isPresent: Bool = false  // Correct: Space after colon
func method(isEmpty!: Bool): RetType { ... } // Correct: Space after colons

method(isEmpty: isPresent) // Correct: Space after named parameter colon

0..MAX_COUNT : -1 // Correct: No space around .., spaces around step colon

var hundred = 0
do { // Correct: Space after do
    hundred++
} while (hundred < 100) // Correct: Space before while

func fn(paramName1: ArgType, paramName2: ArgType): ReturnType { // Correct: No inner paren spaces
    ...
    for (i in 1..4) { // Correct: No spaces around range operator
        ...
    }
}

let listOne: Array<Int64> = [1, 2, 3, 4] // Correct: No inner bracket spaces

let salary = base + bonus // Correct: Spaces around binary operators

x++ // Correct: No space for unary operators
```

- Minimize unnecessary blank lines:

【Incorrect Example】

```cangjie
class MyApp <: App {
    let album = albumCreate()
    let page: Router
    // Blank line
    // Blank line
    // Blank line
    init() {           // Incorrect: Multiple consecutive blank lines
        this.page = Router("album", album)
    }

    override func onCreate(): Unit {

        println( "album Init." )  // Incorrect: Blank lines inside braces

    }
}
```

- Minimize unnecessary semicolons:

【Before Formatting】

```cangjie
package demo.analyzer.filter.impl; // Redundant semicolon

internal import demo.analyzer.filter.StmtFilter; // Redundant semicolon
internal import demo.analyzer.CODEStatment; // Redundant semicolon

func fn(a: Int64): Unit {
    println( "album Init." );
}
```

【After Formatting】

```cangjie
package demo.analyzer.filter.impl // Semicolon removed

internal import demo.analyzer.filter.StmtFilter // Semicolon removed
internal import demo.analyzer.CODEStatment // Semicolon removed

func fn(a: Int64): Unit {
    println("album Init.");
}
```

- Modifier keyword ordering follows priority rules in G.FMT.12.Here are the recommended modifier precedence rules for top-level elements:

```cangjie
public
open/abstract
```

Here are the recommended modifier precedence rules for instance member functions or instance member properties:

```cangjie
public/protected/private
open
override
```

Here are the recommended modifier precedence rules for static member functions:

```cangjie
public/protected/private
static
redef
```

Here are the recommended modifier precedence rules for member variables:

```cangjie
public/protected/private
static
```

- Formatting behavior for multi-line comments

For comments starting with `*`, the `*` symbols will be aligned with each other. Comments not starting with `*` will retain their original formatting.

```cangjie
// Before formatting
/*
      * comment
      */

/*
        comment
        */

// After formatting
/*
 * comment
 */

/*
        comment
 */
```

## Notes

- The Codira formatting tool currently does not support formatting code with syntax errors.

- The Codira formatting tool currently does not support metaprogramming formatting.