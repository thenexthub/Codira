<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: August 6, 2026

Functionality: Instructions for AI coding agents — how to USE the Codira
programming language to write projects (NOT how to build the compiler).
 -->

# Using Codira — Agent Instructions

This document teaches an AI coding agent how to **use** the Codira programming
language to write and run programs. It is *not* about building the compiler;
for compiler-internals work see `spec/LANGUAGE_SPEC.md` §12-§15 and the
`crates/**` sources.

Codira is an ahead-of-time (AOT) compiled, statically typed, hot-reloadable
systems language with an LLVM backend. It compiles to natively executable
machine code, cross-compiles, and ships as reusable libraries.

> **Ground truth** — Language details change fast. Always prefer
> `spec/LANGUAGE_SPEC.md` over this guide when they disagree. This guide is a
> friendly summary of the current (v26.8.1) surface.

---

## 1. What Codira is not

- No `null` / undefined reference value. Absence is spelled `Type?` and handled
  explicitly.
- No template angle brackets — generics use **square brackets** `[T]`.
- `;` is *optional* at the end of a line-terminated statement. The parser is
  not indentation-sensitive; it just treats `;` as always-legal.
- No borrow checker / ownership model. `struct` = value (copied on pass),
  `class` = GC-managed reference (shared mutability).
- No standalone executable by default — you produce a Codira "library"
  (`*.codiralib`) and run it via the `codira start` CLI.

---

## 2. Project layout

A Codira project is a directory holding the source plus a manifest. Example:

```
my_program/
├── src/
│   └── mod.code      # module source (must end in `.code`)
└── codira.toml       # manifest
```

- Source files always end in **`.code`**. Multi-word file names use
  underscores. One file = one module; directories nest modules.
- The manifest (`codira.toml`) declares package metadata:
  ```toml
  [package]
  name="my_program"
  authors=[]
  version="0.1.0"
  ```
- Compiling yields `target/mod.codiralib`; `codira start` invokes a chosen
  entry-point function.

### Scaffold a project

```bash
codira new my_program      # creates a new directory + project
codira init .             # initializes the current directory (or [path])
```

The generated `src/mod.code` starts as:

```
public func main() -> f64 {
    3.14159
}
```

---

## 3. Writing Codira source

### Lexical rules

Full reserved keyword list (the modern, non-Rust surface):

```
as break class comptime effect else enum extend extern false for
func handle if import in init internal let loop macro match never
nil override perform private public resume return root self static
struct super true trait uses var where while
```

The path separator is **`.`** (like Python/Swift), not `::`.

### Functions

```codira
func add(a: i32, b: i32) -> i32 {
    a + b
}
```

Visibility: `private` (default — write nothing), `internal` (same package),
`public` (exported):

```codira
public func distance_squared(a: Point, b: Point) -> f64 {
    let dx = a.x - b.x
    let dy = a.y - b.y
    dx * dx + dy * dy
}
```

Implicit return — the last expression is the return value; `return x` also works.

### Bindings

```codira
let pi = 3.14159     // immutable
var counter = 0       // mutable
counter += 1
```

`var` replaces `let mut`; there is no standalone `mut` keyword.

### Records (value) vs classes (reference)

```codira
// Value type: copied on assignment/pass, value semantics.
struct Point {
    x: f64
    y: f64
}

// Reference type: heap-allocated, garbage collected, shared mutability.
class Actor {
    var position: Point
    var health: f64
}
```

### Methods via `extend` (was `impl`)

The type body holds **fields only**; methods and `init` live in an `extend` block:

```codira
extend Actor {
    init(position: Point) {
        self.position = position
    }
    func take_damage(var self, amount: f64) {
        self.health -= amount
    }
}
```

### Enums & pattern matching

```codira
enum Shape {
    Circle(radius: f64)
    Rectangle(width: f64, height: f64)
}

func area(shape: Shape) -> f64 {
    match shape {
        Shape.Circle(radius) -> 3.14159 * radius * radius
        Shape.Rectangle(width, height) -> width * height
    }
}
```

`match` must be exhaustive (or include a `_` wildcard arm).

### Generics

```codira
struct Box[T] {
    value: T
}

func first[T](items: [T]) -> T? {
    if items.is_empty() { nil } else { items[0] }
}
```

### Optional types

```codira
func find(items: [Value], target: i32) -> i32? {
    for item in items {
        if item == target { return item }
    }
    nil
}

if let value = find(xs, 7) {
    print(value)
}
```

### Modules & imports

```codira
import math
import collections.list
import geometry.{ Point, Vector as Vec2 }

root.tools.clamp(x, 0, 1)     // `root` = package root
super.helper()                 // parent module
```

---

## 4. Building and running

```bash
codira build                                     # compile -> target/mod.codiralib
codira start target/mod.codiralib main          # run entry function `main`
```

`codira start` prints the entry point's return value when its type is `bool`,
`f64`, `i64`, or `()`.

Useful `build` options: `-O 0..3` (default `2`), `--emit-ir`, `--watch`,
`--target <triple>` (cross-compile), `--manifest-path <path>`.

Hot reload: Codira is designed so a running application can swap function
bodies without a manual stop/restart cycle.

---

## 5. End-to-end status today

Verified through the full pipeline (parser → HIR → LLVM → link → run):

- `struct`, `class`, `func`, `let`/`var`
- field access, arithmetic, comparisons
- `if`/`else`, `for`/`while` loops, function calls
- imports and visibility (`public`/`internal`/`private`)

Parsed to a real syntax tree but **not yet lowered** to runnable code
(HIR `Missing` placeholder): `enum`, `trait`, `effect`/`handle`, `match`,
`comptime`, `supervisor`, multiple dispatch, and `Type.method()` associated
call syntax.

---

## 6. Reference material (in this repository)

- `spec/LANGUAGE_SPEC.md` — the canonical, current language spec.
- `spec/HERACLES_Codira_Examples.code` — self-healing framework examples
  (`@heal(...)`, refinement types, `supervisor`).
- `book/src/**` — chapter-based tutorial (getting started, hot-reload, arrays,
  structs).
- `examples/**` — runnable Codira (and Rust, C++, C) example projects.
- `std/` — the Codira standard library source (`*.code`).

Always cross-check written code against `spec/LANGUAGE_SPEC.md` and the parser
tests in `crates/codira_syntax/src/tests.rs`.

---

## 7. Agent coding rules

1. **Use `.code` files.** Start with a single `src/mod.code`; nest modules via
   subdirectories when a package grows.
2. **Prefer explicit types** at module and interop boundaries (inference works
   but less clarity).
3. **No `null`** — design around `Type?` optionals and explicit error returns.
4. **No `::`, no `<>`** — use `.` paths and `[T]` generics on args.
5. **Semicolons** are optional; keep lines mostly semicolon-free unless two
   statements share one line.
6. **Verify** with `codira build` and `codira start` rather than guessing.
7. When this guide and the spec disagree, **obey `spec/LANGUAGE_SPEC.md`**.

Happy coding in Codira!