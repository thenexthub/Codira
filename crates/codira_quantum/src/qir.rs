//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - QIR Base Profile emission (LLVM IR text and bitcode, built with inkwell on
//!   the workspace's LLVM 22) and import of base-profile QIR back into a
//!   [`Circuit`].
//!
//! The emitted module follows the QIR Alliance Base Profile specification
//! (`specification/profiles/Base_Profile.md` in `qir-alliance/qir-spec`):
//! - one entry point `define i64 @<name>() #0` with four blocks -- `entry`
//!   (calls `__quantum__rt__initialize(ptr null)`), `body` (only reversible QIS
//!   calls), `measurements` (only `irreversible` calls) and `output` (only
//!   output recording, then `ret i64 0`), joined by unconditional branches;
//! - qubits and results as constant `inttoptr (i64 N to ptr)` (opaque pointers,
//!   QIR 2.0), never dynamically allocated;
//! - QIS declarations `__quantum__qis__<gate>__body` / `__adj` that return
//!   `void`, with `mz` marked `"irreversible"` and its result parameter
//!   `writeonly`;
//! - output recorded with `__quantum__rt__array_record_output` /
//!   `__quantum__rt__tuple_record_output` and
//!   `__quantum__rt__result_record_output`, each with its own label string
//!   (Q#'s labeling scheme: `0_a`, `1_a0r`, ... for one array);
//! - entry attributes `"entry_point"`, `"qir_profiles"="base_profile"`,
//!   `"output_labeling_schema"`, `"required_num_qubits"`,
//!   `"required_num_results"`;
//! - module flags `qir_major_version` (behavior Error), `qir_minor_version`
//!   (Max), `dynamic_qubit_management` and `dynamic_result_management` (Error,
//!   `i1 false`).
//!
//! The gate names are the ones Microsoft's QDK (`qsharp`) emits and the QIR
//! Alliance's `qir-runner` executes: `h x y z s s__adj t t__adj rx ry rz cx
//! cy cz ccx swap mz`. IR gates outside that set are decomposed into it
//! (each decomposition is checked against the gate's unitary in the tests).
//!
//! Every emitted module is checked by the LLVM verifier, and the emitted
//! text and bitcode are each parsed back with LLVM and verified again before
//! they are returned.

use std::{collections::BTreeMap, f64::consts::FRAC_PI_2};

use inkwell::{
    attributes::{Attribute, AttributeLoc},
    context::Context,
    memory_buffer::MemoryBuffer,
    module::{Linkage, Module},
    types::BasicMetadataTypeEnum,
    values::{BasicMetadataValueEnum, FunctionValue},
    AddressSpace,
};

use crate::circuit::{Circuit, CircuitError, Gate, Op, Register};

/// Emission options.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct QirOptions {
    /// The `qir_major_version` module flag. The current specification is
    /// 2 (opaque pointers, which is all LLVM 22 can write). Q# 1.x tooling
    /// still labels its own (typed-pointer) output 1; this exists for
    /// consumers that insist on that number.
    pub major_version: u32,
    pub minor_version: u32,
}

impl Default for QirOptions {
    fn default() -> Self {
        QirOptions {
            major_version: 2,
            minor_version: 0,
        }
    }
}

/// The value of the entry point's `"output_labeling_schema"` attribute.
pub const OUTPUT_LABELING_SCHEMA: &str = "codira_v1";

/// Why QIR could not be produced or read.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum QirError {
    #[error(transparent)]
    Circuit(#[from] CircuitError),
    #[error("`{0}` is not a valid entry-point name")]
    BadName(String),
    #[error("LLVM rejected the emitted module: {0}")]
    Verify(String),
    #[error("{0}")]
    Import(String),
}

/// One call into the quantum instruction set.
#[derive(Debug, Clone, PartialEq)]
pub(crate) struct QisCall {
    /// The function name without the `__quantum__qis__` prefix, e.g.
    /// `h__body` or `s__adj`.
    pub(crate) name: &'static str,
    pub(crate) angle: Option<f64>,
    pub(crate) qubits: Vec<u32>,
}

fn call(name: &'static str, qubits: &[u32]) -> QisCall {
    QisCall {
        name,
        angle: None,
        qubits: qubits.to_vec(),
    }
}

fn rot(name: &'static str, angle: f64, qubit: u32) -> QisCall {
    QisCall {
        name,
        angle: Some(angle),
        qubits: vec![qubit],
    }
}

