//! The compile-time interpreter.
//!
//! Structural analog of `KGEN/lib/Interpreter` -- evaluates a `codira_mir`
//! [`Body`] to a concrete [`Value`], for plain `comptime { .. }` blocks,
//! generator parameter expressions, and (via [`EvalCtx::store`]) whole
//! call graphs of generators. See `spec/EIDOS_ARCHITECTURE.md` §4.
//!
//! # Evaluation model
//!
//! A [`Body`] is evaluated **strictly, top to bottom**: every op in a
//! region body is evaluated exactly once, in arena order, and its value is
//! recorded so later ops read operands by table lookup rather than
//! re-evaluating them. This matches the IR's SSA-by-construction shape
//! (operands only point backwards) and makes fuel accounting exact: one
//! unit of fuel per op evaluated. Two consequences worth stating:
//!
//! * Dead ops still evaluate (and can fail -- a dead division by zero is an
//!   error here). Removing dead code is the elaborator's `compact` pass's job,
//!   not the interpreter's; the interpreter is the semantics reference and does
//!   exactly what the IR says.
//! * `core.and`/`core.or` are **non-short-circuiting**, as documented on
//!   `codira_mir::fold::fold_op`: both operand ops were already evaluated by
//!   the time the `and`/`or` runs. Short-circuiting is a control-flow concern
//!   -- frontends lower `a && b` to `cf.if` when they need it.
//!
//! All pure ops (arithmetic/comparison/logic/bitwise/shift/neg/not)
//! delegate to [`codira_mir::fold_op`] -- the single source of pure-op
//! semantics shared with the constant folder and e-graph (KGEN's
//! "interpreter reuses fold hooks" design, `Interpreter.md`). The
//! interpreter's own job is exactly the part `fold_op` refuses
//! (`NotFoldable`): control flow, block arguments, tuples, and calls.
//!
//! # Loops, block arguments, and calls
//!
//! `cf.while`/`cf.for` follow the loop-carried-value contract documented
//! on `codira_mir::OpKind::While`/`For` exactly: region block arguments are
//! maintained as a stack of frames (one frame per *loop* region entered --
//! `cf.if` regions share the enclosing frame, matching the verifier's
//! scoping), and `cf.yield` in the body region names the next iteration's
//! carried values. `core.call` resolves its symbol against the
//! [`GeneratorStore`] (never at IR construction time -- see
//! `OpKind::Call`'s doc), evaluates the callee's body with `core.arg i`
//! bound to the i-th evaluated call operand, a **fresh** block-arg stack,
//! and the **same** [`Env`]: generator parameter bindings are shared
//! module-level comptime bindings, while per-call data travels exclusively
//! through `core.arg` values.

use codira_mir::{
    fold_cast, fold_typed_op, normalize, Attr, Body, CastMode, FoldError, GeneratorStore, MemOp,
    Op, OpId, OpKind,
};
use rustc_hash::FxHashMap;

use crate::{
    value::{EvalError, MemState, Value},
    Env,
};

/// Default fuel budget: the number of op evaluations one top-level
/// [`eval_body`]/[`eval_body_with`] call may spend before failing with
/// [`EvalError::FuelExhausted`]. Generous for real comptime code, small
/// enough that a runaway `while true` fails in well under a second.
pub const DEFAULT_FUEL: u64 = 1_000_000;

/// Default `core.call` nesting limit before
/// [`EvalError::CallDepthExceeded`]. Deliberately far below the thread
/// stack limit: the interpreter recurses natively per call, and 128
/// comptime call frames is already deeply unusual code.
pub const DEFAULT_CALL_DEPTH: usize = 128;

/// Evaluation context: interprocedural symbol resolution plus resource
/// limits. Construct with [`EvalCtx::new`] (defaults, no store) or
/// [`EvalCtx::with_store`], adjust fields directly if a test or caller
/// needs a different budget, then call [`EvalCtx::eval`].
///
/// The two free functions [`eval_body`] and [`eval_body_with`] are
/// conveniences over this type and cover almost every caller.
#[derive(Debug, Clone, Copy)]
pub struct EvalCtx<'a> {
    /// Symbol resolution for `core.call` -- `None` means any call fails
    /// with [`EvalError::UnknownCallee`].
    pub store: Option<&'a GeneratorStore>,
    /// Op-evaluation budget -- see [`DEFAULT_FUEL`].
    pub fuel: u64,
    /// Maximum `core.call` nesting -- see [`DEFAULT_CALL_DEPTH`].
    pub call_depth: usize,
}

