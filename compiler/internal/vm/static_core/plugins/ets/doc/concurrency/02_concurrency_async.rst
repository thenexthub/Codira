..
    Copyright (c) 2026 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Asynchronous execution:

Asynchronous execution
**********************

.. meta:
    frontend_status: Done

The asynchronous execution capability addresses the situation when developer's
code regularly needs to wait for external (e.g network, timers or user input) or
internal (e.g. status updates from a |C_JOB| that is running on another
|C_WORKER|) events. For such cases, |LANG| provides a way to suspend execution
of a |C_JOB|, mark the |C_JOB| as blocked on a wait for certain event and resume
its execution later, once the event happens.

The |LANG| features that provide the asynchronous execution support are:

- the ``async`` and ``await`` keywords that mark suspendable (``asynchronous``)
  functions and suspension points inside such functions, respectively
- the ``Promise`` class in the Standard Library, which is an abstraction of the
  unfinished computation result that will get its value at some time in future.

.. _Async Functions:

Asynchronous Functions
======================

.. meta:
    frontend_status: Done

An asynchronous function is a function that can define suspension points inside
its body. A non-asynchronous function can not have suspension points.

.. note::
  A *suspension point* is the point in the function code where function
  execution can be *suspended* (paused) and, at some time in future, *resumed*.
  The suspension implies that control is transferred elsewhere, but all the
  local function state (e.g. the argument and local variable values)
  is preserved until the resumption.

Asynchronous functions should be marked with the ``async`` modifier and return
an instance of the generic :ref:`Concurrency Promise Class` class, which wraps
the return value. The ``async`` modifier is not a part of the function type: a
``non-async`` function that returns ``Promise`` and an ``async`` function with
the same return type and arguments are no different from the type system
perspective. The function that serves as the :ref:`Program Entry Point` can be
asynchronous, too.

Execution of an ``async`` function can be paused at suspension points, which are
defined with the :ref:`await Expression`. If suspension happens, the ``async``
function immediately returns a *pending* ``Promise`` instance. In case of the
first (by the control flow) suspension point, the control is transferred to the
caller of the asynchronous function, and the runtime environment creates a new
|C_CORO| that corresponds to the paused function. Eventually, in accordance with
the :ref:`Scheduling rules`, the ``async`` function resumes its execution from
the suspension point where the execution was paused. In case of second and
further suspension points, after the suspension happens, the runtime environment
schedules another |C_JOB| for execution. The |C_JOB| to be scheduled is chosen
in accordance with the :ref:`Scheduling rules`.

Execution of an asynchronous function completes either by a normal return or by
throwing an error. In both cases, the resulting value or error is
wrapped into a ``Promise`` class instance, *resolving* or *rejecting* it
respectively (see :ref:`Concurrency Promise Class` for details).

An asynchronous function with the return type ``Promise<T>`` can explicitly
return a ``Promise<T>`` instance (in this case, the returned value  is returned
"as is") or a value of type ``T``, which is then automatically boxed in an
instance of ``Promise<T>``. Both options are allowed to be the ``expression`` of
the ``return`` statement inside the ``async`` function body (see :ref:`return
Statements` and :ref:`Return Type Inference`). ``T`` here is a subtype of
:ref:`Type Any`. If ``T`` has ``void`` or ``undefined`` type (see :ref:`Types
void or undefined`) then, like in non-asynchronous functions, an argumentless
``return`` statement is allowed.

.. note::
   Summarizing: an asynchronous function with one or more suspension points
   defines a new |C_CORO| in an |LANG| program, which starts from the first
   suspension and ends with the asynchronous function completion. An
   asynchronous function with zero suspension points does not define any
   additional |C_JOBS|.

A :index:`compile-time error` occurs if:

- ``async`` function is called in a static initialization;
- ``async`` function has an ``abstract`` or a ``native`` modifier;
- return type of an ``async`` function is other than ``Promise``.
- a ``non-async`` function defines any suspension points

