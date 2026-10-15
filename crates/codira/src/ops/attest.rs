//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - `codira attest build|prove|verify`: zero-knowledge proofs that an
//!   `oblivious` Codira function, run on private inputs, returns a public value
//!   -- the roadmap's `attest f(x, y) reveal r`.
//!
//! `build` elaborates the function into an R1CS (`codira_circuits`), writes
//! it as an iden3 `.r1cs` file and as Circom source, and runs the Groth16
//! setup (proving and verifying keys). `prove` evaluates the function on the
//! given inputs inside the circuit and proves the result, which it reveals;
//! the inputs stay private. `verify` needs only the verifying key and the
//! proof: no source and no inputs.

use std::{
    io::stderr,
    path::{Path, PathBuf},
};

use anyhow::{anyhow, bail, Context as _};
use codira_circuits::{
    circom, elaborate, groth16,
    r1cs::{self, R1cs},
    ElabOptions, Mode, Ty,
};
use codira_compiler::{Config, DisplayColor, Driver, PathOrInline, Target};
use serde_json::{json, Value};

use crate::ExitStatus;

#[derive(clap::Args)]
pub struct Args {
    #[clap(subcommand)]
    command: Command,
}

#[derive(clap::Subcommand)]
enum Command {
    /// Compile an `oblivious` function to a circuit (`.r1cs`, `.circom`)
    /// and generate its proving and verifying keys
    Build(BuildArgs),
    /// Run the function on private inputs and prove the revealed result
    Prove(ProveArgs),
    /// Check a proof against a verifying key and a revealed value
    Verify(VerifyArgs),
}

#[derive(clap::Args)]
struct BuildArgs {
    /// A `.code` file, optionally `file.code::function`
    entry: String,

    /// Where to write `<function>.r1cs`, `.circom`, `.pk` and `.vk`
    /// (default: the current directory)
    #[clap(long)]
    out_dir: Option<PathBuf>,
}

#[derive(clap::Args)]
struct ProveArgs {
    /// A `.code` file, optionally `file.code::function`
    entry: String,

    /// A private input, `name=value` (one per parameter)
    #[clap(long = "input", value_name = "NAME=VALUE", allow_hyphen_values = true)]
    inputs: Vec<String>,

    /// The proving key from `codira attest build` (default:
    /// `<function>.pk`)
    #[clap(long)]
    pk: Option<PathBuf>,

    /// Where to write the proof (default: `<function>.proof`)
    #[clap(short = 'o', long)]
    output: Option<PathBuf>,

    /// Also write the full witness in iden3's `.wtns` format (it contains
    /// the private inputs: keep it private)
    #[clap(long)]
    wtns: Option<PathBuf>,
}

#[derive(clap::Args)]
struct VerifyArgs {
    /// The verifying key from `codira attest build`
    #[clap(long)]
    vk: PathBuf,

    /// The proof from `codira attest prove`
    #[clap(long)]
    proof: PathBuf,

    /// The value the proof is checked to reveal (default: the value recorded
    /// in the proof file)
    #[clap(long, allow_hyphen_values = true)]
    reveal: Option<String>,
}

pub fn attest(args: Args) -> Result<ExitStatus, anyhow::Error> {
    match args.command {
        Command::Build(args) => build(&args),
        Command::Prove(args) => prove(&args),
        Command::Verify(args) => verify(&args),
    }
}

/// FNV-1a over the system's iden3 encoding: ties keys and proofs to the
/// circuit they were made for (a consistency check, not a security
/// mechanism -- the verifying key itself binds the circuit).
fn digest(r1cs: &R1cs) -> String {
    let mut hash = 0xcbf2_9ce4_8422_2325u64;
    for byte in r1cs.to_iden3_r1cs() {
        hash ^= u64::from(byte);
        hash = hash.wrapping_mul(0x0000_0100_0000_01b3);
    }
    format!("{hash:016x}")
}

fn hex(bytes: &[u8]) -> String {
    use std::fmt::Write as _;
    bytes.iter().fold(String::new(), |mut s, b| {
        let _ = write!(s, "{b:02x}");
        s
    })
}

fn unhex(text: &str) -> Result<Vec<u8>, anyhow::Error> {
    if !text.len().is_multiple_of(2) {
        bail!("odd-length hex string");
    }
    (0..text.len())
        .step_by(2)
        .map(|i| u8::from_str_radix(&text[i..i + 2], 16).map_err(|e| anyhow!("bad hex: {e}")))
        .collect()
}

fn ty_from_name(name: &str) -> Option<Ty> {
    if name == "bool" {
        return Some(Ty::Bool);
    }
    let signed = match name.as_bytes().first()? {
        b'i' => true,
        b'u' => false,
        _ => return None,
    };
    let width: u32 = name[1..].parse().ok()?;
    matches!(width, 8 | 16 | 32 | 64).then_some(Ty::Int { width, signed })
}

