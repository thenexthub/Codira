//! Copyright (c) 2026 Omnira CJSC
//!
//! General HIR `Body` -> `codira_mir::Generator` lowering.
//!
//! Generalizes `comptime_fold`'s single-tail-expression walker to real
//! function bodies: function parameters become `codira_mir::OpKind::Arg`,
//! the function's own generic parameters become `OpKind::ParamRef`, and
//! `let`-bindings/multi-statement blocks are supported (not just one tail
//! expression). This is the prerequisite `spec/KGEN_SUPERSET_STATUS.md`
//! roadmap item 4/session-2-item-#17 -- without it, elaboration
//! (`codira_comptime::elaborate`) has nothing but hand-built `Generator`s
//! to operate on.
//!
//! Scope, precisely (same honesty convention as `comptime_fold`): supports
//! literals, arithmetic/comparison/logical operators, unary neg/not,
//! `if`/`else`, references to the function's own parameters and generic
//! parameters, and `let`-bindings with a simple `Pat::Bind` pattern and an
//! initializer. **Not** supported (lowering returns `None`, honestly,
//! rather than producing a wrong or partial `Generator`): calls to other
//! functions (`codira_mir` has no `Call` op yet), loops-as-statements,
//! struct/array construction, uninitialized `let` bindings, non-trivial
//! patterns, and any expression-statement (no side effects are modeled).

use std::sync::Arc;

use rustc_hash::FxHashMap;
use smol_str::SmolStr;

use crate::{
    code_model::Function,
    expr::{
        ArithOp, BinaryOp, Body as HirBody, CmpOp, Expr, ExprId, Literal, LiteralInt, LogicOp,
        Ordering, Pat, Statement, UnaryOp,
    },
    HirDatabase, Name,
};

/// Salsa query implementation for `HirDatabase::mir_generator` -- see
/// `db.rs`. Memoized per-`Function`: unchanged functions (by salsa's
/// usual dependency tracking, ultimately rooted in source text) are never
/// re-lowered.
pub(crate) fn mir_generator_query(
    db: &dyn HirDatabase,
    func: Function,
) -> Option<Arc<codira_mir::Generator>> {
    lower_function_to_generator(db, func).map(Arc::new)
}

/// Salsa query implementation for `HirDatabase::elaborate_generator` --
/// see `db.rs`. This is the concrete "incremental elaboration cache"
/// architecture doc §4.1 describes: memoized per `(Function, bindings)`,
/// so re-elaborating the same function with the same concrete parameter
/// values under `codira build --watch` is a cache hit, not repeated work --
/// for free, from salsa, rather than a hand-built DAG-of-expansions cache
/// (contrast KGEN's own `DesignOverview.md` "Dynamic Programming /
/// Caching" section, which describes wanting to build exactly this as
/// future work).
pub(crate) fn elaborate_generator_query(
    db: &dyn HirDatabase,
    func: Function,
    bindings: Vec<(SmolStr, codira_comptime::Value)>,
) -> Option<Arc<codira_mir::Body>> {
    let generator = db.mir_generator(func)?;
    Some(Arc::new(codira_comptime::elaborate(&generator, &bindings)))
}

/// Attempts to lower `func`'s body to a `codira_mir::Generator`. `None`
/// means the body uses a construct outside the scope described in this
/// module's doc comment -- not an error, just "not liftable yet."
pub fn lower_function_to_generator(
    db: &dyn HirDatabase,
    func: Function,
) -> Option<codira_mir::Generator> {
    let data = func.data(db);
    let hir_body = func.body(db);

    let mut param_index: FxHashMap<Name, u32> = FxHashMap::default();
    for (i, (pat_id, _ty)) in hir_body.params().iter().enumerate() {
        if let Pat::Bind { name } = &hir_body[*pat_id] {
            param_index.insert(name.clone(), i as u32);
        }
    }

    let generic_names: FxHashMap<Name, SmolStr> = data
        .generic_params()
        .iter()
        .map(|name| (name.clone(), name.to_string().into()))
        .collect();

    let mut lowerer = Lowerer {
        hir_body: &hir_body,
        param_index,
        generic_names,
        locals: FxHashMap::default(),
    };

    let mut mir_body = codira_mir::Body::new();
    lowerer.lower_expr(hir_body.body_expr(), &mut mir_body)?;

    let params = data
        .generic_params()
        .iter()
        .map(|name| codira_mir::GeneratorParam {
            name: name.to_string().into(),
        })
        .collect();

    Some(codira_mir::Generator {
        name: data.name().to_string().into(),
        params,
        body: mir_body,
    })
}

struct Lowerer<'a> {
    hir_body: &'a HirBody,
    param_index: FxHashMap<Name, u32>,
    generic_names: FxHashMap<Name, SmolStr>,
    /// `let`-bound locals: name -> the HIR `ExprId` of its initializer.
    ///
    /// Deliberately *not* a `codira_mir::OpId`: ops are indices into one
    /// specific `Body`'s arena, but a local can be referenced from inside
    /// a nested `cf.if` branch, which builds into its own fresh, separate
    /// `Body` (`codira_mir` has no cross-region SSA value referencing --
    /// see architecture doc §2, regions are plain nested arenas, not
    /// dominance-tracked blocks). So instead of storing *where* a local's
    /// value was computed, this stores *how* to compute it again, and
    /// every reference re-lowers the initializer fresh into whichever
    /// `Body` is currently being built. Fine for this pass's side-effect-
    /// free restricted subset (re-evaluating is observably identical to
    /// reusing a value) -- see module doc for what's out of scope.
    locals: FxHashMap<Name, ExprId>,
}

