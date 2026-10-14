//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - End-to-end tests for function values: named functions used as values,
//!   closures and their by-value captures, calls through values stored in
//!   locals, fields and arrays, captured objects surviving a collection, and
//!   function values surviving a hot reload (see `codira_codegen::ir::closures`
//!   for the representation).
use codira_runtime::StructRef;
use codira_test::CompileAndRunTestDriver;

fn build(source: &str) -> CompileAndRunTestDriver {
    CompileAndRunTestDriver::new(source, |builder| builder).expect("Failed to build test driver")
}

/// A named function is a value: stored in a local, passed to a function,
/// returned from one, and called through each of those.
#[test]
fn named_functions_are_values() {
    let driver = build(
        r"
    func square(x: i32) -> i32 { x * x }
    func negate(x: i32) -> i32 { -x }

    func apply(f: func(i32) -> i32, x: i32) -> i32 { f(x) }
    func pick(negative: bool) -> func(i32) -> i32 {
        if negative { negate } else { square }
    }

    public func main(x: i32) -> i32 {
        let f = square
        var g = negate
        g = square
        f(3) + apply(negate, x) * 100 + pick(true)(1000) + g(2) * 10000
    }
    ",
    );
    // 9 - 700 - 1000 + 40000
    let result: i32 = driver.runtime.invoke("main", (7i32,)).unwrap();
    assert_eq!(result, 38309);
}

/// A closure captures by value when it is created: a later assignment to the
/// variable outside does not change what the closure sees.
#[test]
fn closures_capture_by_value_at_creation() {
    let driver = build(
        r"
    func adder(a: i32) -> func(i32) -> i32 {
        func(b: i32) -> i32 { a + b }
    }

    public func main() -> i32 {
        var x = 1
        let read_x = func() -> i32 { x }
        x = 5
        let add_ten = adder(10)
        read_x() * 1000 + x * 100 + add_ten(7)
    }
    ",
    );
    let result: i32 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(result, 1517);
}

/// The counter generator: state a closure changes lives in a class instance
/// it captured, so every call sees the previous one's update, and two
/// counters are independent.
#[test]
fn counter_generator_keeps_state_in_a_captured_object() {
    let driver = build(
        r"
    class Counter {
        n: i32,
    }

    func make_counter(start: i32) -> func() -> i32 {
        let state = Counter { n: start }
        func() -> i32 {
            state.n += 1
            state.n
        }
    }

    public func main() -> i32 {
        let c = make_counter(10)
        c()
        c()
        let d = make_counter(0)
        d()
        c() * 100 + d()
    }
    ",
    );
    let result: i32 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(result, 1302);
}

/// A callback stored in a class field is called like a method, and can be
/// replaced.
#[test]
fn callbacks_stored_in_a_class_field() {
    let driver = build(
        r"
    class Button {
        clicks: i32,
        on_click: func(i32) -> i32,
    }

    func press(button: Button) -> i32 {
        button.clicks += 1
        button.on_click(button.clicks)
    }

    public func main() -> i32 {
        let scale = 3
        let button = Button { clicks: 0, on_click: func(n: i32) -> i32 { n * scale } }
        let first = press(button)
        button.on_click = func(n: i32) -> i32 { n + 100 }
        let second = press(button)
        first * 1000 + second
    }
    ",
    );
    let result: i32 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(result, 3102);
}

/// Named functions and closures in one array, called in a loop.
#[test]
fn an_array_of_functions() {
    let driver = build(
        r"
    func double(x: i64) -> i64 { x * 2 }

    public func main() -> i64 {
        let offset: i64 = 100
        let steps = [double, func(x: i64) -> i64 { x + offset }, func(x) { x * x }]
        var value: i64 = 3
        for step in steps {
            value = step(value)
        }
        value + steps.len() as i64
    }
    ",
    );
    // ((3 * 2) + 100)^2 + 3
    let result: i64 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(result, 11239);
}

/// A closure's parameter types (and result) can come from the function type
/// it is passed as.
#[test]
fn closure_parameter_types_come_from_the_expected_function_type() {
    let driver = build(
        r"
    func apply_twice(f: func(i32) -> i32, x: i32) -> i32 { f(f(x)) }

    public func main(k: i32) -> i32 {
        apply_twice(func(x) { x * k }, 2)
    }
    ",
    );
    let result: i32 = driver.runtime.invoke("main", (5i32,)).unwrap();
    assert_eq!(result, 50);
}

/// Closures nest: an inner closure captures through the outer one, and a
/// method's closure captures `self`.
#[test]
fn nested_closures_and_captured_self() {
    let driver = build(
        r"
    class Account {
        balance: i64,
    }

    extend Account {
        func depositor(self) -> func(i64) -> i64 {
            func(amount: i64) -> i64 {
                self.balance += amount
                self.balance
            }
        }
    }

    public func main() -> i64 {
        let base: i64 = 1000
        let make = func(step: i64) -> func() -> i64 {
            func() -> i64 { base + step }
        }
        let account = Account { balance: 5 }
        let deposit = account.depositor()
        deposit(10)
        make(7)() + deposit(20)
    }
    ",
    );
    let result: i64 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(result, 1007 + 35);
}

