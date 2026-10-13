//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Checks every built-in target against the LLVM backend that compiles for
//!   it: the data layout the front end computes sizes with must be the one LLVM
//!   lays memory out with, and code built for the target must come out as an
//!   object of that target's format and architecture.

use codira_hir::HirDatabase;
use codira_target::spec::{self, Target};
use inkwell::{
    context::Context,
    targets::{CodeModel, RelocMode, TargetTriple},
    OptimizationLevel,
};
use object::{Architecture, BinaryFormat, FileFlags, Object, ObjectKind};

use crate::{
    code_gen::{AssemblyBuilder, CodeGenContext},
    db::initialize_target_backends_once,
    linker::in_process_lld_available,
    mock::MockDatabase,
    CodeGenDatabase,
};

/// `EF_RISCV_FLOAT_ABI`: the two `e_flags` bits naming the float ABI.
const EF_RISCV_FLOAT_ABI: u32 = 0x6;
/// `EF_RISCV_FLOAT_ABI_DOUBLE`: floats travel in `f` registers, as `lp64d`
/// has it.
const EF_RISCV_FLOAT_ABI_DOUBLE: u32 = 0x4;

/// The LLVM backend for `target`, or `None` when this build of the compiler
/// leaves that backend out (see the `target-*` features).
fn llvm_backend(target: &Target) -> Option<inkwell::targets::Target> {
    initialize_target_backends_once();
    inkwell::targets::Target::from_triple(&TargetTriple::create(&target.llvm_target)).ok()
}

fn builtin_targets() -> impl Iterator<Item = (&'static str, Target)> {
    spec::get_targets().map(|triple| {
        let target = Target::search(triple).unwrap_or_else(|| panic!("`{triple}` is listed"));
        (triple, target)
    })
}

fn expected_architecture(target: &Target) -> Architecture {
    match &*target.arch {
        "x86_64" => Architecture::X86_64,
        "aarch64" => Architecture::Aarch64,
        "riscv64" => Architecture::Riscv64,
        arch => panic!("no expected `object` architecture for `{arch}`"),
    }
}

/// Whether `target` is one of Apple's, all of which use Mach-O. Only the
/// macOS specs set `is_like_osx`, so it cannot answer this for iOS.
fn is_apple(target: &Target) -> bool {
    target.options.vendor == "apple"
}

/// The format of an object file -- before linking -- for `target`.
fn expected_object_format(target: &Target) -> BinaryFormat {
    if is_apple(target) {
        BinaryFormat::MachO
    } else if target.options.is_like_windows {
        BinaryFormat::Coff
    } else {
        BinaryFormat::Elf
    }
}

/// A front end that computes a struct's size with one layout while LLVM lays
/// it out with another disagrees with itself about where fields are. The
/// x86-64 specs said exactly that about `i128` until LLVM 18 added
/// `i128:128` to them and nobody copied it over.
#[test]
fn every_target_declares_the_data_layout_llvm_uses() {
    let mut checked = 0;
    for (triple, target) in builtin_targets() {
        let Some(backend) = llvm_backend(&target) else {
            eprintln!("skipped `{triple}`: its LLVM backend is not compiled in");
            continue;
        };
        let machine = backend
            .create_target_machine(
                &TargetTriple::create(&target.llvm_target),
                &target.options.cpu,
                &target.options.features,
                OptimizationLevel::None,
                RelocMode::PIC,
                CodeModel::Default,
            )
            .unwrap_or_else(|| panic!("LLVM has no target machine for `{triple}`"));
        let layout = machine.get_target_data().get_data_layout();
        let layout = layout.as_str().to_str().expect("a data layout is ASCII");
        assert_eq!(&*target.data_layout, layout, "data layout of `{triple}`");
        checked += 1;
    }
    assert!(checked > 0, "no target backend is compiled in");
}

const SOURCE: &str = r"
public func add(a: i64, b: i64) -> i64 { a + b }
public func scale(x: f64, k: f64) -> f64 { x * k }
";

/// Builds `SOURCE` for every target whose backend is compiled in, and checks
/// the object's format and architecture. Where this host can link for the
/// target, the linked library is checked too. In-process LLD links ELF and
/// COFF from a Windows host; Mach-O needs an SDK to link against, so Apple
/// targets stop at the object.
#[test]
fn every_target_builds_an_object_of_its_own_kind() {
    for (triple, target) in builtin_targets() {
        if llvm_backend(&target).is_none() {
            eprintln!("skipped `{triple}`: its LLVM backend is not compiled in");
            continue;
        }

        let (mut db, _file_id) = MockDatabase::with_single_file(SOURCE);
        db.set_optimization_level(OptimizationLevel::None);
        db.set_target(target.clone());

        let context = Context::create();
        let code_gen = CodeGenContext::new(&context, &db);
        let partition = db.module_partition();
        for (group_id, _) in partition.iter() {
            let object_file = AssemblyBuilder::new(&code_gen, &partition, group_id)
                .build()
                .unwrap_or_else(|e| panic!("`{triple}`: {e}"))
                .into_object_file()
                .unwrap_or_else(|e| panic!("`{triple}`: {e}"));

            let bytes = std::fs::read(object_file.path()).expect("object file was written");
            let object = object::File::parse(&*bytes).unwrap_or_else(|e| panic!("`{triple}`: {e}"));
            assert_eq!(
                object.format(),
                expected_object_format(&target),
                "`{triple}`"
            );
            assert_eq!(
                object.architecture(),
                expected_architecture(&target),
                "`{triple}`"
            );
            if target.arch == "riscv64" {
                let FileFlags::Elf { e_flags, .. } = object.flags() else {
                    panic!("`{triple}` built a non-ELF object");
                };
                assert_eq!(
                    e_flags & EF_RISCV_FLOAT_ABI,
                    EF_RISCV_FLOAT_ABI_DOUBLE,
                    "`{triple}` must pass floats as lp64d does"
                );
            }

            if !in_process_lld_available() || is_apple(&target) {
                continue;
            }
            let linked = tempfile::NamedTempFile::new().expect("temp file");
            object_file
                .into_shared_object(linked.path(), &[], None)
                .unwrap_or_else(|e| panic!("linking for `{triple}`: {e}"));
            let bytes = std::fs::read(linked.path()).expect("library was written");
            let library =
                object::File::parse(&*bytes).unwrap_or_else(|e| panic!("`{triple}`: {e}"));
            let expected_format = if target.options.is_like_windows {
                BinaryFormat::Pe
            } else {
                BinaryFormat::Elf
            };
            assert_eq!(library.format(), expected_format, "linked `{triple}`");
            assert_eq!(library.kind(), ObjectKind::Dynamic, "linked `{triple}`");
            assert_eq!(
                library.architecture(),
                expected_architecture(&target),
                "linked `{triple}`"
            );
        }
    }
}
