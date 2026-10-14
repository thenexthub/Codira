//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Proof-gated synthesis: fills `??` holes and `choose x where pred`
//!   expressions with expressions Z3 proves correct, by counterexample- guided
//!   inductive synthesis (CEGIS).
//!
//! # The search
//!
//! Each hole gets a *template*, an expression shape whose constants are
//! unknowns, tried in a fixed order from simplest:
//!
//! 1. a constant `c`;
//! 2. a linear combination `c0 + c1*p1 + ... + cn*pn` of the parameters of the
//!    hole's type;
//! 3. `if g { L1 } else { L2 }` with two linear combinations, for every guard
//!    `g` of the form `pi < pj` or `pi < c` (a boolean hole instead tries
//!    `true`/`false`, then `pi op pj` and `pi op c` comparisons).
//!
//! For one template, CEGIS alternates two Z3 queries:
//!
//! * **synthesise**: find constants making every proof obligation of the
//!   function hold on each counterexample found so far (the function is
//!   executed symbolically with its parameters fixed to those values);
//! * **verify**: with those constants, check every obligation for *all* inputs,
//!   exactly as `codira check` would. Success ends the search; a failure's
//!   model is a new counterexample, and the loop repeats.
//!
//! The obligations are the function's own -- `ensures`, refined returns,
//! callee `requires` along the way, loop invariants, division safety -- plus
//! a `choose`'s predicate. So a filled hole is not "a value that worked on
//! some tests": it is proven against the contract, and `codira check`
//! re-proves it after the source is rewritten.
//!
//! # Determinism and bounds
//!
//! Templates are tried in a fixed order, each for at most [`MAX_ROUNDS`]
//! CEGIS rounds, and every Z3 query runs under the deterministic `rlimit`
//! budget. Nothing depends on wall-clock time; the same source always
//! yields the same fill or the same failure.
//!
//! # Limits
//!
//! Holes of integer and `bool` type only; templates mention only the
//! function's parameters (not locals in scope at the hole), and only those of
//! the hole's own type. A function with several holes fills them jointly
//! from the same template level.

use codira_smt::{BoolExpr, BvExpr, CheckOutcome, Context};
use codira_syntax::{AstNode, TextRange};
use rustc_hash::FxHashMap;

use super::{
    exec::{self, int_width, Frame, Sym},
    Value,
};
use crate::{
    code_model::Function, diagnostics::DiagnosticSink, ExprId, HirDatabase, IntTy, TyKind,
};

/// CEGIS rounds per template before it is abandoned.
pub const MAX_ROUNDS: usize = 32;

/// One filled hole.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Fill {
    /// The text the hole occupies in its file.
    pub range: TextRange,
    /// The proven expression, as Codira source.
    pub text: String,
}

/// Pushes an error for every unfilled hole of `func`; returns whether there
/// was one.
pub(crate) fn report_holes(
    db: &dyn HirDatabase,
    func: Function,
    sink: &mut DiagnosticSink<'_>,
) -> bool {
    let frame = Frame::new(db, func);
    let contracts = frame.body.contracts();
    let mut sites: Vec<(ExprId, &str)> = contracts.holes.iter().map(|&h| (h, "`??`")).collect();
    sites.extend(contracts.chooses.keys().map(|&c| (c, "`choose`")));
    for (expr, what) in &sites {
        if let Some(site) = frame.site(*expr) {
            sink.push(crate::diagnostics::UnfilledHole {
                file: site.file_id,
                site: site.value,
                what: (*what).to_string(),
            });
        }
    }
    !sites.is_empty()
}

/// The holes of `func` (`??` and `choose`), in source order.
pub fn holes(db: &dyn HirDatabase, func: Function) -> Vec<(ExprId, TextRange)> {
    let frame = Frame::new(db, func);
    let contracts = frame.body.contracts();
    let mut out: Vec<(ExprId, TextRange)> = contracts
        .holes
        .iter()
        .chain(contracts.chooses.keys())
        .filter_map(|&expr| Some((expr, frame.site(expr)?.value.range())))
        .collect();
    out.sort_by_key(|(_, range)| range.start());
    out
}

