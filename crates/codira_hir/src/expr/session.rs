//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Session types in function bodies: lowering `offer`, recognising
//!   `S.open()`, and the checker that follows every endpoint through its
//!   protocol along every control-flow path.
//!
//! # The surface
//!
//! * `session S { .. }` declares a protocol (see `code_model::session`).
//! * `let (a, b) = S.open()` -- inside a `scope` -- creates a connected pair of
//!   endpoints: `a` follows `S`, `b` follows `dual S`. Like a channel, the pair
//!   belongs to the scope and is freed when it ends.
//! * `ep <- v` sends, `<-ep` receives, `ep.choose(k)` takes branch `k` (an
//!   integer literal) of a `choose`, and `offer ep { .. } or { .. }` runs the
//!   block for the branch the other side chose.
//! * An endpoint is passed to a function or to `spawn` whose parameter is
//!   declared with the endpoint's session type -- `S` or `dual S` -- and only
//!   before the endpoint has been used: the callee then owns it and must take
//!   it to the end of the protocol.
//!
//! # What the checker enforces
//!
//! The checker runs on the body before type inference (inference reads the
//! payload type of each send and receive from it) and tracks, for every
//! endpoint in scope, the point of its protocol it has reached:
//!
//! * every operation must be the one the protocol expects next (a `send` of the
//!   declared type, a `recv`, a `choose` with a branch that exists, an `offer`
//!   with one block per branch);
//! * nothing may be done with an endpoint whose protocol has ended, or that has
//!   been passed to a function or task;
//! * the branches of an `if`, a `select` or an `offer`, and the two sides of
//!   `&&`/`||`, must leave every endpoint at the same point (an `if` without
//!   `else` must not move it at all);
//! * a loop must leave every endpoint where it found it, at the end of the body
//!   and at every `break` and `continue` -- protocols have no repetition, so in
//!   practice an endpoint is not used in a loop;
//! * every endpoint -- a parameter, or one created by `open` -- must have
//!   reached the end of its protocol (or have been passed on) when the function
//!   returns or the block that created it ends;
//! * no `perform`, and no call to a function that `uses` effects, while an
//!   endpoint is part-way through its protocol: a handler could abandon the
//!   function there. A handler arm may not move an endpoint.
//!
//! Endpoints are linear: the task validator (`validator::tasks`) only
//! allows an endpoint to be *used* -- the operand of `<-`, `.choose`,
//! `offer`, or an argument -- never copied, stored or returned.
//!
//! # What that guarantees
//!
//! Protocol fidelity: two tasks holding the two endpoints of a pair each do
//! exactly what their (dual) protocols say, so every send is matched by a
//! receive of the same type, every `choose` by an `offer`, and neither side
//! stops early. Deadlock freedom holds only for a single session between
//! two tasks that block on nothing else: then one side is always able to
//! proceed. Tasks that interleave several sessions or channels can still
//! deadlock each other, and one task holding both endpoints of a pair can
//! deadlock itself; neither is checked.
use std::sync::Arc;

use codira_syntax::{ast, AstPtr};
use rustc_hash::FxHashMap;

use super::{Expr, ExprCollector, ExprId, Literal, Statement};
use crate::{
    code_model::{session::ProtocolNodeId, Protocol, ProtocolStep, Session},
    expr::{BinaryOp, LogicOp, Pat, PatId},
    ids::DefWithBodyId,
    resolve::{resolver_for_expr, ValueNs},
    Body, Function, HirDatabase, Name, Ty, TyKind,
};

impl ExprCollector<'_> {
    /// `offer ep { .. } or { .. }`.
    pub(super) fn collect_offer(
        &mut self,
        offer: &ast::OfferExpr,
        syntax_ptr: AstPtr<ast::Expr>,
    ) -> ExprId {
        let endpoint = self.collect_expr_opt(offer.endpoint());
        let branches = offer.branches().map(|b| self.collect_expr(b)).collect();
        self.alloc_expr(Expr::Offer { endpoint, branches }, syntax_ptr)
    }
}

/// The session `expr` names, when it is a path to a `session` declaration.
fn session_named(db: &dyn HirDatabase, body: &Body, expr: ExprId) -> Option<Session> {
    let Expr::Path(path) = &body[expr] else {
        return None;
    };
    resolver_for_expr(db, body.owner(), expr)
        .resolve_path_as_session(db, path)
        .map(|(id, _)| Session::from(id))
}

