//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - [`check`]: what a process runs before it admits an update. It uses this
//!   crate, `codira_certificate`'s query checker, `object` (through it, to read
//!   symbol tables) and Z3 -- nothing of the parser, type checker, verifier or
//!   code generator.
//!
//! # What a passing check establishes
//!
//! 1. the certificate is intact (`body_sha256`), is for these two libraries
//!    (whole-file SHA-256 of old and new), and carries the intent it hashes;
//! 2. every function entry's machine code, in each library, hashes to the
//!    recorded value and starts where the library's symbol table puts the
//!    symbol, so a changed instruction is pinned on the function it is in;
//! 3. coverage: every function of either version that the intent does not name
//!    has an equivalence obligation; every `equal` function has one; every
//!    `refines`/`replaces` function has its contract obligations; every
//!    function with an effect attribute has an effect obligation; no function
//!    exists in only one version without being named `added`/`removed`;
//! 4. each obligation's queries hash to the recorded values, are plain
//!    declarations and two assertions, and Z3, under the *checker's* rlimit,
//!    answers `unsat` for each; an obligation's `assumes` name obligations that
//!    are in the certificate and passed;
//! 5. every migration verdict, re-derived from the two field types and the
//!    intent here, matches the record, and its guard query is `unsat`;
//! 6. the affected set recomputed from the recorded facts is the recorded one;
//! 7. the certificate records no rejection.
//!
//! # What it does not establish
//!
//! * That the queries say what the programs do. The verification-condition
//!   generator (`codira_hir::update`) turned the source of both versions into
//!   them; the checker re-decides them and cannot see whether they capture the
//!   source, nor that the machine code implements the source, nor that the
//!   recorded facts about the call graph are true.
//! * That the intent is the right one: the certificate shows the update does
//!   what the intent says and nothing else the verifier models.
//! * Authenticity: nothing is signed.
//!
//! The trusted base of a passing check: Z3, the verification-condition
//! generator and fact extractor of the compiler that produced the certificate,
//! the SHA-256 binding of queries to bytes, and this crate.

use std::{collections::BTreeSet, time::Instant};

use codira_certificate::{
    locate_functions, lookup, recheck, sha256_hex, CodeRange, Query, Recheck, SymbolMap,
};

use crate::{
    affected::{self, AffectedSet},
    cert::{query_hash, Obligation, ObligationKind, UpdateCertificate, VersionRecord},
    intent::{Intent, Relation},
    migration::{self, FieldChange, MigrationOutcome, MigrationRecord},
};

/// How [`check`] runs.
#[derive(Debug, Clone)]
pub struct CheckOptions {
    /// The Z3 resource budget every query is re-decided under -- the
    /// checker's own, never the certificate's.
    pub rlimit: u32,
    /// Fail when a library has a function symbol the certificate does not
    /// list. Off by default: a library contains symbols the compiler
    /// generated (instances, glue) that no source function owns; they are
    /// reported as notes.
    pub strict_symbols: bool,
}

impl Default for CheckOptions {
    fn default() -> Self {
        CheckOptions {
            rlimit: codira_smt::DEFAULT_RLIMIT,
            strict_symbols: false,
        }
    }
}

/// How one obligation fared.
#[derive(Debug, Clone, PartialEq)]
pub struct ObligationResult {
    pub id: String,
    pub kind: ObligationKind,
    pub subject: String,
    pub level: u8,
    pub proven: bool,
    /// Vouched for by the compiler, not decided by Z3 here.
    pub attested: bool,
    /// Proven only because the premise is unsatisfiable.
    pub vacuous: bool,
    pub queries: usize,
    /// Time Z3 took to re-decide its queries.
    pub millis: f64,
}

