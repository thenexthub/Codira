//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Tests of the runtime's ARM MTE primitives (`native/memtag_aarch64.c`,
//!   experimental): they compile for aarch64 Linux into an object whose
//!   disassembly has the tagging instructions; a freestanding test program
//!   built from them links; and, only where MTE can actually run (an MTE CPU in
//!   `/proc/cpuinfo`, or `qemu-aarch64` on `PATH`), the program shows that
//!   in-bounds accesses work and that overflowing, untagged and stale pointers
//!   fault. Elsewhere those runs are skipped, saying why.
//! - The host side: loading a memtag library on a host without MTE is refused.

use std::{
    path::{Path, PathBuf},
    process::{Command, Output, Stdio},
    time::{Duration, Instant},
};

fn runtime_dir() -> &'static Path {
    Path::new(env!("CARGO_MANIFEST_DIR"))
}

/// Runs `command`, killing it after 120 seconds.
fn run_with_timeout(mut command: Command) -> Output {
    let mut child = command
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .unwrap_or_else(|e| panic!("could not run {command:?}: {e}"));
    let deadline = Instant::now() + Duration::from_secs(120);
    loop {
        if child.try_wait().expect("child status").is_some() {
            return child.wait_with_output().expect("child output");
        }
        if Instant::now() > deadline {
            let _ignored = child.kill();
            panic!("{command:?} did not finish within 120 s");
        }
        std::thread::sleep(Duration::from_millis(20));
    }
}

/// Clang, if it can be run at all.
fn clang() -> Option<PathBuf> {
    let clang = codira_compiler::find_clang();
    let mut version = Command::new(&clang);
    version.arg("--version");
    version
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .status()
        .ok()
        .filter(std::process::ExitStatus::success)
        .map(|_| clang)
}

/// A tool from the same LLVM installation as `clang`.
fn llvm_tool(clang: &Path, name: &str) -> Option<PathBuf> {
    let path = clang
        .parent()?
        .join(format!("{name}{}", std::env::consts::EXE_SUFFIX));
    path.is_file().then_some(path)
}

#[test]
fn mte_primitives_compile_to_irg_and_tag_stores() {
    let Some(clang) = clang() else {
        eprintln!("skipped: no Clang to compile native/memtag_aarch64.c with");
        return;
    };
    let dir = tempfile::tempdir().unwrap();
    let object = dir.path().join("memtag.o");
    let mut compile = Command::new(&clang);
    compile
        .args([
            "--target=aarch64-unknown-linux-gnu",
            "-O2",
            "-ffreestanding",
            "-Wall",
            "-Werror",
            "-c",
        ])
        .arg(runtime_dir().join("native/memtag_aarch64.c"))
        .arg("-o")
        .arg(&object);
    let output = run_with_timeout(compile);
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );

    let Some(objdump) = llvm_tool(&clang, "llvm-objdump") else {
        eprintln!(
            "skipped the disassembly: no llvm-objdump next to {}",
            clang.display()
        );
        return;
    };
    let mut disassemble = Command::new(objdump);
    disassemble.args(["-d", "--mattr=+mte"]).arg(&object);
    let output = run_with_timeout(disassemble);
    let listing = String::from_utf8_lossy(&output.stdout);
    for mnemonic in ["irg\t", "st2g\t", "stg\t", "ldg\t", "svc\t"] {
        assert!(listing.contains(mnemonic), "no `{mnemonic}` in:\n{listing}");
    }
}

