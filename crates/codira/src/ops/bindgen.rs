//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: September 17, 2026
//!
//! Functionality:
//! - Generates FFI bindings for a Codira library from its `@export("C")`
//!   signatures.

use std::{
    collections::BTreeMap,
    fmt::Write as _,
    path::{Path, PathBuf},
};

use anyhow::anyhow;
use codira_compiler::{Config, Driver, Target};
use codira_hir::{FloatBitness, IntBitness, Package, Signedness, TyKind};
use codira_project::MANIFEST_FILENAME;

use crate::{ops::build::find_manifest, ExitStatus};

#[derive(Copy, Clone, PartialEq, Eq, clap::ValueEnum)]
pub enum Runtime {
    /// Bun, through its built-in `bun:ffi`.
    Bun,
    /// A C header of prototypes, also usable from C++.
    C,
    /// A Rust `unsafe extern "C"` block.
    Rust,
    /// Go, through cgo.
    Go,
}

impl Runtime {
    /// The spelling `--runtime` accepts for this output, for messages that
    /// name it back to the user.
    const fn flag(self) -> &'static str {
        match self {
            Runtime::Bun => "bun",
            Runtime::C => "c",
            Runtime::Rust => "rust",
            Runtime::Go => "go",
        }
    }
}

#[derive(clap::Args)]
pub struct Args {
    /// Path to the manifest of the project
    #[clap(long)]
    manifest_path: Option<PathBuf>,

    /// Runtime or language to generate bindings for
    #[clap(long, value_enum, default_value = "bun")]
    runtime: Runtime,

    /// Where to write the bindings. Defaults to stdout.
    #[clap(long, short = 'o')]
    output: Option<PathBuf>,

    /// Path the generated bindings should `dlopen` at runtime. Defaults to
    /// the assembly this project builds, relative to the output file. Only
    /// `--runtime bun` loads the library itself, so only it accepts this.
    #[clap(long)]
    library: Option<String>,
}

/// A type that crosses the C ABI as a single scalar.
///
/// This is the whole vocabulary of a binding: every output maps these and
/// nothing else, so a type missing here is unbound everywhere rather than
/// guessed at by one language.
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum Scalar {
    Bool,
    I8,
    I16,
    I32,
    I64,
    Isize,
    U8,
    U16,
    U32,
    U64,
    Usize,
    F32,
    F64,
}

/// Maps a Codira type to the scalar that carries it across the C ABI.
fn scalar_for(kind: &TyKind) -> Option<Scalar> {
    Some(match kind {
        TyKind::Bool => Scalar::Bool,

        TyKind::Float(float) => match float.bitness {
            FloatBitness::X32 => Scalar::F32,
            FloatBitness::X64 => Scalar::F64,
        },

        TyKind::Int(int) => {
            let signed = int.signedness == Signedness::Signed;
            match (int.bitness, signed) {
                (IntBitness::X8, true) => Scalar::I8,
                (IntBitness::X8, false) => Scalar::U8,
                (IntBitness::X16, true) => Scalar::I16,
                (IntBitness::X16, false) => Scalar::U16,
                (IntBitness::X32, true) => Scalar::I32,
                (IntBitness::X32, false) => Scalar::U32,
                (IntBitness::X64, true) => Scalar::I64,
                (IntBitness::X64, false) => Scalar::U64,
                (IntBitness::Xsize, true) => Scalar::Isize,
                (IntBitness::Xsize, false) => Scalar::Usize,
                // No portable C ABI for a 128-bit integer, and Bun has no
                // type for one.
                (IntBitness::X128, _) => return None,
            }
        }

        _ => return None,
    })
}

/// A single exported function, resolved down to what a binding needs.
struct Export {
    name: String,
    params: Vec<Scalar>,
    /// `None` is the unit type: a valid return, with no value to carry.
    ret: Option<Scalar>,
}

/// The part of a signature that no output can bind.
enum Unsupported {
    Return { ty: String },
    Param { index: usize, ty: String },
}

impl Unsupported {
    /// Says what is unsupported; `lacking` finishes the sentence in the
    /// terms of the output being generated.
    fn reason(&self, lacking: &str) -> String {
        match self {
            Unsupported::Return { ty } => format!("return type `{ty}` {lacking}"),
            Unsupported::Param { index, ty } => {
                format!("parameter {index} of type `{ty}` {lacking}")
            }
        }
    }
}

/// An exported function whose signature is not made of scalars.
struct Skipped {
    name: String,
    unsupported: Unsupported,
}

/// Everything a project exports, independent of who will call it.
#[derive(Default)]
struct Exports {
    functions: Vec<Export>,
    skipped: Vec<Skipped>,
}