/// What [`check`] found.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Report {
    /// Checks that failed. Any entry means the update must not be admitted.
    pub failures: Vec<String>,
    /// Obligations the certificate itself records as not proven.
    pub rejections: Vec<String>,
    /// Per obligation.
    pub results: Vec<ObligationResult>,
    /// Facts worth knowing that are neither failures nor proofs.
    pub notes: Vec<String>,
    /// Proven obligations whose premise is unsatisfiable.
    pub vacuous: Vec<String>,
    /// Obligations the certificate vouches for without a query (effect
    /// summaries): what the compiler claims, not what Z3 re-proved.
    pub attested: Vec<String>,
    pub functions: usize,
    /// Function entries whose machine code matched, old and new counted
    /// separately.
    pub code_checked: usize,
    pub obligations_proven: usize,
    pub queries_checked: usize,
    /// The highest level of the relational verifier any proof needed.
    pub level: u8,
    /// The functions that must not be active at the swap.
    pub affected: AffectedSet,
    /// How each changed field migrates; a `Guarded` entry is a check the
    /// runtime makes on the live heap before the swap.
    pub migrations: Vec<MigrationRecord>,
    /// Wall-clock time of the whole check.
    pub elapsed_ms: f64,
}

impl Report {
    /// Whether every check passed.
    pub fn passed(&self) -> bool {
        self.failures.is_empty()
    }
}

impl std::fmt::Display for Report {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for failure in &self.failures {
            writeln!(f, "  FAILED: {failure}")?;
        }
        for rejection in &self.rejections {
            writeln!(f, "  not proven: {rejection}")?;
        }
        for vacuous in &self.vacuous {
            writeln!(f, "  vacuous: {vacuous}")?;
        }
        for attested in &self.attested {
            writeln!(f, "  attested (not re-proved): {attested}")?;
        }
        for note in &self.notes {
            writeln!(f, "  note: {note}")?;
        }
        writeln!(
            f,
            "  {} function(s); code of {} matched; {} obligation(s) re-proven with {} Z3 \
             quer{}; relational level {}; {} function(s) in the affected set; {} migration(s)",
            self.functions,
            self.code_checked,
            self.obligations_proven,
            self.queries_checked,
            if self.queries_checked == 1 {
                "y"
            } else {
                "ies"
            },
            self.level,
            self.affected.functions.len(),
            self.migrations.len(),
        )?;
        write!(
            f,
            "  {}",
            if self.passed() {
                "update certificate holds"
            } else {
                "update certificate REJECTED"
            }
        )
    }
}

fn slice(bytes: &[u8], range: CodeRange) -> Option<&[u8]> {
    let start = usize::try_from(range.offset).ok()?;
    let len = usize::try_from(range.len).ok()?;
    bytes.get(start..start.checked_add(len)?)
}

fn symbols_of(bytes: &[u8], label: &str, report: &mut Report) -> SymbolMap {
    match locate_functions(bytes) {
        Ok(map) if !map.is_empty() => map,
        Ok(map) => {
            report.notes.push(format!(
                "the {label} library has no function symbols; code ranges are taken from the \
                 certificate"
            ));
            map
        }
        Err(error) => {
            report.notes.push(format!(
                "could not read the {label} library's symbol table ({error}); code ranges are \
                 taken from the certificate"
            ));
            SymbolMap::new()
        }
    }
}

fn check_version(
    name: &str,
    label: &str,
    version: &VersionRecord,
    bytes: &[u8],
    symbols: &SymbolMap,
    report: &mut Report,
) {
    let Some(code) = &version.code else {
        if let Some(note) = &version.note {
            report
                .notes
                .push(format!("`{name}` ({label}) has no code range: {note}"));
        }
        return;
    };
    match slice(bytes, code.range()) {
        None => report.failures.push(format!(
            "`{name}` ({label}): the certificate places its code at {}..{}, past the end of the \
             file",
            code.offset,
            code.offset + code.len
        )),
        Some(code_bytes) if sha256_hex(code_bytes) != code.sha256 => {
            report.failures.push(format!(
                "`{name}` ({label}): its machine code (bytes {}..{}) differs from the certified \
                 code",
                code.offset,
                code.offset + code.len
            ));
        }
        Some(_) => report.code_checked += 1,
    }
    if !symbols.is_empty() {
        match lookup(symbols, &version.symbol) {
            Some(ranges) if ranges.iter().any(|r| r.offset == code.offset) => {}
            Some(ranges) => report.failures.push(format!(
                "`{name}` ({label}): its symbol `{}` is at byte {} of the library, the \
                 certificate says {}",
                version.symbol, ranges[0].offset, code.offset
            )),
            None => report.failures.push(format!(
                "`{name}` ({label}): the library has no symbol `{}`",
                version.symbol
            )),
        }
    }
}

