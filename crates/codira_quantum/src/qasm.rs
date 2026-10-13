//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - OpenQASM 2.0 and 3.0 emitters for [`Circuit`], and an importer for the
//!   subset of both that describes a static circuit -- which is what Qiskit's
//!   `qasm2.dumps` / `qasm3.dumps` produce for one.
//!
//! The emitters target what Qiskit loads: OpenQASM 2 through
//! `QuantumCircuit.from_qasm_str` (Qiskit's `qelib1.inc`, which names `sx`,
//! `swap`, `cswap`, `crx` and `cry` alongside the original gates) and
//! OpenQASM 3 through `qiskit.qasm3.loads` (`stdgates.inc`). Angles are
//! printed with Rust's shortest round-trip formatting, so importing an
//! emitted file reproduces the circuit bit for bit.
//!
//! The importer accepts register declarations in both spellings, gate calls
//! with register broadcasting, `measure` in both spellings, `reset`,
//! `barrier`, `gphase` (dropped: a global phase is unobservable), parameter
//! expressions, and user `gate` definitions, which are expanded at their
//! call sites. Gates outside the IR's set that Qiskit commonly emits (`rzz`,
//! `cu3`, `csx`, ...) are defined in a small built-in library in terms of
//! IR gates. Classical control (`if`), loops, subroutines, `input`
//! parameters and gate modifiers are rejected with the line they occur on:
//! they describe programs, not circuits, and the base profile this crate
//! targets cannot express them.

use std::{collections::HashMap, fmt::Write as _};

use crate::circuit::{is_identifier, Circuit, CircuitError, Gate, Op, Register};

/// Which OpenQASM dialect to write.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum QasmVersion {
    V2,
    V3,
}

/// Writes `circuit` as OpenQASM 2.0.
pub fn emit_qasm2(circuit: &Circuit) -> Result<String, CircuitError> {
    emit(circuit, QasmVersion::V2)
}

/// Writes `circuit` as OpenQASM 3.0.
pub fn emit_qasm3(circuit: &Circuit) -> Result<String, CircuitError> {
    emit(circuit, QasmVersion::V3)
}

/// Writes `circuit` in the given dialect.
pub fn emit(circuit: &Circuit, version: QasmVersion) -> Result<String, CircuitError> {
    circuit.validate()?;
    let mut out = String::new();
    match version {
        QasmVersion::V2 => {
            out.push_str("OPENQASM 2.0;\ninclude \"qelib1.inc\";\n");
            for reg in &circuit.qregs {
                let _ = writeln!(out, "qreg {}[{}];", reg.name, reg.size);
            }
            for reg in &circuit.cregs {
                let _ = writeln!(out, "creg {}[{}];", reg.name, reg.size);
            }
        }
        QasmVersion::V3 => {
            out.push_str("OPENQASM 3.0;\ninclude \"stdgates.inc\";\n");
            for reg in &circuit.qregs {
                let _ = writeln!(out, "qubit[{}] {};", reg.size, reg.name);
            }
            for reg in &circuit.cregs {
                let _ = writeln!(out, "bit[{}] {};", reg.size, reg.name);
            }
        }
    }

    let qubit = |q: u32| {
        let (reg, offset) = circuit.qubit_location(q).expect("validated");
        format!("{}[{offset}]", reg.name)
    };
    let clbit = |c: u32| {
        let (reg, offset) = circuit.clbit_location(c).expect("validated");
        format!("{}[{offset}]", reg.name)
    };

    for op in &circuit.ops {
        match op {
            Op::Gate { gate, qubits } => {
                let name = match (version, gate) {
                    (QasmVersion::V2, Gate::P(_)) => "u1",
                    (QasmVersion::V2, Gate::U(..)) => "u3",
                    (QasmVersion::V2, Gate::Cp(_)) => "cu1",
                    (QasmVersion::V3, Gate::U(..)) => "U",
                    _ => gate.name(),
                };
                out.push_str(name);
                let params = gate.params();
                if !params.is_empty() {
                    out.push('(');
                    for (i, p) in params.iter().enumerate() {
                        if i > 0 {
                            out.push_str(", ");
                        }
                        out.push_str(&format_angle(*p));
                    }
                    out.push(')');
                }
                out.push(' ');
                let args: Vec<String> = qubits.iter().map(|&q| qubit(q)).collect();
                out.push_str(&args.join(", "));
                out.push_str(";\n");
            }
            Op::Measure { qubit: q, clbit: c } => match version {
                QasmVersion::V2 => {
                    let _ = writeln!(out, "measure {} -> {};", qubit(*q), clbit(*c));
                }
                QasmVersion::V3 => {
                    let _ = writeln!(out, "{} = measure {};", clbit(*c), qubit(*q));
                }
            },
            Op::Reset { qubit: q } => {
                let _ = writeln!(out, "reset {};", qubit(*q));
            }
            Op::Barrier { qubits } => {
                let args: Vec<String> = if qubits.is_empty() {
                    circuit.qregs.iter().map(|r| r.name.clone()).collect()
                } else {
                    qubits.iter().map(|&q| qubit(q)).collect()
                };
                let _ = writeln!(out, "barrier {};", args.join(", "));
            }
        }
    }
    Ok(out)
}

