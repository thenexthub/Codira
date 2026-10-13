//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - `codira import-c`: reads a C header with Clang and writes the Codira
//!   `extern "C"` declarations for the functions it declares.
//!
//! Clang does the parsing (`-Xclang -ast-dump=json -fsyntax-only`), so the
//! header is understood exactly as a C compiler would understand it: macros
//! expanded, typedefs resolved to the canonical type, `#include`s followed.
//! Only declarations written in the header itself are imported, not those of
//! the headers it includes.
//!
//! A function is imported when every parameter and the return type is a
//! scalar Codira can carry across the C ABI: the integer and floating-point
//! types, `_Bool`, and pointers, which cross as `usize` addresses -- the
//! convention the `load_*`/`store_*` intrinsics already use. Everything else
//! is listed in a comment with the reason: structs and unions by value,
//! variadic functions, `static`/`inline` functions (no symbol to call), and
//! names Codira cannot spell.
//!
//! `long` is 32 bits on Windows and 64 elsewhere; the header is read for the
//! target being imported for (`--target`, the host by default), so the
//! declarations match that target's ABI.

use std::{fmt::Write as _, path::PathBuf, process::Command};

use anyhow::{anyhow, bail};
use codira_compiler::Target;
use serde_json::Value;

use crate::ExitStatus;

#[derive(clap::Args)]
pub struct Args {
    /// The C header to import
    header: PathBuf,

    /// Where to write the Codira declarations. Defaults to stdout.
    #[clap(long, short = 'o')]
    output: Option<PathBuf>,

    /// Directories searched for `#include`
    #[clap(short = 'I')]
    include: Vec<PathBuf>,

    /// Preprocessor definitions, `NAME` or `NAME=VALUE`
    #[clap(short = 'D')]
    define: Vec<String>,

    /// Target triple to read the header for. Defaults to the host.
    #[clap(long)]
    target: Option<String>,

    /// Read the header as C++ (`extern "C"` functions only)
    #[clap(long)]
    cxx: bool,
}

/// Keywords that cannot name a Codira binding, including the contextual
/// ones that are keywords where a parameter name would go.
const CODIRA_KEYWORDS: &[&str] = &[
    "as",
    "borrowing",
    "break",
    "class",
    "comptime",
    "consuming",
    "data",
    "decreases",
    "effect",
    "else",
    "ensures",
    "enum",
    "extend",
    "extern",
    "false",
    "for",
    "func",
    "ghost",
    "handle",
    "if",
    "import",
    "in",
    "init",
    "internal",
    "invariant",
    "let",
    "loop",
    "macro",
    "match",
    "never",
    "nil",
    "override",
    "perform",
    "private",
    "public",
    "requires",
    "resume",
    "return",
    "root",
    "self",
    "spawn",
    "static",
    "struct",
    "super",
    "trait",
    "true",
    "type",
    "uses",
    "var",
    "where",
    "while",
];

/// A function that imports.
#[derive(Debug, PartialEq)]
struct Import {
    name: String,
    params: Vec<(String, &'static str)>,
    ret: Option<&'static str>,
    has_pointer: bool,
}

/// What `import-c` makes of one function declaration.
#[derive(Debug, PartialEq)]
enum Imported {
    Function(Import),
    Skipped { name: String, reason: String },
}

/// The Codira type that carries the C type `ty` (a canonical spelling, as
/// Clang's `desugaredQualType` gives it), or why there is none.
fn codira_type(ty: &str, long_is_64: bool) -> Result<&'static str, String> {
    let ty = ty
        .replace("const ", "")
        .replace("volatile ", "")
        .replace(" const", "")
        .trim()
        .to_owned();
    if ty.ends_with('*') || ty.contains("(*)") || ty.ends_with(']') {
        return Ok("usize");
    }
    // C enums are `int`-sized on every target Codira supports.
    if ty.starts_with("enum ") {
        return Ok("i32");
    }
    // `long` is the one integer whose width depends on the target.
    let (long, unsigned_long) = if long_is_64 {
        ("i64", "u64")
    } else {
        ("i32", "u32")
    };
    Ok(match ty.as_str() {
        "_Bool" | "bool" => "bool",
        "char" | "signed char" => "i8",
        "unsigned char" => "u8",
        "short" | "signed short" => "i16",
        "unsigned short" => "u16",
        "int" | "signed int" | "signed" => "i32",
        "unsigned int" | "unsigned" => "u32",
        "long" | "signed long" => long,
        "unsigned long" => unsigned_long,
        "long long" | "signed long long" => "i64",
        "unsigned long long" => "u64",
        "float" => "f32",
        "double" => "f64",
        "long double" => return Err("`long double` has no Codira type".to_owned()),
        other if other.starts_with("struct ") || other.starts_with("union ") => {
            return Err(format!(
                "`{other}` is passed by value, which Codira cannot express yet"
            ))
        }
        other => return Err(format!("`{other}` has no Codira type")),
    })
}