/// What a hole can be filled with.
#[derive(Clone, Copy, Debug)]
enum HoleTy {
    Int(IntTy),
    Bool,
}

/// One template for one hole, over the parameters of matching type.
#[derive(Clone, Debug)]
enum Template {
    Const,
    Linear,
    /// `if p[i] < p[j]` (or `< c` when `j` is `None`) `{ linear } else
    /// { linear }`.
    Guarded {
        i: usize,
        j: Option<usize>,
    },
    /// `p[i] op p[j]` / `p[i] op c` for a boolean hole.
    Compare {
        i: usize,
        j: Option<usize>,
        op: CmpKind,
    },
}

#[derive(Clone, Copy, Debug)]
enum CmpKind {
    Lt,
    Le,
    Eq,
}

impl CmpKind {
    const ALL: [CmpKind; 3] = [CmpKind::Lt, CmpKind::Le, CmpKind::Eq];

    fn text(self) -> &'static str {
        match self {
            CmpKind::Lt => "<",
            CmpKind::Le => "<=",
            CmpKind::Eq => "==",
        }
    }
}

/// The unknown constants of one instantiated template.
#[derive(Clone, Copy)]
enum Coeff<'ctx> {
    /// An integer unknown; `true` when it multiplies a parameter.
    Int(BvExpr<'ctx>, IntTy, bool),
    Bool(BoolExpr<'ctx>),
}

/// The templates tried for a hole, simplest first.
fn templates(ty: HoleTy, params: usize) -> Vec<Template> {
    match ty {
        HoleTy::Int(_) => {
            let mut out = vec![Template::Const];
            if params > 0 {
                out.push(Template::Linear);
            }
            for i in 0..params {
                for j in (i + 1)..params {
                    out.push(Template::Guarded { i, j: Some(j) });
                }
                out.push(Template::Guarded { i, j: None });
            }
            out
        }
        HoleTy::Bool => {
            let mut out = vec![Template::Const];
            for i in 0..params {
                for j in 0..params {
                    if i != j {
                        for op in CmpKind::ALL {
                            out.push(Template::Compare { i, j: Some(j), op });
                        }
                    }
                }
                for op in CmpKind::ALL {
                    out.push(Template::Compare { i, j: None, op });
                }
            }
            out
        }
    }
}

/// A hole being synthesised: its type, which parameters its templates may
/// mention, and which template is current.
struct HolePlan {
    expr: ExprId,
    range: TextRange,
    ty: HoleTy,
    /// Indices into the function's parameter list.
    params: Vec<usize>,
}

/// Instantiates `template` for `hole` over `inputs`, with fresh unknown
/// constants named after `tag`. Returns the hole's value and its unknowns.
fn instantiate<'ctx>(
    ctx: &'ctx Context,
    hole: &HolePlan,
    template: &Template,
    inputs: &[(String, Sym<'ctx>)],
    coeffs: &mut Vec<Coeff<'ctx>>,
    fixed_coeffs: Option<&[Value]>,
    tag: &str,
) -> Sym<'ctx> {
    let mut next = 0usize;
    let mut int_coeff =
        |ty: IntTy, multiplier: bool, coeffs: &mut Vec<Coeff<'ctx>>| -> BvExpr<'ctx> {
            let width = int_width(ty);
            let value = match fixed_coeffs.and_then(|f| f.get(next)) {
                Some(Value::Int { bits, .. }) => ctx.bv_lit(*bits, width),
                _ => ctx.bv_var(&format!("{tag}!c{next}"), width),
            };
            next += 1;
            coeffs.push(Coeff::Int(value, ty, multiplier));
            value
        };
    let param = |idx: usize| -> Option<BvExpr<'ctx>> {
        match inputs.get(hole.params[idx])?.1 {
            Sym::Int(bv, _) => Some(bv),
            _ => None,
        }
    };
    match (hole.ty, template) {
        (HoleTy::Int(ty), Template::Const) => Sym::Int(int_coeff(ty, false, coeffs), ty),
        (HoleTy::Int(ty), Template::Linear) => {
            let mut sum = int_coeff(ty, false, coeffs);
            for idx in 0..hole.params.len() {
                let c = int_coeff(ty, true, coeffs);
                if let Some(p) = param(idx) {
                    sum = sum.add(c.mul(p));
                }
            }
            Sym::Int(sum, ty)
        }
        (HoleTy::Int(ty), Template::Guarded { i, j }) => {
            let pi = param(*i);
            let rhs = match j {
                Some(j) => param(*j),
                None => Some(int_coeff(ty, false, coeffs)),
            };
            let mut linear = |coeffs: &mut Vec<Coeff<'ctx>>| {
                let mut sum = int_coeff(ty, false, coeffs);
                for idx in 0..hole.params.len() {
                    let c = int_coeff(ty, true, coeffs);
                    if let Some(p) = param(idx) {
                        sum = sum.add(c.mul(p));
                    }
                }
                sum
            };
            let then_value = linear(coeffs);
            let else_value = linear(coeffs);
            let (Some(pi), Some(rhs)) = (pi, rhs) else {
                return Sym::Opaque;
            };
            let guard = if ty.signedness.is_signed() {
                pi.slt(rhs)
            } else {
                pi.ult(rhs)
            };
            Sym::Int(guard.ite_bv(then_value, else_value), ty)
        }
        (HoleTy::Bool, Template::Const) => {
            let value = match fixed_coeffs.and_then(|f| f.get(next)) {
                Some(Value::Bool(b)) => {
                    if *b {
                        ctx.bool_true()
                    } else {
                        ctx.bool_false()
                    }
                }
                _ => ctx.bool_var(&format!("{tag}!b{next}")),
            };
            coeffs.push(Coeff::Bool(value));
            Sym::Bool(value)
        }
        (HoleTy::Bool, Template::Compare { i, j, op }) => {
            let (Some(pi), ty) = (param(*i), param_ty(inputs, hole, *i)) else {
                return Sym::Opaque;
            };
            let Some(ty) = ty else {
                return Sym::Opaque;
            };
            let rhs = match j {
                Some(j) => param(*j),
                None => Some(int_coeff(ty, false, coeffs)),
            };
            let Some(rhs) = rhs.filter(|r| r.width() == pi.width()) else {
                return Sym::Opaque;
            };
            let signed = ty.signedness.is_signed();
            Sym::Bool(match op {
                CmpKind::Lt if signed => pi.slt(rhs),
                CmpKind::Lt => pi.ult(rhs),
                CmpKind::Le if signed => pi.sle(rhs),
                CmpKind::Le => pi.ule(rhs),
                CmpKind::Eq => pi.eq(rhs),
            })
        }
        _ => Sym::Opaque,
    }
}

