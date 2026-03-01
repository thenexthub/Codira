..
    Copyright (c) 2024-2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

How to Use System ArkTS
=======================

The main goal of "System |LANG|" is to help developers make their ArkTS code
more efficient.

System ArkTS provides tips to focus on the performance. System |LANG| gives
an appropriate warning and suggests what other features a developer can use
to rewrite the code.

All "System |LANG|" warnings are divided into the following categories:

#. **Subset warnings**: Part of |LANG| that is common with |TS|.
   The original code requires modifications that keep the code within the Common
   Subset with |TS|.
#. **Non-subset warnings**: Part of |LANG| that differs from |TS|.
   The original code requires modifications that are not within the Common
   Subset with |TS|.

Possible options are as follows:

* **ets-subset-warnings**              : Enable all ETS-warnings to keep the code in subset with |TS|.
* **ets-non-subset-warnings**          : Enable all ETS-warnings that are not in subset with |TS|.
* **ets-warnings-all**                 : Enable all ETS-warnings in "System |LANG|".
* **ets-werror**                       : Treat all enabled ETS-warnings as errors.

* **ets-implicit-boxing-unboxing**     : Check if a program contains implicit boxing or unboxing. ETS Subset Warning.
* **ets-boost-equality-expression**    : Suggest boosting equality expressions. ETS Subset Warning.
* **ets-remove-async**                 : Suggest replacing async functions with coroutines. ETS Non-subset Warning.
* **ets-suggest-final**                : Suggest using the keyword ``final``. ETS Non-subset Warning.
* **ets-remove-lambda**                : Suggestions to replace lambda with regular functions. ETS Subset Warning.


To see "System |LANG|" warnings, add "System |LANG|" options from the list above while compiling.

|

Usage Example
-------------

1. Write some |LANG| code. E.g., you have the following chunk of code:

.. code-block:: typescript

    class A {
        foo(): String {
            return "foo";
        };
    }

    class K extends A  {
        foo_to_suggest(): void {};
        override foo(): String {
            return "overridden_foo";
        }
    }

2. Specify one option to the compiler. E.g., add ``--ets-suggest-final`` or ``--ets-suggest-final=true``;
3. Compile your file or project with the options so added;
4. Look through the ETS-Warnings in the output. For the code in the example
   above, "System |LANG|" gives the following warnings:

    * ``Warning: Suggest 'final' modifier for class 'K'. [usage.ets:7:11]``

    * ``Warning: Suggest 'final' modifier for method 'foo_to_suggest'. [usage.ets:8:23]``

    * ``Warning: Suggest 'final' modifier for method 'foo'. [usage.ets:9:21]``

5. Rewrite the code as suggested by "System |LANG|". After rewriting |LANG|,
   the code is as follows:

.. code:: typescript

    // Important! Shown feature is non-subset with TypeScript.
    class A {
        foo(): String {
            return "foo";
        };
    }

    final class K extends A {
        final foo_to_suggest(): void {};
        final override foo(): String {
            return "overridden_foo";
        }
    }

|

``ets-werror`` Usage Example
----------------------------

This section explains handling "System |LANG|" warnings as errors by
continuing the example as above:

1. Write some |LANG| code. E.g., you have the following chunk of code:

.. code-block:: typescript

    class I { // No final - inheritance }

    class A extends I { // Suggest final }

2. Specify one option to the compiler and enable ``ets-werror``. E.g., add ``--ets-suggest-final --ets-werror``;
3. Compile your file or project with the options so added, and a compile-time error occurs;
4. Look through the ETS-Warnings in the output. For the code in the example
   above, "System |LANG|" gives the following warnings:

    * ``System ArkTS. Warning treated as error: Suggest 'final' modifier for class [werror.ets:4:11]``

5. Rewrite the code as suggested by "System |LANG|". After rewriting |LANG|,
   the code is as follows:

.. code:: typescript


    class I { // No final - inheritance }

    final class A extends I { }

|

Status of not implemented features
-----------------------------------

System |LANG| team is working to provide |LANG| developers even more
performance-related tips and suggestions. In the near future we are to
investigate into the following possible performance leaks:

* Union usage;
* Nullable types;
* Rest parameters check vs. Array usage
* Non-throwing function.

See status updates in the following releases.

|
