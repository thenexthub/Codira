<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: August 12, 2026

Functionality: Instructions for AI coding agents — how to EMBED a compiled
Codira library into a *different* host project (Rust, C, or C++), as opposed
to writing Codira source itself (see `USING_CODIRA.md`) or building the
Codira compiler (see `spec/LANGUAGE_SPEC.md` §12-§15 and `crates/**`).
-->

# Embedding Codira in a Host Project — Agent Instructions

This document teaches an AI coding agent how to take an **already-written**
Codira package (a `codira.toml` + `src/**/*.code` tree) and **load, drive, and
hot-reload it from a different project** — a Rust binary, a C application, or
a C++ application. If the task is instead "write some Codira source code",
read `USING_CODIRA.md` first; this document assumes that part is done and
focuses purely on the host-side integration.

> **Ground truth.** Everything below was verified directly against this
> repository's source on 2026-08-12: `crates/codira_runtime/src/**`,
> `crates/codira_runtime_capi/src/**`, `crates/codira_capi_utils/src/**`,
> `cpp/include/codira/**`, `c/include/codira/abi.h`, `book/src/ch02-04-extern-fn.md`,
> `book/src/ch04-04-hot-reloading.md`, and the `examples/rust-pong`,
> `examples/rust-spaceship`, `examples/rust-bevy-simple`,
> `examples/codira-extern`, `examples/codira-marshal` example projects. Where
> this guide and the actual crate/header source disagree, trust the source.

---

## 0. Read this before writing any `.code` file for a host project

