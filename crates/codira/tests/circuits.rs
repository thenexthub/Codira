//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - End-to-end checks of `codira_circuits` against natively compiled Codira:
//!   every operator, on every integer width, through the circuit evaluator and
//!   the R1CS witness, against the same function compiled by LLVM and run;
//!   `codira attest build|prove|verify` on a real file, with a tampered reveal
//!   rejected; and, when the tools are present, circom/snarkjs checking the
//!   emitted Circom and R1CS. That external check reports a skip, and passes,
//!   when the tools are not installed.

use std::{
    collections::HashMap,
    ffi::OsString,
    path::{Path, PathBuf},
    process::{Command, Stdio},
    time::{Duration, Instant},
};

use codira::{run_with_args, ExitStatus};
use codira_circuits::{elaborate, r1cs, ElabOptions, Mode};
use codira_compiler::{Config, Driver, PathOrInline, Target};
use codira_runtime::Runtime;

/// `SplitMix64`, so the random cases are the same on every run.
struct SplitMix(u64);

impl SplitMix {
    fn next(&mut self) -> u64 {
        self.0 = self.0.wrapping_add(0x9E37_79B9_7F4A_7C15);
        let mut z = self.0;
        z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
        z ^ (z >> 31)
    }
}

fn write_package(dir: &Path, source: &str) {
    std::fs::write(
        dir.join("codira.toml"),
        "[package]\nname=\"circuits\"\nauthors=[]\nversion=\"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir_all(dir.join("src")).unwrap();
    std::fs::write(dir.join("src/mod.code"), source).unwrap();
}

fn codira(args: &[&str]) -> ExitStatus {
    let args: Vec<OsString> = std::iter::once("codira")
        .chain(args.iter().copied())
        .map(OsString::from)
        .collect();
    run_with_args(args).unwrap()
}

fn build_native(dir: &Path) -> Runtime {
    let manifest = dir.join("codira.toml");
    assert_eq!(
        codira(&["build", "--manifest-path", manifest.to_str().unwrap()]),
        ExitStatus::Success
    );
    let library = dir.join("target/mod.codiralib");
    // Safety: the library was compiled by this test.
    unsafe { Runtime::builder(&library).finish() }.unwrap()
}

/// Elaborates every function named in `names` from `file`, in `mode`.
fn circuits(
    file: &Path,
    names: &[String],
    mode: Mode,
) -> HashMap<String, (codira_circuits::Circuit, r1cs::R1cs)> {
    let config = Config {
        target: Target::host_target().unwrap(),
        ..Config::default()
    };
    let (driver, _) = Driver::with_file(config, PathOrInline::Path(file.to_path_buf())).unwrap();
    let db = driver.db();
    let mut out = HashMap::new();
    let candidates = match mode {
        Mode::Attest => elaborate::find_oblivious(db),
        Mode::Fpga => elaborate::find_fpga(db),
    };
    for func in candidates {
        let name = func.name(db).to_string();
        if names.contains(&name) {
            let c = codira_circuits::elaborate(db, func, mode, &ElabOptions::default())
                .unwrap_or_else(|e| panic!("{name}: {e}"));
            let r = r1cs::lower(&c);
            out.insert(name, (c, r));
        }
    }
    assert_eq!(out.len(), names.len(), "not every function elaborated");
    out
}

/// Interesting values of every width, as 64-bit patterns that are truncated
/// to each type.
#[allow(clippy::cast_sign_loss)]
fn edges() -> Vec<u64> {
    let mut v = vec![
        0, 1, 2, 3, 5, 7, 8, 15, 16, 31, 32, 63, 64, 100, 127, 128, 255,
    ];
    for w in [8u32, 16, 32, 64] {
        let max = if w == 64 { u64::MAX } else { (1u64 << w) - 1 };
        let half = 1u64 << (w - 1);
        v.extend([max, max - 1, half, half - 1, half + 1]);
    }
    v.extend([
        (-1i64) as u64,
        (-2i64) as u64,
        (-63i64) as u64,
        0x5555_5555_5555_5555,
        0xAAAA_AAAA_AAAA_AAAA,
    ]);
    v.sort_unstable();
    v.dedup();
    v
}

/// Pairs of inputs: every pair of edge values, and random ones.
fn pairs(seed: u64) -> Vec<(u64, u64)> {
    let e = edges();
    let mut out: Vec<(u64, u64)> = e
        .iter()
        .flat_map(|&a| e.iter().map(move |&b| (a, b)))
        .collect();
    let mut rng = SplitMix(seed);
    for _ in 0..300 {
        out.push((rng.next(), rng.next()));
    }
    out
}

const TYPES: &[(&str, u32)] = &[
    ("i64", 64),
    ("u64", 64),
    ("i32", 32),
    ("u16", 16),
    ("i8", 8),
    ("u8", 8),
];

const BINARY: &[(&str, &str)] = &[
    ("add", "+"),
    ("sub", "-"),
    ("mul", "*"),
    ("and", "&"),
    ("or", "|"),
    ("xor", "^"),
    ("shl", "<<"),
    ("shr", ">>"),
];

const COMPARE: &[(&str, &str)] = &[
    ("eq", "=="),
    ("ne", "!="),
    ("lt", "<"),
    ("le", "<="),
    ("gt", ">"),
    ("ge", ">="),
];

/// One `public oblivious func` per operator and type, plus a few composite
/// ones.
fn operator_source() -> (String, Vec<String>) {
    let mut src = String::new();
    let mut names = Vec::new();
    for (ty, _) in TYPES {
        for (name, op) in BINARY {
            let f = format!("{name}_{ty}");
            src.push_str(&format!(
                "public oblivious func {f}(a: {ty}, b: {ty}) -> {ty} {{ a {op} b }}\n"
            ));
            names.push(f);
        }
        for (name, op) in COMPARE {
            let f = format!("{name}_{ty}");
            src.push_str(&format!(
                "public oblivious func {f}(a: {ty}, b: {ty}) -> bool {{ a {op} b }}\n"
            ));
            names.push(f);
        }
        let f = format!("not_{ty}");
        src.push_str(&format!(
            "public oblivious func {f}(a: {ty}, b: {ty}) -> {ty} {{ ~a ^ (b & 0) }}\n"
        ));
        names.push(f);
        let f = format!("sel_{ty}");
        src.push_str(&format!(
            "public oblivious func {f}(a: {ty}, b: {ty}) -> {ty} {{ ct_select(a < b, b - a, a * 3 + b) }}\n"
        ));
        names.push(f);
        if ty.starts_with('i') {
            let f = format!("neg_{ty}");
            src.push_str(&format!(
                "public oblivious func {f}(a: {ty}, b: {ty}) -> {ty} {{ -a + b }}\n"
            ));
            names.push(f);
        }
    }
    // Casts between widths, both ways, and from bool.
    for (from, _) in TYPES {
        for (to, _) in TYPES {
            if from != to {
                let f = format!("cast_{from}_{to}");
                src.push_str(&format!(
                    "public oblivious func {f}(a: {from}, b: {from}) -> {to} {{ (a as {to}) + ((b == a) as {to}) }}\n"
                ));
                names.push(f);
            }
        }
    }
    (src, names)
}

/// Native value of `name(a, b)` (unsigned encoding of the result).
#[allow(
    clippy::cast_possible_truncation,
    clippy::cast_possible_wrap,
    clippy::cast_sign_loss
)]
fn native(rt: &Runtime, name: &str, param: &str, result: &str, a: u64, b: u64) -> u64 {
    macro_rules! call {
        ($t:ty) => {{
            let (a, b) = (a as $t, b as $t);
            match result {
                "bool" => u64::from(rt.invoke::<bool, _>(name, (a, b)).unwrap()),
                "i64" => rt.invoke::<i64, _>(name, (a, b)).unwrap() as u64,
                "u64" => rt.invoke::<u64, _>(name, (a, b)).unwrap(),
                "i32" => u64::from(rt.invoke::<i32, _>(name, (a, b)).unwrap() as u32),
                "u16" => u64::from(rt.invoke::<u16, _>(name, (a, b)).unwrap()),
                "i8" => u64::from(rt.invoke::<i8, _>(name, (a, b)).unwrap() as u8),
                "u8" => u64::from(rt.invoke::<u8, _>(name, (a, b)).unwrap()),
                other => panic!("result type {other}"),
            }
        }};
    }
    match param {
        "i64" => call!(i64),
        "u64" => call!(u64),
        "i32" => call!(i32),
        "u16" => call!(u16),
        "i8" => call!(i8),
        "u8" => call!(u8),
        other => panic!("parameter type {other}"),
    }
}

