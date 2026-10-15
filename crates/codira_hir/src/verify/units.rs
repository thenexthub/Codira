//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Units of measure: dimension checking for numbers annotated `T
//!   unit(<dimension>)`, entirely in the front end.
//!
//! `x: f64 unit(m / s)` gives `x` the dimension metre per second. A
//! dimension is a product of named base units with integer exponents,
//! written with `*`, `/`, parentheses and `1`: `kg * m / (s * s)`. Any
//! identifier is a base unit; units are not declared. A `type` alias can
//! name one: `type Speed = f64 unit(m / s);`.
//!
//! The rules, checked in every function that mentions a unit (its own
//! signature or `let`s, or a callee's signature):
//!
//! * `+`, `-`, comparisons, assignment, `if` arms, arguments and results need
//!   equal dimensions;
//! * `*` adds exponents and `/` subtracts them;
//! * an unannotated numeric parameter, `let` or result is dimensionless;
//! * a numeric literal takes whatever dimension it meets (`x + 1.0` is fine for
//!   any `x`; F# would want `1.0<m>`), and is dimensionless under `*` and `/`
//!   (`2.0 * x` keeps `x`'s dimension);
//! * a value the checker cannot follow (a field, an array element, a call
//!   through anything but a plain function) is unconstrained.
//!
//! The qualifier lowers to its base type, so units cost nothing at run time
//! and code generation never sees them.

use std::{collections::BTreeMap, fmt, sync::Arc};

use codira_syntax::ast::{self, AstNode, BinOp, TypeAscriptionOwner};

use super::exec::Frame;
use crate::{
    code_model::{Function, TypeAlias},
    diagnostics::{DiagnosticSink, UnitMismatch},
    expr::{ArithOp, BinaryOp, Statement},
    Expr, ExprId, HasSource, HirDatabase, Pat, PatId, Ty, TyKind,
};

/// A product of base units with non-zero integer exponents.
#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct Dim(BTreeMap<String, i32>);

impl Dim {
    fn one() -> Dim {
        Dim::default()
    }

    fn base(name: &str) -> Dim {
        Dim(BTreeMap::from([(name.to_string(), 1)]))
    }

    fn combine(&self, other: &Dim, sign: i32) -> Dim {
        let mut out = self.0.clone();
        for (unit, exp) in &other.0 {
            let entry = out.entry(unit.clone()).or_insert(0);
            *entry += sign * exp;
            if *entry == 0 {
                out.remove(unit);
            }
        }
        Dim(out)
    }
}

impl fmt::Display for Dim {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let render = |positive: bool| -> Vec<String> {
            self.0
                .iter()
                .filter(|(_, e)| (**e > 0) == positive)
                .map(|(u, e)| {
                    let e = e.abs();
                    if e == 1 {
                        u.clone()
                    } else {
                        format!("{u}^{e}")
                    }
                })
                .collect()
        };
        let num = render(true);
        let den = render(false);
        let num = if num.is_empty() {
            "1".to_string()
        } else {
            num.join("*")
        };
        if den.is_empty() {
            f.write_str(&num)
        } else if den.len() == 1 {
            write!(f, "{num}/{}", den[0])
        } else {
            write!(f, "{num}/({})", den.join("*"))
        }
    }
}

/// What the checker knows about a value's dimension.
#[derive(Debug, Clone, PartialEq, Eq)]
enum D {
    Known(Dim),
    /// A literal: takes the dimension it meets.
    Poly,
    /// Not followed: unconstrained.
    Unknown,
}

/// Parses a dimension expression (`m / (s * s)`).
fn parse_dim(expr: &ast::Expr) -> Option<Dim> {
    match expr.kind() {
        ast::ExprKind::PathExpr(p) => {
            let segment = p.path()?.segment()?;
            match segment.kind()? {
                ast::PathSegmentKind::Name(name) => Some(Dim::base(&name.text())),
                _ => None,
            }
        }
        ast::ExprKind::Literal(lit) => match lit.kind() {
            ast::LiteralKind::IntNumber(n) if codira_syntax::AstToken::text(&n) == "1" => {
                Some(Dim::one())
            }
            _ => None,
        },
        ast::ExprKind::ParenExpr(p) => parse_dim(&p.expr()?),
        ast::ExprKind::BinExpr(b) => {
            let l = parse_dim(&b.lhs()?)?;
            let r = parse_dim(&b.rhs()?)?;
            match b.op_kind()? {
                BinOp::Multiply => Some(l.combine(&r, 1)),
                BinOp::Divide => Some(l.combine(&r, -1)),
                _ => None,
            }
        }
        _ => None,
    }
}