/// Lowers one IR gate to QIS calls. Exact up to a global phase of the
/// gate's whole unitary (which no measurement can observe).
pub(crate) fn decompose(gate: Gate, q: &[u32]) -> Vec<QisCall> {
    match gate {
        Gate::Id => vec![],
        Gate::H => vec![call("h__body", q)],
        Gate::X => vec![call("x__body", q)],
        Gate::Y => vec![call("y__body", q)],
        Gate::Z => vec![call("z__body", q)],
        Gate::S => vec![call("s__body", q)],
        Gate::Sdg => vec![call("s__adj", q)],
        Gate::T => vec![call("t__body", q)],
        Gate::Tdg => vec![call("t__adj", q)],
        // sx = H S H exactly.
        Gate::Sx => vec![call("h__body", q), call("s__body", q), call("h__body", q)],
        Gate::Rx(a) => vec![rot("rx__body", a, q[0])],
        Gate::Ry(a) => vec![rot("ry__body", a, q[0])],
        Gate::Rz(a) => vec![rot("rz__body", a, q[0])],
        // p(l) = e^{i l/2} rz(l).
        Gate::P(l) => vec![rot("rz__body", l, q[0])],
        // U(t, p, l) = e^{i(p+l)/2} rz(p) ry(t) rz(l).
        Gate::U(t, p, l) => vec![
            rot("rz__body", l, q[0]),
            rot("ry__body", t, q[0]),
            rot("rz__body", p, q[0]),
        ],
        Gate::Cx => vec![call("cx__body", q)],
        Gate::Cy => vec![call("cy__body", q)],
        Gate::Cz => vec![call("cz__body", q)],
        Gate::Swap => vec![call("swap__body", q)],
        Gate::Ccx => vec![call("ccx__body", q)],
        // The decomposition Q# itself uses for `Controlled H`.
        Gate::Ch => vec![
            call("s__body", &q[1..]),
            call("h__body", &q[1..]),
            call("t__body", &q[1..]),
            call("cx__body", q),
            call("t__adj", &q[1..]),
            call("h__body", &q[1..]),
            call("s__adj", &q[1..]),
        ],
        Gate::Crz(a) => controlled_rotation("rz__body", a, q),
        Gate::Cry(a) => controlled_rotation("ry__body", a, q),
        // crx = (I (x) H) crz (I (x) H).
        Gate::Crx(a) => {
            let mut calls = vec![call("h__body", &q[1..])];
            calls.extend(controlled_rotation("rz__body", a, q));
            calls.push(call("h__body", &q[1..]));
            calls
        }
        // cp(l) = rz(l/2) on both, then a controlled rz(l) on the target --
        // equal to diag(1, 1, 1, e^{il}) up to a global phase.
        Gate::Cp(l) => {
            let mut calls = vec![rot("rz__body", l / 2.0, q[0])];
            calls.extend(controlled_rotation("rz__body", l, q));
            calls
        }
        Gate::Cswap => vec![
            call("cx__body", &[q[2], q[1]]),
            call("ccx__body", q),
            call("cx__body", &[q[2], q[1]]),
        ],
    }
}

/// A rotation about Y or Z on `q[1]`, controlled by `q[0]`: X R(a) X =
/// R(-a) for both axes, so `R(a/2) CX R(-a/2) CX` is R(a) when the control
/// is set and the identity when it is not.
fn controlled_rotation(name: &'static str, a: f64, q: &[u32]) -> Vec<QisCall> {
    vec![
        rot(name, a / 2.0, q[1]),
        call("cx__body", q),
        rot(name, -a / 2.0, q[1]),
        call("cx__body", q),
    ]
}

/// Emits `circuit` as base-profile QIR, LLVM IR text.
pub fn emit_qir_text(circuit: &Circuit, options: &QirOptions) -> Result<String, QirError> {
    let context = Context::create();
    let module = build_module(&context, circuit, options)?;
    let text = module.print_to_string().to_string();
    // Round trip through LLVM's own parser: what we hand out must load.
    let reparse_context = Context::create();
    let buffer = MemoryBuffer::create_from_memory_range_copy(text.as_bytes(), "qir");
    let reparsed = reparse_context
        .create_module_from_ir(buffer)
        .map_err(|e| QirError::Verify(format!("emitted text does not parse: {e}")))?;
    reparsed
        .verify()
        .map_err(|e| QirError::Verify(format!("re-parsed text fails verification: {e}")))?;
    Ok(text)
}

