//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Tests of Funis as a library: `funis::link` links in-process, returns instead of exiting,
//!   reports diagnostics as data, can run again after a failure, and is safe to call from
//!   several threads.

mod common;

use common::{Dir, args, elf_object, exported, macho_object, shared_args};
use funis::{LinkError, Outcome, Severity};
use object::{Architecture, Object as _};

const ELF_ARCHES: [(Architecture, &str); 3] = [
    (Architecture::X86_64, "x86_64"),
    (Architecture::Aarch64, "arm64"),
    (Architecture::Riscv64, "riscv64"),
];

fn names(bytes: &[u8]) -> Vec<String> {
    exported(bytes).into_iter().map(|(name, _)| name).collect()
}

#[test]
fn links_a_shared_object_for_each_elf_target() {
    for (arch, target) in ELF_ARCHES {
        let dir = Dir::new(target);
        let a = dir.write("a.o", &elf_object(arch, "a", true, 0));
        let b = dir.write("b.o", &elf_object(arch, "b", false, 0));
        let out = dir.file("libab.so");
        let report = funis::link(shared_args(&out, &[&a, &b])).unwrap_or_else(|e| panic!("{e}"));
        assert_eq!(report.outcome, Outcome::Linked);
        assert_eq!(report.target, Some(target));
        assert_eq!(report.output.as_deref(), Some(out.as_path()));
        assert!(report.threads >= 1);
        let bytes = std::fs::read(&out).unwrap();
        let names = names(&bytes);
        for want in ["f", "a_g", "b_g", "a_ptr", "b_ptr"] {
            assert!(names.iter().any(|n| n == want), "{target}: {want} not exported: {names:?}");
        }
    }
}

#[test]
fn an_input_that_calls_nothing_links_the_same_every_time() {
    let dir = Dir::new("determinism");
    let a = dir.write("a.o", &elf_object(Architecture::X86_64, "a", true, 0));
    let out = dir.file("out.so");
    let mut first = None;
    for _ in 0..5 {
        funis::link(shared_args(&out, &[&a])).unwrap();
        let bytes = std::fs::read(&out).unwrap();
        match &first {
            None => first = Some(bytes),
            Some(first) => assert!(first == &bytes, "the output changed between links"),
        }
    }
}

#[test]
fn version_and_help_come_back_as_values() {
    let report = funis::link(args(["funis", "--version"])).unwrap();
    assert_eq!(report.outcome, Outcome::Informational);
    assert!(report.stdout.contains("funis"), "{:?}", report.stdout);
    assert!(report.output.is_none());

    let report = funis::link(args(["funis", "--help"])).unwrap();
    assert_eq!(report.outcome, Outcome::Informational);
    assert!(report.stdout.contains("Usage"), "{:?}", report.stdout);

    // Mach-O by the program's name.
    let report = funis::link(args(["ld64.funis", "-v"])).unwrap();
    assert_eq!(report.outcome, Outcome::Informational);
    assert_eq!(report.format, funis::Format::MachO);
}

#[test]
fn what_funis_does_not_implement_is_reported_as_unsupported() {
    let dir = Dir::new("unsupported");
    let a = dir.write("a.o", &elf_object(Architecture::X86_64, "a", true, 0));
    let out = dir.file("out.so");

    let mut with_unknown_option = shared_args(&out, &[&a]);
    with_unknown_option.push("--no-such-option".into());
    let err = funis::link(with_unknown_option).unwrap_err();
    assert!(err.is_unsupported(), "{err}");

    let err = funis::link(args(["funis", "-run", "true"])).unwrap_err();
    assert!(err.is_unsupported(), "{err}");

    let mut with_plugin = shared_args(&out, &[&a]);
    with_plugin.push("--plugin=nothing.so".into());
    let err = funis::link(with_plugin).unwrap_err();
    assert!(err.is_unsupported(), "{err}");

    // -C would change the working directory of the whole process.
    let mut with_chdir = shared_args(&out, &[&a]);
    with_chdir.push("-C".into());
    with_chdir.push(dir.0.clone().into_os_string());
    let err = funis::link(with_chdir).unwrap_err();
    assert!(err.is_unsupported(), "{err}");
    assert!(!out.exists());
}

