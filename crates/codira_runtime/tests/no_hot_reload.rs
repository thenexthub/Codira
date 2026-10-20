//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 19, 2026
//!
//! Functionality:
//! - `--no-hot-reload`: all modules of a package form one library, so calls
//!   between modules are direct, and the program computes the same results.
use codira_test::CompileAndRunTestDriver;

const FIXTURE: &str = r#"
    //- /codira.toml
    [package]
    name="foo"
    version="0.0.0"

    //- /src/mod.code
    import root.foo.bar
    import root.baz.twice
    public func main() -> i32 { twice(bar()) + 1 }

    //- /src/foo.code
    public func bar() -> i32 { 5 }

    //- /src/baz.code
    import root.foo.bar
    public func twice(x: i32) -> i32 { x * 2 + bar() }
"#;

fn libraries(driver: &CompileAndRunTestDriver) -> usize {
    std::fs::read_dir(driver.lib_path().parent().unwrap())
        .unwrap()
        .filter(|e| {
            e.as_ref()
                .unwrap()
                .path()
                .extension()
                .is_some_and(|x| x == "codiralib")
        })
        .count()
}

#[test]
fn modules_are_separate_libraries_by_default() {
    let driver = CompileAndRunTestDriver::from_fixture(FIXTURE, |b| b).unwrap();
    assert_eq!(libraries(&driver), 3);
    let r: i32 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(r, 16);
}

#[test]
fn without_hot_reload_the_package_is_one_library_with_the_same_result() {
    let driver =
        CompileAndRunTestDriver::from_fixture_with_driver(FIXTURE, |d| d.set_no_hot_reload(true))
            .unwrap();
    assert_eq!(libraries(&driver), 1);
    let r: i32 = driver.runtime.invoke("main", ()).unwrap();
    assert_eq!(r, 16);
}
