//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 2, 2026
//!
//! Functionality:
//! - Tests the binding generator's renderers against hand-built exports.
//!
//! The expected outputs live in `expected/` as real files of the language
//! they are in, so each one can be handed to that language's compiler as it
//! stands. `scalars.ts` and `single.ts` were captured from the renderer as it
//! was before the other three outputs existed, except that `scalars.ts` now
//! escapes the backslash in its library path: the captured `"target\mod..."`
//! was the bug `ts_string_contents` fixes.

use codira_hir::{FloatTy, IntTy, Substitution, TyKind};

use super::*;

const ALL_SCALARS: [Scalar; 13] = [
    Scalar::Bool,
    Scalar::I8,
    Scalar::I16,
    Scalar::I32,
    Scalar::I64,
    Scalar::Isize,
    Scalar::U8,
    Scalar::U16,
    Scalar::U32,
    Scalar::U64,
    Scalar::Usize,
    Scalar::F32,
    Scalar::F64,
];

fn export(name: &str, params: &[Scalar], ret: Option<Scalar>) -> Export {
    Export {
        name: name.to_owned(),
        params: params.to_vec(),
        ret,
    }
}

/// Exports that bind under a name nobody objects to, with nothing to say
/// about them but their types.
fn plain(names: &[&str]) -> Exports {
    Exports {
        functions: names
            .iter()
            .map(|name| export(name, &[Scalar::I32], Some(Scalar::I32)))
            .collect(),
        skipped: Vec::new(),
    }
}

/// Every scalar in parameter position, a unit return, an empty parameter
/// list, and two functions no output can bind.
fn scalars() -> Exports {
    Exports {
        functions: vec![
            export("all_scalars", &ALL_SCALARS, None),
            export("no_params", &[], Some(Scalar::I32)),
            export("returns_bool", &[Scalar::U8], Some(Scalar::Bool)),
            export("returns_isize", &[], Some(Scalar::Isize)),
            export("unit_no_params", &[], None),
        ],
        skipped: vec![
            Skipped {
                name: "wide".to_owned(),
                unsupported: Unsupported::Return {
                    ty: "i128".to_owned(),
                },
            },
            Skipped {
                name: "takes_wide".to_owned(),
                unsupported: Unsupported::Param {
                    index: 1,
                    ty: "i128".to_owned(),
                },
            },
        ],
    }
}

/// One function per scalar, each taking and returning it, so a width or a
/// signedness that is wrong in one direction only still shows.
fn round_trips() -> Exports {
    Exports {
        functions: ALL_SCALARS
            .iter()
            .map(|scalar| {
                export(
                    &format!("echo_{}", rust_type(*scalar)),
                    &[*scalar],
                    Some(*scalar),
                )
            })
            .collect(),
        skipped: Vec::new(),
    }
}

/// Names that are fine in Codira and a problem somewhere else.
fn awkward_names() -> Exports {
    let mut exports = plain(&[
        "Codira_add",
        "GoString",
        "INT8_MAX",
        "_Atomic",
        "__declspec",
        "_private",
        "c",
        "class",
        "codira_add",
        "crate",
        "dup",
        "dup",
        "func",
        "i32",
        "int",
        "int32_t",
        "main",
        "match",
        "na\u{ef}ve",
        "plain",
        "size_t",
        "struct_stat",
        "try",
    ]);
    exports.skipped.push(Skipped {
        name: "comment".to_owned(),
        unsupported: Unsupported::Return {
            ty: "odd */ type\nname".to_owned(),
        },
    });
    exports
}

/// A fixture as the renderer should produce it: a checkout that rewrote the
/// line endings must not turn into a failure, and the renderer emitting a
/// carriage return must not turn into a pass.
fn expected(fixture: &str) -> String {
    fixture.replace("\r\n", "\n")
}

fn skipped_names(exports: &Exports, runtime: Runtime) -> Vec<String> {
    partition(exports, runtime)
        .1
        .into_iter()
        .map(|(name, _)| name)
        .collect()
}

