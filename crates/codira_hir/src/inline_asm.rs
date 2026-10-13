//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Reads `@asm(...)` attributes: inline assembly declared on a body-less
//!   `extern func`, one variant per architecture.
//!
//! ```codira
//! @asm(arch: "x86_64", "rdtsc; shl $$32, %rdx; or %rdx, %rax", constraints: "={rax},~{rdx}")
//! @asm(arch: "aarch64", "mrs $0, cntvct_el0", constraints: "=r")
//! extern func cycle_counter() -> u64;
//! ```
//!
//! The function never becomes a symbol. Codegen emits its assembly in place
//! at every call site, the way it emits an `extern "codira-intrinsic"`
//! operation, so a call costs exactly the instructions written. The template
//! and the constraint string are LLVM's inline-assembly syntax unchanged:
//! `$0`, `$1`, ... name operands (outputs first), `$$` is a literal `$`, and
//! constraints are `=r` for an output, `r` for an input, `{reg}` to pin a
//! register and `~{reg}` for a clobber.
//!
//! A variant without `arch:` applies to every architecture; one with `arch:`
//! is chosen over it on that architecture. `dialect: "intel"` switches the
//! template from AT&T to Intel syntax on x86. `pure: true` promises the
//! assembly has no effect beyond its outputs, which lets LLVM merge or delete
//! calls to it; without it every call is kept and kept in order.

use codira_syntax::{
    ast::{self, AstNode, AttributeOwner, LiteralKind},
    AstToken, SyntaxKind, T,
};

/// The syntax of an inline-assembly template.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum AsmDialect {
    Att,
    Intel,
}

/// One `@asm(...)` attribute.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct AsmVariant {
    /// The architecture this variant is for (`Target::arch`, e.g. `x86_64`),
    /// or `None` for every architecture.
    pub arch: Option<String>,
    pub template: String,
    pub constraints: String,
    pub dialect: AsmDialect,
    /// The assembly has no effect beyond producing its outputs.
    pub pure: bool,
}

/// What a constraint string asks for, counted the way the operands of the
/// call have to line up with it.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ConstraintCounts {
    pub outputs: usize,
    pub inputs: usize,
}

/// Counts the outputs (`=...`) and inputs of `constraints`, ignoring
/// clobbers (`~{...}`). Indirect operands (`*`) and read-write outputs (`+`)
/// are refused: an operand in memory needs a pointer, which Codira has no
/// type for yet, and LLVM spells a read-write output as an output plus a
/// tied input (`=r,0`).
pub fn count_constraints(constraints: &str) -> Result<ConstraintCounts, String> {
    let mut counts = ConstraintCounts {
        outputs: 0,
        inputs: 0,
    };
    if constraints.trim().is_empty() {
        return Ok(counts);
    }
    for code in constraints.split(',') {
        let code = code.trim();
        if code.is_empty() {
            return Err("the constraint list has an empty entry".to_owned());
        }
        if code.starts_with('~') {
            continue;
        }
        if code.contains('*') {
            return Err(format!(
                "constraint `{code}` is an indirect (memory) operand, which needs a pointer type \
                 Codira does not have yet"
            ));
        }
        if code.starts_with('+') {
            return Err(format!(
                "constraint `{code}` is a read-write output; write it as an output and a tied \
                 input instead (`=r` and then `0`)"
            ));
        }
        if code.starts_with('=') {
            if counts.inputs > 0 {
                return Err("outputs must come before inputs in the constraint list".to_owned());
            }
            counts.outputs += 1;
        } else {
            counts.inputs += 1;
        }
    }
    Ok(counts)
}

/// Whether `attr` is an `@asm(...)` attribute.
fn is_asm_attribute(attr: &ast::Attribute) -> bool {
    attr.path().and_then(|p| p.segment()).is_some_and(
        |s| matches!(s.kind(), Some(ast::PathSegmentKind::Name(n)) if n.text() == "asm"),
    )
}

/// Whether `func` carries any `@asm(...)` attribute, well-formed or not.
pub(crate) fn has_asm_attribute(func: &ast::FunctionDef) -> bool {
    func.attribute_list()
        .is_some_and(|attrs| attrs.attributes().any(|attr| is_asm_attribute(&attr)))
}