.. index::
   async function
   coroutine
   return type
   static initializer
   abstract function
   native function
   function body
   backward compatibility
   annotation
   no-argument return statement
   async function
   return statement
   compatibility

|

.. _Async Lambdas:

Asynchronous Lambdas
====================

.. meta:
    frontend_status: Done

A lambda can have the ``async`` modifier (see :ref:`Lambda Expressions`).

With regard to concurrency, ``async`` lambdas have the same semantics and
follow the same rules as :ref:`Async Functions`.

.. index::
   async lambda
   async modifier
   lambda expression
   coroutine

|

.. _Concurrency Async Methods:

Asynchronous Methods
====================

.. meta:
    frontend_status: Done

A static or instance class method can have the ``async`` modifier
(see :ref:`Method Declarations`).

With regard to concurrency, ``async`` methods have the same semantics and
follow the same rules as :ref:`Async Functions`.

.. index::
   async method
   class method
   async modifier
   method declaration
   coroutine

|

.. _await Expression:

``await`` expression
====================

.. meta:
    frontend_status: Done

An ``await`` expression defines a suspension point within the asynchronous
function body. The syntax of the ``await`` expression is as follows:

.. code-block:: abnf

    awaitExpression:
        'await' expression
        ;

The ``expression`` here should be a subtype of :ref:`Type Any`. The semantics of
``awaitExpression`` depends on the type of the ``expression``:

- If the ``expression`` type is a subtype of :ref:`Concurrency Promise Class`,
  then ``await`` defines a suspension point.
- Otherwise, the ``await`` does not define a suspension point and the type and
  value of the ``awaitExpression`` match those of the ``expression``.

If ``awaitExpression`` defines a suspension point, then:

- its type is ``Awaited<type(expression)>``;
- the execution of the enclosing asynchronous function will be paused until the
  awaited ``Promise`` instance becomes *resolved* or *rejected*;
- if the awaited ``Promise`` instance becomes *resolved* then the value of
  ``awaitExpression`` is the value the ``Promise`` was resolved with;
- if the awaited ``Promise`` instance becomes *rejected* then ``awaitExpression`` throws
  the error the ``Promise`` instance was rejected with;

Under certain circumstances, the |C_CORO| that has been suspended on ``await``
can be moved to another |C_WORKER| upon resumption, i.e. rescheduled (see
:ref:`Scheduling rules`).

A :index:`compile-time error` occurs if ``await`` is used outside of an
asynchronous function, method or lambda body.

.. index::
   syntax
   await expression
   subtype
   expression
   resolution
   async function

|

.. _Concurrency Promise Class:

``Promise`` class
=================

.. meta:
    frontend_status: Done

The ``Promise`` class represents a value that is to be defined at some time in
future, thus allowing for referencing a result of an unfinished calculation or
an incomplete task. All kinds of |C_JOBS| in |LANG| use promises to communicate
their results to the client code.

A ``Promise`` instance can have the following states:

- *pending*: this state means that the resulting value is not yet known;
- *resolved*: this state means that the ``Promise`` has been *fulfilled* and its
  value has been defined;
- *rejected*: this state means that the associated calculation completed
  abnormally, so the ``Promise`` instance contains the error instance that
  describes the reason of abnormal completion;

The only way to get the value that was used to resolve or reject the ``Promise``
is to apply the :ref:`await Expression` to the ``Promise`` instance.

.. note::
    The semantics of ``Promise`` is similar to the semantics of ``Promise`` in
    |JS|/|TS| if it was returned by an asynchronous function on the **main**
    |C_WORKER| or created manually on the **main** |C_WORKER|.
    It is to be defined if such statements should reside in |LANG|
    specification.

.. index::
   promise object
   asynchronous API
   asynchronous operation
   API
   semantics
   proxy
   coroutine
   context
   async function
   qualification
   root coroutine

In general, the ``Promise`` instances are safe to be accessed concurrently.
The exceptions for this rule and the detailed API is described in the
:ref:`API details and restrictions` section.

|