/// `S.open()`: the session `S` when `expr` is a method call on a session's
/// name, whatever the method (inference reports any but `open()`).
pub fn opened_session(db: &dyn HirDatabase, body: &Body, expr: ExprId) -> Option<Session> {
    match &body[expr] {
        Expr::MethodCall { receiver, .. } => session_named(db, body, *receiver),
        _ => None,
    }
}

/// What the checker found in one body.
#[derive(Debug, Default, PartialEq, Eq)]
pub struct SessionFlow {
    /// The payload type of each `ep <- v` and `<-ep` the protocol allows.
    pub payloads: FxHashMap<ExprId, Ty>,
    /// Protocol violations, at the expression they concern.
    pub errors: Vec<(ExprId, String)>,
}

pub(crate) fn session_flow_query(db: &dyn HirDatabase, def: DefWithBodyId) -> Arc<SessionFlow> {
    let body = db.body(def);
    let DefWithBodyId::FunctionId(id) = def;
    let function = Function::from(id);

    let mut checker = Checker {
        db,
        body: Arc::clone(&body),
        endpoints: FxHashMap::default(),
        loops: Vec::new(),
        flow: SessionFlow::default(),
    };
    let mut state = State::default();
    if let Some(sig) = function.ty(db).callable_sig(db) {
        for ((pat, _), ty) in body.params().iter().zip(sig.params()) {
            if let TyKind::Session(session, dual) = ty.interned() {
                checker.declare(*pat, *session, *dual, &mut state);
            }
        }
    }
    let has_open = body
        .exprs()
        .any(|(id, _)| opened_session(db, &body, id).is_some());
    if state.is_empty() && !has_open {
        return Arc::new(checker.flow);
    }

    let root = body.body_expr();
    if checker.walk(root, &mut state) {
        checker.check_finished(root, &state, "the function returns");
    }
    Arc::new(checker.flow)
}

/// Where an endpoint is.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Slot {
    At(ProtocolNodeId),
    /// Passed to a function or a task, which now owns it.
    Passed,
}

type State = FxHashMap<PatId, Slot>;

struct Endpoint {
    session: Session,
    dual: bool,
    name: Name,
    protocol: Arc<Protocol>,
}

struct Checker<'a> {
    db: &'a dyn HirDatabase,
    body: Arc<Body>,
    endpoints: FxHashMap<PatId, Endpoint>,
    /// The state at the head of each enclosing loop, innermost last.
    loops: Vec<State>,
    flow: SessionFlow,
}

