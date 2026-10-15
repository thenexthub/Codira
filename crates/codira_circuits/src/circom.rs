//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Writes an [`R1cs`] as a Circom 2 template: the same variables, the same
//!   witness computation (as `<--` assignments) and the same constraints (as
//!   `===`), so `circom` compiles it to the system Codira proves and its
//!   witness generator computes the same values.
//!
//! Circom's optimiser may then merge linear constraints, so circom's own
//! constraint count can be lower than [`R1cs::num_constraints`]; the
//! relation proven is the same.

use std::fmt::Write as _;

use ark_bn254::Fr;
use ark_ff::PrimeField;

use crate::r1cs::{Hint, Lc, R1cs, Var, ONE};

/// Words circom reserves; a Codira name equal to one gets a `_` suffix.
const KEYWORDS: &[&str] = &[
    "signal",
    "input",
    "output",
    "public",
    "template",
    "component",
    "var",
    "function",
    "return",
    "if",
    "else",
    "for",
    "while",
    "do",
    "log",
    "assert",
    "include",
    "pragma",
    "circom",
    "main",
    "parallel",
    "custom",
    "bus",
];

/// The name the public output signal gets.
pub const OUTPUT_SIGNAL: &str = "reveal";

/// A field element as a signed decimal, the shorter of `x` and `x - p`.
fn signed_decimal(x: Fr) -> String {
    if x.into_bigint() > Fr::MODULUS_MINUS_ONE_DIV_TWO {
        format!("-{}", -x)
    } else {
        x.to_string()
    }
}

struct Names {
    inputs: Vec<String>,
    output: String,
    first_internal: Var,
}

impl Names {
    fn of(&self, r1cs: &R1cs, v: Var) -> String {
        if v == r1cs.output_var {
            self.output.clone()
        } else if let Some(i) = r1cs.input_vars.iter().position(|x| *x == v) {
            self.inputs[i].clone()
        } else {
            format!("v[{}]", v - self.first_internal)
        }
    }

    fn lc(&self, r1cs: &R1cs, lc: &Lc) -> String {
        if lc.terms().is_empty() {
            return "0".to_string();
        }
        let mut out = String::new();
        for (i, (v, c)) in lc.terms().iter().enumerate() {
            let coeff = signed_decimal(*c);
            let (negative, magnitude) = match coeff.strip_prefix('-') {
                Some(m) => (true, m.to_string()),
                None => (false, coeff),
            };
            if i == 0 {
                if negative {
                    out.push('-');
                }
            } else {
                out.push_str(if negative { " - " } else { " + " });
            }
            if *v == ONE {
                out.push_str(&magnitude);
            } else if magnitude == "1" {
                out.push_str(&self.of(r1cs, *v));
            } else {
                let _ = write!(out, "{magnitude}*{}", self.of(r1cs, *v));
            }
        }
        out
    }
}

/// A circom identifier for a Codira name.
fn identifier(name: &str, taken: &[String]) -> String {
    let mut id = name.to_string();
    while KEYWORDS.contains(&id.as_str()) || id == "v" || id == "t" || taken.contains(&id) {
        id.push('_');
    }
    id
}

/// The Circom 2 source for `r1cs`: one template named after the function,
/// its inputs as private input signals and the revealed result as the public
/// output signal [`OUTPUT_SIGNAL`].
pub fn emit(r1cs: &R1cs) -> String {
    let mut inputs: Vec<String> = Vec::new();
    for port in &r1cs.inputs {
        let id = identifier(&port.name, &inputs);
        inputs.push(id);
    }
    let names = Names {
        output: identifier(OUTPUT_SIGNAL, &inputs),
        first_internal: r1cs.output_var + 1 + u32::try_from(inputs.len()).unwrap_or(0),
        inputs,
    };
    let internal = r1cs.num_vars - names.first_internal;
    let template = identifier(&r1cs.name, &[]);

    let mut out = String::new();
    out.push_str("pragma circom 2.0.0;\n\n");
    let _ = writeln!(
        out,
        "// `{}`, elaborated by Codira (`codira attest build`).",
        r1cs.signature
    );
    let _ = writeln!(
        out,
        "// Inputs are private. `{}` is public: the result's unsigned {}-bit encoding.",
        names.output,
        r1cs.output_ty.width()
    );
    let _ = writeln!(
        out,
        "// {} constraints over {} variables, exactly the system Codira proves.\n",
        r1cs.num_constraints(),
        r1cs.num_vars
    );
    let _ = writeln!(out, "template {template}() {{");
    for (name, port) in names.inputs.iter().zip(&r1cs.inputs) {
        let _ = writeln!(out, "    signal input {name}; // {}", port.ty);
    }
    let _ = writeln!(
        out,
        "    signal output {}; // {}",
        names.output, r1cs.output_ty
    );
    if internal > 0 {
        let _ = writeln!(out, "    signal v[{internal}];");
    }
    out.push_str("    var t;\n\n    // Witness computation.\n");
    for hint in &r1cs.hints {
        match hint {
            Hint::Bits {
                value,
                first,
                count,
            } => {
                let _ = writeln!(out, "    t = {};", names.lc(r1cs, value));
                let _ = writeln!(
                    out,
                    "    for (var i = 0; i < {count}; i++) {{ v[{} + i] <-- (t >> i) & 1; }}",
                    first - names.first_internal
                );
            }
            Hint::Quadratic { out: o, a, b, c } => {
                let target = names.of(r1cs, *o);
                if a.terms().is_empty() || b.terms().is_empty() {
                    let _ = writeln!(out, "    {target} <-- {};", names.lc(r1cs, c));
                } else {
                    let _ = writeln!(
                        out,
                        "    {target} <-- ({}) * ({}) + ({});",
                        names.lc(r1cs, a),
                        names.lc(r1cs, b),
                        names.lc(r1cs, c)
                    );
                }
            }
            Hint::Inverse { out: o, value } => {
                let _ = writeln!(out, "    t = {};", names.lc(r1cs, value));
                let _ = writeln!(out, "    {} <-- t != 0 ? 1 / t : 0;", names.of(r1cs, *o));
            }
        }
    }
    out.push_str("\n    // Constraints.\n");
    for k in &r1cs.constraints {
        let _ = writeln!(
            out,
            "    ({}) * ({}) === {};",
            names.lc(r1cs, &k.a),
            names.lc(r1cs, &k.b),
            names.lc(r1cs, &k.c)
        );
    }
    out.push_str("}\n\n");
    let _ = writeln!(out, "component main = {template}();");
    out
}