fn is_identifier(name: &str) -> bool {
    let mut chars = name.chars();
    chars
        .next()
        .is_some_and(|c| c.is_ascii_alphabetic() || c == '_')
        && chars.all(|c| c.is_ascii_alphanumeric() || c == '_')
}

/// The canonical type of an AST node's `type` object.
fn canonical(node: &Value) -> Option<&str> {
    let ty = node.get("type")?;
    ty.get("desugaredQualType")
        .or_else(|| ty.get("qualType"))
        .and_then(Value::as_str)
}

/// The return type out of a function type's spelling, `ret (params)`.
fn return_type(function_type: &str) -> &str {
    function_type
        .find('(')
        .map_or(function_type, |paren| &function_type[..paren])
        .trim()
}

/// Every `typedef` in the translation unit, the included headers' too, by
/// name, to its underlying type.
type Typedefs = std::collections::BTreeMap<String, String>;

fn collect_typedefs(ast: &Value, typedefs: &mut Typedefs) {
    for node in ast
        .get("inner")
        .and_then(Value::as_array)
        .into_iter()
        .flatten()
    {
        match node.get("kind").and_then(Value::as_str) {
            Some("TypedefDecl") => {
                if let (Some(name), Some(ty)) =
                    (node.get("name").and_then(Value::as_str), canonical(node))
                {
                    typedefs.insert(name.to_owned(), ty.to_owned());
                }
            }
            Some("LinkageSpecDecl") => collect_typedefs(node, typedefs),
            _ => {}
        }
    }
}

/// Resolves `ty` through `typedefs` until it is a type C spells directly.
///
/// Parameters come with Clang's `desugaredQualType`, but a function's return
/// type only appears inside the function type's spelling, still written
/// with its typedef (`int32_t (int32_t, int32_t)`).
fn resolve(ty: &str, typedefs: &Typedefs) -> String {
    let mut ty = ty.trim().to_owned();
    for _ in 0..16 {
        let bare = ty
            .replace("const ", "")
            .replace("volatile ", "")
            .trim()
            .to_owned();
        match typedefs.get(&bare) {
            Some(underlying) if *underlying != bare => ty = underlying.clone(),
            _ => break,
        }
    }
    ty
}

/// Reads one `FunctionDecl` node.
fn import_function(node: &Value, long_is_64: bool, typedefs: &Typedefs) -> Imported {
    let name = node
        .get("name")
        .and_then(Value::as_str)
        .unwrap_or_default()
        .to_owned();
    let skip = |reason: &str| Imported::Skipped {
        name: name.clone(),
        reason: reason.to_owned(),
    };

    if node.get("storageClass").and_then(Value::as_str) == Some("static")
        || node.get("inline").and_then(Value::as_bool) == Some(true)
    {
        return skip("it is `static` or `inline`, so there is no symbol to call");
    }
    if node.get("variadic").and_then(Value::as_bool) == Some(true) {
        return skip("it is variadic");
    }
    if !is_identifier(&name) || CODIRA_KEYWORDS.contains(&name.as_str()) {
        return skip("its name is not a Codira identifier");
    }

    let Some(function_type) = canonical(node) else {
        return skip("Clang gave it no type");
    };
    let mut has_pointer = false;
    let ret = resolve(return_type(function_type), typedefs);
    let ret = match ret.as_str() {
        "void" => None,
        ret => match codira_type(ret, long_is_64) {
            Ok(ty) => {
                has_pointer |= ty == "usize" && ret.contains('*');
                Some(ty)
            }
            Err(reason) => return skip(&format!("its return type {reason}")),
        },
    };

    let mut params = Vec::new();
    let parameters = node
        .get("inner")
        .and_then(Value::as_array)
        .into_iter()
        .flatten()
        .filter(|inner| inner.get("kind").and_then(Value::as_str) == Some("ParmVarDecl"));
    for (index, param) in parameters.enumerate() {
        let Some(ty) = canonical(param) else {
            return skip("a parameter has no type");
        };
        let ty = resolve(ty, typedefs);
        let ty = ty.as_str();
        let codira = match codira_type(ty, long_is_64) {
            Ok(codira) => codira,
            Err(reason) => return skip(&format!("parameter {} {reason}", index + 1)),
        };
        has_pointer |= ty.contains('*');
        let mut param_name = param
            .get("name")
            .and_then(Value::as_str)
            .filter(|n| is_identifier(n))
            .map_or_else(|| format!("arg{index}"), str::to_owned);
        if CODIRA_KEYWORDS.contains(&param_name.as_str()) {
            param_name.push('_');
        }
        params.push((param_name, codira));
    }

    Imported::Function(Import {
        name,
        params,
        ret,
        has_pointer,
    })
}