pub fn bindgen(args: Args) -> Result<ExitStatus, anyhow::Error> {
    // Refused rather than ignored: a path that silently does nothing reads
    // as a path that was honoured.
    if args.library.is_some() && args.runtime != Runtime::Bun {
        return Err(anyhow!(
            "`--library` only applies to `--runtime bun`: `--runtime {}` generates declarations \
             and leaves loading or linking the library to the caller",
            args.runtime.flag()
        ));
    }

    let manifest_path = match &args.manifest_path {
        None => {
            let current_dir =
                std::env::current_dir().expect("could not determine current working directory");
            find_manifest(&current_dir).ok_or_else(|| {
                anyhow!(
                    "could not find {} in '{}' or a parent directory",
                    MANIFEST_FILENAME,
                    current_dir.display()
                )
            })?
        }
        Some(path) => std::fs::canonicalize(Path::new(&path)).map_err(|_error| {
            anyhow!(
                "'{}' does not refer to a valid manifest path",
                path.display()
            )
        })?,
    };

    let config = Config {
        target: Target::host_target().expect("unable to determine host target"),
        // Bindings are derived from signatures, which optimization does not
        // change; this only has to type-check, not codegen.
        optimization_lvl: codira_compiler::OptimizationLevel::None,
        mode: codira_compiler::BuildMode::default(),
        out_dir: None,
        emit_ir: false,
    };

    let (_package, driver) = Driver::with_package_path(&manifest_path, config)?;
    let db = driver.db();

    let mut exports = Exports::default();
    let mut library_path = None;

    for package in Package::all(db) {
        for module in package.modules(db) {
            if library_path.is_none() {
                library_path = Some(driver.assembly_output_path(module));
            }

            for function in module.all_functions(db) {
                if function.export_abi(db).as_deref() != Some("C") {
                    continue;
                }

                let name = function.name(db).to_string();
                let ret_ty = function.ret_type(db);
                let ret = match ret_ty.interned() {
                    // The unit type is a valid *return*; in parameter
                    // position it falls through to "unsupported" below.
                    TyKind::Tuple(0, _) => None,
                    kind => {
                        let Some(scalar) = scalar_for(kind) else {
                            exports.skipped.push(Skipped {
                                name,
                                unsupported: Unsupported::Return {
                                    ty: format!("{ret_ty:?}"),
                                },
                            });
                            continue;
                        };
                        Some(scalar)
                    }
                };

                let mut params = Vec::new();
                let mut unsupported = None;
                for param in function.params(db) {
                    let ty = param.ty();
                    let Some(scalar) = scalar_for(ty.interned()) else {
                        // This includes a parameter of unit type, which has
                        // no C representation to pass.
                        unsupported = Some(Unsupported::Param {
                            index: param.index(),
                            ty: format!("{ty:?}"),
                        });
                        break;
                    };
                    params.push(scalar);
                }

                match unsupported {
                    Some(unsupported) => exports.skipped.push(Skipped { name, unsupported }),
                    None => exports.functions.push(Export { name, params, ret }),
                }
            }
        }
    }

    // A stable order keeps the generated file diffable; HIR iteration order
    // is not guaranteed to be.
    exports.functions.sort_by(|a, b| a.name.cmp(&b.name));

    if exports.functions.is_empty() {
        return Err(anyhow!(
            "no `@export(\"C\")` functions found in this project -- nothing to generate \
             bindings for"
        ));
    }

    let (bound, skipped) = partition(&exports, args.runtime);

    // Scalars alone do not make a function bindable everywhere: a name can be
    // a keyword in one language, and Go has no spelling for some types. An
    // empty file would look like success, so this is an error instead.
    if bound.is_empty() {
        for (name, reason) in &skipped {
            eprintln!("warning: skipped `{name}`: {reason}");
        }
        return Err(anyhow!(
            "none of this project's `@export(\"C\")` functions can be bound for `--runtime {}`",
            args.runtime.flag()
        ));
    }

    let rendered = match args.runtime {
        Runtime::Bun => {
            let default_library = library_path.as_deref().map_or_else(
                || "library.codiralib".to_owned(),
                |path| path.display().to_string(),
            );
            render_bun(&exports, &args.library.unwrap_or(default_library))
        }
        Runtime::C => render_c(&exports),
        Runtime::Rust => render_rust(&exports),
        Runtime::Go => render_go(&exports),
    };

    match &args.output {
        Some(path) => {
            std::fs::write(path, &rendered)
                .map_err(|e| anyhow!("could not write '{}': {e}", path.display()))?;
            eprintln!(
                "wrote {} binding{} to {}",
                bound.len(),
                if bound.len() == 1 { "" } else { "s" },
                path.display()
            );
        }
        None => print!("{rendered}"),
    }

    for (name, reason) in &skipped {
        eprintln!("warning: skipped `{name}`: {reason}");
    }

    Ok(ExitStatus::Success)
}

/// Splits what a project exports into the functions `runtime` can bind and
/// the ones it cannot, each of the latter with its reason.
///
/// Every renderer goes through this, and so does the command for its
/// warnings, which is what keeps the file and the terminal telling the same
/// story about what was left out.
fn partition(exports: &Exports, runtime: Runtime) -> (Vec<&Export>, Vec<(String, String)>) {
    let lacking = match runtime {
        Runtime::Bun => "has no Bun FFI type",
        Runtime::C | Runtime::Rust | Runtime::Go => "is not a scalar the C ABI can carry",
    };

    let mut skipped: Vec<(String, String)> = exports
        .skipped
        .iter()
        .map(|s| (s.name.clone(), s.unsupported.reason(lacking)))
        .collect();

    // Two declarations of one name are either redundant or contradictory,
    // and nothing here can tell which symbol the linker would pick. Go has a
    // second way to collide: two exports that differ only in the case of
    // their first letter get the same wrapper name.
    let mut symbols: BTreeMap<&str, usize> = BTreeMap::new();
    let mut go_names: BTreeMap<String, usize> = BTreeMap::new();
    for function in &exports.functions {
        *symbols.entry(function.name.as_str()).or_default() += 1;
        if let Ok(go_name) = go_wrapper_name(&function.name) {
            *go_names.entry(go_name).or_default() += 1;
        }
    }

    let mut bound = Vec::new();
    for function in &exports.functions {
        let duplicated = runtime != Runtime::Bun && symbols[function.name.as_str()] > 1;
        let obstacle = if duplicated {
            Some(
                "more than one `@export(\"C\")` function has this name, and a binding can only \
                 name one symbol"
                    .to_owned(),
            )
        } else {
            match runtime {
                // Bun binds every scalar signature; its output predates the
                // other three and is kept exactly as it was.
                Runtime::Bun => None,
                Runtime::C => c_obstacle(function, true),
                Runtime::Rust => rust_name(&function.name).err(),
                Runtime::Go => go_obstacle(function, &go_names),
            }
        };

        match obstacle {
            Some(reason) => skipped.push((function.name.clone(), reason)),
            None => bound.push(function),
        }
    }

    // Sorted for the same reason the functions are: a diffable file.
    skipped.sort();
    (bound, skipped)
}

