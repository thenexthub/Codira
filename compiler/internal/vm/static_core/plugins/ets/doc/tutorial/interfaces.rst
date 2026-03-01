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

Interfaces
==========

|

An interface declaration introduces a new type. Interfaces are a common way
to define contracts between various parts of code.

Interfaces are used to write polymorphic code applicable to any class
instances that implement a particular interface.

An interface usually contains properties and method headers:

.. code-block:: typescript

    interface Style {
        color: string // property
    }
    interface Area {
        calculateArea(): number // method header
        someMethod() : void;    // method header
    }

The examples below illustrate a class that implements an interface:

.. code-block:: typescript

    // Interface:
    interface Area {
        calculateArea(): number // method header
        someMethod() : void;    // method header
    }

    // Implementation:
    class Rectangle implements Area {
        private width: number = 0
        private height: number = 0
        someMethod() : void {
            console.log("someMethod called")
        }
        calculateArea(): number {
            this.someMethod() // calls another method and returns result 
            return this.width * this.height
        }
    }

|

Interface Properties
--------------------

An interface property can have the form of a field, a getter, a setter, or both.

A property field is just a shortcut notation of a getter/setter pair. The
following notations are equal:

.. code-block:: typescript

    interface Style {
        color: string
    }

    interface Style {
        get color(): string
        set color(x: string)
    }

A class that implements an interface can also use a short or a long notation:

.. code-block:: typescript

    interface Style {
        color: string
    }

    class StyledRectangle implements Style {
        color: string = ""
    }

The short notation implicitly defines a private field, a getter, and a setter:

.. code-block:: typescript

    interface Style {
        color: string
    }

    class StyledRectangle implements Style {
        private _color: string = ""
        get color(): string { return this._color }
        set color(x: string) { this._color = x }
    }

|

Interface Inheritance
---------------------

An interface can extend other interfaces as in the example below:

.. code-block:: typescript

    interface Style {
        color: string
    }

    interface ExtendedStyle extends Style {
        width: number
    }

An extended interface contains all properties and methods of the
interface it extends. It also can add its own properties and methods.


|