/// Formats an angle so that parsing it back yields the same `f64`, always
/// with a decimal point (OpenQASM 2's real-number grammar requires one).
pub fn format_angle(value: f64) -> String {
    // `Display` for f64 is the shortest string that round-trips, and never
    // uses an exponent.
    let s = format!("{value}");
    if s.contains('.') {
        s
    } else {
        format!("{s}.0")
    }
}

/// An import failure, with the 1-based source line it was found on.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
#[error("line {line}: {message}")]
pub struct QasmError {
    pub line: u32,
    pub message: String,
}

fn err<T>(line: u32, message: impl Into<String>) -> Result<T, QasmError> {
    Err(QasmError {
        line,
        message: message.into(),
    })
}

/// Gates outside the IR's set that Qiskit's exporters emit, defined in terms
/// of IR gates. Each definition equals the Qiskit gate of the same name up
/// to a global phase (checked in `tests`, and against Qiskit itself by the
/// Python binding's cross-check).
const LIBRARY: &str = "
gate sxdg a { h a; sdg a; h a; }
gate u0(gamma) a { id a; }
gate r(theta, phi) a { u3(theta, phi - pi/2, -phi + pi/2) a; }
gate rzz(theta) a, b { cx a, b; rz(theta) b; cx a, b; }
gate rxx(theta) a, b { h a; h b; cx a, b; rz(theta) b; cx a, b; h a; h b; }
gate ryy(theta) a, b { rx(pi/2) a; rx(pi/2) b; cx a, b; rz(theta) b; cx a, b; rx(-pi/2) a; rx(-pi/2) b; }
gate rzx(theta) a, b { h b; cx a, b; rz(theta) b; cx a, b; h b; }
gate cu3(theta, phi, lambda) c, t { p((lambda + phi)/2) c; p((lambda - phi)/2) t; cx c, t; u3(-theta/2, 0, -(phi + lambda)/2) t; cx c, t; u3(theta/2, phi, 0) t; }
gate cu(theta, phi, lambda, gamma) c, t { p(gamma) c; cu3(theta, phi, lambda) c, t; }
gate csx a, b { h b; cp(pi/2) a, b; h b; }
gate cs a, b { cp(pi/2) a, b; }
gate csdg a, b { cp(-pi/2) a, b; }
gate ccz a, b, c { h c; ccx a, b, c; h c; }
gate dcx a, b { cx a, b; cx b, a; }
gate iswap a, b { s a; s b; h a; cx a, b; cx b, a; h b; }
gate ecr a, b { rzx(pi/4) a, b; x a; rzx(-pi/4) a, b; }
";

/// Parses an OpenQASM 2 or 3 program into a circuit named `name`.
pub fn import(source: &str, name: &str) -> Result<Circuit, QasmError> {
    let mut parser = Parser {
        tokens: tokenize(source)?,
        pos: 0,
        version: None,
        circuit: Circuit {
            name: name.to_string(),
            ..Circuit::default()
        },
        gates: HashMap::new(),
    };
    parser.load_library();
    parser.parse_program()?;
    let circuit = parser.circuit;
    circuit.validate().map_err(|e| QasmError {
        line: 0,
        message: e.to_string(),
    })?;
    Ok(circuit)
}