impl Default for EvalCtx<'_> {
    fn default() -> Self {
        Self {
            store: None,
            fuel: DEFAULT_FUEL,
            call_depth: DEFAULT_CALL_DEPTH,
        }
    }
}

impl<'a> EvalCtx<'a> {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn with_store(store: &'a GeneratorStore) -> Self {
        Self {
            store: Some(store),
            ..Self::default()
        }
    }

    /// Evaluates `body` to its result value under this context's store and
    /// limits.
    pub fn eval(&self, body: &Body, env: &Env) -> Result<Value, EvalError> {
        let mut interp = Interp {
            store: self.store,
            fuel: self.fuel,
            depth: 0,
            depth_limit: self.call_depth,
            next_alloc: 0,
        };
        interp.eval_toplevel(body, env)
    }
}

/// Evaluates a [`Body`] to its result value (the value of its last op),
/// under the given parameter bindings, with default limits and no
/// generator store (so `core.call` fails with `UnknownCallee`).
pub fn eval_body(body: &Body, env: &Env) -> Result<Value, EvalError> {
    EvalCtx::new().eval(body, env)
}

/// Like [`eval_body`], but with an optional [`GeneratorStore`] for
/// resolving `core.call` symbols. Default fuel/call-depth limits; use
/// [`EvalCtx`] directly to customize those.
pub fn eval_body_with(
    body: &Body,
    env: &Env,
    store: Option<&GeneratorStore>,
) -> Result<Value, EvalError> {
    EvalCtx {
        store,
        ..EvalCtx::default()
    }
    .eval(body, env)
}

/// Internal mutable interpreter state. Split from [`EvalCtx`] so the
/// public context stays a plain immutable configuration value.
struct Interp<'a> {
    store: Option<&'a GeneratorStore>,
    fuel: u64,
    depth: usize,
    depth_limit: usize,
    /// The id the next `mem.alloc` evaluated gets. Allocations are
    /// numbered in evaluation order, so the same body always allocates the
    /// same ids.
    next_alloc: u32,
}

/// What evaluating one region body produced: a plain result value (or
/// `None` for an empty body), or -- for loop body regions only -- the
/// values named by the terminating `cf.yield`.
enum Outcome {
    Value(Option<Value>),
    Yielded(Vec<Value>),
}

