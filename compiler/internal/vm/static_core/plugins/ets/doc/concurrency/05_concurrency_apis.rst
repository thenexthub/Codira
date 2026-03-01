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

.. _API details and restrictions:

API details and restrictions
****************************

.. meta:
    frontend_status: Done


.. _Worker domains:

Worker domains
==============

.. meta:
    frontend_status: Done

This section describes the concept of **worker domain** that is used by the ``launch
API``.

.. index::
   unhandled promise
   rejected promise
   unhandled rejection
   rejection handler
   call
   program completion

|



.. _Promise API:

Promise class API
=================

.. note::
   This section requires clarification and improvement.

Safe actions:

- Pass ``Promise`` from one coroutine to another, and avoid using it again in
  the original coroutine.
- Pass ``Promise`` from one coroutine to another, use it in both coroutines,
  and call ``then`` only in one coroutine.
- Pass ``Promise`` from one coroutine to another, use it in both coroutines,
  and call ``then`` in both coroutines. The user is to provide custom
  synchronization to guarantee that ``then`` is not called simultaneously
  for this ``Promise``.

The methods are used as follows:

-  ``then`` takes two arguments. The first argument is the callback used if the
   promise is fulfilled. The second argument is used if it is rejected, and
   returns ``Promise<U>``.

-  If ``then`` is called from the same parent coroutine several times, then the
   order of ``then`` is the same if called in |JS|/|TS|.
   The callback is called on the coroutine when ``then`` called, and if
   ``Promise`` is passed from one coroutine to another and called ``then`` in
   both, then they are called in different coroutines (possibly concurrently).
   The developer must consider a possible data race, and take appropriate care.

.. index::
   coroutine
   custom synchronization
   method
   argument
   callback
   concurrency
   data race

..
        Promise<U>::then<U, E = never>(onFulfilled: ((value: T) => U|PromiseLike<U> throws)|undefined, onRejected: ((error: Any) => E|PromiseLike<E> throws)|undefined): Promise<U|E>

.. code-block:: typescript

        Promise<U>::then<U, E = never>(onFulfilled: ((value: T) => U|PromiseLike<U> throws)|undefined, onRejected: ((error: Any) => E|PromiseLike<E> throws)|undefined): Promise<Awaited<U|E>>

-  ``catch`` takes one argument (the callback called after promise is rejected) and returns ``Promise<Awaited<U|T>>``

.. code-block-meta:

.. code-block:: typescript

        Promise<U>::catch<U = never>(onRejected?: (error: Any) => U|PromiseLike<U> throws): Promise<Awaited<T | U>>

-  ``finally`` takes one argument (the callback called after ``promise`` is
   either fulfilled or rejected) and returns ``Promise<Awaited<T>>``.


.. code-block:: typescript

        finally(onFinally?: () => void throws): Promise<Awaited<T>>

|

.. _Unhandled Rejected Promises:

Unhandled Rejected Promises
===========================

.. meta:
    frontend_status: Done

.. note::
   The semantics of unhandled rejections will be revisited later, once the
   design of |LANG| concurrency subsystem is complete.

.. index::
   unhandled promise
   rejected promise
   unhandled rejection
   rejection handler
   call
   program completion

|
