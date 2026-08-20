//! Concrete compile-time values and the evaluation environment.

use rustc_hash::FxHashMap;
use smol_str::SmolStr;

/// A concrete value produced by interpreting a `codira_mir` region.
///
/// Distinct from `codira_mir::Attr`: an `Attr` may still contain an
/// unresolved `ParamRef`; a `Value` is always fully concrete. Elaboration
/// (see `Env`) is exactly the process of turning `Attr`s into `Value`s.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Value {
    Int(i64),
    Bool(bool),
    Unit,
}

impl Value {
    pub fn type_name(self) -> &'static str {
        match self {
            Value::Int(_) => "int",
            Value::Bool(_) => "bool",
            Value::Unit => "unit",
        }
    }

    pub fn as_int(self) -> Result<i64, EvalError> {
        match self {
            Value::Int(v) => Ok(v),
            other => Err(EvalError::TypeMismatch {
                expected: "int",
                found: other.type_name(),
            }),
        }
    }

    pub fn as_bool(self) -> Result<bool, EvalError> {
        match self {
            Value::Bool(v) => Ok(v),
            other => Err(EvalError::TypeMismatch {
                expected: "bool",
                found: other.type_name(),
            }),
        }
    }
}

/// Bindings for `param.ref`/`ParamRef` names during elaboration -- e.g. `N
/// -> Value::Int(4)` when elaborating `SIMD[f32, 4]`'s generator with `N`
/// bound to `4`. Empty for plain `comptime { .. }` blocks, which reference
/// no generator parameters.
#[derive(Debug, Clone, Default)]
pub struct Env {
    bindings: FxHashMap<SmolStr, Value>,
}

impl Env {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn bind(&mut self, name: impl Into<SmolStr>, value: Value) -> &mut Self {
        self.bindings.insert(name.into(), value);
        self
    }

    pub fn lookup(&self, name: &str) -> Result<Value, EvalError> {
        self.bindings
            .get(name)
            .copied()
            .ok_or_else(|| EvalError::UnresolvedParam(name.into()))
    }
}

/// Everything that can go wrong evaluating a `codira_mir` region at compile
/// time. Deliberately does not have a catch-all `Other(String)` variant --
/// every failure mode this interpreter can hit is enumerated, so callers
/// (and tests) can match exhaustively instead of string-matching messages.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum EvalError {
    #[error("type mismatch: expected {expected}, found {found}")]
    TypeMismatch {
        expected: &'static str,
        found: &'static str,
    },
    #[error("unresolved parameter reference: {0}")]
    UnresolvedParam(SmolStr),
    #[error("division by zero")]
    DivideByZero,
    #[error("empty region has no result value")]
    EmptyRegion,
    #[error("`cf.if` requires exactly one condition operand and two regions (then, else)")]
    MalformedIf,
    #[error("{0} is not yet interpreted -- see spec/KGEN_SUPERSET_STATUS.md")]
    UnsupportedOp(&'static str),
}