impl Interp<'_> {
    fn eval_toplevel(&mut self, body: &Body, env: &Env) -> Result<Value, EvalError> {
        let mut frames = Vec::new();
        match self.eval_in(body, env, &[], &mut frames, false)? {
            Outcome::Value(Some(value)) => Ok(value),
            Outcome::Value(None) => Err(EvalError::EmptyRegion),
            // `eval_in` only yields when `expect_yield` is set.
            Outcome::Yielded(_) => unreachable!("yield outcome without expect_yield"),
        }
    }

    /// Evaluates every op of `body` in order.
    ///
    /// * `args` -- the enclosing `core.call`'s evaluated operands, read by
    ///   `core.arg` (empty at top level, where `core.arg` is an error).
    /// * `frames` -- the block-argument stack: one frame per enclosing *loop*
    ///   region, innermost last, read by `cf.block_arg`.
    /// * `expect_yield` -- true iff `body` is a loop body region, whose last op
    ///   must be `cf.yield`; the yielded values are returned as
    ///   [`Outcome::Yielded`].
    fn eval_in(
        &mut self,
        body: &Body,
        env: &Env,
        args: &[Value],
        frames: &mut Vec<Vec<Value>>,
        expect_yield: bool,
    ) -> Result<Outcome, EvalError> {
        let mut values: FxHashMap<OpId, Value> = FxHashMap::default();
        let last_id = body.result();
        let mut last_value = None;

        for (id, op) in body.iter() {
            self.spend_fuel()?;

            if let OpKind::Yield = op.kind {
                if expect_yield && Some(id) == last_id {
                    let yielded = op
                        .operands
                        .iter()
                        .map(|operand| operand_value(&values, *operand))
                        .collect::<Result<Vec<_>, _>>()?;
                    return Ok(Outcome::Yielded(yielded));
                }
                // Outside a loop body's terminator position, `cf.yield`
                // is structurally invalid (the verifier rejects it too).
                return Err(EvalError::MalformedLoop(
                    "cf.yield is only valid as the last op of a loop body region",
                ));
            }

            let value = self.eval_op(body, op, &values, env, args, frames)?;
            last_value = Some(value.clone());
            values.insert(id, value);
        }

        if expect_yield {
            // Non-empty bodies ending in `Yield` returned above; empty or
            // yield-less loop bodies are malformed.
            return Err(EvalError::MalformedLoop(
                "loop body region must end in cf.yield",
            ));
        }
        Ok(Outcome::Value(last_value))
    }

    fn eval_op(
        &mut self,
        body: &Body,
        op: &Op,
        values: &FxHashMap<OpId, Value>,
        env: &Env,
        args: &[Value],
        frames: &mut Vec<Vec<Value>>,
    ) -> Result<Value, EvalError> {
        match &op.kind {
            // In the representation the op's type has at run time: an `f32`
            // literal is an `f32`, not the `f64` it was parsed as.
            OpKind::Const(attr) => attr_to_value(&normalize(attr.clone(), op.ty), env),

            OpKind::ParamRef(name) => env.lookup(name),

            OpKind::Arg(index) => match args.get(*index as usize) {
                Some(value) => Ok(value.clone()),
                // At top level (or with too few call operands) argument
                // #index has no compile-time value -- same honest error
                // the pre-region-IR interpreter reported.
                None => Err(EvalError::RuntimeArgument(*index)),
            },

            OpKind::BlockArg(index) => {
                let frame = frames.last().ok_or(EvalError::MalformedLoop(
                    "cf.block_arg outside any loop region",
                ))?;
                frame
                    .get(*index as usize)
                    .cloned()
                    .ok_or(EvalError::MalformedLoop(
                        "cf.block_arg index out of range for enclosing region",
                    ))
            }

            OpKind::Tuple => {
                let elements = op
                    .operands
                    .iter()
                    .map(|operand| operand_value(values, *operand))
                    .collect::<Result<Vec<_>, _>>()?;
                Ok(Value::Tuple(elements))
            }

            OpKind::TupleGet(index) => {
                let [operand] = op.operands.as_slice() else {
                    return Err(EvalError::Fold(codira_mir::FoldError::WrongOperandCount));
                };
                match operand_value(values, *operand)? {
                    Value::Tuple(elements) => elements
                        .into_iter()
                        .nth(*index as usize)
                        .ok_or(EvalError::MalformedTuple),
                    other => Err(EvalError::TypeMismatch {
                        expected: "tuple",
                        found: other.type_name(),
                    }),
                }
            }

            OpKind::If => {
                let [cond_id] = op.operands.as_slice() else {
                    return Err(EvalError::MalformedIf);
                };
                let [then_region, else_region] = op.regions.as_slice() else {
                    return Err(EvalError::MalformedIf);
                };
                let cond = operand_value(values, *cond_id)?.as_bool()?;
                let region = if cond { then_region } else { else_region };
                // `cf.if` regions take no block arguments and share the
                // enclosing frame (verifier: `cf.if regions take no block
                // arguments`, block-arg scope passes through) -- so no
                // frame push here.
                match self.eval_in(&region.body, env, args, frames, false)? {
                    Outcome::Value(Some(value)) => Ok(value),
                    // Empty branch (`if c { .. }` with no else): Unit.
                    Outcome::Value(None) => Ok(Value::Unit),
                    Outcome::Yielded(_) => unreachable!("if region cannot yield"),
                }
            }

            OpKind::While => {
                let [cond_region, body_region] = op.regions.as_slice() else {
                    return Err(EvalError::MalformedLoop(
                        "cf.while takes exactly two regions (cond, body)",
                    ));
                };
                let mut carried = op
                    .operands
                    .iter()
                    .map(|operand| operand_value(values, *operand))
                    .collect::<Result<Vec<_>, _>>()?;
                let n = carried.len();
                if cond_region.num_args as usize != n || body_region.num_args as usize != n {
                    return Err(EvalError::MalformedLoop(
                        "cf.while regions must take one block arg per carried value",
                    ));
                }
                loop {
                    // One fuel per iteration on top of the per-op charges,
                    // so even a degenerate loop body cannot iterate for
                    // free.
                    self.spend_fuel()?;

                    frames.push(carried.clone());
                    let cond = self
                        .eval_region_value(&cond_region.body, env, args, frames)
                        .and_then(|v| match v {
                            Some(value) => value.as_bool(),
                            None => Err(EvalError::MalformedLoop(
                                "cf.while cond region must not be empty",
                            )),
                        });
                    let cond = match cond {
                        Ok(cond) => cond,
                        Err(err) => {
                            frames.pop();
                            return Err(err);
                        }
                    };
                    if !cond {
                        frames.pop();
                        break;
                    }
                    let next = self.eval_in(&body_region.body, env, args, frames, true);
                    frames.pop();
                    match next? {
                        Outcome::Yielded(next) if next.len() == n => carried = next,
                        // `eval_in(expect_yield = true)` errors rather than
                        // returning a plain value; a yield of the wrong
                        // arity is the one shape left to reject here.
                        Outcome::Yielded(_) => {
                            return Err(EvalError::MalformedLoop(
                                "cf.yield must name exactly one value per loop-carried value",
                            ))
                        }
                        Outcome::Value(_) => unreachable!("loop body must yield"),
                    }
                }
                Ok(carried_result(carried))
            }

            OpKind::For => {
                if op.operands.len() < 3 {
                    return Err(EvalError::MalformedLoop(
                        "cf.for takes at least three operands (start, end, step)",
                    ));
                }
                let [body_region] = op.regions.as_slice() else {
                    return Err(EvalError::MalformedLoop(
                        "cf.for takes exactly one region (body)",
                    ));
                };
                let start = operand_value(values, op.operands[0])?.as_int()?;
                let end = operand_value(values, op.operands[1])?.as_int()?;
                let step = operand_value(values, op.operands[2])?.as_int()?;
                if step == 0 {
                    return Err(EvalError::MalformedLoop("cf.for step must be non-zero"));
                }
                let mut carried = op.operands[3..]
                    .iter()
                    .map(|operand| operand_value(values, *operand))
                    .collect::<Result<Vec<_>, _>>()?;
                let n = carried.len();
                if body_region.num_args as usize != n + 1 {
                    return Err(EvalError::MalformedLoop(
                        "cf.for body must take the induction variable plus one block arg per carried value",
                    ));
                }

                // `iv` advances by wrapping i64 addition, consistent with
                // `Attr::Int`'s wrapping arithmetic everywhere else; a
                // pathological wrap-around loop is bounded by fuel, not by
                // undefined behavior.
                let mut iv = start;
                while if step > 0 { iv < end } else { iv > end } {
                    self.spend_fuel()?;

                    let mut frame = Vec::with_capacity(n + 1);
                    frame.push(Value::Int(iv));
                    frame.extend(carried.iter().cloned());
                    frames.push(frame);
                    let next = self.eval_in(&body_region.body, env, args, frames, true);
                    frames.pop();
                    match next? {
                        Outcome::Yielded(next) if next.len() == n => carried = next,
                        Outcome::Yielded(_) => {
                            return Err(EvalError::MalformedLoop(
                                "cf.yield must name exactly one value per loop-carried value",
                            ))
                        }
                        Outcome::Value(_) => unreachable!("loop body must yield"),
                    }
                    iv = iv.wrapping_add(step);
                }
                Ok(carried_result(carried))
            }

            OpKind::Call(symbol) => {
                let store = self
                    .store
                    .ok_or_else(|| EvalError::UnknownCallee(symbol.clone()))?;
                let (_, generator) = store
                    .generator_by_name(symbol)
                    .ok_or_else(|| EvalError::UnknownCallee(symbol.clone()))?;

                let call_args = op
                    .operands
                    .iter()
                    .map(|operand| operand_value(values, *operand))
                    .collect::<Result<Vec<_>, _>>()?;

                if self.depth >= self.depth_limit {
                    return Err(EvalError::CallDepthExceeded {
                        limit: self.depth_limit,
                    });
                }
                self.depth += 1;
                // Fresh block-arg stack: the callee's regions are scoped to
                // its own body. Same `env`: generator parameter bindings
                // are shared module-level comptime bindings (see the
                // module doc); the call-specific data is `call_args`.
                let mut callee_frames = Vec::new();
                let result =
                    self.eval_in(&generator.body, env, &call_args, &mut callee_frames, false);
                self.depth -= 1;
                match result? {
                    Outcome::Value(Some(value)) => Ok(value),
                    Outcome::Value(None) => Err(EvalError::EmptyRegion),
                    Outcome::Yielded(_) => unreachable!("function body cannot yield"),
                }
            }

            OpKind::Yield => unreachable!("yield handled in eval_in"),

            OpKind::Mem(mem) => self.eval_mem(*mem, op, values),

            // Every remaining kind is a pure attribute-level computation:
            // delegate to the shared fold hook. Scalar `Value`s convert to
            // `Attr`s losslessly; `Tuple` has no `Attr` form, so using a
            // tuple as a pure-op operand is a type mismatch.
            // A conversion's meaning depends on both its source and target
            // types, so it is folded by `fold_cast` with both. An operand
            // whose type is unknown makes it decline (an error here), never
            // guess.
            OpKind::Cast(kind, CastMode::Wrapping) => {
                let [operand] = op.operands.as_slice() else {
                    return Err(EvalError::Fold(FoldError::WrongOperandCount));
                };
                let attr = value_to_attr(&operand_value(values, *operand)?)?;
                let folded = fold_cast(*kind, &attr, body.get(*operand).ty, op.ty)?;
                attr_to_value(&folded, env)
            }

            pure => {
                let attrs = op
                    .operands
                    .iter()
                    .map(|operand| value_to_attr(&operand_value(values, *operand)?))
                    .collect::<Result<Vec<_>, _>>()?;
                let operand_tys: Vec<_> = op.operands.iter().map(|o| body.get(*o).ty).collect();
                let folded = fold_typed_op(pure, &attrs, &operand_tys, op.ty)?;
                attr_to_value(&folded, env)
            }
        }
    }

    /// Evaluates a non-loop-body region (a `cf.while` cond region) to its
    /// result value, `None` if empty.
    fn eval_region_value(
        &mut self,
        body: &Body,
        env: &Env,
        args: &[Value],
        frames: &mut Vec<Vec<Value>>,
    ) -> Result<Option<Value>, EvalError> {
        match self.eval_in(body, env, args, frames, false)? {
            Outcome::Value(value) => Ok(value),
            Outcome::Yielded(_) => unreachable!("non-loop region cannot yield"),
        }
    }

    /// Memory operations -- see `codira_mir::mem` for the model. Memory is
    /// a value here exactly as in the IR: a store returns a new state and the
    /// state it was given stays readable.
    fn eval_mem(
        &mut self,
        mem: MemOp,
        op: &Op,
        values: &FxHashMap<OpId, Value>,
    ) -> Result<Value, EvalError> {
        let operand = |n: usize| -> Result<Value, EvalError> {
            let id = op
                .operands
                .get(n)
                .ok_or(EvalError::Fold(codira_mir::FoldError::WrongOperandCount))?;
            operand_value(values, *id)
        };
        let state = |value: Value| match value {
            Value::Mem(state) => Ok(state),
            other => Err(EvalError::TypeMismatch {
                expected: "memory state",
                found: other.type_name(),
            }),
        };
        let pointer = |value: Value| match value {
            Value::Ptr { alloc, index } => Ok((alloc, index)),
            other => Err(EvalError::TypeMismatch {
                expected: "pointer",
                found: other.type_name(),
            }),
        };
        // The cell a pointer names in a state, checked.
        let cell = |state: &MemState, alloc: u32, index: i64| -> Result<usize, EvalError> {
            let cells = state.cells.get(&alloc).ok_or(EvalError::DanglingPointer)?;
            usize::try_from(index)
                .ok()
                .filter(|&i| i < cells.len())
                .ok_or(EvalError::OutOfBounds {
                    index,
                    len: cells.len(),
                })
        };

        match mem {
            MemOp::Entry => Ok(Value::Mem(MemState::default())),
            MemOp::Alloc { count, .. } => {
                let mut next = state(operand(0)?)?;
                let id = self.next_alloc;
                self.next_alloc += 1;
                std::sync::Arc::make_mut(&mut next.cells).insert(id, vec![None; count as usize]);
                next.fresh = Some(id);
                Ok(Value::Mem(next))
            }
            MemOp::Addr => {
                let alloc = state(operand(0)?)?
                    .fresh
                    .ok_or(EvalError::DanglingPointer)?;
                Ok(Value::Ptr { alloc, index: 0 })
            }
            MemOp::Load => {
                let memory = state(operand(0)?)?;
                let (alloc, index) = pointer(operand(1)?)?;
                let at = cell(&memory, alloc, index)?;
                memory.cells[&alloc][at]
                    .clone()
                    .ok_or(EvalError::UninitializedLoad)
            }
            MemOp::Store => {
                let mut next = state(operand(0)?)?;
                let (alloc, index) = pointer(operand(1)?)?;
                let value = operand(2)?;
                let at = cell(&next, alloc, index)?;
                std::sync::Arc::make_mut(&mut next.cells)
                    .get_mut(&alloc)
                    .expect("checked by cell()")[at] = Some(value);
                next.fresh = None;
                Ok(Value::Mem(next))
            }
            MemOp::Offset { .. } => {
                let (alloc, index) = pointer(operand(0)?)?;
                let delta = operand(1)?.as_int()?;
                Ok(Value::Ptr {
                    alloc,
                    index: index.wrapping_add(delta),
                })
            }
        }
    }

    fn spend_fuel(&mut self) -> Result<(), EvalError> {
        self.fuel = self.fuel.checked_sub(1).ok_or(EvalError::FuelExhausted)?;
        Ok(())
    }
}

