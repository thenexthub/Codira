//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Writes a [`Circuit`] as one synthesizable Verilog-2001 module: an input
//!   port per parameter, an output port `result`, and one continuous assignment
//!   per live node.
//!
//! # Exactness
//!
//! Verilog sizes an expression by its operands and the assignment's target,
//! and treats it as unsigned unless every operand is signed. To leave no
//! room for those rules to change a value, every node gets its own wire of
//! exactly its type's width, every operator combines operands of that same
//! width, and every net is declared unsigned:
//!
//! * `+ - * & | ^ ~` and unary `-` on equal widths give the low `w` bits;
//! * `<` on two unsigned nets is the unsigned comparison; the signed one is
//!   written `$signed(a) < $signed(b)`;
//! * `<<` and `>>` shift in zeros and `$signed(a) >>> b` copies the sign bit;
//!   the amount is always read as unsigned, and an amount of the width or more
//!   gives 0 (or all sign bits) -- the IR's rule;
//! * casts are part-selects and replications (`{{k{a[w-1]}}, a}`).
//!
//! What is checked in this repository: the netlist evaluator
//! ([`Circuit::eval`]) against native execution; and, when the `YoWASP`
//! build of Yosys is available, Yosys parsing, synthesising and evaluating
//! the emitted text (see the crate README). No simulator such as Icarus or
//! Verilator has run it.

use std::{collections::HashSet, fmt::Write as _};

use crate::ir::{Circuit, Op, Ty, Wire};

/// Verilog-2001 reserved words (and `SystemVerilog`'s, so the output stays
/// valid for tools that read it as `SystemVerilog`).
const KEYWORDS: &[&str] = &[
    "always",
    "and",
    "assign",
    "automatic",
    "begin",
    "buf",
    "bufif0",
    "bufif1",
    "case",
    "casex",
    "casez",
    "cell",
    "cmos",
    "config",
    "deassign",
    "default",
    "defparam",
    "design",
    "disable",
    "edge",
    "else",
    "end",
    "endcase",
    "endconfig",
    "endfunction",
    "endgenerate",
    "endmodule",
    "endprimitive",
    "endspecify",
    "endtable",
    "endtask",
    "event",
    "for",
    "force",
    "forever",
    "fork",
    "function",
    "generate",
    "genvar",
    "highz0",
    "highz1",
    "if",
    "ifnone",
    "incdir",
    "include",
    "initial",
    "inout",
    "input",
    "instance",
    "integer",
    "join",
    "large",
    "liblist",
    "library",
    "localparam",
    "macromodule",
    "medium",
    "module",
    "nand",
    "negedge",
    "nmos",
    "nor",
    "noshowcancelled",
    "not",
    "notif0",
    "notif1",
    "or",
    "output",
    "parameter",
    "pmos",
    "posedge",
    "primitive",
    "pull0",
    "pull1",
    "pulldown",
    "pullup",
    "pulsestyle_onevent",
    "pulsestyle_ondetect",
    "rcmos",
    "real",
    "realtime",
    "reg",
    "release",
    "repeat",
    "rnmos",
    "rpmos",
    "rtran",
    "rtranif0",
    "rtranif1",
    "scalared",
    "showcancelled",
    "signed",
    "small",
    "specify",
    "specparam",
    "strong0",
    "strong1",
    "supply0",
    "supply1",
    "table",
    "task",
    "time",
    "tran",
    "tranif0",
    "tranif1",
    "tri",
    "tri0",
    "tri1",
    "triand",
    "trior",
    "trireg",
    "unsigned",
    "use",
    "uwire",
    "vectored",
    "wait",
    "wand",
    "weak0",
    "weak1",
    "while",
    "wire",
    "wor",
    "xnor",
    "xor",
    "bit",
    "byte",
    "int",
    "logic",
    "longint",
    "shortint",
    "type",
    "var",
    "void",
    "return",
    "break",
    "continue",
    "do",
    "enum",
    "struct",
    "union",
    "typedef",
    "class",
    "this",
    "super",
    "new",
    "null",
    "virtual",
    "static",
    "const",
    "string",
    "unique",
    "priority",
    "final",
    "import",
    "export",
    "package",
    "interface",
    "modport",
    "program",
    "property",
    "sequence",
    "assert",
    "assume",
    "cover",
    "expect",
    "inside",
    "dist",
    "with",
    "matches",
    "tagged",
    "extends",
    "local",
    "protected",
    "pure",
    "rand",
    "randc",
    "constraint",
    "solve",
    "before",
    "chandle",
    "context",
    "alias",
    "always_comb",
    "always_ff",
    "always_latch",
    "bind",
    "bins",
    "binsof",
    "clocking",
    "covergroup",
    "coverpoint",
    "cross",
    "endclass",
    "endclocking",
    "endgroup",
    "endinterface",
    "endpackage",
    "endprogram",
    "endproperty",
    "endsequence",
    "first_match",
    "foreach",
    "forkjoin",
    "iff",
    "ignore_bins",
    "illegal_bins",
    "intersect",
    "join_any",
    "join_none",
    "randcase",
    "randsequence",
    "ref",
    "shortreal",
    "throughout",
    "timeprecision",
    "timeunit",
    "wait_order",
    "wildcard",
    "within",
];