/// The dimension a written type carries, following `type` aliases.
fn dim_of_type_ref(
    db: &dyn HirDatabase,
    file_ty: Option<ast::TypeRef>,
    ty: Option<&Ty>,
) -> Option<Dim> {
    if let Some(type_ref) = file_ty {
        match type_ref.kind() {
            ast::TypeRefKind::UnitType(u) => return u.dimension().and_then(|d| parse_dim(&d)),
            ast::TypeRefKind::ParenType(p) => return dim_of_type_ref(db, p.type_ref(), ty),
            _ => {}
        }
    }
    match ty?.interned() {
        TyKind::TypeAlias(alias) => alias_dim(db, *alias, 0),
        _ => None,
    }
}

fn alias_dim(db: &dyn HirDatabase, alias: TypeAlias, depth: usize) -> Option<Dim> {
    if depth > 16 {
        return None;
    }
    let src = alias.source(db);
    let type_ref = src.value.type_ref()?;
    if let ast::TypeRefKind::UnitType(u) = type_ref.kind() {
        return u.dimension().and_then(|d| parse_dim(&d));
    }
    let data = alias.data(db);
    let ty = Ty::from_hir(
        db,
        &crate::resolve::HasResolver::resolver(alias.id, db),
        data.type_ref_map(),
        data.type_ref_id,
    )
    .0;
    match ty.interned() {
        TyKind::TypeAlias(next) => alias_dim(db, *next, depth + 1),
        _ => None,
    }
}

fn is_numeric(ty: &Ty) -> bool {
    matches!(ty.interned(), TyKind::Int(_) | TyKind::Float(_))
}

/// A function's declared dimensions: per parameter, and of the result.
/// `None` where the type is not numeric (dimensions do not apply).
#[derive(Debug, Clone)]
struct Signature {
    params: Vec<Option<Dim>>,
    result: Option<Dim>,
    /// Whether any of it was written with `unit(..)`.
    annotated: bool,
}

fn signature(db: &dyn HirDatabase, func: Function) -> Signature {
    let src = func.source(db).value;
    let sig_tys = db.callable_sig(func.into());
    let data = func.data(db);
    let resolver = crate::resolve::HasResolver::resolver(func.id, db);
    let ast_params: Vec<ast::Param> = src
        .param_list()
        .map(|list| list.params().collect())
        .unwrap_or_default();
    let mut annotated = false;
    let mut params = Vec::new();
    for (idx, type_ref) in data.params().iter().enumerate() {
        let raw = Ty::from_hir(db, &resolver, data.type_ref_map(), *type_ref).0;
        let declared = dim_of_type_ref(
            db,
            ast_params
                .get(idx)
                .and_then(TypeAscriptionOwner::ascribed_type),
            Some(&raw),
        );
        annotated |= declared.is_some();
        let numeric = sig_tys.params().get(idx).is_some_and(is_numeric);
        params.push(if numeric {
            Some(declared.unwrap_or_default())
        } else {
            None
        });
    }
    let raw_ret = Ty::from_hir(db, &resolver, data.type_ref_map(), *data.ret_type()).0;
    let declared = dim_of_type_ref(
        db,
        src.ret_type().and_then(|r| r.type_ref()),
        Some(&raw_ret),
    );
    annotated |= declared.is_some();
    let result = is_numeric(sig_tys.ret()).then(|| declared.unwrap_or_default());
    Signature {
        params,
        result,
        annotated,
    }
}

struct Checker<'a> {
    db: &'a dyn HirDatabase,
    frame: Frame,
    locals: rustc_hash::FxHashMap<PatId, D>,
    /// The function's declared result dimension, for `return`.
    result: Option<Dim>,
    errors: Vec<(ExprId, String)>,
}