/// The arguments of an attribute, each with the `name:` label in front of it
/// if it has one. The grammar records a label as a loose `IDENT :` token pair
/// ahead of the argument expression (see `attribute_arg_list`).
fn labelled_args(arg_list: &ast::ArgList) -> Vec<(Option<String>, ast::Expr)> {
    let mut args = Vec::new();
    let mut label = None;
    let mut pending_ident: Option<String> = None;
    for element in arg_list.syntax().children_with_tokens() {
        if element.kind().is_trivia() {
            continue;
        }
        if let Some(token) = element.as_token() {
            match token.kind() {
                SyntaxKind::IDENT => pending_ident = Some(token.text().to_owned()),
                T![:] => label = pending_ident.take(),
                _ => pending_ident = None,
            }
        } else if let Some(node) = element.as_node() {
            if let Some(expr) = ast::Expr::cast(node.clone()) {
                args.push((label.take(), expr));
            }
            pending_ident = None;
        }
    }
    args
}

fn literal_of(expr: &ast::Expr) -> Option<ast::Literal> {
    match expr.kind() {
        ast::ExprKind::Literal(literal) => Some(literal),
        _ => None,
    }
}

fn string_arg(expr: &ast::Expr, what: &str) -> Result<String, String> {
    match literal_of(expr).map(|l| l.kind()) {
        Some(LiteralKind::String(token)) => {
            let (value, errors) = crate::expr::string_lit(token.text());
            if errors.is_empty() {
                Ok(value)
            } else {
                Err(format!("the {what} has a malformed escape"))
            }
        }
        _ => Err(format!("the {what} must be a string literal")),
    }
}

fn bool_arg(expr: &ast::Expr, what: &str) -> Result<bool, String> {
    match literal_of(expr).map(|l| l.kind()) {
        Some(LiteralKind::Bool(value)) => Ok(value),
        _ => Err(format!("`{what}:` must be `true` or `false`")),
    }
}

/// Reads one `@asm(...)` attribute.
fn parse_variant(attr: &ast::Attribute) -> Result<AsmVariant, String> {
    let Some(arg_list) = attr.arg_list() else {
        return Err("`@asm` needs an assembly template and `constraints:`".to_owned());
    };

    let mut template = None;
    let mut constraints = None;
    let mut arch = None;
    let mut dialect = AsmDialect::Att;
    let mut pure = false;
    for (label, value) in labelled_args(&arg_list) {
        match label.as_deref() {
            None if template.is_none() => {
                template = Some(string_arg(&value, "assembly template")?);
            }
            None => return Err("`@asm` takes one unlabelled argument, the template".to_owned()),
            Some("constraints") => constraints = Some(string_arg(&value, "constraint list")?),
            Some("arch") => arch = Some(string_arg(&value, "architecture")?),
            Some("dialect") => {
                dialect = match string_arg(&value, "dialect")?.as_str() {
                    "att" => AsmDialect::Att,
                    "intel" => AsmDialect::Intel,
                    other => {
                        return Err(format!(
                            "unknown assembly dialect `{other}`: expected `att` or `intel`"
                        ))
                    }
                };
            }
            Some("pure") => pure = bool_arg(&value, "pure")?,
            Some(other) => {
                return Err(format!(
                    "`@asm` has no `{other}:` argument; it takes a template, `constraints:`, \
                     `arch:`, `dialect:` and `pure:`"
                ))
            }
        }
    }

    let template = template.ok_or_else(|| "`@asm` needs an assembly template string".to_owned())?;
    let constraints = constraints.ok_or_else(|| {
        "`@asm` needs `constraints:`, even if empty (`constraints: \"\"`)".to_owned()
    })?;
    count_constraints(&constraints)?;
    Ok(AsmVariant {
        arch,
        template,
        constraints,
        dialect,
        pure,
    })
}

/// Every `@asm(...)` attribute on `func`: the ones that parse, and an error
/// message for each one that does not.
pub(crate) fn asm_variants(func: &ast::FunctionDef) -> (Vec<AsmVariant>, Vec<String>) {
    let mut variants = Vec::new();
    let mut errors = Vec::new();
    let Some(attrs) = func.attribute_list() else {
        return (variants, errors);
    };
    for attr in attrs.attributes().filter(is_asm_attribute) {
        match parse_variant(&attr) {
            Ok(variant) => {
                if variants.iter().any(|v: &AsmVariant| v.arch == variant.arch) {
                    errors.push(match &variant.arch {
                        Some(arch) => format!("more than one `@asm` variant for `{arch}`"),
                        None => "more than one `@asm` variant without `arch:`".to_owned(),
                    });
                } else {
                    variants.push(variant);
                }
            }
            Err(error) => errors.push(error),
        }
    }
    (variants, errors)
}

