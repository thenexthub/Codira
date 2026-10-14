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
//! `{ ptr, usize }` over constant bytes, and generic functions are
//! monomorphised (see `crate::mono`). What is left is `nil`, which still
//! has no representation to produce, and the few generic declarations
//! monomorphisation does not cover (see `validate_generics`). A sixth gap was
//! found later: every construct that parses but lowers to `Expr::Missing`
//! (`match`, closures, postfix `!` and postfix `^`) passed
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
    diagnostics::{DiagnosticSink, ExprError, UnsupportedByCodegen},
    mono,
    ty::{Ty, TypeWalk},
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

        // A problem with the declaration's generics is reported once,
        // against the function itself, rather than once per mention of `T`:
        // the expression walk below would add nothing but noise for the same
        // root cause.
        if self.validate_generics(sink) {
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
    /// Several expression forms -- `match`, closures, postfix `!` and
    /// postfix `^` -- are fully parsed and then lowered to
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
            SyntaxKind::TRANSFER_EXPR => (
                "postfix `^` (transfer)",
                "there is no move/ownership model in the code generator yet",
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

    /// The function's *name*, where a diagnostic about the whole declaration
    /// goes.
    ///
    /// Not its whole syntax node. The whole node is often many lines, and
    /// the renderer this feeds (`annotate_snippets` with `fold: true`) can
    /// panic on a multi-line annotation range that extends past whatever
    /// window it decides to fold to: confirmed against
    /// `std/collections/binary_heap.code`'s `iter_next`, an 8-line function
    /// whose whole-node span crashed rendering with "`SourceAnnotation` range
    /// is beyond the end of buffer". The name is a single short,
    /// single-line node that still points the reader at the right function.
    fn name_node(&self) -> SyntaxNodePtr {
        let source = self.func.source(self.db);
        source.value.name().map_or_else(
            || SyntaxNodePtr::new(source.value.syntax()),
            |name| SyntaxNodePtr::new(name.syntax()),
        )
    }

    /// Checks what monomorphisation needs of this function's generics, and
    /// returns whether it reported anything.
    ///
    /// * Every type parameter the signature or body mentions must be one of the
    ///   function's own (its `[T]` or its `extend Box[T]`'s). Another one can
    ///   only come from a generic type written without its arguments -- `b:
    ///   Box` for `struct Box[T]` means `Box[T]` with `Box`'s own `T` -- and no
    ///   instance of this function says what that `T` is.
    /// * A generic function has one body per instance and so no single symbol:
    ///   `@export`, `@asm` and `@test` need one, and are refused. A `uses`
    ///   clause is refused too, as a gap: evidence passing has not been taught
    ///   to instances yet.
    /// * Instantiation must terminate. Starting from a non-generic function,
    ///   the instances it reaches are collected exactly as the code generator
    ///   collects them; a call that asks for an ever larger type (polymorphic
    ///   recursion) is reported instead of looping.
    fn validate_generics(&self, sink: &mut DiagnosticSink<'_>) -> bool {
        let file = self.func.source(self.db).file_id;
        let environment = self.func.generic_environment(self.db);
        let report = |message: String, sink: &mut DiagnosticSink<'_>| {
            sink.push(ExprError {
                file,
                node: self.name_node(),
                message,
            });
        };

        let foreign_param = |ty: &Ty| {
            let mut found = None;
            ty.walk(&mut |inner| {
                if let TyKind::TypeParam(id, name) = inner.interned() {
                    if found.is_none() && !environment.contains(id) {
                        found = Some(name.clone());
                    }
                }
            });
            found.map(|name| (ty.clone(), name))
        };
        let signature = self
            .func
            .self_param_ty(self.db)
            .into_iter()
            .chain(
                self.func
                    .params(self.db)
                    .into_iter()
                    .map(|p| p.ty().clone()),
            )
            .chain(std::iter::once(self.func.ret_type(self.db)));
        let body_types = self
            .body
            .exprs()
            .map(|(expr, _)| self.infer[expr].clone())
            .chain(self.body.pats().map(|(pat, _)| self.infer[pat].clone()));
        if let Some((ty, name)) = signature
            .chain(body_types)
            .find_map(|ty| foreign_param(&ty))
        {
            report(
                format!(
                    "the type `{}` mentions the generic parameter `{name}`, which `{}` does not \
                     declare: a generic type needs its arguments written out, e.g. `Box[i32]` \
                     rather than `Box`",
                    ty.display(self.db),
                    self.func.name(self.db),
                ),
                sink,
            );
            return true;
        }

        if !environment.is_empty() {
            let name = self.func.name(self.db);
            let refusal = if self.func.export_abi(self.db).is_some() {
                Some(format!(
                    "`@export` cannot be applied to the generic function `{name}`: it has one \
                     body per instantiation and so no single symbol to export; export a \
                     non-generic function that calls it at the types the host needs"
                ))
            } else if self.func.is_inline_asm(self.db) {
                Some(format!(
                    "`@asm` cannot be applied to the generic function `{name}`: inline assembly \
                     has fixed operand types"
                ))
            } else if self.func.is_test(self.db) {
                Some(format!(
                    "the `@test` function `{name}` cannot be generic: the test runner calls it \
                     with no arguments, so nothing says what its parameters are"
                ))
            } else {
                None
            };
            if let Some(message) = refusal {
                report(message, sink);
                return true;
            }
            if !self.func.effects(self.db).is_empty() {
                sink.push(UnsupportedByCodegen {
                    file,
                    node: self.name_node(),
                    what: format!("the generic function `{name}` with a `uses` clause"),
                    because: "effect evidence is passed to one body per function, and a generic \
                              function has one per instantiation"
                        .to_string(),
                });
                return true;
            }
            return false;
        }

        if let Err(overflow) = mono::collect_instances(self.db, [self.func], |_| false) {
            report(
                format!(
                    "instantiating the generic functions `{}` calls does not terminate: `{}` \
                     calls `{}` at an ever larger type (its type arguments nest more than {} \
                     deep and keep growing), so there is no finite set of instances to generate",
                    self.func.name(self.db),
                    overflow.caller.name(self.db),
                    overflow.instance.function().name(self.db),
                    mono::MAX_INSTANCE_TYPE_DEPTH,
                ),
                sink,
            );
            return true;
        }
        false
    }
}