/// Every function declared in the header itself, in the order it declares
/// them.
fn imports_from_ast(ast: &Value, long_is_64: bool, typedefs: &Typedefs) -> Vec<Imported> {
    let mut imported = Vec::new();
    let mut seen = std::collections::BTreeSet::new();
    for node in ast
        .get("inner")
        .and_then(Value::as_array)
        .into_iter()
        .flatten()
    {
        // Declarations from an `#include`d header carry `includedFrom`, on
        // their location or, for a macro-built one, on its expansion.
        let loc = node.get("loc");
        let included = loc.is_some_and(|loc| {
            loc.get("includedFrom").is_some()
                || loc
                    .get("expansionLoc")
                    .is_some_and(|e| e.get("includedFrom").is_some())
        });
        if included || node.get("isImplicit").and_then(Value::as_bool) == Some(true) {
            continue;
        }
        match node.get("kind").and_then(Value::as_str) {
            Some("FunctionDecl") => {
                let import = import_function(node, long_is_64, typedefs);
                let name = match &import {
                    Imported::Function(f) => f.name.clone(),
                    Imported::Skipped { name, .. } => name.clone(),
                };
                // A header may declare a function more than once.
                if seen.insert(name) {
                    imported.push(import);
                }
            }
            Some("LinkageSpecDecl") => {
                imported.extend(imports_from_ast(node, long_is_64, typedefs));
            }
            _ => {}
        }
    }
    imported
}

fn render(header: &str, imports: &[Imported]) -> String {
    let mut out = format!(
        "// Generated by `codira import-c {header}`. Do not edit.\n//\n// Pointers cross as \
         `usize` addresses; read and write through them with the\n// `load_*`/`store_*` \
         intrinsics.\n\nextern \"C\" {{\n"
    );
    for import in imports {
        if let Imported::Function(f) = import {
            let params: Vec<String> = f
                .params
                .iter()
                .map(|(name, ty)| format!("{name}: {ty}"))
                .collect();
            let ret = f.ret.map_or_else(String::new, |ty| format!(" -> {ty}"));
            let note = if f.has_pointer {
                "  // takes or returns a pointer"
            } else {
                ""
            };
            let _ = writeln!(
                out,
                "    func {}({}){ret};{note}",
                f.name,
                params.join(", ")
            );
        }
    }
    out.push_str("}\n");
    let skipped: Vec<&Imported> = imports
        .iter()
        .filter(|i| matches!(i, Imported::Skipped { .. }))
        .collect();
    if !skipped.is_empty() {
        out.push_str("\n// Not imported:\n");
        for import in skipped {
            if let Imported::Skipped { name, reason } = import {
                let _ = writeln!(out, "//   {name}: {reason}");
            }
        }
    }
    out
}

