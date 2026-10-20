//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 19, 2026
//!
//! Functionality:
//! - A source-level mutation generator for fault injection: flip a comparison,
//!   change a bound by one, change a constant, delete a guard or a contract
//!   clause, swap operands, add an allocation to an annotated hot path (with
//!   and without dropping the annotation), change a field type. Mutants are
//!   produced from the *new* version of a real update, so each one is a
//!   plausible faulty variant of that update.
//!
//! The generator works on text, line by line. Comments and string literals
//! are never mutated. It does not know whether a mutant compiles; the
//! campaign compiles each one and counts those the compiler rejects.

use std::collections::BTreeMap;

use crate::rng::Rng;

/// The mutation operators.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Class {
    /// `<` to `<=`, `==` to `!=`, ...
    FlipComparison,
    /// An integer literal plus or minus one.
    OffByOne,
    /// An integer literal replaced by 0, 1 or its double.
    ChangeConstant,
    /// A one-line guard `if .. { return .. }` removed.
    DeleteGuard,
    /// A `requires`, `ensures`, `invariant` or `decreases` clause removed.
    DeleteContract,
    /// `a - b` to `b - a`, or the two arguments of a call exchanged.
    SwapOperands,
    /// `+` to `-` or `-` to `+`.
    SwapArithmetic,
    /// An allocation added to a function annotated `@noalloc`/`@realtime`,
    /// annotation kept (the compiler should refuse).
    AllocKeepingAnnotation,
    /// The same, with the annotation removed (the compiler accepts).
    AllocDroppingAnnotation,
    /// A field declared `i64` becomes `i32`.
    ChangeFieldType,
}

impl Class {
    /// Name used in tables.
    pub fn name(self) -> &'static str {
        match self {
            Class::FlipComparison => "flip-comparison",
            Class::OffByOne => "off-by-one",
            Class::ChangeConstant => "change-constant",
            Class::DeleteGuard => "delete-guard",
            Class::DeleteContract => "delete-contract",
            Class::SwapOperands => "swap-operands",
            Class::SwapArithmetic => "swap-arithmetic",
            Class::AllocKeepingAnnotation => "alloc-keep-annotation",
            Class::AllocDroppingAnnotation => "alloc-drop-annotation",
            Class::ChangeFieldType => "change-field-type",
        }
    }

    /// All classes.
    pub const ALL: [Class; 10] = [
        Class::FlipComparison,
        Class::OffByOne,
        Class::ChangeConstant,
        Class::DeleteGuard,
        Class::DeleteContract,
        Class::SwapOperands,
        Class::SwapArithmetic,
        Class::AllocKeepingAnnotation,
        Class::AllocDroppingAnnotation,
        Class::ChangeFieldType,
    ];
}

/// One faulty variant of a version.
#[derive(Clone, Debug)]
pub struct Mutant {
    /// Operator.
    pub class: Class,
    /// File (path below the version directory).
    pub file: String,
    /// 1-based line of the change.
    pub line: usize,
    /// Enclosing function, or empty.
    pub function: String,
    /// Whether the line lies in a function the update changed.
    pub in_diff: bool,
    /// The original line.
    pub before: String,
    /// The replacement (may be several lines, or empty for a deletion).
    pub after: String,
    /// The complete new text of `file`.
    pub text: String,
}

/// Characters of `line` that are code: comments and strings blanked out with
/// spaces so that column positions stay valid.
fn code_only(line: &str) -> String {
    let mut out = String::with_capacity(line.len());
    let bytes: Vec<char> = line.chars().collect();
    let mut i = 0;
    let mut in_str = false;
    while i < bytes.len() {
        let c = bytes[i];
        if in_str {
            out.push(' ');
            if c == '\\' && i + 1 < bytes.len() {
                out.push(' ');
                i += 1;
            } else if c == '"' {
                in_str = false;
            }
        } else if c == '"' {
            in_str = true;
            out.push(' ');
        } else if c == '/' && i + 1 < bytes.len() && bytes[i + 1] == '/' {
            while i < bytes.len() {
                out.push(' ');
                i += 1;
            }
            break;
        } else {
            out.push(c);
        }
        i += 1;
    }
    out
}

/// A top-level function: name and the line range (0-based, inclusive start,
/// exclusive end) including its attributes.
struct Func {
    name: String,
    start: usize,
    body_start: usize,
    end: usize,
    attrs: Vec<usize>,
}

