//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - `codira quantum build|run|import|prelude`: compile `@target(qpu)` kernels
//!   to QIR (text or bitcode) or OpenQASM 2/3, run them on the built-in
//!   state-vector simulator, and bring OpenQASM / QIR circuits (e.g. from
//!   Qiskit or Q#) into Codira source.
//!
//! An entry is either a Codira file, optionally naming its kernel as
//! `file.code::kernel`, or an existing circuit file (`.qasm`, `.ll`, `.bc`)
//! that is imported directly -- so `codira quantum run circuit.qasm` runs a
//! Qiskit export with no Codira source at all.

// The doc comments below double as `--help` text, where backticks around
// the language name "OpenQASM" would print literally.
#![allow(clippy::doc_markdown)]

use std::{
    io::stderr,
    path::{Path, PathBuf},
};

use anyhow::{anyhow, bail, Context as _};
use codira_compiler::{Config, DisplayColor, Driver, PathOrInline, Target};
use codira_quantum::{
    elaborate::{self, ElabOptions},
    qasm, qir, sim, Circuit,
};

use crate::ExitStatus;

#[derive(clap::Args)]
pub struct Args {
    #[clap(subcommand)]
    command: Command,
}

#[derive(clap::Subcommand)]
enum Command {
    /// Elaborate a kernel and write it as QIR or OpenQASM
    Build(BuildArgs),
    /// Elaborate a kernel and sample it on the built-in simulator
    Run(RunArgs),
    /// Turn an OpenQASM or QIR file into a Codira source file
    Import(ImportArgs),
    /// Print the `extern "codira-quantum"` block that declares the gates
    Prelude,
}

#[derive(clap::Args)]
struct EntryArgs {
    /// A `.code` file (optionally `file.code::kernel`), or a `.qasm`, `.ll`
    /// or `.bc` circuit file
    entry: String,

    /// The kernel to elaborate, if the file has more than one
    #[clap(long)]
    kernel: Option<String>,

    /// A compile-time value for a kernel parameter, `name=value`
    /// (repeatable)
    #[clap(long = "param", value_name = "NAME=VALUE")]
    params: Vec<String>,

    /// Loop iterations plus calls allowed during elaboration
    #[clap(long, default_value_t = ElabOptions::default().fuel)]
    fuel: u64,
}

#[derive(Clone, Copy, PartialEq, Eq, clap::ValueEnum)]
enum Emit {
    /// QIR base profile, LLVM IR text
    Qir,
    /// QIR base profile, LLVM bitcode
    QirBc,
    /// OpenQASM 2.0
    Qasm2,
    /// OpenQASM 3.0
    Qasm3,
}

#[derive(clap::Args)]
struct BuildArgs {
    #[clap(flatten)]
    entry: EntryArgs,

    /// Output format
    #[clap(long, value_enum, default_value = "qir")]
    emit: Emit,

    /// Output file; standard output if omitted (not allowed for bitcode)
    #[clap(short = 'o', long)]
    output: Option<PathBuf>,

    /// The `qir_major_version` module flag (2 = current spec, opaque
    /// pointers)
    #[clap(long, default_value_t = 2)]
    qir_major: u32,
}

#[derive(clap::Args)]
struct RunArgs {
    #[clap(flatten)]
    entry: EntryArgs,

    /// Number of shots to sample
    #[clap(long, default_value_t = 1024)]
    shots: u64,

    /// Random seed; the same seed gives the same counts
    #[clap(long, default_value_t = 0)]
    seed: u64,

    /// Print exact outcome probabilities instead of sampled counts
    /// (measurement-terminal circuits only)
    #[clap(long)]
    exact: bool,

    /// Print a JSON object `{outcome: count-or-probability}` and nothing
    /// else (for scripts, e.g. the Python binding)
    #[clap(long)]
    json: bool,
}

#[derive(clap::Args)]
struct ImportArgs {
    /// An OpenQASM 2/3 (`.qasm`) or QIR (`.ll`, `.bc`) file
    input: PathBuf,

    /// The generated kernel's name; defaults to the file stem
    #[clap(long)]
    name: Option<String>,