impl Checker<'_> {
    fn error(&mut self, expr: ExprId, message: String) {
        self.flow.errors.push((expr, message));
    }

    fn declare(&mut self, pat: PatId, session: Session, dual: bool, state: &mut State) {
        let name = match &self.body[pat] {
            Pat::Bind { name } => name.clone(),
            _ => Name::missing(),
        };
        let protocol = session.protocol(self.db);
        state.insert(pat, Slot::At(protocol.start()));
        self.endpoints.insert(
            pat,
            Endpoint {
                session,
                dual,
                name,
                protocol,
            },
        );
    }

    /// The endpoint binding `expr` names, if it is a path to one.
    fn endpoint_of(&self, expr: ExprId) -> Option<PatId> {
        let Expr::Path(path) = &self.body[expr] else {
            return None;
        };
        match resolver_for_expr(self.db, self.body.owner(), expr)
            .resolve_path_as_value_fully(self.db, path)?
        {
            (ValueNs::LocalBinding(pat), _) if self.endpoints.contains_key(&pat) => Some(pat),
            _ => None,
        }
    }

    fn describe(&self, pat: PatId, node: ProtocolNodeId) -> String {
        let endpoint = &self.endpoints[&pat];
        endpoint.protocol.describe(self.db, node, endpoint.dual)
    }

    /// The point `pat` is at, reporting a use after it was passed on.
    fn current(&mut self, at: ExprId, pat: PatId, state: &State) -> Option<ProtocolNodeId> {
        match state.get(&pat) {
            Some(Slot::At(node)) => Some(*node),
            Some(Slot::Passed) => {
                let name = self.endpoints[&pat].name.clone();
                self.error(
                    at,
                    format!(
                        "`{name}` was passed to another function or task, which owns it now: \
                         it cannot be used here"
                    ),
                );
                None
            }
            None => None,
        }
    }

    fn unexpected(&mut self, at: ExprId, pat: PatId, node: ProtocolNodeId, doing: &str) {
        let name = self.endpoints[&pat].name.clone();
        let endpoint = &self.endpoints[&pat];
        let message = if matches!(
            endpoint.protocol.step(node, endpoint.dual),
            ProtocolStep::End
        ) {
            format!(
                "the protocol of `{name}` has ended: nothing more can be done with it (this \
                 {doing})"
            )
        } else {
            format!(
                "this {doing}, but the protocol of `{name}` expects: {}",
                self.describe(pat, node)
            )
        };
        self.error(at, message);
    }

    /// `ep <- v` (`send`) or `<-ep`.
    fn advance(&mut self, op: ExprId, pat: PatId, send: bool, state: &mut State) {
        let Some(node) = self.current(op, pat, state) else {
            return;
        };
        let endpoint = &self.endpoints[&pat];
        let next = match (endpoint.protocol.step(node, endpoint.dual), send) {
            (ProtocolStep::Send(ty, next), true) | (ProtocolStep::Recv(ty, next), false) => {
                self.flow.payloads.insert(op, ty.clone());
                next
            }
            _ => {
                let doing = if send { "sends" } else { "receives" };
                self.unexpected(op, pat, node, doing);
                return;
            }
        };
        state.insert(pat, Slot::At(next));
    }

    /// `ep.choose(k)`.
    fn choose(
        &mut self,
        call: ExprId,
        pat: PatId,
        method: &Name,
        args: &[ExprId],
        state: &mut State,
    ) {
        if method.as_str() != Some("choose") {
            self.error(
                call,
                format!(
                    "an endpoint has one method, `.choose(branch)`, not `.{method}`: sends and \
                     receives are `ep <- value` and `<-ep`"
                ),
            );
            return;
        }
        let branch = match args {
            [arg] => match &self.body[*arg] {
                Expr::Literal(Literal::Int(lit)) => usize::try_from(lit.value).ok(),
                _ => None,
            },
            _ => None,
        };
        let Some(branch) = branch else {
            self.error(
                call,
                "`.choose` takes the branch as an integer literal, `ep.choose(0)`: which branch \
                 is taken must be known to check the rest of the protocol"
                    .to_string(),
            );
            return;
        };
        let Some(node) = self.current(call, pat, state) else {
            return;
        };
        let endpoint = &self.endpoints[&pat];
        let next = match endpoint.protocol.step(node, endpoint.dual) {
            ProtocolStep::Choose(branches) if branch < branches.len() => branches[branch],
            ProtocolStep::Choose(branches) => {
                let count = branches.len();
                let name = endpoint.name.clone();
                self.error(
                    call,
                    format!(
                        "`{name}` can choose branch 0 to {}, not {branch}",
                        count - 1
                    ),
                );
                return;
            }
            _ => {
                self.unexpected(call, pat, node, "chooses a branch");
                return;
            }
        };
        state.insert(pat, Slot::At(next));
    }

    /// Passing endpoint `pat` as argument `index` of a call to `callee`.
    fn pass(
        &mut self,
        arg: ExprId,
        callee: Option<Function>,
        index: usize,
        pat: PatId,
        state: &mut State,
    ) {
        let Some(node) = self.current(arg, pat, state) else {
            return;
        };
        let endpoint = &self.endpoints[&pat];
        let expected = TyKind::Session(endpoint.session, endpoint.dual).intern();
        let param = callee
            .and_then(|f| f.ty(self.db).callable_sig(self.db))
            .and_then(|sig| sig.params().get(index).cloned());
        let name = endpoint.name.clone();
        if param.as_ref() != Some(&expected) {
            self.error(
                arg,
                format!(
                    "`{name}` can only be passed to a parameter of its own session type: the \
                     function takes it over and must follow the whole protocol"
                ),
            );
        } else if node != endpoint.protocol.start() {
            self.error(
                arg,
                format!(
                    "`{name}` has already been used, so it cannot be passed on: a function \
                     taking it would start the protocol from the beginning"
                ),
            );
        }
        state.insert(pat, Slot::Passed);
    }

    /// Reports every endpoint in `state` that has not finished its protocol.
    fn check_finished(&mut self, at: ExprId, state: &State, when: &str) {
        let mut pending: Vec<(PatId, ProtocolNodeId)> = state
            .iter()
            .filter_map(|(pat, slot)| match slot {
                Slot::At(node) => Some((*pat, *node)),
                Slot::Passed => None,
            })
            .collect();
        pending.sort_by_key(|(pat, _)| u32::from(pat.into_raw()));
        for (pat, node) in pending {
            self.check_one_finished(at, pat, node, when);
        }
    }

    fn check_one_finished(&mut self, at: ExprId, pat: PatId, node: ProtocolNodeId, when: &str) {
        let endpoint = &self.endpoints[&pat];
        if matches!(
            endpoint.protocol.step(node, endpoint.dual),
            ProtocolStep::End
        ) {
            return;
        }
        let name = endpoint.name.clone();
        let next = self.describe(pat, node);
        self.error(
            at,
            format!("{when} before `{name}` has finished its protocol: it still has to {next}"),
        );
    }

    /// Endpoints part-way through their protocol, by name.
    fn unfinished(&self, state: &State) -> Vec<Name> {
        let mut names: Vec<(u32, Name)> = state
            .iter()
            .filter_map(|(pat, slot)| {
                let Slot::At(node) = slot else { return None };
                let endpoint = &self.endpoints[pat];
                let started = *node != endpoint.protocol.start();
                let ended = matches!(
                    endpoint.protocol.step(*node, endpoint.dual),
                    ProtocolStep::End
                );
                (started && !ended).then(|| (u32::from(pat.into_raw()), endpoint.name.clone()))
            })
            .collect();
        names.sort_by_key(|(idx, _)| *idx);
        names.into_iter().map(|(_, name)| name).collect()
    }

    fn check_no_abandon(&mut self, at: ExprId, state: &State, what: &str) {
        if let Some(name) = self.unfinished(state).first() {
            self.error(
                at,
                format!(
                    "{what} while `{name}` is part-way through its protocol: an effect handler \
                     could abandon this function there, leaving the other side waiting"
                ),
            );
        }
    }

    /// Joins the states at the end of alternative paths. Paths that diverge
    /// (`false`) do not count; the others must agree.
    fn merge(
        &mut self,
        at: ExprId,
        state: &mut State,
        paths: Vec<(bool, State)>,
        what: &str,
    ) -> bool {
        let live: Vec<State> = paths
            .into_iter()
            .filter_map(|(continues, s)| continues.then_some(s))
            .collect();
        let Some(first) = live.first().cloned() else {
            return false;
        };
        let mut reported = Vec::new();
        for other in &live[1..] {
            for (pat, slot) in &first {
                if other.get(pat) != Some(slot) && !reported.contains(pat) {
                    reported.push(*pat);
                }
            }
        }
        reported.sort_by_key(|pat| u32::from(pat.into_raw()));
        for pat in reported {
            let name = self.endpoints[&pat].name.clone();
            self.error(
                at,
                format!(
                    "{what} leave `{name}` at different points of its protocol: every path must \
                     do the same with an endpoint"
                ),
            );
        }
        *state = first;
        true
    }

    /// A part that may or may not run (`if` without `else`, the right of
    /// `&&`/`||`): it must not move any endpoint.
    fn optional(&mut self, at: ExprId, expr: ExprId, state: &mut State, what: &str) -> bool {
        let mut inner = state.clone();
        let continues = self.walk(expr, &mut inner);
        let before = state.clone();
        self.merge(at, state, vec![(true, before), (continues, inner)], what)
    }

    /// Whether the `loop` `body` contains a `break` for itself.
    fn breaks(&self, expr: ExprId) -> bool {
        match &self.body[expr] {
            Expr::Break { .. } => true,
            Expr::Loop { .. } | Expr::While { .. } | Expr::For { .. } => false,
            other => {
                let mut found = false;
                other.walk_child_exprs(|child| found |= self.breaks(child));
                found
            }
        }
    }

    fn check_loop_state(&mut self, at: ExprId, state: &State, what: &str) {
        let Some(head) = self.loops.last().cloned() else {
            return;
        };
        let mut moved: Vec<PatId> = head
            .iter()
            .filter(|(pat, slot)| state.get(pat) != Some(slot))
            .map(|(pat, _)| *pat)
            .collect();
        moved.sort_by_key(|pat| u32::from(pat.into_raw()));
        // Endpoints opened inside the loop body are left behind here.
        let mut created: Vec<(PatId, ProtocolNodeId)> = state
            .iter()
            .filter(|(pat, _)| !head.contains_key(pat))
            .filter_map(|(pat, slot)| match slot {
                Slot::At(node) => Some((*pat, *node)),
                Slot::Passed => None,
            })
            .collect();
        created.sort_by_key(|(pat, _)| u32::from(pat.into_raw()));
        for (pat, node) in created {
            self.check_one_finished(at, pat, node, what);
        }
        for pat in moved {
            let name = self.endpoints[&pat].name.clone();
            self.error(
                at,
                format!(
                    "{what} with `{name}` at a different point of its protocol than where the \
                     loop started: a protocol has no repetition, so a loop may not move an \
                     endpoint"
                ),
            );
        }
    }

    fn walk_loop(&mut self, at: ExprId, parts: &[ExprId], state: &mut State) {
        self.loops.push(state.clone());
        let mut inner = state.clone();
        let mut continues = true;
        for &part in parts {
            if !self.walk(part, &mut inner) {
                continues = false;
                break;
            }
        }
        if continues {
            self.check_loop_state(at, &inner, "an iteration ends");
        }
        self.loops.pop();
    }

    /// Walks `expr` in evaluation order, updating `state`. Returns whether
    /// control can continue after it.
    fn walk(&mut self, expr: ExprId, state: &mut State) -> bool {
        let body = Arc::clone(&self.body);
        match &body[expr] {
            Expr::ChannelSend { channel, value } => {
                if !self.walk(*value, state) {
                    return false;
                }
                if let Some(pat) = self.endpoint_of(*channel) {
                    self.advance(expr, pat, true, state);
                }
                true
            }
            Expr::ChannelReceive { channel } => {
                if let Some(pat) = self.endpoint_of(*channel) {
                    self.advance(expr, pat, false, state);
                }
                true
            }
            Expr::MethodCall {
                receiver,
                args,
                method_name,
            } => {
                for &arg in args {
                    if !self.walk(arg, state) {
                        return false;
                    }
                }
                if let Some(pat) = self.endpoint_of(*receiver) {
                    self.choose(expr, pat, method_name, args, state);
                    return true;
                }
                if opened_session(self.db, &body, expr).is_some() {
                    self.error(
                        expr,
                        "a session's endpoints are created by `let (a, b) = S.open()`, which \
                         names both"
                            .to_string(),
                    );
                    return true;
                }
                if !self.walk(*receiver, state) {
                    return false;
                }
                for (idx, &arg) in args.iter().enumerate() {
                    if let Some(pat) = self.endpoint_of(arg) {
                        // A method is never declared with a session parameter
                        // a task could own; `pass` reports it.
                        self.pass(arg, None, idx, pat, state);
                    }
                }
                true
            }
            Expr::Call { callee, args } => {
                for &arg in args {
                    if !self.walk(arg, state) {
                        return false;
                    }
                }
                let function = self.callee(*callee);
                if function.is_some_and(|f| !f.effects(self.db).is_empty()) {
                    self.check_no_abandon(expr, state, "a call to a function that uses effects");
                }
                for (idx, &arg) in args.iter().enumerate() {
                    if let Some(pat) = self.endpoint_of(arg) {
                        self.pass(arg, function, idx, pat, state);
                    }
                }
                true
            }
            Expr::Offer { endpoint, branches } => self.walk_offer(expr, *endpoint, branches, state),
            Expr::If {
                condition,
                then_branch,
                else_branch,
            } => {
                if !self.walk(*condition, state) {
                    return false;
                }
                match else_branch {
                    Some(else_branch) => {
                        let mut then_state = state.clone();
                        let then_continues = self.walk(*then_branch, &mut then_state);
                        let mut else_state = state.clone();
                        let else_continues = self.walk(*else_branch, &mut else_state);
                        self.merge(
                            expr,
                            state,
                            vec![(then_continues, then_state), (else_continues, else_state)],
                            "the branches of this `if`",
                        )
                    }
                    None => self.optional(expr, *then_branch, state, "an `if` without `else` may"),
                }
            }
            Expr::BinaryOp {
                lhs,
                rhs,
                op: Some(BinaryOp::LogicOp(LogicOp::And | LogicOp::Or)),
            } => {
                if !self.walk(*lhs, state) {
                    return false;
                }
                self.optional(expr, *rhs, state, "the two outcomes of this `&&`/`||`")
            }
            Expr::Select { arms } => {
                for arm in arms {
                    for operand in arm.operands() {
                        if !self.walk(operand, state) {
                            return false;
                        }
                    }
                }
                let paths = arms
                    .iter()
                    .map(|arm| {
                        let mut arm_state = state.clone();
                        let continues = self.walk(arm.body, &mut arm_state);
                        (continues, arm_state)
                    })
                    .collect();
                self.merge(expr, state, paths, "the arms of this `select`")
            }
            Expr::Loop { body: loop_body } => {
                self.walk_loop(expr, &[*loop_body], state);
                self.breaks(*loop_body)
            }
            Expr::While {
                condition,
                body: loop_body,
            } => {
                self.walk_loop(expr, &[*condition, *loop_body], state);
                true
            }
            Expr::For {
                iterable,
                body: loop_body,
                ..
            } => {
                if !self.walk(*iterable, state) {
                    return false;
                }
                self.walk_loop(expr, &[*loop_body], state);
                true
            }
            Expr::Break { expr: value } => {
                if let Some(value) = value {
                    if !self.walk(*value, state) {
                        return false;
                    }
                }
                self.check_loop_state(expr, state, "this `break` leaves the loop");
                false
            }
            Expr::Continue => {
                self.check_loop_state(expr, state, "this `continue` starts the next iteration");
                false
            }
            Expr::Return { expr: value } => {
                if let Some(value) = value {
                    if !self.walk(*value, state) {
                        return false;
                    }
                }
                self.check_finished(expr, state, "this `return` leaves the function");
                false
            }
            Expr::Perform { args, .. } => {
                for &arg in args {
                    if !self.walk(arg, state) {
                        return false;
                    }
                }
                self.check_no_abandon(expr, state, "`perform`");
                true
            }
            Expr::Handle {
                body: handled,
                arms,
            } => {
                if !self.walk(*handled, state) {
                    return false;
                }
                // An arm runs any number of times, at any `perform`: it
                // may not move an endpoint.
                for arm in arms {
                    self.optional(expr, arm.body, state, "a handler arm may");
                }
                true
            }
            Expr::Block { statements, tail } => self.walk_block(statements, *tail, state),
            other => {
                let mut children = Vec::new();
                other.walk_child_exprs(|child| children.push(child));
                for child in children {
                    if !self.walk(child, state) {
                        return false;
                    }
                }
                true
            }
        }
    }

    fn callee(&self, callee: ExprId) -> Option<Function> {
        let Expr::Path(path) = &self.body[callee] else {
            return None;
        };
        match resolver_for_expr(self.db, self.body.owner(), callee)
            .resolve_path_as_value_fully(self.db, path)?
        {
            (ValueNs::FunctionId(id), _) => Some(Function::from(id)),
            _ => None,
        }
    }

    fn walk_offer(
        &mut self,
        expr: ExprId,
        endpoint: ExprId,
        branches: &[ExprId],
        state: &mut State,
    ) -> bool {
        let Some(pat) = self.endpoint_of(endpoint) else {
            // Not an endpoint: inference reports it. Check the branches
            // as alternatives anyway.
            let paths = branches
                .iter()
                .map(|&branch| {
                    let mut branch_state = state.clone();
                    (self.walk(branch, &mut branch_state), branch_state)
                })
                .collect();
            return self.merge(expr, state, paths, "the branches of this `offer`");
        };
        let starts: Option<Vec<ProtocolNodeId>> = self.current(expr, pat, state).and_then(|node| {
            let endpoint = &self.endpoints[&pat];
            match endpoint.protocol.step(node, endpoint.dual) {
                ProtocolStep::Offer(offered) if offered.len() == branches.len() => {
                    Some(offered.to_vec())
                }
                ProtocolStep::Offer(offered) => {
                    let (name, count) = (endpoint.name.clone(), offered.len());
                    self.error(
                        expr,
                        format!(
                            "the protocol of `{name}` offers {count} branches here, and this \
                             `offer` has {}",
                            branches.len()
                        ),
                    );
                    None
                }
                _ => {
                    self.unexpected(expr, pat, node, "`offer` waits for the other side's choice");
                    None
                }
            }
        });
        let paths = branches
            .iter()
            .enumerate()
            .map(|(idx, &branch)| {
                let mut branch_state = state.clone();
                if let Some(starts) = &starts {
                    branch_state.insert(pat, Slot::At(starts[idx]));
                }
                (self.walk(branch, &mut branch_state), branch_state)
            })
            .collect();
        self.merge(expr, state, paths, "the branches of this `offer`")
    }

    fn walk_block(
        &mut self,
        statements: &[Statement],
        tail: Option<ExprId>,
        state: &mut State,
    ) -> bool {
        let body = Arc::clone(&self.body);
        // The endpoints this block creates, with the `open` that made them.
        let mut created: Vec<(PatId, ExprId)> = Vec::new();
        let mut continues = true;
        for statement in statements {
            match statement {
                Statement::Let {
                    pat,
                    initializer: Some(init),
                    ..
                } if opened_session(self.db, &body, *init).is_some() => {
                    let session = opened_session(self.db, &body, *init).expect("checked above");
                    match &body[*pat] {
                        Pat::Tuple(pats)
                            if pats.len() == 2
                                && pats.iter().all(|p| matches!(body[*p], Pat::Bind { .. })) =>
                        {
                            self.declare(pats[0], session, false, state);
                            self.declare(pats[1], session, true, state);
                            created.push((pats[0], *init));
                            created.push((pats[1], *init));
                        }
                        _ => self.error(
                            *init,
                            "`S.open()` creates two endpoints: bind both, `let (a, b) = \
                             S.open()`"
                                .to_string(),
                        ),
                    }
                }
                Statement::Let {
                    initializer: Some(init),
                    ..
                }
                | Statement::Expr(init) => {
                    if !self.walk(*init, state) {
                        continues = false;
                        break;
                    }
                }
                Statement::Let { .. } => {}
            }
        }
        if continues {
            if let Some(tail) = tail {
                continues = self.walk(tail, state);
            }
        }
        for (pat, open) in created {
            if continues {
                if let Some(Slot::At(node)) = state.get(&pat).copied() {
                    self.check_one_finished(open, pat, node, "the block that opened it ends");
                }
            }
            state.remove(&pat);
        }
        continues
    }
}