/// The output port's name.
pub const OUTPUT_PORT: &str = "result";

/// A Verilog identifier for a Codira name, not equal to anything in `taken`.
fn identifier(name: &str, taken: &HashSet<String>) -> String {
    let mut id = name.to_string();
    while KEYWORDS.contains(&id.as_str()) || taken.contains(&id) || is_internal_name(&id) {
        id.push('_');
    }
    id
}

/// Internal nets are `n<digits>`; a port must not look like one.
fn is_internal_name(id: &str) -> bool {
    id.strip_prefix('n')
        .is_some_and(|rest| !rest.is_empty() && rest.bytes().all(|b| b.is_ascii_digit()))
}

fn range(ty: Ty) -> String {
    match ty.width() {
        1 => String::new(),
        w => format!("[{}:0] ", w - 1),
    }
}

fn literal(ty: Ty, value: u64) -> String {
    format!("{}'d{}", ty.width(), value & ty.mask())
}

/// The module for `circuit`. The module is named after the function (made a
/// valid identifier); ports are the parameters, in order, then
/// [`OUTPUT_PORT`].
pub fn emit(circuit: &Circuit) -> String {
    let mut taken = HashSet::new();
    let module = identifier(&circuit.name, &taken);
    taken.insert(module.clone());
    let mut ports = Vec::new();
    for port in &circuit.inputs {
        let id = identifier(&port.name, &taken);
        taken.insert(id.clone());
        ports.push(id);
    }
    let output = identifier(OUTPUT_PORT, &taken);

    let name = |w: Wire| match circuit.node(w).op {
        Op::Input(k) => ports[k as usize].clone(),
        _ => w.to_string(),
    };

    let mut out = String::new();
    let _ = writeln!(
        out,
        "// `{}`, compiled by Codira from an `@target(fpga)` function.",
        circuit.signature()
    );
    out.push_str("// Combinational; integers are two's complement, ports and nets unsigned.\n");
    out.push_str("`default_nettype none\n\n");
    let _ = writeln!(out, "module {module} (");
    for (id, port) in ports.iter().zip(&circuit.inputs) {
        let _ = writeln!(
            out,
            "    input  wire {}{id}, // {}",
            range(port.ty),
            port.ty
        );
    }
    let out_ty = circuit.output_ty();
    let _ = writeln!(
        out,
        "    output wire {}{output}  // {out_ty}",
        range(out_ty)
    );
    out.push_str(");\n");

    let live = circuit.live();
    for (i, node) in circuit.nodes.iter().enumerate() {
        if !live[i] || matches!(node.op, Op::Input(_)) {
            continue;
        }
        let w = Wire(u32::try_from(i).expect("node index fits u32"));
        let ty = node.ty;
        let width = ty.width();
        let operand_ty = |x: Wire| circuit.node(x).ty;
        let expr = match node.op {
            Op::Input(_) => unreachable!("skipped above"),
            Op::Const(c) => literal(ty, c),
            Op::Not(a) => format!("~{}", name(a)),
            Op::Neg(a) => format!("-{}", name(a)),
            Op::Add(a, b) => format!("{} + {}", name(a), name(b)),
            Op::Sub(a, b) => format!("{} - {}", name(a), name(b)),
            Op::Mul(a, b) => format!("{} * {}", name(a), name(b)),
            Op::And(a, b) => format!("{} & {}", name(a), name(b)),
            Op::Or(a, b) => format!("{} | {}", name(a), name(b)),
            Op::Xor(a, b) => format!("{} ^ {}", name(a), name(b)),
            Op::Shl(a, b) => format!("{} << {}", name(a), name(b)),
            Op::LShr(a, b) => format!("{} >> {}", name(a), name(b)),
            Op::AShr(a, b) => format!("$signed({}) >>> {}", name(a), name(b)),
            Op::Eq(a, b) => format!("{} == {}", name(a), name(b)),
            Op::Ult(a, b) => format!("{} < {}", name(a), name(b)),
            Op::Slt(a, b) => format!("$signed({}) < $signed({})", name(a), name(b)),
            Op::Mux(c, a, b) => format!("{} ? {} : {}", name(c), name(a), name(b)),
            Op::Trunc(a) => {
                if width == 1 {
                    format!("{}[0]", name(a))
                } else {
                    format!("{}[{}:0]", name(a), width - 1)
                }
            }
            Op::Zext(a) => {
                let from = operand_ty(a).width();
                if from == width {
                    name(a)
                } else {
                    format!("{{{{{}{{1'b0}}}}, {}}}", width - from, name(a))
                }
            }
            Op::Sext(a) => {
                let from = operand_ty(a).width();
                let top = if from == 1 {
                    name(a)
                } else {
                    format!("{}[{}]", name(a), from - 1)
                };
                format!("{{{{{}{{{top}}}}}, {}}}", width - from, name(a))
            }
        };
        let _ = writeln!(out, "    wire {}{w} = {expr};", range(ty));
    }
    let _ = writeln!(out, "    assign {output} = {};", name(circuit.output));
    out.push_str("endmodule\n\n`default_nettype wire\n");
    out
}