#[test]
fn every_operator_matches_native_code_on_every_width() {
    let project = tempfile::tempdir().unwrap();
    let (src, names) = operator_source();
    write_package(project.path(), &src);
    let rt = build_native(project.path());
    let circuits = circuits(&project.path().join("src/mod.code"), &names, Mode::Attest);

    let mut checked = 0usize;
    for name in &names {
        let (c, r) = &circuits[name];
        let param = c.inputs[0].ty.to_string();
        let result = c.output_ty().to_string();
        let width = u64::from(c.inputs[0].ty.width());
        let mask = c.inputs[0].ty.mask();
        for (i, (a, b)) in pairs(name.len() as u64).into_iter().enumerate() {
            // Native shifts by the width or more are undefined (LLVM poison);
            // keep amounts in range, including width - 1.
            let b = if name.starts_with("sh") { b % width } else { b };
            // A large, regular sample for the R1CS (the slowest path).
            let with_r1cs = i % 3 == 0;
            let (a, b) = (a & mask, b & mask);
            let want = native(&rt, name, &param, &result, a, b) & c.output_ty().mask();
            assert_eq!(c.eval(&[a, b]), want, "circuit {name}({a:#x}, {b:#x})");
            if with_r1cs {
                let w = r.witness(&[a, b]).unwrap_or_else(|e| panic!("{name}: {e}"));
                assert_eq!(r.output(&w), want, "R1CS {name}({a:#x}, {b:#x})");
            }
            checked += 1;
        }
    }
    assert!(checked > 100_000, "{checked}");
}

