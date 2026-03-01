# ANI Mangling

## Value Types

The following encodings are used for primitive types in ANI:

```
PrimitiveEncoding ::=
    BooleanLetter
    | ByteLetter
    | CharLetter
    | ShortLetter
    | IntLetter
    | LongLetter
    | FloatLetter
    | DoubleLetter
    ;

BooleanLetter ::= 'z';
ByteLetter ::= 'b';
CharLetter ::= 'c';
ShortLetter ::= 's';
IntLetter ::= 'i';
LongLetter ::= 'l';
FloatLetter ::= 'f';
DoubleLetter ::= 'd';
```

From language point of view, all _value types_ are references. From runtime point of view, all _value types_ which are used as standalone non-generic types in source code are represented as primitive types in runtime. In all other cases (generic type constraints, unions, function parameters with default values or optional parameters), _value types_ become instances of their corresponding classes in runtime. In the following example, only the first argument `a0` will be compiled as primitive type `IntLetter ::= 'i'`, while all other primitives will be references and have reference encoding in ANI.

```ts
function foo<T extends int>(a0: int, a1: int | string, a2: T, a3?: int)
```

## Reference Types

### Nullish Types

Special single-letter encoding is used for `undefined` type:

```
UndefinedEncoding ::= 'U';
```

### Classes, Interfaces, Enums, Partial<T>

_Runtime names_ are used for fully qualified names of classes, interfaces, enums, namespaces, and modules. For example, `app.ns.Klass` _runtime name_ would encode class `Klass` from namespace `ns` of module `app`. For full explanation of runtime names and their formatting see runtime specification.

Group of ANI APIs `FindClass` / `FindEnum` / `FindNamespace` accepts _runtime names_ for finding entities in runtime. Example:

```c
ani_class cls;
env->FindClass("app.ns.Klass", &cls);
```

The following sections will denote _runtime names_ as `RuntimeName`:

```
ClassOrInterfaceEncoding ::=
    'C' '{' RuntimeName '}'
    ;

EnumEncoding ::=
    'E' '{' RuntimeName '}'
    ;

PartialEncoding ::=
    'P' '{' RuntimeName '}'
    ;
```

### Fixed Arrays

```
FixedArrayEncoding ::=
    'A' '{' FixedArrayElement '}'
    ;

FixedArrayElement ::= TypeEncoding;
```

Note that `FindClass` API additionally accepts `FixedArrayEncoding` names for searching classes which correspond to Fixed Arrays. Example:

```c
ani_class arrCls;
env->FindClass("A{C{app.ns.Klass}}", &arrCls);
```

### Function Types

* _Function types_ are represented with their runtime types - see [Function.ets](../../../stdlib/std/core/Function.ets)
* Count number of _required parameters_ of lambda, which is later denoted as `RequiredParamsCount`

```
FunctionalTypeEncoding ::=
    'C' '{' 'std.core.Function' RequiredParamsCount '}'
    ;

FunctionalTypeWithRestParamsEncoding ::=
    'C' '{' 'std.core.FunctionR' RequiredParamsCount '}'
    ;
```

### Union Types

In runtime, unions are represented as _normalized types_, as defined in `3.20.1 Union Types Normalization` language specification chapter. The same process of unions normalization must be applied to the union encodings which are used in ANI. Additionally, the following actions must be also applied:
* `undefined` does not participate in runtime representation and must be removed from list of constituent types. For example, type `string | number | undefined` on ANI level will be encoded in the same way as `string | number`, while `string | undefined` will be encoded just as `string`
* _Value types_ are promoted to their corresponding boxed primitive types
* Generic type constraints are substituted instead of generic types themselves
* Unions' constituent types must be ordered in alphabetical order according to their ANI encodings. Note that this order differs from panda files and runtime, where constituent types are ordered according to their _panda assembler names_. Implementation of ANI must check correct order of constituent types and reorder them accordingly to match runtime union types

```
UnionEncoding ::=
    'X' '{' UnionConstituentTypeEncoding (UnionConstituentTypeEncoding)+ '}'
    ;

UnionConstituentTypeEncoding ::=
    ClassOrInterfaceEncoding
    | EnumEncoding
    | PartialEncoding
    | FixedArrayEncoding
    | FunctionalTypeEncoding
    | FunctionalTypeWithRestParamsEncoding
    ;
```

## Functions and Methods Signatures Mangling

```
TypeEncoding ::=
    PrimitiveEncoding
    | UndefinedEncoding
    | ClassOrInterfaceEncoding
    | EnumEncoding
    | PartialEncoding
    | FixedArrayEncoding
    | FunctionalTypeEncoding
    | FunctionalTypeWithRestParamsEncoding
    | UnionEncoding
    ;

SignatureEncoding ::=
    (TypeEncoding)* ':' (TypeEncoding)?
    ;
```

## Examples

### Function with void as return type

```
// app.ets
function foo(a0: Integral, a1: double, arg2: float, arg3: SomeEnum): void

// ANI mangled type
"C{std.core.Integral}dfE{app.ns.SomeEnum}:"
```

### Function types in arguments

```
// app.ets
function foo(a0: Required<Iface>, a1: null, a2: undefined, a3: (...args: FixedArray<double>) => double): Double

// ANI mangled type
"C{app.ns.Iface}C{std.core.Null}UC{std.core.FunctionR0}:C{std.core.Double}"
```

### Union types representations in ANI

```
// app.ets
type T1 = (C1 | I1) | (C2 | I2)
type T2 = number | string | undefined
function foo<T extends I1 | I2>(a0: T | FixedArray<T> | T[]): number | string | null

// ANI mangled types
const char *T1 = "X{C{app.C1}C{app.C2}C{app.I1}C{app.I2}}";
const char *T2 = "X{C{std.core.Double}C{std.core.String}}";
const char *T3 = "X{A{X{C{app.I1}C{app.I2}}}C{std.core.Array}C{app.I1}C{app.I2}}:X{C{std.core.Double}C{std.core.Null}C{std.core.String}}";
```

## General Notes

* Some types are erased according to `15.11 Type Erasure` language specification chapter
* In runtime `null` is represented as the single instance of `std.core.Null` class, hence `null` type in signatures is represented as `C{std.core.Null}`
* When being used as standalone type annotation, `undefined` is represented as root of type system (_Any_ type)
* Tuples are represented as generic classes depending on number of their corresponding types, see [Tuple.ets](../../../stdlib/std/core/Tuple.ets)