fn bound_names(exports: &Exports, runtime: Runtime) -> Vec<&str> {
    partition(exports, runtime)
        .0
        .into_iter()
        .map(|export| export.name.as_str())
        .collect()
}

#[test]
fn bun_output_is_unchanged() {
    assert_eq!(
        render_bun(&scalars(), "target\\mod.codiralib"),
        expected(include_str!("expected/scalars.ts"))
    );
}

#[test]
fn bun_library_path_is_a_valid_string_literal() {
    let exports = Exports {
        functions: vec![export("no_params", &[], Some(Scalar::I32))],
        skipped: Vec::new(),
    };
    let rendered = render_bun(&exports, "C:\\lib\\\"odd\"\n\u{2028}.codiralib");
    assert!(
        rendered.contains(
            "export const LIBRARY_PATH = \"C:\\\\lib\\\\\\\"odd\\\"\\n\\u{2028}.codiralib\";"
        ),
        "{rendered}"
    );
}

#[test]
fn bun_output_is_unchanged_without_skips() {
    let exports = Exports {
        functions: vec![export("no_params", &[], Some(Scalar::I32))],
        skipped: Vec::new(),
    };
    assert_eq!(
        render_bun(&exports, "mod.codiralib"),
        expected(include_str!("expected/single.ts"))
    );
}

#[test]
fn bun_binds_every_scalar_signature_whatever_its_name() {
    let exports = awkward_names();
    let (bound, skipped) = partition(&exports, Runtime::Bun);
    assert_eq!(bound.len(), exports.functions.len());
    assert_eq!(
        skipped,
        [(
            "comment".to_owned(),
            "return type `odd */ type\nname` has no Bun FFI type".to_owned()
        )]
    );
}

#[test]
fn c_header_for_every_scalar() {
    assert_eq!(
        render_c(&scalars()),
        expected(include_str!("expected/scalars.h"))
    );
}

#[test]
fn c_header_round_trips() {
    assert_eq!(
        render_c(&round_trips()),
        expected(include_str!("expected/round_trips.h"))
    );
}

#[test]
fn c_header_for_awkward_names() {
    assert_eq!(
        render_c(&awkward_names()),
        expected(include_str!("expected/awkward_names.h"))
    );
}

#[test]
fn rust_block_for_every_scalar() {
    assert_eq!(
        render_rust(&scalars()),
        expected(include_str!("expected/scalars.rs"))
    );
}

#[test]
fn rust_block_round_trips() {
    assert_eq!(
        render_rust(&round_trips()),
        expected(include_str!("expected/round_trips.rs"))
    );
}

#[test]
fn rust_block_for_awkward_names() {
    assert_eq!(
        render_rust(&awkward_names()),
        expected(include_str!("expected/awkward_names.rs"))
    );
}

#[test]
fn go_file_for_every_scalar() {
    assert_eq!(
        render_go(&scalars()),
        expected(include_str!("expected/scalars.go"))
    );
}

#[test]
fn go_file_round_trips() {
    assert_eq!(
        render_go(&round_trips()),
        expected(include_str!("expected/round_trips.go"))
    );
}

#[test]
fn go_file_for_awkward_names() {
    assert_eq!(
        render_go(&awkward_names()),
        expected(include_str!("expected/awkward_names.go"))
    );
}