The single most important trap in this repository: **`spec/LANGUAGE_SPEC.md`
describes a *new* keyword surface (`func`/`public`/`var`/`import`/`extend`,
`struct` vs `class` as the memory-kind keyword itself), but every example
project under `examples/**` and the entire `std/` tree still contains the
*old*, pre-redesign syntax** (`fn`, `pub fn`, `struct(value)`/`struct(gc)`,
`use` instead of `import`). This is explicitly documented as unfinished
migration work in `spec/LANGUAGE_SPEC.md` §13 ("Every example's `.code` file
content ... still contains pre-redesign Codira syntax") and confirmed by
direct inspection:

- `examples/codira-extern/src/mod.code`, `examples/codira-marshal/src/mod.code`,
  `examples/rust-pong/codira/src/mod.code`, and the buoyancy listings all use
  `fn`/`pub fn`/`struct(value) Foo {}`/`struct(gc) Foo {}` — **none of this
  parses under the current grammar** (`fn`, `pub`, and the `(gc)`/`(value)`
  parenthetical annotation were removed as keywords/syntax).
- `book/src/**` is a **mixed bag**: some listings (e.g.
  `book/listings/ch02-basic-concepts/*.code`, `ch04-structs/listing13.code`,
  `listing01.code`/`listing02.code` in ch04-structs) already use the new
  syntax (`func`, `public func`, plain `struct Foo { x: f32 }`), but the spec
  itself notes the book's doctest suite is only 16/47 passing overall — do
  not assume an arbitrary book listing compiles without checking it.
- `std/**` (the in-progress standard library) mixes old syntax
  (`struct(value) Optional[T] {}`, `use std.builtin.traits;`) with syntax
  the compiler doesn't support at all yet even under the new grammar
  (`func Error.new(message: String) -> Error { }` — declaring an associated
  function outside an `extend` block is not part of the spec's own grammar).
  **Treat `std/` as a design reference only, never as code guaranteed to
  compile.**

**Rule for agents:** when you write new `.code` source that a host project
will embed, always use the syntax in `spec/LANGUAGE_SPEC.md` §1-§11 (the same
rules documented in `USING_CODIRA.md`), never copy an `examples/**` or
`std/**` file verbatim as if it reflects current syntax. Then confirm with a
real `codira build` before wiring up the host side.

Also respect the implementation-status split from `USING_CODIRA.md` §5 when
designing the Codira side of an embedding: **only** `struct`, `class`, `func`,
`let`/`var`, field access, arithmetic/comparisons, `if`/`else`, `for`/`while`,
function calls, `extern` function declarations, and `import`/visibility are
verified end-to-end through parse → HIR → LLVM → link → run. `enum`, `trait`,
`match`, `effect`/`handle`/`perform`, `comptime`, generics-at-runtime,
multiple-dispatch overloads, and `Type.method()` associated-function calls
currently parse but are **not** safe to depend on for a host integration —
avoid designing the Codira side of an embedded script around them for now.

---

## 1. Why embed Codira at all

Codira is designed the way `mun-lang` was: not as a standalone executable
language, but as an **ahead-of-time-compiled, hot-reloadable scripting/logic
layer that a native host application loads as a library**. `codira build`
never produces an OS executable — it produces `target/<module>.codiralib`, a
dynamic library with a stable C-compatible ABI. A host process:

1. Loads that `.codiralib` (and its dependencies) through the **Codira
   Runtime**.
2. Supplies implementations for any `extern` functions the Codira code
   declared (the Codira → host direction).
3. Calls public Codira functions by name (the host → Codira direction).
4. Optionally polls the runtime once per frame/tick so that, while the
   compiler is running in `--watch` mode and recompiling on save, the host
   process swaps in the new code **without restarting** — this is
   hot-reloading, and it is the feature this whole architecture exists for.

There are three ways to be "the host", depending on the host language:

| Host language | What you link against | Primary source of truth |
|---|---|---|
| Rust | the `codira_runtime` crate | `crates/codira_runtime/src/lib.rs`, `adt.rs`, `array.rs` |
| C | `codira_runtime_capi` (built as a C-ABI library) + `c/include/codira/abi.h` | `crates/codira_runtime_capi/src/**` |
| C++ | the header-only wrapper in `cpp/include/codira/**` (itself built on the C API) | `cpp/include/codira/runtime.h`, `struct_ref.h`, `array_ref.h`, `marshal.h` |

All three are different faces on the exact same underlying runtime; the Rust
API is the most complete/idiomatic and is what every example project in this
repo actually uses (`examples/rust-pong`, `examples/rust-spaceship`,
`examples/rust-bevy-simple`, `examples/buoyancy`).

---

## 2. Project layout for an embedding

The convention used throughout `examples/**` is a host project that contains
a full, nested Codira package:

```
my_host_project/
├── Cargo.toml                 # (or CMakeLists.txt for C/C++)
├── src/
│   └── main.rs                # loads and drives the runtime
└── codira/                      # a complete, independent Codira package
    ├── codira.toml
    └── src/
        └── mod.code
```

Nothing links the two build systems together automatically — `cargo
build`/`cmake` builds the host binary, and `codira build` (run separately,
typically with `--watch`) produces the `.codiralib` that the host loads *at
runtime* by path. This is why every example's README says to run two
commands in two terminals (see `examples/rust-pong/README.md`):

```bash
# terminal 1 — compiler daemon, rebuilds on every save
codira build --watch --manifest-path=my_host_project/codira/codira.toml

# terminal 2 — the host binary, which loads codira/target/mod.codiralib
cargo run
```

---

## 3. Embedding from Rust (the primary, best-supported path)

### 3.1 Dependency

```toml
# Cargo.toml
[dependencies]
codira_runtime = { path = "../../crates/codira_runtime" }
```

This repository does not appear to publish `codira_runtime` to crates.io (no
`[package.publish]`/registry metadata was found and every example uses a
relative `path` dependency) — use a path or git dependency, not a version
requirement, unless you've confirmed a registry release exists.

### 3.2 Loading a library and calling into it

```rust
use codira_runtime::Runtime;

fn main() {
    // Safety: the caller is asserting the file at this path is a
    // Codira-compiler-produced .codiralib. Runtime::builder().finish() is
    // `unsafe` for exactly this reason — it does not re-validate the file.
    let builder = Runtime::builder("codira/target/mod.codiralib");
    let mut runtime = unsafe { builder.finish() }.expect("Failed to spawn Runtime");

    // Turbofish/annotation picks which Rust type the Codira return value is
    // marshalled into; argument types are inferred from the tuple.
    let result: i64 = runtime.invoke("some_function", (1i64, 2i64)).unwrap();
    println!("{result}");
}
```

Key API surface on `codira_runtime::Runtime` / `RuntimeBuilder`
(`crates/codira_runtime/src/lib.rs`):

- `Runtime::builder<P: Into<PathBuf>>(library_path) -> RuntimeBuilder`
- `RuntimeBuilder::insert_fn<S: Into<String>, F>(name, function) -> RuntimeBuilder`
  — registers a host function that satisfies an `extern` declaration in the
  Codira source (see §3.3). Chainable — call it once per `extern` function.
- `unsafe { builder.finish() } -> Result<Runtime, InitError>` — actually
  loads the library and links `extern` symbols. `unsafe` because it trusts
  the library path/contents.
- `runtime.invoke::<ReturnType, ArgTypes>(function_name: &str, arguments) ->
  Result<ReturnType, InvokeErr<...>>` — calls a **public** Codira function.
  `ArgTypes` is a tuple (`()`, `(i64,)`, `(f32, StructRef)`, …); trailing
  comma required for one-argument tuples.
- `unsafe { runtime.update(&mut self) -> bool }` — polls for a hot-reloaded
  library and swaps it in if the watching compiler produced a new build;
  returns whether an update happened. Call this once per host frame/tick if
  you started the compiler with `--watch`. `unsafe` for the same
  code-swap-under-your-feet reason `finish()` is.
- `runtime.get_function_definition(name) -> Option<Arc<FunctionDefinition>>`,
  `get_type_info_by_name`/`get_type_info_by_id` — reflection, useful for
  generic tooling rather than typical gameplay code.
- `runtime.gc() -> &GarbageCollector`, `runtime.gc_collect() -> bool`,
  `runtime.gc_stats() -> gc::Stats` — the Codira heap is a separate,
  runtime-managed GC; you generally don't need to drive it manually.
- `runtime.construct_array<...>` / `construct_typed_array<...>` — build a
  Codira-side array from a Rust iterator to pass into `invoke`.

### 3.3 `extern` functions — Codira calling into the host

A Codira function declared `extern` has no body; the *host* must supply the
implementation before the library can be loaded, or `finish()` fails at link
time:

```codira
extern func random() -> i64;

public func random_bool() -> bool {
    random() % 2 == 0
}
```

```rust
use codira_runtime::Runtime;

extern "C" fn random() -> i64 {
    std::time::Instant::now().elapsed().subsec_nanos() as i64
}

fn main() {
    let builder = Runtime::builder("main.codiralib")
        .insert_fn("random", random as extern "C" fn() -> i64);
    let mut runtime = unsafe { builder.finish() }.expect("Failed to spawn Runtime");
    let result: bool = runtime.invoke("random_bool", ()).unwrap();
}
```

Two things that will bite you if skipped:

- The Rust function **must** be `extern "C"` and must be cast to its
  explicit `extern "C" fn(...) -> ...` type at the `insert_fn` call site —
  every Rust function has a distinct anonymous type, so the cast is what
  makes it match the ABI-level function pointer `insert_fn` expects.
- If you forget an `extern` binding, `finish()` returns an error whose
  message is literally `Failed to link: function `<name>` is missing.` —
  that error means "call `.insert_fn` for this name", not a build problem on
  the Codira side.

### 3.4 Structs across the boundary: `StructRef`, `RootedStruct`, `ArrayRef`

Codira `struct`/`class` values crossing into Rust show up as
`codira_runtime::StructRef<'s>` — a typed-but-reflective handle, borrowed from
the runtime and **not** rooted (i.e., eligible for GC unless you root it):

```rust
use codira_runtime::{Runtime, StructRef, RootedStruct};

let state: StructRef = runtime.invoke("new_state", ()).unwrap();
let state: RootedStruct = state.root();       // keep it alive across GC cycles

// later, any time you need it again as a StructRef to read/write fields:
let state_ref = state.as_ref(&runtime);
let score: u32 = state_ref.get("score").unwrap();
let mut paddle: StructRef = state_ref.get("paddle_left").unwrap();
paddle.set("move_up", true).unwrap();
```

- `StructRef::get::<T>(field_name) -> Result<T, String>` and
  `StructRef::set::<T>(field_name, value) -> Result<(), String>` /
  `replace::<T>(...)` are the field accessors; `T` must implement the
  runtime's `Marshal` trait (see §3.5) and its static type must match the
  field's declared Codira type or you get a descriptive `Err`, not a panic.
- `.root()` converts a `StructRef` into an owned `RootedStruct` that keeps
  the GC from collecting it — store this in your host-side game/app state,
  not the borrowed `StructRef`.
- `RootedStruct::as_ref(&runtime) -> StructRef` converts back when you need
  to actually read/write it; this is the standard pattern seen throughout
  `examples/rust-pong/src/main.rs`.
- Arrays follow the same rooted/unrooted split via `ArrayRef<'a, T>`
  (`crates/codira_runtime/src/array.rs`).
- **Struct value-vs-reference kind is transparent to you as the host.**
  Whether the Codira type is a value `struct` or a GC `class`, you always get
  back a `StructRef`/heap handle at the FFI boundary — the compiler
  auto-boxes value-struct returns/arguments onto the GC heap specifically for
  cross-boundary calls (see the dispatch-table bug/fix described in
  `spec/LANGUAGE_SPEC.md` §15 for exactly why this had to be made consistent).
  You don't need to special-case `struct` vs `class` on the Rust side.

### 3.5 What marshals directly (no `StructRef` needed)

`crates/codira_runtime/src/reflection.rs` implements `ArgumentReflection` /
`ReturnTypeReflection` (the traits `Marshal` builds on) for these primitive
Rust types, which map 1:1 to Codira's primitive types of the same
bit-width: `bool`, `f32`, `f64`, `i8`, `i16`, `i32`, `i64`, `i128`, `u8`,
`u16`, `u32`, `u64`, `u128`. `()` marshals for a Codira function with no
return value. Anything else — `struct`/`class` instances and arrays — goes
through `StructRef`/`ArrayRef` as shown above.

There is **no** built-in marshalling for `String`/text. `examples/codira-marshal`
(which exercises every primitive pairwise) has 128-bit ints and their
struct-wrapped forms commented out with `// TODO: Add 128-bit integers`,
even though the reflection macro list above does include `i128`/`u128` —
treat 128-bit and any string/collection marshalling as **unverified**; test
it directly before depending on it, and prefer plain fixed-width
scalars/structs for anything you need working today.

### 3.6 Hot reload from the host side

Hot reload requires nothing extra beyond calling `unsafe {
runtime.update() }` periodically (once per frame is typical — see
`examples/rust-pong/src/main.rs`'s `update()` handler and
`book/listings/ch04-structs/listing14.rs`'s main loop) while a `codira build
--watch` process is recompiling the same library on save. What survives a
reload and how, per `book/src/ch04-04-hot-reloading.md` (verified accurate
against that chapter):

- Struct/array hot reload applies **recursively** (a struct containing a
  struct field, or an array of structs, reloads correctly).
- A newly **inserted** field is recursively zero-initialized on next reload.
- Field-diffing priority when a struct's shape changes between reloads:
  1. Same name + same type → unchanged, the value is **moved** as-is.
  2. Same name, different type → treated as the same field undergoing a
     **type conversion** (and possibly moved).
  3. Different name, same type → treated as a possible **rename** (nearest
     original-index candidate wins when ambiguous).
- Restrictions: a struct can't simultaneously be renamed **and** have its
  fields edited in the same reload; a field can't simultaneously be renamed
  **and** type-converted. Either case is instead treated as an independent
  insertion + deletion.
- Practical trick used in the book's tutorial: add a throwaway `token: u32`
  field plus a `hot_reload_token() -> u32` extern/const function that you
  bump by hand; compare it once per reload to run one-time re-initialization
  logic against the freshly hot-reloaded state, then delete the scaffolding
  once you're done iterating.

---

## 4. Embedding from C

The C ABI is exposed by the `codira_runtime_capi` crate
(`crates/codira_runtime_capi/src/**`), built as a C-compatible library; the
matching `abi.h` (struct/type layout definitions, not the runtime API itself)
lives at `c/include/codira/abi.h`. The **complete** list of exported C
functions, taken directly from `#[no_mangle] pub extern "C" fn` in that
crate plus its `codira_capi_utils` dependency, is:

```
codira_runtime_create(library_path: *const c_char, options: RuntimeOptions, handle: *mut Runtime) -> ErrorHandle
codira_runtime_destroy(runtime: Runtime) -> ErrorHandle
codira_runtime_update(runtime: Runtime, updated: *mut bool) -> ErrorHandle
codira_runtime_find_function_definition(...) -> ErrorHandle
codira_runtime_get_type_info_by_name(...) -> ErrorHandle
codira_runtime_get_type_info_by_id(...) -> ErrorHandle

codira_gc_alloc(runtime: Runtime, ty: Type, obj: *mut GcPtr) -> ErrorHandle
codira_gc_ptr_type(...) -> ErrorHandle
codira_gc_root(runtime: Runtime, obj: GcPtr) -> ErrorHandle
codira_gc_unroot(runtime: Runtime, obj: GcPtr) -> ErrorHandle
codira_gc_collect(runtime: Runtime, reclaimed: *mut bool) -> ErrorHandle

codira_function_add_reference(function: Function) -> ErrorHandle
codira_function_release(function: Function) -> ErrorHandle
codira_function_fn_ptr(...) -> ErrorHandle
codira_function_name(...) -> ErrorHandle
codira_function_argument_types(...) -> ErrorHandle
codira_function_return_type(...) -> ErrorHandle

codira_error_destroy(error: ErrorHandle)
codira_string_destroy(string: *const c_char)
```

Conventions to follow when calling this API directly from C:

- **Error handling.** Nearly every function returns `ErrorHandle` (`struct
  ErrorHandle(*const c_char)`, `crates/codira_capi_utils/src/error.rs`).
  `ErrorHandle::is_ok()` corresponds to a null inner pointer; a non-null
  handle carries a C string error message you can read and **must** free
  with `codira_error_destroy` once you're done with it.
- **Startup:** call `codira_runtime_create(library_path, options, &mut
  handle)`. `options: RuntimeOptions { functions: *const
  ExternalFunctionDefinition, num_functions: u32 }` is how you supply
  `extern` bindings up front (the C equivalent of Rust's chained
  `insert_fn` calls) — build the array of `ExternalFunctionDefinition`
  (name, arg types, return type, raw `fn_ptr`) before the single create
  call, there is no incremental "insert" call in the C API.
- **Shutdown:** `codira_runtime_destroy(runtime)`.
- **Hot reload:** call `codira_runtime_update(runtime, &mut updated)`
  periodically, exactly like the Rust `runtime.update()`.
- **GC:** `codira_gc_root`/`codira_gc_unroot` around any `GcPtr` you hold
  onto outside of a single call (the C-level equivalent of Rust's
  `StructRef::root()` → `RootedStruct`), `codira_gc_collect` to force a
  collection, `codira_gc_alloc` to allocate Codira-managed memory directly.

Because this is a raw C ABI, prefer the C++ wrapper (§5) or the Rust API
(§3) whenever the host language allows it — they add lifetime/RAII/type
safety over the exact same underlying calls.

---

## 5. Embedding from C++

`cpp/include/codira/**` is a **header-only** C++ wrapper around the C API in
§4 — link against the same `codira_runtime_capi`-produced library, but write
idiomatic C++ against these headers instead of the raw `extern "C"`
functions:

| Header | Purpose |
|---|---|
| `codira.h` | umbrella include |
| `runtime.h` | `codira::Runtime` — RAII wrapper: construction/`make_runtime(library_path, RuntimeOptions, Error*)`, `update(Error* = nullptr)`, `gc_alloc`/`gc_collect`/`gc_root_ptr`/`gc_unroot_ptr`/`ptr_type`, `find_function_info` |
| `struct_ref.h` | C++ equivalent of Rust's `StructRef` |
| `array_ref.h` | C++ equivalent of Rust's `ArrayRef` |
| `marshal.h` | primitive marshalling glue |
| `function.h`, `runtime_function.h` | typed function handles/invocation |
| `reflection.h`, `type.h`, `struct_type.h`, `array_type.h`, `field_info.h`, `static_type_info.h` | reflection/type-info surface |
| `error.h` | RAII `Error` wrapper over `ErrorHandle` |
| `gc.h` | GC pointer type (`CodiraGcPtr`) |
| `runtime_capi.h` | the raw C declarations the rest of these headers build on |

`Runtime`'s constructor takes a `CodiraRuntime` C handle and is move-only
(`Runtime(Runtime&&)`); the free function that actually creates one takes the
library path plus a `RuntimeOptions`-equivalent (a list of
`CodiraExternalFunctionDefinition`s, each type-reference-counted via
`codira_type_add_reference` internally) and an optional `Error*` out-parameter,
mirroring the Rust `RuntimeBuilder`/C `codira_runtime_create` shape one-to-one.
`update()` and the `gc_*` methods forward directly to the C functions in §4
with the same semantics (e.g. `update(Error* out_error = nullptr)` returns
whether a reload occurred, `CODIRA_ASSERT`-wrapping the C call otherwise).

---

## 6. Two-directional interop from the Codira side

Everything above is the *host's* view. The Codira-source side of the same
contract (declared in `spec/LANGUAGE_SPEC.md` §10) is:

```codira
extern "C" {
    func sqrt(x: f64) -> f64;
    func malloc(size: usize) -> RawPointer[u8];
}

@export("C")                      // expose this Codira function to C/C++/Rust callers
public func codira_add(a: i32, b: i32) -> i32 { a + b }
```

- A bare `extern func name(...) -> T;` (no `"C"`/`"C++"` block, as used by
  `runtime.insert_fn`/`RuntimeOptions` in §3-§4) is the *runtime-supplied*
  extern mechanism — resolved by the host's `Runtime` at load time, not by
  the system linker.
- An `extern "C" { ... }` block is a normal linker-resolved C symbol
  (`sqrt`, `malloc`, libc, etc.) — resolved at Codira's own link step, not by
  the embedding host.
- `@export("C")` is how a Codira function becomes callable *from* C/C++
  (unmangled, C calling convention) — this is the mechanism
  `codira_runtime`/`codira_runtime_capi` themselves use to expose the runtime to
  C, generalized to any function you mark this way.
- `extern "C++"` blocks parse identically to `extern "C"` today; C++'s lack
  of a stable cross-compiler name-mangling scheme means proper mangled-name
  metadata for this block is **designed but not implemented yet** — don't
  rely on calling a real C++ (as opposed to `extern "C"`-declared) symbol
  this way until that lands.
- `RawPointer[T]` is the escape hatch for anything without a `Marshal`
  impl (raw buffers, C strings, etc.) — no GC/bounds-checking on that
  pointer, by design, the same way `unsafe` boundaries work in any other
  safe language with C interop.

---

## 7. Minimal end-to-end checklist for a new embedding

1. `codira new my_project/codira` (or `codira init` inside an existing
   `codira/` subdirectory) — write the Codira side using
   `spec/LANGUAGE_SPEC.md` §1-§11 syntax (§0 above), not any `examples/**`
   file's syntax.
2. Declare every function the host must call as `public func`. Declare every
   function the host must implement as `extern func name(...) -> T;`.
3. On the host side, add a dependency on `codira_runtime` (Rust) or link
   against `codira_runtime_capi`/use the `cpp/include/codira` headers (C/C++).
4. Build the Codira runtime handle: `Runtime::builder(path)` →
   `.insert_fn(...)` for every `extern` → `unsafe { .finish() }`.
5. Call in with `runtime.invoke::<Ret, Args>("name", args)`; for anything
   that returns a `struct`/`class`, immediately `.root()` it if you need to
   keep it past this call.
6. If you want hot reload, start the compiler with `codira build --watch
   --manifest-path=<path to codira.toml>` and call `unsafe {
   runtime.update() }` once per host tick.
7. Verify by actually running both processes side by side — this is
   integration behavior no `cargo check`/type-checker can confirm for you.

Happy embedding!