/// The obligation ids the intent and the two versions require.
fn required_ids(certificate: &UpdateCertificate) -> Vec<(String, String)> {
    let mut required = Vec::new();
    for entry in &certificate.functions {
        let exists_in_both = entry.old.is_some() && entry.new.is_some();
        match entry.relation {
            None | Some(Relation::Equal) if exists_in_both && !entry.external => required.push((
                format!("equivalence:{}", entry.name),
                format!(
                    "`{}` is unchanged or `equal` and needs an equivalence proof",
                    entry.name
                ),
            )),
            Some(Relation::Refines) => {
                for part in ["pre", "post", "body"] {
                    required.push((
                        format!("contract:{}:{part}", entry.name),
                        format!(
                            "`{}` refines its contract and needs its `{part}` proof",
                            entry.name
                        ),
                    ));
                }
            }
            Some(Relation::Replaces) => required.push((
                format!("contract:{}:body", entry.name),
                format!(
                    "`{}` replaces its contract and needs the body proof",
                    entry.name
                ),
            )),
            _ => {}
        }
        if !entry.effect_attrs.is_empty() && entry.new.is_some() {
            required.push((
                format!("effect:{}", entry.name),
                format!(
                    "`{}` carries {} and needs an effect proof",
                    entry.name,
                    entry.effect_attrs.join(", ")
                ),
            ));
        }
    }
    required
}

/// Checks `certificate` against the two libraries. See the module docs for
/// exactly what a passing report establishes.
pub fn check(
    certificate: &UpdateCertificate,
    old_artifact: &[u8],
    new_artifact: &[u8],
    options: &CheckOptions,
) -> Report {
    let started = Instant::now();
    let mut report = Report {
        functions: certificate.functions.len(),
        level: certificate.level,
        affected: certificate.affected.clone(),
        migrations: certificate.migrations.clone(),
        ..Report::default()
    };
    check_into(
        certificate,
        old_artifact,
        new_artifact,
        options,
        &mut report,
    );
    report.elapsed_ms = started.elapsed().as_secs_f64() * 1000.0;
    report
}

