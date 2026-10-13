//! Copyright (c) 2026 Omnira CJSC. All Rights Reserved.
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - The extraction cost table, keyed by the compilation target.
//!
//! Extraction picks the cheapest program in each e-class, and "cheapest"
//! is only meaningful for a machine. A 64-bit integer divide is a handful
//! of cycles on a recent x86-64 core and several dozen on an older one; on
//! a GPU it is a software sequence. This table is where that knowledge
//! lives, in one place, chosen once from the target the build is for.
//!
//! # What the numbers are
//!
//! Approximate result latencies in cycles for 64-bit scalar integer
//! operations, rounded, taken from the published instruction tables (Agner
//! Fog's for x86-64; Arm's software optimization guides for the Neoverse /
//! Cortex-A7x cores; NVIDIA's PTX documentation for which integer
//! operations are emulated). They are relative weights for choosing
//! between equivalent programs, not a timing model: nothing here was
//! measured on this machine, and no output depends on them being exact,
//! only on their order (a divide costs more than a multiply, which costs
//! more than an add).
//!
//! # Determinism
//!
//! The table is a pure function of the target triple's architecture and
//! the CPU name the build was configured with -- both salsa inputs. It
//! never consults the machine the compiler happens to run on. A
//! `--target-cpu=native` build is resolved to a concrete CPU name before
//! it gets here, so the same inputs always select the same table.

/// Per-operation extraction costs. See the module doc for what the
/// numbers mean.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct CostTable {
    /// Which row of the table this is, for reports and tests.
    pub name: &'static str,
    /// A literal. Free by default: it folds into an instruction's
    /// immediate field, and preferring it is what makes constant folding
    /// win during extraction.
    pub literal: u64,
    /// A reference to an argument, block argument or generator parameter.
    pub leaf: u64,
    /// Add, subtract, negate, compare, logic and bitwise operations, tuple
    /// packing and projection.
    pub alu: u64,
    /// Shifts. Separate from `alu` because some cores pay extra for them
    /// (x86-64 shifts by a register amount are two micro-ops).
    pub shift: u64,
    /// Integer multiply.
    pub mul: u64,
    /// Integer divide and remainder.
    pub div: u64,
    /// A load or a store (`mem.load`, `mem.store`). Address arithmetic and
    /// allocation bookkeeping cost `alu`.
    pub memory: u64,
    /// An opaque op (control flow, a call): never chosen against an
    /// alternative, but it must cost something so extraction does not
    /// prefer duplicating work into it.
    pub opaque: u64,
}

impl CostTable {
    /// The table every build used before costs were keyed by target, and
    /// the one used when the target is not recognized: add 1, multiply 3,
    /// divide 8.
    pub const GENERIC: CostTable = CostTable {
        name: "generic",
        literal: 0,
        leaf: 1,
        alu: 1,
        shift: 1,
        mul: 3,
        div: 8,
        memory: 4,
        opaque: 1,
    };

    /// x86-64 cores whose 64-bit `IDIV` is microcoded and slow (everything
    /// before Ice Lake and Zen 3, and the `x86-64` baseline CPU, which
    /// has to assume them).
    pub const X86_64: CostTable = CostTable {
        name: "x86-64",
        literal: 0,
        leaf: 1,
        alu: 1,
        shift: 1,
        mul: 3,
        div: 40,
        memory: 4,
        opaque: 1,
    };

    /// x86-64 cores with the fast radix-16 divider: Ice Lake and later
    /// Intel cores, Zen 3 and later AMD cores.
    pub const X86_64_FAST_DIV: CostTable = CostTable {
        name: "x86-64-fast-div",
        div: 14,
        ..CostTable::X86_64
    };

    /// `AArch64` application cores (Cortex-A7x/X, Neoverse, Apple).
    pub const AARCH64: CostTable = CostTable {
        name: "aarch64",
        literal: 0,
        leaf: 1,
        alu: 1,
        shift: 1,
        mul: 3,
        div: 12,
        memory: 4,
        opaque: 1,
    };