#[derive(Debug, Clone, PartialEq)]
enum Tok {
    Ident(String),
    Num(f64),
    /// An integer literal, kept exact for sizes and indices.
    Int(u64),
    Str(String),
    Sym(&'static str),
    Eof,
}

#[derive(Debug, Clone)]
struct Token {
    tok: Tok,
    line: u32,
}

const SYMBOLS: &[&str] = &[
    "->", "==", "**", "!=", ">=", "<=", "++", ";", ",", "(", ")", "[", "]", "{", "}", "+", "-",
    "*", "/", "^", "=", "@", ":", ">", "<", "!", "&", "|", "~", "%", "$",
];

fn tokenize(src: &str) -> Result<Vec<Token>, QasmError> {
    let mut tokens = Vec::new();
    let mut line = 1u32;
    let chars: Vec<char> = src.chars().collect();
    let mut i = 0;
    while i < chars.len() {
        let c = chars[i];
        if c == '\n' {
            line += 1;
            i += 1;
        } else if c.is_whitespace() {
            i += 1;
        } else if c == '/' && chars.get(i + 1) == Some(&'/') {
            while i < chars.len() && chars[i] != '\n' {
                i += 1;
            }
        } else if c == '/' && chars.get(i + 1) == Some(&'*') {
            i += 2;
            while i < chars.len() && !(chars[i] == '*' && chars.get(i + 1) == Some(&'/')) {
                if chars[i] == '\n' {
                    line += 1;
                }
                i += 1;
            }
            i += 2;
        } else if c == '"' || c == '\'' {
            let start = i + 1;
            i += 1;
            while i < chars.len() && chars[i] != c {
                i += 1;
            }
            if i >= chars.len() {
                return err(line, "unterminated string");
            }
            tokens.push(Token {
                tok: Tok::Str(chars[start..i].iter().collect()),
                line,
            });
            i += 1;
        } else if c.is_ascii_digit()
            || (c == '.' && chars.get(i + 1).is_some_and(char::is_ascii_digit))
        {
            let start = i;
            while i < chars.len() && (chars[i].is_ascii_digit() || chars[i] == '_') {
                i += 1;
            }
            let mut is_real = false;
            if i < chars.len() && chars[i] == '.' {
                is_real = true;
                i += 1;
                while i < chars.len() && chars[i].is_ascii_digit() {
                    i += 1;
                }
            }
            if i < chars.len() && (chars[i] == 'e' || chars[i] == 'E') {
                let save = i;
                i += 1;
                if i < chars.len() && (chars[i] == '+' || chars[i] == '-') {
                    i += 1;
                }
                if i < chars.len() && chars[i].is_ascii_digit() {
                    is_real = true;
                    while i < chars.len() && chars[i].is_ascii_digit() {
                        i += 1;
                    }
                } else {
                    i = save;
                }
            }
            let text: String = chars[start..i].iter().filter(|&&c| c != '_').collect();
            let tok = if is_real {
                Tok::Num(text.parse().map_err(|_e| QasmError {
                    line,
                    message: format!("bad number `{text}`"),
                })?)
            } else {
                Tok::Int(text.parse().map_err(|_e| QasmError {
                    line,
                    message: format!("bad integer `{text}`"),
                })?)
            };
            tokens.push(Token { tok, line });
        } else if c.is_alphabetic() || c == '_' {
            let start = i;
            while i < chars.len() && (chars[i].is_alphanumeric() || chars[i] == '_') {
                i += 1;
            }
            tokens.push(Token {
                tok: Tok::Ident(chars[start..i].iter().collect()),
                line,
            });
        } else {
            let rest: String = chars[i..chars.len().min(i + 2)].iter().collect();
            let Some(sym) = SYMBOLS.iter().find(|s| rest.starts_with(**s)) else {
                return err(line, format!("unexpected character `{c}`"));
            };
            tokens.push(Token {
                tok: Tok::Sym(sym),
                line,
            });
            i += sym.chars().count();
        }
    }
    tokens.push(Token {
        tok: Tok::Eof,
        line,
    });
    Ok(tokens)
}

/// A parameter expression, kept as a tree so a gate definition's body can
/// be evaluated once per call with that call's arguments.
#[derive(Debug, Clone, PartialEq)]
enum Expr {
    Num(f64),
    Var(String),
    Neg(Box<Expr>),
    Bin(char, Box<Expr>, Box<Expr>),
    Call(String, Box<Expr>),
}

impl Expr {
    fn eval(&self, env: &HashMap<String, f64>, line: u32) -> Result<f64, QasmError> {
        Ok(match self {
            Expr::Num(v) => *v,
            Expr::Var(name) => match env.get(name) {
                Some(v) => *v,
                None => match name.as_str() {
                    "pi" | "π" => std::f64::consts::PI,
                    "tau" | "τ" => std::f64::consts::TAU,
                    "euler" | "ℇ" => std::f64::consts::E,
                    _ => return err(line, format!("unknown parameter `{name}`")),
                },
            },
            Expr::Neg(e) => -e.eval(env, line)?,
            Expr::Bin(op, a, b) => {
                let (a, b) = (a.eval(env, line)?, b.eval(env, line)?);
                match op {
                    '+' => a + b,
                    '-' => a - b,
                    '*' => a * b,
                    '/' => a / b,
                    '^' => a.powf(b),
                    _ => unreachable!("parser only builds the operators above"),
                }
            }
            Expr::Call(f, e) => {
                let v = e.eval(env, line)?;
                match f.as_str() {
                    "sin" => v.sin(),
                    "cos" => v.cos(),
                    "tan" => v.tan(),
                    "asin" | "arcsin" => v.asin(),
                    "acos" | "arccos" => v.acos(),
                    "atan" | "arctan" => v.atan(),
                    "exp" => v.exp(),
                    "ln" | "log" => v.ln(),
                    "sqrt" => v.sqrt(),
                    _ => return err(line, format!("unknown function `{f}`")),
                }
            }
        })
    }
}

/// A user (or library) `gate` definition.
#[derive(Debug, Clone)]
struct GateDef {
    params: Vec<String>,
    qargs: Vec<String>,
    body: Vec<BodyStmt>,
}

#[derive(Debug, Clone)]
enum BodyStmt {
    Call {
        name: String,
        params: Vec<Expr>,
        qargs: Vec<String>,
        line: u32,
    },
    Barrier,
}

/// A qubit or classical-bit operand: a whole register, or one element.
#[derive(Debug, Clone)]
enum Operand {
    Whole(String),
    Indexed(String, u64),
}

struct Parser {
    tokens: Vec<Token>,
    pos: usize,
    version: Option<u32>,
    circuit: Circuit,
    gates: HashMap<String, GateDef>,
}

/// The deepest gate-definition nesting the importer expands.
const MAX_GATE_DEPTH: usize = 64;

impl Parser {
    fn load_library(&mut self) {
        let mut lib = Parser {
            tokens: tokenize(LIBRARY).expect("library tokenizes"),
            pos: 0,
            version: Some(2),
            circuit: Circuit::default(),
            gates: HashMap::new(),
        };
        while lib.peek() != &Tok::Eof {
            lib.parse_gate_def().expect("library parses");
        }
        self.gates = lib.gates;
    }