impl Lowerer<'_> {
    fn lower_expr(&mut self, id: ExprId, body: &mut codira_mir::Body) -> Option<codira_mir::OpId> {
        use codira_mir::{Attr, OpKind, Region};

        match &self.hir_body[id] {
            Expr::Literal(Literal::Bool(b)) => Some(body.push(OpKind::Const(Attr::Bool(*b)), [])),
            Expr::Literal(Literal::Int(LiteralInt { value, .. })) => {
                let v = i64::try_from(*value).ok()?;
                Some(body.push(OpKind::Const(Attr::Int(v)), []))
            }

            Expr::Path(path) => {
                let name = path.as_ident()?;
                if let Some(&idx) = self.param_index.get(name) {
                    Some(body.push(OpKind::Arg(idx), []))
                } else if let Some(param_name) = self.generic_names.get(name) {
                    Some(body.push(OpKind::ParamRef(param_name.clone()), []))
                } else if let Some(&init) = self.locals.get(name) {
                    self.lower_expr(init, body)
                } else {
                    None
                }
            }

            Expr::UnaryOp { expr, op } => {
                let inner = self.lower_expr(*expr, body)?;
                let kind = match op {
                    UnaryOp::Neg => OpKind::Neg,
                    UnaryOp::Not => OpKind::Not,
                };
                Some(body.push(kind, [inner]))
            }

            Expr::BinaryOp {
                lhs,
                rhs,
                op: Some(op),
            } => {
                let lhs_id = self.lower_expr(*lhs, body)?;
                let rhs_id = self.lower_expr(*rhs, body)?;
                let kind = binary_op_kind(*op)?;
                Some(body.push(kind, [lhs_id, rhs_id]))
            }

            Expr::If {
                condition,
                then_branch,
                else_branch,
            } => {
                let cond_id = self.lower_expr(*condition, body)?;

                let mut then_body = codira_mir::Body::new();
                self.lower_expr(*then_branch, &mut then_body)?;

                let mut else_body = codira_mir::Body::new();
                if let Some(else_branch) = else_branch {
                    self.lower_expr(*else_branch, &mut else_body)?;
                }

                Some(body.push_with_regions(
                    OpKind::If,
                    [cond_id],
                    [Region::new(then_body), Region::new(else_body)],
                ))
            }

            Expr::Block { statements, tail } => {
                for stmt in statements {
                    match stmt {
                        Statement::Let {
                            pat,
                            initializer: Some(init),
                            ..
                        } => {
                            // Validate the initializer lowers *now*, into
                            // a throwaway probe body, even though the
                            // local is only actually re-lowered (into
                            // whichever body needs it) on first reference
                            // below. Without this, an unused local whose
                            // initializer contains an unsupported
                            // construct (a call, say) would silently drop
                            // it instead of honestly bailing -- and unlike
                            // a bare expression-statement (already
                            // rejected outright), a `let` initializer
                            // could easily hide a side effect nobody
                            // meant to discard.
                            self.lower_expr(*init, &mut codira_mir::Body::new())?;
                            match &self.hir_body[*pat] {
                                Pat::Bind { name } => {
                                    self.locals.insert(name.clone(), *init);
                                }
                                // Non-trivial patterns (tuple-struct
                                // destructuring, wildcards standing in for
                                // side-effect-only initializers, etc.) are
                                // out of scope -- see module doc.
                                _ => return None,
                            }
                        }
                        // Uninitialized `let` and expression-statements
                        // are out of scope -- see module doc.
                        Statement::Let {
                            initializer: None, ..
                        }
                        | Statement::Expr(_) => return None,
                    }
                }
                match tail {
                    Some(tail) => self.lower_expr(*tail, body),
                    None => Some(body.push(OpKind::Const(Attr::Unit), [])),
                }
            }

            // `Call`, `MethodCall`, `Index`, `Array`, `RecordLit`, `Field`,
            // `Loop`, `While`, `Return`, `Break`, `Missing`, and
            // string/float literals are all out of scope for this pass --
            // see module doc.
            _ => None,
        }
    }
}

fn binary_op_kind(op: BinaryOp) -> Option<codira_mir::OpKind> {
    use codira_mir::OpKind;
    Some(match op {
        BinaryOp::ArithOp(ArithOp::Add) => OpKind::Add,
        BinaryOp::ArithOp(ArithOp::Subtract) => OpKind::Sub,
        BinaryOp::ArithOp(ArithOp::Multiply) => OpKind::Mul,
        BinaryOp::ArithOp(ArithOp::Divide) => OpKind::Div,
        BinaryOp::ArithOp(ArithOp::Remainder) => OpKind::Rem,
        // Bitwise/shift ops have no codira_mir core.* op yet -- same
        // scoping as comptime_fold's identical restriction.
        BinaryOp::ArithOp(
            ArithOp::LeftShift | ArithOp::RightShift | ArithOp::BitAnd | ArithOp::BitOr
            | ArithOp::BitXor,
        ) => return None,
        BinaryOp::LogicOp(LogicOp::And) => OpKind::And,
        BinaryOp::LogicOp(LogicOp::Or) => OpKind::Or,
        BinaryOp::CmpOp(CmpOp::Eq { negated: false }) => OpKind::Eq,
        BinaryOp::CmpOp(CmpOp::Eq { negated: true }) => OpKind::Ne,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Less,
            strict: true,
        }) => OpKind::Lt,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Less,
            strict: false,
        }) => OpKind::Le,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Greater,
            strict: true,
        }) => OpKind::Gt,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Greater,
            strict: false,
        }) => OpKind::Ge,
        BinaryOp::Assignment { .. } => return None,
    })
}

#[cfg(test)]
mod tests;