/// Whether `name` is spelled with nothing but ASCII letters, digits and
/// underscores, starting with a letter or an underscore.
///
/// Codira identifiers may be any Unicode `XID` sequence. C, Rust's `extern`
/// blocks and cgo do not agree on what to do with one, so the three of them
/// bind only the spelling they all accept.
fn is_ascii_identifier(name: &str) -> bool {
    let mut chars = name.chars();
    chars
        .next()
        .is_some_and(|c| c.is_ascii_alphabetic() || c == '_')
        && chars.all(|c| c.is_ascii_alphanumeric() || c == '_')
}

/// The reason a name cannot be declared by any of the C-family outputs, if
/// there is one.
fn common_name_obstacle(name: &str) -> Option<String> {
    if !is_ascii_identifier(name) {
        return Some("its name is not an ASCII identifier".to_owned());
    }
    // Declaring it would collide with the entry point of the program doing
    // the calling, in all three languages.
    if name == "main" {
        return Some("`main` is the entry point of the program that would call it".to_owned());
    }
    None
}

/// Makes text safe to place inside a generated comment.
///
/// The text is a type printed by the compiler, or a name: neither is
/// expected to contain a line break or a comment terminator, but a comment
/// that ends early turns the rest of it into code, so this does not rely on
/// expectation.
fn comment_text(text: &str) -> String {
    text.replace(char::is_control, " ").replace("*/", "* /")
}

/// 64-bit FNV-1a.
///
/// Written out because the header's include guard is derived from it and
/// must not change between compiler releases, which `std`'s hasher is free
/// to do.
fn fnv1a64(bytes: &[u8]) -> u64 {
    let mut hash: u64 = 0xcbf2_9ce4_8422_2325;
    for byte in bytes {
        hash ^= u64::from(*byte);
        hash = hash.wrapping_mul(0x0000_0100_0000_01b3);
    }
    hash
}

/// Escapes `text` for the inside of a double-quoted TypeScript string literal.
///
/// Backslashes are escaped rather than swapped for forward slashes: the
/// generated file is data for `dlopen`, and rewriting a Windows path into a
/// shape the platform did not give us is the kind of helpfulness that breaks
/// on a directory with an unusual name.
///
/// Every library path goes through this, the one given with `--library`
/// included. Only the default path used to be escaped, so `--library
/// target\mod.codiralib` was written out as `"target\mod.codiralib"`, which
/// JavaScript reads as `targetmod.codiralib`: `\m` is not an escape, so the
/// backslash silently disappears.
fn ts_string_contents(text: &str) -> String {
    let mut out = String::with_capacity(text.len());
    for c in text.chars() {
        match c {
            '\\' => out.push_str("\\\\"),
            '"' => out.push_str("\\\""),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            // U+2028 and U+2029 end a string literal in JavaScript before
            // ES2019, and every other control character is unreadable.
            c if c.is_control() || c == '\u{2028}' || c == '\u{2029}' => {
                let _ = write!(out, "\\u{{{:x}}}", u32::from(c));
            }
            c => out.push(c),
        }
    }
    out
}

/// How one scalar crosses into a Bun FFI signature.
struct Binding {
    /// The `FFIType` member naming this type on the JavaScript side.
    ffi: &'static str,
    /// The TypeScript type a value of it arrives as.
    ts: &'static str,
    /// Set when the chosen `FFIType` is a correct but slow way to pass this
    /// type, with the reason.
    caveat: Option<&'static str>,
}

impl Binding {
    const fn fast(ffi: &'static str, ts: &'static str) -> Self {
        Binding {
            ffi,
            ts,
            caveat: None,
        }
    }
}

