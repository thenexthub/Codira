//! The core IR data model: [`Op`], [`OpKind`], [`Attr`], [`Body`], [`Region`].
//!
//! This mirrors MLIR's generic `Operation`/`Region`/`Attribute` model (and,
//! more specifically, the shape of KGEN's `kgen`/`pop`/`hlcf` dialects --
//! see `spec/KGEN_SUPERSET_ARCHITECTURE.md` §2) without a TableGen/C++
//! dialect-registration mechanism: op kinds are grouped into "dialects" by
//! naming convention on the [`OpKind`] variants (`core.*`, `param.*`,
//! `simd.*`, `cf.*`) rather than by separate Rust crates or types, per
//! §7.2's "one IR, not four dialects" simplification.

use la_arena::{Arena, Idx};
use smallvec::SmallVec;
use smol_str::SmolStr;

/// A compile-time-known value attached to an [`Op`].
///
/// KGEN's terminology (`DesignOverview.md`, "Generator Parameter
/// Arguments"): attributes are *not* SSA values -- they are the meta-program
/// data a generator acts on at elaboration time, as opposed to [`OpId`]
/// operands, which are ordinary SSA values computed at (kernel) runtime.
/// Both distinctions are preserved here.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Attr {
    Int(i64),
    Bool(bool),
    Unit,
    /// An unresolved reference to a generator parameter (e.g. `N` in
    /// `SIMD[T, N: usize]`) -- only becomes a concrete `Int`/`Bool`/etc.
    /// during elaboration (`codira_comptime`). See architecture doc §3.
    ParamRef(SmolStr),
}

/// One IR operation. Grouped into "dialects" by variant name prefix in the
/// doc comment below, matching the table in the architecture doc §2.1.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum OpKind {
    // ---- core.* (KGEN analog: concrete `kgen.*` ops) ----------------------
    /// `core.const` -- materializes a literal [`Attr`] as a value.
    Const(Attr),
    Add,
    Sub,
    Mul,
    Div,
    Rem,
    Neg,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
    And,
    Or,
    Not,

    // ---- param.* (KGEN analog: `kgen.param.*`) -----------------------------
    /// `param.ref` -- reference to a not-yet-resolved generator parameter
    /// (compile-time; substituted away by elaboration, see
    /// `codira_comptime`'s `elaborate` module and architecture doc §4).
    ParamRef(SmolStr),
    /// `core.arg` -- reference to the generator's Nth *runtime* argument
    /// (KGEN's distinction, `DesignOverview.md` "Generator/Function
    /// Arguments" vs. "Generator Parameter Arguments": arguments are SSA
    /// values computed at call time, parameters are compile-time-known).
    /// Elaboration passes these through unchanged -- specializing a
    /// generator's parameters must not, and cannot, eliminate genuine
    /// runtime inputs.
    Arg(u32),

    // ---- cf.* (KGEN analog: `hlcf.*`) --------------------------------------
    /// `cf.if` -- one operand (the condition), two regions (`then`, `else`).
    /// Both regions must be present; a missing `else` is represented as an
    /// empty region yielding `Attr::Unit`.
    If,
    /// `cf.for` -- structured for-loop. Declared for IR completeness (see
    /// architecture doc §2.1) but **not yet interpreted** by
    /// `codira_comptime` this session -- see spec/KGEN_SUPERSET_STATUS.md.
    For,
    /// `cf.while` -- structured while-loop. Same status as `For`.
    While,
    // ---- simd.* (KGEN analog: `pop.*`) -------------------------------------
    // Deliberately not modeled yet: no SIMD op variants exist until a real
    // consumer (the std/builtin/simd.code lowering) is wired up. Adding
    // speculative variants with no interpreter/codegen support would just be
    // unverified surface area -- see architecture doc §8 non-goals.
}

/// One operation: a kind, its SSA operands (other ops in the same [`Body`]),
/// and any nested [`Region`]s (e.g. `cf.if`'s `then`/`else` bodies).
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Op {
    pub kind: OpKind,
    pub operands: SmallVec<[OpId; 2]>,
    pub regions: SmallVec<[Region; 0]>,
}

pub type OpId = Idx<Op>;

/// A straight-line sequence of [`Op`]s. Its "result" (matching MLIR's
/// single-block-region-yields-last-value convention, used here instead of a
/// full multi-block CFG since nothing in this IR yet needs back-edges within
/// a single region -- `cf.for`/`cf.while` bodies are themselves `Body`s, not
/// arbitrary block graphs) is the value of its last op, if any.
#[derive(Debug, Clone, PartialEq, Eq, Hash, Default)]
pub struct Body {
    ops: Arena<Op>,
}

impl Body {
    pub fn new() -> Self {
        Self { ops: Arena::new() }
    }

    pub fn push(&mut self, kind: OpKind, operands: impl IntoIterator<Item = OpId>) -> OpId {
        self.ops.alloc(Op {
            kind,
            operands: operands.into_iter().collect(),
            regions: SmallVec::new(),
        })
    }

    pub fn push_with_regions(
        &mut self,
        kind: OpKind,
        operands: impl IntoIterator<Item = OpId>,
        regions: impl IntoIterator<Item = Region>,
    ) -> OpId {
        self.ops.alloc(Op {
            kind,
            operands: operands.into_iter().collect(),
            regions: regions.into_iter().collect(),
        })
    }

    pub fn get(&self, id: OpId) -> &Op {
        &self.ops[id]
    }

    /// The op whose value this body evaluates to: its last op, if any.
    pub fn result(&self) -> Option<OpId> {
        self.ops.iter().last().map(|(id, _)| id)
    }

    pub fn is_empty(&self) -> bool {
        self.ops.iter().next().is_none()
    }

    pub fn iter(&self) -> impl Iterator<Item = (OpId, &Op)> {
        self.ops.iter()
    }
}

/// A nested region: just a [`Body`] today. Kept as a distinct type (rather
/// than using `Body` directly as the field type on [`Op`]) so the
/// architecture doc's "Region" terminology has a stable, addressable Rust
/// name to grow block-arguments/multi-block support into later without
/// another rename.
#[derive(Debug, Clone, PartialEq, Eq, Hash, Default)]
pub struct Region {
    pub body: Body,
}

impl Region {
    pub fn new(body: Body) -> Self {
        Self { body }
    }
}