fn functions(lines: &[&str]) -> Vec<Func> {
    let mut starts = Vec::new();
    for (i, l) in lines.iter().enumerate() {
        let t = l
            .strip_prefix("public ")
            .or_else(|| l.strip_prefix("internal "))
            .unwrap_or(l);
        let is_item = ["func ", "struct ", "class ", "enum ", "extend ", "type "]
            .iter()
            .any(|k| t.starts_with(k));
        if is_item && !l.starts_with(' ') {
            let mut j = i;
            while j > 0 && (lines[j - 1].starts_with('@') || lines[j - 1].starts_with("//")) {
                j -= 1;
            }
            let name = t
                .split_whitespace()
                .nth(1)
                .unwrap_or("")
                .split(['(', '[', '{', ':'])
                .next()
                .unwrap_or("")
                .to_owned();
            starts.push((j, i, name, t.starts_with("func ")));
        }
    }
    let mut out = Vec::new();
    for (n, (j, i, name, is_func)) in starts.iter().enumerate() {
        if !is_func {
            continue;
        }
        let end = starts.get(n + 1).map_or(lines.len(), |s| s.0);
        let attrs = (*j..*i).filter(|k| lines[*k].starts_with('@')).collect();
        // The body starts at the first line that is exactly or ends with `{`
        // at or after the signature.
        let body_start = (*i..end)
            .find(|k| lines[*k].trim_end().ends_with('{'))
            .unwrap_or(*i);
        out.push(Func {
            name: name.clone(),
            start: *j,
            body_start,
            end,
            attrs,
        });
    }
    out
}

fn contains_word(s: &str, w: &str) -> bool {
    s.split(|c: char| !c.is_alphanumeric() && c != '_')
        .any(|t| t == w)
}

fn is_contract_line(t: &str) -> bool {
    ["requires ", "ensures ", "invariant ", "decreases "]
        .iter()
        .any(|k| t.trim_start().starts_with(k))
}

/// All comparison operator occurrences: (byte offset, length, replacement).
fn comparisons(code: &str) -> Vec<(usize, usize, &'static str)> {
    let b = code.as_bytes();
    let mut out = Vec::new();
    let mut i = 0;
    while i < b.len() {
        let two = code.get(i..i + 2).unwrap_or("");
        let prev = if i > 0 { b[i - 1] } else { b' ' };
        let next2 = b.get(i + 2).copied().unwrap_or(b' ');
        match two {
            "<=" if next2 != b'=' => {
                out.push((i, 2, "<"));
                i += 2;
                continue;
            }
            ">=" => {
                out.push((i, 2, ">"));
                i += 2;
                continue;
            }
            "==" => {
                out.push((i, 2, "!="));
                i += 2;
                continue;
            }
            "!=" => {
                out.push((i, 2, "=="));
                i += 2;
                continue;
            }
            "<<" | ">>" | "<-" | "->" | "|>" | "+=" | "-=" | "*=" | "/=" | "%=" => {
                i += 2;
                continue;
            }
            _ => {}
        }
        let c = b[i];
        let next = b.get(i + 1).copied().unwrap_or(b' ');
        if (c == b'<' || c == b'>') && prev == b' ' && next == b' ' {
            out.push((i, 1, if c == b'<' { "<=" } else { ">=" }));
        }
        i += 1;
    }
    out
}

/// Integer literals: (offset, length, value).
fn literals(code: &str) -> Vec<(usize, usize, i64)> {
    let b = code.as_bytes();
    let mut out = Vec::new();
    let mut i = 0;
    while i < b.len() {
        if b[i].is_ascii_digit()
            && (i == 0
                || !(b[i - 1].is_ascii_alphanumeric() || b[i - 1] == b'_' || b[i - 1] == b'.'))
        {
            let s = i;
            while i < b.len() && b[i].is_ascii_digit() {
                i += 1;
            }
            let after = b.get(i).copied().unwrap_or(b' ');
            if after == b'.' || after.is_ascii_alphabetic() || after == b'_' {
                continue;
            }
            if let Ok(v) = code[s..i].parse::<i64>() {
                out.push((s, i - s, v));
            }
        } else {
            i += 1;
        }
    }
    out
}

#[allow(clippy::too_many_arguments)]
fn push(
    out: &mut Vec<Mutant>,
    files: &BTreeMap<String, String>,
    file: &str,
    lines: &[&str],
    line: usize,
    class: Class,
    func: &str,
    in_diff: bool,
    after: String,
) {
    let mut v: Vec<String> = lines.iter().map(|s| (*s).to_owned()).collect();
    if after.is_empty() {
        v.remove(line);
    } else {
        v[line] = after.clone();
    }
    let mut text = v.join("\n");
    if files[file].ends_with('\n') && !text.ends_with('\n') {
        text.push('\n');
    }
    out.push(Mutant {
        class,
        file: file.to_owned(),
        line: line + 1,
        function: func.to_owned(),
        in_diff,
        before: lines[line].to_owned(),
        after,
        text,
    });
}

