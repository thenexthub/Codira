..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _How to Use the Cookbook:

How to Use the Cookbook
=======================

The main goal of this cookbook is to provide recipes for all partially
supported features, and explicitly list all unsupported features. The document
is built on the feature-by-feature basis. If you do not find a feature, then
you can safely consider it **supported**. Otherwise, a suggestion on how to
rewrite your code and work around an unsupported case is found in a recipe.

|

.. _Recipe Explained:

Recipe Explained
----------------

The original |TS| code containing the keyword ``var``:

.. code-block:: typescript

    function addTen(x: number): number {
        var ten = 10
        return x + ten
    }

---must be rewritten as follows:

.. code-block:: typescript

    // Important! This is still valid TypeScript.
    function addTen(x: number): number {
        let ten = 10
        return x + ten
    }

|

.. _Severity Levels:

Severity Levels
---------------

Each recipe has a severity level mark that supports the following values:

- |CB_ERROR|: You are to follow the recipe, otherwise program compilation fails.
- |CB_WARNING|: You are highly recommended to follow the recipe. Failing to
  follow the recipe does not affect compilation currently, but can cause a
  compilation failure in the future versions.

|

.. _Status of Unsupported Features:

Status of Unsupported Features
------------------------------

Currently unsupported are mainly the features that degrade the following:

- Runtime performance, by being related to dynamic typing;
- Project build time, by requiring extra support during compilation.

However, the |LANG| team reserves the right to reconsider and **shrink** the
list in future releases based on the developer feedback and more real-world
data experiments.

|

.. raw:: pdf

    PageBreak