fn check_into(
    certificate: &UpdateCertificate,
    old_artifact: &[u8],
    new_artifact: &[u8],
    options: &CheckOptions,
    report: &mut Report,
) {
    // 0. The certificate itself.
    if certificate.format != crate::cert::FORMAT {
        report.failures.push(format!(
            "certificate format `{}` is not `{}`",
            certificate.format,
            crate::cert::FORMAT
        ));
        return;
    }
    if certificate.compute_body_hash() != certificate.body_sha256 {
        report.failures.push(
            "the certificate's body does not hash to its recorded value: it was edited after it \
             was written"
                .to_string(),
        );
    }
    let here = codira_smt::z3_version();
    if here != certificate.header.solver {
        report.notes.push(format!(
            "proved with `{}`, re-checked with `{here}`: a query that needed most of its budget \
             may answer unknown here",
            certificate.header.solver
        ));
    }

    // 1. The two libraries.
    for (label, record, bytes) in [
        ("old", &certificate.header.old, old_artifact),
        ("new", &certificate.header.new, new_artifact),
    ] {
        let actual = sha256_hex(bytes);
        if actual != record.sha256 || bytes.len() as u64 != record.size {
            report.failures.push(format!(
                "the {label} library is not the file this certificate was made for (SHA-256 \
                 {actual}, {} bytes; certificate: {}, {} bytes)",
                bytes.len(),
                record.sha256,
                record.size
            ));
        }
    }

    // 2. The intent.
    if sha256_hex(certificate.intent.as_bytes()) != certificate.header.intent_sha256 {
        report
            .failures
            .push("the embedded intent does not hash to the recorded intent hash".to_string());
    }
    let intent = match Intent::parse(&certificate.intent) {
        Ok(intent) => intent,
        Err(error) => {
            report.failures.push(error);
            return;
        }
    };
    for declared in &intent.functions {
        match certificate
            .functions
            .iter()
            .find(|f| f.name == declared.name)
        {
            None => report.failures.push(format!(
                "the intent names `{}`, which neither version has",
                declared.name
            )),
            Some(entry) if entry.relation != Some(declared.relation) => {
                report.failures.push(format!(
                    "the intent says `{}` is `{}`, the certificate treats it as {:?}",
                    declared.name, declared.relation, entry.relation
                ));
            }
            Some(entry) => {
                let ok = match declared.relation {
                    Relation::Added => entry.old.is_none() && entry.new.is_some(),
                    Relation::Removed => entry.old.is_some() && entry.new.is_none(),
                    _ => entry.old.is_some() && entry.new.is_some(),
                };
                if !ok {
                    report.failures.push(format!(
                        "`{}` is declared `{}` but exists in {} of the versions",
                        declared.name,
                        declared.relation,
                        match (entry.old.is_some(), entry.new.is_some()) {
                            (true, true) => "both",
                            (true, false) => "only the old one",
                            (false, true) => "only the new one",
                            (false, false) => "neither",
                        }
                    ));
                }
            }
        }
    }
    for entry in &certificate.functions {
        if entry.relation.is_some() != intent.function(&entry.name).is_some() {
            report.failures.push(format!(
                "`{}`: the certificate and the intent disagree on whether it is declared changed",
                entry.name
            ));
        }
        if entry.relation.is_none() && (entry.old.is_none() || entry.new.is_none()) {
            report.failures.push(format!(
                "`{}` exists in only the {} version and the intent does not declare it `{}`",
                entry.name,
                if entry.old.is_some() { "old" } else { "new" },
                if entry.old.is_some() {
                    "removed"
                } else {
                    "added"
                },
            ));
        }
    }

    // 3. Each function's code, in each library.
    let old_symbols = symbols_of(old_artifact, "old", report);
    let new_symbols = symbols_of(new_artifact, "new", report);
    for entry in &certificate.functions {
        if let Some(version) = &entry.old {
            check_version(
                &entry.name,
                "old",
                version,
                old_artifact,
                &old_symbols,
                report,
            );
        }
        if let Some(version) = &entry.new {
            check_version(
                &entry.name,
                "new",
                version,
                new_artifact,
                &new_symbols,
                report,
            );
        }
    }
    for (label, symbols, take) in [("old", &old_symbols, true), ("new", &new_symbols, false)] {
        let listed: BTreeSet<&str> = certificate
            .functions
            .iter()
            .filter_map(|f| if take { f.old.as_ref() } else { f.new.as_ref() })
            .map(|v| v.symbol.as_str())
            .collect();
        let mut unlisted: Vec<&str> = symbols
            .keys()
            .map(|k| k.strip_prefix('_').unwrap_or(k))
            .filter(|k| !listed.contains(k))
            .collect();
        unlisted.sort_unstable();
        unlisted.dedup();
        if !unlisted.is_empty() {
            let message = format!(
                "the {label} library has {} function symbol(s) no entry of the certificate \
                 accounts for (compiler-generated code, or code the update does not cover): {}",
                unlisted.len(),
                unlisted
                    .iter()
                    .take(8)
                    .copied()
                    .collect::<Vec<_>>()
                    .join(", ")
            );
            if options.strict_symbols {
                report.failures.push(message);
            } else {
                report.notes.push(message);
            }
        }
    }

    // 4. Coverage.
    for (id, why) in required_ids(certificate) {
        let rejected = certificate.rejections.iter().any(|r| {
            id.ends_with(&format!(":{}", r.subject)) || id.contains(&format!(":{}:", r.subject))
        });
        if certificate.obligation(&id).is_none() && !rejected {
            report
                .failures
                .push(format!("missing obligation `{id}`: {why}"));
        }
    }

    // 5. The rejections the certificate records.
    for rejection in &certificate.rejections {
        report.rejections.push(rejection.to_string());
    }
    if !certificate.rejections.is_empty() {
        report.failures.push(format!(
            "the certificate records {} obligation(s) it could not prove",
            certificate.rejections.len()
        ));
    }

    // 6. Every query, again.
    let mut passed_ids: BTreeSet<&str> = BTreeSet::new();
    let mut seen_ids: BTreeSet<&str> = BTreeSet::new();
    for obligation in &certificate.obligations {
        if !seen_ids.insert(&obligation.id) {
            report
                .failures
                .push(format!("obligation id `{}` appears twice", obligation.id));
        }
        let result = recheck_obligation(obligation, options.rlimit, report);
        if result.attested {
            passed_ids.insert(&obligation.id);
            report.attested.push(format!(
                "{} of `{}`: {}",
                obligation.kind, obligation.subject, obligation.what
            ));
        }
        if result.proven {
            passed_ids.insert(&obligation.id);
            report.obligations_proven += 1;
            if result.vacuous {
                report.vacuous.push(format!(
                    "{} of `{}` holds only because its premise is unsatisfiable",
                    obligation.kind, obligation.subject
                ));
            }
        }
        report.results.push(result);
    }
    for obligation in &certificate.obligations {
        for assumed in &obligation.assumes {
            if !passed_ids.contains(assumed.as_str()) {
                report.failures.push(format!(
                    "`{}` relies on `{assumed}`, which is not an obligation of this certificate \
                     that holds",
                    obligation.id
                ));
            }
        }
    }
    let level = certificate
        .obligations
        .iter()
        .filter(|o| o.kind == ObligationKind::Equivalence)
        .map(|o| o.level)
        .max()
        .unwrap_or(0);
    if level != certificate.level {
        report.failures.push(format!(
            "the certificate says level {} but its equivalence proofs need level {level}",
            certificate.level
        ));
    }

    // 7. Migrations.
    let changed_types: BTreeSet<String> = certificate.changed_types.iter().cloned().collect();
    for record in &certificate.migrations {
        if !changed_types.contains(&record.type_name) {
            report.failures.push(format!(
                "{}: its type is not among the changed types",
                record.subject()
            ));
        }
        if let Err(error) = migration::verify_record(record) {
            report.failures.push(error);
        }
        if let MigrationOutcome::Unsupported { reason } = &record.outcome {
            report
                .failures
                .push(format!("{}: {reason}", record.subject()));
        }
        if record.change == FieldChange::Removed && record.outcome == MigrationOutcome::Discards {
            let declared = intent
                .field(&record.type_name, &record.field)
                .is_some_and(|f| f.drop);
            if !declared {
                report.failures.push(format!(
                    "{}: the field is removed, its live values are lost, and the intent does \
                     not say `drop = true`",
                    record.subject()
                ));
            }
        }
        if record.change == FieldChange::Retyped {
            let expected_policy = intent
                .field(&record.type_name, &record.field)
                .map_or(crate::intent::MigrationPolicy::Reject, |f| f.migrate);
            if expected_policy != record.policy {
                report.failures.push(format!(
                    "{}: the intent's migration policy is `{expected_policy:?}`, the record \
                     says `{:?}`",
                    record.subject(),
                    record.policy
                ));
            }
            match &record.obligation {
                None => report.failures.push(format!(
                    "{}: a retyped field needs a migration obligation",
                    record.subject()
                )),
                Some(id) if !passed_ids.contains(id.as_str()) => {
                    report.failures.push(format!(
                        "{}: its migration obligation `{id}` is missing or does not hold",
                        record.subject()
                    ));
                }
                Some(_) => {}
            }
            // The checker's own query, not the record's.
            if let (Some(old), Some(new)) = (
                record.old_ty.as_deref().and_then(migration::IntType::parse),
                record.new_ty.as_deref().and_then(migration::IntType::parse),
            ) {
                let query = migration::obligation_query(old, new);
                match recheck(&query.smtlib2, options.rlimit) {
                    Ok(Recheck::Unsat { .. }) => report.queries_checked += 1,
                    other => report.failures.push(format!(
                        "{}: the checker's own migration query does not hold ({})",
                        record.subject(),
                        match other {
                            Ok(Recheck::Sat) => "satisfiable".to_string(),
                            Ok(Recheck::Unknown(why)) => why,
                            Err(error) => error,
                            Ok(Recheck::Unsat { .. }) => unreachable!(),
                        }
                    )),
                }
            }
        }
    }

    // 8. The affected set.
    let changed_functions: BTreeSet<String> = certificate
        .functions
        .iter()
        .filter(|f| matches!(f.relation, Some(r) if r != Relation::Added))
        .map(|f| f.name.clone())
        .collect();
    let types: BTreeSet<String> = certificate.changed_types.iter().cloned().collect();
    let recomputed = affected::compute(&certificate.facts, &changed_functions, &types);
    if recomputed != certificate.affected {
        report.failures.push(
            "the affected set in the certificate is not the one its own facts, intent and \
             changed types give"
                .to_string(),
        );
    }
    report.affected = recomputed;
}

