//! Copyright (c) 2026 Omnira CJSC
//!
//! The pass manager and the passes that run between elaboration and
//! codegen -- the analog of KGEN's `lib/Transforms` plus the pipeline
//! composition in `lib/Compiler/Pipeline/Pipeline.cpp`.
//!
//! A [`Pass`] rewrites one [`Body`] (function-at-a-time, like MLIR's
//! function passes; interprocedural context, when a pass needs it, comes
//! in via [`PassContext`]'s generator store). Passes must preserve the
//! verifier's invariants: [`PassManager::run`] re-verifies after every
//! pass in debug builds and returns the first violation as an error, so a
//! buggy rewrite is caught at the pass boundary that introduced it, not
//! three passes later.

mod const_fold;
mod cse;
mod dce;
mod inline;
mod rewrite;
mod simplify;
mod unroll;

pub use const_fold::ConstFoldPass;
pub use cse::CsePass;
pub use dce::DcePass;
pub use inline::InlinePass;
pub use simplify::SimplifyPass;
pub use unroll::UnrollPass;

use crate::{
    op::Body,
    verify::{verify_body, VerifyError},
    GeneratorStore,
};

/// Read-only context shared by every pass in a pipeline run.
#[derive(Default)]
pub struct PassContext<'a> {
    /// Symbol resolution for `core.call` -- `None` when running on a body
    /// with no interprocedural context (unit tests, comptime probes).
    pub generators: Option<&'a GeneratorStore>,
}

/// Whether a pass changed anything -- pipelines use this for fixpoint
/// iteration ([`PassManager::run_to_fixpoint`]).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Changed {
    Yes,
    No,
}

impl Changed {
    pub fn from_bool(changed: bool) -> Changed {
        if changed {
            Changed::Yes
        } else {
            Changed::No
        }
    }

    pub fn or(self, other: Changed) -> Changed {
        if self == Changed::Yes || other == Changed::Yes {
            Changed::Yes
        } else {
            Changed::No
        }
    }
}

/// One body-to-body rewrite. Implementations live in this module's
/// submodules; pipelines are composed with [`PassManager`].
pub trait Pass {
    /// Stable, kebab-case name (KGEN convention: `sccp`, `simplify-cf`).
    fn name(&self) -> &'static str;

    /// Rewrites `body` in place. Must leave `body` verifier-clean.
    fn run(&self, body: &mut Body, ctx: &PassContext<'_>) -> Changed;
}

/// A straight-line sequence of passes with per-pass re-verification.
#[derive(Default)]
pub struct PassManager {
    passes: Vec<Box<dyn Pass>>,
}

impl PassManager {
    pub fn new() -> Self {
        Self::default()
    }

    /// Appends a pass to the pipeline.
    //
    // Not `std::ops::Add`: this is a builder that consumes and returns
    // the manager, not an arithmetic operation.
    #[allow(clippy::should_implement_trait)]
    pub fn add(mut self, pass: impl Pass + 'static) -> Self {
        self.passes.push(Box::new(pass));
        self
    }

    /// Runs every pass once, in order. Verifies after each pass; the
    /// error names the pass that broke the IR.
    pub fn run(&self, body: &mut Body, ctx: &PassContext<'_>) -> Result<Changed, PipelineError> {
        let mut changed = Changed::No;
        for pass in &self.passes {
            changed = changed.or(pass.run(body, ctx));
            if let Err(source) = verify_body(body) {
                return Err(PipelineError {
                    pass: pass.name(),
                    source,
                });
            }
        }
        Ok(changed)
    }