#[cfg(test)]
mod tests {
    use crate::utils::tests::diagnostics;

    const PROTOCOLS: &str = r"
    session Checkout {
        send i64; recv i64;
        choose { send bool; recv i64 } or { send i32 }
    }
    session Ping { send i64; recv i64 }
    ";

    fn check(body: &str) -> String {
        diagnostics(&format!("{PROTOCOLS}\n{body}"))
    }

    fn assert_clean(body: &str) {
        let diags = check(body);
        assert!(diags.is_empty(), "expected no diagnostics, got:\n{diags}");
    }

    fn assert_reports(body: &str, needle: &str) {
        let diags = check(body);
        assert!(
            diags.contains(needle),
            "expected a diagnostic containing `{needle}`, got:\n{diags}"
        );
    }

    #[test]
    fn a_dual_pair_following_the_protocol_is_clean() {
        assert_clean(
            r"
    func shop(ep: dual Checkout) {
        let item = <-ep;
        ep <- item * 2;
        offer ep {
            let pay = <-ep;
            if pay { ep <- 1 } else { ep <- 0 }
        } or {
            let reason = <-ep;
        }
    }
    func buy(budget: i64) -> i64 {
        scope {
            let (me, them) = Checkout.open();
            spawn shop(them);
            me <- 5;
            let price = <-me;
            if price < budget {
                me.choose(0);
                me <- true;
                <-me
            } else {
                me.choose(1);
                me <- 3;
                0
            }
        }
    }
    ",
        );
    }

