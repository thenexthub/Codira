<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: August 13, 2026

Functionality: Instructions for AI coding agents — how to migrate/port Rust
source code to Codira, including an honest account of which parts of the
target language are actually execution-verified versus parse-only today.
-->

# Rust → Codira Migration Guide — Agent Instructions

This document is for an agent asked to **port or translate Rust code into
Codira**. Read `USING_CODIRA.md` first for the baseline language tutorial;
this document is a delta focused specifically on *"here's the Rust idiom,
here's the Codira equivalent, here's whether it actually runs today."*

> **Ground truth and verification method.** Every claim below was checked on
> 2026-08-13 against `spec/LANGUAGE_SPEC.md`, the exhaustive
> "Implementation status" §12 of that spec, `crates/codira_syntax/**`,
> `crates/codira_project/src/manifest/toml.rs` (the `codira.toml` schema),
> `crates/codira/src/lib.rs` (the actual CLI command list), and `std/**`
> (read directly, not assumed). Where the language *design* (what
> `LANGUAGE_SPEC.md` describes) and the *current compiler* (what actually
> parses/lowers/runs) diverge, both are stated explicitly — Rust engineers
> reflexively reach for pattern matching, `Result`, traits, and closures,
> and **most of those do not execute correctly in Codira yet**, even though
> they're valid syntax that appears to parse.

---

## 0. The two-tier reality check (read this first)

Codira's syntax layer (parser) is far ahead of its semantic layer (HIR
lowering/codegen). `spec/LANGUAGE_SPEC.md` §12 draws an explicit line
between:

**Tier 1 — verified end-to-end (parser → HIR → LLVM → link → run):**
`struct`, `class`, `func`, `let`/`var`, field access, arithmetic,
comparisons, `if`/`else`, `for`/`while` loops, function calls, `import`s and
visibility (`public`/`internal`/`private`), and `extern`-function host
interop (see `EMBEDDING_CODIRA.md`).

