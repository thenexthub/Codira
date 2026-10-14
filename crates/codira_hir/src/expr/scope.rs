//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
use std::sync::Arc;

use la_arena::{Arena, Idx};
use rustc_hash::FxHashMap;

use crate::{
    expr::{Body, Expr, ExprId, Pat, PatId, Statement},
    ids::DefWithBodyId,
    DefDatabase, Name,
};

/// The ID of a scope in an `ExprScopes`
pub type LocalScopeId = Idx<ScopeData>;

#[derive(Debug, PartialEq, Eq)]
pub struct ExprScopes {
    scopes: Arena<ScopeData>,
    scope_by_expr: FxHashMap<ExprId, LocalScopeId>,
}

#[derive(Debug, PartialEq, Eq)]
pub struct ScopeEntry {
    name: Name,
    pat: PatId,
}

impl ScopeEntry {
    pub(crate) fn name(&self) -> &Name {
        &self.name
    }

    pub(crate) fn pat(&self) -> PatId {
        self.pat
    }
}

#[derive(Debug, PartialEq, Eq)]
pub struct ScopeData {
    parent: Option<LocalScopeId>,
    entries: Vec<ScopeEntry>,
}

impl ExprScopes {
    pub(crate) fn expr_scopes_query(db: &dyn DefDatabase, def: DefWithBodyId) -> Arc<ExprScopes> {
        let body = db.body(def);
        Arc::new(ExprScopes::new(&body))
    }

    fn new(body: &Body) -> ExprScopes {
        let mut scopes = ExprScopes {
            scopes: Arena::default(),
            scope_by_expr: FxHashMap::default(),
        };
        let root = scopes.root_scope();
        if let Some(self_param) = body.self_param {
            scopes.add_bindings(body, root, self_param.0);
        }
        scopes.add_params_bindings(body, root, body.params().iter().map(|p| &p.0));
        compute_expr_scopes(body.body_expr(), body, &mut scopes, root);

        // `requires` sees the parameters; `ensures` additionally sees
        // `result`, bound in a scope of its own so it shadows a parameter
        // that happens to share the name.
        let contracts = body.contracts();
        for &clause in &contracts.requires {
            compute_expr_scopes(clause, body, &mut scopes, root);
        }
        if !contracts.ensures.is_empty() {
            let ensures_scope = scopes.new_scope(root);
            if let Some(result) = contracts.result_pat {
                scopes.add_bindings(body, ensures_scope, result);
            }
            for &clause in &contracts.ensures {
                compute_expr_scopes(clause, body, &mut scopes, ensures_scope);
            }
        }
        scopes
    }

    pub(crate) fn entries(&self, scope: LocalScopeId) -> &[ScopeEntry] {
        &self.scopes[scope].entries
    }

    pub(crate) fn scope_chain(
        &'_ self,
        scope: Option<LocalScopeId>,
    ) -> impl Iterator<Item = LocalScopeId> + '_ {
        std::iter::successors(scope, move |&scope| self.scopes[scope].parent)
    }

    pub(crate) fn scope_for(&self, expr: ExprId) -> Option<LocalScopeId> {
        self.scope_by_expr.get(&expr).copied()
    }

    pub(crate) fn scope_by_expr(&self) -> &FxHashMap<ExprId, LocalScopeId> {
        &self.scope_by_expr
    }

    fn root_scope(&mut self) -> LocalScopeId {
        self.scopes.alloc(ScopeData {
            parent: None,
            entries: vec![],
        })
    }

    fn new_scope(&mut self, parent: LocalScopeId) -> LocalScopeId {
        self.scopes.alloc(ScopeData {
            parent: Some(parent),
            entries: vec![],
        })
    }

    fn add_bindings(&mut self, body: &Body, scope: LocalScopeId, pat: PatId) {
        match &body[pat] {
            Pat::Bind { name, .. } => {
                // bind can have a sub pattern, but it's actually not allowed
                // to bind to things in there
                let entry = ScopeEntry {
                    name: name.clone(),
                    pat,
                };
                self.scopes[scope].entries.push(entry);
            }
            p => p.walk_child_pats(|pat| self.add_bindings(body, scope, pat)),
        }
    }

    fn add_params_bindings<'a>(
        &mut self,
        body: &Body,
        scope: LocalScopeId,
        params: impl Iterator<Item = &'a PatId>,
    ) {
        params.for_each(|pat| self.add_bindings(body, scope, *pat));
    }

    fn set_scope(&mut self, node: ExprId, scope: LocalScopeId) {
        self.scope_by_expr.insert(node, scope);
    }
}

