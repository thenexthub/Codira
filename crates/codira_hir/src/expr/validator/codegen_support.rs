//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: September 17, 2026
//!
//! Reports constructs the code generator cannot lower yet, as diagnostics,
//! before it is asked to try.
//!
//! # Why this exists
//!
//! `codira_codegen` had `unimplemented!()` for several constructs that pass
//! the whole front end. Five of them were reachable from ordinary source:
//!
//! ```text
//! let s = "hello"        // panicked at ir/body.rs:567
//! let x: i64? = nil      // panicked at ir/body.rs:569
//! func id[T](x: T) -> T  // panicked at ir/ty.rs:240 / 438 / 452
//! ```
//!
//! The string case is no longer among them: a literal lowers to
//! `{ ptr, usize }` over constant bytes. What is left is `nil`, which still
//! has no representation to produce, and generic instantiation, which type
//! inference now handles but code generation does not. A sixth gap was
//! found later: every construct that parses but lowers to `Expr::Missing`
//! (`match`, closures, postfix `!`, the effect and channel operators) passed
//! every check and then hit `gen_expr`'s `unimplemented!`. Those are now
//! reported here too, named after what was written.
//!
//! A `panic!` reaching a user is the worst outcome available: it aborts with
//! a Rust backtrace and an invitation to debug the compiler, gives no source
//! location, and -- for a toolchain meant for mission-critical work -- is
//! indistinguishable from a compiler bug, because it *is* one.
//!
//! `spec/EIDOS_RFC_002.md` §3.4 states the rule this module implements, in
//! the context of effects but as a general principle: what a backend gap
//! needs is "**a declining arm in codegen**: `Expr::Perform => None` plus a
//! `CannotCodegen` diagnostic -- *not* `unimplemented!()`".
//!
//! Declining here rather than inside codegen is what makes that practical.
//! `gen_expr` returns `Option`, where `None` already means "this expression
//! diverges"; overloading it with "the backend declined" would conflate two
//! very different things at every call site. Running as a validator instead
//! means the diagnostic carries a real source span, compilation stops before
//! codegen is entered, and codegen's own `unimplemented!()`s become what
//! they claim to be -- unreachable.
//!
//! # What is reported
//!
//! Only constructs that genuinely cannot be lowered today. This list is
//! meant to shrink: each entry is a backend gap, not a language rule, and
//! the diagnostics say so.

use codira_syntax::{ast::NameOwner, AstNode, SyntaxNodePtr};

use super::ExprValidator;
use crate::{
    diagnostics::{DiagnosticSink, UnsupportedByCodegen},
    ty::Ty,
    Expr, HasSource, HirDisplay, Literal, TyKind,
};