/// The variant of `variants` that applies on `arch`: the one written for it,
/// or else the one without `arch:`.
pub fn select_variant<'a>(variants: &'a [AsmVariant], arch: &str) -> Option<&'a AsmVariant> {
    variants
        .iter()
        .find(|v| v.arch.as_deref() == Some(arch))
        .or_else(|| variants.iter().find(|v| v.arch.is_none()))
}

#[cfg(test)]
mod tests {
    use codira_syntax::{ast::ModuleItemOwner, SourceFile};

    use super::*;

    fn func(src: &str) -> ast::FunctionDef {
        SourceFile::parse(src)
            .tree()
            .items()
            .find_map(|item| match item.kind() {
                ast::ModuleItemKind::FunctionDef(f) => Some(f),
                _ => None,
            })
            .expect("no function in fixture")
    }

    #[test]
    fn variants_are_read_with_their_labels() {
        let (variants, errors) = asm_variants(&func(
            r#"
            @asm(arch: "x86_64", "lea ($1,$2), $0", constraints: "=r,r,r", pure: true)
            @asm("add $0, $1, $2", constraints: "=r,r,r", dialect: "intel")
            extern func add(a: i64, b: i64) -> i64
            "#,
        ));
        assert_eq!(errors, Vec::<String>::new());
        assert_eq!(
            variants,
            vec![
                AsmVariant {
                    arch: Some("x86_64".to_owned()),
                    template: "lea ($1,$2), $0".to_owned(),
                    constraints: "=r,r,r".to_owned(),
                    dialect: AsmDialect::Att,
                    pure: true,
                },
                AsmVariant {
                    arch: None,
                    template: "add $0, $1, $2".to_owned(),
                    constraints: "=r,r,r".to_owned(),
                    dialect: AsmDialect::Intel,
                    pure: false,
                },
            ]
        );
        assert!(select_variant(&variants, "x86_64").unwrap().pure);
        assert_eq!(select_variant(&variants, "riscv64").unwrap().arch, None);
    }

    #[test]
    fn escapes_in_the_template_are_decoded() {
        let (variants, errors) = asm_variants(&func(
            r#"@asm("nop\n\tnop", constraints: "") extern func two_nops()"#,
        ));
        assert!(errors.is_empty(), "{errors:?}");
        assert_eq!(variants[0].template, "nop\n\tnop");
    }

    #[test]
    fn malformed_attributes_are_errors_not_variants() {
        let (variants, errors) = asm_variants(&func(
            r#"
            @asm(arch: "x86_64", "nop")
            @asm(arch: "aarch64", nop_not_a_string, constraints: "")
            @asm(arch: "riscv64", "nop", constraints: "", volatile: true)
            @asm("nop", constraints: "=*m")
            extern func f()
            "#,
        ));
        assert!(variants.is_empty(), "{variants:?}");
        assert_eq!(errors.len(), 4, "{errors:?}");
        assert!(errors[0].contains("constraints:"), "{}", errors[0]);
        assert!(errors[1].contains("string literal"), "{}", errors[1]);
        assert!(errors[2].contains("`volatile:`"), "{}", errors[2]);
        assert!(errors[3].contains("indirect"), "{}", errors[3]);
    }

    #[test]
    fn two_variants_for_one_architecture_are_an_error() {
        let (variants, errors) = asm_variants(&func(
            r#"
            @asm(arch: "x86_64", "nop", constraints: "")
            @asm(arch: "x86_64", "pause", constraints: "")
            extern func spin()
            "#,
        ));
        assert_eq!(variants.len(), 1);
        assert_eq!(errors, vec!["more than one `@asm` variant for `x86_64`"]);
    }

    #[test]
    fn constraints_are_counted() {
        assert_eq!(
            count_constraints("=r,r,r,~{memory},~{dirflag}"),
            Ok(ConstraintCounts {
                outputs: 1,
                inputs: 2
            })
        );
        assert_eq!(
            count_constraints("={rax},~{rdx}"),
            Ok(ConstraintCounts {
                outputs: 1,
                inputs: 0
            })
        );
        assert_eq!(
            count_constraints(""),
            Ok(ConstraintCounts {
                outputs: 0,
                inputs: 0
            })
        );
        assert!(count_constraints("r,=r").is_err());
        assert!(count_constraints("+r").is_err());
        assert!(count_constraints("=r,,r").is_err());
    }
}