/// Why this host cannot run MTE code, or `None` if it can, with the way to
/// run an aarch64 program (natively, or under `qemu-aarch64 -cpu max`).
fn mte_environment() -> Result<Option<PathBuf>, String> {
    let cpuinfo = std::fs::read_to_string("/proc/cpuinfo");
    let native_mte = cfg!(target_arch = "aarch64")
        && cpuinfo.as_ref().is_ok_and(|info| {
            info.lines()
                .filter(|line| line.starts_with("Features"))
                .any(|line| line.split_whitespace().any(|feature| feature == "mte"))
        });
    if native_mte {
        return Ok(None);
    }
    let qemu = std::env::var_os("PATH").and_then(|paths| {
        std::env::split_paths(&paths)
            .map(|dir| dir.join(format!("qemu-aarch64{}", std::env::consts::EXE_SUFFIX)))
            .find(|path| path.is_file())
    });
    if let Some(qemu) = qemu {
        return Ok(Some(qemu));
    }
    let cpu = match cpuinfo {
        Err(_) => "there is no /proc/cpuinfo".to_owned(),
        Ok(_) if !cfg!(target_arch = "aarch64") => {
            format!("the CPU is {}, not aarch64", std::env::consts::ARCH)
        }
        Ok(_) => "/proc/cpuinfo lists no `mte` feature".to_owned(),
    };
    Err(format!(
        "no MTE-capable environment: {cpu}, and no qemu-aarch64 is on PATH"
    ))
}

#[test]
fn mte_faults_on_overflowing_untagged_and_stale_pointers() {
    let Some(clang) = clang() else {
        eprintln!("skipped: no Clang to build the MTE test program with");
        return;
    };
    let dir = tempfile::tempdir().unwrap();
    let program = dir.path().join("memtag_harness");
    let mut build = Command::new(&clang);
    build
        .args([
            "--target=aarch64-unknown-linux-gnu",
            "-O1",
            "-ffreestanding",
            "-nostdlib",
            "-static",
            "-fuse-ld=lld",
        ])
        .arg(runtime_dir().join("tests/memtag/harness.c"))
        .arg(runtime_dir().join("native/memtag_aarch64.c"))
        .arg("-o")
        .arg(&program);
    let output = run_with_timeout(build);
    if !output.status.success() {
        let stderr = String::from_utf8_lossy(&output.stderr);
        // Linking needs LLD; compiling alone is covered above.
        assert!(
            stderr.contains("lld") || stderr.contains("linker"),
            "building the MTE test program failed: {stderr}"
        );
        eprintln!("skipped: could not link the MTE test program here: {stderr}");
        return;
    }

    let runner = match mte_environment() {
        Ok(runner) => runner,
        Err(reason) => {
            eprintln!("skipped running the MTE test program (it was built): {reason}");
            return;
        }
    };
    let run = |scenario: &str| {
        let mut command = match &runner {
            Some(qemu) => {
                let mut command = Command::new(qemu);
                command.args(["-cpu", "max"]).arg(&program);
                command
            }
            None => Command::new(&program),
        };
        command.arg(scenario);
        run_with_timeout(command).status
    };

    let ok = run("ok");
    assert_eq!(ok.code(), Some(0), "in-bounds accesses: {ok:?}");
    for scenario in ["overflow", "untagged", "stale"] {
        let status = run(scenario);
        #[cfg(unix)]
        {
            use std::os::unix::process::ExitStatusExt;
            assert_eq!(
                status.signal(),
                Some(11),
                "`{scenario}` must end in SIGSEGV, ended {status:?}"
            );
        }
        #[cfg(not(unix))]
        assert!(
            !status.success() && status.code() != Some(6),
            "`{scenario}` must fault, ended {status:?}"
        );
    }
}

/// On a host without MTE, a memtag library is refused at load time rather
/// than run with its checks off.
#[test]
fn activating_mte_without_mte_is_refused() {
    if mte_environment().is_ok() && cfg!(all(target_arch = "aarch64", target_os = "linux")) {
        eprintln!("skipped: this host has MTE");
        return;
    }
    let error = codira_runtime::memtag::activate().unwrap_err();
    let message = error.to_string();
    assert!(
        message.contains("--sanitize=memtag (experimental)"),
        "{message}"
    );
    assert!(!codira_runtime::memtag::is_active());
}
