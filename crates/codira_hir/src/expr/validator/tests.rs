//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
#[cfg(test)]
use crate::utils::tests::*;

#[test]
fn test_uninitialized_access() {
    insta::assert_snapshot!(
        diagnostics(r#"
    func foo() {
        let a:i64;
        let b = a + 3;
    }
    "#), @"40..41: use of possibly-uninitialized variable"
    );
}

#[test]
fn test_uninitialized_access_if() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func foo() {
        let a:i64;
        if true { a = 3; } else { a = 4; }
        let b = a + 4;  // correct, `a` is initialized either way
    }

    func bar() {
        let a:i64;
        if true { a = 3; }
        let b = a + 4;  // `a` is possibly-unitialized
    }

    func baz() {
        let a:i64;
        if true { return } else { a = 4 };
        let b = a + 4;  // correct, `a` is initialized either way
    }

    func foz() {
        let a:i64;
        if true { a = 4 } else { return };
        let b = a + 4;  // correct, `a` is initialized either way
    }

    func boz() {
        let a:i64;
        return;
        let b = a + 4;  // `a` is not initialized but this is dead code anyway
    }
    "#,
    ), @"195..196: use of possibly-uninitialized variable");
}

#[test]
fn test_uninitialized_access_while() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func foo(b:i64) {
        let a:i64;
        while b < 4 { b += 1; a = b; a += 1; }
        let c = a + 4;  // `a` is possibly-unitialized
    }
    "#,
    ), @"88..89: use of possibly-uninitialized variable");
}

#[test]
fn test_free_type_alias_without_type_ref() {
    insta::assert_snapshot!(diagnostics(
        r#"
    type Foo; // `Foo` must have a target type
    "#,
    ), @"0..9: free type alias without type ref");
}

#[test]
fn test_private_leak_function_return() {
    insta::assert_snapshot!(diagnostics(
        r#"
    struct Foo(usize);

    public func bar() -> Foo { // Foo is not public
        Foo(0)
    }

    public func baz(a: usize, b: usize) -> Foo {
        Foo(2)
    }

    public struct FooBar(usize);

    public func FooBaz() -> FooBar {
        FooBar(0)
    }

    func BarBaz() -> FooBar {
        FooBar(1)
    }
    "#,
    ), @"
    41..44: can't leak private type
    121..124: can't leak private type
    ");
}

#[test]
fn test_private_leak_function_args() {
    insta::assert_snapshot!(diagnostics(
        r#"
    struct Foo(usize);

    public func bar(a: Foo, b: isize) -> usize{ // Foo is not public
        0
    }

    public func baz(a: isize, b: Foo) -> isize {
        -1
    }

    public struct FooBar(usize);

    public func FooBaz(a: FooBar) -> FooBar {
        a
    }

    func BarBaz(a: isize, b: FooBar) -> isize {
        a
    }
    "#,
    ), @"
    39..42: can't leak private type
    123..126: can't leak private type
    ");
}

#[test]
fn test_private_leak_function_scoped() {
    insta::assert_snapshot!(diagnostics(
        r#"
    // Illegal, Bar has a smaller scope than this use statement
    public(super) struct Bar;

    // Illegal, Bar has a smaller scope than this function
    public func baz() -> Bar {
        Bar
    }
    "#,
    ), @"
    Offset(66): expected a declaration
    Offset(67): expected a declaration
    Offset(72): expected a declaration
    163..166: can't leak private type
    ");
}

#[test]
fn test_private_leak_alias() {
    insta::assert_snapshot!(diagnostics(
        r#"
    type Bar = usize;

    public func baz() -> Bar {
        0
    }
    "#,
    ), @"40..43: can't leak private type");
}

#[test]
fn test_type_alias_with_private_struct() {
    insta::assert_snapshot!(diagnostics(
        r#"
    struct Foo;

    public type Bar = Foo;
    "#,
    ), @"13..35: struct `Foo` is private");
}

#[test]
fn test_type_alias_with_private_type_alias() {
    insta::assert_snapshot!(diagnostics(
        r#"
    type Foo = i32;

    public type Bar = Foo;
    "#,
    ), @"17..39: type alias `Foo` is private");
}

/// Constructs the front end parses but does not lower become `Expr::Missing`.
/// Each one must surface as a diagnostic naming what was written, so that it
/// stops the build here instead of reaching the code generator's
/// `Expr::Missing` arm and crashing the compiler.
#[test]
fn test_unlowered_constructs_are_reported_not_crashed() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func with_match(x: i32) -> i32 {
        match x { 1 -> 10, _ -> 20 }
    }

    func with_closure() {
        let f = func(a: i32) -> i32 { a };
    }

    func with_force_unwrap(x: i32) -> i32 {
        x!
    }
    "#,
    ), @"
    37..65: `match` is not supported by the code generator yet: pattern matching has no lowering yet; use `if`/`else` chains for now
    177..179: postfix `!` (force-unwrap) is not supported by the code generator yet: `Type?` optionals have no runtime representation yet
    ");
}

/// Misuses of `for`, ranges and `continue` are type errors with a message
/// saying what is allowed.
#[test]
fn test_for_range_and_continue_errors() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func f(x: i32, b: bool) {
        let r = 0..10
        for i in x { }
        for i in 0..b { }
        for 1 in [1, 2] { }
        continue
    }
    "#,
    ), @"
    38..43: a range can only be used as the iterable of a `for` loop for now; there is no range value type yet
    57..58: cannot iterate over a value of type `i32`: a `for` loop iterates over a range (`a..b`, `a..=b`) or an array
    79..80: mismatched type
    76..77: a range's bounds must be integers, found `bool`
    93..94: a `for` loop's pattern must always match: use a name, `_` or a tuple of those
    113..121: `continue` outside of a loop
    ");
}