/// Every mutant the operators can make of `files` (only `.code` files that are
/// not the inspection module). `changed` names functions of the update as
/// `module.name`; mutants inside them are marked `in_diff`.
pub fn enumerate(files: &BTreeMap<String, String>, changed: &[String]) -> Vec<Mutant> {
    let mut out = Vec::new();
    for (file, text) in files {
        if !file.ends_with(".code") || file.ends_with("inspect.code") {
            continue;
        }
        let module = file
            .rsplit('/')
            .next()
            .unwrap_or(file)
            .trim_end_matches(".code")
            .to_owned();
        let lines: Vec<&str> = text.split('\n').collect();
        let funcs = functions(&lines);
        for f in &funcs {
            let in_diff = changed.iter().any(|c| *c == format!("{module}.{}", f.name));
            let annotated = f
                .attrs
                .iter()
                .any(|a| lines[*a].starts_with("@noalloc") || lines[*a].starts_with("@realtime"));
            for li in f.start..f.end {
                let raw = lines[li];
                let code = code_only(raw);
                if li < f.body_start && !is_contract_line(&code) {
                    continue;
                }
                let t = code.trim();
                if t.is_empty() {
                    continue;
                }
                if is_contract_line(&code) {
                    push(
                        &mut out,
                        files,
                        file,
                        &lines,
                        li,
                        Class::DeleteContract,
                        &f.name,
                        in_diff,
                        String::new(),
                    );
                    continue;
                }
                // Guards: `if .. { return .. }` on one line.
                if t.starts_with("if ")
                    && t.ends_with('}')
                    && contains_word(t, "return")
                    && !t.contains("else")
                {
                    push(
                        &mut out,
                        files,
                        file,
                        &lines,
                        li,
                        Class::DeleteGuard,
                        &f.name,
                        in_diff,
                        String::new(),
                    );
                }
                for (o, l, rep) in comparisons(&code) {
                    let after = format!("{}{}{}", &raw[..o], rep, &raw[o + l..]);
                    push(
                        &mut out,
                        files,
                        file,
                        &lines,
                        li,
                        Class::FlipComparison,
                        &f.name,
                        in_diff,
                        after,
                    );
                }
                for (o, l, v) in literals(&code) {
                    if t.starts_with("for ") && li == f.body_start {
                        continue;
                    }
                    for (class, nv) in [
                        (Class::OffByOne, v.saturating_add(1)),
                        (Class::OffByOne, v.saturating_sub(1)),
                        (Class::ChangeConstant, i64::from(v == 0)),
                        (Class::ChangeConstant, v.saturating_mul(2)),
                    ] {
                        if nv == v || nv < 0 {
                            continue;
                        }
                        let after = format!("{}{}{}", &raw[..o], nv, &raw[o + l..]);
                        push(
                            &mut out, files, file, &lines, li, class, &f.name, in_diff, after,
                        );
                    }
                }
                // Arithmetic swaps and operand swaps on ` - ` / ` + `.
                for (o, ch) in code.char_indices() {
                    if (ch == '+' || ch == '-')
                        && code[..o].ends_with(' ')
                        && code[o + 1..].starts_with(' ')
                        && !t.starts_with("//")
                    {
                        let rep = if ch == '+' { "-" } else { "+" };
                        let after = format!("{}{}{}", &raw[..o], rep, &raw[o + 1..]);
                        push(
                            &mut out,
                            files,
                            file,
                            &lines,
                            li,
                            Class::SwapArithmetic,
                            &f.name,
                            in_diff,
                            after,
                        );
                        if ch == '-' {
                            let left = code[..o].trim_end();
                            let right = code[o + 1..].trim_start();
                            let lt: String = left
                                .chars()
                                .rev()
                                .take_while(|c| c.is_alphanumeric() || *c == '_' || *c == '.')
                                .collect::<Vec<_>>()
                                .into_iter()
                                .rev()
                                .collect();
                            let rt: String = right
                                .chars()
                                .take_while(|c| c.is_alphanumeric() || *c == '_' || *c == '.')
                                .collect();
                            if !lt.is_empty() && !rt.is_empty() {
                                let lstart = left.len() - lt.len();
                                let rstart = o + 1 + (code[o + 1..].len() - right.len());
                                let after = format!(
                                    "{}{} - {}{}",
                                    &raw[..lstart],
                                    rt,
                                    lt,
                                    &raw[rstart + rt.len()..]
                                );
                                push(
                                    &mut out,
                                    files,
                                    file,
                                    &lines,
                                    li,
                                    Class::SwapOperands,
                                    &f.name,
                                    in_diff,
                                    after,
                                );
                            }
                        }
                    }
                }
                // Call argument swap: f(a, b) with two simple arguments.
                if let Some(open) = code.find('(') {
                    if let Some(close) = code[open..].find(')') {
                        let args = &code[open + 1..open + close];
                        let parts: Vec<&str> = args.split(',').collect();
                        let callee = code[..open].trim_end();
                        let is_call = callee
                            .chars()
                            .last()
                            .is_some_and(|c| c.is_alphanumeric() || c == '_')
                            && !t.starts_with("func ")
                            && !t.starts_with("public func")
                            && !t.starts_with("if ")
                            && !t.starts_with("while ");
                        if is_call
                            && parts.len() == 2
                            && parts
                                .iter()
                                .all(|p| !p.trim().is_empty() && !p.contains('('))
                            && parts[0].trim() != parts[1].trim()
                        {
                            let after = format!(
                                "{}({}, {}){}",
                                &raw[..open],
                                parts[1].trim(),
                                parts[0].trim(),
                                &raw[open + close + 1..]
                            );
                            push(
                                &mut out,
                                files,
                                file,
                                &lines,
                                li,
                                Class::SwapOperands,
                                &f.name,
                                in_diff,
                                after,
                            );
                        }
                    }
                }
                // Field type: `name: i64,` inside a type declaration.
            }
            if annotated {
                // Insert an allocation as the first statement of the body.
                let bs = f.body_start;
                let indent = "    ";
                if lines[bs].trim_end().ends_with('{') {
                    let alloc = format!("{indent}let scratch_ = ScratchBox {{ v: 0 }}");
                    let mut v: Vec<String> = lines.iter().map(|s| (*s).to_owned()).collect();
                    let decl = "class ScratchBox {\n    v: i64,\n}\n";
                    // keep annotation
                    let mut keep = v.clone();
                    keep.insert(bs + 1, alloc.clone());
                    let text = format!(
                        "{decl}
{}",
                        keep.join(
                            "
"
                        )
                    );
                    out.push(Mutant {
                        class: Class::AllocKeepingAnnotation,
                        file: file.clone(),
                        line: bs + 2,
                        function: f.name.clone(),
                        in_diff,
                        before: String::new(),
                        after: alloc.clone(),
                        text,
                    });
                    // drop annotation lines @noalloc / @realtime
                    let drop: Vec<usize> = f
                        .attrs
                        .iter()
                        .copied()
                        .filter(|a| {
                            lines[*a].starts_with("@noalloc") || lines[*a].starts_with("@realtime")
                        })
                        .collect();
                    v.insert(bs + 1, alloc.clone());
                    let kept: Vec<String> = v
                        .iter()
                        .enumerate()
                        .filter(|(i, _)| !drop.contains(i))
                        .map(|(_, s)| s.clone())
                        .collect();
                    out.push(Mutant {
                        class: Class::AllocDroppingAnnotation,
                        file: file.clone(),
                        line: bs + 2,
                        function: f.name.clone(),
                        in_diff,
                        before: String::new(),
                        after: alloc,
                        text: format!("{decl}\n{}", kept.join("\n")),
                    });
                }
            }
        }
        // Field types in type declarations.
        let mut in_type = false;
        for (li, raw) in lines.iter().enumerate() {
            if !raw.starts_with(' ') {
                in_type = ["struct ", "class ", "public struct ", "public class "]
                    .iter()
                    .any(|k| raw.starts_with(k));
            } else if in_type {
                let t = raw.trim();
                if t.ends_with(": i64,") || t.contains(": i64,") {
                    let after = raw.replacen(": i64,", ": i32,", 1);
                    let tname = (0..=li)
                        .rev()
                        .find(|k| !lines[*k].starts_with(' '))
                        .map(|k| lines[k].to_owned())
                        .unwrap_or_default();
                    let type_name = tname
                        .split_whitespace()
                        .find(|w| *w != "public" && *w != "struct" && *w != "class")
                        .unwrap_or("")
                        .to_owned();
                    let in_diff = changed
                        .iter()
                        .any(|c| c.ends_with(&format!(".{type_name}")));
                    push(
                        &mut out,
                        files,
                        file,
                        &lines,
                        li,
                        Class::ChangeFieldType,
                        &type_name,
                        in_diff,
                        after,
                    );
                }
            }
        }
    }
    out
}