pub(super) fn unit_diagnostics(
    db: &dyn HirDatabase,
    func: Function,
    sink: &mut DiagnosticSink<'_>,
) {
    if func.is_extern(db) {
        return;
    }
    let frame = Frame::new(db, func);
    let sig = signature(db, func);
    let mut checker = Checker {
        db,
        frame,
        locals: rustc_hash::FxHashMap::default(),
        result: sig.result.clone(),
        errors: Vec::new(),
    };
    let body = Arc::clone(&checker.frame.body);
    // Only functions that mention units are checked at all.
    let mentions = sig.annotated
        || body.pats().any(|(pat, _)| checker.let_unit(pat).is_some())
        || body.exprs().any(|(id, e)| {
            matches!(e, Expr::Call { .. })
                && checker
                    .callee(id)
                    .is_some_and(|f| signature(db, f).annotated)
        });
    if !mentions {
        return;
    }
    for (idx, (pat, _)) in body.params().iter().enumerate() {
        let d = sig
            .params
            .get(idx)
            .cloned()
            .flatten()
            .map_or(D::Unknown, D::Known);
        checker.locals.insert(*pat, d);
    }
    let result = checker.dim(body.body_expr());
    if let Some(expected) = &sig.result {
        if !checker.frame.infer[body.body_expr()].is_never() {
            let tail = checker.tail_of(body.body_expr());
            checker.unify(tail, &D::Known(expected.clone()), &result, "returned");
        }
    }
    let mut seen = rustc_hash::FxHashSet::default();
    for (expr, message) in checker.errors {
        if !seen.insert((expr, message.clone())) {
            continue;
        }
        if let Some(site) = checker.frame.site(expr) {
            sink.push(UnitMismatch {
                file: site.file_id,
                site: site.value,
                message,
            });
        }
    }
}

