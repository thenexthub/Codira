//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Finds the refinement predicate behind a parameter or return type (written
//!   inline, `x: i64 { v | v > 0 }`, or through a chain of `type` aliases) and
//!   encodes it over a concrete symbolic value.
//!
//! `TypeRef::Refinement` keeps only the base type, so the predicate is read
//! back from the syntax tree, the same way `crate::refinement_check` reads
//! it for its satisfiability check. What differs is the theory: there the
//! binder is an unbounded integer (a well-formedness question), here it is
//! the bit-vector of the base type's width, because the value it constrains
//! is a real machine integer.

use codira_hir_input::FileId;
use codira_smt::{BoolExpr, BvExpr, Context};
use codira_syntax::{
    ast::{self, AstNode, BinOp, PrefixOp, TypeAscriptionOwner},
    SyntaxNodePtr,
};

use super::exec::Sym;
use crate::{
    code_model::{Function, TypeAlias},
    expr::{integer_lit, Literal},
    in_file::InFile,
    resolve::HasResolver,
    HasSource, HirDatabase, Ty, TyKind,
};

/// A refinement predicate, as written.
#[derive(Debug, Clone)]
pub(crate) struct Refinement {
    /// The name the predicate uses for the refined value.
    pub binder: String,
    pub predicate: ast::Expr,
    /// The predicate's source text, for diagnostics.
    pub text: String,
    pub site: InFile<SyntaxNodePtr>,
}

/// How many aliases deep a refinement is looked for; a cycle is reported
/// elsewhere (`validate_acyclic`), this only has to terminate.
const MAX_ALIAS_DEPTH: usize = 16;

fn from_ast(file: FileId, type_ref: &ast::TypeRef) -> Option<Refinement> {
    match type_ref.kind() {
        ast::TypeRefKind::RefinementType(rt) => {
            let binder = rt.binder()?.text().to_string();
            let predicate = rt.predicate()?;
            Some(Refinement {
                binder,
                text: predicate.syntax().text().to_string(),
                site: InFile::new(file, SyntaxNodePtr::new(predicate.syntax())),
                predicate,
            })
        }
        ast::TypeRefKind::ParenType(p) => from_ast(file, &p.type_ref()?),
        _ => None,
    }
}

/// The refinement a type alias (or the alias it names, transitively)
/// stands for.
pub(crate) fn alias_refinement(
    db: &dyn HirDatabase,
    alias: TypeAlias,
    depth: usize,
) -> Option<Refinement> {
    if depth > MAX_ALIAS_DEPTH {
        return None;
    }
    let src = alias.source(db);
    let type_ref = src.value.type_ref()?;
    if let Some(found) = from_ast(src.file_id, &type_ref) {
        return Some(found);
    }
    let data = alias.data(db);
    let ty = Ty::from_hir(
        db,
        &alias.id.resolver(db),
        data.type_ref_map(),
        data.type_ref_id,
    )
    .0;
    match ty.interned() {
        TyKind::TypeAlias(next) => alias_refinement(db, *next, depth + 1),
        _ => None,
    }
}

/// The refinement behind an already-lowered type, if it names an alias.
fn ty_refinement(db: &dyn HirDatabase, ty: &Ty) -> Option<Refinement> {
    match ty.interned() {
        TyKind::TypeAlias(alias) => alias_refinement(db, *alias, 0),
        _ => None,
    }
}

/// The refinement of each ordinary parameter of `func`, in order.
pub(crate) fn param_refinements(db: &dyn HirDatabase, func: Function) -> Vec<Option<Refinement>> {
    let src = func.source(db);
    let data = func.data(db);
    let resolver = func.id.resolver(db);
    let ast_params: Vec<ast::Param> = src
        .value
        .param_list()
        .map(|list| list.params().collect())
        .unwrap_or_default();
    data.params()
        .iter()
        .enumerate()
        .map(|(idx, type_ref)| {
            if let Some(found) = ast_params
                .get(idx)
                .and_then(TypeAscriptionOwner::ascribed_type)
                .and_then(|t| from_ast(src.file_id, &t))
            {
                return Some(found);
            }
            let ty = Ty::from_hir(db, &resolver, data.type_ref_map(), *type_ref).0;
            ty_refinement(db, &ty)
        })
        .collect()
}

/// The refinement of `func`'s return type.
pub(crate) fn return_refinement(db: &dyn HirDatabase, func: Function) -> Option<Refinement> {
    let src = func.source(db);
    if let Some(found) = src
        .value
        .ret_type()
        .and_then(|r| r.type_ref())
        .and_then(|t| from_ast(src.file_id, &t))
    {
        return Some(found);
    }
    ty_refinement(db, &func.ret_type(db))
}

/// Encodes `refinement`'s predicate with its binder standing for `value`.
pub(crate) fn encode<'ctx>(
    ctx: &'ctx Context,
    refinement: &Refinement,
    value: Sym<'ctx>,
) -> Result<BoolExpr<'ctx>, String> {
    match lower(ctx, &refinement.binder, value, &refinement.predicate)? {
        Sym::Bool(b) => Ok(b),
        _ => Err(format!(
            "the refinement `{}` is not a boolean the verifier can model",
            refinement.text
        )),
    }
}