const SCORE: &str = r#"
oblivious func bonus(x: i64, y: i64) -> i64 {
    ct_select(x > y, 10i64, -10i64)
}

oblivious func score(x: i64, y: i64) -> i64 {
    var total = 0
    for i in 0..3 {
        total += x * (i + 1) - (y >> i)
    }
    total + bonus(x, y)
}

public func score_native(x: i64, y: i64) -> i64 {
    declassify(score(x, y))
}
"#;

#[test]
fn attest_builds_proves_and_verifies_and_rejects_a_wrong_reveal() {
    let dir = tempfile::tempdir().unwrap();
    write_package(dir.path(), SCORE);
    let rt = build_native(dir.path());
    let src = dir.path().join("src/mod.code");
    let src = src.to_str().unwrap();
    let out = dir.path().join("keys");
    let out_s = out.to_str().unwrap();

    assert_eq!(
        codira(&[
            "attest",
            "build",
            &format!("{src}::score"),
            "--out-dir",
            out_s
        ]),
        ExitStatus::Success
    );
    for ext in ["r1cs", "circom", "pk", "vk"] {
        assert!(out.join(format!("score.{ext}")).is_file(), "score.{ext}");
    }

    let (x, y) = (-7i64, 1234i64);
    let expected: i64 = rt.invoke("score_native", (x, y)).unwrap();
    let proof = out.join("score.proof");
    let wtns = out.join("score.wtns");
    assert_eq!(
        codira(&[
            "attest",
            "prove",
            &format!("{src}::score"),
            "--input",
            &format!("x={x}"),
            "--input",
            &format!("y={y}"),
            "--pk",
            out.join("score.pk").to_str().unwrap(),
            "-o",
            proof.to_str().unwrap(),
            "--wtns",
            wtns.to_str().unwrap(),
        ]),
        ExitStatus::Success
    );
    let proof_json: serde_json::Value =
        serde_json::from_str(&std::fs::read_to_string(&proof).unwrap()).unwrap();
    assert_eq!(proof_json["reveal"], expected.to_string());
    // The proof file does not contain the private inputs.
    let text = std::fs::read_to_string(&proof).unwrap();
    assert!(!text.contains("1234") && !text.contains("-7,"), "{text}");

    let vk = out.join("score.vk");
    let verify = |reveal: Option<&str>| {
        let mut args = vec![
            "attest",
            "verify",
            "--vk",
            vk.to_str().unwrap(),
            "--proof",
            proof.to_str().unwrap(),
        ];
        if let Some(r) = reveal {
            args.extend(["--reveal", r]);
        }
        codira(&args)
    };
    assert_eq!(verify(None), ExitStatus::Success);
    assert_eq!(verify(Some(&expected.to_string())), ExitStatus::Success);
    assert_eq!(verify(Some(&(expected + 1).to_string())), ExitStatus::Error);
    assert_eq!(verify(Some(&(expected - 1).to_string())), ExitStatus::Error);

    // A proof with a corrupted body is rejected, not accepted or a crash.
    let mut tampered = proof_json.clone();
    let hex = tampered["proof"].as_str().unwrap().to_string();
    let flipped = if hex.starts_with('0') { "1" } else { "0" };
    tampered["proof"] = serde_json::Value::String(format!("{flipped}{}", &hex[1..]));
    std::fs::write(&proof, tampered.to_string()).unwrap();
    assert_eq!(verify(None), ExitStatus::Error);
}