    #[test]
    fn an_operation_the_protocol_does_not_expect() {
        assert_reports(
            "func f(ep: Ping) { let x = <-ep; }",
            "this receives, but the protocol of `ep` expects: send i64",
        );
    }

    #[test]
    fn a_payload_of_the_wrong_type() {
        let diags = check("func f(ep: Ping) { ep <- true; let x = <-ep; }");
        assert!(!diags.is_empty(), "a `bool` sent where `i64` is expected");
    }

    #[test]
    fn use_after_the_protocol_ended() {
        assert_reports(
            "func f(ep: Ping) { ep <- 1; let x = <-ep; ep <- 2; }",
            "the protocol of `ep` has ended",
        );
    }

    #[test]
    fn a_parameter_must_finish_its_protocol() {
        assert_reports(
            "func f(ep: Ping) { ep <- 1; }",
            "the function returns before `ep` has finished its protocol: it still has to recv i64",
        );
        assert_reports(
            "func f(ep: Ping) {}",
            "before `ep` has finished its protocol",
        );
    }

    #[test]
    fn an_opened_pair_must_finish_in_its_block() {
        assert_reports(
            r"
    func peer(ep: dual Ping) { let x = <-ep; ep <- x }
    func f() { scope { let (a, b) = Ping.open(); spawn peer(b); a <- 1; } }
    ",
            "the block that opened it ends before `a` has finished its protocol",
        );
    }