/// Maps a scalar to the `bun:ffi` type that carries it; `None` is a unit
/// return.
///
/// # Why 64-bit integers map to `i64_fast`, not `i64`
///
/// `FFIType.i64` hands JavaScript a `BigInt`, which is a heap allocation on
/// every single call. That one choice dominates the cost of calling a Codira
/// function from Bun: measured over 500,000 calls of an identical function,
/// `i64` costs 37.97 ns/call against 7.02 ns/call for `u32` -- the 31 ns
/// difference is `BigInt` boxing and nothing else.
///
/// `i64_fast` returns a plain `number` while the value fits in a double's
/// integer range and a `BigInt` beyond it, which measured 12.35 ns/call. It
/// is not a precision trade: round-tripping 0, 2^53-1, 2^53+1, `i64::MAX` and
/// `i64::MIN` through it is lossless -- it switches representation exactly at
/// the point a `number` would stop being exact. The only cost is that the
/// TypeScript type is `number | bigint` rather than one or the other.
///
/// So the generated bindings are ~3x faster than hand-written `i64` ones by
/// default, and a signature written in `i32`/`u32`/`f64` where the range
/// allows is ~5x faster again. That ordering is the whole reason this
/// generator exists rather than a documentation note: the fast spelling and
/// the obvious spelling are not the same, and picking by hand is how the
/// slow one gets picked.
fn bun_binding(scalar: Option<Scalar>) -> Binding {
    let Some(scalar) = scalar else {
        return Binding::fast("void", "void");
    };
    match scalar {
        Scalar::Bool => Binding::fast("bool", "boolean"),
        Scalar::F32 => Binding::fast("f32", "number"),
        Scalar::F64 => Binding::fast("f64", "number"),
        Scalar::I8 => Binding::fast("i8", "number"),
        Scalar::U8 => Binding::fast("u8", "number"),
        Scalar::I16 => Binding::fast("i16", "number"),
        Scalar::U16 => Binding::fast("u16", "number"),
        Scalar::I32 => Binding::fast("i32", "number"),
        Scalar::U32 => Binding::fast("u32", "number"),
        // See this function's doc comment: `*_fast` is both faster and
        // lossless, so it is the default rather than an opt-in.
        Scalar::I64 => Binding::fast("i64_fast", "number | bigint"),
        Scalar::U64 => Binding::fast("u64_fast", "number | bigint"),
        // `usize`/`isize` bind as `ptr`, not as Bun's `usize`/`isize`.
        //
        // Bun's own pointer-width types go through BigInt unconditionally --
        // there is no `_fast` spelling for them -- so they are both the slow
        // option and an awkward one: the caller would have to write
        // `BigInt(ptr(buffer))`. `FFIType.ptr` is the same 64 bits and is
        // what `ptr()` already returns, as a plain `number`.
        //
        // This is a judgement about what a pointer-width integer is doing in
        // a C ABI signature: it is an address. A Codira function that takes
        // `(src, dst, count)` to process a caller's buffer is the reason the
        // intrinsics exist, and binding it any other way makes the fast path
        // the inconvenient one. A `count` passed this way is carried exactly
        // too -- verified against both addresses and lengths.
        Scalar::Isize | Scalar::Usize => Binding {
            ffi: "ptr",
            ts: "number",
            caveat: Some(
                "bound as FFIType.ptr: pass an address from ptr(buffer), or a plain length. Exact \
                 below 2^53, which covers every address and every buffer that fits in memory",
            ),
        },
    }
}

fn render_bun(exports: &Exports, library: &str) -> String {
    let (bound, skipped) = partition(exports, Runtime::Bun);
    let mut out = String::new();

    out.push_str(
        "// Generated by `codira bindgen --runtime bun`. Do not edit.\n\
         //\n\
         // Every `@export(\"C\")` function in the project appears below with the\n\
         // `FFIType` that actually carries its Codira signature, so the types are\n\
         // the ones the compiler resolved rather than the ones a caller guessed.\n\
         //\n\
         // On speed: 64-bit integers bind as `i64_fast`/`u64_fast`, which return a\n\
         // plain `number` inside the safe-integer range and a `BigInt` outside it.\n\
         // That is lossless and roughly 3x faster than `FFIType.i64`, which boxes a\n\
         // BigInt on every call (measured over 500,000 calls: 12.35 ns/call against\n\
         // 37.97 ns/call). A signature written in `i32`/`u32`/`f64` binds at about\n\
         // 7 ns/call, so prefer those in Codira where the range allows.\n\n",
    );

    if !skipped.is_empty() {
        out.push_str("// Not bound, because these have no Bun FFI representation:\n");
        for (name, reason) in &skipped {
            let _ = writeln!(out, "//   {name}: {reason}");
        }
        out.push('\n');
    }

    out.push_str("import { dlopen, FFIType } from \"bun:ffi\";\n\n");
    let _ = writeln!(
        out,
        "export const LIBRARY_PATH = \"{}\";\n",
        ts_string_contents(library)
    );

    out.push_str("export function load(path: string = LIBRARY_PATH) {\n");
    out.push_str("  return dlopen(path, {\n");
    for export in &bound {
        let args = export
            .params
            .iter()
            .map(|p| format!("FFIType.{}", bun_binding(Some(*p)).ffi))
            .collect::<Vec<_>>()
            .join(", ");
        let _ = writeln!(
            out,
            "    {}: {{ args: [{args}], returns: FFIType.{} }},",
            export.name,
            bun_binding(export.ret).ffi
        );
    }
    out.push_str("  }).symbols;\n}\n\n");

    // The declared shape is what makes the caller's types survive; `dlopen`
    // on its own infers `unknown` for anything it cannot see statically.
    out.push_str("export interface Symbols {\n");
    for export in &bound {
        if let Some(caveat) = export
            .params
            .iter()
            .map(|p| Some(*p))
            .chain(std::iter::once(export.ret))
            .find_map(|scalar| bun_binding(scalar).caveat)
        {
            let _ = writeln!(out, "  /** {caveat} */");
        }
        let params = export
            .params
            .iter()
            .enumerate()
            .map(|(i, p)| format!("a{i}: {}", bun_binding(Some(*p)).ts))
            .collect::<Vec<_>>()
            .join(", ");
        let _ = writeln!(
            out,
            "  {}({params}): {};",
            export.name,
            bun_binding(export.ret).ts
        );
    }
    out.push_str("}\n\n");

    out.push_str("export const symbols = load() as unknown as Symbols;\n");
    out
}

