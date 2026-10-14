//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - `codira synth`: fill every `??` hole and `choose` expression in the
//!   package with an expression proven to meet the contract, and write it back
//!   into the source file.
//!
//! By default the candidates come from the compiler's own CEGIS search
//! (`codira_hir::verify::synth`). With `--oracle "<command>"` they come from
//! an external program instead -- typically a language-model CLI -- which
//! receives the function, its contract and every counterexample so far on
//! stdin and answers with one Codira expression on stdout. The compiler
//! never trusts the answer: each candidate is spliced into the source and the
//! whole function is re-verified, and only a candidate with every proof
//! obligation proven is kept. A rejected candidate's counterexample is fed
//! back for the next attempt, up to `--attempts`. The compiler itself opens
//! no network connection; whatever the oracle does is its own business.

use std::{
    io::Write,
    path::PathBuf,
    process::{Command, Stdio},
};

use anyhow::anyhow;
use codira_compiler::{Config, Driver, RelativePathBuf, Target};
use codira_hir::{
    verify::{synth, Verdict},
    Function, HirDatabase, Package,
};
use codira_hir_input::SourceDatabase;

use crate::{ops::build::locate_manifest, ExitStatus};

#[derive(clap::Args)]
pub struct Args {
    /// Path to the manifest of the project
    #[clap(long)]
    manifest_path: Option<PathBuf>,

    /// Only fill holes in the function with this name
    #[clap(long)]
    function: Option<String>,

    /// Ask this shell command for candidates instead of searching: it gets
    /// the function, its contract and the counterexamples so far on stdin,
    /// and must print one Codira expression
    #[clap(long)]
    oracle: Option<String>,

    /// How many oracle candidates to try per hole before giving up
    #[clap(long, default_value_t = 3)]
    attempts: usize,

    /// Print the fills instead of writing them into the source
    #[clap(long)]
    dry_run: bool,
}

pub fn synth(args: Args) -> anyhow::Result<ExitStatus> {
    let manifest = locate_manifest(args.manifest_path.as_deref())?;
    let package = codira_project::Package::from_file(&manifest)?;
    let source_dir = package.source_directory();
    let config = Config {
        target: Target::host_target().expect("unable to determine host target"),
        optimization_lvl: codira_compiler::OptimizationLevel::None,
        out_dir: None,
        emit_ir: false,
        mode: codira_compiler::BuildMode::default(),
    };
    let (_, mut driver) = Driver::with_package_path(&manifest, config)?;

    let targets = functions_with_holes(driver.db(), args.function.as_deref());
    if targets.is_empty() {
        println!("no `??` or `choose` holes to fill");
        return Ok(ExitStatus::Success);
    }

    let mut all_filled = true;
    for (name, file) in targets {
        let outcome = match &args.oracle {
            Some(command) => fill_with_oracle(&mut driver, &name, &file, command, args.attempts),
            None => fill_with_search(&driver, &name, &file),
        };
        match outcome {
            Ok(fills) => {
                for fill in &fills {
                    println!("filled a hole in `{name}` with {}", fill.text);
                }
                let path = source_dir.join(file.as_str());
                let text = driver
                    .db()
                    .file_text(file_id_of(&driver, &file)?)
                    .to_string();
                let rewritten = apply(&text, &fills);
                if args.dry_run {
                    println!("{rewritten}");
                } else {
                    std::fs::write(&path, &rewritten)?;
                }
                // Later targets are located in the driver's copy of the
                // file, so keep it in step with what was written.
                driver.set_file_text(file.as_str(), &rewritten)?;
            }
            Err(reason) => {
                all_filled = false;
                println!("could not fill the holes in `{name}`: {reason}");
            }
        }
    }
    Ok(ExitStatus::from(all_filled))
}

/// Every function with a hole, as `(name, file relative to the source
/// directory)`, optionally only the one called `only`.
fn functions_with_holes(
    db: &dyn HirDatabase,
    only: Option<&str>,
) -> Vec<(String, RelativePathBuf)> {
    let mut out = Vec::new();
    for package in Package::all(db) {
        for module in package.modules(db) {
            for function in module.all_functions(db) {
                let name = function.name(db).to_string();
                if only.is_some_and(|only| only != name) {
                    continue;
                }
                if !synth::holes(db, function).is_empty() {
                    out.push((name, db.file_relative_path(function.file_id(db))));
                }
            }
        }
    }
    out
}

/// The function called `name` in `file`, in the driver's current revision.
fn find(db: &dyn HirDatabase, name: &str, file: &RelativePathBuf) -> Option<Function> {
    Package::all(db)
        .into_iter()
        .flat_map(|package| package.modules(db))
        .flat_map(|module| module.all_functions(db))
        .find(|f| f.name(db).to_string() == name && &db.file_relative_path(f.file_id(db)) == file)
}

fn file_id_of(driver: &Driver, file: &RelativePathBuf) -> anyhow::Result<codira_compiler::FileId> {
    driver
        .get_file_id_for_path(file)
        .ok_or_else(|| anyhow!("`{file}` is not part of the package"))
}

/// `text` with each fill's range replaced by its text.
fn apply(text: &str, fills: &[synth::Fill]) -> String {
    let mut fills: Vec<&synth::Fill> = fills.iter().collect();
    fills.sort_by_key(|f| std::cmp::Reverse(f.range.start()));
    let mut out = text.to_string();
    for fill in fills {
        let range = std::ops::Range::<usize>::from(fill.range);
        out.replace_range(range, &fill.text);
    }
    out
}