    fn peek(&self) -> &Tok {
        &self.tokens[self.pos].tok
    }

    fn line(&self) -> u32 {
        self.tokens[self.pos].line
    }

    fn next(&mut self) -> Tok {
        let tok = self.tokens[self.pos].tok.clone();
        if self.pos + 1 < self.tokens.len() {
            self.pos += 1;
        }
        tok
    }

    fn eat(&mut self, sym: &str) -> bool {
        if matches!(self.peek(), Tok::Sym(s) if *s == sym) {
            self.next();
            true
        } else {
            false
        }
    }

    fn expect(&mut self, sym: &str) -> Result<(), QasmError> {
        if self.eat(sym) {
            Ok(())
        } else {
            err(
                self.line(),
                format!("expected `{sym}`, found {}", describe(self.peek())),
            )
        }
    }

    fn ident(&mut self) -> Result<String, QasmError> {
        match self.next() {
            Tok::Ident(name) => Ok(name),
            other => err(
                self.tokens[self.pos.saturating_sub(1)].line,
                format!("expected a name, found {}", describe(&other)),
            ),
        }
    }

    fn int(&mut self) -> Result<u64, QasmError> {
        match self.next() {
            Tok::Int(v) => Ok(v),
            other => err(
                self.tokens[self.pos.saturating_sub(1)].line,
                format!("expected an integer, found {}", describe(&other)),
            ),
        }
    }

    fn parse_program(&mut self) -> Result<(), QasmError> {
        if let Tok::Ident(kw) = self.peek() {
            if kw == "OPENQASM" {
                let line = self.line();
                self.next();
                let version = match self.next() {
                    Tok::Num(v) => v,
                    Tok::Int(v) => v as f64,
                    other => return err(line, format!("bad version {}", describe(&other))),
                };
                self.version = Some(if version < 3.0 { 2 } else { 3 });
                if !(version == 2.0 || version == 3.0 || version == 3.1) {
                    return err(line, format!("unsupported OpenQASM version {version}"));
                }
                self.expect(";")?;
            }
        }
        while self.peek() != &Tok::Eof {
            self.parse_statement()?;
        }
        Ok(())
    }

