..
    Copyright (c) 2021-2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Annotations
###########

An *annotation* is a special language element that changes the semantics of
the declarations it is applied to by adding metadata.

The example below illustrates the declaring and using of an annotation:

.. code-block:: typescript
   :linenos:

    // Annotation declaration:
    @interface ClassAuthor {
        authorName: string
    }

    // Annotation use:
    @ClassAuthor({authorName: "Bob"})
    class MyClass {/*body*/}

The annotation *ClassAuthor* in the example above adds meta information to
the class declaration.

An annotation must be placed immediately before the declaration it is applied to.
The annotation usage can include arguments as in the example above.

For an annotation to be used, its name must be prefixed with the symbol ``@``
(e.g., ``@MyAnno``).
Spaces and line separators between the symbol ``@`` and the name are not allowed:

.. code-block::
   :linenos:

    ClassAuthor({authorName: "Bob"}) // compile-time error, no '@'
    @ ClassAuthor({authorName: "Bob"}) // compile-time error, space is forbidden

A compile-time error occurs if the annotation
name is not accessible at the place of usage. An annotation declaration can be
exported and used in other compilation units.

Multiple annotations can be applied to one declaration:

.. code-block:: typescript
   :linenos:

    @MyAnno()
    @ClassAuthor({authorName: "John Smith"})
    class MyClass {/*body*/}


As annotations is not |TS| feature in can be used only in ``.ets/.d.ets`` files.

|

.. _Declaration of User-Defined Annotation:

Declaration of User-Defined Annotation
======================================

The definition of a *user-defined annotation* is similar to that of an
interface where the keyword ``interface`` is prefixed with the symbol ``@``:

.. code-block:: abnf

    userDefinedAnnotationDeclaration:
        '@interface' Identifier '{' annotationField* '}'
        ;
    annotationField:
        Identifier ':' TypeNode initializer?
        ;
    initializer:
        '=' constantExpression
        ;
    constantExpression:
        Expression
        ;

As any other declarated entity, an annotation can be exported, using ``export`` keyword.

A ``TypeNode`` in the annotation field is restricted (see :ref:`Types of Annotation Fields`).

The default value of an *annotation field* can be specified
using *initializer* as *constant expression*. A compile-time occurs in the value
of this expression cannot be evaluated in compile-time.

An *user-defined annotation* must be defined at top-level,
otherwise a compile-time error occurs.

An *user-defined annotation* cannot be extended (inheritance is not supported).

The name of an *user-defined annotation* cannot coincide with a name of other entity.

.. code-block:: typescript
   :linenos:

    @interface Position {/*properties*/}

    class Position {/*body*/} // compile-time error: duplicate identifier

An annotation declaration does not define a type, so a type alias
cannot be applied to the annotation and it cannot be used as an interface:

.. code-block:: typescript
   :linenos:

    @interface Position {}
    type Pos = Position // compile-time error

    class A implements Position {} // compile-time error

|

.. _Types of Annotation Fields:

Types of Annotation Fields
==========================

The choice of types for annotation fields is limited to the types listed below:

- Type ``number``;
- Type ``boolean``;
- Type ``string``;
- Enumeration types (``const enum`` only);
- Array of above types, e.g., ``string[]``.

A compile-time error occurs if any other type is used as type of an *annotation
field*.

.. _Using of User-Defined Annotation:

Using of User-Defined Annotation
================================

The following syntax is used to apply an
annotation to a declaration,
and to define the values of annotation properties:

.. code-block:: abnf

    userDefinedAnnotationUsage:
        '@' qualifiedName userDefinedAnnotationParamList?
        ;
    qualifiedName:
        Identifier ('.' Identifier)*
        ;
    userDefinedAnnotationParamList:
        '(' ObjectLiteralExpression? ')'
        ;

An annotation declaration is presented in the example below:

.. code-block:: typescript
   :linenos:

    @interface ClassPreamble {
        authorName: string
        revision: number = 1
    }
    @interface MyAnno{}