/// The width and signedness of the binder: literals in the predicate take
/// the refined value's type, since that is the only type in scope.
fn lower<'ctx>(
    ctx: &'ctx Context,
    binder: &str,
    value: Sym<'ctx>,
    expr: &ast::Expr,
) -> Result<Sym<'ctx>, String> {
    let unsupported = || {
        format!(
            "`{}` is outside what refinement predicates support",
            expr.syntax().text()
        )
    };
    let (width, int_ty) = match value {
        Sym::Int(bv, ty) => (bv.width(), Some(ty)),
        _ => (0, None),
    };
    match expr.kind() {
        ast::ExprKind::Literal(lit) => match lit.kind() {
            ast::LiteralKind::Bool(b) => Ok(Sym::Bool(if b {
                ctx.bool_true()
            } else {
                ctx.bool_false()
            })),
            ast::LiteralKind::IntNumber(n) => {
                let (text, suffix) = n.split_into_parts();
                let (Literal::Int(lit), errors) = integer_lit(text, suffix) else {
                    return Err(unsupported());
                };
                let Some(ty) = int_ty else {
                    return Err(unsupported());
                };
                if !errors.is_empty() {
                    return Err(unsupported());
                }
                Ok(Sym::Int(ctx.bv_lit(lit.value, width), ty))
            }
            _ => Err(unsupported()),
        },
        ast::ExprKind::PathExpr(p) => {
            let segment = p.path().and_then(|p| p.segment()).ok_or_else(unsupported)?;
            match segment.kind() {
                Some(ast::PathSegmentKind::Name(name)) if name.text() == binder => Ok(value),
                _ => Err(format!(
                    "`{}` is not `{binder}`, the only name a refinement predicate can use",
                    expr.syntax().text()
                )),
            }
        }
        ast::ExprKind::ParenExpr(p) => {
            lower(ctx, binder, value, &p.expr().ok_or_else(unsupported)?)
        }
        ast::ExprKind::PrefixExpr(p) => {
            let inner = lower(ctx, binder, value, &p.expr().ok_or_else(unsupported)?)?;
            match (p.op_kind(), inner) {
                (Some(PrefixOp::Not), Sym::Bool(b)) => Ok(Sym::Bool(b.not())),
                (Some(PrefixOp::Neg), Sym::Int(bv, ty)) => Ok(Sym::Int(bv.neg(), ty)),
                (Some(PrefixOp::BitNot), Sym::Int(bv, ty)) => Ok(Sym::Int(bv.bitnot(), ty)),
                _ => Err(unsupported()),
            }
        }
        ast::ExprKind::BinExpr(b) => {
            let lhs = lower(ctx, binder, value, &b.lhs().ok_or_else(unsupported)?)?;
            let rhs = lower(ctx, binder, value, &b.rhs().ok_or_else(unsupported)?)?;
            let op = b.op_kind().ok_or_else(unsupported)?;
            binary(op, lhs, rhs).ok_or_else(unsupported)
        }
        _ => Err(unsupported()),
    }
}

/// One binary operator over two encoded operands, or `None` when the
/// operator does not apply to them -- including two integers of different
/// widths, which Z3 would reject outright.
pub(crate) fn binary<'ctx>(op: BinOp, lhs: Sym<'ctx>, rhs: Sym<'ctx>) -> Option<Sym<'ctx>> {
    Some(match (lhs, rhs) {
        (Sym::Int(l, _), Sym::Int(r, _)) if l.width() != r.width() => return None,
        (Sym::Int(l, ty), Sym::Int(r, _)) => {
            let signed = ty.signedness.is_signed();
            let int = |v: BvExpr<'ctx>| Sym::Int(v, ty);
            match op {
                BinOp::Add => int(l.add(r)),
                BinOp::Subtract => int(l.sub(r)),
                BinOp::Multiply => int(l.mul(r)),
                BinOp::Divide if signed => int(l.sdiv(r)),
                BinOp::Divide => int(l.udiv(r)),
                BinOp::Remainder if signed => int(l.srem(r)),
                BinOp::Remainder => int(l.urem(r)),
                BinOp::BitwiseAnd => int(l.bitand(r)),
                BinOp::BitwiseOr => int(l.bitor(r)),
                BinOp::BitwiseXor => int(l.bitxor(r)),
                BinOp::Equals => Sym::Bool(l.eq(r)),
                BinOp::NotEqual => Sym::Bool(l.eq(r).not()),
                BinOp::Less if signed => Sym::Bool(l.slt(r)),
                BinOp::Less => Sym::Bool(l.ult(r)),
                BinOp::LessEqual if signed => Sym::Bool(l.sle(r)),
                BinOp::LessEqual => Sym::Bool(l.ule(r)),
                BinOp::Greater if signed => Sym::Bool(l.sgt(r)),
                BinOp::Greater => Sym::Bool(l.ugt(r)),
                BinOp::GreatEqual if signed => Sym::Bool(l.sge(r)),
                BinOp::GreatEqual => Sym::Bool(l.uge(r)),
                _ => return None,
            }
        }
        (Sym::Bool(l), Sym::Bool(r)) => match op {
            BinOp::BooleanAnd => Sym::Bool(l.and(r)),
            BinOp::BooleanOr => Sym::Bool(l.or(r)),
            BinOp::Equals => Sym::Bool(l.iff(r)),
            BinOp::NotEqual => Sym::Bool(l.xor(r)),
            _ => return None,
        },
        _ => return None,
    })
}