/// Emits `circuit` as base-profile QIR, LLVM bitcode.
pub fn emit_qir_bitcode(circuit: &Circuit, options: &QirOptions) -> Result<Vec<u8>, QirError> {
    let context = Context::create();
    let module = build_module(&context, circuit, options)?;
    let bitcode = module.write_bitcode_to_memory().as_slice().to_vec();
    let reparse_context = Context::create();
    let buffer = MemoryBuffer::create_from_memory_range_copy(&bitcode, "qir");
    let reparsed = Module::parse_bitcode_from_buffer(&buffer, &reparse_context)
        .map_err(|e| QirError::Verify(format!("emitted bitcode does not parse: {e}")))?;
    reparsed
        .verify()
        .map_err(|e| QirError::Verify(format!("re-parsed bitcode fails verification: {e}")))?;
    Ok(bitcode)
}

fn build_module<'ctx>(
    context: &'ctx Context,
    circuit: &Circuit,
    options: &QirOptions,
) -> Result<Module<'ctx>, QirError> {
    circuit.check_base_profile()?;
    if !crate::circuit::is_identifier(&circuit.name) {
        return Err(QirError::BadName(circuit.name.clone()));
    }

    let module = context.create_module(&circuit.name);
    let builder = context.create_builder();
    let ptr = context.ptr_type(AddressSpace::default());
    let i64t = context.i64_type();
    let f64t = context.f64_type();
    let void = context.void_type();

    // --- Declarations, created on first use and cached by name.
    let mut functions: BTreeMap<String, FunctionValue<'ctx>> = BTreeMap::new();
    let mut declare = |name: &str, params: &[BasicMetadataTypeEnum<'ctx>]| {
        *functions
            .entry(name.to_string())
            .or_insert_with(|| module.add_function(name, void.fn_type(params, false), None))
    };

    // --- The entry point and its four blocks.
    let entry_fn = module.add_function(&circuit.name, i64t.fn_type(&[], false), None);
    for (key, value) in [
        ("entry_point", String::new()),
        ("qir_profiles", "base_profile".to_string()),
        ("output_labeling_schema", OUTPUT_LABELING_SCHEMA.to_string()),
        ("required_num_qubits", circuit.num_qubits().to_string()),
        ("required_num_results", circuit.num_clbits().to_string()),
    ] {
        entry_fn.add_attribute(
            AttributeLoc::Function,
            context.create_string_attribute(key, &value),
        );
    }
    let entry = context.append_basic_block(entry_fn, "entry");
    let body = context.append_basic_block(entry_fn, "body");
    let measurements = context.append_basic_block(entry_fn, "measurements");
    let output = context.append_basic_block(entry_fn, "output");

    let qubit_ptr = |q: u32| -> BasicMetadataValueEnum<'ctx> {
        i64t.const_int(u64::from(q), false)
            .const_to_pointer(ptr)
            .into()
    };
    let build_err = |e: inkwell::builder::BuilderError| QirError::Verify(e.to_string());

    builder.position_at_end(entry);
    let init = declare("__quantum__rt__initialize", &[ptr.into()]);
    builder
        .build_call(init, &[ptr.const_null().into()], "")
        .map_err(build_err)?;
    builder
        .build_unconditional_branch(body)
        .map_err(build_err)?;

    // --- Reversible part: every gate in program order. Measurements are
    // deferred to the next block; `check_base_profile` guarantees no qubit is
    // touched after its measurement, so moving them later changes nothing.
    builder.position_at_end(body);
    for op in &circuit.ops {
        let Op::Gate { gate, qubits } = op else {
            continue;
        };
        for qis in decompose(*gate, qubits) {
            let mut params: Vec<BasicMetadataTypeEnum<'ctx>> = Vec::new();
            let mut args: Vec<BasicMetadataValueEnum<'ctx>> = Vec::new();
            if let Some(angle) = qis.angle {
                params.push(f64t.into());
                args.push(f64t.const_float(angle).into());
            }
            for &q in &qis.qubits {
                params.push(ptr.into());
                args.push(qubit_ptr(q));
            }
            let f = declare(&format!("__quantum__qis__{}", qis.name), &params);
            builder.build_call(f, &args, "").map_err(build_err)?;
        }
    }
    builder
        .build_unconditional_branch(measurements)
        .map_err(build_err)?;

    // --- Irreversible part.
    builder.position_at_end(measurements);
    let has_measurement = circuit
        .ops
        .iter()
        .any(|op| matches!(op, Op::Measure { .. }));
    if has_measurement {
        let mz = declare("__quantum__qis__mz__body", &[ptr.into(), ptr.into()]);
        mz.add_attribute(
            AttributeLoc::Function,
            context.create_string_attribute("irreversible", ""),
        );
        mz.add_attribute(
            AttributeLoc::Param(1),
            context.create_enum_attribute(Attribute::get_named_enum_kind_id("writeonly"), 0),
        );
        for op in &circuit.ops {
            if let Op::Measure { qubit, clbit } = op {
                builder
                    .build_call(mz, &[qubit_ptr(*qubit), qubit_ptr(*clbit)], "")
                    .map_err(build_err)?;
            }
        }
    }
    builder
        .build_unconditional_branch(output)
        .map_err(build_err)?;

    // --- Output recording: one array per classical register, inside a
    // tuple when there is more than one register.
    builder.position_at_end(output);
    let label = |text: String| {
        let bytes = text.as_bytes();
        let ty = context.i8_type().array_type(bytes.len() as u32 + 1);
        let global = module.add_global(ty, None, "");
        global.set_initializer(&context.const_string(bytes, true));
        global.set_constant(true);
        global.set_linkage(Linkage::Internal);
        global.as_pointer_value()
    };
    let mut index = 0usize;
    if circuit.cregs.len() > 1 {
        let tuple = declare(
            "__quantum__rt__tuple_record_output",
            &[i64t.into(), ptr.into()],
        );
        let l = label(format!("{index}_t"));
        index += 1;
        builder
            .build_call(
                tuple,
                &[
                    i64t.const_int(circuit.cregs.len() as u64, false).into(),
                    l.into(),
                ],
                "",
            )
            .map_err(build_err)?;
    }
    let mut clbit = 0u32;
    for (reg_index, reg) in circuit.cregs.iter().enumerate() {
        let prefix = if circuit.cregs.len() > 1 {
            format!("t{reg_index}a")
        } else {
            "a".to_string()
        };
        let array = declare(
            "__quantum__rt__array_record_output",
            &[i64t.into(), ptr.into()],
        );
        let l = label(format!("{index}_{prefix}"));
        index += 1;
        builder
            .build_call(
                array,
                &[i64t.const_int(u64::from(reg.size), false).into(), l.into()],
                "",
            )
            .map_err(build_err)?;
        let record = declare(
            "__quantum__rt__result_record_output",
            &[ptr.into(), ptr.into()],
        );
        for element in 0..reg.size {
            let l = label(format!("{index}_{prefix}{element}r"));
            index += 1;
            builder
                .build_call(record, &[qubit_ptr(clbit), l.into()], "")
                .map_err(build_err)?;
            clbit += 1;
        }
    }
    builder
        .build_return(Some(&i64t.const_zero()))
        .map_err(build_err)?;

    // --- Module flags (behaviors: 1 = Error, 7 = Max). Built as metadata
    // nodes directly because inkwell's `FlagBehavior` has no `Max`.
    let i32t = context.i32_type();
    let flags: [(u64, &str, BasicMetadataValueEnum<'ctx>); 4] = [
        (
            1,
            "qir_major_version",
            i32t.const_int(u64::from(options.major_version), false)
                .into(),
        ),
        (
            7,
            "qir_minor_version",
            i32t.const_int(u64::from(options.minor_version), false)
                .into(),
        ),
        (
            1,
            "dynamic_qubit_management",
            context.bool_type().const_zero().into(),
        ),
        (
            1,
            "dynamic_result_management",
            context.bool_type().const_zero().into(),
        ),
    ];
    for (behavior, key, value) in flags {
        let node = context.metadata_node(&[
            i32t.const_int(behavior, false).into(),
            context.metadata_string(key).into(),
            value,
        ]);
        module
            .add_global_metadata("llvm.module.flags", &node)
            .map_err(|e| QirError::Verify(e.to_string()))?;
    }

    module
        .verify()
        .map_err(|e| QirError::Verify(e.to_string()))?;
    Ok(module)
}