    /// Runs the whole pipeline repeatedly until nothing changes (or the
    /// iteration cap is hit -- a safety net against ping-ponging rewrite
    /// pairs, which are a pass bug but should degrade to "stops improving"
    /// rather than "hangs the compiler").
    pub fn run_to_fixpoint(
        &self,
        body: &mut Body,
        ctx: &PassContext<'_>,
        max_iterations: usize,
    ) -> Result<Changed, PipelineError> {
        let mut ever_changed = Changed::No;
        for _ in 0..max_iterations {
            match self.run(body, ctx)? {
                Changed::Yes => ever_changed = Changed::Yes,
                Changed::No => break,
            }
        }
        Ok(ever_changed)
    }
}

/// A pass broke the structural invariants: which pass, and what rule.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
#[error("pass `{pass}` produced invalid IR: {source}")]
pub struct PipelineError {
    pub pass: &'static str,
    pub source: VerifyError,
}

/// The cleanup pipeline: constant propagation, algebraic canonicalization,
/// redundancy elimination, then dead-code removal.
///
/// # Phase ordering
///
/// The order is the one KGEN's own post-elaboration pipeline uses
/// (`modular/KGEN/lib/Compiler/Pipeline/Pipeline.cpp`,
/// `buildFirstOptPipeline`: `SCCP` -> `Canonicalizer` -> `CSE` ->
/// `EliminateDeadSymbols`), and for the same reasons:
///
/// 1. **`const-fold` first.** Constants are what unlock everything downstream
///    -- `simplify` recognizes `x * 1` only once the `1` is a literal, and
///    `unroll` needs constant loop bounds.
/// 2. **`simplify` second.** Canonicalization (commutative operand order, `Gt`
///    -> `Lt`, strength reduction) makes syntactically different but
///    semantically identical expressions *structurally* identical, which is
///    precisely what CSE keys on.
/// 3. **`cse` third.** With canonical forms in hand, duplicate computations
///    collapse to one definition.
/// 4. **`dce` last.** Every pass above leaves orphaned definitions behind (the
///    arena is append-only); this is what makes "fewer ops than we started
///    with" true of the returned body.
///
/// This ordering is a heuristic, and a fixed order is exactly the
/// phase-ordering problem compilers have lived with for fifty years:
/// `simplify` can expose a fold that `const-fold` already ran past.
/// [`PassManager::run_to_fixpoint`] mitigates it by iterating;
/// `codira_egraph`'s equality saturation *dissolves* it by exploring all
/// orders simultaneously.
pub fn default_pipeline() -> PassManager {
    PassManager::new()
        .add(ConstFoldPass)
        .add(SimplifyPass)
        .add(CsePass)
        .add(DcePass)
}

/// The full pipeline: the structure-changing passes (`inline`,
/// `loop-unroll`) followed by [`default_pipeline`]'s cleanup.
///
/// Inlining and unrolling *grow* the IR in order to expose optimization
/// opportunities across a call or loop boundary; the cleanup passes then
/// collect the winnings. Running them before the cleanup rather than after
/// is the whole point -- an unrolled loop body whose constants are never
/// folded is strictly worse than the loop it replaced.
///
/// Best driven via [`PassManager::run_to_fixpoint`]: `inline`'s depth-1
/// policy (see [`InlinePass`]) inlines one level of a call chain per
/// iteration, and each newly inlined body can expose constants that let
/// `unroll` fire on the next round.
pub fn full_pipeline() -> PassManager {
    PassManager::new()
        .add(InlinePass)
        .add(UnrollPass)
        .add(ConstFoldPass)
        .add(SimplifyPass)
        .add(CsePass)
        .add(DcePass)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{print_body, verify_body, Attr, OpKind};

    #[test]
    fn default_pipeline_collapses_identities_and_constants() {
        // `((a * 1) + 0) + (2 * 3)`  ==>  `a + 6`
        let mut body = Body::new();
        let a = body.push(OpKind::Arg(0), []);
        let one = body.push(OpKind::Const(Attr::Int(1)), []);
        let scaled = body.push(OpKind::Mul, [a, one]);
        let zero = body.push(OpKind::Const(Attr::Int(0)), []);
        let shifted = body.push(OpKind::Add, [scaled, zero]);
        let two = body.push(OpKind::Const(Attr::Int(2)), []);
        let three = body.push(OpKind::Const(Attr::Int(3)), []);
        let six = body.push(OpKind::Mul, [two, three]);
        body.push(OpKind::Add, [shifted, six]);

        default_pipeline()
            .run_to_fixpoint(&mut body, &PassContext::default(), 8)
            .expect("pipeline must preserve IR validity");
        verify_body(&body).unwrap();

        assert_eq!(
            print_body(&body),
            "\
%0 = core.arg 0
%1 = core.const 6
%2 = core.add %0, %1
"
        );
    }

    #[test]
    fn pipeline_reports_the_pass_that_broke_the_ir() {
        // A deliberately broken pass: emits a `cf.yield` at top level,
        // which the verifier rejects. The error must name it.
        struct BadPass;
        impl Pass for BadPass {
            fn name(&self) -> &'static str {
                "bad-pass"
            }
            fn run(&self, body: &mut Body, _ctx: &PassContext<'_>) -> Changed {
                let v = body.push(OpKind::Const(Attr::Int(0)), []);
                body.push(OpKind::Yield, [v]);
                Changed::Yes
            }
        }

        let mut body = Body::new();
        body.push(OpKind::Const(Attr::Int(1)), []);
        let err = PassManager::new()
            .add(BadPass)
            .run(&mut body, &PassContext::default())
            .expect_err("the verifier must catch the broken rewrite");
        assert_eq!(err.pass, "bad-pass");
    }