/// The mapping tables, written out a second time by hand. A snapshot shows
/// that the output did not change; this shows that it was right.
#[test]
fn scalar_types_are_exact_in_every_language() {
    // (scalar, C, Rust, cgo and Go)
    let table = [
        (Scalar::Bool, "bool", "bool", None),
        (Scalar::I8, "int8_t", "i8", Some(("C.int8_t", "int8"))),
        (Scalar::I16, "int16_t", "i16", Some(("C.int16_t", "int16"))),
        (Scalar::I32, "int32_t", "i32", Some(("C.int32_t", "int32"))),
        (Scalar::I64, "int64_t", "i64", Some(("C.int64_t", "int64"))),
        (Scalar::Isize, "intptr_t", "isize", None),
        (Scalar::U8, "uint8_t", "u8", Some(("C.uint8_t", "uint8"))),
        (
            Scalar::U16,
            "uint16_t",
            "u16",
            Some(("C.uint16_t", "uint16")),
        ),
        (
            Scalar::U32,
            "uint32_t",
            "u32",
            Some(("C.uint32_t", "uint32")),
        ),
        (
            Scalar::U64,
            "uint64_t",
            "u64",
            Some(("C.uint64_t", "uint64")),
        ),
        (
            Scalar::Usize,
            "size_t",
            "usize",
            Some(("C.size_t", "uintptr")),
        ),
        (Scalar::F32, "float", "f32", Some(("C.float", "float32"))),
        (Scalar::F64, "double", "f64", Some(("C.double", "float64"))),
    ];

    // The table has to cover the scalars for the loop below to mean anything.
    assert_eq!(table.map(|row| row.0), ALL_SCALARS);

    for (scalar, c, rust, go) in table {
        assert_eq!(c_type(scalar), c, "C type of {scalar:?}");
        assert_eq!(rust_type(scalar), rust, "Rust type of {scalar:?}");
        let binding = go_binding(scalar).ok().map(|b| (b.cgo, b.go));
        assert_eq!(binding, go, "Go types of {scalar:?}");
    }
}

#[test]
fn bun_types_are_unchanged() {
    let table: [(Option<Scalar>, &str, &str, bool); 14] = [
        (None, "void", "void", false),
        (Some(Scalar::Bool), "bool", "boolean", false),
        (Some(Scalar::I8), "i8", "number", false),
        (Some(Scalar::I16), "i16", "number", false),
        (Some(Scalar::I32), "i32", "number", false),
        (Some(Scalar::I64), "i64_fast", "number | bigint", false),
        (Some(Scalar::Isize), "ptr", "number", true),
        (Some(Scalar::U8), "u8", "number", false),
        (Some(Scalar::U16), "u16", "number", false),
        (Some(Scalar::U32), "u32", "number", false),
        (Some(Scalar::U64), "u64_fast", "number | bigint", false),
        (Some(Scalar::Usize), "ptr", "number", true),
        (Some(Scalar::F32), "f32", "number", false),
        (Some(Scalar::F64), "f64", "number", false),
    ];
    for (scalar, ffi, ts, caveat) in table {
        let binding = bun_binding(scalar);
        assert_eq!(binding.ffi, ffi, "FFIType of {scalar:?}");
        assert_eq!(binding.ts, ts, "TypeScript type of {scalar:?}");
        assert_eq!(binding.caveat.is_some(), caveat, "caveat of {scalar:?}");
    }
}

#[test]
fn codira_types_resolve_to_the_scalar_of_the_same_width_and_sign() {
    let table = [
        (TyKind::Bool, Scalar::Bool),
        (TyKind::Int(IntTy::i8()), Scalar::I8),
        (TyKind::Int(IntTy::i16()), Scalar::I16),
        (TyKind::Int(IntTy::i32()), Scalar::I32),
        (TyKind::Int(IntTy::i64()), Scalar::I64),
        (TyKind::Int(IntTy::isize()), Scalar::Isize),
        (TyKind::Int(IntTy::u8()), Scalar::U8),
        (TyKind::Int(IntTy::u16()), Scalar::U16),
        (TyKind::Int(IntTy::u32()), Scalar::U32),
        (TyKind::Int(IntTy::u64()), Scalar::U64),
        (TyKind::Int(IntTy::usize()), Scalar::Usize),
        (TyKind::Float(FloatTy::f32()), Scalar::F32),
        (TyKind::Float(FloatTy::f64()), Scalar::F64),
    ];
    let mut covered = Vec::new();
    for (kind, scalar) in table {
        assert_eq!(scalar_for(&kind), Some(scalar));
        covered.push(scalar);
    }
    assert_eq!(covered, ALL_SCALARS);
}

#[test]
fn codira_types_that_are_not_scalars_resolve_to_nothing() {
    for kind in [
        TyKind::Int(IntTy::i128()),
        TyKind::Int(IntTy::u128()),
        TyKind::Str,
        TyKind::Never,
        // The unit type is only meaningful as a return, and the caller
        // handles that case before asking.
        TyKind::Tuple(0, Substitution::empty()),
    ] {
        assert_eq!(scalar_for(&kind), None);
    }
}