/// Runs Clang on `args.header` and returns the Codira declarations.
fn import(args: &Args) -> anyhow::Result<String> {
    let target = match &args.target {
        Some(triple) => {
            Target::search(triple).ok_or_else(|| anyhow!("unknown target `{triple}`"))?
        }
        None => Target::host_target().ok_or_else(|| anyhow!("unable to determine host target"))?,
    };
    let long_is_64 = target.pointer_width == 64 && !target.options.is_like_windows;

    let clang = codira_compiler::find_clang();
    let mut command = Command::new(&clang);
    command
        .arg(format!("--target={}", target.llvm_target))
        .args(["-x", if args.cxx { "c++-header" } else { "c-header" }])
        .args(["-Xclang", "-ast-dump=json", "-fsyntax-only"]);
    for dir in &args.include {
        command.arg(format!("-I{}", dir.display()));
    }
    for define in &args.define {
        command.arg(format!("-D{define}"));
    }
    command.arg(&args.header);

    let output = command.output().map_err(|e| {
        anyhow!(
            "could not run Clang ('{}'): {e}. Set CODIRA_CLANG to a clang executable.",
            clang.display()
        )
    })?;
    if !output.status.success() {
        bail!(
            "Clang could not read '{}':\n{}",
            args.header.display(),
            String::from_utf8_lossy(&output.stderr)
        );
    }
    let ast: Value = serde_json::from_slice(&output.stdout)
        .map_err(|e| anyhow!("Clang's AST dump was not JSON: {e}"))?;
    let mut typedefs = Typedefs::new();
    collect_typedefs(&ast, &mut typedefs);
    let imports = imports_from_ast(&ast, long_is_64, &typedefs);
    if !imports.iter().any(|i| matches!(i, Imported::Function(_))) {
        bail!(
            "'{}' declares no function Codira can import",
            args.header.display()
        );
    }
    let header = args.header.file_name().map_or_else(
        || args.header.display().to_string(),
        |n| n.to_string_lossy().into_owned(),
    );
    Ok(render(&header, &imports))
}

pub fn import_c(args: Args) -> anyhow::Result<ExitStatus> {
    let rendered = import(&args)?;
    match &args.output {
        Some(path) => std::fs::write(path, rendered)
            .map_err(|e| anyhow!("could not write '{}': {e}", path.display()))?,
        None => print!("{rendered}"),
    }
    Ok(ExitStatus::Success)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn c_types_map_to_the_codira_type_of_the_same_width() {
        assert_eq!(codira_type("int", true), Ok("i32"));
        assert_eq!(codira_type("unsigned long long", true), Ok("u64"));
        assert_eq!(codira_type("long", false), Ok("i32"));
        assert_eq!(codira_type("long", true), Ok("i64"));
        assert_eq!(codira_type("const char *", true), Ok("usize"));
        assert_eq!(codira_type("void (*)(int)", true), Ok("usize"));
        assert_eq!(codira_type("_Bool", true), Ok("bool"));
        assert!(codira_type("struct P", true).is_err());
        assert!(codira_type("long double", true).is_err());
    }

    /// Runs the real Clang on a header and checks what imports.
    #[test]
    fn a_header_imports_through_clang() {
        let dir = tempfile::tempdir().unwrap();
        let header = dir.path().join("api.h");
        std::fs::write(
            &header,
            "#include <stdint.h>\n#include <stddef.h>\n\
             int32_t add(int32_t a, int32_t b);\n\
             double scale(double x, float y);\n\
             void reset(void);\n\
             size_t length(const char *text);\n\
             int checked(_Bool loop, unsigned char type);\n\
             static inline int hidden(int x) { return x; }\n\
             int variadic(int n, ...);\n\
             struct point { int x, y; };\n\
             int by_value(struct point p);\n\
             int add(int32_t a, int32_t b);\n",
        )
        .unwrap();
        let args = Args {
            header,
            output: None,
            include: Vec::new(),
            define: Vec::new(),
            target: Some("x86_64-unknown-linux-gnu".to_owned()),
            cxx: false,
        };
        let rendered = match import(&args) {
            Ok(rendered) => rendered,
            Err(error) if error.to_string().contains("could not run Clang") => {
                eprintln!("skipped: {error}");
                return;
            }
            Err(error) => panic!("{error}"),
        };
        for expected in [
            "    func add(a: i32, b: i32) -> i32;",
            "    func scale(x: f64, y: f32) -> f64;",
            "    func reset();",
            "    func length(text: usize) -> u64;  // takes or returns a pointer",
            "    func checked(loop_: bool, type_: u8) -> i32;",
            "//   hidden: it is `static` or `inline`",
            "//   variadic: it is variadic",
            "//   by_value: parameter 1 `struct point` is passed by value",
        ] {
            assert!(
                rendered.contains(expected),
                "missing `{expected}` in:\n{rendered}"
            );
        }
        assert_eq!(rendered.matches("func add(").count(), 1, "{rendered}");
        // Nothing from <stdint.h> or <stddef.h>.
        assert!(!rendered.contains("func memcpy"), "{rendered}");
    }
}