    #[test]
    fn empty_pipeline_changes_nothing() {
        let mut body = Body::new();
        body.push(OpKind::Const(Attr::Int(1)), []);
        let before = print_body(&body);
        assert_eq!(
            PassManager::new()
                .run(&mut body, &PassContext::default())
                .unwrap(),
            Changed::No
        );
        assert_eq!(print_body(&body), before);
    }

    /// Every pass rebuilds the body; each must carry every op's type
    /// across. `cse` and `dce` used to rebuild untyped, so on the second
    /// round `const_fold` saw a `u32` constant with no type and a `zext`
    /// with nothing to read the width from -- and folded `1000 as u64` to
    /// `0`. Runs each pass alone, then the whole pipeline.
    #[test]
    fn every_pass_preserves_op_types() {
        use crate::{op::CastMode, CastKind, TypeId};

        let build = || {
            let mut body = Body::new();
            let a = body.push_typed(OpKind::Arg(0), [], TypeId::U32);
            let c = body.push_typed(OpKind::Const(Attr::Int(7)), [], TypeId::U32);
            let sum = body.push_typed(OpKind::Add, [a, c], TypeId::U32);
            body.push_typed(
                OpKind::Cast(CastKind::Zext, CastMode::Wrapping),
                [sum],
                TypeId::U64,
            );
            body
        };
        let types = |body: &Body| body.iter().map(|(_, op)| op.ty).collect::<Vec<_>>();
        let expected = vec![TypeId::U32, TypeId::U32, TypeId::U32, TypeId::U64];

        let passes: Vec<Box<dyn Pass>> = vec![
            Box::new(InlinePass),
            Box::new(UnrollPass),
            Box::new(ConstFoldPass),
            Box::new(SimplifyPass),
            Box::new(CsePass),
            Box::new(DcePass),
        ];
        for pass in passes {
            // Force a rebuild: a no-op pass that keeps the input proves
            // nothing, so prefix an unused constant every pass removes or
            // copies.
            let mut body = Body::new();
            body.push(OpKind::Const(Attr::Int(99)), []);
            let original = build();
            let mut map = rustc_hash::FxHashMap::default();
            for (id, op) in original.iter() {
                let operands: Vec<_> = op.operands.iter().map(|o| map[o]).collect();
                map.insert(id, body.push_typed(op.kind.clone(), operands, op.ty));
            }
            pass.run(&mut body, &PassContext::default());
            let kept: Vec<_> = types(&body)
                .into_iter()
                .filter(|t| !t.is_untyped())
                .collect();
            assert_eq!(
                kept,
                expected,
                "`{}` dropped a type:
{}",
                pass.name(),
                print_body(&body)
            );
        }

        let mut body = build();
        full_pipeline()
            .run_to_fixpoint(&mut body, &PassContext::default(), 8)
            .unwrap();
        assert_eq!(types(&body), expected, "{}", print_body(&body));
    }

    #[test]
    fn widening_cast_of_a_constant_folds_after_any_rebuild() {
        use crate::{op::CastMode, CastKind, TypeId};

        // `let a: u32 = 1000; a as u64` -- with `cse` and `dce` ahead of
        // `const_fold`, exactly the order the fixpoint loop reaches on its
        // second round.
        let mut body = Body::new();
        let a = body.push_typed(OpKind::Const(Attr::Int(1000)), [], TypeId::U32);
        body.push_typed(
            OpKind::Cast(CastKind::Zext, CastMode::Wrapping),
            [a],
            TypeId::U64,
        );
        PassManager::new()
            .add(CsePass)
            .add(DcePass)
            .add(ConstFoldPass)
            .add(DcePass)
            .run_to_fixpoint(&mut body, &PassContext::default(), 8)
            .unwrap();
        assert_eq!(
            print_body(&body),
            "%0 = core.const 1000
"
        );
        let (_, folded) = body.iter().next().unwrap();
        assert_eq!(folded.ty, TypeId::U64);
    }
}