    #[test]
    fn return_before_the_protocol_ends() {
        assert_reports(
            "func f(ep: Ping, early: bool) { ep <- 1; if early { return; }; let x = <-ep; }",
            "this `return` leaves the function before `ep` has finished its protocol",
        );
    }

    #[test]
    fn branches_must_agree() {
        assert_reports(
            "func f(ep: Ping, c: bool) { if c { ep <- 1 } else { }; ep <- 2; }",
            "leave `ep` at different points of its protocol",
        );
        assert_reports(
            "func f(ep: Ping, c: bool) { if c { ep <- 1 }; let x = <-ep; }",
            "an `if` without `else` may leave `ep` at different points",
        );
    }

    #[test]
    fn short_circuits_may_not_move_an_endpoint() {
        assert_reports(
            "func f(ep: Ping, c: bool) { if c && { ep <- 1; true } { }; let x = <-ep; }",
            "the two outcomes of this `&&`/`||` leave `ep` at different points",
        );
    }

    #[test]
    fn select_waits_on_channels_only() {
        assert_reports(
            "func f(ep: Ping) { ep <- 1; select { x = <-ep -> {} }; }",
            "a `select` waits on channels, not on session endpoints",
        );
    }

    #[test]
    fn loops_may_not_move_an_endpoint() {
        assert_reports(
            "func f(ep: Ping) { let i = 0; while i < 1 { ep <- 1; i += 1 }; let x = <-ep; }",
            "a loop may not move an endpoint",
        );
    }