/// A closure captures by value: assigning to a captured variable, or to a
/// field of a captured value struct, inside the closure is an error. A field
/// of a captured class instance is shared storage and may be assigned, as may
/// the closure's own locals. A parameter type must come from somewhere, and
/// a tuple cannot be captured yet (the runtime has no type info for it).
#[test]
fn test_closure_capture_rules() {
    insta::assert_snapshot!(diagnostics(
        r#"
    struct Point { x: i32, y: i32 }
    class Counter { n: i32 }

    func f() {
        var x = 1
        let p = Point { x: 1, y: 2 }
        let c = Counter { n: 1 }
        let g = func() { x = 2 }
        let h = func() { p.x = 3 }
        let k = func() { c.n = 3 }
        let m = func() { var y = 1; y = 2 }
        let u = func(a) { a }
        let t = (1, 2)
        let w = func() -> i32 { t.0 }
    }
    "#,
    ), @"
    166..171: cannot assign to `x` inside this closure: the closure captured `x` by value when it was created, so the assignment would change only the closure's copy; keep state the closure changes in a class instance and capture that
    195..202: cannot assign to `p` inside this closure: the closure captured `p` by value when it was created, so the assignment would change only the closure's copy; keep state the closure changes in a class instance and capture that
    293..294: cannot infer the type of this closure parameter: write it out (`x: T`), or pass the closure where a function type is expected
    333..354: this closure cannot capture `t` of type `(i32, i32)` yet: a closure can capture integers, floats, `bool`, structs, classes, arrays and function values
    ");
}

/// Function values are typed by signature; using one at the wrong type,
/// with the wrong arguments, or with `==` is a type error, and a generic
/// function cannot become a value.
#[test]
fn test_function_value_type_errors() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func square(x: i32) -> i32 { x * x }
    func id[T](x: T) -> T { x }

    func f() -> bool {
        let a = square
        let b: func(i32) -> bool = square
        let c = id
        a(1, 2)
        a == a
    }
    "#,
    ), @"
    42..44: a generic function (`T` is not instantiated) is not supported by the code generator yet: monomorphisation is not implemented yet, so there is no concrete type to generate code for
    135..141: mismatched type
    154..156: the generic function `id` cannot be used as a function value yet: generic functions cannot be instantiated
    161..168: this function takes 1 parameters but 2 parameters was supplied
    173..179: cannot apply binary operator
    ");
}

/// Closures carry no effect handlers and are not tasks: they cannot perform,
/// a function that uses effects cannot become a value, `spawn` cannot run a
/// function value, and `break` does not leave the closure.
#[test]
fn test_closure_effects_tasks_and_loops() {
    insta::assert_snapshot!(diagnostics(
        r#"
    effect Fail {
        func fail(code: i32) -> i32
    }

    func risky() uses Fail -> i32 { perform Fail.fail(1) }

    func g() uses Fail -> i32 {
        let f = func() -> i32 { perform Fail.fail(2) }
        let h = risky
        let k = func() -> i32 { risky() }
        1
    }

    func work() {}

    func t() {
        scope {
            let f = work
            spawn f()
        }
    }

    func l() {
        loop {
            let f = func() { break }
        }
    }
    "#,
    ), @"
    196..201: `risky` uses effects, so it cannot be used as a function value: a function value carries no effect handlers
    161..181: a closure cannot `perform` an effect: a function value carries no effect handlers
    230..237: a closure cannot call `risky`, which uses effects: a function value carries no effect handlers
    323..326: `spawn` cannot run a function value: a closure may carry references to shared objects, and nothing proves them safe to use from another task; spawn a named function of this module instead
    383..388: `break` outside of a loop
    ");
}

/// A closure that captures a secret is secret; a constant-time function
/// neither creates nor calls function values; a verified function's closure
/// bodies are reported as unverified.
#[test]
fn test_closure_secrets_and_contracts() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func leak(key: secret u64) -> func() -> u64 {
        func() -> u64 { key }
    }

    @constant_time
    func ct(a: secret u64) -> secret u64 {
        let f = func(x: u64) -> u64 { x }
        f(a)
    }

    func verified(x: i64) -> i64
        requires x > 0
    {
        let f = func(y: i64) -> i64 { y / x }
        f(1)
    }
    "#,
    ), @"
    50..71: secret value returned from a function whose result is not `secret` (wrap it in `declassify(..)` if revealing it is intended)
    141..166: a constant-time function cannot create a closure: whatever calls it is not known to be constant-time
    171..175: a constant-time function cannot call a function value: the callee is not known to be constant-time
    173..174: secret value passed to a function value, whose parameters are never `secret`
    241..270: this closure's body is not verified yet: the obligations in it (divisions, calls to functions with contracts) would go unchecked; move that code into a named function
    ");
}

/// A function-typed parameter of a generic function is passed on to another
/// generic function: the instantiated parameter unifies with the caller's
/// own `T` (only the code generator's "not instantiated" notes remain).
#[test]
fn test_function_parameter_passed_between_generic_functions() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func g[T](a: T, cmp: func(T, T) -> i32) -> i32 { cmp(a, a) }
    func f[T](a: T, cmp: func(T, T) -> i32) -> i32 {
        g(a, cmp)
    }
    "#,
    ), @"
    5..6: a generic function (`T` is not instantiated) is not supported by the code generator yet: monomorphisation is not implemented yet, so there is no concrete type to generate code for
    66..67: a generic function (`T` is not instantiated) is not supported by the code generator yet: monomorphisation is not implemented yet, so there is no concrete type to generate code for
    ");
}