/// The C type that is exactly this scalar: same width, same signedness, on
/// every target.
///
/// Nothing here is `int` or `long`. Those are the types whose width depends
/// on who is compiling, and a prototype that is right on one platform and
/// four bytes off on another is the failure this generator exists to remove.
///
/// `usize` is `size_t` and `isize` is `intptr_t`. Codira defines both as the
/// width of a pointer, and every target it compiles for has a flat address
/// space, where `size_t` is that width too.
const fn c_type(scalar: Scalar) -> &'static str {
    match scalar {
        Scalar::Bool => "bool",
        Scalar::I8 => "int8_t",
        Scalar::I16 => "int16_t",
        Scalar::I32 => "int32_t",
        Scalar::I64 => "int64_t",
        Scalar::Isize => "intptr_t",
        Scalar::U8 => "uint8_t",
        Scalar::U16 => "uint16_t",
        Scalar::U32 => "uint32_t",
        Scalar::U64 => "uint64_t",
        Scalar::Usize => "size_t",
        Scalar::F32 => "float",
        Scalar::F64 => "double",
    }
}

/// Keywords of C through C23, with the two GNU C treats as keywords in its
/// default mode (`asm`, `typeof`). The `_Capitalised` ones are covered by
/// the reserved-prefix rule in [`c_obstacle`] instead.
const C_KEYWORDS: &[&str] = &[
    "alignas",
    "alignof",
    "asm",
    "auto",
    "bool",
    "break",
    "case",
    "char",
    "const",
    "constexpr",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "false",
    "float",
    "for",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "nullptr",
    "register",
    "restrict",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "struct",
    "switch",
    "thread_local",
    "true",
    "typedef",
    "typeof",
    "typeof_unqual",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
];

/// Keywords and alternative tokens of C++ through C++23 that are not already
/// C keywords. A header has to survive being included from C++ as well.
const CPP_KEYWORDS: &[&str] = &[
    "and",
    "and_eq",
    "bitand",
    "bitor",
    "catch",
    "char16_t",
    "char32_t",
    "char8_t",
    "class",
    "co_await",
    "co_return",
    "co_yield",
    "compl",
    "concept",
    "const_cast",
    "consteval",
    "constinit",
    "decltype",
    "delete",
    "dynamic_cast",
    "explicit",
    "export",
    "friend",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "not_eq",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "reinterpret_cast",
    "requires",
    "static_cast",
    "template",
    "this",
    "throw",
    "try",
    "typeid",
    "typename",
    "using",
    "virtual",
    "wchar_t",
    "xor",
    "xor_eq",
];

/// Names `<stddef.h>` and `<stdint.h>` define that the patterns in
/// [`c_obstacle`] do not already cover. (`<stdbool.h>` only defines names
/// that are keywords elsewhere, or reserved.)
const C_HEADER_NAMES: &[&str] = &[
    "NULL",
    "PTRDIFF_MAX",
    "PTRDIFF_MIN",
    "PTRDIFF_WIDTH",
    "SIG_ATOMIC_MAX",
    "SIG_ATOMIC_MIN",
    "SIG_ATOMIC_WIDTH",
    "SIZE_MAX",
    "SIZE_WIDTH",
    "WCHAR_MAX",
    "WCHAR_MIN",
    "WCHAR_WIDTH",
    "WINT_MAX",
    "WINT_MIN",
    "WINT_WIDTH",
    "max_align_t",
    "nullptr_t",
    "offsetof",
    "ptrdiff_t",
    "size_t",
    "unreachable",
    "wchar_t",
];

/// The reason a function cannot be declared in C, if there is one. `cpp`
/// adds the names only C++ reserves, for a declaration C++ will also read.
fn c_obstacle(export: &Export, cpp: bool) -> Option<String> {
    let name = export.name.as_str();
    if let Some(reason) = common_name_obstacle(name) {
        return Some(reason);
    }
    if C_KEYWORDS.contains(&name) {
        return Some("its name is a C keyword".to_owned());
    }
    if cpp && CPP_KEYWORDS.contains(&name) {
        return Some(
            "its name is a C++ keyword, and the header has to compile as C++ too".to_owned(),
        );
    }
    // C reserves these two shapes for the implementation for *any* use
    // (C17 7.1.3), which is where `_Bool`, `__declspec`, `__attribute__` and
    // the compilers' other private keywords live.
    let mut chars = name.chars();
    if chars.next() == Some('_')
        && chars
            .next()
            .is_some_and(|c| c == '_' || c.is_ascii_uppercase())
    {
        return Some(
            "its name begins with `__` or `_` and a capital, which C reserves for the compiler"
                .to_owned(),
        );
    }
    // `<stdint.h>` owns these two patterns outright, including names it has
    // not defined yet (C17 7.31.10).
    let stdint_type = (name.starts_with("int") || name.starts_with("uint")) && name.ends_with("_t");
    let stdint_macro = (name.starts_with("INT") || name.starts_with("UINT"))
        && ["_MAX", "_MIN", "_WIDTH", "_C"]
            .iter()
            .any(|suffix| name.ends_with(suffix));
    if stdint_type || stdint_macro || C_HEADER_NAMES.contains(&name) {
        return Some("its name belongs to a standard header the declarations depend on".to_owned());
    }
    None
}

