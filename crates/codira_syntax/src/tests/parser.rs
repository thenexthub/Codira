//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
//!
use crate::SourceFile;

#[test]
fn class_and_trait_and_extend() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        trait Shape {
            func area(self) -> f64;
            func perimeter(self) -> f64 { 0.0 }
        }

        class Animal {
            var name: i32
        }

        class Dog: Animal, Shape {
            var breed: i32
        }

        extend Dog {
            override func area(self) -> f64 { 0.0 }
        }

        extend Animal {
            func speak(self) -> i32 { 0 }
        }

        extend Animal: Shape {
            func perimeter(self) -> f64 { 1.0 }
        }
        "#,
    )
    .debug_dump());
}

#[test]
fn enum_and_match() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        enum Shape {
            Circle(radius: f64)
            Rectangle(width: f64, height: f64)
            Point
        }

        func area(shape: Shape) -> f64 {
            match shape {
                Shape.Circle(radius) -> radius,
                Shape.Rectangle(width, height) -> width,
                Shape.Point -> 0.0,
                42 -> 1.0,
                _ -> 0.0,
            }
        }
        "#,
    )
    .debug_dump());
}

#[test]
fn comptime_and_generics_and_where() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        comptime func fibonacci(n: i32) -> i32 {
            n
        }

        struct Box[T] {
            value: T
        }

        func max[T](a: T, b: T) -> T where T: Comparable {
            a
        }

        func use_it() -> i32 {
            comptime {
                fibonacci(10)
            }
        }
        "#,
    )
    .debug_dump());
}

#[test]
fn effects_and_handlers() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        effect Logger {
            func log(message: i32)
        }

        func risky() uses Logger -> i32 {
            perform Logger.log(1)
            42
        }

        func run() -> i32 {
            handle risky() {
                Logger.log(message) -> {
                    resume()
                }
            }
        }
        "#,
    )
    .debug_dump());
}

#[test]
fn attributes_and_extern_blocks() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        @derive(Show)
        @heal(on: 1, strategies: 2)
        struct Point {
            x: f64
        }

        extern "C" {
            func sqrt(x: f64) -> f64;
        }

        @export("C")
        public func codira_add(a: i32, b: i32) -> i32 { a }
        "#,
    )
    .debug_dump());
}

#[test]
fn optional_types_and_force_unwrap() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        func find(x: i32) -> i32? {
            nil
        }

        func run() -> i32 {
            let forced = find(1)!;
            forced
        }
        "#,
    )
    .debug_dump());
}

#[test]
fn var_bindings_and_macro_def() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        macro derive_show(target: i32) -> i32 {
            target
        }

        func counter() -> i32 {
            var count = 0;
            count += 1;
            count
        }
        "#,
    )
    .debug_dump());
}

#[test]
fn refinement_types_and_supervisor() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        type RecordId = i32 { x | x > 0 };

        func fetch_record(id: i32 { x | x > 0 }) -> RecordId {
            id
        }

        supervisor DatabaseConnection {
            strategy: 1,
            max_restarts: 5,

            child ConnPool {
                strategy: 2,
            }
        }
        "#,
    )
    .debug_dump());
}

/// Covers the ownership/hardware/concurrency scaffolding keywords in
/// spec/LANGUAGE_SPEC.md section 14: `consuming`/`borrowing` parameter
/// conventions, `~Trait` inheritance-list opt-out, `@target(..)` (which
/// reuses the pre-existing generic `@name(..)` attribute grammar), and
/// `spawn <expr>`.
#[test]
fn ownership_and_concurrency_scaffolding() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        struct GPUBuffer: ~Copyable {
            len: i32,
        }

        func consume_it(consuming buf: GPUBuffer) -> i32 {
            buf.len
        }

        func borrow_it(borrowing buf: GPUBuffer) -> i32 {
            buf.len
        }

        @target(gpu)
        func compute() -> i32 {
            spawn compute();
            42
        }
        "#,
    )
    .debug_dump());
}