#[test]
fn output_uses_unix_line_endings_and_ends_with_one() {
    for exports in [scalars(), round_trips(), awkward_names()] {
        for rendered in [
            render_bun(&exports, "mod.codiralib"),
            render_c(&exports),
            render_rust(&exports),
            render_go(&exports),
        ] {
            assert!(!rendered.contains('\r'));
            assert!(rendered.ends_with('\n'));
            assert!(!rendered.ends_with("\n\n"));
        }
    }
}

#[test]
fn c_skips_what_c_or_cpp_cannot_declare() {
    assert_eq!(
        skipped_names(&awkward_names(), Runtime::C),
        [
            "INT8_MAX",
            "_Atomic",
            "__declspec",
            "class",
            "comment",
            "dup",
            "dup",
            "int",
            "int32_t",
            "main",
            "na\u{ef}ve",
            "size_t",
            "try",
        ]
    );
    for keyword in C_KEYWORDS.iter().chain(CPP_KEYWORDS) {
        assert_eq!(skipped_names(&plain(&[keyword]), Runtime::C), [*keyword]);
    }
}

#[test]
fn rust_escapes_keywords_and_skips_the_ones_it_cannot_escape() {
    assert_eq!(
        skipped_names(&awkward_names(), Runtime::Rust),
        ["comment", "crate", "dup", "dup", "main", "na\u{ef}ve"]
    );
    for keyword in RUST_KEYWORDS {
        assert_eq!(rust_name(keyword), Ok(format!("r#{keyword}")));
    }
    for keyword in ["crate", "self", "super", "Self", "_"] {
        assert_eq!(skipped_names(&plain(&[keyword]), Runtime::Rust), [keyword]);
    }
    assert_eq!(rust_name("plain"), Ok("plain".to_owned()));
}

#[test]
fn go_skips_what_cgo_cannot_reach() {
    assert_eq!(
        skipped_names(&awkward_names(), Runtime::Go),
        [
            "Codira_add",
            "GoString",
            "INT8_MAX",
            "_Atomic",
            "__declspec",
            "_private",
            "c",
            "codira_add",
            "comment",
            "dup",
            "dup",
            "func",
            "int",
            "int32_t",
            "main",
            "na\u{ef}ve",
            "size_t",
            "struct_stat",
        ]
    );
    for keyword in C_KEYWORDS.iter().chain(GO_KEYWORDS).chain(CGO_NAMES) {
        assert_eq!(skipped_names(&plain(&[keyword]), Runtime::Go), [*keyword]);
    }
    // cgo compiles the preamble as C, so C++'s keywords are ordinary names.
    assert_eq!(
        bound_names(&plain(&["class", "try"]), Runtime::Go),
        ["class", "try"]
    );
}

#[test]
fn go_skips_the_scalars_it_has_no_exact_type_for() {
    let exports = Exports {
        functions: vec![
            export("takes_bool", &[Scalar::I32, Scalar::Bool], None),
            export("returns_bool", &[], Some(Scalar::Bool)),
            export("takes_isize", &[Scalar::Isize], None),
            export("returns_isize", &[], Some(Scalar::Isize)),
            export("fine", &[Scalar::Usize], Some(Scalar::F32)),
        ],
        skipped: Vec::new(),
    };
    assert_eq!(bound_names(&exports, Runtime::Go), ["fine"]);
    assert_eq!(partition(&exports, Runtime::C).0.len(), 5);
    assert_eq!(partition(&exports, Runtime::Rust).0.len(), 5);
}

#[test]
fn go_wrapper_names_capitalise_only_the_first_letter() {
    assert_eq!(go_wrapper_name("codira_add"), Ok("Codira_add".to_owned()));
    assert_eq!(go_wrapper_name("Already"), Ok("Already".to_owned()));
    assert_eq!(go_wrapper_name("x"), Ok("X".to_owned()));
    assert!(go_wrapper_name("_hidden").is_err());
    assert!(go_wrapper_name("").is_err());
}