/// A value of type `ty` as Codira prints it.
fn show(ty: Ty, bits: u64) -> String {
    match ty {
        Ty::Bool => (bits & 1 == 1).to_string(),
        Ty::Int { .. } => ty.to_i128(bits).to_string(),
    }
}

/// Elaborates and lowers the entry's oblivious function. `None` means the
/// source had errors, already printed.
fn load(entry: &str) -> Result<Option<R1cs>, anyhow::Error> {
    let (path_text, wanted) = match entry.rsplit_once("::") {
        Some((p, f)) => (p.to_string(), Some(f.to_string())),
        None => (entry.to_string(), None),
    };
    let path = PathBuf::from(&path_text);
    let config = Config {
        target: Target::host_target().ok_or_else(|| anyhow!("unable to determine host target"))?,
        optimization_lvl: codira_compiler::OptimizationLevel::None,
        out_dir: None,
        emit_ir: false,
        ..Config::default()
    };
    let (driver, _file) = Driver::with_file(config, PathOrInline::Path(path.clone()))
        .with_context(|| format!("could not read '{}'", path.display()))?;
    // The diagnostics include the information-flow checker: an `oblivious`
    // function that leaks or branches on a secret stops here.
    if driver.emit_diagnostics(&mut stderr(), DisplayColor::Auto)? {
        eprintln!(
            "error: '{}' has errors; nothing was attested",
            path.display()
        );
        return Ok(None);
    }
    let db = driver.db();
    let candidates = elaborate::find_oblivious(db);
    let names: Vec<String> = candidates.iter().map(|f| f.name(db).to_string()).collect();
    let func = match wanted {
        Some(w) => candidates
            .iter()
            .zip(&names)
            .find(|(_, n)| **n == w)
            .map(|(f, _)| *f)
            .ok_or_else(|| {
                anyhow!(
                    "no `oblivious func {w}` in '{}' (oblivious functions: {})",
                    path.display(),
                    if names.is_empty() {
                        "none".to_string()
                    } else {
                        names.join(", ")
                    }
                )
            })?,
        None => match candidates.as_slice() {
            [] => bail!(
                "'{}' has no `oblivious func`; only oblivious functions can be attested",
                path.display()
            ),
            [only] => *only,
            _ => bail!(
                "'{}' has several oblivious functions ({}); pick one with `{}::<function>`",
                path.display(),
                names.join(", "),
                path.display()
            ),
        },
    };
    match codira_circuits::elaborate(db, func, Mode::Attest, &ElabOptions::default()) {
        Ok(circuit) => Ok(Some(r1cs::lower(&circuit))),
        Err(mut e) => {
            if let Some(loc) = &mut e.location {
                loc.file = path.display().to_string();
            }
            eprintln!("error: {e}");
            Ok(None)
        }
    }
}

fn write(path: &Path, bytes: impl AsRef<[u8]>) -> Result<(), anyhow::Error> {
    std::fs::write(path, bytes).with_context(|| format!("could not write '{}'", path.display()))
}

fn read(path: &Path) -> Result<Vec<u8>, anyhow::Error> {
    std::fs::read(path).with_context(|| format!("could not read '{}'", path.display()))
}

fn build(args: &BuildArgs) -> Result<ExitStatus, anyhow::Error> {
    let Some(r1cs) = load(&args.entry)? else {
        return Ok(ExitStatus::Error);
    };
    let dir = args.out_dir.clone().unwrap_or_else(|| PathBuf::from("."));
    std::fs::create_dir_all(&dir)
        .with_context(|| format!("could not create '{}'", dir.display()))?;
    let base = dir.join(&r1cs.name);

    let mut rng = groth16::os_rng()?;
    let (pk, vk) = groth16::setup(&r1cs, &mut rng)?;
    let digest = digest(&r1cs);

    write(&base.with_extension("r1cs"), r1cs.to_iden3_r1cs())?;
    write(&base.with_extension("circom"), circom::emit(&r1cs))?;
    let mut pk_bytes = digest.clone().into_bytes();
    pk_bytes.extend(groth16::to_bytes(&pk));
    write(&base.with_extension("pk"), pk_bytes)?;
    let vk_json = json!({
        "function": r1cs.name,
        "signature": r1cs.signature,
        "output_type": r1cs.output_ty.to_string(),
        "circuit": digest,
        "vk": hex(&groth16::to_bytes(&vk)),
    });
    write(
        &base.with_extension("vk"),
        serde_json::to_string_pretty(&vk_json)? + "\n",
    )?;
    println!(
        "{}: {} constraints over {} variables",
        r1cs.signature,
        r1cs.num_constraints(),
        r1cs.num_vars
    );
    println!(
        "wrote {0}.r1cs, {0}.circom, {0}.pk (proving key) and {0}.vk (verifying key)",
        base.display()
    );
    Ok(ExitStatus::Success)
}

