//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Functions the compiler provides itself, with no declaration in any source
//!   file: `print` and `println`.
//!
//! # Why these are built in rather than library functions
//!
//! `println("x = ", x)` takes any number of arguments of any printable type.
//! Codira has neither variadic parameters nor overloading yet, so no
//! declaration written in Codira can have that signature -- the standard
//! library cannot provide it, only the compiler can. (The same is true of
//! Swift's `print(_ items: Any...)` and of Rust's `println!`, which is a
//! macro for exactly this reason.)
//!
//! # Resolution
//!
//! A builtin is what a bare call to its name means *when nothing in scope
//! defines that name*. Any user definition -- a local, a function, an
//! import -- wins, so existing code that declares its own `print` (the
//! standard library's testing suite does) keeps calling its own.
//!
//! # Lowering
//!
//! Inference records each builtin call in
//! [`InferenceResult::builtin_call`](crate::InferenceResult::builtin_call);
//! the code generator lowers every argument to one call to a runtime
//! function chosen by the argument's type (`codira.print_str`,
//! `codira.print_i64`, ...), which the runtime registers in its dispatch
//! table alongside `new` and `new_array`.

use crate::name::Name;

/// A function provided by the compiler rather than declared in source.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum BuiltinFn {
    /// `print(a, b, ..)` writes each argument in turn to standard output,
    /// with no separator and no trailing newline.
    Print,
    /// `println(a, b, ..)` is `print` followed by a newline. With no
    /// arguments it writes just the newline.
    Println,
    /// `declassify(e)` returns `e` unchanged and marks it public for the
    /// information-flow checker (`verify::flow`). No code of its own.
    Declassify,
    /// `ct_select(c, a, b)` is `if c { a } else { b }` evaluated without a
    /// branch: both operands are computed and code generation emits one
    /// LLVM `select` marked `!unpredictable`. The way to choose on a
    /// `secret` in a constant-time function.
    CtSelect,
}

impl BuiltinFn {
    /// Every builtin, for lookup by name.
    const ALL: &'static [BuiltinFn] = &[
        BuiltinFn::Print,
        BuiltinFn::Println,
        BuiltinFn::Declassify,
        BuiltinFn::CtSelect,
    ];

    /// The name a program calls this builtin by.
    pub fn name(self) -> &'static str {
        match self {
            BuiltinFn::Print => "print",
            BuiltinFn::Println => "println",
            BuiltinFn::Declassify => "declassify",
            BuiltinFn::CtSelect => "ct_select",
        }
    }

    /// The builtin called `name`, if there is one.
    pub fn from_name(name: &Name) -> Option<BuiltinFn> {
        let text = name.as_str()?;
        Self::ALL.iter().copied().find(|it| it.name() == text)
    }
}
