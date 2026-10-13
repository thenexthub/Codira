//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Semantic search: which functions can satisfy a predicate over `result` and
//!   their parameters, with a witness input for each.
//!
//! `codira query "result < 0"` asks, for every function: is there an input
//! satisfying the function's `requires` and parameter refinements on which
//! the function returns a value satisfying the predicate? The function's
//! return value is taken from its *body* (the same symbolic execution the
//! verifier runs, callees summarised by their contracts), so the answer is
//! about what the code can actually return, not what its signature or
//! `ensures` allow. A function whose body cannot be modelled at all (an
//! `extern` declaration) is answered from its `ensures` and return
//! refinement alone.
//!
//! The answer is an over-approximation in exactly the places the verifier's
//! model is: a value it does not model (a struct field, an array element, a
//! loop variable after a loop without invariant) is unconstrained, so a
//! match there may be a value the program never produces. A function is
//! never *missed* because of that -- unconstrained values only add matches.
//!
//! One Z3 context serves the whole query, and every check runs under the
//! deterministic `rlimit` budget; a function whose check exhausts it is
//! listed separately as unknown, never as a match or a non-match.

use codira_smt::{CheckOutcome, Context, UnknownReason};
use codira_syntax::{ast, AstNode, SourceFile};
use rustc_hash::FxHashMap;

use super::{
    exec::{self, Sym},
    Value,
};
use crate::{code_model::Function, HasSource, HirDatabase, Package};

/// A function that can satisfy the predicate.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct QueryMatch {
    pub function: Function,
    /// `path:line` of the function's declaration.
    pub location: String,
    /// The signature as written.
    pub signature: String,
    /// The witness: a value for every modelled parameter.
    pub inputs: Vec<(String, Value)>,
    /// What the function returns on the witness, if it is modelled.
    pub result: Option<Value>,
}

/// Everything a query found.
#[derive(Debug, Default, Clone, PartialEq, Eq)]
pub struct QueryOutcome {
    pub matches: Vec<QueryMatch>,
    /// Functions whose check ran out of budget: `(location, signature)`.
    pub unknown: Vec<(String, String)>,
    /// How many functions the predicate does not apply to (it names a
    /// parameter they do not have, or compares values of the wrong type).
    pub not_applicable: usize,
}

/// Parses `predicate` as a Codira expression.
pub fn parse_predicate(predicate: &str) -> Result<ast::Expr, String> {
    let source = format!("func __query() {{ {predicate} }}");
    let parse = SourceFile::parse(&source);
    if let Some(error) = parse.errors().first() {
        return Err(format!("`{predicate}` is not an expression: {error}"));
    }
    parse
        .tree()
        .syntax()
        .descendants()
        .find_map(ast::BlockExpr::cast)
        .and_then(|block| block.expr())
        .ok_or_else(|| format!("`{predicate}` is not an expression"))
}

/// Runs `predicate` against every function of every package in `db`.
pub fn query(db: &dyn HirDatabase, predicate: &str) -> Result<QueryOutcome, String> {
    let expr = parse_predicate(predicate)?;
    let ctx = Context::new();
    let mut outcome = QueryOutcome::default();
    let mut functions: Vec<Function> = Package::all(db)
        .into_iter()
        .flat_map(|package| package.modules(db))
        .flat_map(|module| module.all_functions(db))
        .collect();
    functions.sort_by_key(|f| {
        (
            f.file_id(db),
            f.source(db).value.syntax().text_range().start(),
        )
    });

    for function in functions {
        let Some(paths) = exec::return_paths(db, function, &ctx) else {
            outcome.not_applicable += 1;
            continue;
        };
        let mut applicable = false;
        let mut found = None;
        let mut unknown = false;
        let solver = ctx.solver();
        for (pc, value) in &paths.returns {
            let mut env: FxHashMap<String, Sym<'_>> = paths.inputs.iter().cloned().collect();
            env.insert("result".to_string(), *value);
            let Ok(goal) = encode(&ctx, &expr, &env, None) else {
                continue;
            };
            let Sym::Bool(goal) = goal else {
                continue;
            };
            applicable = true;
            solver.push();
            solver.assert(*pc);
            solver.assert(goal);
            match solver.check_detailed() {
                CheckOutcome::Sat => {
                    if let Some(model) = solver.model() {
                        let read = |sym: Sym<'_>| match sym {
                            Sym::Int(bv, ty) => Some(Value::Int {
                                bits: model.eval_bv_bits(bv)?,
                                signed: ty.signedness.is_signed(),
                                width: bv.width(),
                            }),
                            Sym::Bool(b) => model.eval_bool(b).map(Value::Bool),
                            Sym::Unit | Sym::Opaque => None,
                        };
                        let inputs = paths
                            .inputs
                            .iter()
                            .filter_map(|(name, sym)| Some((name.clone(), read(*sym)?)))
                            .collect();
                        found = Some((inputs, read(*value)));
                    }
                }
                CheckOutcome::Unknown(UnknownReason::ResourceLimit | UnknownReason::Other(_)) => {
                    unknown = true;
                }
                CheckOutcome::Unsat => {}
            }
            solver.pop(1);
            if found.is_some() {
                break;
            }
        }

        if !applicable {
            outcome.not_applicable += 1;
            continue;
        }
        let location = location(db, function);
        let signature = signature(db, function);
        match found {
            Some((inputs, result)) => outcome.matches.push(QueryMatch {
                function,
                location,
                signature,
                inputs,
                result,
            }),
            None if unknown => outcome.unknown.push((location, signature)),
            None => {}
        }
    }
    Ok(outcome)
}

