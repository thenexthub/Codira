//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Experimental hardware-assisted memory safety for the raw-memory escape
//!   hatches (`stack_alloc`, region arenas, OS pages): which mode a build uses,
//!   the checks that refuse a target that cannot honour it, and the
//!   `set_hardware_safety` hook a library built with a mode exports.
//!
//! # `--sanitize=memtag` (ARM MTE, experimental)
//!
//! For `aarch64` Linux targets whose features include `+mte`. Generated code
//! tags every `stack_alloc` slot: `llvm.aarch64.irg` picks a random tag for
//! the slot's address (never tag 0, so an untagged pointer to the slot
//! faults), `llvm.aarch64.settag` writes it to every 16-byte granule of the
//! slot (`STG`/`ST2G`), and the tagged address is what `stack_alloc`
//! returns. Before every return the granules are set back to tag 0, so a
//! pointer that outlives the frame faults on its next use. The runtime does
//! the same for region arenas and OS pages (`codira_runtime::memtag`).
//!
//! Nothing on the machine that wrote this has MTE: the instructions are
//! checked to be in the object (`test_memtag`), never executed here.

use codira_target::spec::Target;

/// Which experimental memory-safety mode generated code is built with. The
/// default is none.
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq, Hash)]
pub struct HardwareSafety {
    /// `--sanitize=memtag`: ARM MTE tags on `stack_alloc` slots (and, in the
    /// runtime, on region and page memory).
    pub memtag: bool,
}

impl HardwareSafety {
    /// No mode at all: the escape hatches are unchecked, as they always were.
    pub const NONE: HardwareSafety = HardwareSafety { memtag: false };

    /// `--sanitize=memtag`.
    pub const MEMTAG: HardwareSafety = HardwareSafety { memtag: true };

    /// Whether any mode is on.
    pub fn is_enabled(self) -> bool {
        self.memtag
    }

    /// The flags the library's `set_hardware_safety` returns.
    pub fn abi_flags(self) -> u32 {
        let mut flags = 0;
        if self.memtag {
            flags |= codira_abi::HARDWARE_SAFETY_MEMTAG;
        }
        flags
    }
}

/// Whether `target`'s feature string turns `feature` on: the last `+name`
/// or `-name` for it wins, as it does for LLVM.
pub fn target_has_feature(target: &Target, feature: &str) -> bool {
    let mut enabled = false;
    for entry in target.options.features.split(',').map(str::trim) {
        if let Some(name) = entry.strip_prefix('+') {
            if name == feature {
                enabled = true;
            }
        } else if let Some(name) = entry.strip_prefix('-') {
            if name == feature {
                enabled = false;
            }
        }
    }
    enabled
}

/// Checks that `target` can honour `safety`, or says why not.
pub fn validate(safety: HardwareSafety, target: &Target) -> Result<(), String> {
    if !safety.memtag {
        return Ok(());
    }
    if target.arch != "aarch64" {
        return Err(format!(
            "--sanitize=memtag (experimental) needs an aarch64 target with the `+mte` feature, \
             but {} is {}: memory tagging is an ARMv8.5 extension that no other architecture has",
            target.llvm_target, target.arch
        ));
    }
    if !target_has_feature(target, "mte") {
        return Err(format!(
            "--sanitize=memtag (experimental) needs the `+mte` target feature, and {} (cpu `{}`) \
             does not enable it; add `--target-feature +mte`, or pick a CPU that has MTE with \
             `--target-cpu` (for example `cortex-a710` or `neoverse-v2`)",
            target.llvm_target, target.options.cpu
        ));
    }
    if target.options.os != "linux" {
        return Err(format!(
            "--sanitize=memtag (experimental) needs Linux: tagged memory is mapped with \
             `PROT_MTE` and checked after `prctl(PR_SET_TAGGED_ADDR_CTRL)`, and {} (os `{}`) has \
             no such interface",
            target.llvm_target, target.options.os
        ));
    }
    Ok(())
}

