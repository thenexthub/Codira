//! The elaborator: generator specialization (monomorphization).
//!
//! Structural analog of `KGEN/lib/Elaborator` -- see
//! `spec/KGEN_SUPERSET_ARCHITECTURE.md` §4 point 2. Given a
//! [`codira_mir::Generator`] and concrete values for its compile-time
//! parameters, produces a new [`Body`] with every `param.ref` replaced by
//! the concrete value, every `core.arg` (a genuine runtime value) left
//! untouched, and -- unlike a naive substitution pass -- every
//! sub-expression that becomes fully closed over constants after
//! substitution folded down to a single `Const` op. This mirrors KGEN's
//! own pre/post-elaboration `SCCP`/`Canonicalizer` passes
//! (`modular/KGEN/docs/MojoCompilerWalkthrough.md` §"Pre-Elaboration
//! Optimization": "Fewer, smaller, simpler generators are faster to
//! instantiate") -- the point isn't just correctness, it's that the
//! elaborated body should do less work at codegen/runtime than a literal
//! copy-and-substitute would produce.
//!
//! Constant folding is implemented by delegating to [`crate::eval_body`]
//! on a small throwaway body built from just the already-constant operands
//! -- this reuses the interpreter's arithmetic/comparison/logic semantics
//! instead of re-implementing them, so there is exactly one place that
//! defines what `+`/`==`/etc. mean at compile time.

use codira_mir::{Attr, Body, Generator, Op, OpId, OpKind, Region};
use rustc_hash::{FxHashMap, FxHashSet};
use smol_str::SmolStr;

use crate::value::Value;
use crate::Env;

/// Elaborates `generator`'s body under `bindings` (name -> concrete value
/// for each compile-time parameter the caller has resolved; parameters not
/// present in `bindings` are left as `param.ref` -- partial specialization
/// is allowed, matching KGEN's parameters being independently bindable).
pub fn elaborate(generator: &Generator, bindings: &[(SmolStr, Value)]) -> Body {
    let mut env = Env::new();
    for (name, value) in bindings {
        env.bind(name.clone(), *value);
    }
    let elaborated = Elaborator { env: &env }.elaborate_body(&generator.body);
    compact(&elaborated)
}

/// Dead-code elimination: keeps only ops reachable from `body`'s result
/// (transitively through operands and, recursively, through the bodies of
/// any regions those ops carry), renumbering as it goes. Substitution and
/// folding above leave the ops they replaced behind in the arena (it's
/// append-only) -- this pass is what actually makes the "fewer ops than
/// naive substitution" claim true of the *returned* body, not just of the
/// values it would compute if you bothered to walk it.
fn compact(body: &Body) -> Body {
    let Some(root) = body.result() else {
        return Body::new();
    };

    let mut reachable: FxHashSet<OpId> = FxHashSet::default();
    let mut stack = vec![root];
    while let Some(id) = stack.pop() {
        if !reachable.insert(id) {
            continue;
        }
        for operand in &body.get(id).operands {
            stack.push(*operand);
        }
    }

    let mut new_body = Body::new();
    let mut map: FxHashMap<OpId, OpId> = FxHashMap::default();
    for (old_id, op) in body.iter() {
        if !reachable.contains(&old_id) {
            continue;
        }
        let operands: Vec<OpId> = op.operands.iter().map(|id| map[id]).collect();
        let regions: Vec<Region> = op
            .regions
            .iter()
            .map(|r| Region::new(compact(&r.body)))
            .collect();
        let new_id = new_body.push_with_regions(op.kind.clone(), operands, regions);
        map.insert(old_id, new_id);
    }
    new_body
}

struct Elaborator<'a> {
    env: &'a Env,
}