    fn parse_statement(&mut self) -> Result<(), QasmError> {
        let line = self.line();
        let word = match self.peek() {
            Tok::Ident(w) => w.clone(),
            Tok::Sym(";") => {
                self.next();
                return Ok(());
            }
            Tok::Sym("$") => return err(line, "physical qubits (`$n`) are not supported"),
            other => return err(line, format!("unexpected {}", describe(other))),
        };
        match word.as_str() {
            "include" => {
                self.next();
                match self.next() {
                    Tok::Str(file) => {
                        if !matches!(file.as_str(), "qelib1.inc" | "stdgates.inc") {
                            return err(
                                line,
                                format!(
                                    "cannot include `{file}`: only qelib1.inc and stdgates.inc are \
                                     known (inline the gate definitions instead)"
                                ),
                            );
                        }
                    }
                    other => {
                        return err(
                            line,
                            format!("expected a file name, found {}", describe(&other)),
                        )
                    }
                }
                self.expect(";")
            }
            "qreg" | "creg" => {
                self.next();
                let name = self.ident()?;
                self.expect("[")?;
                let size = self.int()?;
                self.expect("]")?;
                self.expect(";")?;
                self.declare(word == "qreg", name, size, line)
            }
            "qubit" | "bit" => {
                self.next();
                let size = if self.eat("[") {
                    let size = self.int()?;
                    self.expect("]")?;
                    size
                } else {
                    1
                };
                let name = self.ident()?;
                if self.eat("=") {
                    // `bit[2] c = measure q;`
                    self.declare(word == "qubit", name.clone(), size, line)?;
                    self.parse_measure_tail(Operand::Whole(name), line)?;
                    return Ok(());
                }
                self.expect(";")?;
                self.declare(word == "qubit", name, size, line)
            }
            "gate" => self.parse_gate_def(),
            "measure" => {
                self.next();
                let q = self.operand()?;
                self.expect("->")?;
                let c = self.operand()?;
                self.expect(";")?;
                self.measure(&q, &c, line)
            }
            "reset" => {
                self.next();
                let q = self.operand()?;
                self.expect(";")?;
                for qubit in self.qubits_of(&q, line)? {
                    self.circuit.ops.push(Op::Reset { qubit });
                }
                Ok(())
            }
            "barrier" => {
                self.next();
                let mut qubits = Vec::new();
                if !self.eat(";") {
                    loop {
                        let q = self.operand()?;
                        qubits.extend(self.qubits_of(&q, line)?);
                        if !self.eat(",") {
                            break;
                        }
                    }
                    self.expect(";")?;
                }
                self.circuit.ops.push(Op::Barrier { qubits });
                Ok(())
            }
            "gphase" => {
                // A global phase changes no measurement statistics.
                self.next();
                self.expect("(")?;
                self.expr()?;
                self.expect(")")?;
                self.expect(";")
            }
            "opaque" => err(
                line,
                "`opaque` gates have no definition to simulate or emit",
            ),
            "if" => err(
                line,
                "classically controlled operations (`if`) need measurement feedback, which the \
                 base profile does not have",
            ),
            "ctrl" | "negctrl" | "inv" | "pow" => err(
                line,
                format!("gate modifier `{word} @` is not supported; define the gate explicitly"),
            ),
            "for" | "while" | "def" | "defcal" | "cal" | "defcalgrammar" | "input" | "output"
            | "const" | "let" | "box" | "delay" | "switch" | "return" | "extern" | "int"
            | "uint" | "float" | "angle" | "bool" | "complex" | "duration" | "stretch"
            | "array" => err(
                line,
                format!(
                    "`{word}` describes a classical program, not a static circuit; only \
                     straight-line OpenQASM is imported"
                ),
            ),
            _ => self.parse_gate_call_or_assign(word, line),
        }
    }

    fn declare(
        &mut self,
        quantum: bool,
        name: String,
        size: u64,
        line: u32,
    ) -> Result<(), QasmError> {
        if self
            .circuit
            .qregs
            .iter()
            .chain(&self.circuit.cregs)
            .any(|r| r.name == name)
        {
            return err(line, format!("register `{name}` is declared twice"));
        }
        if !is_identifier(&name) {
            return err(line, format!("`{name}` is not a valid register name"));
        }
        let size = u32::try_from(size)
            .ok()
            .filter(|&s| s > 0)
            .ok_or_else(|| QasmError {
                line,
                message: format!("register `{name}` has an invalid size {size}"),
            })?;
        let reg = Register::new(name, size);
        if quantum {
            self.circuit.qregs.push(reg);
        } else {
            self.circuit.cregs.push(reg);
        }
        Ok(())
    }