fn location(db: &dyn HirDatabase, function: Function) -> String {
    let src = function.source(db);
    let path = db.file_relative_path(src.file_id);
    // The signature, not the node: a function node also owns the comments
    // and blank lines in front of it.
    let line = db
        .line_index(src.file_id)
        .line_col(src.value.signature_range().start())
        .line;
    format!("{path}:{}", line + 1)
}

fn signature(db: &dyn HirDatabase, function: Function) -> String {
    let src = function.source(db);
    let range = src.value.signature_range();
    let start = src.value.syntax().text_range().start();
    let text = src.value.syntax().text().to_string();
    let from = usize::from(range.start() - start);
    let to = usize::from(range.end() - start).min(text.len());
    text[from..to]
        .split_whitespace()
        .collect::<Vec<_>>()
        .join(" ")
}

/// Encodes the predicate over `env`. Integer literals take the type of the
/// value they meet (`result < 0` compares at `result`'s width); `hint` is
/// that type when the caller knows it.
fn encode<'ctx>(
    ctx: &'ctx Context,
    expr: &ast::Expr,
    env: &FxHashMap<String, Sym<'ctx>>,
    hint: Option<Sym<'ctx>>,
) -> Result<Sym<'ctx>, ()> {
    match expr.kind() {
        ast::ExprKind::Literal(lit) => match lit.kind() {
            ast::LiteralKind::Bool(b) => Ok(Sym::Bool(if b {
                ctx.bool_true()
            } else {
                ctx.bool_false()
            })),
            ast::LiteralKind::IntNumber(n) => {
                let (text, suffix) = n.split_into_parts();
                let (crate::Literal::Int(lit), errors) = crate::expr::integer_lit(text, suffix)
                else {
                    return Err(());
                };
                if !errors.is_empty() {
                    return Err(());
                }
                match hint {
                    Some(Sym::Int(bv, ty)) => Ok(Sym::Int(ctx.bv_lit(lit.value, bv.width()), ty)),
                    _ => Err(()),
                }
            }
            _ => Err(()),
        },
        ast::ExprKind::PathExpr(p) => {
            let segment = p.path().and_then(|p| p.segment()).ok_or(())?;
            match segment.kind() {
                Some(ast::PathSegmentKind::Name(name)) => match env.get(name.text().as_str()) {
                    Some(Sym::Opaque | Sym::Unit) | None => Err(()),
                    Some(value) => Ok(*value),
                },
                _ => Err(()),
            }
        }
        ast::ExprKind::ParenExpr(p) => encode(ctx, &p.expr().ok_or(())?, env, hint),
        ast::ExprKind::PrefixExpr(p) => {
            let inner = encode(ctx, &p.expr().ok_or(())?, env, hint)?;
            match (p.op_kind(), inner) {
                (Some(ast::PrefixOp::Not), Sym::Bool(b)) => Ok(Sym::Bool(b.not())),
                (Some(ast::PrefixOp::Neg), Sym::Int(bv, ty)) => Ok(Sym::Int(bv.neg(), ty)),
                (Some(ast::PrefixOp::BitNot), Sym::Int(bv, ty)) => Ok(Sym::Int(bv.bitnot(), ty)),
                _ => Err(()),
            }
        }
        ast::ExprKind::BinExpr(b) => {
            let (lhs, rhs) = (b.lhs().ok_or(())?, b.rhs().ok_or(())?);
            let op = b.op_kind().ok_or(())?;
            // Type the literal side from the other side.
            let int_hint = |v: Sym<'ctx>| Some(v).filter(|v| matches!(v, Sym::Int(..)));
            let (l, r) = if let Ok(l) = encode(ctx, &lhs, env, hint) {
                (l, encode(ctx, &rhs, env, int_hint(l))?)
            } else {
                let r = encode(ctx, &rhs, env, hint)?;
                (encode(ctx, &lhs, env, int_hint(r))?, r)
            };
            super::refinement::binary(op, l, r).ok_or(())
        }
        _ => Err(()),
    }
}