/// Reads base-profile QIR bitcode (as written by this module, by Q#, or by
/// any producer LLVM 22 can read) into a circuit.
pub fn import_qir_bitcode(bitcode: &[u8]) -> Result<Circuit, QirError> {
    let context = Context::create();
    let buffer = MemoryBuffer::create_from_memory_range_copy(bitcode, "qir");
    let module = Module::parse_bitcode_from_buffer(&buffer, &context)
        .map_err(|e| QirError::Import(format!("not LLVM bitcode LLVM 22 can read: {e}")))?;
    import_qir_text(&module.print_to_string().to_string())
}

/// Reads base-profile QIR text into a circuit.
///
/// This is a textual reader on purpose: it accepts both opaque-pointer QIR
/// (`ptr`, QIR 2.0) and the typed-pointer form Q# 1.x still prints
/// (`%Qubit*`, QIR 1.0), which LLVM 22's own parser no longer reads. It
/// understands the entry point's straight-line calls -- QIS gates,
/// measurements, resets, and output recording -- and rejects anything that
/// makes the program more than a circuit (conditional branches, dynamic
/// qubit allocation, classical computation).
pub fn import_qir_text(text: &str) -> Result<Circuit, QirError> {
    let ierr = |line: usize, msg: String| QirError::Import(format!("line {}: {msg}", line + 1));

    // Find the entry point: the definition whose attribute group carries
    // "entry_point" (or the only definition).
    let mut groups: BTreeMap<String, String> = BTreeMap::new();
    for line in text.lines() {
        if let Some(rest) = line.trim().strip_prefix("attributes #") {
            if let Some((id, body)) = rest.split_once('=') {
                groups.insert(id.trim().to_string(), body.trim().to_string());
            }
        }
    }
    let lines: Vec<&str> = text.lines().collect();
    let defines: Vec<usize> = (0..lines.len())
        .filter(|&i| lines[i].trim_start().starts_with("define "))
        .collect();
    let entry_line = defines
        .iter()
        .copied()
        .find(|&i| {
            let l = lines[i];
            l.contains("\"entry_point\"")
                || l.split('#').skip(1).any(|g| {
                    let id: String = g.chars().take_while(char::is_ascii_digit).collect();
                    groups
                        .get(&id)
                        .is_some_and(|body| body.contains("\"entry_point\""))
                })
        })
        .or_else(|| (defines.len() == 1).then(|| defines[0]))
        .ok_or_else(|| {
            QirError::Import("no entry point (no `\"entry_point\"` attribute)".into())
        })?;

    let header = lines[entry_line];
    let name = header
        .split('@')
        .nth(1)
        .and_then(|s| s.split('(').next())
        .map(|s| s.trim_matches('"').to_string())
        .filter(|s| !s.is_empty())
        .ok_or_else(|| ierr(entry_line, "cannot read the entry point's name".into()))?;
    let attr_text = header
        .split('#')
        .skip(1)
        .filter_map(|g| {
            let id: String = g.chars().take_while(char::is_ascii_digit).collect();
            groups.get(&id).cloned()
        })
        .collect::<Vec<_>>()
        .join(" ");
    let required = |key: &str| -> Option<u32> {
        let needle = format!("\"{key}\"=\"");
        let start = attr_text.find(&needle)? + needle.len();
        attr_text[start..].split('"').next()?.parse().ok()
    };

    let mut gates: Vec<Op> = Vec::new();
    let mut max_qubit: Option<u32> = None;
    let mut max_result: Option<u32> = None;
    // Output recording, in order: (kind, count or result index).
    let mut records: Vec<Record> = Vec::new();

    for (offset, raw) in lines[entry_line + 1..].iter().enumerate() {
        let lineno = entry_line + 1 + offset;
        let line = raw.trim();
        if line == "}" {
            break;
        }
        let is_label = line
            .split_whitespace()
            .next()
            .is_some_and(|w| w.ends_with(':'));
        if line.is_empty() || line.starts_with(';') || is_label {
            continue;
        }
        if line.starts_with("br ") {
            if line.starts_with("br i1") {
                return Err(ierr(
                    lineno,
                    "conditional branch: branching on measurement results needs the adaptive \
                     profile, which this importer does not read"
                        .into(),
                ));
            }
            continue;
        }
        if line.starts_with("ret ") {
            continue;
        }
        let Some(call_at) = line.find("call ") else {
            return Err(ierr(
                lineno,
                format!("`{line}` is not a call; base-profile QIR has no classical computation"),
            ));
        };
        let call_text = &line[call_at..];
        let Some(fn_start) = call_text.find('@') else {
            return Err(ierr(lineno, format!("cannot read call `{line}`")));
        };
        let fn_name: String = call_text[fn_start + 1..]
            .chars()
            .take_while(|c| c.is_ascii_alphanumeric() || *c == '_' || *c == '.')
            .collect();
        let args_text = call_text[fn_start + 1 + fn_name.len()..].trim();
        let args = split_args(
            args_text
                .strip_prefix('(')
                .and_then(|s| s.rsplit_once(')'))
                .map_or("", |(inner, _)| inner),
        );
        let values = args
            .iter()
            .map(|a| parse_arg(a))
            .collect::<Result<Vec<_>, String>>()
            .map_err(|e| ierr(lineno, e))?;

        if let Some(rt) = fn_name.strip_prefix("__quantum__rt__") {
            match rt {
                "initialize" => {}
                "tuple_record_output" => records.push(Record::Tuple),
                "array_record_output" => {
                    let n = values.first().and_then(Arg::as_int).ok_or_else(|| {
                        ierr(lineno, "array_record_output without a length".into())
                    })?;
                    records.push(Record::Array(n as u32));
                }
                "result_record_output" => {
                    let r = values.first().and_then(Arg::as_ptr).ok_or_else(|| {
                        ierr(lineno, "result_record_output without a result".into())
                    })?;
                    records.push(Record::Result(r));
                }
                other if other.starts_with("qubit_allocate") || other.starts_with("result_") => {
                    return Err(ierr(
                        lineno,
                        format!("`{fn_name}`: dynamic qubit/result management is not base profile"),
                    ))
                }
                other => {
                    return Err(ierr(
                        lineno,
                        format!("runtime function `__quantum__rt__{other}` is not supported"),
                    ))
                }
            }
            continue;
        }
        let Some(qis) = fn_name.strip_prefix("__quantum__qis__") else {
            return Err(ierr(
                lineno,
                format!("call to `{fn_name}` is not a QIS or runtime function"),
            ));
        };
        let angles: Vec<f64> = values.iter().filter_map(Arg::as_double).collect();
        let ptrs: Vec<u32> = values.iter().filter_map(Arg::as_ptr).collect();
        let mut note_qubits = |qs: &[u32]| {
            for &q in qs {
                max_qubit = Some(max_qubit.map_or(q, |m| m.max(q)));
            }
        };
        match qis {
            "m__body" | "mz__body" | "mresetz__body" => {
                let [q, r] = ptrs[..] else {
                    return Err(ierr(lineno, format!("`{qis}` takes a qubit and a result")));
                };
                note_qubits(&[q]);
                max_result = Some(max_result.map_or(r, |m| m.max(r)));
                gates.push(Op::Measure { qubit: q, clbit: r });
                if qis == "mresetz__body" {
                    gates.push(Op::Reset { qubit: q });
                }
            }
            "reset__body" => {
                note_qubits(&ptrs);
                gates.push(Op::Reset { qubit: ptrs[0] });
            }
            _ => {
                let expanded = qis_to_gates(qis, &angles, &ptrs)
                    .ok_or_else(|| ierr(lineno, format!("unknown quantum instruction `{qis}`")))?;
                note_qubits(&ptrs);
                gates.extend(expanded);
            }
        }
    }

    let num_qubits = required("required_num_qubits")
        .unwrap_or(0)
        .max(max_qubit.map_or(0, |m| m + 1));
    let num_results = required("required_num_results")
        .unwrap_or(0)
        .max(max_result.map_or(0, |m| m + 1));

    // Classical registers from the output records: each recorded array
    // becomes a register; results recorded outside any array share one
    // register `c`; results measured but never recorded go to `unrecorded`.
    let mut cregs: Vec<(String, Vec<u32>)> = Vec::new();
    let mut pending_array: Option<(u32, Vec<u32>)> = None;
    let mut loose: Vec<u32> = Vec::new();
    for record in &records {
        match *record {
            Record::Tuple => {}
            Record::Array(n) => {
                if let Some((_, results)) = pending_array.take() {
                    cregs.push((format!("c{}", cregs.len()), results));
                }
                pending_array = Some((n, Vec::new()));
                if n == 0 {
                    pending_array = None;
                }
            }
            Record::Result(r) => {
                if let Some((n, results)) = &mut pending_array {
                    results.push(r);
                    if results.len() as u32 == *n {
                        let (_, results) = pending_array.take().expect("just matched");
                        cregs.push((format!("c{}", cregs.len()), results));
                    }
                } else {
                    loose.push(r);
                }
            }
        }
    }
    if let Some((_, results)) = pending_array.take() {
        cregs.push((format!("c{}", cregs.len()), results));
    }
    if !loose.is_empty() {
        cregs.push(("c".to_string(), loose));
    }
    if cregs.len() == 1 {
        cregs[0].0 = "c".to_string();
    }
    let recorded: Vec<u32> = cregs
        .iter()
        .flat_map(|(_, rs)| rs.iter().copied())
        .collect();
    let unrecorded: Vec<u32> = (0..num_results).filter(|r| !recorded.contains(r)).collect();
    if !unrecorded.is_empty() {
        cregs.push(("unrecorded".to_string(), unrecorded));
    }
    let order: Vec<u32> = cregs
        .iter()
        .flat_map(|(_, rs)| rs.iter().copied())
        .collect();
    let clbit_of = |r: u32| order.iter().position(|&x| x == r).map(|p| p as u32);

    let mut circuit = Circuit {
        name,
        qregs: if num_qubits > 0 {
            vec![Register::new("q", num_qubits)]
        } else {
            vec![]
        },
        cregs: cregs
            .iter()
            .map(|(n, rs)| Register::new(n.clone(), rs.len() as u32))
            .collect(),
        ops: Vec::with_capacity(gates.len()),
    };
    for op in gates {
        circuit.ops.push(match op {
            Op::Measure { qubit, clbit } => Op::Measure {
                qubit,
                clbit: clbit_of(clbit).ok_or_else(|| {
                    QirError::Import(format!("result {clbit} is measured but out of range"))
                })?,
            },
            other => other,
        });
    }
    circuit.validate()?;
    Ok(circuit)
}