**Tier 2 — parses into a correct syntax tree, but is not yet lowered to
runnable code** (an `enum` type won't resolve in a signature; `match`
expressions lower to a `Missing` HIR placeholder so surrounding code still
type-checks, but the match itself doesn't execute correctly):
`enum`, `trait`, `match`, `effect`/`uses`/`perform`/`handle`/`resume`,
`comptime` (currently just runs its body at normal runtime — "transparently
lowered to their inner block in HIR for now", i.e. it does **not** yet
guarantee compile-time evaluation despite the name), generics (`[T]`
verified only at the parser level — the full-stack capstone test in spec
§12 that actually built→linked→ran never exercised a generic function),
multiple-dispatch/function-overloading (**actively rejected** by the
compiler today — defining two functions with the same name is a compile
error, "a value named `combine` has already been defined in this module"),
`Type.method()` associated/static-function call syntax (no working call
site exists — `Point.new(...)` does not resolve), `Type?` optionals and `!`
force-unwrap (both parse; `Type?` lowers *transparently to its base type*
with **no compile-time nil-safety enforced**, and `!` lowers to the same
`Missing` placeholder as `match`), refinement types, and `supervisor`.

**When porting Rust code, default every design decision toward Tier 1.**
Where Rust idiomatically needs a Tier-2 feature (pattern matching, `Result`,
trait objects, generics), this guide gives you the closest working Tier-1
substitute plus the "correct" Tier-2 code to leave as a comment/TODO for
when that feature lands — see §9.

Corroborating evidence this isn't theoretical: `std/testing/testing.code`
(this repo's own standard library) declares two functions both named
`assert_true` with different parameter lists — exactly the overload pattern
the compiler currently rejects. Treat **all** of `std/**` as design
reference, not working code, until you've personally verified a given
module compiles (see `EMBEDDING_CODIRA.md` §0 for the parallel warning about
`examples/**`).

---

## 1. Keyword and punctuation mapping

| Rust | Codira | Notes |
|---|---|---|
| `fn` | `func` | |
| `pub` | `public` | Codira adds `internal` (package-wide) between `private` (default) and `public`; there is no `pub(crate)`/`pub(in path)` fine-grained visibility |
| `let` | `let` | immutable, same meaning |
| `let mut` | `var` | `var` fully replaces `let mut`; no standalone `mut` keyword anywhere (params/fields/patterns all use `var`) |
| `impl Type { }` | `extend Type { }` | see §4 — no relation to Rust's trait-impl; `struct`/`class` bodies hold **fields only**, all methods/`init` live in `extend` |
| `trait Name { }` | `trait Name { }` | same spelling, Tier 2 (parses, not resolved) |
| `use a::b;` | `import a.b` | `;` optional |
| `mod foo;` / file-based modules | file-based modules | same concept: one file = one module, directories nest modules |
| `::` path separator | `.` | one operator for modules, types, and values (Python/Swift-style) |
| `crate::foo` / `self::foo` (from crate root) | `root.foo` | `root` = package root |
| `<T>` generics, turbofish `foo::<T>()` | `[T]` generics, **no turbofish** | type args only in type position / static member access; ordinary calls always infer |
| `#[derive(...)]`, `#[attr]` | `@derive(...)`, `@name(...)` | decorator-style, above the item |
| `macro_rules!` / proc-macros | `macro name(...) { }` | operates on the item's syntax tree at `comptime`, no textual token-pasting |
| `&T`, `&mut T`, lifetimes `'a` | **none** | no borrow checker at all — see §3 |
| `Option<T>` | `T?` (sugar) + `std/collections/optional.code` (Tier 2) | not compiler-enforced yet — see §8 |
| `Result<T, E>`, `?` operator | no language builtin; `std/builtin/error.code`'s `Error` struct convention | no `?`-propagation operator exists in the grammar — see §8 |
| `match` | `match` | same shape, Tier 2 |
| `enum` | `enum` | same shape (labeled-tuple variants), Tier 2 |
| closures `\|x\| x + 1`, `Fn`/`FnMut`/`FnOnce` | **no equivalent found** | see §7 |
| `dyn Trait`, trait objects | `trait` conformance + `class` inheritance | dispatch mechanism designed (§5) but not lowered yet |
| `async`/`await`, threads, `Send`/`Sync` | **no evidence found in the grammar or parser** | do not port concurrent Rust code expecting a Codira equivalent to exist |
| `Drop` trait / destructors | **no evidence found** | no RAII-on-scope-exit mechanism documented |
| `Rc<T>`/`Arc<T>`/`Box<T>` | `class` (GC reference type) | Codira's GC already gives you shared, aliasable ownership — you generally don't need an explicit smart pointer |
| `RefCell`/`Mutex`-style interior mutability | `var` field on a `class` | mutation through any alias is already legal for `class` instances (§3) |

---

## 2. Project/build system

| Rust (`Cargo.toml` / `cargo`) | Codira (`codira.toml` / `codira`) |
|---|---|
| `[package] name/version/authors` | `[package] name/version/authors` — **identical shape**, but this is *all* `codira.toml` currently supports (`crates/codira_project/src/manifest/toml.rs`: `TomlProject { name, version, authors }`, nothing else) |
| `[dependencies]`, crates.io | **no equivalent exists yet.** There is no dependency section in the manifest schema and no package registry. A Codira package today is a single, self-contained module tree — plan ports as one package, not a multi-crate workspace, until dependency support exists |
| `cargo new`/`cargo init` | `codira new <path>` / `codira init [path]` |
| `cargo build` | `codira build` (`-O 0..3`, `--emit-ir`, `--watch`, `--target <triple>`, `--manifest-path <path>`, `--color`) |
| `cargo run` | **no `codira run`.** Build then `codira start <lib> [entry]`, which only supports entry points returning `bool`/`f64`/`i64`/`()` |
| `cargo test` | **no `codira test` subcommand.** The CLI's full command list (`crates/codira/src/lib.rs`) is exactly `language-server`, `build`, `new`, `init`, `start` — nothing else. A `codira_test` crate exists in this workspace but is not wired to a user-facing CLI command; `std/testing/testing.code` provides `assert_*` helpers you can call manually from `main`, and `@test` is listed in the spec as a planned built-in attribute, but there is no `codira test` runner today |
| `src/main.rs` binary | Codira produces **libraries only** (`target/<name>.codiralib`), never a standalone OS executable — see `EMBEDDING_CODIRA.md` for how something else (a Rust/C/C++ host, or `codira start` itself) drives it |
| `src/lib.rs` + `mod` tree | `src/mod.code` + nested directories — same one-file-one-module philosophy Rust uses, just a different file extension (`.code`) and root file name |
| `rustc --edition`, editions | no equivalent found — treat the language as a single, currently-evolving surface (the "Next Generation Syntax" redesign in `spec/LANGUAGE_SPEC.md` **is** the only surface; there is no edition flag to opt into old vs. new syntax) |

---

## 3. Ownership, mutability, and aliasing — the biggest mental-model shift

This is the change that will break the most Rust intuition. Codira
**deliberately has no borrow checker, no lifetimes, and no `&`/`&mut`
anywhere in the grammar** (`spec/LANGUAGE_SPEC.md` §8, titled exactly for
this reason). Instead:

- `struct` = value type. Copied on assignment/pass, exactly like a Rust
  type that is `Copy` (or that you'd `.clone()` everywhere) — no aliasing
  is possible by construction, so there's nothing to borrow-check.
- `class` = GC-managed reference type. Any number of `let`/`var` bindings
  can alias the same instance; mutation through one binding is visible
  through all of them — the same mental model as `Rc<RefCell<T>>` in Rust,
  except it's the *default* behavior of `class`, not something you opt into
  with wrapper types.
- There is no move semantics to reason about for `struct`s (they copy) and
  no borrow conflicts to reason about for `class`es (mutation is always
  shared/GC-tracked). This is a **deliberate design tradeoff against**
  Rust's model, explicitly because Codira's hot-reload feature depends on
  being able to swap a live `class`'s method implementations while keeping
  its heap identity and fields intact — something a genuinely
  borrow-checked, static-stack-layout region cannot support.
- `consuming`/`borrowing` parameter-position keywords exist in the grammar
  (Mojo/Swift-inspired scaffolding, `spec/LANGUAGE_SPEC.md` §14) and a `~Trait`
  inheritance-list opt-out (e.g. `struct GPUBuffer: ~Copyable {}`) for
  marking a type as a non-duplicable resource — but **neither is enforced**.
  A `consuming` parameter doesn't actually invalidate the caller's binding,
  and there is no `Copyable` marker trait yet to opt out of. Don't rely on
  either for actual move/ownership safety; they currently only communicate
  intent.

**Porting rule:** a Rust `struct` with no `Rc`/`RefCell`/interior mutability
→ Codira `struct`. A Rust type wrapped in `Rc<RefCell<T>>`, `Arc<Mutex<T>>`,
or anything relying on shared mutable state → Codira `class`. If a Rust
function takes `&mut self` to mutate through a shared reference, the direct
translation is a `class` method (`func f(var self, ...)`); if it takes
`&self`, translate to an ordinary `class` or `struct` method with a
non-`var` `self`.

---

## 4. Structs, methods, and `init`

Rust:

```rust
struct Point { x: f64, y: f64 }

impl Point {
    fn new(x: f64, y: f64) -> Self {
        Point { x, y }
    }

    fn distance_squared(&self, other: &Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        dx * dx + dy * dy
    }
}
```

Codira (Tier 1 — verified working, modulo §0's note that `Point.new(...)`
associated-call syntax doesn't resolve — see the caption below):

```codira
struct Point {
    x: f64
    y: f64
}

extend Point {
    init(x: f64, y: f64) {
        self.x = x
        self.y = y
    }

    func distance_squared(self, other: Point) -> f64 {
        let dx = self.x - other.x
        let dy = self.y - other.y
        dx * dx + dy * dy
    }
}
```

Key differences from Rust's `impl` block:

- `struct`/`class` bodies hold **fields only** (no trailing commas needed,
  newline-separated). All behavior — `init`, instance methods, `override`s
  — goes in a separate `extend Type { }` block. You cannot mix fields and
  methods in one block the way Rust's `struct` + `impl` conceptually can
  feel merged.
- There is no `Self` type alias in the examples the spec shows — spell the
  type name out (`Point`, not `Self`).
- `init` replaces both Rust's conventional `fn new(...) -> Self` **and**
  struct-literal-with-explicit-fields — but unlike Rust, you cannot call it
  as `Point.new(...)` or `Point.init(...)`; **there is currently no working
  syntax to invoke a static/associated function at all** (§0, Tier 2). Two
  practical workarounds while porting:
  1. Prefer a plain top-level function (`func new_point(x: f64, y: f64) ->
     Point { Point { x: x, y: y } }`) over an `init`-only constructor when
     you need to actually construct instances from other code today.
  2. For a `class`, `init` **is** reachable, just not through `Type.method()`
     call syntax — the runtime-facing capstone example in spec §12
     constructs a plain `struct` via a struct literal (`Point { x: 3.0, y:
     4.0 }`), not via `init`; treat direct struct-literal construction as
     the Tier-1-verified path, and treat `extend`-block `init`/methods as
     verified for **instance methods on an already-constructed value**
     (`p.distance_squared(other)`), not for construction itself.
- Mutability of `self` in a method mirrors Rust's `&self` vs `&mut self`
  split, but spelled as a keyword on the parameter itself:
  `func take_damage(var self, amount: f64)` (mutating) vs.
  `func area(self) -> f64` (read-only) — no `&`/`&mut` prefix, ever.

---

## 5. Enums, pattern matching, and traits (Tier 2 — don't rely on these executing)

Rust:

```rust
enum Shape {
    Circle { radius: f64 },
    Rectangle { width: f64, height: f64 },
}

fn area(shape: &Shape) -> f64 {
    match shape {
        Shape::Circle { radius } => std::f64::consts::PI * radius * radius,
        Shape::Rectangle { width, height } => width * height,
    }
}
```

The syntactically-corresponding Codira (this **parses** but the `match`
lowers to a `Missing` HIR placeholder — do not port working logic this way
today):

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

**Practical porting strategy for a Rust `enum` + `match`:** until enums are
lowered, model the same domain with a Tier-1 substitute:

- A small, closed set of variants with no payload → distinct `bool`/`i32`
  "tag" comparisons with `if`/`else if`, or (if the set is genuinely fixed
  and small) one `struct`/`class` per variant plus separate handling
  functions.
- Variants that each carry different data (a Rust "sum type") → a `struct`
  with one field per possible payload plus a discriminant field you check
  by hand, or split into one function per case called directly by whatever
  produced the enum value in Rust. This is less elegant than Rust's `enum`,
  but it is what actually runs today.
- Keep the "correct" `enum`/`match` version around as a comment so the code
  is a trivial rewrite once enums are lowered — don't delete the intent.

**Traits.** `trait Name { }` and `extend Type: Trait { }` parse, including
default method bodies, `class` single inheritance, mandatory `override`,
and a designed multiple-dispatch/runtime-type-tag fallback for trait-typed
parameters (`spec/LANGUAGE_SPEC.md` §5) — but per §12, `trait` items are not
yet lowered into name-resolvable HIR items, so a `Type` named only by a
trait bound won't resolve in a signature, and the "most-specific-applicable
method" resolution rule for multiple dispatch is explicitly **not
implemented** (`method_resolution.rs`). **Function overloading itself is
actively rejected** — you cannot even declare two functions with the same
name and different parameter types today (confirmed both by the spec's
capstone-verification notes and by `std/testing/testing.code` shipping code
that would hit exactly this error). Port Rust trait-object-polymorphism to
distinctly-named Codira functions per concrete type for now (e.g.
`circle_area(c: Circle)` / `rectangle_area(r: Rectangle)` instead of a
shared `area()` overload set or a `dyn Shape`).

---

## 6. Optionals and error handling

Rust `Option<T>`/`Result<T, E>` + `?` have **no directly working Codira
equivalent** yet:

- `Type?` is valid Codira syntax (`if let value = find(xs, 7) { ... }`,
  `find(xs, 7)!` to force-unwrap) and is the *designed* replacement for
  `Option<T>` — but per §12, `Type?` **lowers transparently to its base
  type**, so nothing is statically enforced about nullability today, and
  `!` force-unwrap lowers to the same `Missing` HIR placeholder `match`
  does. Treat `Type?` as a documentation-only convention right now, not a
  safety guarantee.
- There is **no `Result<T, E>` in the language** and **no `?`-propagation
  operator** in the grammar at all. `std/builtin/error.code`'s `Error`
  struct (message + numeric code) plus a documented "functions that can
  fail return `Result[T, Error]`" *convention* is aspirational — it
  references a generic `Result[T, Error]` type that does not exist anywhere
  else in `std/` or the spec, and the `?`-propagation comment in that same
  file describes an operator that isn't in the grammar. Don't port Rust's
  `?`-heavy error-propagation style expecting it to compile.
- **Practical porting strategy:** for fallible Rust functions, return a
  sentinel value (a documented "impossible" value, e.g. `-1.0` for a
  distance function, or a separate `bool`/status `out`-parameter pattern),
  or return a `struct` with an explicit `ok: bool` field plus the payload,
  and check it explicitly at every call site with `if`. This is more
  verbose than idiomatic Rust but is Tier-1 (plain structs + `if`), so it
  actually runs.
- Rust `panic!`/`.unwrap()`/`.expect(...)` → the spec documents that
  bounds-checked array indexing and (once implemented) `!` force-unwrap
  should produce "a recoverable panic with source location" — but this is
  design intent from §7, not confirmed in the Tier-1 verified list. Don't
  assume a panic path is safely recoverable until you've checked it
  directly against a build of the compiler you're targeting.

---

## 7. Closures, iterators, and function values

**No evidence of closures, `Fn`/`FnMut`/`FnOnce`, or any first-class
function-value/lambda syntax was found anywhere in `spec/LANGUAGE_SPEC.md`
or the `codira_syntax` grammar.** This is a real, current gap, not a
documentation omission — search both yourself before assuming otherwise if
this guide is ever stale.

Practical consequences when porting:

- Rust iterator-chain code (`.iter().map(...).filter(...).collect()`) has
  no direct translation. Port it to an explicit `for`/`while` loop over an
  array (Tier 1) accumulating into a `var`-declared result.
- Rust callback parameters (`fn register(cb: impl Fn(i32))`) have no
  Codira equivalent as ordinary in-language function values. The only
  place a "function pointer" concept exists at all is the **host-embedding
  boundary** — `extern func name(...) -> T;` plus a host-supplied
  `extern "C" fn` (see `EMBEDDING_CODIRA.md` §3.3) — which is a
  cross-language FFI mechanism, not a general Codira closure type. Model
  callback-shaped Rust code as either (a) inline logic instead of an
  injected function, or (b) an `extern`-declared hook if the "callback" is
  genuinely meant to be supplied by the host application.
- Algebraic effects (`effect`/`uses`/`perform`/`handle`/`resume`, §6 of the
  spec) are explicitly *designed* to cover some of what Rust would use
  closures/dependency-injection/`Result`-based error handling for (logging,
  failure, DI-style effects) — but per §0 this is Tier 2 (parses only, not
  lowered), so don't reach for it as a working substitute today either.

---

## 8. Generics

Rust `Vec<T>`/`HashMap<K, V>`/`fn max<T: PartialOrd>(a: T, b: T) -> T` →
Codira uses square brackets (`Box[T]`, `func max[T](a: T, b: T) -> T where
T: Comparable`), never angle brackets, and **there is no turbofish** —
explicit type arguments are only legal in type position and static member
access, never at an ordinary call site (arguments are always inferred).

Status: generics are listed among the constructs with **63 passing
lexer/parser tests**, but the full build→link→run capstone verification in
spec §12 exercised only non-generic `struct`/`func`/`let`/arithmetic code —
it did not exercise a generic function or type end-to-end. Treat generics
as **unverified at the codegen/runtime level** even though they parse
correctly; if you port a generic Rust function, expect to need a
non-generic (monomorphized-by-hand, one function per concrete type)
fallback if the generic version doesn't actually build.

---

## 9. Worked example: porting a small Rust module

Rust:

```rust
pub struct Inventory {
    items: Vec<Item>,
}

pub struct Item {
    pub name: String,
    pub quantity: u32,
}

impl Inventory {
    pub fn new() -> Self {
        Inventory { items: Vec::new() }
    }

    pub fn total_quantity(&self) -> u32 {
        self.items.iter().map(|i| i.quantity).sum()
    }

    pub fn find(&self, name: &str) -> Option<&Item> {
        self.items.iter().find(|i| i.name == name)
    }
}
```

Codira port, written to match what's actually Tier-1-verified today (no
`enum`/`match`/`Option`/closures/iterator chains/associated `new()`):

```codira
public struct Item {
    name: String
    quantity: u32
}

public struct Inventory {
    items: [Item]
}

// Constructor: plain top-level function, not Type.new() (§4 — associated
// call syntax doesn't resolve yet). Caller does `new_inventory(some_items)`.
public func new_inventory(items: [Item]) -> Inventory {
    Inventory { items: items }
}

// No .iter().map().sum() — explicit loop instead (§7: no iterator chains).
public func total_quantity(inv: Inventory) -> u32 {
    var total = 0u32
    for item in inv.items {
        total += item.quantity
    }
    total
}

// No Option<&Item> — explicit "found" flag instead of Type? (§6: Type?
// isn't enforced, and there's no clean way to express "no item" as a
// first-class value yet without enum support). Caller must check `found`
// before trusting `item`.
public struct FindResult {
    found: bool
    item: Item
}

public func find_item(inv: Inventory, name: String) -> FindResult {
    for item in inv.items {
        if item.name == name {
            return FindResult { found: true, item: item }
        }
    }
    FindResult { found: false, item: Item { name: "", quantity: 0 } }
}
```

Note everything above only uses Tier-1 constructs (`struct`, `func`,
`let`/`var`, `for`, `if`, arithmetic, array type `[T]`, struct literals) —
this is code an agent can actually expect to compile and run today, not
just parse.

---

## 10. Migration readiness checklist

Good candidates to port to Codira **today**:

- Plain-old-data-heavy simulation/gameplay state (structs of numbers/bools,
  updated every frame) — this is exactly what `examples/buoyancy` and
  `examples/rust-pong`'s Codira side model, and it's squarely Tier 1.
- Straight-line numeric/algorithmic code (no enums, traits, or closures) —
  translates almost mechanically per §1's keyword table.
- Code that already treats "fallible" as "return a sentinel/flag", not
  `Result`/`?`-heavy code.

Poor candidates until the relevant Tier-2 gaps close — port these last, or
keep them on the host (Rust) side and only expose thin Tier-1 Codira
surface via `extern`/`@export("C")` (`EMBEDDING_CODIRA.md` §6):

- Anything built around `enum` + `match` as the primary control structure.
- Trait-object/`dyn`-heavy polymorphism, or any reliance on function
  overloading.
- `Result`/`?`-based error propagation.
- Iterator-chain-heavy code, or anything passing closures/callbacks as
  values within the language itself.
- Generic-heavy libraries (verify generics work for your specific case
  before committing to a large generic-based port).
- Anything concurrent (threads, `async`/`await`, channels) — no evidence
  any of this exists yet beyond the unenforced `spawn <expr>` keyword
  scaffold in spec §14.

When in doubt, write the smallest Tier-1 version first and confirm it with
a real `codira build` (and, if embedding, `codira start`/a host `invoke`
call per `EMBEDDING_CODIRA.md`) before trusting the port — this document
describes what should work, not a substitute for actually running the
compiler.
