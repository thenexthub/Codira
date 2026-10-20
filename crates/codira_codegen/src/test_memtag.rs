//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Tests of `--sanitize=memtag` (experimental) code generation: an aarch64
//!   Linux object built with `+mte` contains the MTE instructions that tag and
//!   untag `stack_alloc` slots (found by decoding the object's instruction
//!   stream, and with `llvm-objdump` when the LLVM installation has one), an
//!   object built without the mode has none, and targets or code the mode
//!   cannot protect are refused with the reason.

use std::{path::Path, process::Command, time::Duration};

use codira_hir::HirDatabase;
use codira_target::spec::Target;
use inkwell::{context::Context, targets::TargetTriple, OptimizationLevel};
use object::{Object, ObjectSection, ObjectSymbol, SectionKind};

use crate::{
    code_gen::{AssemblyBuilder, CodeGenContext},
    db::initialize_target_backends_once,
    mock::MockDatabase,
    CodeGenDatabase, HardwareSafety,
};

const SOURCE: &str = r#"
extern "codira-intrinsic" {
    func stack_alloc(size: usize, align: usize) -> usize;
    func store_u64(addr: usize, value: u64);
    func load_u64(addr: usize) -> u64;
}

public func scratch(x: u64) -> u64 {
    let slot = stack_alloc(40, 8);
    store_u64(slot, x);
    store_u64(slot + 32, x + 1);
    load_u64(slot) + load_u64(slot + 32)
}
"#;

/// `aarch64-unknown-linux-gnu` with MTE, or `None` when this build of the
/// compiler leaves the `AArch64` backend out.
fn aarch64_with_mte() -> Option<Target> {
    initialize_target_backends_once();
    let mut target = Target::search("aarch64-unknown-linux-gnu").expect("built-in target");
    inkwell::targets::Target::from_triple(&TargetTriple::create(&target.llvm_target)).ok()?;
    target.options.features = "+v8a,+mte".to_owned();
    Some(target)
}

/// The MTE instructions in one `AArch64` instruction stream, by mnemonic.
#[derive(Debug, Default, PartialEq, Eq)]
struct MteInstructions {
    irg: usize,
    /// `STG`, `ST2G`, `STZG`, `STZ2G` and `STGP`: everything that writes tags.
    tag_stores: usize,
    addg_subg: usize,
}

impl MteInstructions {
    fn total(&self) -> usize {
        self.irg + self.tag_stores + self.addg_subg
    }
}

/// Decodes the MTE instructions in `code` (little-endian A64), following the
/// encodings in the Arm Architecture Reference Manual (C6.2).
fn count_mte_instructions(code: &[u8]) -> MteInstructions {
    let mut found = MteInstructions::default();
    for word in code.chunks_exact(4) {
        let w = u32::from_le_bytes([word[0], word[1], word[2], word[3]]);
        // IRG Xd, Xn, Xm: 1001 1010 110 Xm 000100 Xn Xd
        if w & 0xFFE0_FC00 == 0x9AC0_1000 {
            found.irg += 1;
        }
        // STG/STZG/ST2G/STZ2G: 1101 1001 opc 1 imm9 op2 Xn Xt, op2 != 00
        // (op2 == 00 is STZGM/LDG/STGM/LDGM).
        let tag_store_group = w & 0xFF20_0000;
        if matches!(tag_store_group, 0xD920_0000) && w & 0x0000_0C00 != 0 {
            found.tag_stores += 1;
        }
        // STGP (post-index, signed offset, pre-index).
        if matches!(w & 0xFFC0_0000, 0x6880_0000 | 0x6900_0000 | 0x6980_0000) {
            found.tag_stores += 1;
        }
        // ADDG/SUBG: x 0 0 100011 0 uimm6 00 uimm4 Xn Xd
        if matches!(w & 0xFFC0_C000, 0x9180_0000 | 0xD180_0000) {
            found.addg_subg += 1;
        }
    }
    found
}