#[test]
fn go_file_with_nothing_to_bind_has_no_unused_import() {
    let exports = Exports {
        functions: vec![export("returns_bool", &[], Some(Scalar::Bool))],
        skipped: Vec::new(),
    };
    let rendered = render_go(&exports);
    assert!(rendered.ends_with("package bindings\n"));
    assert!(!rendered.contains("import"));
}

#[test]
fn a_name_exported_twice_is_bound_by_nobody_but_bun() {
    let exports = plain(&["dup", "dup", "once"]);
    for runtime in [Runtime::C, Runtime::Rust, Runtime::Go] {
        assert_eq!(bound_names(&exports, runtime), ["once"]);
    }
    assert_eq!(bound_names(&exports, Runtime::Bun), ["dup", "dup", "once"]);
}

#[test]
fn skip_reasons_name_the_function_and_the_type() {
    let exports = scalars();
    assert_eq!(
        partition(&exports, Runtime::Bun).1,
        [
            (
                "takes_wide".to_owned(),
                "parameter 1 of type `i128` has no Bun FFI type".to_owned()
            ),
            (
                "wide".to_owned(),
                "return type `i128` has no Bun FFI type".to_owned()
            ),
        ]
    );
    for runtime in [Runtime::C, Runtime::Rust] {
        assert_eq!(
            partition(&exports, runtime).1,
            [
                (
                    "takes_wide".to_owned(),
                    "parameter 1 of type `i128` is not a scalar the C ABI can carry".to_owned()
                ),
                (
                    "wide".to_owned(),
                    "return type `i128` is not a scalar the C ABI can carry".to_owned()
                ),
            ]
        );
    }
}

#[test]
fn comments_cannot_be_ended_early() {
    assert_eq!(comment_text("odd */ type\nname"), "odd * / type name");
    assert_eq!(comment_text("a\r\nb\tc"), "a  b c");
    assert_eq!(comment_text("i128"), "i128");
}

#[test]
fn only_ascii_identifiers_are_bound_outside_bun() {
    for name in ["a", "_", "_a", "A9", "snake_case", "__x"] {
        assert!(is_ascii_identifier(name), "{name}");
    }
    for name in ["", "9a", "na\u{ef}ve", "a-b", "a b", "a.b", "\u{3b1}"] {
        assert!(!is_ascii_identifier(name), "{name}");
    }
}

/// The published FNV-1a test vectors. The include guard is built from this
/// hash, so a slip here would change every generated header.
#[test]
fn fnv1a64_matches_its_test_vectors() {
    assert_eq!(fnv1a64(b""), 0xcbf2_9ce4_8422_2325);
    assert_eq!(fnv1a64(b"a"), 0xaf63_dc4c_8601_ec8c);
    assert_eq!(fnv1a64(b"foobar"), 0x8594_4171_f739_67e8);
}

#[test]
fn include_guards_differ_exactly_when_the_declarations_do() {
    let guard = |exports: &Exports| {
        render_c(exports)
            .lines()
            .find(|line| line.starts_with("#ifndef "))
            .map(ToOwned::to_owned)
    };
    assert!(guard(&scalars()).is_some());
    assert_eq!(guard(&scalars()), guard(&scalars()));
    assert_ne!(guard(&scalars()), guard(&round_trips()));
}

#[test]
fn runtime_flags_are_the_ones_clap_accepts() {
    use clap::ValueEnum as _;

    for runtime in Runtime::value_variants() {
        let value = runtime.to_possible_value().map(|v| v.get_name().to_owned());
        assert_eq!(value.as_deref(), Some(runtime.flag()));
    }
    assert_eq!(Runtime::value_variants().len(), 4);
}

#[test]
fn library_is_refused_where_it_would_be_ignored() {
    for runtime in ["c", "rust", "go"] {
        let error = crate::run_with_args([
            "codira",
            "bindgen",
            "--runtime",
            runtime,
            "--library",
            "mod.codiralib",
        ])
        .expect_err("`--library` has no meaning outside Bun");
        assert!(error.to_string().contains("`--library` only applies"));
    }
}
