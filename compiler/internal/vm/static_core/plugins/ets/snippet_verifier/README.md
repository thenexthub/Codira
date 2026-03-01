## Requirements

#### Soft

* Python 3.6+
* Ubuntu 22+ (not tested on other systems)
* tsc
* builded es2panda

#### Python pakages:

* os
* argparse
* re

## Testing process with terminal

```powershell
cd static_core/plugits/ets/spec_verifier
./verificate.sh  [-b= | --build=path/to/build]  [-s= | --spec=/path/to/spec] / [-r= | --rst=path/to/specific/rst/file ]
```
### Flags:

* `--build` - path to panda build
* `--spec` - path to spec directory (plugins/ets/doc/spec)
* `--rst` - path to specific .rst file

You can test just the single .rst file with option `--rst=...` or test the entire directory with the `--spec=...`

### Verifier syntax
Due to the active development status of the interface in the ETS specification, discrepancies may arise between the stated and actual behavior of the given code examples. To automate the verification of the specification's accuracy, it is proposed to introduce a tool with the capabilities of specifying parameters for each snippet directly in .rst files.

#### Snippet parameters list:
- skip - do not test this snippet
- partN - The code block is the N part of the overall snippet
- expect-cte: "..."(optional) - the type of error stated in the specification. If the error type is not specified, but the code should not be compiled, any error will be considered valid. The parameter value must contain the required elements of the error message
- not-subset - this code is not subset of ts

```rts
.. code-block-meta:
    skip
    partN
    expect-cte: "compile-time error name"
    not-subset

.. code-block:: typescript
...
```
__Default:__
- skip: false
- part: 0 (it is a whole snippet)
- expect-cte: false (successful compilation is expected)
- not-subset: false

The parameters of the snippet should be declared immediately before the code block in .rst file.
Parameters unrelated to the snippet can be omitted. For example:

```rts
.. code-block-meta:
    skip
```
```rts
.. code-block-meta:
    not-subset
    expect-cte
```
```rts
.. code-block-meta:
    except-cte: "SyntaxError: Field type annotation expected"
```

### Options for snippet:
- #### blank code-block-meta
**Every snippet in .rst should be marked with ".. code-block-meta:"**, but there is cases when snippet has no specific metadata.
In case all snippet's options are default, you can simply write ".. code-block-meta:" without any additional parameters.

Example: *plugins/ets/doc/spec/2_lexical.rst*
``` rst
.. code-block-meta:

.. code-block:: typescript
:linenos:

let s1 = 'Hello, world!'
let s2 = "Hello, world!"
let s3 = "\\"
let s4 = ""
let s5 = "don’t worry, be happy"
let s6 = 'don\'t worry, be happy'
let s7 = 'don\u0027t worry, be happy'
|
```
- #### *skip*
Some snippets does not require compilation attempts.
Example: *plugins/ets/doc/spec/2_lexical.rst::455:*
```typescript
153 // decimal literal
1_153 // decimal literal
0xBAD3 // hex literal
0xBAD_3 // hex literal
0o777 // octal literal
0b101 // binary literal
```
- #### *partN*
In the specification, there are examples that are split into several separate code (such as code block - explanation - code block), but they should be tested as a single program. (such as \**code block* - *explanation* - *code block*\*).
Example: *plugins/ets/doc/spec/7_expressions.rst*
```rst
The examples below illustrate valid and invalid usages of variance.
Let class ``Base`` be defined as follows:

.. code-block-meta:
    part1

.. code-block:: typescript
:linenos:

class Base {
method_one(p: Base): Base {}
method_two(p: Derived): Base {}
method_three(p: Derived): Derived {}
}

Then the code below is valid:

.. code-block-meta:
        part2

.. code-block:: typescript
:linenos:

class Derived extends Base {
// invariance: parameter type and return type are unchanged
override method_one(p: Base): Base {}

// covariance for the return type: Derived is a subtype of Base
override method_two(p: Derived): Derived {}

// contravariance for parameter types: Base is a super type for Derived
override method_three(p: Base): Derived {}
}

```
- #### *expect-cte*

Some snippets are not intended to be compiled, and this affects the results of the verifier

- Example without a specific error:
*plugins/ets/doc/spec/3_types.rst* should not compile without any extra conditions


```rst

.. code-block-meta:
    expect-cte

.. code-block:: typescript
:linenos:

let x: void // compile-time error - void
tsatsulya/arkcompiler_runtime_core
tsatsulya/arkcompiler_runtime_core
gitee.com
used as type annotation

function foo (): void
let y = foo() // void used as a value
```
During development, not only the support status of a feature may change, but also the type of compilation error it produces. Therefore, in this plugin, there is an option to *specify the specific type of error if the code should not compile*(**NOT IMPLEMENTED YET**).
- Example with a specific error: */plugins/ets/doc/spec/14_ambients.rst*
``` rst
.. code-block-meta:
    expect-cte: SyntaxError: Field type annotation expected

.. code-block:: typescript
:linenos:

declare namespace A{
declare function foo(): void // compile-time error
}
```
In this case compilation error should be *"SyntaxError: Field type annotation expected"* (**NOT IMPLEMENTED YET**)


- #### *not-subset*
Subset code should be tested not only with es2panda but also with tsc. Not subset - shouldn't
Example: *plugins/ets/doc/spec/17_experimental.rst*
```rst
Similarly to |TS|, |LANG| supports the launching of a coroutine by calling
the function *async* (see :ref:`Async Functions`). No restrictions apply as to
from what scope to call the function *async*.

.. index::
return type
function call
coroutine
function async
restriction

.. code-block-meta:
    not-subset

.. code-block:: typescript
:linenos:

async function foo(): Promise<int> {}

// This will create and launch coroutine
let resfoo = foo()

```

- #### *name:*
    This option may be used for adding snippet in *skiplist*
    Skiplist file contains names of the snippets which should be skiped. For example, frontend status for some snippets is "None", so there is no point in verifying them.

### Examples of the syntax using

```rst
.. code-block-meta:
    expect-cte
    not-subset

.. code-block:: typescript
:linenos:
*...code...*
```
### Incorrect syntax using
```rst
.. code-block-meta:
    expect-cte
    not-subset

.. code-block:: typescript
/* ... */
```
Explanation: in context of this verifier "subset code" - is a code, which is correct for .ts AND .ets. For this reason, expectation of compilation error can't be combined with any subset status

### Results

In **spec_verifier/snippets** you can find the snippets that were found in .rst file(files).
The snippet names are written in *theme_line* format, where theme - is the name of .rst file, line - line in .rst file where snippet were found. E.g.
> 3_types_232.ets - the code that can be found in the 232 line in 3_types.rst

**The snippets/_output.txt** contains the results of an attempts to compile snippets
snippets/abs contains .abs and .json files.

**spec_verifier/resuls** you can find the .md tables with the results for each theme and **main_results.txt** - file with all results in csv format

### Conditions for successful execute
1) All snippets are marked at least with ".. code-block-meta".
2) All parameter combinations are correct
3) All real statuses match the expected ones