#[derive(Debug, Clone, Copy)]
enum Record {
    Tuple,
    Array(u32),
    Result(u32),
}

#[derive(Debug, Clone, Copy, PartialEq)]
enum Arg {
    Int(u64),
    Double(f64),
    Ptr(u32),
    /// A label or other non-index pointer (`ptr @0`, a `getelementptr`).
    Label,
}

impl Arg {
    fn as_int(&self) -> Option<u64> {
        match self {
            Arg::Int(v) => Some(*v),
            _ => None,
        }
    }
    fn as_double(&self) -> Option<f64> {
        match self {
            Arg::Double(v) => Some(*v),
            _ => None,
        }
    }
    fn as_ptr(&self) -> Option<u32> {
        match self {
            Arg::Ptr(v) => Some(*v),
            _ => None,
        }
    }
}

/// Splits a call's argument list on top-level commas.
fn split_args(s: &str) -> Vec<String> {
    let mut out = Vec::new();
    let mut depth = 0i32;
    let mut cur = String::new();
    for c in s.chars() {
        match c {
            '(' | '[' | '{' => depth += 1,
            ')' | ']' | '}' => depth -= 1,
            ',' if depth == 0 => {
                out.push(cur.trim().to_string());
                cur.clear();
                continue;
            }
            _ => {}
        }
        cur.push(c);
    }
    if !cur.trim().is_empty() {
        out.push(cur.trim().to_string());
    }
    out
}

