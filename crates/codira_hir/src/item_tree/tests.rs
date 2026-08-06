//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
//!
use std::fmt;

use codira_hir_input::WithFixture;

use crate::{mock::MockDatabase, DefDatabase, DiagnosticSink};

fn print_item_tree(text: &str) -> Result<String, fmt::Error> {
    let (db, file_id) = MockDatabase::with_single_file(text);
    let item_tree = db.item_tree(file_id);
    let mut result_str = super::pretty::print_item_tree(&db, &item_tree)?;
    let mut sink = DiagnosticSink::new(|diag| {
        result_str.push_str(&format!(
            "\n{:?}: {}",
            diag.highlight_range(),
            diag.message()
        ));
    });

    item_tree
        .diagnostics
        .iter()
        .for_each(|diag| diag.add_to(&db, &item_tree, &mut sink));

    drop(sink);
    Ok(result_str)
}

#[test]
fn top_level_items() {
    insta::assert_snapshot!(print_item_tree(
        r#"
    func foo(a:i32, b:u8, c:String) -> i32 {}
    public func bar(a:i32, b:u8, c:String) ->  {}
    internal func bar(a:i32, b:u8, c:String) ->  {}
    internal func baz(a:i32, b:, c:String) ->  {}
    extern func eval(a:String) -> bool;

    struct Foo {
        a: i32,
        b: u8,
        c: String,
    }
    struct Foo2 {
        a: i32,
        b: ,
        c: String,
    }
    struct Bar (i32, u32, String)
    struct Baz;

    type FooBar = Foo;
    type FooBar = root.Foo;
    "#
    )
    .unwrap());
}

#[test]
fn test_use() {
    insta::assert_snapshot!(print_item_tree(
        r#"
    public import foo
    import super.bar
    import super.*
    import foo.{bar as _, baz.hello as world}
        "#
    )
    .unwrap());
}

#[test]
fn test_impls() {
    insta::assert_snapshot!(print_item_tree(
        r#"
    extend Bar {
        func foo(a:i32, b:u8, c:String) -> i32 {}
        public func bar(a:i32, b:u8, c:String) ->  {}
    }
    "#
    )
    .unwrap());
}

#[test]
fn test_duplicate_import() {
    insta::assert_snapshot!(print_item_tree(
        r#"
    import foo.Bar
    import baz.Bar

    struct Bar {}
    "#
    )
    .unwrap());
}