#[test]
fn a_link_error_is_an_error_not_unsupported_and_the_next_link_works() {
    let dir = Dir::new("failure");
    let a = dir.write("a.o", &elf_object(Architecture::X86_64, "a", true, 0));
    let dup = dir.write("dup.o", &elf_object(Architecture::X86_64, "dup", true, 0));
    let out = dir.file("out.so");

    // f is defined twice.
    let err = funis::link(shared_args(&out, &[&a, &dup])).unwrap_err();
    assert!(matches!(err, LinkError::Failed { .. }), "{err}");
    assert!(
        err.diagnostics()
            .iter()
            .any(|d| d.severity == Severity::Error && d.message.contains("duplicate symbol")),
        "{err}"
    );
    assert!(!out.exists(), "a failed link left its output behind");
    let leftovers: Vec<_> = std::fs::read_dir(&dir.0).unwrap().flatten().collect();
    assert_eq!(leftovers.len(), 2, "temporary files left behind: {leftovers:?}");

    // An input that does not exist ends the link at once.
    let mut missing = shared_args(&out, &[&dir.file("missing.o")]);
    missing.extend(args(["-m", "elf_x86_64"]));
    let err = funis::link(missing).unwrap_err();
    assert!(matches!(err, LinkError::Failed { .. }), "{err}");
    assert!(err.diagnostics().iter().any(|d| d.message.contains("missing.o")), "{err}");

    // Nothing carried over from the failures.
    let report = funis::link(shared_args(&out, &[&a])).unwrap();
    assert_eq!(report.outcome, Outcome::Linked);
    assert!(report.diagnostics.iter().all(|d| d.severity != Severity::Error));
    assert!(out.exists());
}

#[test]
fn a_corrupt_input_ends_the_link_from_inside_the_pool_and_the_next_link_works() {
    let dir = Dir::new("corrupt");
    let good = elf_object(Architecture::X86_64, "a", true, 70_000);
    let a = dir.write("a.o", &good);
    let out = dir.file("out.so");
    // Cut inside the section headers, and inside the sections: each makes a
    // worker thread of the pool end the link with a fatal error.
    for cut in [good.len() - 40, good.len() / 2, 200] {
        let bad = dir.write("bad.o", &good[..cut]);
        let mut cmd = shared_args(&out, &[&bad]);
        cmd.extend(args(["-m", "elf_x86_64"]));
        let err = funis::link(cmd).unwrap_err();
        assert!(matches!(err, LinkError::Failed { .. }), "cut at {cut}: {err}");
        assert!(!out.exists());
        funis::link(shared_args(&out, &[&a])).unwrap();
        std::fs::remove_file(&out).unwrap();
    }
}

#[test]
fn warnings_are_data() {
    let dir = Dir::new("warnings");
    let a = dir.write("a.o", &elf_object(Architecture::X86_64, "a", true, 0));
    let out = dir.file("out.so");
    let mut cmd = shared_args(&out, &[&a]);
    cmd.push("-zno-such-thing".into());
    let report = funis::link(cmd).unwrap();
    assert!(
        report.diagnostics.iter().any(|d| d.severity == Severity::Warning
            && d.message.contains("unknown command line option")),
        "{:?}",
        report.diagnostics
    );
}

#[test]
fn options_of_one_link_do_not_leak_into_the_next() {
    let dir = Dir::new("options");
    let a = dir.write("a.o", &elf_object(Architecture::X86_64, "a", true, 0));
    let dup = dir.write("dup.o", &elf_object(Architecture::X86_64, "dup", true, 0));
    let out = dir.file("out.so");

    // --noinhibit-exec turns the duplicate definition into a warning and the link succeeds.
    let mut cmd = shared_args(&out, &[&a, &dup]);
    cmd.push("--noinhibit-exec".into());
    let report = funis::link(cmd).unwrap_or_else(|e| panic!("{e}"));
    assert!(report.diagnostics.iter().any(|d| d.message.contains("duplicate symbol")));
    assert!(out.exists());
    std::fs::remove_file(&out).unwrap();

    // The same inputs without it fail: the option was not remembered.
    let err = funis::link(shared_args(&out, &[&a, &dup])).unwrap_err();
    assert!(matches!(err, LinkError::Failed { .. }), "{err}");
    assert!(!out.exists());
}

#[test]
fn the_thread_count_follows_the_input_or_the_option() {
    let dir = Dir::new("threads");
    let a = dir.write("a.o", &elf_object(Architecture::X86_64, "a", true, 0));
    let out = dir.file("out.so");
    let report = funis::link(shared_args(&out, &[&a])).unwrap();
    assert_eq!(report.threads, 1, "a few hundred bytes of input need one thread");

    let mut cmd = shared_args(&out, &[&a]);
    cmd.push("--thread-count=3".into());
    let report = funis::link(cmd).unwrap();
    assert_eq!(report.threads, 3);
}