    /// Output `.code` file; defaults to the input with a `.code` extension
    #[clap(short = 'o', long)]
    output: Option<PathBuf>,
}

pub fn quantum(args: Args) -> Result<ExitStatus, anyhow::Error> {
    match args.command {
        Command::Build(args) => build(args),
        Command::Run(args) => run(args),
        Command::Import(args) => import(&args),
        Command::Prelude => {
            print!("{}", elaborate::PRELUDE);
            Ok(ExitStatus::Success)
        }
    }
}

fn build(args: BuildArgs) -> Result<ExitStatus, anyhow::Error> {
    let Some(circuit) = load(&args.entry)? else {
        return Ok(ExitStatus::Error);
    };
    let options = qir::QirOptions {
        major_version: args.qir_major,
        ..qir::QirOptions::default()
    };
    let bytes: Vec<u8> = match args.emit {
        Emit::Qir => qir::emit_qir_text(&circuit, &options)?.into_bytes(),
        Emit::QirBc => {
            if args.output.is_none() {
                bail!("bitcode is binary; pass -o <file>");
            }
            qir::emit_qir_bitcode(&circuit, &options)?
        }
        Emit::Qasm2 => qasm::emit_qasm2(&circuit)?.into_bytes(),
        Emit::Qasm3 => qasm::emit_qasm3(&circuit)?.into_bytes(),
    };
    match &args.output {
        Some(path) => std::fs::write(path, &bytes)
            .with_context(|| format!("could not write '{}'", path.display()))?,
        None => print!("{}", String::from_utf8_lossy(&bytes)),
    }
    Ok(ExitStatus::Success)
}

fn run(args: RunArgs) -> Result<ExitStatus, anyhow::Error> {
    let Some(circuit) = load(&args.entry)? else {
        return Ok(ExitStatus::Error);
    };
    if args.json {
        let fields: Vec<String> = if args.exact {
            sim::outcome_probabilities(&circuit)?
                .iter()
                .map(|(k, p)| format!("\"{k}\": {p:?}"))
                .collect()
        } else {
            sim::run(&circuit, args.shots, args.seed)?
                .iter()
                .map(|(k, n)| format!("\"{k}\": {n}"))
                .collect()
        };
        println!("{{{}}}", fields.join(", "));
        return Ok(ExitStatus::Success);
    }
    println!(
        "{}: {} qubits, {} classical bits, {} operations",
        circuit.name,
        circuit.num_qubits(),
        circuit.num_clbits(),
        circuit.ops.len()
    );
    if args.exact {
        let probs = sim::outcome_probabilities(&circuit)?;
        for (outcome, p) in &probs {
            println!("{outcome:>width$}  {p:.12}", width = outcome.len().max(7));
        }
        return Ok(ExitStatus::Success);
    }
    let counts = sim::run(&circuit, args.shots, args.seed)?;
    println!("{} shots, seed {}", args.shots, args.seed);
    let width = counts.keys().map(String::len).max().unwrap_or(0).max(7);
    let max = counts.values().copied().max().unwrap_or(1);
    for (outcome, n) in &counts {
        // Rounded to the nearest bar character; precision is in the count.
        let bar = (n * 40 + max / 2) / max;
        println!(
            "{outcome:>width$}  {n:>8}  {}",
            "#".repeat(usize::try_from(bar).unwrap_or(0))
        );
    }
    Ok(ExitStatus::Success)
}

fn import(args: &ImportArgs) -> Result<ExitStatus, anyhow::Error> {
    let name = match &args.name {
        Some(n) => n.clone(),
        None => kernel_name_from(&args.input),
    };
    if !codira_quantum::circuit::is_identifier(&name) {
        bail!("`{name}` is not a valid kernel name; pass --name <identifier>");
    }
    let mut circuit = read_circuit_file(&args.input, &name)?;
    let output = args
        .output
        .clone()
        .unwrap_or_else(|| args.input.with_extension("code"));
    let origin = args.input.file_name().map_or_else(
        || args.input.display().to_string(),
        |n| n.to_string_lossy().into_owned(),
    );
    circuit.name = name;
    std::fs::write(&output, elaborate::circuit_to_codira(&circuit, &origin))
        .with_context(|| format!("could not write '{}'", output.display()))?;
    println!(
        "wrote {} (kernel `{}`: {} qubits, {} classical bits, {} operations)",
        output.display(),
        circuit.name,
        circuit.num_qubits(),
        circuit.num_clbits(),
        circuit.ops.len()
    );
    Ok(ExitStatus::Success)
}

