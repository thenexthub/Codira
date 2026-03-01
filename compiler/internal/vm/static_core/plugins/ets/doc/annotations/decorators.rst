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

.. _TS Decorators:

|TS| Decorators
###############

Annotations appear similar to the |TS| decorators, but they are quite
different by nature.

A decorator always refers to a user-defined function
that modifies run-time behavior of an entity it is applied to.

Annotations are common for mainstream static languages, and are mostly
used to attach metadata to code.

In short, using a decorator requires a developer to write the exact
modification code, while using an annotation presumes that the developer
adds meta information that, in turn, instructs the compiler, linker, or other
tool to transform the code in specific way, or to produce some useful output.

Annotations are useful in implementing AOP, declarative UI frameworks, and
other frameworks. Using annotations is generally safer, and often more
efficient.

|