    fn operand(&mut self) -> Result<Operand, QasmError> {
        if matches!(self.peek(), Tok::Sym("$")) {
            return err(self.line(), "physical qubits (`$n`) are not supported");
        }
        let name = self.ident()?;
        if self.eat("[") {
            let index = self.int()?;
            self.expect("]")?;
            Ok(Operand::Indexed(name, index))
        } else {
            Ok(Operand::Whole(name))
        }
    }

    fn resolve(
        regs: &[Register],
        op: &Operand,
        line: u32,
        what: &str,
    ) -> Result<Vec<u32>, QasmError> {
        let (name, index) = match op {
            Operand::Whole(n) => (n, None),
            Operand::Indexed(n, i) => (n, Some(*i)),
        };
        let mut start = 0u32;
        for reg in regs {
            if &reg.name == name {
                return match index {
                    None => Ok((start..start + reg.size).collect()),
                    Some(i) if i < u64::from(reg.size) => Ok(vec![start + i as u32]),
                    Some(i) => err(
                        line,
                        format!("index {i} is out of range for `{name}` (size {})", reg.size),
                    ),
                };
            }
            start += reg.size;
        }
        err(line, format!("unknown {what} register `{name}`"))
    }

    fn qubits_of(&self, op: &Operand, line: u32) -> Result<Vec<u32>, QasmError> {
        Self::resolve(&self.circuit.qregs, op, line, "qubit")
    }

    fn clbits_of(&self, op: &Operand, line: u32) -> Result<Vec<u32>, QasmError> {
        Self::resolve(&self.circuit.cregs, op, line, "classical")
    }

    fn measure(&mut self, q: &Operand, c: &Operand, line: u32) -> Result<(), QasmError> {
        let qubits = self.qubits_of(q, line)?;
        let clbits = self.clbits_of(c, line)?;
        if qubits.len() != clbits.len() {
            return err(
                line,
                format!(
                    "measuring {} qubits into {} classical bits",
                    qubits.len(),
                    clbits.len()
                ),
            );
        }
        for (qubit, clbit) in qubits.into_iter().zip(clbits) {
            self.circuit.ops.push(Op::Measure { qubit, clbit });
        }
        Ok(())
    }

    /// After `c =` (or a declaration's `=`): `measure q;`.
    fn parse_measure_tail(&mut self, target: Operand, line: u32) -> Result<(), QasmError> {
        match self.next() {
            Tok::Ident(w) if w == "measure" => {}
            other => {
                return err(
                    line,
                    format!(
                        "only `measure` can be assigned to a bit here, found {}",
                        describe(&other)
                    ),
                )
            }
        }
        let q = self.operand()?;
        self.expect(";")?;
        self.measure(&q, &target, line)
    }

    fn parse_gate_call_or_assign(&mut self, name: String, line: u32) -> Result<(), QasmError> {
        self.next();
        // `c[0] = measure q[0];` / `c = measure q;`
        if matches!(self.peek(), Tok::Sym("[" | "=")) && !self.gates.contains_key(&name) {
            let target = if self.eat("[") {
                let index = self.int()?;
                self.expect("]")?;
                Operand::Indexed(name, index)
            } else {
                Operand::Whole(name)
            };
            self.expect("=")?;
            return self.parse_measure_tail(target, line);
        }
        if matches!(self.peek(), Tok::Sym("@")) {
            return err(line, format!("gate modifier `{name} @` is not supported"));
        }

        let mut params = Vec::new();
        if self.eat("(") && !self.eat(")") {
            loop {
                params.push(self.expr()?);
                if !self.eat(",") {
                    break;
                }
            }
            self.expect(")")?;
        }
        let mut operands = Vec::new();
        loop {
            operands.push(self.operand()?);
            if !self.eat(",") {
                break;
            }
        }
        self.expect(";")?;

        let env = HashMap::new();
        let values = params
            .iter()
            .map(|p| p.eval(&env, line))
            .collect::<Result<Vec<_>, _>>()?;

        // Broadcast: whole-register operands must agree in size; single
        // qubits repeat.
        let lists = operands
            .iter()
            .map(|o| self.qubits_of(o, line))
            .collect::<Result<Vec<_>, _>>()?;
        let width = lists
            .iter()
            .zip(&operands)
            .filter(|(_, o)| matches!(o, Operand::Whole(_)))
            .map(|(l, _)| l.len())
            .max()
            .unwrap_or(1);
        for (list, o) in lists.iter().zip(&operands) {
            if matches!(o, Operand::Whole(_)) && list.len() != width {
                return err(line, "registers of different sizes in one gate call");
            }
        }
        for k in 0..width {
            let qubits: Vec<u32> = lists
                .iter()
                .map(|l| if l.len() == 1 { l[0] } else { l[k] })
                .collect();
            self.apply(&name, &values, &qubits, line, 0)?;
        }
        Ok(())
    }