/// The result-value convention shared by `cf.while` and `cf.for` (see
/// `OpKind::While`'s doc): the value itself for one carried value, a tuple
/// for several, `Unit` for none.
fn carried_result(mut carried: Vec<Value>) -> Value {
    match carried.len() {
        0 => Value::Unit,
        1 => carried.remove(0),
        _ => Value::Tuple(carried),
    }
}

/// Reads an already-evaluated operand. A miss is impossible on IR the
/// verifier accepts (operands always name earlier ops in the same body);
/// on broken IR it reports the operand as an unresolved computation rather
/// than panicking.
fn operand_value(values: &FxHashMap<OpId, Value>, id: OpId) -> Result<Value, EvalError> {
    values.get(&id).cloned().ok_or(EvalError::TypeMismatch {
        expected: "an evaluated operand",
        found: "a forward or foreign op reference",
    })
}

fn attr_to_value(attr: &Attr, env: &Env) -> Result<Value, EvalError> {
    match attr {
        Attr::Int(v) => Ok(Value::Int(*v)),
        Attr::Bool(v) => Ok(Value::Bool(*v)),
        Attr::Float(bits) => Ok(Value::Float(f64::from_bits(*bits))),
        Attr::Str(s) => Ok(Value::Str(s.clone())),
        Attr::Unit => Ok(Value::Unit),
        Attr::ParamRef(name) => env.lookup(name),
    }
}

/// Converts a concrete scalar [`Value`] to its [`Attr`] form for
/// `fold_op`. Tuples are not attributes (see `Value`'s doc) -- feeding one
/// to a pure op is a type error, not a missing feature.
fn value_to_attr(value: &Value) -> Result<Attr, EvalError> {
    match value {
        Value::Int(v) => Ok(Attr::Int(*v)),
        Value::Bool(v) => Ok(Attr::Bool(*v)),
        Value::Float(v) => Ok(Attr::float(*v)),
        Value::Str(s) => Ok(Attr::Str(s.clone())),
        Value::Unit => Ok(Attr::Unit),
        Value::Tuple(_) | Value::Mem(_) | Value::Ptr { .. } => Err(EvalError::TypeMismatch {
            expected: "a scalar (int, bool, float, string, or unit)",
            found: value.type_name(),
        }),
    }
}