/// Why a CHERI or Morello target cannot be built, if `triple` names one.
///
/// A Morello target would give hardware-enforced capabilities, but it cannot
/// be faked: every pointer becomes a 129-bit capability (64-bit address,
/// compressed bounds and permissions, and a tag bit kept outside
/// addressable memory), which needs a compiler that understands
/// capabilities from the IR down (CHERI LLVM), the purecap ABI and a
/// runtime, C library and loader built for it. None of that exists here.
pub fn refuse_capability_target(triple: &str) -> Option<String> {
    let lower = triple.to_ascii_lowercase();
    if !["morello", "cheri", "purecap"]
        .iter()
        .any(|name| lower.contains(name))
    {
        return None;
    }
    Some(format!(
        "`{triple}` is a CHERI capability target, and this compiler cannot build for it. On \
         Morello every pointer is a 129-bit capability -- a 64-bit address plus bounds \
         compressed relative to it and permissions in 64 more bits, and a validity tag the \
         hardware keeps outside addressable memory -- so pointer size, alignment, the stack and \
         every load and store change. That needs a capability-aware LLVM (CHERI LLVM, which \
         this build does not include), the purecap ABI (`-mabi=purecap`), and a Codira \
         runtime, C library and loader built for it, none of which exist here"
    ))
}

/// Generates `uint32_t set_hardware_safety(void *table)` in `module` for a
/// library built with a mode: it returns the mode's flags, so the runtime
/// can refuse a library it cannot honour. Libraries built without a mode do
/// not have the function.
pub(crate) fn gen_set_hardware_safety_fn<'ink>(
    context: &'ink inkwell::context::Context,
    module: &inkwell::module::Module<'ink>,
    safety: HardwareSafety,
) {
    if !safety.is_enabled() {
        return;
    }
    let ptr_type = context.ptr_type(inkwell::AddressSpace::default());
    let fn_type = context.i32_type().fn_type(&[ptr_type.into()], false);
    let function = module.add_function(
        codira_abi::SET_HARDWARE_SAFETY_FN_NAME,
        fn_type,
        Some(inkwell::module::Linkage::DLLExport),
    );
    let builder = context.create_builder();
    builder.position_at_end(context.append_basic_block(function, "body"));
    builder
        .build_return(Some(
            &context
                .i32_type()
                .const_int(u64::from(safety.abi_flags()), false),
        ))
        .expect("failed to build return");
}

#[cfg(test)]
mod tests {
    use codira_target::spec::Target;

    use super::{refuse_capability_target, target_has_feature, validate, HardwareSafety};

    fn with_features(triple: &str, features: &str) -> Target {
        let mut target = Target::search(triple).expect("built-in target");
        target.options.features = features.to_owned();
        target
    }

    #[test]
    fn the_last_mention_of_a_feature_wins() {
        assert!(target_has_feature(
            &with_features("aarch64-unknown-linux-gnu", "+v8a,+mte"),
            "mte"
        ));
        assert!(!target_has_feature(
            &with_features("aarch64-unknown-linux-gnu", "+mte,-mte"),
            "mte"
        ));
        assert!(!target_has_feature(
            &with_features("aarch64-unknown-linux-gnu", "+v8a,+mtex"),
            "mte"
        ));
    }

    #[test]
    fn memtag_needs_aarch64_linux_with_mte() {
        let ok = with_features("aarch64-unknown-linux-gnu", "+v8a,+mte");
        assert_eq!(validate(HardwareSafety::MEMTAG, &ok), Ok(()));

        let x86 = Target::search("x86_64-unknown-linux-gnu").unwrap();
        let error = validate(HardwareSafety::MEMTAG, &x86).unwrap_err();
        assert!(error.contains("needs an aarch64 target"), "{error}");

        let no_mte = Target::search("aarch64-unknown-linux-gnu").unwrap();
        let error = validate(HardwareSafety::MEMTAG, &no_mte).unwrap_err();
        assert!(error.contains("`+mte`"), "{error}");
        assert!(error.contains("--target-feature +mte"), "{error}");

        let darwin = with_features("aarch64-apple-darwin", "+mte");
        let error = validate(HardwareSafety::MEMTAG, &darwin).unwrap_err();
        assert!(error.contains("needs Linux"), "{error}");

        // No mode, no requirement.
        assert_eq!(validate(HardwareSafety::NONE, &x86), Ok(()));
    }

    #[test]
    fn morello_and_cheri_targets_are_refused_with_the_reason() {
        for triple in [
            "aarch64-morello",
            "aarch64-unknown-freebsd-purecap",
            "riscv64-cheri",
        ] {
            let reason = refuse_capability_target(triple).expect(triple);
            assert!(reason.contains("CHERI LLVM"), "{reason}");
            assert!(reason.contains("purecap ABI"), "{reason}");
            assert!(reason.contains("129-bit"), "{reason}");
        }
        assert_eq!(refuse_capability_target("aarch64-unknown-linux-gnu"), None);
    }
}