fn param_ty(inputs: &[(String, Sym<'_>)], hole: &HolePlan, idx: usize) -> Option<IntTy> {
    match inputs.get(hole.params[idx])?.1 {
        Sym::Int(_, ty) => Some(ty),
        _ => None,
    }
}

/// Renders a filled template as Codira source.
fn render(hole: &HolePlan, template: &Template, names: &[String], coeffs: &[Value]) -> String {
    let name = |idx: usize| names[hole.params[idx]].clone();
    let mut next = 0usize;
    let mut take = || {
        let v = coeffs.get(next).copied();
        next += 1;
        v
    };
    let linear = |take: &mut dyn FnMut() -> Option<Value>| {
        let constant = take();
        let mut terms: Vec<String> = Vec::new();
        for idx in 0..hole.params.len() {
            let c = take();
            match c.and_then(|c| c.as_i128()) {
                Some(0) | None => {}
                Some(1) => terms.push(name(idx)),
                Some(-1) => terms.push(format!("-{}", name(idx))),
                Some(_) => terms.push(format!(
                    "{} * {}",
                    c.map(|c| c.to_source()).unwrap_or_default(),
                    name(idx)
                )),
            }
        }
        match constant {
            Some(c) if c.as_i128() != Some(0) || terms.is_empty() => {
                terms.push(c.to_source());
            }
            _ => {}
        }
        terms.join(" + ").replace("+ -", "- ")
    };
    match (hole.ty, template) {
        (HoleTy::Int(_) | HoleTy::Bool, Template::Const) => {
            take().map(|v| v.to_source()).unwrap_or_default()
        }
        (HoleTy::Int(_), Template::Linear) => linear(&mut take),
        (HoleTy::Int(_), Template::Guarded { i, j }) => {
            let rhs = match j {
                Some(j) => name(*j),
                None => take().map(|v| v.to_source()).unwrap_or_default(),
            };
            let then_text = linear(&mut take);
            let else_text = linear(&mut take);
            format!(
                "if {} < {rhs} {{ {then_text} }} else {{ {else_text} }}",
                name(*i)
            )
        }
        (HoleTy::Bool, Template::Compare { i, j, op }) => {
            let rhs = match j {
                Some(j) => name(*j),
                None => take().map(|v| v.to_source()).unwrap_or_default(),
            };
            format!("{} {} {rhs}", name(*i), op.text())
        }
        _ => String::new(),
    }
}

fn read_coeff(model: &codira_smt::Model<'_>, coeff: Coeff<'_>) -> Option<Value> {
    match coeff {
        Coeff::Int(bv, ty, _) => Some(Value::Int {
            bits: model.eval_bv_bits(bv)?,
            signed: ty.signedness.is_signed(),
            width: bv.width(),
        }),
        Coeff::Bool(b) => model.eval_bool(b).map(Value::Bool),
    }
}

/// Fills every hole of `func` with a proven expression, or says why it
/// could not.
pub fn synthesize(db: &dyn HirDatabase, func: Function) -> Result<Vec<Fill>, String> {
    let frame = Frame::new(db, func);
    let hole_sites = holes(db, func);
    if hole_sites.is_empty() {
        return Ok(Vec::new());
    }

    // Parameter types, by position.
    let param_tys: Vec<Option<HoleTy>> = frame
        .body
        .params()
        .iter()
        .map(|(pat, _)| hole_ty(db, &frame.infer[*pat]))
        .collect();

    let mut plans = Vec::new();
    for (expr, range) in &hole_sites {
        let ty = hole_ty(db, &frame.infer[*expr]).ok_or_else(|| {
            format!(
                "the hole at {range:?} is not an integer or `bool` (or its type is not known; \
                 annotate the binding it flows into)"
            )
        })?;
        // Templates over integers use the parameters of the hole's own
        // type; comparisons use any integer parameters.
        let params = param_tys
            .iter()
            .enumerate()
            .filter(|(_, p)| match (ty, p) {
                (HoleTy::Int(h), Some(HoleTy::Int(q))) => {
                    int_width(h) == int_width(*q) && h.signedness == q.signedness
                }
                (HoleTy::Bool, Some(HoleTy::Int(_))) => true,
                _ => false,
            })
            .map(|(idx, _)| idx)
            .collect();
        plans.push(HolePlan {
            expr: *expr,
            range: *range,
            ty,
            params,
        });
    }

    let names: Vec<String> = frame
        .body
        .params()
        .iter()
        .enumerate()
        .map(|(idx, (pat, _))| match &frame.body[*pat] {
            crate::Pat::Bind { name } => name.to_string(),
            _ => format!("arg{idx}"),
        })
        .collect();

    // Every hole tries its templates in lock step: level `k` gives each
    // hole its `k`-th template (or its last one, if it has fewer).
    let per_hole: Vec<Vec<Template>> = plans
        .iter()
        .map(|p| templates(p.ty, p.params.len()))
        .collect();
    let levels = per_hole.iter().map(Vec::len).max().unwrap_or(0);
    let ctx = Context::new();
    let mut last_failure = String::from("no template fits");
    for level in 0..levels {
        let chosen: Vec<Template> = per_hole
            .iter()
            .map(|t| t[level.min(t.len() - 1)].clone())
            .collect();
        // Small constants first: a counterexample pins down only a few
        // points, and over 64-bit coefficients the solver would otherwise
        // keep proposing exotic fits for them. The unbounded search runs
        // only if the bounded one fails.
        let result = cegis(db, func, &ctx, &plans, &chosen, level, true)
            .or_else(|_| cegis(db, func, &ctx, &plans, &chosen, level, false));
        match result {
            Ok(coeffs) => {
                return Ok(plans
                    .iter()
                    .zip(&chosen)
                    .zip(coeffs)
                    .map(|((plan, template), coeffs)| {
                        let text = render(plan, template, &names, &coeffs);
                        let atomic = text.chars().all(|c| c.is_alphanumeric() || c == '_')
                            || text.starts_with('(');
                        Fill {
                            range: plan.range,
                            text: if atomic { text } else { format!("({text})") },
                        }
                    })
                    .collect());
            }
            Err(reason) => last_failure = reason,
        }
    }
    Err(format!(
        "no candidate from the template grammar meets the contract ({last_failure})"
    ))
}

fn hole_ty(db: &dyn HirDatabase, ty: &crate::Ty) -> Option<HoleTy> {
    match ty.interned() {
        TyKind::Int(int) => Some(HoleTy::Int(crate::ResolveBitness::resolve(
            int,
            &db.target_data_layout(),
        ))),
        TyKind::Bool => Some(HoleTy::Bool),
        TyKind::TypeAlias(alias) => hole_ty(db, &alias.target_type(db)),
        _ => None,
    }
}

/// One CEGIS run for one template per hole. Returns the constants of each
/// hole's template on success.
fn cegis<'ctx>(
    db: &dyn HirDatabase,
    func: Function,
    ctx: &'ctx Context,
    plans: &[HolePlan],
    templates: &[Template],
    level: usize,
    bounded: bool,
) -> Result<Vec<Vec<Value>>, String> {
    let mut counterexamples: Vec<Vec<Option<Value>>> = Vec::new();
    for round in 0..MAX_ROUNDS {
        // Synthesise: constants that work on every counterexample so far.
        let solver = ctx.solver();
        let mut unknowns: Vec<Vec<Coeff<'ctx>>> = Vec::new();
        // The unknowns are shared across counterexamples: instantiate once
        // per counterexample with the same names, which Z3 identifies.
        let tag = format!("t{level}r{round}{}", if bounded { "b" } else { "u" });
        for (k, cex) in counterexamples.iter().enumerate() {
            let mut coeffs_per_hole: Vec<Vec<Coeff<'ctx>>> = Vec::new();
            let (_, obligations) =
                exec::synthesis_obligations(db, func, ctx, Some(cex.clone()), |inputs| {
                    let mut values = FxHashMap::default();
                    for (h, (plan, template)) in plans.iter().zip(templates).enumerate() {
                        let mut coeffs = Vec::new();
                        let value = instantiate(
                            ctx,
                            plan,
                            template,
                            inputs,
                            &mut coeffs,
                            None,
                            &format!("{tag}h{h}"),
                        );
                        values.insert(plan.expr, value);
                        coeffs_per_hole.push(coeffs);
                    }
                    values
                })?;
            for (premise, goal) in obligations {
                solver.assert(premise.implies(goal));
            }
            if k == 0 {
                unknowns = coeffs_per_hole;
            }
        }
        if bounded {
            for coeff in unknowns.iter().flatten() {
                if let Coeff::Int(bv, ty, multiplier) = *coeff {
                    let limit: i128 = if multiplier { 4 } else { 1 << 12 };
                    let width = bv.width();
                    let lit = |v: i128| ctx.bv_lit(v as u128, width);
                    let in_range = if ty.signedness.is_signed() {
                        bv.sge(lit(-limit)).and(bv.sle(lit(limit)))
                    } else {
                        bv.ule(lit(limit))
                    };
                    solver.assert(in_range);
                }
            }
        }
        let constants: Vec<Vec<Value>> = if counterexamples.is_empty() {
            Vec::new()
        } else {
            match solver.check_detailed() {
                CheckOutcome::Sat => {
                    let model = solver.model().ok_or("no model")?;
                    unknowns
                        .iter()
                        .map(|coeffs| {
                            coeffs
                                .iter()
                                .map(|c| read_coeff(&model, *c))
                                .collect::<Option<Vec<_>>>()
                        })
                        .collect::<Option<Vec<_>>>()
                        .ok_or("unreadable model")?
                }
                CheckOutcome::Unsat => {
                    return Err("the template cannot fit the counterexamples".to_string())
                }
                CheckOutcome::Unknown(reason) => return Err(format!("unknown ({reason})")),
            }
        };

        // Verify: those constants, all inputs.
        let mut filled_coeffs: Vec<Vec<Value>> = Vec::new();
        let (inputs, obligations) = exec::synthesis_obligations(db, func, ctx, None, |inputs| {
            let mut values = FxHashMap::default();
            for (h, (plan, template)) in plans.iter().zip(templates).enumerate() {
                let mut coeffs = Vec::new();
                let fixed = constants.get(h).map(Vec::as_slice);
                let value = instantiate(
                    ctx,
                    plan,
                    template,
                    inputs,
                    &mut coeffs,
                    Some(fixed.unwrap_or(&[])),
                    &format!("{tag}v{h}"),
                );
                values.insert(plan.expr, value);
                filled_coeffs.push(coeffs.iter().map(|_| Value::Bool(false)).collect());
            }
            values
        })?;
        // With no counterexample yet, the "constants" are the defaults the
        // instantiation picked (fresh unknowns); verification below then
        // simply produces the first counterexample.
        let verifier = ctx.solver();
        let mut failed = None;
        for (premise, goal) in &obligations {
            verifier.push();
            verifier.assert(*premise);
            verifier.assert(goal.not());
            match verifier.check_detailed() {
                CheckOutcome::Unsat => {}
                CheckOutcome::Sat => {
                    let model = verifier.model().ok_or("no model")?;
                    let cex: Vec<Option<Value>> = inputs
                        .iter()
                        .map(|(_, sym)| match *sym {
                            Sym::Int(bv, ty) => Some(Value::Int {
                                bits: model.eval_bv_bits(bv)?,
                                signed: ty.signedness.is_signed(),
                                width: bv.width(),
                            }),
                            Sym::Bool(b) => model.eval_bool(b).map(Value::Bool),
                            _ => None,
                        })
                        .collect();
                    failed = Some(cex);
                }
                CheckOutcome::Unknown(reason) => {
                    return Err(format!("unknown ({reason})"));
                }
            }
            verifier.pop(1);
            if failed.is_some() {
                break;
            }
        }
        match failed {
            None if !counterexamples.is_empty() || plans_have_no_unknowns(&filled_coeffs) => {
                return Ok(constants);
            }
            None => {
                // Proven with the unknowns still free means any constants
                // work: take zero/false.
                return Ok(filled_coeffs
                    .iter()
                    .zip(plans)
                    .map(|(coeffs, plan)| {
                        coeffs
                            .iter()
                            .map(|_| match plan.ty {
                                HoleTy::Int(ty) => Value::Int {
                                    bits: 0,
                                    signed: ty.signedness.is_signed(),
                                    width: int_width(ty),
                                },
                                HoleTy::Bool => Value::Bool(false),
                            })
                            .collect()
                    })
                    .collect());
            }
            Some(cex) => {
                if counterexamples.contains(&cex) {
                    return Err("the synthesiser repeated a counterexample".to_string());
                }
                counterexamples.push(cex);
            }
        }
    }
    Err(format!("no fit within {MAX_ROUNDS} rounds"))
}

fn plans_have_no_unknowns(coeffs: &[Vec<Value>]) -> bool {
    coeffs.iter().all(Vec::is_empty)
}

/// Source text of `func`, for an oracle's prompt.
pub fn function_text(db: &dyn HirDatabase, func: Function) -> String {
    use crate::HasSource;
    func.source(db).value.syntax().text().to_string()
}