fn parse_arg(arg: &str) -> Result<Arg, String> {
    let words: Vec<&str> = arg.split_whitespace().collect();
    let Some(&ty) = words.first() else {
        return Err("empty argument".into());
    };
    // Skip parameter attributes between the type and the value.
    let value = words
        .iter()
        .skip(1)
        .find(|w| {
            !matches!(
                **w,
                "writeonly" | "readonly" | "noundef" | "nonnull" | "nocapture"
            )
        })
        .copied()
        .unwrap_or("");
    if ty == "double" {
        return parse_double(value)
            .map(Arg::Double)
            .ok_or_else(|| format!("bad double `{value}`"));
    }
    if ty == "i64" {
        return value
            .parse::<u64>()
            .map(Arg::Int)
            .map_err(|_e| format!("bad integer `{value}`"));
    }
    if ty == "ptr" || ty.ends_with('*') {
        if value == "null" {
            return Ok(Arg::Ptr(0));
        }
        if value.starts_with("inttoptr") {
            let inner = arg
                .split("i64")
                .nth(1)
                .and_then(|s| s.split_whitespace().next())
                .ok_or_else(|| format!("bad inttoptr `{arg}`"))?;
            return inner
                .parse::<u32>()
                .map(Arg::Ptr)
                .map_err(|_e| format!("bad qubit/result index in `{arg}`"));
        }
        if value.starts_with('@') || value.starts_with("getelementptr") {
            return Ok(Arg::Label);
        }
        return Err(format!(
            "pointer argument `{arg}` is not a constant qubit or result (dynamic management is not \
             base profile)"
        ));
    }
    Err(format!("unsupported argument `{arg}`"))
}