#[test]
fn attest_refuses_what_is_not_an_oblivious_circuit() {
    let dir = tempfile::tempdir().unwrap();
    let file = dir.path().join("bad.code");
    let file_s = file.to_str().unwrap();
    let out = dir.path().to_str().unwrap();

    // No oblivious function at all.
    std::fs::write(&file, "func f(x: i64) -> i64 { x }\n").unwrap();
    assert!(run_with_args(["codira", "attest", "build", file_s, "--out-dir", out]).is_err());

    // The information-flow checker stops a leaking oblivious function.
    std::fs::write(
        &file,
        "oblivious func f(x: i64) -> i64 {\n    if x > 0 { 1 } else { 2 }\n}\n",
    )
    .unwrap();
    assert_eq!(
        codira(&["attest", "build", file_s, "--out-dir", out]),
        ExitStatus::Error
    );

    // A float is outside the subset.
    std::fs::write(&file, "oblivious func f(x: f64) -> f64 {\n    x * 2.0\n}\n").unwrap();
    assert_eq!(
        codira(&["attest", "build", file_s, "--out-dir", out]),
        ExitStatus::Error
    );
}

/// Runs `program` with `args`, killing it after `limit`; `None` if it could
/// not be started.
fn run_with_timeout(program: &Path, args: &[&str], cwd: &Path, limit: Duration) -> Option<String> {
    let mut child = Command::new(program)
        .args(args)
        .current_dir(cwd)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .ok()?;
    let start = Instant::now();
    loop {
        if child.try_wait().ok()?.is_some() {
            break;
        }
        if start.elapsed() > limit {
            let _ = child.kill();
            panic!("{} timed out after {limit:?}", program.display());
        }
        std::thread::sleep(Duration::from_millis(50));
    }
    let output = child.wait_with_output().ok()?;
    Some(
        String::from_utf8_lossy(&output.stdout).into_owned()
            + &String::from_utf8_lossy(&output.stderr),
    )
}