    #[test]
    fn endpoints_are_linear() {
        assert_reports(
            "func f(ep: Ping) { let copy = ep; copy <- 1; let x = <-copy; }",
            "a session endpoint can only be used with `<-`",
        );
        assert_reports(
            "func f(ep: Ping) -> Ping { ep }",
            "a session endpoint can only be used with `<-`",
        );
    }

    #[test]
    fn passing_an_endpoint_hands_it_over() {
        assert_reports(
            r"
    func g(ep: Ping) { ep <- 1; let x = <-ep; }
    func f(ep: Ping) { g(ep); ep <- 1; }
    ",
            "`ep` was passed to another function or task",
        );
        assert_reports(
            r"
    func g(ep: Ping) { ep <- 1; let x = <-ep; }
    func f(ep: Ping) { ep <- 1; g(ep); }
    ",
            "`ep` has already been used, so it cannot be passed on",
        );
        assert_reports(
            r"
    func g(ep: dual Ping) { let x = <-ep; ep <- x; }
    func f(ep: Ping) { g(ep); }
    ",
            "`ep` can only be passed to a parameter of its own session type",
        );
    }

    #[test]
    fn choose_takes_an_existing_literal_branch() {
        assert_reports(
            "func f(ep: Checkout) { ep <- 1; let p = <-ep; ep.choose(2); }",
            "`ep` can choose branch 0 to 1, not 2",
        );
        assert_reports(
            "func f(ep: Checkout, k: i64) { ep <- 1; let p = <-ep; ep.choose(k); }",
            "`.choose` takes the branch as an integer literal",
        );
    }