/// A valid Codira/QIR identifier derived from a file stem.
fn kernel_name_from(path: &Path) -> String {
    let stem = path.file_stem().map_or_else(
        || "kernel".to_string(),
        |s| s.to_string_lossy().into_owned(),
    );
    let mut name: String = stem
        .chars()
        .map(|c| if c.is_ascii_alphanumeric() { c } else { '_' })
        .collect();
    if !name.starts_with(|c: char| c.is_ascii_alphabetic() || c == '_') {
        name.insert(0, '_');
    }
    name
}

fn read_circuit_file(path: &Path, name: &str) -> Result<Circuit, anyhow::Error> {
    let ext = path
        .extension()
        .map(|e| e.to_string_lossy().to_ascii_lowercase())
        .unwrap_or_default();
    let bytes =
        std::fs::read(path).with_context(|| format!("could not read '{}'", path.display()))?;
    let circuit = match ext.as_str() {
        "bc" => qir::import_qir_bitcode(&bytes)?,
        "ll" => qir::import_qir_text(&String::from_utf8(bytes)?)?,
        "qasm" | "qasm2" | "qasm3" => {
            let text = String::from_utf8(bytes)?;
            qasm::import(&text, name).map_err(|e| anyhow!("{}: {e}", path.display()))?
        }
        other => bail!(
            "'{}': unknown circuit format `.{other}` (expected .qasm, .ll or .bc)",
            path.display()
        ),
    };
    Ok(circuit)
}

/// Loads the entry's circuit: elaborates a Codira kernel, or imports a
/// circuit file. `None` means diagnostics were already printed.
fn load(args: &EntryArgs) -> Result<Option<Circuit>, anyhow::Error> {
    let (path_text, kernel_suffix) = match args.entry.rsplit_once("::") {
        Some((p, k)) => (p.to_string(), Some(k.to_string())),
        None => (args.entry.clone(), None),
    };
    let path = PathBuf::from(&path_text);
    let is_code = path.extension().is_some_and(|e| e == "code");
    if !is_code {
        if !args.params.is_empty() || args.kernel.is_some() || kernel_suffix.is_some() {
            bail!("--param and kernel names apply to Codira sources, not to circuit files");
        }
        return read_circuit_file(&path, &kernel_name_from(&path)).map(Some);
    }

    let config = Config {
        target: Target::host_target().ok_or_else(|| anyhow!("unable to determine host target"))?,
        optimization_lvl: codira_compiler::OptimizationLevel::None,
        out_dir: None,
        emit_ir: false,
        ..Config::default()
    };
    let (driver, _file) = Driver::with_file(config, PathOrInline::Path(path.clone()))
        .with_context(|| format!("could not read '{}'", path.display()))?;
    if driver.emit_diagnostics(&mut stderr(), DisplayColor::Auto)? {
        eprintln!(
            "error: '{}' has errors; nothing was elaborated",
            path.display()
        );
        return Ok(None);
    }

    let db = driver.db();
    let kernels = elaborate::find_kernels(db);
    let names: Vec<String> = kernels.iter().map(|k| k.name(db).to_string()).collect();
    let wanted = kernel_suffix.or_else(|| args.kernel.clone());
    let kernel = match wanted {
        Some(w) => kernels
            .iter()
            .zip(&names)
            .find(|(_, n)| **n == w)
            .map(|(k, _)| *k)
            .ok_or_else(|| {
                anyhow!(
                    "no `@target(qpu)` function `{w}` in '{}' (kernels: {})",
                    path.display(),
                    list(&names)
                )
            })?,
        None => match kernels.len() {
            0 => bail!("'{}' has no `@target(qpu)` function", path.display()),
            1 => kernels[0],
            _ => kernels
                .iter()
                .zip(&names)
                .find(|(_, n)| *n == "main")
                .map(|(k, _)| *k)
                .ok_or_else(|| {
                    anyhow!(
                        "'{}' has several kernels ({}); pick one with `{}::<kernel>` or --kernel",
                        path.display(),
                        list(&names),
                        path.display()
                    )
                })?,
        },
    };

    let params = args
        .params
        .iter()
        .map(|p| {
            p.split_once('=')
                .map(|(n, v)| (n.trim().to_string(), v.trim().to_string()))
                .ok_or_else(|| anyhow!("--param expects name=value, got `{p}`"))
        })
        .collect::<Result<Vec<_>, _>>()?;
    let options = ElabOptions {
        fuel: args.fuel,
        ..ElabOptions::default()
    };
    match elaborate::elaborate(db, kernel, &params, &options) {
        Ok(circuit) => Ok(Some(circuit)),
        Err(mut e) => {
            // The driver names a single file `mod.code`; report the real path.
            if let Some(loc) = &mut e.location {
                loc.file = path.display().to_string();
            }
            eprintln!("error: {e}");
            Ok(None)
        }
    }
}