fn fill_with_search(
    driver: &Driver,
    name: &str,
    file: &RelativePathBuf,
) -> Result<Vec<synth::Fill>, String> {
    let db = driver.db();
    let function = find(db, name, file).ok_or("the function disappeared")?;
    synth::synthesize(db, function)
}

/// Asks `command` for a candidate for each hole of the function, verifying
/// every answer. Only a single-hole function is handed to an oracle: one
/// answer, one place to put it.
fn fill_with_oracle(
    driver: &mut Driver,
    name: &str,
    file: &RelativePathBuf,
    command: &str,
    attempts: usize,
) -> Result<Vec<synth::Fill>, String> {
    let function = find(driver.db(), name, file).ok_or("the function disappeared")?;
    let holes = synth::holes(driver.db(), function);
    let [(_, range)] = holes.as_slice() else {
        return Err(format!(
            "an oracle fills one hole per function; `{name}` has {}",
            holes.len()
        ));
    };
    let range = *range;
    let original = driver
        .db()
        .file_text(function.file_id(driver.db()))
        .to_string();
    let function_text = synth::function_text(driver.db(), function);

    let mut feedback: Vec<String> = Vec::new();
    for attempt in 1..=attempts {
        let prompt = prompt(&function_text, &feedback);
        let answer = run_oracle(command, &prompt).map_err(|e| e.to_string())?;
        let candidate = clean_answer(&answer);
        if candidate.is_empty() {
            feedback.push(format!("attempt {attempt}: the answer was empty"));
            continue;
        }
        let fill = synth::Fill {
            range,
            text: format!("({candidate})"),
        };
        let spliced = apply(&original, std::slice::from_ref(&fill));
        driver
            .set_file_text(file.as_str(), &spliced)
            .map_err(|e| e.to_string())?;
        let verdict = judge(driver, name, file);
        // Leave the driver as it was; the caller writes the accepted fill.
        driver
            .set_file_text(file.as_str(), &original)
            .map_err(|e| e.to_string())?;
        match verdict {
            Ok(()) => {
                println!("oracle candidate {attempt} for `{name}` is proven: {candidate}");
                return Ok(vec![fill]);
            }
            Err(reason) => {
                println!("oracle candidate {attempt} for `{name}` rejected: {reason}");
                feedback.push(format!("`{candidate}` was rejected: {reason}"));
            }
        }
    }
    Err(format!(
        "no oracle candidate was proven in {attempts} attempt(s)"
    ))
}

/// Accepts a spliced candidate only if the file parses, the function
/// type-checks, and every proof obligation is proven.
fn judge(driver: &Driver, name: &str, file: &RelativePathBuf) -> Result<(), String> {
    let db = driver.db();
    let function =
        find(db, name, file).ok_or("the candidate does not parse as part of the function")?;
    let parse = codira_hir::AstDatabase::parse(db, function.file_id(db));
    if let Some(error) = parse.errors().first() {
        return Err(format!("it does not parse: {error}"));
    }
    let mut errors = Vec::new();
    function.diagnostics(
        db,
        &mut codira_hir::DiagnosticSink::new(|d| errors.push(d.message())),
    );
    let report = db.verify_function(function);
    if let Some(failure) = report.failures().next() {
        let detail = match &failure.verdict {
            Verdict::Refuted(cex) => format!("{} on {}", failure.message(), cex.call(name)),
            _ => failure.message(),
        };
        return Err(detail);
    }
    if let Some(error) = errors.first() {
        return Err(error.clone());
    }
    Ok(())
}

fn prompt(function_text: &str, feedback: &[String]) -> String {
    let mut out = String::from(
        "Replace the hole (`??` or the `choose ... where ...` expression) in this Codira \
         function with a single Codira expression that makes every `requires`/`ensures` \
         clause hold. Integers wrap at their width. Answer with the expression only.\n\n",
    );
    out.push_str(function_text);
    out.push('\n');
    if !feedback.is_empty() {
        out.push_str("\nEarlier answers that the verifier rejected:\n");
        for line in feedback {
            out.push_str("- ");
            out.push_str(line);
            out.push('\n');
        }
    }
    out
}

/// The first non-empty line of an answer, without Markdown code fences.
fn clean_answer(answer: &str) -> String {
    answer
        .lines()
        .map(str::trim)
        .find(|line| !line.is_empty() && !line.starts_with("```"))
        .unwrap_or("")
        .trim_end_matches(';')
        .to_string()
}

fn run_oracle(command: &str, prompt: &str) -> anyhow::Result<String> {
    let mut child = if cfg!(windows) {
        Command::new("cmd").args(["/C", command]).spawn_piped()?
    } else {
        Command::new("sh").args(["-c", command]).spawn_piped()?
    };
    if let Some(mut stdin) = child.stdin.take() {
        // An oracle that exits without reading its input is not an error.
        let _ = stdin.write_all(prompt.as_bytes());
    }
    let output = child.wait_with_output()?;
    if !output.status.success() {
        return Err(anyhow!(
            "the oracle exited with {}: {}",
            output.status,
            String::from_utf8_lossy(&output.stderr).trim()
        ));
    }
    Ok(String::from_utf8_lossy(&output.stdout).into_owned())
}

trait SpawnPiped {
    fn spawn_piped(&mut self) -> std::io::Result<std::process::Child>;
}

impl SpawnPiped for Command {
    fn spawn_piped(&mut self) -> std::io::Result<std::process::Child> {
        self.stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn()
    }
}
