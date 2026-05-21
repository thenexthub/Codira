---
name: zero-language
description: Compact Zero syntax and semantics guide for agents.
---

# Zero Language

Use this when writing or reviewing `.0` source, especially if the model has no prior Zero training. Zero favors explicit capabilities, explicit errors, and small syntax.

## Minimal Program

```zero
pub fun main(world: World) -> Void raises {
    check world.out.write("hello from zero\n")
}
```

`pub fun` exports a function. `World` carries runtime capabilities. `raises` marks a fallible function. `check` calls a fallible operation and propagates failure.

## Declarations

Top-level declarations include:

- `use std.mem` or `use helpers`
- `const answer: i32 = 42`
- `shape Point { x: i32, y: i32 }`
- `enum Mode { off, on }`
- `choice Result { ok: i32, err: String }`
- `fun answer() -> i32 { return 42 }`
- `pub fun main(...) -> Void ...`
- `test "name" { expect(true) }`

Use `.0` for source files.

## Values And Control Flow

```zero
fun answer() -> i32 {
    return 40 + 2
}

pub fun main(world: World) -> Void raises {
    let value = answer()
    if value == 42 {
        check world.out.write("math works\n")
    } else {
        check world.out.write("math broke\n")
    }
}
```

Use `let` by default and `let mut` only when a binding changes. Conditions are `Bool`; do not rely on truthy integers or strings.

## Types

Common primitive types:

```text
Bool Void String char
i8 i16 i32 i64 isize
u8 u16 u32 u64 usize
f32 f64
```

Integer literals are checked against context. Use suffixes such as `_u8` or `_usize` when needed. Use `as` for intentional integer casts.

## Shapes, Enums, And Choices

```zero
shape Point {
    x: i32,
    y: i32,
}

enum Mode {
    fast,
    small,
}

choice Result {
    ok: i32,
    err: String,
}
```

Construct a shape with field names:

```zero
let point = Point { x: 1, y: 2 }
```

Choice payload cases use the choice name:

```zero
let result = Result.ok(42)
```

Matches must be exhaustive unless they use the fallback arm `_`:

```zero
match result {
    .ok => value {
        expect(value == 42)
    }
    .err => message {
        expect(true)
    }
}
```

## Errors

```zero
fun parse(ok: Bool) -> i32 raises { InvalidInput } {
    if ok {
        return 42
    }
    raise InvalidInput
}

pub fun main() -> Void raises { InvalidInput } {
    let value = check parse(true)
    expect(value == 42)
}
```

`raises { ... }` restricts the error set. A plain `raises` marker is open. Calling a fallible function requires `check`.

## Borrowing And Memory Views

- `ref<T>` is a read-only borrow, passed with `&value`.
- `mutref<T>` is a mutable borrow, passed with `&mut value`.
- `[N]T` is a fixed array.
- `Span<T>` is a read-only contiguous view.
- `MutSpan<T>` is a writable contiguous view.
- `Maybe<T>` represents absence; inspect `.has` and `.value`.
- `owned<T>` marks explicit resource ownership.

```zero
fun bump(point: mutref<Point>) -> Void {
    point.x = point.x + 1
}
```

## Generics

```zero
shape Box<T> {
    value: T,
}

fun id<T>(value: T) -> T {
    return value
}
```

Static generic values are declared with `static`:

```zero
shape FixedVec<T, static N: usize> {
    len: usize,
    items: [N]T,
}
```

If unsure, run `zero check --json <file>` and use the diagnostic span instead of inventing syntax.