All values in an *object literal expression* must be constant expressions,
otherwise a compile-time error occurs.

Annotation usage is presented in the example below:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    class C1 {/*body*/}

    @ClassPreamble({authorName: "Bob"}) // default value for revision = 1
    class C2 {/*body*/}

    @MyAnno()
    class C3 {/*body*/}

The current version of the language allows to use annotations only
for non-abstract class declarations
and method declarations in non-abstract classes.
Otherwise, a compile-time error occurs:

.. code-block:: typescript
   :linenos:

    @MyAnno()
    function foo() {/*body*/} // compile-time error

    @MyAnno()
    abstract class A {} // compile-time error

Repeatable annotations
(applying the same annotation more than once to the entity)
are not supported:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John"})
    @ClassPreamble({authorName: "Bob"}) // compile-time error
    class C {/*body*/}

The order of properties does not matter in an annotation usage:

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    // the same as:
    @ClassPreamble({revision: 2, authorName: "John"})


When using an annotation, all fields without default values must be listed.
Otherwise, a compile-time error occurs:

.. code-block:: typescript
   :linenos:

    @ClassPreamble() // compile-time error, authorName is not defined
    class C1 {/*body*/}

If a field of an array type is defined for an annotation, then the array
literal syntax is used to set its value:

.. code-block:: typescript
   :linenos:

    @interface ClassPreamble {
        authorName: string
        revision: number = 1
        reviewers: string[]
    }

    @ClassPreamble(
        {authorName: "Alice",
        reviewers: ["Bob", "Clara"]}
    )
    class C3 {/*body*/}

The parentheses after the annotation name can be omitted,
if there is no need to set annotation properties:

.. code-block:: typescript
   :linenos:

    @MyAnno
    class C4 {/*body*/}

.. _Exporting and Importing Annotations:

Exporting and Importing Annotations
===================================

An annotation can be exported and imported,
only few forms of export and import directives are supported.

To export an annotation its declaration must be marked with ``export`` keyword:

.. code-block:: typescript
   :linenos:

    // a.ets
    export @interface MyAnno {}

An annotation can be imported as part of the imported module. In this case
it is accessed by qualified name:

.. code-block:: typescript
   :linenos:

    // b.ets
    import * as ns from "./a"

    @ns.MyAnno
    class C {/*body*/}

Unqualified import is also allowed:

.. code-block:: typescript
   :linenos:

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

As an annotation is not a type, it is forbidden to export or import
using ``export type`` or ``import type`` notations:

.. code-block:: typescript
   :linenos:

    import type { MyAnno } from "./a" // compile-time error


The following cases are forbidden for annotations:

- Export default,

- Import default,

- Rename in export,

- Rename in import.

.. code-block:: typescript
   :linenos:

    import {MyAnno as Anno} from "./a" // compile-time error


.. _Annotations in .d.ets Files:

Annotations in .d.ets Files
===========================

Ambient annotations can be declared in .d.ets file.

.. code-block:: abnf

    ambientAnnotationDeclaration:
        'declare' userDefinedAnnotationDeclaration
        ;

Such declaration does not introduce a new annotation, but provides type information
for using annotation that must be defined somewhere else.
A runtime error occurs, if there no annotation that corresponds to the ambient annotation,
used in the program.

The ambient declaration and annotation that implements it must be exactly the same,
including fields initialization:

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface NameAnno{name: string = ""}

    // a.ets
    export @interface NameAnno{name: string = ""} // ok

The following example shows incorrect code,
as ambient declaration is not the same as annotation declaration:

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface VersionAnno{version: number} // initialization is missing

    // a.ets
    export @interface VersionAnno{version: number = 1}


An ambient declaration can be imported and used exactly the same way as a regular annotation.

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface MyAnno {}

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

If an annotation is applied to an ambient declaration in .d.ets file
(see the example below),
it is not automatically applied to the declaration that implements
this ambient declaration.
It is up to the developer to apply it to the implementation declaration.

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface MyAnno {}

    @MyAnno
    declare class C {}
