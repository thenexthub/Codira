//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
#[macro_use]
mod util;

use std::{
    cell::Cell,
    thread::sleep,
    time::{Duration, Instant},
};

use codira_runtime::{Runtime, StructRef};
use codira_test::{CompileAndRunTestDriver, CompileTestDriver};

thread_local! {
    /// Whether `Runtime::update` reported a failed relink on this thread.
    static RELINK_FAILED: Cell<bool> = const { Cell::new(false) };
}

/// Observes the one thing `Runtime::update` says about a relink that failed.
///
/// `update` returns `false` both when the compiler has not written anything
/// yet and when the reload was attempted and rejected, so its return value
/// cannot tell a test that the rejection has happened. The error it logs can.
/// `update` logs on the thread that called it, which is the test's own, so a
/// thread-local keeps tests that run in parallel from seeing each other's
/// failures.
struct RelinkFailureLogger;

impl log::Log for RelinkFailureLogger {
    fn enabled(&self, metadata: &log::Metadata<'_>) -> bool {
        metadata.level() == log::Level::Error
    }

    fn log(&self, record: &log::Record<'_>) {
        if self.enabled(record.metadata())
            && record
                .args()
                .to_string()
                .starts_with("Failed to relink assemblies")
        {
            RELINK_FAILED.with(|failed| failed.set(true));
        }
    }

    fn flush(&self) {}
}

/// Calls `Runtime::update` until `is_done` holds for one of its results, the
/// way `CompileAndRunTestDriver::update_file` waits for a successful reload.
fn update_until(runtime: &mut Runtime, what: &str, mut is_done: impl FnMut(bool) -> bool) {
    let start_time = Instant::now();

    // Safety: We compiled the library ourselves, therefor updating the runtime is
    // safe.
    while !is_done(unsafe { runtime.update() }) {
        if start_time.elapsed() > Duration::from_secs(10) {
            panic!("runtime did not {what} after recompilation within 10 seconds");
        } else {
            sleep(Duration::from_millis(1));
        }
    }
}

#[test]
fn reloadable_function_single_file() {
    let mut driver = CompileAndRunTestDriver::new(
        r"
    public func main() -> i32 { 5 }
    ",
        |builder| builder,
    )
    .expect("Failed to build test driver");
    assert_invoke_eq!(i32, 5, driver, "main");

    driver.update_file(
        "mod.code",
        r"
    public func main() -> i32 { 10 }
    ",
    );
    assert_invoke_eq!(i32, 10, driver, "main");
}

#[test]
fn reloadable_function_multi_file() {
    let mut driver = CompileAndRunTestDriver::from_fixture(
        r#"
    //- /codira.toml
    [package]
    name="foo"
    version="0.0.0"

    //- /src/mod.code
    import root.foo.bar
    public func main() -> i32 { bar() }

    //- /src/foo.code
    public func bar() -> i32 { 5 }
    "#,
        |builder| builder,
    )
    .expect("Failed to build test driver");
    assert_invoke_eq!(i32, 5, driver, "main");

    driver.update_file(
        "foo.code",
        r#"
    public func bar() -> i32 { 10 }
    "#,
    );
    assert_invoke_eq!(i32, 10, driver, "main");
}

#[test]
fn reloadable_struct_decl_single_file() {
    let mut driver = CompileAndRunTestDriver::new(
        r#"
    public class Args {
        n: i32,
        foo: Bar,
    }
    
    class Bar {
        m: i32,
    }

    public func args() -> Args {
        Args { n: 3, foo: Bar { m: 1 }, }
    }
    "#,
        |builder| builder,
    )
    .expect("Failed to build test driver");

    let args: StructRef<'_> = driver
        .runtime
        .invoke("args", ())
        .expect("Failed to call function");

    let foo_struct: StructRef<'_> = args.get("foo").expect("Failed to get struct field");
    assert_eq!(
        foo_struct
            .get::<i32>("m")
            .expect("Failed to get struct field"),
        1
    );

    let foo_struct = foo_struct.root();

    driver.update_file(
        "mod.code",
        r#"
    public class Args {
        n: i32,
        foo: Bar,
    }
    
    class Bar {
        m: i64,
    }

    public func args() -> Args {
        Args { n: 3, foo: Bar { m: 1 }, }
    }
    "#,
    );

    let foo_struct = foo_struct.as_ref(&driver.runtime);
    assert_eq!(
        foo_struct
            .get::<i64>("m")
            .expect("Failed to get struct field"),
        1
    );
}