    /// Applies gate `name` -- an IR gate, or a definition expanded in place.
    fn apply(
        &mut self,
        name: &str,
        params: &[f64],
        qubits: &[u32],
        line: u32,
        depth: usize,
    ) -> Result<(), QasmError> {
        if depth > MAX_GATE_DEPTH {
            return err(
                line,
                format!("gate `{name}` nests more than {MAX_GATE_DEPTH} deep"),
            );
        }
        // A user definition shadows the IR gate of the same name, matching
        // OpenQASM's scoping; definitions of the standard gates themselves
        // (as `qelib1.inc` would add when inlined) are skipped at definition
        // time instead, so this only catches genuinely new gates.
        if let Some(def) = self.gates.get(name).cloned() {
            if def.params.len() != params.len() || def.qargs.len() != qubits.len() {
                return err(
                    line,
                    format!(
                        "gate `{name}` takes {} parameters and {} qubits, given {} and {}",
                        def.params.len(),
                        def.qargs.len(),
                        params.len(),
                        qubits.len()
                    ),
                );
            }
            let env: HashMap<String, f64> = def
                .params
                .iter()
                .cloned()
                .zip(params.iter().copied())
                .collect();
            let qmap: HashMap<&str, u32> = def
                .qargs
                .iter()
                .map(String::as_str)
                .zip(qubits.iter().copied())
                .collect();
            for stmt in &def.body {
                match stmt {
                    BodyStmt::Call {
                        name: inner,
                        params: exprs,
                        qargs,
                        line: inner_line,
                    } => {
                        let values = exprs
                            .iter()
                            .map(|e| e.eval(&env, *inner_line))
                            .collect::<Result<Vec<_>, _>>()?;
                        let mapped = qargs
                            .iter()
                            .map(|a| {
                                qmap.get(a.as_str()).copied().ok_or_else(|| QasmError {
                                    line: *inner_line,
                                    message: format!("`{a}` is not an argument of gate `{name}`"),
                                })
                            })
                            .collect::<Result<Vec<_>, _>>()?;
                        self.apply(inner, &values, &mapped, *inner_line, depth + 1)?;
                    }
                    BodyStmt::Barrier => {}
                }
            }
            return Ok(());
        }
        match Gate::from_name(name, params) {
            Some(gate) => {
                if gate.num_qubits() != qubits.len() {
                    return err(
                        line,
                        format!(
                            "`{name}` acts on {} qubits, given {}",
                            gate.num_qubits(),
                            qubits.len()
                        ),
                    );
                }
                self.circuit.ops.push(Op::Gate {
                    gate,
                    qubits: qubits.to_vec(),
                });
                Ok(())
            }
            None if (0..=3).any(|n| Gate::from_name(name, &[0.0; 3][..n]).is_some()) => err(
                line,
                format!(
                    "`{name}` given the wrong number of parameters ({})",
                    params.len()
                ),
            ),
            None => err(line, format!("unknown gate `{name}`")),
        }
    }

