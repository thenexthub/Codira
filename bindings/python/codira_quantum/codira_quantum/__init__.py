# Copyright (c) 2026 Omnira CJSC
# Author: Tunjay Akbarli
# Date: October 13, 2026
#
# Functionality:
# - Python side of Codira's quantum support. Every function drives the
#   `codira quantum` CLI, so the Codira compiler stays the single source of
#   truth for elaboration, OpenQASM and QIR; this package only moves files
#   and objects across the boundary.
#
#   to_qiskit(entry)    -> qiskit.QuantumCircuit (through OpenQASM 2 or 3)
#   from_qiskit(circ)   -> path of a generated .code file with a @target(qpu)
#                          kernel (through OpenQASM 2)
#   to_qir(entry)       -> QIR base-profile bitcode bytes (or text), the
#                          input format QDK / Azure Quantum targets take
#   to_qasm(entry)      -> OpenQASM 2 or 3 text
#   run(entry)          -> {outcome: count} from the built-in simulator
#   probabilities(entry)-> {outcome: exact probability}
#
# `entry` is a `.code` file (optionally `file.code::kernel`) or a circuit
# file (`.qasm`, `.ll`, `.bc`). Kernel parameters are compile-time values:
# pass them as `params={"n": 4}`.
"""Move quantum circuits between Codira, Qiskit, and QIR (QDK / Azure Quantum)."""

from __future__ import annotations

import json
import os
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any, Dict, Mapping, Optional, Union

__all__ = [
    "CodiraError",
    "codira_executable",
    "to_qasm",
    "to_qir",
    "to_qiskit",
    "from_qiskit",
    "run",
    "probabilities",
]

PathLike = Union[str, os.PathLike]


class CodiraError(RuntimeError):
    """The `codira` CLI reported an error; the message is its stderr."""


def codira_executable() -> str:
    """The `codira` binary: `$CODIRA` if set, else `codira` on PATH."""
    explicit = os.environ.get("CODIRA")
    if explicit:
        return explicit
    found = shutil.which("codira")
    if found:
        return found
    raise CodiraError(
        "cannot find the `codira` executable; put it on PATH or set CODIRA=/path/to/codira"
    )


def _entry_args(
    entry: PathLike, params: Optional[Mapping[str, Any]], kernel: Optional[str]
) -> list:
    args = [os.fspath(entry)]
    if kernel is not None:
        args += ["--kernel", kernel]
    for name, value in (params or {}).items():
        if isinstance(value, bool):
            value = "true" if value else "false"
        elif isinstance(value, float):
            value = repr(value)
        args += ["--param", f"{name}={value}"]
    return args


def _codira(*args: str, binary: bool = False) -> Union[str, bytes]:
    proc = subprocess.run(
        [codira_executable(), "quantum", *args],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        raise CodiraError(proc.stderr.decode(errors="replace").strip() or "codira failed")
    return proc.stdout if binary else proc.stdout.decode()


def to_qasm(
    entry: PathLike,
    version: int = 3,
    params: Optional[Mapping[str, Any]] = None,
    kernel: Optional[str] = None,
) -> str:
    """OpenQASM text (version 2 or 3) for a kernel or circuit file."""
    if version not in (2, 3):
        raise ValueError("OpenQASM version must be 2 or 3")
    return _codira("build", *_entry_args(entry, params, kernel), "--emit", f"qasm{version}")


def to_qir(
    entry: PathLike,
    params: Optional[Mapping[str, Any]] = None,
    kernel: Optional[str] = None,
    text: bool = False,
    qir_major_version: int = 2,
) -> Union[bytes, str]:
    """QIR base-profile for a kernel: LLVM bitcode bytes by default (what
    `azure.quantum` targets accept as input data), or LLVM IR text."""
    common = [*_entry_args(entry, params, kernel), "--qir-major", str(qir_major_version)]
    if text:
        return _codira("build", *common, "--emit", "qir")
    with tempfile.TemporaryDirectory() as d:
        out = Path(d) / "kernel.bc"
        _codira("build", *common, "--emit", "qir-bc", "-o", str(out))
        return out.read_bytes()


def to_qiskit(
    entry: PathLike,
    params: Optional[Mapping[str, Any]] = None,
    kernel: Optional[str] = None,
    qasm_version: int = 2,
):
    """A `qiskit.QuantumCircuit` for a kernel or circuit file.

    OpenQASM 2 (the default) loads with `QuantumCircuit.from_qasm_str`, which
    needs nothing beyond Qiskit. OpenQASM 3 uses `qiskit.qasm3.loads`, which
    needs the separate `qiskit-qasm3-import` package.
    """
    from qiskit import QuantumCircuit  # deferred: Qiskit is optional

    source = to_qasm(entry, version=qasm_version, params=params, kernel=kernel)
    if qasm_version == 2:
        return QuantumCircuit.from_qasm_str(source)
    from qiskit import qasm3

    return qasm3.loads(source)


def from_qiskit(
    circuit,
    path: Optional[PathLike] = None,
    name: Optional[str] = None,
) -> Path:
    """Writes a Qiskit circuit as a Codira `.code` file containing one
    `@target(qpu)` kernel, and returns its path.

    The circuit goes through `qiskit.qasm2.dumps` and `codira quantum
    import`. Gates outside Codira's set that Qiskit commonly uses (`rzz`,
    `cu`, `csx`, ...) are expanded into Codira gates; anything Qiskit cannot
    write as OpenQASM 2 (classical control, for instance) fails here too.
    """
    from qiskit import qasm2

    kernel = name or "".join(c if c.isalnum() else "_" for c in (circuit.name or "kernel"))
    if not (kernel[0].isalpha() or kernel[0] == "_"):
        kernel = "_" + kernel
    target = Path(path) if path is not None else Path.cwd() / f"{kernel}.code"
    with tempfile.TemporaryDirectory() as d:
        qasm_file = Path(d) / f"{kernel}.qasm"
        qasm_file.write_text(qasm2.dumps(circuit))
        _codira("import", str(qasm_file), "--name", kernel, "-o", str(target))
    return target


def run(
    entry: PathLike,
    shots: int = 1024,
    seed: int = 0,
    params: Optional[Mapping[str, Any]] = None,
    kernel: Optional[str] = None,
) -> Dict[str, int]:
    """Samples a kernel on Codira's built-in simulator. Keys follow Qiskit's
    `get_counts()` format (highest classical bit first)."""
    out = _codira(
        "run",
        *_entry_args(entry, params, kernel),
        "--shots",
        str(shots),
        "--seed",
        str(seed),
        "--json",
    )
    return {k: int(v) for k, v in json.loads(out).items()}


def probabilities(
    entry: PathLike,
    params: Optional[Mapping[str, Any]] = None,
    kernel: Optional[str] = None,
) -> Dict[str, float]:
    """Exact outcome probabilities from Codira's simulator."""
    out = _codira("run", *_entry_args(entry, params, kernel), "--exact", "--json")
    return {k: float(v) for k, v in json.loads(out).items()}