#[test]
fn reloadable_struct_decl_multi_file() {
    let mut driver = CompileAndRunTestDriver::from_fixture(
        r#"
    //- /codira.toml
    [package]
    name="foo"
    version="0.0.0"

    //- /src/mod.code
    import root.foo.Bar
    public class Args {
        n: i32,
        foo: Bar,
    }

    public func args() -> Args {
        Args { n: 3, foo: Bar { m: 1 }, }
    }

    //- /src/foo.code
    public class Bar {
        m: i64,
    }
    "#,
        |builder| builder,
    )
    .expect("Failed to build test driver");

    let args: StructRef<'_> = driver
        .runtime
        .invoke("args", ())
        .expect("Failed to call function");

    assert_eq!(args.get::<i32>("n").expect("Failed to get struct field"), 3);

    let foo_struct: StructRef<'_> = args.get("foo").expect("Failed to get struct field");
    assert_eq!(
        foo_struct
            .get::<i64>("m")
            .expect("Failed to get struct field"),
        1
    );

    let args = args.root();
    let foo_struct = foo_struct.root();

    driver.update_file(
        "mod.code",
        r#"
    import root.foo.Bar
    public class Args {
        n: i64,
        foo: Bar,
    }

    public func args() -> Args {
        Args { n: 3, foo: Bar { m: 1 }, }
    }
    "#,
    );

    let args = args.as_ref(&driver.runtime);
    assert_eq!(args.get::<i64>("n").expect("Failed to get struct field"), 3);

    let foo_struct = foo_struct.as_ref(&driver.runtime);
    assert_eq!(
        foo_struct
            .get::<i64>("m")
            .expect("Failed to get struct field"),
        1
    );
}

/// A reload that is rejected must leave the program exactly as it was.
///
/// The edit below changes the layout of `Bar` *and* calls a function nothing
/// defines. Relinking used to migrate the heap to the new layout before it
/// linked the functions, so the missing function was only discovered after
/// every live `Bar` had already been rewritten: the runtime kept the old code
/// and the old type table, but the objects they describe were gone.
#[test]
fn failed_reload_does_not_migrate_memory() {
    // The logger is process-wide and can only be installed once; another test
    // in this binary having done so already is fine.
    static LOGGER: RelinkFailureLogger = RelinkFailureLogger;
    if log::set_logger(&LOGGER).is_ok() {
        log::set_max_level(log::LevelFilter::Error);
    }

    // `CompileAndRunTestDriver::update_file` waits for a reload to *succeed*,
    // so the compiler and the runtime are driven separately here.
    let mut compiler = CompileTestDriver::from_file(
        r#"
    public class Args {
        n: i32,
        foo: Bar,
    }

    public class Bar {
        m: i32,
    }

    public func args() -> Args {
        Args { n: 3, foo: Bar { m: 7 }, }
    }

    public func bar_m(bar: Bar) -> i32 {
        bar.m
    }
    "#,
    );

    // Safety: We compiled the library ourselves, therefor loading the codiralib is
    // safe.
    let mut runtime =
        unsafe { Runtime::builder(compiler.lib_path()).finish() }.expect("Failed to build runtime");

    let args: StructRef<'_> = runtime.invoke("args", ()).expect("Failed to call function");
    let foo_struct: StructRef<'_> = args.get("foo").expect("Failed to get struct field");
    assert_eq!(
        foo_struct
            .get::<i32>("m")
            .expect("Failed to get struct field"),
        7
    );

    let args = args.root();
    let foo_struct = foo_struct.root();

    compiler.update_file(
        "mod.code",
        r#"
    extern func failed_reload_missing_dependency() -> i64;

    public class Args {
        n: i64,
        foo: Bar,
    }

    public class Bar {
        m: i64,
    }

    public func args() -> Args {
        Args { n: 3, foo: Bar { m: failed_reload_missing_dependency() }, }
    }

    public func bar_m(bar: Bar) -> i64 {
        bar.m
    }
    "#,
    );

    RELINK_FAILED.with(|failed| failed.set(false));
    update_until(&mut runtime, "reject the reload", |reloaded| {
        assert!(!reloaded, "an unlinkable assembly must not be reloaded");
        RELINK_FAILED.with(Cell::get)
    });

    // The objects still have the layout the old code was compiled against.
    {
        let args = args.as_ref(&runtime);
        assert_eq!(args.get::<i32>("n").expect("Failed to get struct field"), 3);

        let foo_struct = foo_struct.as_ref(&runtime);
        assert_eq!(
            foo_struct
                .get::<i32>("m")
                .expect("Failed to get struct field"),
            7
        );

        // And the old code, which is what is still loaded, can still read them.
        let m: i32 = runtime
            .invoke("bar_m", (foo_struct,))
            .expect("Failed to call function");
        assert_eq!(m, 7);
    }

    // Having refused the edit, the runtime must still accept the next one that
    // does link, and only then migrate the objects.
    compiler.update_file(
        "mod.code",
        r#"
    public class Args {
        n: i64,
        foo: Bar,
    }

    public class Bar {
        m: i64,
    }

    public func args() -> Args {
        Args { n: 3, foo: Bar { m: 7 }, }
    }

    public func bar_m(bar: Bar) -> i64 {
        bar.m
    }
    "#,
    );
    update_until(&mut runtime, "update", |reloaded| reloaded);

    let args = args.as_ref(&runtime);
    assert_eq!(args.get::<i64>("n").expect("Failed to get struct field"), 3);

    let foo_struct = foo_struct.as_ref(&runtime);
    assert_eq!(
        foo_struct
            .get::<i64>("m")
            .expect("Failed to get struct field"),
        7
    );

    let m: i64 = runtime
        .invoke("bar_m", (foo_struct,))
        .expect("Failed to call function");
    assert_eq!(m, 7);
}