/// The prototype of one function, without parameter names.
///
/// A parameter name in a header is one more identifier a macro in the
/// including file can rewrite; the types are the contract, and they need no
/// names.
fn c_prototype(export: &Export) -> String {
    let params = if export.params.is_empty() {
        // `()` in C before C23 declares a function taking *unspecified*
        // arguments, which switches argument checking off.
        "void".to_owned()
    } else {
        export
            .params
            .iter()
            .map(|p| c_type(*p))
            .collect::<Vec<_>>()
            .join(", ")
    };
    format!(
        "{} {}({params});",
        export.ret.map_or("void", c_type),
        export.name
    )
}

fn render_c(exports: &Exports) -> String {
    let (bound, skipped) = partition(exports, Runtime::C);

    let mut prototypes = String::new();
    for export in &bound {
        let _ = writeln!(prototypes, "{}", c_prototype(export));
    }

    // The guard is derived from the declarations rather than fixed. Two
    // Codira libraries bound into one translation unit would otherwise share
    // a guard, the second header would be skipped whole, and a call to an
    // undeclared function is at best an error and at worst (C89) an assumed
    // `int` return. Identical declarations still share a guard, which is
    // exactly the case a guard is for.
    let guard = format!("CODIRA_BINDINGS_{:016X}_H", fnv1a64(prototypes.as_bytes()));

    let mut out = String::new();
    out.push_str(
        "/* Generated by `codira bindgen --runtime c`. Do not edit.\n \
         *\n \
         * Every `@export(\"C\")` function in the project is declared below with the\n \
         * fixed-width type that matches its Codira signature, so the types are the\n \
         * ones the compiler resolved rather than the ones a caller guessed.\n \
         *\n \
         * `usize` is `size_t` and `isize` is `intptr_t`.\n \
         *\n \
         * These are declarations only: link against the library the project builds.\n \
         * A program that loads it at runtime instead declares function pointers of\n \
         * these same types.\n \
         */\n\n",
    );

    if !skipped.is_empty() {
        out.push_str("/* Not declared, with the reason for each:\n");
        for (name, reason) in &skipped {
            let _ = writeln!(out, " *   {}: {}", comment_text(name), comment_text(reason));
        }
        out.push_str(" */\n\n");
    }

    let _ = writeln!(out, "#ifndef {guard}\n#define {guard}\n");
    out.push_str("#include <stdbool.h>\n#include <stddef.h>\n#include <stdint.h>\n\n");
    // Without this a C++ caller would look for mangled names that the
    // library does not export.
    out.push_str("#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");
    out.push_str(&prototypes);
    out.push_str("\n#ifdef __cplusplus\n}\n#endif\n\n");
    let _ = writeln!(out, "#endif /* {guard} */");
    out
}

/// The Rust type that is exactly this scalar. Rust's primitive integers are
/// fixed-width by definition, and its `bool` is specified to match C's.
const fn rust_type(scalar: Scalar) -> &'static str {
    match scalar {
        Scalar::Bool => "bool",
        Scalar::I8 => "i8",
        Scalar::I16 => "i16",
        Scalar::I32 => "i32",
        Scalar::I64 => "i64",
        Scalar::Isize => "isize",
        Scalar::U8 => "u8",
        Scalar::U16 => "u16",
        Scalar::U32 => "u32",
        Scalar::U64 => "u64",
        Scalar::Usize => "usize",
        Scalar::F32 => "f32",
        Scalar::F64 => "f64",
    }
}

/// Strict and reserved keywords of every Rust edition through 2024, apart
/// from the four in [`rust_name`] that cannot be written as raw identifiers.
const RUST_KEYWORDS: &[&str] = &[
    "abstract", "as", "async", "await", "become", "box", "break", "const", "continue", "do", "dyn",
    "else", "enum", "extern", "false", "final", "fn", "for", "gen", "if", "impl", "in", "let",
    "loop", "macro", "match", "mod", "move", "mut", "override", "priv", "pub", "ref", "return",
    "static", "struct", "trait", "true", "try", "type", "typeof", "unsafe", "unsized", "use",
    "virtual", "where", "while", "yield",
];

/// How a function is named in a Rust declaration, or the reason it cannot be
/// declared.
///
/// A keyword becomes a raw identifier: `r#match` declares, and links
/// against, the symbol `match`. That is a real escape, unlike renaming,
/// which would declare a symbol the library does not have.
fn rust_name(name: &str) -> Result<String, String> {
    if let Some(reason) = common_name_obstacle(name) {
        return Err(reason);
    }
    if ["crate", "self", "super", "Self"].contains(&name) {
        return Err(
            "its name is a Rust keyword that cannot be written as a raw identifier".to_owned(),
        );
    }
    // `_` on its own is a pattern in Rust, not a name.
    if name == "_" {
        return Err("`_` is not an identifier in Rust".to_owned());
    }
    if RUST_KEYWORDS.contains(&name) {
        return Ok(format!("r#{name}"));
    }
    Ok(name.to_owned())
}