impl ExprValidator<'_> {
    /// Pushes a diagnostic for each construct in this function that the
    /// code generator cannot lower.
    pub fn validate_codegen_support(&self, sink: &mut DiagnosticSink<'_>) {
        // An `extern` declaration has no body for the code generator to
        // produce: `extern "codira-intrinsic"` items are lowered per call
        // site, directly from that call's own already-concrete argument and
        // return types, never by generating code for the declaration
        // itself. A generic one -- `func sizeof_of[T](marker: T) -> usize;`
        // -- is therefore not the same gap as a generic *function
        // definition*, which does need a monomorphized body and does not
        // have one. Reporting it here would decline every caller of a
        // perfectly callable intrinsic, using this function's own signature
        // instead of the caller's.
        if self.func.is_extern(self.db) {
            return;
        }

        let file = self.func.source(self.db).file_id;

        // A generic declaration is reported once, against the function
        // itself, rather than once per mention of `T`. The gap is
        // instantiation, not any individual expression, and one diagnostic
        // naming the function is what the reader can act on.
        if let Some(ty) = self.first_type_parameter() {
            let source = self.func.source(self.db);
            // The function's *name*, not its whole syntax node. The whole
            // node is often many lines -- every method here that needs this
            // diagnostic at all has a body -- and the renderer this feeds
            // (`annotate_snippets` with `fold: true`) can panic on a
            // multi-line annotation range that extends past whatever window
            // it decides to fold to: confirmed against
            // `std/collections/binary_heap.code`'s `iter_next`, an 8-line
            // function whose whole-node span crashed rendering with
            // "SourceAnnotation range is beyond the end of buffer" the
            // moment `contains_type_parameter` started looking inside a
            // struct's substitution and so started firing on it. The name
            // is a single short, single-line node that still points the
            // reader at the right function.
            let node = source.value.name().map_or_else(
                || SyntaxNodePtr::new(source.value.syntax()),
                |name| SyntaxNodePtr::new(name.syntax()),
            );
            sink.push(UnsupportedByCodegen {
                file,
                node,
                what: format!(
                    "a generic function (`{}` is not instantiated)",
                    ty.display(self.db)
                ),
                because: "monomorphisation is not implemented yet, so there is no concrete type \
                          to generate code for"
                    .to_string(),
            });
            // One report is enough; the expression walk below would add
            // nothing but noise for the same root cause.
            return;
        }

        for (expr_id, expr) in self.body.exprs() {
            if let Expr::Missing = expr {
                self.report_missing_expr(expr_id, sink);
                continue;
            }
            let Expr::Literal(literal) = expr else {
                continue;
            };
            let what = match literal {
                Literal::Nil => "`nil`",
                // A string literal lowers to `{ ptr, usize }` over constant
                // bytes now, so it is no longer a backend gap.
                Literal::String(_) | Literal::Bool(_) | Literal::Int(_) | Literal::Float(_) => {
                    continue
                }
            };

            let Some(node) = self.body_source_map.expr_syntax(expr_id).map(|ptr| {
                ptr.value
                    .either(|it| it.syntax_node_ptr(), |it| it.syntax_node_ptr())
            }) else {
                continue;
            };

            sink.push(UnsupportedByCodegen {
                file,
                node,
                what: what.to_string(),
                because: match literal {
                    Literal::String(_) => {
                        "there is no string representation in the runtime yet, so a string \
                         literal has nothing to lower to"
                    }
                    _ => {
                        "`Type?` optionals lower transparently to their base type, so there is \
                         no null representation for `nil` to produce"
                    }
                }
                .to_string(),
            });
        }
    }

    /// Reports an `Expr::Missing` that stands in for a construct the front
    /// end parses but does not lower.
    ///
    /// Several expression forms -- `match`, closures, postfix `!`, the
    /// effect and channel operators -- are fully parsed and then lowered to
    /// `Expr::Missing`, so that the code around them still type-checks.
    /// `Missing` infers to the error type, which unifies with everything and
    /// reports nothing, so without this check such a function sailed through
    /// every diagnostic and reached `gen_expr`'s `Expr::Missing` arm: a
    /// compiler crash with a Rust backtrace instead of a source location.
    ///
    /// The construct is named from the syntax node the placeholder was
    /// allocated for, so the message says what was written. A `Missing` with
    /// no syntax node behind it comes from a syntax error (an absent operand,
    /// an absent body), which the parser has already reported; reporting it
    /// again here would only repeat that error with a worse span.
    fn report_missing_expr(&self, expr_id: crate::ExprId, sink: &mut DiagnosticSink<'_>) {
        use codira_syntax::SyntaxKind;

        let Some(node) = self.body_source_map.expr_syntax(expr_id).map(|ptr| {
            ptr.value
                .either(|it| it.syntax_node_ptr(), |it| it.syntax_node_ptr())
        }) else {
            return;
        };

        let (what, because) = match node.kind() {
            SyntaxKind::MATCH_EXPR => (
                "`match`",
                "pattern matching has no lowering yet; use `if`/`else` chains for now",
            ),
            SyntaxKind::CLOSURE_EXPR => (
                "a closure",
                "there is no function-value representation or capture analysis yet",
            ),
            SyntaxKind::TRY_EXPR => (
                "postfix `!` (force-unwrap)",
                "`Type?` optionals have no runtime representation yet",
            ),
            SyntaxKind::PERFORM_EXPR => (
                "`perform`",
                "effect operations are type-checked but not yet lowered",
            ),
            SyntaxKind::HANDLE_EXPR => (
                "`handle`",
                "effect handlers are type-checked but not yet lowered",
            ),
            SyntaxKind::SPAWN_EXPR => ("`spawn`", "there is no task scheduler to lower it to yet"),
            SyntaxKind::TRANSFER_EXPR => (
                "postfix `^` (transfer)",
                "there is no move/ownership model in the code generator yet",
            ),
            SyntaxKind::CHANNEL_RECV_EXPR | SyntaxKind::CHANNEL_SEND_EXPR => (
                "a channel operation",
                "there is no channel value model in the code generator yet",
            ),
            _ => (
                "this expression",
                "it is parsed, but nothing lowers it to code yet",
            ),
        };

        sink.push(UnsupportedByCodegen {
            file: self.func.source(self.db).file_id,
            node,
            what: what.to_string(),
            because: because.to_string(),
        });
    }

    /// The first type parameter appearing in this function's signature,
    /// if any.
    ///
    /// The signature is enough: a type parameter can only enter a body
    /// through a parameter, the return type, or a generic type mentioned in
    /// them, so a function whose signature is concrete has no uninstantiated
    /// parameter to trip over. Checking the signature also means the
    /// diagnostic lands on the declaration, which is where the reader can
    /// do something about it.
    fn first_type_parameter(&self) -> Option<Ty> {
        self.func
            .self_param_ty(self.db)
            .into_iter()
            .chain(
                self.func
                    .params(self.db)
                    .into_iter()
                    .map(|p| p.ty().clone()),
            )
            .chain(std::iter::once(self.func.ret_type(self.db)))
            .find(contains_type_parameter)
    }
}

/// Whether `ty` is, or contains, a generic type parameter.
pub(crate) fn contains_type_parameter(ty: &Ty) -> bool {
    match ty.interned() {
        TyKind::TypeParam(..) => true,
        TyKind::Array(element) => contains_type_parameter(element),
        // `Box[T]` is `TyKind::Struct` with `T` inside its substitution, not
        // a bare `TyKind::TypeParam` itself -- a parameter of type `Box[T]`
        // is exactly as uninstantiated as one of type `T` directly, and
        // missing that here would let a struct-wrapped type parameter slip
        // past this check into a codegen path that still cannot handle it.
        TyKind::Tuple(_, substs) | TyKind::Struct(_, substs) => {
            substs.iter().any(contains_type_parameter)
        }
        _ => false,
    }
}
