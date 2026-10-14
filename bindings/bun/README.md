<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: October 13, 2026

Functionality: Using Codira modules from Bun.
 -->

# codira-bun

Import Codira code into Bun as if it were TypeScript:

```ts
// bunfig.toml: preload = ["codira-bun/register"]
import { add, checksum } from "./native/src/mod.code";

add(1, 2);
```

Importing a `.code` file builds the Codira package it belongs to (the nearest
`codira.toml` above it), generates `bun:ffi` bindings for that module's
`@export("C")` functions with `codira bindgen --runtime bun`, and loads the
built library. Every exported function becomes a named export.

## Setup

```toml
# bunfig.toml
preload = ["codira-bun/register"]
```

or call it yourself with options:

```ts
import { plugin } from "bun";
import { codira } from "codira-bun";

plugin(codira({ codira: "/path/to/codira", optimization: 3 }));
```

The `codira` executable is taken from the `codira` option, then `$CODIRA`, then
`PATH`.

## Speed

Calls go through `bun:ffi`'s JIT-compiled trampolines with nothing of this
package in between. A call costs roughly 7 ns for `i32`/`u32`/`f64`
signatures and about 12 ns for 64-bit integers, which bind as `i64_fast` and
come back as a `number` while they fit and a `BigInt` beyond that. Crossing
the boundary per element of an array costs far more than the work, so pass a
buffer once: a Codira function taking `(address: usize, length: usize)` can
read a `TypedArray` passed as `ptr(array)` from `bun:ffi`, through the
`load_*`/`store_*` intrinsics, at native speed.

## Hot reload

Each build is loaded from a private copy named after the hash of the library,
never from the compiler's output in place. An open library is locked on
Windows and cached by path on Linux and macOS; loading the output directly
would block the next build from replacing it and hand a reload the old code.
With the copy, `bun --hot` picks up a rebuilt module on the next import.

## Limits

Only functions whose parameters and return are scalars bind (see
`codira bindgen`). A Codira function that allocates garbage-collected objects
needs the Codira runtime, which a plain `dlopen` does not set up; keep the
exported surface to scalars and caller-owned buffers.