/// The directory whose `node_modules` holds `circom2` and `snarkjs`
/// (`CODIRA_ZK_TOOLS`), if set and usable.
fn zk_tools() -> Option<(PathBuf, PathBuf)> {
    let dir = PathBuf::from(std::env::var_os("CODIRA_ZK_TOOLS")?);
    let circom = dir.join("node_modules/circom2/cli.js");
    let snarkjs = dir.join("node_modules/snarkjs/build/cli.cjs");
    (circom.is_file() && snarkjs.is_file()).then_some((circom, snarkjs))
}

#[test]
#[allow(clippy::cast_sign_loss)]
fn circom_and_snarkjs_accept_the_emitted_circuit_and_witnesses() {
    let Some((circom, snarkjs)) = zk_tools() else {
        eprintln!(
            "SKIPPED: set CODIRA_ZK_TOOLS to a directory with circom2 and snarkjs in node_modules"
        );
        return;
    };
    let node = PathBuf::from("node");
    let limit = Duration::from_secs(300);
    let dir = tempfile::tempdir().unwrap();
    write_package(dir.path(), SCORE);
    let rt = build_native(dir.path());
    let circuits = circuits(
        &dir.path().join("src/mod.code"),
        &["score".to_string()],
        Mode::Attest,
    );
    let (c, r) = &circuits["score"];
    let work = dir.path();
    std::fs::write(work.join("ours.r1cs"), r.to_iden3_r1cs()).unwrap();
    std::fs::write(work.join("score.circom"), codira_circuits::circom::emit(r)).unwrap();

    let run = |script: &Path, args: &[&str]| {
        let mut all = vec![script.to_str().unwrap()];
        all.extend_from_slice(args);
        run_with_timeout(&node, &all, work, limit).expect("node starts")
    };
    let log = run(&circom, &["score.circom", "--r1cs", "--wasm", "-o", "."]);
    assert!(log.contains("Everything went okay"), "{log}");

    for (x, y) in [(-7i64, 1234i64), (i64::MAX, i64::MIN), (0, -1), (12345, 3)] {
        let expected: i64 = rt.invoke("score_native", (x, y)).unwrap();
        let inputs = [x as u64, y as u64];

        // snarkjs checks Codira's own .r1cs and .wtns files.
        let w = r.witness(&inputs).unwrap();
        std::fs::write(work.join("ours.wtns"), r1cs::R1cs::to_iden3_wtns(&w)).unwrap();
        let log = run(&snarkjs, &["wtns", "check", "ours.r1cs", "ours.wtns"]);
        assert!(log.contains("WITNESS IS CORRECT"), "{log}");

        // circom's witness generator, from the emitted Circom, computes the
        // same output, and snarkjs checks it against circom's R1CS.
        std::fs::write(
            work.join("input.json"),
            format!("{{\"x\": \"{}\", \"y\": \"{}\"}}", inputs[0], inputs[1]),
        )
        .unwrap();
        let log = run(
            &snarkjs,
            &[
                "wtns",
                "calculate",
                "score_js/score.wasm",
                "input.json",
                "circom.wtns",
            ],
        );
        assert!(!log.to_lowercase().contains("error"), "{log}");
        let log = run(&snarkjs, &["wtns", "check", "score.r1cs", "circom.wtns"]);
        assert!(log.contains("WITNESS IS CORRECT"), "{log}");
        run(
            &snarkjs,
            &["wtns", "export", "json", "circom.wtns", "circom.json"],
        );
        let values: Vec<String> =
            serde_json::from_str(&std::fs::read_to_string(work.join("circom.json")).unwrap())
                .unwrap();
        assert_eq!(values[1], (expected as u64).to_string(), "score({x}, {y})");
        assert_eq!(c.eval(&inputs), expected as u64);
    }
}