fn render_rust(exports: &Exports) -> String {
    let (bound, skipped) = partition(exports, Runtime::Rust);
    let mut out = String::new();

    out.push_str(
        "// Generated by `codira bindgen --runtime rust`. Do not edit.\n\
         //\n\
         // Every `@export(\"C\")` function in the project is declared below with the\n\
         // Rust type that matches its Codira signature, so the types are the ones\n\
         // the compiler resolved rather than the ones a caller guessed.\n\
         //\n\
         // The block is spelled `unsafe extern`, which edition 2024 requires and\n\
         // every edition accepts from Rust 1.82. Calling these is `unsafe`: the\n\
         // compiler cannot check a foreign signature, only trust this file for it.\n\
         //\n\
         // These are declarations only: link against the library the project\n\
         // builds, from a build script or a `#[link]` attribute.\n\n",
    );

    // One declaration per line, whatever its length, is what keeps this file
    // diffable; without the attribute rustfmt would rewrap it in the caller's
    // tree. The list of what was left out sits inside the block so that the
    // attribute covers it as well.
    out.push_str("#[rustfmt::skip]\nunsafe extern \"C\" {\n");

    if !skipped.is_empty() {
        out.push_str("    // Not declared, with the reason for each:\n");
        for (name, reason) in &skipped {
            let _ = writeln!(
                out,
                "    //   {}: {}",
                comment_text(name),
                comment_text(reason)
            );
        }
        if !bound.is_empty() {
            out.push('\n');
        }
    }

    for export in &bound {
        // `partition` only lets through names this succeeds for.
        let Ok(name) = rust_name(&export.name) else {
            continue;
        };
        let params = export
            .params
            .iter()
            .enumerate()
            .map(|(i, p)| format!("a{i}: {}", rust_type(*p)))
            .collect::<Vec<_>>()
            .join(", ");
        let ret = export
            .ret
            .map_or_else(String::new, |ret| format!(" -> {}", rust_type(ret)));
        let _ = writeln!(out, "    pub fn {name}({params}){ret};");
    }
    out.push_str("}\n");
    out
}

/// How one scalar is spelled on both sides of a cgo call.
struct GoBinding {
    /// The cgo name of the C type the prototype declares.
    cgo: &'static str,
    /// The Go type the wrapper takes or returns.
    go: &'static str,
}

/// Maps a scalar to its cgo and Go types, or to the reason Go leaves it
/// unbound.
///
/// Every pair here is an explicit conversion between two types of the same
/// width and signedness, which is what makes the wrappers lossless. The two
/// refusals are the scalars for which that could not be established.
fn go_binding(scalar: Scalar) -> Result<GoBinding, &'static str> {
    let (cgo, go) = match scalar {
        Scalar::I8 => ("C.int8_t", "int8"),
        Scalar::I16 => ("C.int16_t", "int16"),
        Scalar::I32 => ("C.int32_t", "int32"),
        Scalar::I64 => ("C.int64_t", "int64"),
        Scalar::U8 => ("C.uint8_t", "uint8"),
        Scalar::U16 => ("C.uint16_t", "uint16"),
        Scalar::U32 => ("C.uint32_t", "uint32"),
        Scalar::U64 => ("C.uint64_t", "uint64"),
        // `uintptr`, not `uint`: Go only promises pointer width of the
        // former, and pointer width is what Codira's `usize` is.
        Scalar::Usize => ("C.size_t", "uintptr"),
        Scalar::F32 => ("C.float", "float32"),
        Scalar::F64 => ("C.double", "float64"),
        // cgo's documentation lists the C numeric types it translates and
        // `bool` is not among them. It works in practice, but this generator
        // was not checked against a Go toolchain, and "in practice" is not
        // the standard a binding is held to.
        Scalar::Bool => {
            return Err(
                "`bool` is not bound for Go: cgo does not document how it converts C's \
                        `bool`",
            )
        }
        // Go's signed integers are fixed-width or `int`, and `int` is not
        // promised to be the width of a pointer.
        Scalar::Isize => {
            return Err(
                "`isize` is not bound for Go: Go has no signed integer type that is \
                        guaranteed to be pointer-sized",
            )
        }
    };
    Ok(GoBinding { cgo, go })
}

/// Go's keywords. `C.<name>` has to parse as a Go selector, and a keyword
/// after the dot does not.
const GO_KEYWORDS: &[&str] = &[
    "break",
    "case",
    "chan",
    "const",
    "continue",
    "default",
    "defer",
    "else",
    "fallthrough",
    "for",
    "func",
    "go",
    "goto",
    "if",
    "import",
    "interface",
    "map",
    "package",
    "range",
    "return",
    "select",
    "struct",
    "switch",
    "type",
    "var",
];

/// Names cgo gives a meaning of its own inside the `C` pseudo-package: its
/// helper functions, and its spellings of the C types whose own names are
/// more than one word (`C.uint` is `unsigned int`).
const CGO_NAMES: &[&str] = &[
    "CBytes",
    "CString",
    "GoBytes",
    "GoString",
    "GoStringN",
    "complexdouble",
    "complexfloat",
    "longlong",
    "malloc",
    "schar",
    "uchar",
    "uint",
    "ulong",
    "ulonglong",
    "ushort",
];

/// Prefixes cgo reads as "the struct, union or enum of this name" and "the
/// size of this type" rather than as an identifier.
const CGO_PREFIXES: &[&str] = &["enum_", "sizeof_", "struct_", "union_"];

