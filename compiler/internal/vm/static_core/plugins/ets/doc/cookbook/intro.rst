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

|

Introduction
============

Welcome to the "|TS| to |LANG|" cookbook. This document provides short
recipes to rewrite your standard |TS| code to |LANG|. Although |LANG| is
designed to be close to |TS|, some limitations are added for the sake of
performance. As a result, all |TS| features can be categorized as follows:

#. **Fully supported features** require no modification of the original code.
   According to our measurements, projects that already follow the best |TS|
   practices can keep 90% to 97% of their codebase intact.
#. **Partially supported features** require some minor code refactoring.
   Example: The keyword ``var`` must be replaced for ``let`` to declare
   variables. Please note that your code still remains a valid |TS| code
   after rewriting by our recipes.
#. **Unsupported features** can require more extensive code refactoring.
   Example: Type ``any`` is unsupported, and you are to introduce explicit
   typing to your code everywhere ``any`` is used.

|

.. raw:: pdf

   PageBreak