/// Covers the remaining spec/LANGUAGE_SPEC.md section-14 scaffolding:
/// `def` (Python-style dynamic functions with untyped parameters),
/// `inout`/`mut`/`out` parameter conventions, postfix `x^` transfer,
/// `<-ch` channel receive, and `ch <- x` channel send.
#[test]
fn transfer_channels_and_def() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        def greet(name, count = 3) -> nil {
            nil
        }

        func swap(inout a: i32, inout b: i32) {
            let tmp = a^;
            a = b^;
            b = tmp;
        }

        func pipeline(ch: Channel[i32]) -> i32 {
            let got = <-ch;
            ch <- got + 1;
            got
        }
        "#,
    )
    .debug_dump());
}

#[test]
fn tuple_record() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        public struct Foo(public i32, i32);
        "#
    )
    .debug_dump());
}

#[test]
fn method_call() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        func main() {
            a.foo();
            a.0.foo();
            a.0.0.foo();
            a.0 .f32();
        }
        "#
    )
    .debug_dump());
}

#[test]
fn index_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main() {
        let a = [1,2,3,4]
        let b = a[0];
        let c = a[b];
        a[0] = c;
        let a = { [3,4,5] }[1];
    }"#,
    ).debug_dump());
}

#[test]
fn array_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main() {
        let a = [1,2,3,]
        let a = []
        let a = [call(123)]
        let a = [Struct { }, Struct { }]
    }"#,
    ).debug_dump());
}

#[test]
fn missing_field_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        value.
    }"#,
    ).debug_dump());
}

#[test]
fn impl_block() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        extend Foo {}
        extend Bar {
            func bar() {}
            struct Baz {}
        }
        public extend FooBar {}
        "#).debug_dump());
}

#[test]
fn array_type() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main(a: [int]) {
        let a:[[bool]];
    }"#,
    ).debug_dump());
}

#[test]
fn empty() {
    insta::assert_snapshot!(SourceFile::parse(r#""#).debug_dump(), @"SOURCE_FILE@0..0
");
}

#[test]
fn function() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    // Source file comment

    // Comment that belongs to the function
    func a() {}
    func b(value:number) {}
    public func d() {}
    public func c()->never {}
    func b(value:number)->number {}"#,
    ).debug_dump());
}

#[test]
fn block() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        let a;
        let b:i32;
        let c:string;
    }"#,
    ).debug_dump());
}

#[test]
fn literals() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        let a = true;
        let b = false;
        let c = 1;
        let d = 1.12;
        let e = "Hello, world!"
    }
    "#,
    ).debug_dump());
}

#[test]
fn struct_def() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    struct Foo      // error: expected a ';', or a '{'
    struct Foo;
    struct Foo;;    // error: expected a declaration
    struct Foo {}
    struct Foo {};
    struct Foo {,}; // error: expected a field declaration
    struct Foo {
        a: f64,
    }
    struct Foo {
        a: f64,
        b: i32,
    };
    struct Foo()
    struct Foo();
    struct Foo(,);  // error: expected a type
    struct Foo(f64)
    struct Foo(f64,);
    struct Foo(f64, i32)
    "#,
    ).debug_dump());
}

#[test]
fn unary_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        let a = --3;
        let b = !!true;
    }
    "#,
    ).debug_dump());
}

#[test]
fn binary_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        let a = 3+4*5
        let b = 3*4+10/2
    }
    "#,
    ).debug_dump());
}

#[test]
fn expression_statement() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        let a = "hello"
        let b = "world"
        let c
        b = "Hello, world!"
        !-5+2*(a+b);
        -3
    }
    "#,
    ).debug_dump());
}

#[test]
fn function_calls() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func bar(i:number) { }
    func foo(i:number) {
      bar(i+1)
    }
    func baz(self) { }
    func qux(self, i:number) { }
    func foo(self i:number) { } // error: expected comma
    "#,
    ).debug_dump());
}

#[test]
fn patterns() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main(_:number) {
       let a = 0;
       let _ = a;
    }
    "#,
    ).debug_dump());
}

