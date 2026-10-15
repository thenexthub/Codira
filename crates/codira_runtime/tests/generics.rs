//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - End-to-end tests of generics: generic functions, structs and classes built
//!   through LLVM by monomorphisation and run by the runtime, including across
//!   modules and through a hot reload.
#[macro_use]
mod util;

use codira_runtime::StructRef;
use codira_test::CompileAndRunTestDriver;

/// One generic function, called at several types in one module: each call
/// runs the instance for its own type.
#[test]
fn generic_functions_at_several_instantiations() {
    let driver = CompileAndRunTestDriver::new(
        r"
    struct Point { x: i64, y: i64 }
    class Counter { n: i64 }

    func id[T](x: T) -> T { x }

    func pick[T](a: T, b: T, first: bool) -> T {
        if first { a } else { b }
    }

    public func ints() -> i64 { id(40) + pick(1, 2, false) }

    public func floats() -> f64 { pick(1.5, id(2.25), false) }

    public func bools() -> bool { pick(false, id(true), false) }

    public func points() -> i64 {
        let p = pick(Point { x: 1, y: 2 }, Point { x: 3, y: 4 }, true);
        id(p).x * 10 + p.y
    }

    public func counters() -> i64 {
        let a = Counter { n: 5 };
        let b = pick(a, Counter { n: 9 }, true);
        b.n = 7;
        a.n
    }

    public func arrays() -> i64 {
        let xs = pick([1, 2, 3], [4, 5], true);
        id(xs)[2]
    }

    public func generic_calls_generic[T](x: T) -> T { id(id(x)) }

    public func nested() -> i64 { generic_calls_generic(17) }
    ",
        |builder| builder,
    )
    .expect("Failed to build test driver");

    assert_invoke_eq!(i64, 42, driver, "ints");
    assert_invoke_eq!(f64, 2.25, driver, "floats");
    assert_invoke_eq!(bool, true, driver, "bools");
    assert_invoke_eq!(i64, 12, driver, "points");
    // A class is a reference: `pick` returns the same object, not a copy.
    assert_invoke_eq!(i64, 7, driver, "counters");
    assert_invoke_eq!(i64, 3, driver, "arrays");
    assert_invoke_eq!(i64, 17, driver, "nested");

    // A generic function has no code of its own, so the host cannot call
    // it, even when it is public: only its instances exist, and they are
    // internal to the module that uses them.
    let result: Result<i64, _> = driver.runtime.invoke("generic_calls_generic", (1i64,));
    assert!(result.is_err(), "a generic function must not be callable");
}

/// Generic structs and classes with methods, instantiated explicitly
/// (`Box[i32]`) and by inference, and nested (`Box[Pair[i64, f64]]`).
#[test]
fn generic_structs_and_classes() {
    let driver = CompileAndRunTestDriver::new(
        r"
    public struct Pair[A, B] { a: A, b: B }
    public class Box[T] { value: T }

    extend Box[T] {
        func get(self) -> T { self.value }
        func set(self, value: T) { self.value = value }
        func new(value: T) -> Box[T] { Box { value: value } }
    }

    extend Pair[A, B] {
        func swap(self) -> Pair[B, A] { Pair { a: self.b, b: self.a } }
    }

    public func explicit() -> i32 {
        let b: Box[i32] = Box { value: 41 };
        b.set(b.get() + 1);
        b.get()
    }

    public func inferred() -> f64 {
        let p = Pair { a: 1.5, b: 2 };
        let s = p.swap();
        s.b
    }

    public func nested() -> f64 {
        let b = Box.new(Pair { a: 3, b: 0.5 });
        let p: Pair[i64, f64] = b.get();
        p.b + (p.a as f64)
    }

    public func boxed_pair() -> Box[Pair[i64, f64]] {
        Box { value: Pair { a: 7, b: 0.25 } }
    }
    ",
        |builder| builder,
    )
    .expect("Failed to build test driver");

    assert_invoke_eq!(i32, 42, driver, "explicit");
    assert_invoke_eq!(f64, 1.5, driver, "inferred");
    assert_invoke_eq!(f64, 3.5, driver, "nested");

    // The instantiation is a runtime type of its own, with its fields read
    // through its arguments.
    let boxed: StructRef<'_> = driver
        .runtime
        .invoke("boxed_pair", ())
        .expect("Failed to call function");
    let pair: StructRef<'_> = boxed.get("value").expect("Failed to get struct field");
    assert_eq!(pair.get::<i64>("a").expect("Failed to get field a"), 7);
    assert_eq!(pair.get::<f64>("b").expect("Failed to get field b"), 0.25);
}

/// A generic function of another module is instantiated in the module that
/// calls it, and the private helper it calls -- which its own module does
/// not export -- comes along with it. The generic class it returns is
/// instantiated there too, and its generic method with it.
#[test]
fn generic_function_from_another_module() {
    let driver = CompileAndRunTestDriver::from_fixture(
        r#"
    //- /codira.toml
    [package]
    name="foo"
    version="0.0.0"

    //- /src/mod.code
    import root.util.tag
    import root.util.wrap

    public func main() -> i64 {
        let (v, h) = tag(20);
        wrap(v + h).get()
    }

    //- /src/util.code
    public class Box[T] { value: T }

    extend Box[T] {
        public func get(self) -> T { self.value }
    }

    public func wrap[T](x: T) -> Box[T] { Box { value: x } }

    func helper() -> i64 { 41 }

    public func tag[T](x: T) -> (T, i64) {
        (x, helper())
    }
    "#,
        |builder| builder,
    )
    .expect("Failed to build test driver");

    assert_invoke_eq!(i64, 61, driver, "main");
}

/// Changing the layout of a generic struct changes the GUID of each of its
/// instantiations, so hot reloading migrates the objects of `Box[i32]` to
/// the new layout exactly as it does for a non-generic struct.
#[test]
fn reloading_a_generic_struct_migrates_its_instantiations() {
    let mut driver = CompileAndRunTestDriver::new(
        r"
    public class Box[T] { value: T }

    public func make() -> Box[i32] {
        Box { value: 5 }
    }
    ",
        |builder| builder,
    )
    .expect("Failed to build test driver");

    let boxed: StructRef<'_> = driver
        .runtime
        .invoke("make", ())
        .expect("Failed to call function");
    assert_eq!(boxed.get::<i32>("value").expect("Failed to get field"), 5);
    let boxed = boxed.root();

    driver.update_file(
        "mod.code",
        r"
    public class Box[T] { tag: i64, value: T }

    public func make() -> Box[i32] {
        Box { tag: 1, value: 5 }
    }
    ",
    );

    let boxed = boxed.as_ref(&driver.runtime);
    assert_eq!(boxed.get::<i32>("value").expect("Failed to get field"), 5);
    assert_eq!(boxed.get::<i64>("tag").expect("Failed to get new field"), 0);
}