    #[test]
    fn offer_needs_one_block_per_branch() {
        assert_reports(
            "func f(ep: dual Checkout) { let i = <-ep; ep <- i; offer ep { let b = <-ep; ep <- 1 } }",
            "offers 2 branches here, and this `offer` has 1",
        );
    }

    #[test]
    fn no_perform_part_way_through_a_protocol() {
        assert_reports(
            r"
    effect Ask { func ask() -> i64 }
    func f(ep: Ping) uses Ask { ep <- 1; let y = perform Ask.ask(); let x = <-ep; }
    ",
            "`perform` while `ep` is part-way through its protocol",
        );
    }

    #[test]
    fn sessions_open_inside_a_scope() {
        assert_reports(
            r"
    func peer(ep: dual Ping) { let x = <-ep; ep <- x }
    func f() { let (a, b) = Ping.open(); spawn peer(b); a <- 1; let y = <-a; }
    ",
            "a session must be opened inside a `scope { .. }` block",
        );
    }

    #[test]
    fn sessions_carry_scalars() {
        let diags = diagnostics("struct P { x: i32 }\nsession S { send P }");
        assert!(
            diags.contains("a session carries integers, floats or `bool`, not `P`"),
            "{diags}"
        );
    }

    #[test]
    fn dual_applies_to_sessions_only() {
        let diags = diagnostics("func f(x: dual i32) {}");
        assert!(
            diags.contains("`dual` applies to a session type"),
            "{diags}"
        );
    }
}