#[test]
fn arithmetic_operands() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main() {
        let _ = a + b;
        let _ = a - b;
        let _ = a * b;
        let _ = a / b;
        let _ = a % b;
        let _ = a << b;
        let _ = a >> b;
        let _ = a & b;
        let _ = a | b;
        let _ = a ^ b;
    }
    "#,
    ).debug_dump());
}

#[test]
fn assignment_operands() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main() {
        let a = b;
        a += b;
        a -= b;
        a *= b;
        a /= b;
        a %= b;
        a <<= b;
        a >>= b;
        a &= b;
        a |= b;
        a ^= b;
    }
    "#,
    ).debug_dump());
}

#[test]
fn compare_operands() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main() {
        let _ = a == b;
        let _ = a == b;
        let _ = a != b;
        let _ = a < b;
        let _ = a > b;
        let _ = a <= b;
        let _ = a >= b;
    }
    "#,
    ).debug_dump());
}

#[test]
fn logic_operands() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main() {
        let _ = a || b;
        let _ = a && b;
    }
    "#,
    ).debug_dump());
}

#[test]
fn if_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func bar() {
        if true {};
        if true {} else {};
        if true {} else if false {} else {};
        if {true} {} else {}
    }
    "#,
    ).debug_dump());
}

#[test]
fn block_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func bar() {
        {3}
    }
    "#,
    ).debug_dump());
}

#[test]
fn return_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        return;
        return 50;
    }
    "#,
    ).debug_dump());
}

#[test]
fn loop_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        loop {}
    }"#,
    ).debug_dump());
}

#[test]
fn break_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        break;
        if break { 3; }
        if break 4 { 3; }
    }
    "#,
    ).debug_dump());
}

#[test]
fn while_expr() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        while true {};
        while { true } {};
    }
    "#,
    ).debug_dump());
}

#[test]
fn struct_lit() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func foo() {
        U;
        S {};
        S { x, y: 32, };
        S { x: 32, y: 64 };
        TupleStruct { 0: 1 };
        T(1.23);
        T(1.23, 4,)
    }
    "#,
    ).debug_dump());
}

#[test]
fn struct_field_index() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    func main() {
        foo.a
        foo.a.b
        foo.0
        foo.0.1
        foo.10
        foo.01  // index: .0
        foo.0 1 // index: .0 
        foo.a.0
    }
    "#,
    ).debug_dump());
}

#[test]
fn struct_and_class_memory_kind() {
    // The `(gc)`/`(value)` parenthetical annotation is gone: `struct` is
    // always a value type and `class` is always a heap-allocated,
    // garbage-collected reference type -- the keyword choice *is* the memory
    // kind now.
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    struct Foo {};
    class Baz {};
    struct Bar {};
    "#,
    ).debug_dump());
}

#[test]
fn visibility() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    public struct Foo {};
    internal class Baz {};
    internal func foo() {}
    internal func bar() {}
    public func baz() {}
    "#,
    ).debug_dump());
}

#[test]
fn extern_fn() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    public extern func foo();
    "#,
    ).debug_dump());
}

#[test]
fn type_alias_def() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
    type Foo = i32;
    type Bar = Foo;
    "#,
    ).debug_dump());
}

#[test]
fn function_return_path() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        func main() -> self.Foo {}
        func main1() -> super.Foo {}
        func main2() -> root.Foo {}
        func main3() -> root.foo.Foo {}
    "#,
    ).debug_dump());
}

#[test]
fn use_() {
    insta::assert_snapshot!(SourceFile::parse(
        r#"
        // Simple paths
        import package_name;
        import self.item_in_scope_or_package_name;
        import self.module.Item;
        import root.Item;
        import self.some.Struct;
        import root.some_item;

        // Use tree list
        import crate.{Item};
        import self.{Item};

        // Wildcard import
        import *; // Error
        import .*; // Error
        import crate.*;
        import crate.{*};

        // Renames
        import some.path as some_name;
        import some.{
            other.path as some_other_name,
            different.path as different_name,
            yet.another.path,
            running.out.of.synonyms.for_.different.*
        };
        import Foo as _;
        "#,
    ).debug_dump());
}