fn list(names: &[String]) -> String {
    if names.is_empty() {
        "none".to_string()
    } else {
        names.join(", ")
    }
}

#[cfg(test)]
mod tests {
    use crate::{run_with_args, ExitStatus};

    fn codira(args: &[&str]) -> ExitStatus {
        run_with_args(std::iter::once("codira").chain(args.iter().copied())).unwrap()
    }

    #[test]
    fn build_import_and_run_round_trip_through_files() {
        let dir = tempfile::tempdir().unwrap();
        let source = dir.path().join("ghz.code");
        std::fs::write(
            &source,
            format!(
                "{}\n@target(qpu)\nfunc ghz(n: i64) {{\n    h(0);\n    let mut i = 1;\n    while i < n \
                 {{ cx(i - 1, i); i += 1; }}\n    let mut j = 0;\n    while j < n {{ measure(j, j); j \
                 += 1; }}\n}}\n",
                codira_quantum::elaborate::PRELUDE
            ),
        )
        .unwrap();
        let src = source.to_str().unwrap();

        let qasm = dir.path().join("ghz.qasm");
        assert_eq!(
            codira(&[
                "quantum",
                "build",
                src,
                "--param",
                "n=3",
                "--emit",
                "qasm2",
                "-o",
                qasm.to_str().unwrap()
            ]),
            ExitStatus::Success
        );
        let text = std::fs::read_to_string(&qasm).unwrap();
        assert!(text.contains("cx q[1], q[2];"), "{text}");

        let bc = dir.path().join("ghz.bc");
        assert_eq!(
            codira(&[
                "quantum",
                "build",
                src,
                "--param",
                "n=3",
                "--emit",
                "qir-bc",
                "-o",
                bc.to_str().unwrap()
            ]),
            ExitStatus::Success
        );
        let circuit =
            codira_quantum::qir::import_qir_bitcode(&std::fs::read(&bc).unwrap()).unwrap();
        assert_eq!(circuit.num_qubits(), 3);

        // The QASM file comes back as a kernel that elaborates to the same
        // distribution.
        let imported = dir.path().join("again.code");
        assert_eq!(
            codira(&[
                "quantum",
                "import",
                qasm.to_str().unwrap(),
                "-o",
                imported.to_str().unwrap()
            ]),
            ExitStatus::Success
        );
        assert!(std::fs::read_to_string(&imported)
            .unwrap()
            .contains("@target(qpu)"));
        assert_eq!(
            codira(&["quantum", "run", imported.to_str().unwrap(), "--exact"]),
            ExitStatus::Success
        );

        // A missing compile-time parameter is a reported failure, not a panic.
        assert_eq!(codira(&["quantum", "run", src]), ExitStatus::Error);
        // So is selecting a kernel that does not exist.
        assert!(run_with_args(["codira", "quantum", "run", &format!("{src}::nope")]).is_err());
    }
}
