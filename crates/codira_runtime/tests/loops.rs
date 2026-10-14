//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - End-to-end tests for `for` loops over ranges and arrays, `continue`, array
//!   `len()`, `unchecked_get`, and short-circuiting `&&`/`||`.

#[macro_use]
mod util;

use codira_runtime::capture_output;
use codira_test::CompileAndRunTestDriver;

fn driver(source: &str) -> CompileAndRunTestDriver {
    CompileAndRunTestDriver::new(source, |builder| builder).expect("failed to build driver")
}

#[test]
fn for_over_ranges() {
    let driver = driver(
        r#"
    public func sum_exclusive(n: i64) -> i64 {
        var total: i64 = 0
        for i in 0..n { total += i }
        total
    }

    public func sum_inclusive(n: i64) -> i64 {
        var total: i64 = 0
        for i in 1..=n { total += i }
        total
    }

    // An inclusive range ending at the type's maximum must stop, not wrap.
    public func count_all_u8() -> i32 {
        var count = 0
        for _ in 0u8..=255u8 { count += 1 }
        count
    }

    // A range whose start is past its end runs zero times.
    public func backwards() -> i32 {
        var count = 0
        for _ in 10..0 { count += 1 }
        count
    }

    public func negative_bounds() -> i32 {
        var total = 0
        for i in -3..=3 { total += i * i }
        total
    }
    "#,
    );
    assert_invoke_eq!(i64, 45, driver, "sum_exclusive", 10i64);
    assert_invoke_eq!(i64, 0, driver, "sum_exclusive", 0i64);
    assert_invoke_eq!(i64, 55, driver, "sum_inclusive", 10i64);
    assert_invoke_eq!(i32, 256, driver, "count_all_u8");
    assert_invoke_eq!(i32, 0, driver, "backwards");
    assert_invoke_eq!(i32, 28, driver, "negative_bounds");
}

#[test]
fn for_over_arrays_and_len() {
    let driver = driver(
        r#"
    public func sum() -> i32 {
        let xs = [3, 1, 4, 1, 5, 9, 2, 6]
        var total = 0
        for x in xs { total += x }
        total
    }

    public func length() -> usize {
        let xs = [3, 1, 4]
        xs.len()
    }

    // Indexing by position, the way most loops over arrays are written.
    public func max() -> i32 {
        let xs = [7, -2, 19, 4]
        var best = xs[0]
        for i in 1..xs.len() {
            if xs[i] > best { best = xs[i] }
        }
        best
    }

    public func unchecked() -> i32 {
        let xs = [10, 20, 30]
        var total = 0
        for i in 0..xs.len() { total += xs.unchecked_get(i) }
        total
    }
    "#,
    );
    assert_invoke_eq!(i32, 31, driver, "sum");
    assert_invoke_eq!(usize, 3, driver, "length");
    assert_invoke_eq!(i32, 19, driver, "max");
    assert_invoke_eq!(i32, 60, driver, "unchecked");
}

#[test]
fn continue_and_break() {
    let driver = driver(
        r#"
    // Sum of the odd numbers below 20, stopping at the first above 12.
    public func in_for() -> i32 {
        var total = 0
        for i in 0..20 {
            if i % 2 == 0 { continue }
            if i > 12 { break }
            total += i
        }
        total
    }

    public func in_while() -> i32 {
        var i = 0
        var total = 0
        while i < 10 {
            i += 1
            if i % 3 == 0 { continue }
            total += i
        }
        total
    }

    public func in_loop() -> i32 {
        var i = 0
        var total = 0
        loop {
            i += 1
            if i > 6 { break }
            if i == 2 { continue }
            total += i
        }
        total
    }

    // `continue` and `break` apply to the innermost loop only.
    public func nested() -> i32 {
        var count = 0
        for i in 0..5 {
            for j in 0..5 {
                if j > i { break }
                if j == 1 { continue }
                count += 1
            }
        }
        count
    }
    "#,
    );
    assert_invoke_eq!(i32, 36, driver, "in_for");
    assert_invoke_eq!(i32, 37, driver, "in_while");
    assert_invoke_eq!(i32, 19, driver, "in_loop");
    assert_invoke_eq!(i32, 11, driver, "nested");
}

/// `&&` and `||` evaluate their right operand only when the left one does
/// not already decide the result -- a bounds-checked index behind a length
/// guard must not run, and neither must a call made for its side effect.
#[test]
fn logical_operators_short_circuit() {
    let driver = driver(
        r#"
    func noisy(result: bool) -> bool {
        println("evaluated")
        result
    }

    public func guarded(i: usize) -> bool {
        let xs = [1, 2, 3]
        i < xs.len() && xs[i] > 1
    }

    public func main() {
        println(false && noisy(true))
        println(true || noisy(false))
        println(true && noisy(true))
        println(false || noisy(false))
    }
    "#,
    );
    assert_invoke_eq!(bool, true, driver, "guarded", 1usize);
    assert_invoke_eq!(bool, false, driver, "guarded", 0usize);
    assert_invoke_eq!(bool, false, driver, "guarded", 7usize);

    let ((), output) = capture_output(|| driver.runtime.invoke("main", ()).unwrap());
    assert_eq!(output, "false\ntrue\nevaluated\ntrue\nevaluated\nfalse\n");
}

#[test]
fn printing_inside_a_for_loop() {
    let driver = driver(
        r#"
    public func main() {
        for i in 1..=3 {
            println(i, " squared is ", i * i)
        }
    }
    "#,
    );
    let ((), output) = capture_output(|| driver.runtime.invoke("main", ()).unwrap());
    assert_eq!(output, "1 squared is 1\n2 squared is 4\n3 squared is 9\n");
}