    fn parse_gate_def(&mut self) -> Result<(), QasmError> {
        let line = self.line();
        self.next(); // `gate`
        let name = self.ident()?;
        let mut params = Vec::new();
        if self.eat("(") && !self.eat(")") {
            loop {
                params.push(self.ident()?);
                if !self.eat(",") {
                    break;
                }
            }
            self.expect(")")?;
        }
        let mut qargs = Vec::new();
        loop {
            qargs.push(self.ident()?);
            if !self.eat(",") {
                break;
            }
        }
        self.expect("{")?;
        let mut body = Vec::new();
        while !self.eat("}") {
            let stmt_line = self.line();
            let word = self.ident()?;
            if word == "barrier" {
                while !self.eat(";") {
                    self.next();
                }
                body.push(BodyStmt::Barrier);
                continue;
            }
            if word == "gphase" {
                self.expect("(")?;
                self.expr()?;
                self.expect(")")?;
                self.expect(";")?;
                continue;
            }
            if matches!(self.peek(), Tok::Sym("@")) {
                return err(
                    stmt_line,
                    format!("gate modifier `{word} @` is not supported"),
                );
            }
            let mut call_params = Vec::new();
            if self.eat("(") && !self.eat(")") {
                loop {
                    call_params.push(self.expr()?);
                    if !self.eat(",") {
                        break;
                    }
                }
                self.expect(")")?;
            }
            let mut call_qargs = Vec::new();
            loop {
                call_qargs.push(self.ident()?);
                if !self.eat(",") {
                    break;
                }
            }
            self.expect(";")?;
            body.push(BodyStmt::Call {
                name: word,
                params: call_params,
                qargs: call_qargs,
                line: stmt_line,
            });
        }
        // Redefinitions of gates the IR already has natively (an inlined
        // `qelib1.inc`, or Qiskit re-stating a standard gate) are dropped:
        // the native gate is the same matrix and keeps its name.
        let native = Gate::from_name(&name, &vec![0.0; params.len()]).is_some();
        if !native {
            if self.gates.contains_key(&name) && !LIBRARY_NAMES.contains(&name.as_str()) {
                return err(line, format!("gate `{name}` is defined twice"));
            }
            self.gates.insert(
                name,
                GateDef {
                    params,
                    qargs,
                    body,
                },
            );
        }
        Ok(())
    }

    // Expression grammar: sum := term (('+'|'-') term)*; term := unary
    // (('*'|'/') unary)*; unary := '-' unary | power; power := atom
    // (('^'|'**') unary)?
    fn expr(&mut self) -> Result<Expr, QasmError> {
        let mut lhs = self.term()?;
        loop {
            if self.eat("+") {
                lhs = Expr::Bin('+', Box::new(lhs), Box::new(self.term()?));
            } else if self.eat("-") {
                lhs = Expr::Bin('-', Box::new(lhs), Box::new(self.term()?));
            } else {
                return Ok(lhs);
            }
        }
    }

    fn term(&mut self) -> Result<Expr, QasmError> {
        let mut lhs = self.unary()?;
        loop {
            if self.eat("*") {
                lhs = Expr::Bin('*', Box::new(lhs), Box::new(self.unary()?));
            } else if self.eat("/") {
                lhs = Expr::Bin('/', Box::new(lhs), Box::new(self.unary()?));
            } else {
                return Ok(lhs);
            }
        }
    }

    fn unary(&mut self) -> Result<Expr, QasmError> {
        if self.eat("-") {
            return Ok(Expr::Neg(Box::new(self.unary()?)));
        }
        if self.eat("+") {
            return self.unary();
        }
        let base = self.atom()?;
        if self.eat("^") || self.eat("**") {
            return Ok(Expr::Bin('^', Box::new(base), Box::new(self.unary()?)));
        }
        Ok(base)
    }

    fn atom(&mut self) -> Result<Expr, QasmError> {
        let line = self.line();
        match self.next() {
            Tok::Num(v) => Ok(Expr::Num(v)),
            Tok::Int(v) => Ok(Expr::Num(v as f64)),
            Tok::Ident(name) => {
                if self.eat("(") {
                    let arg = self.expr()?;
                    self.expect(")")?;
                    Ok(Expr::Call(name, Box::new(arg)))
                } else {
                    Ok(Expr::Var(name))
                }
            }
            Tok::Sym("(") => {
                let e = self.expr()?;
                self.expect(")")?;
                Ok(e)
            }
            other => err(
                line,
                format!("expected an expression, found {}", describe(&other)),
            ),
        }
    }
}

/// Names the built-in library defines; a program may redefine them (Qiskit
/// emits its own definitions of e.g. `rzz` when exporting), and its
/// definition then wins.
const LIBRARY_NAMES: &[&str] = &[
    "sxdg", "u0", "r", "rzz", "rxx", "ryy", "rzx", "cu3", "cu", "csx", "cs", "csdg", "ccz", "dcx",
    "iswap", "ecr",
];

fn describe(tok: &Tok) -> String {
    match tok {
        Tok::Ident(s) => format!("`{s}`"),
        Tok::Num(v) => format!("`{v}`"),
        Tok::Int(v) => format!("`{v}`"),
        Tok::Str(s) => format!("\"{s}\""),
        Tok::Sym(s) => format!("`{s}`"),
        Tok::Eof => "end of input".to_string(),
    }
}

#[cfg(test)]
mod tests;