/// Builds `source` for `target` under `safety` and returns the object bytes
/// and what the mode refused.
fn build_object(
    source: &str,
    target: &Target,
    safety: HardwareSafety,
    opt: OptimizationLevel,
) -> (Vec<u8>, Vec<String>) {
    let (mut db, _file_id) = MockDatabase::with_single_file(source);
    db.set_optimization_level(opt);
    db.set_target(target.clone());
    db.set_hardware_safety(safety);

    let context = Context::create();
    let code_gen = CodeGenContext::new(&context, &db);
    let partition = db.module_partition();
    let (group_id, _) = partition.iter().next().expect("one module group");
    let object_file = AssemblyBuilder::new(&code_gen, &partition, group_id)
        .build()
        .expect("assembly")
        .into_object_file()
        .expect("object file");
    let bytes = std::fs::read(object_file.path()).expect("object file was written");
    let refusals = code_gen.safety_refusals.borrow().clone();
    (bytes, refusals)
}

fn text_of(bytes: &[u8]) -> Vec<u8> {
    let object = object::File::parse(bytes).expect("an ELF object");
    object
        .sections()
        .filter(|section| section.kind() == SectionKind::Text)
        .flat_map(|section| section.data().expect("section data").to_vec())
        .collect()
}

/// Runs `program` with `args`, killing it after 60 seconds.
fn run_with_timeout(program: &Path, args: &[&std::ffi::OsStr]) -> Option<std::process::Output> {
    let mut child = Command::new(program)
        .args(args)
        .stdout(std::process::Stdio::piped())
        .stderr(std::process::Stdio::piped())
        .spawn()
        .ok()?;
    let deadline = std::time::Instant::now() + Duration::from_secs(60);
    loop {
        if child.try_wait().ok()?.is_some() {
            return child.wait_with_output().ok();
        }
        if std::time::Instant::now() > deadline {
            let _ignored = child.kill();
            panic!("`{}` did not finish within 60 s", program.display());
        }
        std::thread::sleep(Duration::from_millis(20));
    }
}

#[test]
fn memtag_tags_and_untags_stack_slots_with_mte_instructions() {
    let Some(target) = aarch64_with_mte() else {
        eprintln!("skipped: the AArch64 LLVM backend is not compiled in");
        return;
    };
    for opt in [OptimizationLevel::None, OptimizationLevel::Default] {
        let (tagged, refusals) = build_object(SOURCE, &target, HardwareSafety::MEMTAG, opt);
        assert!(refusals.is_empty(), "{refusals:?}");
        let found = count_mte_instructions(&text_of(&tagged));
        // One IRG for the slot; the 48-byte slot is tagged on entry and
        // untagged before the return, each with at least one tag store.
        assert!(found.irg >= 1, "{opt:?}: {found:?}");
        assert!(found.tag_stores >= 2, "{opt:?}: {found:?}");

        let object = object::File::parse(&*tagged).unwrap();
        assert!(
            object
                .symbols()
                .any(|symbol| symbol.name() == Ok(codira_abi::SET_HARDWARE_SAFETY_FN_NAME)),
            "a memtag object exports `set_hardware_safety`"
        );

        // The same code without the mode has no MTE instruction at all.
        let (plain, _) = build_object(SOURCE, &target, HardwareSafety::NONE, opt);
        assert_eq!(
            count_mte_instructions(&text_of(&plain)).total(),
            0,
            "{opt:?}"
        );
    }
}

/// The same object, disassembled by the LLVM installation's own
/// `llvm-objdump` when it has one.
#[test]
fn llvm_objdump_shows_irg_and_tag_stores() {
    let Some(target) = aarch64_with_mte() else {
        eprintln!("skipped: the AArch64 LLVM backend is not compiled in");
        return;
    };
    let objdump = option_env!("LLVM_SYS_221_PREFIX")
        .map(|prefix| {
            Path::new(prefix)
                .join("bin")
                .join(format!("llvm-objdump{}", std::env::consts::EXE_SUFFIX))
        })
        .filter(|path| path.is_file());
    let Some(objdump) = objdump else {
        eprintln!("skipped: no llvm-objdump next to the LLVM this compiler was built with");
        return;
    };
    let (bytes, _) = build_object(
        SOURCE,
        &target,
        HardwareSafety::MEMTAG,
        OptimizationLevel::Default,
    );
    let file = tempfile::NamedTempFile::new().unwrap();
    std::fs::write(file.path(), bytes).unwrap();
    let output = run_with_timeout(
        &objdump,
        &[
            "-d".as_ref(),
            "--mattr=+mte".as_ref(),
            file.path().as_os_str(),
        ],
    )
    .expect("llvm-objdump runs");
    let listing = String::from_utf8_lossy(&output.stdout);
    assert!(listing.contains("irg\t"), "{listing}");
    assert!(
        listing.contains("st2g\t") || listing.contains("stg\t") || listing.contains("stzg\t"),
        "{listing}"
    );
}