#[test]
fn links_from_several_threads_take_turns() {
    let dir = Dir::new("concurrent");
    let expected: Vec<_> = {
        let a = dir.write("ref.o", &elf_object(Architecture::X86_64, "ref", true, 4096));
        let out = dir.file("ref.so");
        funis::link(shared_args(&out, &[&a])).unwrap();
        names(&std::fs::read(&out).unwrap())
    };
    assert!(!expected.is_empty());

    let handles: Vec<_> = (0..4)
        .map(|n| {
            let root = dir.0.clone();
            std::thread::spawn(move || {
                for round in 0..15 {
                    let input = root.join(format!("t{n}-{round}.o"));
                    std::fs::write(&input, elf_object(Architecture::X86_64, "ref", true, 4096))
                        .unwrap();
                    let out = root.join(format!("t{n}-{round}.so"));
                    let report = funis::link(shared_args(&out, &[&input]))
                        .unwrap_or_else(|e| panic!("thread {n} round {round}: {e}"));
                    assert_eq!(report.outcome, Outcome::Linked);
                    assert_eq!(report.output.as_deref(), Some(out.as_path()));
                    // A failing link in the middle must not disturb the others.
                    if round % 5 == 0 {
                        let missing = root.join("nope.o");
                        assert!(funis::link(shared_args(&out, &[&missing])).is_err());
                    }
                }
            })
        })
        .collect();
    for handle in handles {
        handle.join().unwrap();
    }
    for n in 0..4 {
        let bytes = std::fs::read(dir.file(&format!("t{n}-14.so"))).unwrap();
        assert_eq!(names(&bytes), expected);
    }
}

#[test]
fn macho_dylib_links_in_process() {
    for (arch, target, name) in
        [(Architecture::Aarch64, "arm64", "arm64"), (Architecture::X86_64, "x86_64", "x86_64")]
    {
        let dir = Dir::new(&format!("macho-{target}"));
        let a = dir.write("a.o", &macho_object(arch, "a", 0));
        let out = dir.file("liba.dylib");
        std::fs::create_dir(dir.file("lib")).unwrap();
        dir.write("lib/libSystem.tbd", LIBSYSTEM_TBD.as_bytes());
        let cmd = macho_args(&dir, &out, &a, name);
        let report = funis::link(cmd).unwrap_or_else(|e| panic!("{target}: {e}"));
        assert_eq!(report.outcome, Outcome::Linked);
        assert_eq!(report.format, funis::Format::MachO);
        assert_eq!(report.target, Some(target));
        let bytes = std::fs::read(&out).unwrap();
        let file = object::File::parse(bytes.as_slice()).expect("a Mach-O file");
        assert_eq!(file.format(), object::BinaryFormat::MachO);
        let exports: Vec<String> = file
            .exports()
            .unwrap()
            .iter()
            .map(|e| String::from_utf8_lossy(e.name()).into_owned())
            .collect();
        assert!(exports.iter().any(|e| e == "_a_g"), "{target}: {exports:?}");

        // A second link, and one after a failure.
        assert!(funis::link(args(["ld64.funis", "-dylib", "-o", "x", "no-such.o"])).is_err());
        funis::link(macho_args(&dir, &out, &dir.file("a.o"), name)).unwrap();
    }
}

/// A stand-in for the SDK's libSystem, which a dylib must link with.
const LIBSYSTEM_TBD: &str = "--- !tapi-tbd
tbd-version:     4
targets:         [ x86_64-macos, arm64-macos ]
install-name:    '/usr/lib/libSystem.B.dylib'
current-version: 1311
exports:
  - targets:         [ x86_64-macos, arm64-macos ]
    symbols:         [ dyld_stub_binder ]
...
";

fn macho_args(
    dir: &Dir,
    out: &std::path::Path,
    input: &std::path::Path,
    arch: &str,
) -> Vec<std::ffi::OsString> {
    let mut cmd = args(["ld64.funis", "-dylib", "-arch", arch, "-platform_version"]);
    cmd.extend(args(["macos", "11.0", "11.0", "-L"]));
    cmd.push(dir.file("lib").into());
    cmd.extend(args(["-lSystem", "-o"]));
    cmd.push(out.into());
    cmd.push(input.into());
    cmd
}

/// The library is the command line's linker, not a lookalike: the same arguments give the same
/// bytes, for every ELF target.
#[test]
fn the_output_is_the_command_lines_output() {
    for (arch, target) in ELF_ARCHES {
        let dir = Dir::new(&format!("cli-{target}"));
        let a = dir.write("a.o", &elf_object(arch, "a", true, 70_000));
        let b = dir.write("b.o", &elf_object(arch, "b", false, 0));
        let lib_out = dir.file("lib.so");
        funis::link(shared_args(&lib_out, &[&a, &b])).unwrap();

        let cli_out = dir.file("cli.so");
        let status = std::process::Command::new(env!("CARGO_BIN_EXE_funis"))
            .args(["--shared", "-o"])
            .arg(&cli_out)
            .arg(&a)
            .arg(&b)
            .status()
            .unwrap();
        assert!(status.success(), "{target}: the command line failed");
        assert!(
            std::fs::read(&lib_out).unwrap() == std::fs::read(&cli_out).unwrap(),
            "{target}: library and command line disagree"
        );
    }
}
