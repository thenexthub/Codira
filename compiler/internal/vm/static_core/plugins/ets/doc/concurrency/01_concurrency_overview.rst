..
    Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

This chapter covers the |LANG| execution model and the language features that
provide support for asynchronous and parallel code execution.

|

.. _Execution model:

Execution model
***************

.. meta:
    frontend_status: Done

A program in |LANG| defines one or more tasks (|C_JOBS|) that are executed by
the runtime environment. If requested, and if target platform allows this,
|C_JOBS| can run in parallel for efficient hardware resource utilization. 

Given that the target platform allows for concurrent code execution, a
|C_WORKER| is an abstraction over platform provided unit of concurrency.
Typically, it will map 1:1 with OS threads. That means:

-  every |C_JOB| is hosted by a |C_WORKER| with only one |C_JOB| per |C_WORKER|
   being executed at once
-  if two |C_JOBS| run on different |C_WORKERS| then their code is able to run
   in parallel
-  if two |C_JOBS| share the same |C_WORKER| then their code can never run in
   parallel

A |C_JOB| can have zero or more suspension points. |C_JOB| execution can be
paused at a suspension point and resumed at a later moment in time. Once
suspended, a |C_JOB| allows another |C_JOB| to be executed on the same
|C_WORKER| .

Any |LANG| program implicitly defines one# *main** |C_JOB|, which corresponds to
the :ref:`Program Entry Point`. The execution starts from it, and its completion
initiates the program termination sequence.

The exact language features and standard library APIs that are used for defining
|C_JOBS| and their respective suspension points are described in the subsequent
sections.

The program memory is shared between all |C_JOBS| , which allows for efficient
data sharing but implies that the developer should use the provided means of
synchronization to avoid race conditions and guarantee thread safety.


.. _Overview of concurrency features:

Overview of concurrency features
********************************

|LANG| allows for both asynchronous and parallel programming, and provides
machinery for trustworthy concurrent programs by providing the following:

- :ref:`Asynchronous execution` primitives: ``async`` / ``await`` / ``Promise``;
- :ref:`Parallel execution` API: ``EAWorker API`` / ``Taskpool API`` / 
  ``launch API``, including the structured concurrency support;
- :ref:`Synchronization` API: ``this`` and ``that``;

The :ref:`API details and restrictions` section provides the detailed API
description and the restrictions on its usage.

.. index::
   concurrency
   asynchronous programming
   coroutine
   shared memory
   functionality
   parallel-run coroutine
   structured concurrency
   synchronization

|