/// The objects a closure captured are reachable through it: a collection
/// keeps them, and the closure still reads them afterwards.
#[test]
fn captured_objects_survive_a_collection() {
    let driver = build(
        r"
    class Counter {
        n: i64,
    }

    public class Holder {
        f: func() -> i64,
    }

    public func make() -> Holder {
        let counter = Counter { n: 41 }
        let values = [1, 2, 3]
        Holder { f: func() -> i64 { counter.n += 1; counter.n * 10 + values[2] } }
    }

    public func garbage(n: i64) -> i64 {
        var i: i64 = 0
        var total: i64 = 0
        while i < n {
            let c = Counter { n: i }
            total += c.n
            i += 1
        }
        total
    }

    public func call(holder: Holder) -> i64 {
        holder.f()
    }
    ",
    );
    let runtime = &driver.runtime;
    let holder: StructRef<'_> = runtime.invoke("make", ()).unwrap();
    let holder = holder.root();

    let before = runtime.gc_stats().allocated_memory;
    assert!(!runtime.gc_collect(), "nothing is unreachable yet");
    assert_eq!(runtime.gc_stats().allocated_memory, before);

    // Reuse any memory that was wrongly freed.
    let _: i64 = runtime.invoke("garbage", (1000i64,)).unwrap();
    assert!(runtime.gc_collect(), "the garbage counters are collected");
    let _: i64 = runtime.invoke("garbage", (1000i64,)).unwrap();

    let first: i64 = runtime.invoke("call", (holder.as_ref(runtime),)).unwrap();
    let second: i64 = runtime.invoke("call", (holder.as_ref(runtime),)).unwrap();
    assert_eq!((first, second), (423, 433));

    drop(holder);
    assert!(runtime.gc_collect());
    assert_eq!(runtime.gc_stats().allocated_memory, 0);
}

/// A function value created before a hot reload calls the reloaded code of
/// the same closure (and the same named function) afterwards, with the
/// captures it was created with: calls go through the runtime's cell, not
/// into the unloaded library.
#[test]
fn function_values_call_the_reloaded_code() {
    let mut driver = build(
        r"
    func step(x: i32) -> i32 { x + 1 }

    public class Holder {
        f: func(i32) -> i32,
        g: func(i32) -> i32,
    }

    public func make() -> Holder {
        let k = 10
        Holder { f: func(x: i32) -> i32 { x + k }, g: step }
    }

    public func call(holder: Holder, x: i32) -> i32 {
        holder.f(x) * 1000 + holder.g(x)
    }
    ",
    );
    let holder: StructRef<'_> = driver.runtime.invoke("make", ()).unwrap();
    let holder = holder.root();
    let before: i32 = driver
        .runtime
        .invoke("call", (holder.as_ref(&driver.runtime), 2i32))
        .unwrap();
    assert_eq!(before, 12003);

    driver.update_file(
        "mod.code",
        r"
    func step(x: i32) -> i32 { x + 5 }

    public class Holder {
        f: func(i32) -> i32,
        g: func(i32) -> i32,
    }

    public func make() -> Holder {
        let k = 10
        Holder { f: func(x: i32) -> i32 { x * k }, g: step }
    }

    public func call(holder: Holder, x: i32) -> i32 {
        holder.f(x) * 1000 + holder.g(x)
    }
    ",
    );
    let after: i32 = driver
        .runtime
        .invoke("call", (holder.as_ref(&driver.runtime), 2i32))
        .unwrap();
    assert_eq!(after, 20007);
}

/// A value struct is captured as a copy; `return` leaves the closure, and a
/// closure without a result can print.
#[test]
fn value_structs_early_return_and_printing() {
    let driver = build(
        r#"
    struct Point {
        x: i32,
        y: i32,
    }

    public func main() -> i32 {
        var p = Point { x: 1, y: 2 }
        let f = func() -> i32 { p.x * 10 + p.y }
        p.x = 7
        let clamp = func(x: i32) -> i32 {
            if x > 10 { return 10 }
            x
        }
        let say = func(x: i32) { println("x = ", x) }
        say(clamp(42))
        f() * 100 + p.x + clamp(3) * 10000
    }
    "#,
    );
    let (result, output) =
        codira_runtime::capture_output(|| -> i32 { driver.runtime.invoke("main", ()).unwrap() });
    assert_eq!(output, "x = 10\n");
    assert_eq!(result, 31207);
}

/// Function values cross module boundaries both ways: a named function of
/// another module becomes a value here (its thunk calls it through the
/// dispatch table), and a closure created there is called here.
#[test]
fn function_values_across_modules() {
    let driver = CompileAndRunTestDriver::from_fixture(
        r#"
    //- /codira.toml
    [package]
    name="foo"
    version="0.0.0"

    //- /src/mod.code
    import root.foo.{inc, twice, adder}
    public func main() -> i32 {
        twice(inc, 5) * 100 + adder(10)(1)
    }

    //- /src/foo.code
    public func inc(x: i32) -> i32 { x + 1 }
    public func twice(f: func(i32) -> i32, x: i32) -> i32 { f(f(x)) }
    public func adder(k: i32) -> func(i32) -> i32 {
        func(x: i32) -> i32 { x + k }
    }
    "#,
        |builder| builder,
    )
    .expect("Failed to build test driver");
    let result: i32 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(result, 711);
}