/// Picks up to `n` mutants, balanced across classes, preferring (80%) lines
/// in functions the update changed. Deterministic given `seed`.
pub fn sample(all: &[Mutant], n: usize, seed: u64) -> Vec<Mutant> {
    let mut rng = Rng::new(seed);
    let mut by_class: BTreeMap<Class, Vec<&Mutant>> = BTreeMap::new();
    for m in all {
        by_class.entry(m.class).or_default().push(m);
    }
    for v in by_class.values_mut() {
        // in-diff first with 80% weight: shuffle each group separately.
        let (mut d, mut o): (Vec<&Mutant>, Vec<&Mutant>) = v.iter().partition(|m| m.in_diff);
        rng.shuffle(&mut d);
        rng.shuffle(&mut o);
        let mut merged = Vec::new();
        while !d.is_empty() || !o.is_empty() {
            let take_diff = !d.is_empty() && (o.is_empty() || rng.chance(80));
            merged.push(if take_diff { d.pop() } else { o.pop() }.expect("non-empty"));
        }
        *v = merged;
    }
    let mut out = Vec::new();
    let mut idx: BTreeMap<Class, usize> = BTreeMap::new();
    let mut classes: Vec<Class> = by_class.keys().copied().collect();
    while out.len() < n {
        rng.shuffle(&mut classes);
        let mut progressed = false;
        for c in &classes {
            if out.len() >= n {
                break;
            }
            let i = idx.entry(*c).or_default();
            if let Some(m) = by_class[c].get(*i) {
                out.push((*m).clone());
                *i += 1;
                progressed = true;
            }
        }
        if !progressed {
            break;
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn files(src: &str) -> BTreeMap<String, String> {
        BTreeMap::from([("src/t.code".to_owned(), src.to_owned())])
    }

    const SRC: &str = "// a < b in a comment\n@noalloc\npublic func f(a: i64, b: i64) -> i64\n    requires a >= 0\n{\n    if a > b { return 7 }\n    let s = \"x < y\"\n    a - b + 3\n}\n\npublic class C {\n    n: i64,\n}\n";

    #[test]
    fn every_class_is_produced_where_the_source_has_a_site() {
        let m = enumerate(&files(SRC), &["t.f".to_owned()]);
        let classes: std::collections::BTreeSet<Class> = m.iter().map(|m| m.class).collect();
        for c in Class::ALL {
            assert!(classes.contains(&c), "missing {}", c.name());
        }
    }

    #[test]
    fn comments_and_strings_are_left_alone() {
        let m = enumerate(&files(SRC), &[]);
        assert!(m.iter().all(|m| !m.after.contains("<=")
            || !m.before.contains("\"x < y\"")
            || m.after.contains("\"x < y\"")));
        assert!(m
            .iter()
            .all(|m| m.line != 1 && m.line != 7 || m.class == Class::SwapArithmetic));
    }

    #[test]
    fn a_comparison_flip_changes_exactly_one_operator() {
        let m = enumerate(&files(SRC), &[]);
        let flip = m
            .iter()
            .find(|m| m.class == Class::FlipComparison && m.line == 6)
            .expect("a flip on the guard");
        assert_eq!(flip.after, "    if a >= b { return 7 }");
        assert_eq!(flip.text.matches('\n').count(), SRC.matches('\n').count());
    }

    #[test]
    fn sampling_is_deterministic_and_class_balanced() {
        let m = enumerate(&files(SRC), &["t.f".to_owned()]);
        let a = sample(&m, 10, 5);
        let b = sample(&m, 10, 5);
        assert_eq!(
            a.iter()
                .map(|m| (m.line, m.after.clone()))
                .collect::<Vec<_>>(),
            b.iter()
                .map(|m| (m.line, m.after.clone()))
                .collect::<Vec<_>>()
        );
        let classes: std::collections::BTreeSet<Class> = a.iter().map(|m| m.class).collect();
        assert!(classes.len() >= 5);
    }

    #[test]
    fn a_real_workload_yields_hundreds_of_sites() {
        let root =
            std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../eval/workloads/kv/v05");
        let files = crate::manifest::read_tree(&root).expect("read v05");
        let all = enumerate(&files, &["table.find_slot".to_owned()]);
        assert!(all.len() > 100, "only {} sites", all.len());
    }
}
