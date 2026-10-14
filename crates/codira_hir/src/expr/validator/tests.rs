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
    103..128: a closure is not supported by the code generator yet: there is no function-value representation or capture analysis yet
    177..179: postfix `!` unwraps an optional (`T?`), but this is `i32`
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

/// A `match` must cover every value: a missing `bool` literal, a missing
/// `nil`, or no catch-all for an integer are each reported, naming what is
/// not covered. Patterns are checked against the scrutinee's type.
#[test]
fn test_match_exhaustiveness_and_pattern_errors() {
    insta::assert_snapshot!(diagnostics(
        r#"
    func f(n: i32, b: bool, o: i32?) {
        let a = match n { 0 -> 1, 1 -> 2 };
        let c = match b { true -> 1 };
        let d = match o { 5 -> 1, v -> v };
        let e = match o { nil -> 0, 5 -> 1 };
        let g = match n { "x" -> 1, _ -> 2 };
        let h = match n { nil -> 1, _ -> 2 };
        let i = n!;
        let ok1 = match b { true -> 1, false -> 2 };
        let ok2 = match o { nil -> 0, v -> v };
        let ok3 = match (n, b) { (0, true) -> 1, (_, _) -> 2 };
    }
    "#,
    ), @"
    47..73: `match` is not exhaustive: every `i32` not matched by a literal is not covered; add an arm for it, or a `_ ->` arm
    87..108: `match` is not exhaustive: `false` is not covered; add an arm for it, or a `_ ->` arm
    122..148: `match` is not exhaustive: `nil` is not covered; add an arm for it, or a `_ ->` arm
    162..190: `match` is not exhaustive: a present value (every `i32` not matched by a literal) is not covered; add an arm for it, or a `_ ->` arm
    214..217: this pattern is a `str`, but the value it matches is a `i32`
    256..259: `nil` only matches an optional (`T?`), not `i32`
    288..290: postfix `!` unwraps an optional (`T?`), but this is `i32`
    ");
}

/// `T` converts to `T?` where an optional is expected; `nil` takes the
/// optional type it is used as; optionals compare with `==`/`!=` but are
/// not ordered.
#[test]
fn test_optional_typing() {
    insta::assert_snapshot!(diagnostics(
        r#"
    struct P { x: i32 }

    func f() -> i32? {
        let a: i32? = 5;
        let b: i32? = nil;
        let c = a == b;
        let d = a < b;
        let p: P? = nil;
        let q = p == p;
        let r = p == nil;
        let unknown = nil;
        if let x = a { return x }
        nil
    }
    "#,
    ), @"
    116..121: optionals (`i32?`) are not ordered; compare them with `==`/`!=` or unwrap them first
    156..162: `P` values cannot be compared with `==`; compare the optional with `nil` instead
    204..207: type annotations needed: the type of this expression is ambiguous
    194..201: type annotations needed: the type of this expression is ambiguous
    ");
}
