//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - What Eidos is allowed to do in a build: the build mode (`--mode`) and the
//!   e-graph bounds from a package's `[eidos]` manifest table. Both are one
//!   salsa input, so changing either recompiles exactly what depends on it and
//!   nothing else.

use codira_egraph::{CostTable, EGraphOptimizer};
use codira_target::spec::Target;

/// How much of the Eidos mid-level stack a build runs.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
pub enum BuildMode {
    /// Eidos is skipped: every function is lowered straight from HIR to
    /// LLVM IR. This is the edit-build loop's mode -- the MIR pipeline,
    /// equality saturation and the comptime interpreter cost compile time
    /// on every function and only pay off in the code that is shipped.
    Dev,
    /// Eidos runs on every function it applies to. The default, because it
    /// is what every build did before the mode existed.
    #[default]
    Release,
}

impl BuildMode {
    /// The spelling `--mode` accepts.
    pub fn name(self) -> &'static str {
        match self {
            BuildMode::Dev => "dev",
            BuildMode::Release => "release",
        }
    }

    pub fn from_name(name: &str) -> Option<BuildMode> {
        match name {
            "dev" => Some(BuildMode::Dev),
            "release" => Some(BuildMode::Release),
            _ => None,
        }
    }
}

/// Everything that decides what Eidos does to a function, as one salsa
/// input (`CodeGenDatabase::eidos_options`).
///
/// Every bound is a count of work -- nodes, rounds, search steps -- and
/// never a duration: the same sources and options give the same program on
/// every machine (`KGEN_SUPERSET_STATUS` §32.5). That is why there is no
/// timeout here, and why the manifest parser refuses `egraph_timeout_ms`.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct EidosOptions {
    pub mode: BuildMode,
    /// Hard e-node cap per region (`[eidos] max_egraph_nodes_per_region`).
    pub max_egraph_nodes_per_region: usize,
    /// Rule-application rounds per region (`[eidos]
    /// max_egraph_iterations`).
    pub max_egraph_iterations: usize,
    /// Exact-extraction search steps per region (`[eidos]
    /// exact_extraction_budget`); `0` extracts greedily.
    pub exact_extraction_budget: u64,
}

impl Default for EidosOptions {
    fn default() -> Self {
        EidosOptions {
            mode: BuildMode::default(),
            max_egraph_nodes_per_region: EGraphOptimizer::DEFAULT_MAX_NODES,
            max_egraph_iterations: EGraphOptimizer::DEFAULT_MAX_ITERATIONS,
            exact_extraction_budget: EGraphOptimizer::DEFAULT_EXACT_EXTRACTION_BUDGET,
        }
    }
}

impl EidosOptions {
    /// The same options in another build mode.
    pub fn with_mode(self, mode: BuildMode) -> Self {
        EidosOptions { mode, ..self }
    }

    /// The e-graph optimizer this build uses on `target`, or `None` when
    /// Eidos does not run at all (`--mode=dev`).
    ///
    /// The cost table is chosen here, from the target's architecture and
    /// the CPU the build was configured for -- both already salsa inputs --
    /// so a target or CPU change re-extracts and nothing else does.
    pub fn optimizer(&self, target: &Target) -> Option<EGraphOptimizer> {
        match self.mode {
            BuildMode::Dev => None,
            BuildMode::Release => Some(EGraphOptimizer {
                max_iterations: self.max_egraph_iterations,
                max_nodes: self.max_egraph_nodes_per_region,
                exact_extraction_budget: self.exact_extraction_budget,
                costs: CostTable::for_target(&target.arch, &target.options.cpu),
            }),
        }
    }
}

#[cfg(test)]
mod tests {
    use codira_target::spec::Target;

    use super::{BuildMode, EidosOptions};

    #[test]
    fn dev_mode_has_no_optimizer() {
        let target = Target::host_target().unwrap();
        let options = EidosOptions::default().with_mode(BuildMode::Dev);
        assert!(options.optimizer(&target).is_none());
        let release = EidosOptions::default().optimizer(&target).unwrap();
        assert_eq!(release.max_nodes, 10_000);
        assert_eq!(release.max_iterations, 8);
    }

    #[test]
    fn the_cost_table_follows_the_target_cpu() {
        let mut target = Target::search("x86_64-unknown-linux-gnu").unwrap();
        let options = EidosOptions::default();
        assert_eq!(options.optimizer(&target).unwrap().costs.name, "x86-64");
        target.options.cpu = "znver4".into();
        assert_eq!(
            options.optimizer(&target).unwrap().costs.name,
            "x86-64-fast-div"
        );
    }

    #[test]
    fn mode_names_round_trip() {
        for mode in [BuildMode::Dev, BuildMode::Release] {
            assert_eq!(BuildMode::from_name(mode.name()), Some(mode));
        }
        assert_eq!(BuildMode::from_name("debug"), None);
    }
}