fn recheck_obligation(
    obligation: &Obligation,
    rlimit: u32,
    report: &mut Report,
) -> ObligationResult {
    let what = format!("{} `{}`", obligation.kind, obligation.subject);
    let mut result = ObligationResult {
        id: obligation.id.clone(),
        kind: obligation.kind,
        subject: obligation.subject.clone(),
        level: obligation.level,
        proven: false,
        attested: false,
        vacuous: false,
        queries: obligation.queries.len(),
        millis: 0.0,
    };
    if obligation.attested {
        if obligation.queries.is_empty() {
            result.attested = true;
        } else {
            report.failures.push(format!(
                "{what} is attested but carries queries; an obligation is attested or proven, not both"
            ));
        }
        return result;
    }
    if obligation.queries.is_empty() {
        report.failures.push(format!(
            "{what} is recorded as proven but has no query to check"
        ));
        return result;
    }
    if obligation.query_sha256.len() != obligation.queries.len() {
        report.failures.push(format!(
            "{what}: {} queries but {} query hashes",
            obligation.queries.len(),
            obligation.query_sha256.len()
        ));
        return result;
    }
    let started = Instant::now();
    let mut holds = true;
    let mut vacuous = true;
    for (query, recorded) in obligation.queries.iter().zip(&obligation.query_sha256) {
        if query_hash(query) != *recorded {
            report.failures.push(format!(
                "{what}: a query does not hash to its recorded value: it was edited"
            ));
            holds = false;
            continue;
        }
        if query.expected != "unsat" {
            report.failures.push(format!(
                "{what} is recorded as proven, but one of its queries was `{}`",
                query.expected
            ));
            holds = false;
            continue;
        }
        report.queries_checked += 1;
        match check_query(query, rlimit) {
            Ok(v) => vacuous &= v,
            Err(error) => {
                report.failures.push(format!("{what}: {error}"));
                holds = false;
            }
        }
    }
    result.millis = started.elapsed().as_secs_f64() * 1000.0;
    result.proven = holds;
    result.vacuous = holds && vacuous;
    result
}

/// `Ok(vacuous)` when the query is `unsat`.
fn check_query(query: &Query, rlimit: u32) -> Result<bool, String> {
    match recheck(&query.smtlib2, rlimit) {
        Ok(Recheck::Unsat { vacuous }) => Ok(vacuous),
        Ok(Recheck::Sat) => Err("Z3 finds the query satisfiable -- it does not hold".to_string()),
        Ok(Recheck::Unknown(reason)) => Err(format!("Z3 could not re-prove it ({reason})")),
        Err(error) => Err(format!("malformed query: {error}")),
    }
}
