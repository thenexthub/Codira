..
    Copyright (c) 2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

+++++++++++++++++++++++++++++++++++++++++
Backlog(not related features for interop)
+++++++++++++++++++++++++++++++++++++++++

Backlog JS
##########

``eval``
********

Strict mode
***********

You can declare the strict mode by adding ``'use strict';`` or ``"use strict";`` at the beginning of a program.
This is an opportunity to make writing code safer and neater by limiting the use of some functions available in the language.
A complete program may be composed of both strict mode and non-strict mode source text units.

* Strict mode for scripts

    .. code-block:: javascript
        :linenos:

        // Whole-script strict mode syntax
        "use strict";
        const v = "Hi! I'm a strict mode script!";

* Strict mode for functions

    .. code-block:: javascript
        :linenos:

        function myStrictFunction() {
            // Function-level strict mode syntax
            "use strict";
            function nested() {
                return "And so am I!";
            }
            return `Hi! I'm a strict mode function! ${nested()}`;
        }

        function myNotStrictFunction() {
            return "I'm not strict.";
        }

Using ``"use strict"`` in functions with ``rest``, ``default``, or ``destructured`` parameters is a ``syntax error``.

* Strict mode for modules

The entire contents of JavaScript modules are automatically in strict mode, with no statement needed to initiate it.

* Strict mode for classes

All parts of a class's body are strict mode code, including both class declarations and class expressions.

**Changes in strict mode**

Strict mode changes both syntax and runtime behavior. Changes generally fall into these categories:

* changes converting mistakes into errors (as syntax errors or at runtime)

    * Assigning to undeclared variables

    * Failing to assign to object properties

    * Failing to delete object properties

    * Duplicate parameter names

    * Legacy octal literals

    * Setting properties on primitive values

    * Duplicate property names

* changes simplifying how variable references are resolved

    * Removal of the ``with`` statement

    * Block-scoped function declarations

    * Non-leaking ``eval``

* changes simplifying ``eval`` and ``arguments``

    * Preventing binding or assigning ``eval`` and ``arguments``

* changes making it easier to write "secure" JavaScript

    * no ``this`` substitution

    In sloppy mode, function calls like f() would pass ``the global object`` as the ``this`` value. In strict mode, it is now ``undefined``.
    When a function was called with call or apply, if the value was a primitive value, this one was boxed into an object (or the global object for undefined and null). In strict mode, the value is passed directly without conversion or replacement.

* changes anticipating future ECMAScript evolution


Scripts
*******
It's about the <script> tag. In HTML, JavaScript code is inserted between <script> and </script> tags.

.. .. code-block:: javascript
..     :linenos:

..     <script>
..     document.getElementById("demo").innerHTML = "My First JavaScript";
..     </script>


Template literals
*****************

Template literals are enclosed by backtick (`) characters instead of double or single quotes.

Along with having normal strings, template literals can also contain other parts called placeholders, which are embedded expressions delimited by a dollar sign and curly braces: ${expression}. The strings and placeholders get passed to a function — either a default function, or a function you supply. The default function (when you don't supply your own) just performs string interpolation to do substitution of the placeholders and then concatenate the parts into a single string.

To supply a function of your own, precede the template literal with a function name; the result is called a tagged template. In that case, the template literal is passed to your tag function, where you can then perform whatever operations you want on the different parts of the template literal.

.. code-block:: javascript
    :linenos:

    `string text`

    `string text line 1
    string text line 2`

    `string text ${expression} string text`

    tagFunction`string text ${expression} string text`


Function hoisting  (empty)
**************************

Description
^^^^^^^^^^^

Hoisting is a behavior in which a function or a variable can be used before declaration.

.. code-block:: javascript
    :linenos:

    sqr0(5); /* hoisting */
    // function declaration
    function sqr0 (val) { return val * val; }

Backlog ArkTS
#############

Shadowing by parameter
**********************

New Expressions
***************

Unary/Binary Expressions, Multiplicative Expressions
****************************************************