/// The name of the Go wrapper for a function: its own name with the first
/// letter capitalised, because that is the only way Go exports anything.
///
/// The rest of the name is left alone. Rewriting `snake_case` into Go's
/// preferred style would make two Codira names able to collide, and would
/// make the wrapper harder to match to the export it calls.
fn go_wrapper_name(name: &str) -> Result<String, String> {
    let mut chars = name.chars();
    match chars.next() {
        Some(first) if first.is_ascii_alphabetic() => {
            Ok(format!("{}{}", first.to_ascii_uppercase(), chars.as_str()))
        }
        _ => Err(
            "its name does not begin with a letter, so Go cannot export a wrapper for it"
                .to_owned(),
        ),
    }
}

/// The reason a function cannot be bound for Go, if there is one.
/// `go_names` counts how many exports each wrapper name would be given to.
fn go_obstacle(export: &Export, go_names: &BTreeMap<String, usize>) -> Option<String> {
    let name = export.name.as_str();
    // The preamble is compiled as C, so what C cannot declare, cgo cannot
    // either. C++'s extra keywords do not apply: cgo never reads it as C++.
    if let Some(reason) = c_obstacle(export, false) {
        return Some(reason);
    }
    if GO_KEYWORDS.contains(&name) {
        return Some("its name is a Go keyword, which cannot follow `C.`".to_owned());
    }
    if CGO_NAMES.contains(&name) || CGO_PREFIXES.iter().any(|prefix| name.starts_with(prefix)) {
        return Some("cgo gives its name a meaning of its own inside `C.`".to_owned());
    }
    let go_name = match go_wrapper_name(name) {
        Ok(go_name) => go_name,
        Err(reason) => return Some(reason),
    };
    // The wrapper would shadow the `C` pseudo-package it has to call through.
    if go_name == "C" {
        return Some("its Go wrapper would be named `C`, which is the cgo import".to_owned());
    }
    if go_names.get(&go_name).copied().unwrap_or(0) > 1 {
        return Some(format!(
            "another export would also be given the Go name `{go_name}`"
        ));
    }
    export
        .params
        .iter()
        .chain(export.ret.iter())
        .find_map(|scalar| go_binding(*scalar).err())
        .map(ToOwned::to_owned)
}

fn render_go(exports: &Exports) -> String {
    let (bound, skipped) = partition(exports, Runtime::Go);
    let mut out = String::new();

    // The first line is the form Go's tooling recognises as generated code.
    out.push_str(
        "// Code generated by `codira bindgen --runtime go`. DO NOT EDIT.\n\
         //\n\
         // Every `@export(\"C\")` function in the project is wrapped below, with the\n\
         // fixed-width Go type that matches its Codira signature, so the types are\n\
         // the ones the compiler resolved rather than the ones a caller guessed.\n\
         //\n\
         // Each wrapper is the export's own name with its first letter capitalised,\n\
         // which is what makes it visible outside this package.\n\
         //\n\
         // These are declarations only: tell cgo how to link the library the\n\
         // project builds, with a `#cgo LDFLAGS` line in another file of this\n\
         // package or with `CGO_LDFLAGS`.\n\n",
    );

    if !skipped.is_empty() {
        out.push_str("// Not bound, with the reason for each:\n");
        for (name, reason) in &skipped {
            let _ = writeln!(out, "//   {}: {}", comment_text(name), comment_text(reason));
        }
        out.push('\n');
    }

    out.push_str("package bindings\n");

    // An `import "C"` nothing refers to is an unused import, so a file with
    // nothing to bind stops at the package clause.
    if bound.is_empty() {
        return out;
    }

    // The comment directly above `import "C"` is the preamble cgo compiles
    // as C; a blank line between the two would make it an ordinary comment.
    out.push_str("\n/*\n#include <stddef.h>\n#include <stdint.h>\n\n");
    for export in &bound {
        let _ = writeln!(out, "{}", c_prototype(export));
    }
    out.push_str("*/\nimport \"C\"\n");

    for export in &bound {
        // `partition` only lets through functions all of this succeeds for.
        let Ok(go_name) = go_wrapper_name(&export.name) else {
            continue;
        };
        let Ok(params) = export
            .params
            .iter()
            .map(|p| go_binding(*p))
            .collect::<Result<Vec<_>, _>>()
        else {
            continue;
        };
        let Ok(ret) = export.ret.map(go_binding).transpose() else {
            continue;
        };

        let signature = params
            .iter()
            .enumerate()
            .map(|(i, p)| format!("a{i} {}", p.go))
            .collect::<Vec<_>>()
            .join(", ");
        let arguments = params
            .iter()
            .enumerate()
            .map(|(i, p)| format!("{}(a{i})", p.cgo))
            .collect::<Vec<_>>()
            .join(", ");
        let call = format!("C.{}({arguments})", export.name);

        let _ = writeln!(
            out,
            "\n// {go_name} calls the Codira export `{}`.",
            export.name
        );
        match ret {
            Some(ret) => {
                let _ = writeln!(
                    out,
                    "func {go_name}({signature}) {} {{\n\treturn {}({call})\n}}",
                    ret.go, ret.go
                );
            }
            None => {
                let _ = writeln!(out, "func {go_name}({signature}) {{\n\t{call}\n}}");
            }
        }
    }
    out
}

#[cfg(test)]
mod tests;
