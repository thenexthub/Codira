..
    Copyright (c) 2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

|

Warning Suppression
====================

"System |LANG|" has some features to boost performance. However, developers can
enable or disable any single feature or a group of features if wanting to avoid
checks in some cases.

Developers can also disable checks for certain strings or parts of code by
using ``ETSNOLINT``. Possible warning-suppressions are as follows:

* ``ETSNOLINT``:
    Applies to the current line;
* ``ETSNOLINT-NEXTLINE``:
    Applies to the next line; and
* ``ETSNOLINT-BEGIN`` with ``ETSNOLINT-END``:
    Applies to all lines (including the line with the directive) until closed by the corresponding ``ETSNOLINT-END``.


**Note**: ``ETSNOLINT-END`` reset the impact of ``ETSNOLINT-BEGIN`` in
accordance with the arguments. Using ``ETSNOLINT-BEGIN(ets-remove-async,ets-remove-lambda)``
and ``ETSNOLINT-END(ets-remove-async)`` makes ``ets-remove-lambda`` continue
until another ``ETSNOLINT-END`` directive containing ``ets-remove-lambda`` is
found, e.g. ``ETSNOLINT-END(ets-remove-lambda)``.

All types of ``ETSNOLINT`` can be used either with or without arguments.
Arguments must be separated with commas without whitespaces.
Only the specified warning types are suppressed.
If there is no argument, then all warnings will be suppressed.

* ``ETSNOLINT(ets-suggest-final)``:
    Disable ``ets-suggest-final``. Applies to the current line.
* ``ETSNOLINT-NEXTLINE(ets-implicit-boxing-unboxing)``:
    Disable ``ets-implicit-boxing-unboxing``. Applies to the next line.
* ``ETSNOLINT-BEGIN(ets-implicit-boxing-unboxing,ets-suggest-final)``:
    Disable ``ets-implicit-boxing-unboxing`` and ``ets-suggest-final`` checks. Applied to all lines (including the line with the directive) until closed by the corresponding ``ETSNOLINT-END``.

Other combinations are also valid. The list of possible arguments is the same as the list of ETS-warnings. Add any type of suppression to single-line
or multiline comments.

Appropriate examples with |LANG| code are provided below:

|LANG| Warning Suppression Common
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A { /* ETSNOLINT */
        foo(): String {
            return "foo";
        };
    }

or

.. code-block:: typescript

    class A { // ETSNOLINT
        foo(): String {
            return "foo";
        };
    }

|LANG| Warning Suppression Special
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A { /* ETSNOLINT(ets-suggest-final) */
        foo(): String {
            return "foo";
        };
    }

or

.. code-block:: typescript

    class A { // ETSNOLINT(ets-suggest-final)
        foo(): String {
            return "foo";
        };
    }


|LANG| Warning Suppression -- More Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // ETSNOLINT-BEGIN
    class A {
        foo(): String {
            return "foo";
        };
    }
    // ETSNOLINT-END

.. code-block:: typescript

    // ETSNOLINT-NEXTLINE
    class A {
        foo(): String {
            return "foo";
        };
    }

.. code-block:: typescript

    // ETSNOLINT-NEXTLINE(ets-suggest-final)
    class A {
        foo(): String {
            return "foo";
        };
    }

.. code-block:: typescript

    // ETSNOLINT-BEGIN(ets-suggest-final)
    class A {
        foo(): String {
            return "foo";
        };
    }
    // ETSNOLINT-END(ets-suggest-final)


|
