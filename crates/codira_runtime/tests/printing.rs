//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - End-to-end tests for the `print`/`println` builtins: Codira source is
//!   compiled, loaded into a runtime, invoked, and what it printed is compared
//!   against what it should have printed.

use codira_runtime::capture_output;
use codira_test::CompileAndRunTestDriver;

/// Compiles `source`, invokes `main` and returns everything it printed.
fn run_and_capture(source: &str) -> String {
    let driver =
        CompileAndRunTestDriver::new(source, |builder| builder).expect("failed to build driver");
    let ((), output) = capture_output(|| driver.runtime.invoke("main", ()).unwrap());
    output
}

#[test]
fn hello_world() {
    let output = run_and_capture(
        r#"
    public func main() {
        println("Hello, world!")
    }
    "#,
    );
    assert_eq!(output, "Hello, world!\n");
}

#[test]
fn print_concatenates_its_arguments_without_separators() {
    let output = run_and_capture(
        r#"
    public func main() {
        let x = 5
        print("x = ", x)
        print(", y = ")
        println(x * 2)
        println()
        println("done")
    }
    "#,
    );
    assert_eq!(output, "x = 5, y = 10\n\ndone\n");
}

#[test]
fn every_scalar_type_prints() {
    let output = run_and_capture(
        r#"
    public func main() {
        let a: i8 = -8
        let b: u8 = 255
        let c: i16 = -16
        let d: u16 = 65535
        let e: i32 = -32
        let f: u32 = 4294967295
        let g: i64 = -9223372036854775808
        let h: u64 = 18446744073709551615
        let i: i128 = -170141183460469231731687303715884105728
        let j: u128 = 340282366920938463463374607431768211455
        let k: usize = 7
        let l: isize = -7
        println(a, " ", b, " ", c, " ", d, " ", e, " ", f)
        println(g, " ", h)
        println(i, " ", j)
        println(k, " ", l)
        println(1.5, " ", 0.1, " ", 2.0, " ", 0.1f32, " ", -0.25f32)
        println(true, " ", false, " ", 1 < 2)
    }
    "#,
    );
    assert_eq!(
        output,
        "-8 255 -16 65535 -32 4294967295\n\
         -9223372036854775808 18446744073709551615\n\
         -170141183460469231731687303715884105728 340282366920938463463374607431768211455\n\
         7 -7\n\
         1.5 0.1 2.0 0.1 -0.25\n\
         true false true\n"
    );
}

#[test]
fn printing_from_a_loop_and_a_called_function() {
    let output = run_and_capture(
        r#"
    func show(label: str, value: i64) {
        println(label, ": ", value)
    }

    public func main() {
        var i = 0
        var total: i64 = 0
        while i < 4 {
            total += i
            show("total", total)
            i += 1
        }
    }
    "#,
    );
    assert_eq!(output, "total: 0\ntotal: 1\ntotal: 3\ntotal: 6\n");
}

/// A program that defines its own `print` calls its own, not the builtin.
#[test]
fn a_user_definition_shadows_the_builtin() {
    let driver = CompileAndRunTestDriver::new(
        r#"
    func print(x: i32) -> i32 { x * 2 }

    public func main() -> i32 {
        print(21)
    }
    "#,
        |builder| builder,
    )
    .expect("failed to build driver");
    let (result, output) = capture_output(|| -> i32 { driver.runtime.invoke("main", ()).unwrap() });
    assert_eq!(result, 42);
    assert_eq!(output, "");
}

/// Output printed outside `capture_output` goes to stdout, and a capture
/// only sees its own thread's output.
#[test]
fn capture_is_per_thread() {
    let driver = CompileAndRunTestDriver::new(
        r#"
    public func main() {
        print("from main")
    }
    "#,
        |builder| builder,
    )
    .expect("failed to build driver");

    let ((), outer) = capture_output(|| {
        let ((), inner) = capture_output(|| driver.runtime.invoke("main", ()).unwrap());
        assert_eq!(inner, "from main");
    });
    assert_eq!(outer, "");
}
