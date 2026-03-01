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

.. _Runtime implementation details:

Runtime implementation details
******************************

.. meta:
    frontend_status: Done

.. _Scheduling rules:

Scheduling rules
================

.. meta:
    frontend_status: Done

The runtime environment schedules the |C_JOBS| that are defined by an |LANG|
program in accordance with the following rules:

- Every |C_JOB| has a priority, which depends solely on its type. The list of
  types, from highest to lowest priority:

  1. |C_COROS| and ``Promise`` callbacks (``.then()``, etc.);
  2. Other |C_JOBS|
  
- Within each |C_WORKER|, the |C_JOBS| with higher priority are scheduled
  before |C_JOBS| with lower priority;
- All |C_JOBS| with the same priority are scheduled in the FIFO order;

.. note::
   These rules will be updated.

.. index::
   term1
   term 2

|