/// `stack_alloc` reached from a closure body and from a generic instance is
/// tagged and untagged like one in a plain function: each has its own `IRG`,
/// and tag stores both on entry and before its return.
#[test]
fn memtag_covers_closures_and_generic_instances() {
    let Some(target) = aarch64_with_mte() else {
        eprintln!("skipped: the AArch64 LLVM backend is not compiled in");
        return;
    };
    let source = r#"
extern "codira-intrinsic" {
    func stack_alloc(size: usize, align: usize) -> usize;
    func store_u64(addr: usize, value: u64);
    func load_u64(addr: usize) -> u64;
}

func through_slot[T](x: T) -> T {
    let slot = stack_alloc(16, 8);
    store_u64(slot, 1);
    x
}

public func run(n: u64) -> u64 {
    let f = func(v: u64) -> u64 {
        let slot = stack_alloc(32, 8);
        store_u64(slot, v);
        load_u64(slot)
    };
    let flag = if through_slot(true) { 1 } else { 0 };
    through_slot(f(n)) + flag
}
"#;
    let (bytes, refusals) = build_object(
        source,
        &target,
        HardwareSafety::MEMTAG,
        OptimizationLevel::None,
    );
    assert!(refusals.is_empty(), "{refusals:?}");
    let found = count_mte_instructions(&text_of(&bytes));
    // The closure body and two instances of `through_slot` (`u64` and
    // `bool`): three slots, so three tags picked and at
    // least six tag stores (on entry and before each return).
    assert!(found.irg >= 3, "{found:?}");
    assert!(found.tag_stores >= 6, "{found:?}");
}

#[test]
fn memtag_refuses_a_stack_slot_sized_at_run_time() {
    let Some(target) = aarch64_with_mte() else {
        eprintln!("skipped: the AArch64 LLVM backend is not compiled in");
        return;
    };
    let source = r#"
extern "codira-intrinsic" {
    func stack_alloc(size: usize, align: usize) -> usize;
}

public func dynamic(n: usize) -> usize {
    stack_alloc(n, 16)
}
"#;
    let (_, refusals) = build_object(
        source,
        &target,
        HardwareSafety::MEMTAG,
        OptimizationLevel::None,
    );
    assert_eq!(refusals.len(), 1, "{refusals:?}");
    assert!(
        refusals[0].starts_with("mod.code:7:5: `stack_alloc` with a size computed at run time"),
        "{refusals:?}"
    );

    // The whole build fails with that reason.
    let (mut db, _file_id) = MockDatabase::with_single_file(source);
    db.set_target(target);
    db.set_hardware_safety(HardwareSafety::MEMTAG);
    let partition = db.module_partition();
    let (group_id, _) = partition.iter().next().unwrap();
    let assembly = db.target_assembly(group_id);
    let error = assembly.error().expect("the build is refused");
    assert!(
        error.contains("cannot be tagged by --sanitize=memtag"),
        "{error}"
    );
}

#[test]
fn memtag_refuses_targets_without_mte() {
    for (triple, features, expected) in [
        ("x86_64-unknown-linux-gnu", None, "needs an aarch64 target"),
        (
            "aarch64-unknown-linux-gnu",
            None,
            "needs the `+mte` target feature",
        ),
        (
            "aarch64-unknown-linux-gnu",
            Some("+v8a,+mte,-mte"),
            "needs the `+mte` target feature",
        ),
    ] {
        let mut target = Target::search(triple).unwrap();
        if let Some(features) = features {
            target.options.features = features.to_owned();
        }
        let (mut db, _file_id) = MockDatabase::with_single_file(SOURCE);
        db.set_target(target);
        db.set_hardware_safety(HardwareSafety::MEMTAG);
        let partition = db.module_partition();
        let (group_id, _) = partition.iter().next().unwrap();
        let assembly = db.target_assembly(group_id);
        let error = assembly.error().expect("the build is refused");
        assert!(error.contains(expected), "{triple}: {error}");
        assert!(error.contains("experimental"), "{triple}: {error}");
    }
}
