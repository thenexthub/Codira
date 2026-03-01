# Types

Types are divided into next kinds:

1. Semantic (abstract) types. They used only for values classification without taking into consideration any ohter information
   like storage size, bit width, etc.
2. Storage types
3. Physycal (concrete) types. They are parameterized types by abstract and storage types.

# Type structure

Types are formed by some `Sort`, an uniq identifier of the type family, and particular parameters.

`Sort` + `parameters` = `Type`.

Each parameter is a some type accompanied with a variance flag.

Variance of the parameter is classic: covariant, contrvariant, invariant.
It defines subtyping relation of types in parameters in subtyping relation of parameterized
types.

Some conventions about notation:

- Sorts are denoted by some `Literals`. Where `Literal` is a word composed of characters from set `[0-9a-zA-Z_]`.
  Examples: `Array`, `Function`, `ref`
- Types are denoted by sort literal and parameters in parenthesis. For instance: `Array(~i8())`.
- Type parameters are just types prepended by variance sign, `-` - contrvariant, `+` - variant, `~` - invariant.
  Examples: `+i8()`, `+Function(-i8(), +i16())`, etc.

# Subtyping relation

Subtyping relation is denoted by `<:`. Subtyping is a realtion on types which is used to determine if the particular
value of the particular type may be used in the place where other type is expected.

If two types, say `A()` and `B()` related as `A() <: B()`, i.e. `A()` is the subtype of `B()`, then in every
place, where a value of type `B()` is expected, value of type `A()` may be safely used.

By default, it is considered, that a type is in subtyping relation with itself, i.e.
`A() <: A()` for every type in the type universe.

Useful short notations for subtyping relation:

- `A() <: B() <: C()` - means `A() <: B()`, `A() <: C()` and `B() <: C()`

- `(A() | B()) <: C()` - means `A() <: C()` and `B() <: C()`

- `(A() | B()) <: (C() | D())` - means `A() <: C()`, `B() <: C()`, `A() <: D()` and `B() <: D()`

In short `|` means composition of types (syntactically) in set, and `<:` is distriuted over `|`.

## How to read notation of type parameters and to determine subtyping relation?

Suppose, we have types `T(+i8())` and `T(+i16())` and `i8() <: i16()`, how to relate types `T(...)`?

May be relation is `T(+i16()) <: T(+i8())`? Let's see, according to `+` (covariance), relation of types of parameters
should be the same as of `T(...)`, i.e. `i16() <: i8()`. And that is obviously wrong, because of the initial condition 
`i8() <: i16()`. I.e. we have a contradiction here.

Let's check `T(+i8()) <: T(+i16())`. So we have `i8() <: i16()` for the first parameters which is in line
with initial conditions. So, finally, subtyping relation is `T(+i8()) <: T(+i16())`.

# Type universe

All kinds of types live in the same type universe under the same subtyping relation.

It seems to me, that single univerce of parameterized types with properly defined subtyping relation is enough to
deal with all cases of types usage in VM.

## Initial and final types

Type system includes two special types:

- `Bot` - is a subtype of all types
- `Top` - is a supertype of all types

So for every type `Bot <: T <: Top`

# Classes of equivalence of types

Types which are in circular subtyping relation, like `A <: B <: C <: A`, are indistinguishable
in the type system. They form a single class of equivalence.

# Type hieararchy

  Here is basic type hierarchy of Panda VM.

## Abstract types

### Primitive types

#### Integral types

- Signed integers

- Unsigned ones

- Integral

#### Floating point types

#### Arrays

`Array(+T)` - where T is type of elements.

#### Objects/Records

TODO

## Storage types

TODO

## Concrete types

TODO