/// LLVM prints a double as a decimal when that round-trips exactly, and
/// otherwise as `0x` followed by the IEEE-754 bits in hex.
fn parse_double(s: &str) -> Option<f64> {
    if let Some(hex) = s.strip_prefix("0x") {
        return u64::from_str_radix(hex, 16).ok().map(f64::from_bits);
    }
    s.parse().ok()
}

/// Maps a QIS function (without prefix) back to IR gates.
fn qis_to_gates(name: &str, angles: &[f64], q: &[u32]) -> Option<Vec<Op>> {
    let g = |gate: Gate, qubits: &[u32]| Op::Gate {
        gate,
        qubits: qubits.to_vec(),
    };
    let arity_ok = |n: usize| q.len() == n;
    let ops = match (name, angles) {
        ("h__body", []) if arity_ok(1) => vec![g(Gate::H, q)],
        ("x__body", []) if arity_ok(1) => vec![g(Gate::X, q)],
        ("y__body", []) if arity_ok(1) => vec![g(Gate::Y, q)],
        ("z__body", []) if arity_ok(1) => vec![g(Gate::Z, q)],
        ("s__body", []) if arity_ok(1) => vec![g(Gate::S, q)],
        ("s__adj", []) if arity_ok(1) => vec![g(Gate::Sdg, q)],
        ("t__body", []) if arity_ok(1) => vec![g(Gate::T, q)],
        ("t__adj", []) if arity_ok(1) => vec![g(Gate::Tdg, q)],
        ("sx__body", []) if arity_ok(1) => vec![g(Gate::Sx, q)],
        ("rx__body", [a]) if arity_ok(1) => vec![g(Gate::Rx(*a), q)],
        ("ry__body", [a]) if arity_ok(1) => vec![g(Gate::Ry(*a), q)],
        ("rz__body", [a]) if arity_ok(1) => vec![g(Gate::Rz(*a), q)],
        ("cx__body" | "cnot__body", []) if arity_ok(2) => vec![g(Gate::Cx, q)],
        ("cy__body", []) if arity_ok(2) => vec![g(Gate::Cy, q)],
        ("cz__body", []) if arity_ok(2) => vec![g(Gate::Cz, q)],
        ("swap__body", []) if arity_ok(2) => vec![g(Gate::Swap, q)],
        ("ccx__body", []) if arity_ok(3) => vec![g(Gate::Ccx, q)],
        ("rzz__body", [a]) if arity_ok(2) => {
            vec![g(Gate::Cx, q), g(Gate::Rz(*a), &q[1..]), g(Gate::Cx, q)]
        }
        ("rxx__body", [a]) if arity_ok(2) => vec![
            g(Gate::H, &q[..1]),
            g(Gate::H, &q[1..]),
            g(Gate::Cx, q),
            g(Gate::Rz(*a), &q[1..]),
            g(Gate::Cx, q),
            g(Gate::H, &q[..1]),
            g(Gate::H, &q[1..]),
        ],
        ("ryy__body", [a]) if arity_ok(2) => vec![
            g(Gate::Rx(FRAC_PI_2), &q[..1]),
            g(Gate::Rx(FRAC_PI_2), &q[1..]),
            g(Gate::Cx, q),
            g(Gate::Rz(*a), &q[1..]),
            g(Gate::Cx, q),
            g(Gate::Rx(-FRAC_PI_2), &q[..1]),
            g(Gate::Rx(-FRAC_PI_2), &q[1..]),
        ],
        _ => return None,
    };
    Some(ops)
}

#[cfg(test)]
mod tests;