fn prove(args: &ProveArgs) -> Result<ExitStatus, anyhow::Error> {
    let Some(r1cs) = load(&args.entry)? else {
        return Ok(ExitStatus::Error);
    };
    let mut values = Vec::with_capacity(r1cs.inputs.len());
    for port in &r1cs.inputs {
        let given: Vec<&str> = args
            .inputs
            .iter()
            .filter_map(|i| i.split_once('='))
            .filter(|(n, _)| n.trim() == port.name)
            .map(|(_, v)| v.trim())
            .collect();
        let [text] = given.as_slice() else {
            bail!(
                "`{}` needs exactly one --input {}=<{}>",
                r1cs.name,
                port.name,
                port.ty
            );
        };
        let value = port
            .ty
            .parse(text)
            .ok_or_else(|| anyhow!("`{text}` is not a `{}` (input `{}`)", port.ty, port.name))?;
        values.push(value);
    }
    if let Some(extra) = args.inputs.iter().find(|i| {
        i.split_once('=')
            .is_none_or(|(n, _)| !r1cs.inputs.iter().any(|p| p.name == n.trim()))
    }) {
        bail!(
            "`{}` has no parameter for --input {extra} (parameters: {})",
            r1cs.name,
            r1cs.inputs
                .iter()
                .map(|p| p.name.as_str())
                .collect::<Vec<_>>()
                .join(", ")
        );
    }

    let pk_path = args
        .pk
        .clone()
        .unwrap_or_else(|| PathBuf::from(format!("{}.pk", r1cs.name)));
    let pk_bytes = read(&pk_path)?;
    let digest = digest(&r1cs);
    if pk_bytes.len() < 16 || pk_bytes[..16] != *digest.as_bytes() {
        bail!(
            "'{}' was made for a different circuit; run `codira attest build` again",
            pk_path.display()
        );
    }
    let pk: groth16::ProvingKey = groth16::from_bytes(&pk_bytes[16..])?;

    let witness = r1cs.witness(&values)?;
    let mut rng = groth16::os_rng()?;
    let proof = groth16::prove(&r1cs, &pk, &witness, &mut rng)?;
    let reveal = show(r1cs.output_ty, r1cs.output(&witness));

    let output = args
        .output
        .clone()
        .unwrap_or_else(|| PathBuf::from(format!("{}.proof", r1cs.name)));
    let proof_json = json!({
        "function": r1cs.name,
        "circuit": digest,
        "reveal": reveal,
        "proof": hex(&groth16::to_bytes(&proof)),
    });
    write(&output, serde_json::to_string_pretty(&proof_json)? + "\n")?;
    if let Some(path) = &args.wtns {
        write(path, R1cs::to_iden3_wtns(&witness))?;
    }
    println!("{} reveals {reveal}", r1cs.name);
    println!("wrote {}", output.display());
    Ok(ExitStatus::Success)
}

fn field<'a>(json: &'a Value, key: &str, file: &Path) -> Result<&'a str, anyhow::Error> {
    json.get(key)
        .and_then(Value::as_str)
        .ok_or_else(|| anyhow!("'{}' has no `{key}`", file.display()))
}

fn verify(args: &VerifyArgs) -> Result<ExitStatus, anyhow::Error> {
    let vk_json: Value = serde_json::from_slice(&read(&args.vk)?)
        .with_context(|| format!("'{}' is not a verifying key", args.vk.display()))?;
    let proof_json: Value = serde_json::from_slice(&read(&args.proof)?)
        .with_context(|| format!("'{}' is not a proof", args.proof.display()))?;

    let name = field(&vk_json, "function", &args.vk)?;
    let ty_name = field(&vk_json, "output_type", &args.vk)?;
    let ty = ty_from_name(ty_name).ok_or_else(|| anyhow!("unknown output type `{ty_name}`"))?;
    if field(&vk_json, "circuit", &args.vk)? != field(&proof_json, "circuit", &args.proof)? {
        println!("INVALID: the proof was made for a different circuit than this key");
        return Ok(ExitStatus::Error);
    }
    let vk: groth16::VerifyingKey = groth16::from_bytes(&unhex(field(&vk_json, "vk", &args.vk)?)?)?;
    let proof: groth16::Proof =
        match groth16::from_bytes(&unhex(field(&proof_json, "proof", &args.proof)?)?) {
            Ok(proof) => proof,
            Err(e) => {
                println!("INVALID: the proof does not decode ({e})");
                return Ok(ExitStatus::Error);
            }
        };
    let reveal = match &args.reveal {
        Some(r) => r.clone(),
        None => field(&proof_json, "reveal", &args.proof)?.to_string(),
    };
    let bits = ty
        .parse(&reveal)
        .ok_or_else(|| anyhow!("`{reveal}` is not a `{ty}`"))?;
    let public = [r1cs::Fr::from(bits)];
    if groth16::verify(&vk, &public, &proof)? {
        println!("valid: `{name}` returned {reveal} on inputs the prover knows");
        Ok(ExitStatus::Success)
    } else {
        println!("INVALID: the proof does not show that `{name}` returned {reveal}");
        Ok(ExitStatus::Error)
    }
}