impl Checker<'_> {
    fn callee(&self, call: ExprId) -> Option<Function> {
        match &self.frame.body[call] {
            Expr::Call { callee, .. } => match self.frame.infer[*callee].interned() {
                TyKind::FnDef(crate::CallableDef::Function(f), _) => Some(*f),
                _ => None,
            },
            _ => None,
        }
    }

    /// The dimension a `let` binding `pat` declares, if it declares one.
    fn let_unit(&self, pat: PatId) -> Option<Dim> {
        let src = self.frame.source_map.pat_syntax(pat)?;
        let root = self.db.parse(src.file_id).syntax_node();
        let node = src.value.either(
            |p| p.to_node(&root).syntax().clone(),
            |p| p.to_node(&root).syntax().clone(),
        );
        let let_stmt = node.ancestors().find_map(ast::LetStmt::cast)?;
        dim_of_type_ref(self.db, let_stmt.ascribed_type(), None)
    }

    fn tail_of(&self, mut expr: ExprId) -> ExprId {
        while let Expr::Block {
            tail: Some(tail), ..
        } = &self.frame.body[expr]
        {
            expr = *tail;
        }
        expr
    }

    /// Requires `a` and `b` to agree; returns the more precise of the two.
    fn unify(&mut self, at: ExprId, a: &D, b: &D, what: &str) -> D {
        match (a, b) {
            (D::Known(x), D::Known(y)) if x != y => {
                self.errors.push((
                    at,
                    format!("dimension mismatch: {what} `{y}` where `{x}` is required"),
                ));
                a.clone()
            }
            (D::Known(_), _) => a.clone(),
            (_, D::Known(_)) => b.clone(),
            (D::Poly, _) | (_, D::Poly) => D::Poly,
            _ => D::Unknown,
        }
    }

    fn dim(&mut self, expr: ExprId) -> D {
        let body = Arc::clone(&self.frame.body);
        match &body[expr] {
            Expr::Literal(_) => D::Poly,
            Expr::Path(path) => self
                .frame
                .resolve_local(self.db, expr, path)
                .and_then(|pat| self.locals.get(&pat).cloned())
                .unwrap_or(D::Unknown),
            Expr::UnaryOp { expr: inner, .. } | Expr::Cast { expr: inner, .. } => self.dim(*inner),
            Expr::BinaryOp { lhs, rhs, op } => {
                let l = self.dim(*lhs);
                let r = self.dim(*rhs);
                match op {
                    Some(BinaryOp::ArithOp(ArithOp::Add | ArithOp::Subtract)) => {
                        self.unify(*rhs, &l, &r, "added or subtracted")
                    }
                    Some(BinaryOp::ArithOp(ArithOp::Multiply)) => match (l, r) {
                        (D::Known(a), D::Known(b)) => D::Known(a.combine(&b, 1)),
                        (D::Known(a), D::Poly) | (D::Poly, D::Known(a)) => D::Known(a),
                        (D::Poly, D::Poly) => D::Poly,
                        _ => D::Unknown,
                    },
                    Some(BinaryOp::ArithOp(ArithOp::Divide)) => match (l, r) {
                        (D::Known(a), D::Known(b)) => D::Known(a.combine(&b, -1)),
                        (D::Known(a), D::Poly) => D::Known(a),
                        (D::Poly, D::Known(b)) => D::Known(Dim::one().combine(&b, -1)),
                        (D::Poly, D::Poly) => D::Poly,
                        _ => D::Unknown,
                    },
                    Some(BinaryOp::ArithOp(ArithOp::Remainder)) => {
                        self.unify(*rhs, &l, &r, "taken modulo")
                    }
                    Some(BinaryOp::CmpOp(_)) => {
                        self.unify(*rhs, &l, &r, "compared");
                        D::Unknown
                    }
                    Some(BinaryOp::Assignment { op }) => {
                        if let (Expr::Path(path), None | Some(ArithOp::Add | ArithOp::Subtract)) =
                            (&body[*lhs], op)
                        {
                            if let Some(pat) = self.frame.resolve_local(self.db, *lhs, path) {
                                let current = self.locals.get(&pat).cloned().unwrap_or(D::Unknown);
                                let merged = self.unify(*rhs, &current, &r, "assigned");
                                self.locals.insert(pat, merged);
                            }
                        }
                        D::Unknown
                    }
                    _ => D::Unknown,
                }
            }
            Expr::Call { args, .. } => {
                let dims: Vec<D> = args.iter().map(|a| self.dim(*a)).collect();
                let Some(callee) = self.callee(expr) else {
                    return D::Unknown;
                };
                let sig = signature(self.db, callee);
                let name = callee.name(self.db).to_string();
                for ((arg, d), expected) in args.iter().zip(&dims).zip(&sig.params) {
                    if let Some(expected) = expected {
                        self.unify(
                            *arg,
                            &D::Known(expected.clone()),
                            d,
                            &format!("argument of `{name}` is"),
                        );
                    }
                }
                sig.result.map_or(D::Unknown, D::Known)
            }
            Expr::If {
                condition,
                then_branch,
                else_branch,
            } => {
                self.dim(*condition);
                let a = self.dim(*then_branch);
                match else_branch {
                    Some(e) => {
                        let b = self.dim(*e);
                        self.unify(*e, &a, &b, "an `if` arm is")
                    }
                    None => D::Unknown,
                }
            }
            Expr::Block { statements, tail } => {
                for stmt in statements {
                    match stmt {
                        Statement::Let {
                            pat, initializer, ..
                        } => {
                            let init = initializer.map_or(D::Unknown, |i| self.dim(i));
                            let declared = self.let_unit(*pat);
                            let numeric = is_numeric(&self.frame.infer[*pat]);
                            let d = match (declared, initializer) {
                                (Some(dim), Some(i)) => {
                                    self.unify(*i, &D::Known(dim.clone()), &init, "bound");
                                    D::Known(dim)
                                }
                                (Some(dim), None) => D::Known(dim),
                                // A literal-initialised binding takes its
                                // dimension from what is later assigned or
                                // combined with it (`var total = 0.0;
                                // total += d`).
                                (None, _) if numeric => init,
                                (None, _) => D::Unknown,
                            };
                            if matches!(&body[*pat], Pat::Bind { .. }) {
                                self.locals.insert(*pat, d);
                            }
                        }
                        Statement::Expr(e) => {
                            self.dim(*e);
                        }
                    }
                }
                tail.map_or(D::Unknown, |t| self.dim(t))
            }
            Expr::Return { expr: Some(value) } => {
                let d = self.dim(*value);
                if let Some(expected) = self.result.clone() {
                    self.unify(*value, &D::Known(expected), &d, "returned");
                }
                D::Unknown
            }
            other => {
                let mut children = Vec::new();
                other.walk_child_exprs(|c| children.push(c));
                for c in children {
                    self.dim(c);
                }
                D::Unknown
            }
        }
    }
}