impl Elaborator<'_> {
    fn elaborate_body(&self, old: &Body) -> Body {
        let mut new_body = Body::new();
        let mut map: FxHashMap<OpId, OpId> = FxHashMap::default();
        for (old_id, op) in old.iter() {
            let new_id = self.elaborate_op(op, &map, &mut new_body);
            map.insert(old_id, new_id);
        }
        new_body
    }

    fn elaborate_op(
        &self,
        op: &Op,
        map: &FxHashMap<OpId, OpId>,
        new_body: &mut Body,
    ) -> OpId {
        match &op.kind {
            OpKind::ParamRef(name) => match self.env.lookup(name) {
                Ok(value) => new_body.push(OpKind::Const(value_to_attr(value)), []),
                // Not bound (partial specialization): pass the reference
                // through unresolved, exactly as written.
                Err(_) => new_body.push(OpKind::ParamRef(name.clone()), []),
            },

            OpKind::Const(_) | OpKind::Arg(_) => new_body.push(op.kind.clone(), []),

            OpKind::If => {
                let cond_new = map[&op.operands[0]];
                let then_region = Region::new(self.elaborate_body(&op.regions[0].body));
                let else_region = Region::new(self.elaborate_body(&op.regions[1].body));

                match const_attr(new_body, cond_new) {
                    Some(Attr::Bool(cond)) => {
                        let chosen = if cond { then_region } else { else_region };
                        splice(&chosen.body, new_body)
                    }
                    _ => new_body.push_with_regions(OpKind::If, [cond_new], [then_region, else_region]),
                }
            }

            // `cf.for`/`cf.while`: no interpreter support to fold with yet
            // (see codira_comptime's own EvalError::UnsupportedOp) -- copy
            // the (recursively elaborated) region through unchanged rather
            // than failing the whole elaboration over one loop.
            OpKind::For | OpKind::While => {
                let regions: Vec<Region> = op
                    .regions
                    .iter()
                    .map(|r| Region::new(self.elaborate_body(&r.body)))
                    .collect();
                let operands: Vec<OpId> = op.operands.iter().map(|id| map[id]).collect();
                new_body.push_with_regions(op.kind.clone(), operands, regions)
            }

            // Every other op kind is a pure, operand-only computation
            // (arithmetic/comparison/logic/negation) -- fold it if every
            // remapped operand is now constant.
            pure_kind => {
                let operands: Vec<OpId> = op.operands.iter().map(|id| map[id]).collect();
                if let Some(folded) = try_fold(new_body, pure_kind, &operands) {
                    new_body.push(OpKind::Const(folded), [])
                } else {
                    new_body.push(pure_kind.clone(), operands)
                }
            }
        }
    }
}

/// If `id` names a `Const` op in `body`, returns its attribute.
fn const_attr(body: &Body, id: OpId) -> Option<Attr> {
    match &body.get(id).kind {
        OpKind::Const(attr) => Some(attr.clone()),
        _ => None,
    }
}

/// Attempts to constant-fold `kind` applied to `operands`, all of which
/// must already be `Const` ops in `body`. Builds a minimal throwaway body
/// reproducing just this one operation and evaluates it via
/// [`crate::eval_body`], so the fold is defined by the same code as the
/// interpreter itself.
fn try_fold(body: &Body, kind: &OpKind, operands: &[OpId]) -> Option<Attr> {
    let attrs: Vec<Attr> = operands
        .iter()
        .map(|id| const_attr(body, *id))
        .collect::<Option<Vec<_>>>()?;

    let mut probe = Body::new();
    let ids: Vec<OpId> = attrs
        .into_iter()
        .map(|attr| probe.push(OpKind::Const(attr), []))
        .collect();
    probe.push(kind.clone(), ids);

    crate::eval_body(&probe, &Env::new()).ok().map(value_to_attr)
}

/// Copies every op in `src` into `dst`, remapping operand references, and
/// returns the id of the final (last) op in `dst`'s numbering -- used to
/// inline a taken `if`/`else` branch directly into its parent body once
/// the condition has folded to a constant.
fn splice(src: &Body, dst: &mut Body) -> OpId {
    let mut map: FxHashMap<OpId, OpId> = FxHashMap::default();
    let mut last = None;
    for (old_id, op) in src.iter() {
        let operands: Vec<OpId> = op.operands.iter().map(|id| map[id]).collect();
        let regions: Vec<Region> = op
            .regions
            .iter()
            .map(|r| {
                let mut sub = Body::new();
                splice(&r.body, &mut sub);
                Region::new(sub)
            })
            .collect();
        let new_id = dst.push_with_regions(op.kind.clone(), operands, regions);
        map.insert(old_id, new_id);
        last = Some(new_id);
    }
    match last {
        Some(id) => id,
        // Empty branch (e.g. `if cond { .. }` with no `else`): its value is
        // Unit, matching the interpreter's `OpKind::If` semantics.
        None => dst.push(OpKind::Const(Attr::Unit), []),
    }
}

fn value_to_attr(value: Value) -> Attr {
    match value {
        Value::Int(v) => Attr::Int(v),
        Value::Bool(v) => Attr::Bool(v),
        Value::Unit => Attr::Unit,
    }
}