fn compute_block_scopes(
    block: ExprId,
    statements: &[Statement],
    tail: Option<ExprId>,
    body: &Body,
    scopes: &mut ExprScopes,
    mut scope: LocalScopeId,
) {
    // Ghost lets live outside `statements` (see `BodyContracts`), each
    // tagged with the statement index it precedes; they open scopes exactly
    // as an ordinary `let` in that position would.
    let ghosts = body.contracts().ghost_lets.get(&block);
    let ghosts_at = |idx: usize| {
        ghosts
            .into_iter()
            .flatten()
            .filter(move |(pos, _)| *pos == idx)
            .map(|(_, stmt)| stmt)
    };
    for stmt in statements
        .iter()
        .enumerate()
        .flat_map(|(idx, stmt)| ghosts_at(idx).chain(std::iter::once(stmt)))
        .chain(ghosts_at(statements.len()))
    {
        match stmt {
            Statement::Let {
                pat, initializer, ..
            } => {
                if let Some(expr) = initializer {
                    scopes.set_scope(*expr, scope);
                    compute_expr_scopes(*expr, body, scopes, scope);
                }
                scope = scopes.new_scope(scope);
                scopes.add_bindings(body, scope, *pat);
            }
            Statement::Expr(expr) => {
                scopes.set_scope(*expr, scope);
                compute_expr_scopes(*expr, body, scopes, scope);
            }
        }
    }
    if let Some(expr) = tail {
        compute_expr_scopes(expr, body, scopes, scope);
    }
}

fn compute_expr_scopes(expr: ExprId, body: &Body, scopes: &mut ExprScopes, scope: LocalScopeId) {
    scopes.set_scope(expr, scope);
    match &body[expr] {
        Expr::Block { statements, tail } => {
            compute_block_scopes(expr, statements, *tail, body, scopes, scope);
        }
        // Each handler arm binds its operation's parameters in a scope of its
        // own, visible only to that arm's body.
        Expr::Handle {
            body: handled,
            arms,
        } => {
            compute_expr_scopes(*handled, body, scopes, scope);
            for arm in arms {
                let arm_scope = scopes.new_scope(scope);
                scopes.add_params_bindings(body, arm_scope, arm.params.iter());
                compute_expr_scopes(arm.body, body, scopes, arm_scope);
            }
        }
        // A `select` arm's binding is visible only in that arm's body; the
        // operands are evaluated before any arm runs.
        Expr::Select { arms } => {
            for arm in arms {
                for operand in arm.operands() {
                    compute_expr_scopes(operand, body, scopes, scope);
                }
            }
            for arm in arms {
                let arm_scope = match arm.binding {
                    Some(binding) => {
                        let arm_scope = scopes.new_scope(scope);
                        scopes.add_bindings(body, arm_scope, binding);
                        arm_scope
                    }
                    None => scope,
                };
                compute_expr_scopes(arm.body, body, scopes, arm_scope);
            }
        }
        // The iterable is evaluated outside the loop, so it cannot see the
        // loop variable; the body can.
        Expr::For {
            pat,
            iterable,
            body: loop_body,
        } => {
            compute_expr_scopes(*iterable, body, scopes, scope);
            let loop_scope = scopes.new_scope(scope);
            scopes.add_bindings(body, loop_scope, *pat);
            compute_expr_scopes(*loop_body, body, scopes, loop_scope);
        }
        e => e.walk_child_exprs(|e| compute_expr_scopes(e, body, scopes, scope)),
    };
    // `choose x where pred`: the predicate sees everything the expression
    // does, plus `x`.
    if let Some(choose) = body.contracts().chooses.get(&expr) {
        let inner = scopes.new_scope(scope);
        scopes.add_bindings(body, inner, choose.binder);
        compute_expr_scopes(choose.predicate, body, scopes, inner);
    }
    // A loop's `invariant`/`decreases` clauses are evaluated at the loop
    // head, so they see exactly what the loop expression itself sees.
    if let Some(contract) = body.contracts().loops.get(&expr) {
        for &clause in contract.invariants.iter().chain(contract.decreases.iter()) {
            compute_expr_scopes(clause, body, scopes, scope);
        }
    }
}