    /// WebAssembly. The module is compiled again by the engine for the
    /// machine it lands on, which nothing at build time knows; the
    /// generic weights are the honest choice.
    pub const WASM: CostTable = CostTable {
        name: "wasm",
        ..CostTable::GENERIC
    };

    /// NVIDIA PTX. 64-bit integer multiply is a short sequence of 32-bit
    /// multiplies and 64-bit divide is a software routine on every
    /// current architecture.
    pub const NVPTX: CostTable = CostTable {
        name: "nvptx",
        literal: 0,
        leaf: 1,
        alu: 1,
        shift: 1,
        mul: 4,
        div: 64,
        memory: 32,
        opaque: 1,
    };

    /// Selects the table for a target, from its architecture (the
    /// `arch` of `codira_target::spec::Target`, e.g. `"x86_64"`) and the
    /// CPU the build targets (an LLVM CPU name such as `"znver4"`,
    /// already resolved -- never `"native"`).
    ///
    /// Unknown architectures get [`CostTable::GENERIC`].
    pub fn for_target(arch: &str, cpu: &str) -> CostTable {
        match arch {
            "x86_64" | "x86-64" => {
                if x86_has_fast_divider(cpu) {
                    CostTable::X86_64_FAST_DIV
                } else {
                    CostTable::X86_64
                }
            }
            "aarch64" | "arm64" => CostTable::AARCH64,
            "wasm32" | "wasm64" => CostTable::WASM,
            "nvptx" | "nvptx64" => CostTable::NVPTX,
            _ => CostTable::GENERIC,
        }
    }
}

impl Default for CostTable {
    fn default() -> Self {
        CostTable::GENERIC
    }
}

/// Whether an x86-64 CPU (by LLVM name) has the fast integer divider.
/// Names are matched exactly or by family prefix; anything unrecognized
/// is assumed slow, which only ever makes division look more expensive
/// than it is.
fn x86_has_fast_divider(cpu: &str) -> bool {
    const FAST: &[&str] = &[
        "znver3",
        "znver4",
        "znver5",
        "icelake-client",
        "icelake-server",
        "tigerlake",
        "rocketlake",
        "alderlake",
        "raptorlake",
        "meteorlake",
        "arrowlake",
        "arrowlake-s",
        "lunarlake",
        "pantherlake",
        "sapphirerapids",
        "emeraldrapids",
        "graniterapids",
        "graniterapids-d",
        "diamondrapids",
        "x86-64-v4",
    ];
    FAST.contains(&cpu)
}

#[cfg(test)]
mod tests {
    use super::CostTable;

    #[test]
    fn tables_are_selected_by_architecture_and_cpu() {
        assert_eq!(CostTable::for_target("x86_64", "x86-64").name, "x86-64");
        assert_eq!(
            CostTable::for_target("x86_64", "znver4").name,
            "x86-64-fast-div"
        );
        assert_eq!(CostTable::for_target("aarch64", "apple-m1").name, "aarch64");
        assert_eq!(CostTable::for_target("wasm32", "generic").name, "wasm");
        assert_eq!(CostTable::for_target("nvptx64", "sm_80").name, "nvptx");
        assert_eq!(CostTable::for_target("riscv64", "generic").name, "generic");
    }

    #[test]
    fn every_table_orders_divide_above_multiply_above_add() {
        for table in [
            CostTable::GENERIC,
            CostTable::X86_64,
            CostTable::X86_64_FAST_DIV,
            CostTable::AARCH64,
            CostTable::WASM,
            CostTable::NVPTX,
        ] {
            assert!(table.alu < table.mul, "{}", table.name);
            assert!(table.mul < table.div, "{}", table.name);
            assert_eq!(table.literal, 0, "{}", table.name);
        }
    }
}
