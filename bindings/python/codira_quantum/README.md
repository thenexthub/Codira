<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: October 13, 2026

Functionality: Usage and verification record of the codira-quantum Python
package (Qiskit / QDK / QIR interop for Codira @target(qpu) kernels).
-->

# codira-quantum (Python)

Moves quantum circuits between Codira `@target(qpu)` kernels, Qiskit, and
QIR for the QDK / Azure Quantum. Pure Python with no required dependencies:
every function calls the `codira quantum` CLI (put `codira` on `PATH` or set
`CODIRA=/path/to/codira`), so the Codira compiler stays the single source of
truth for elaboration, OpenQASM and QIR.

```python
import codira_quantum as cq

qc = cq.to_qiskit("ghz.code", params={"n": 5})        # qiskit.QuantumCircuit
path = cq.from_qiskit(qc, path="from_qiskit.code")    # a .code file with a @target(qpu) kernel
bitcode = cq.to_qir("ghz.code", params={"n": 5})      # QIR base-profile bitcode (bytes)
text = cq.to_qir("ghz.code", params={"n": 5}, text=True)
qasm3 = cq.to_qasm("ghz.code", version=3, params={"n": 5})
counts = cq.run("ghz.code", shots=1000, seed=7, params={"n": 5})
exact = cq.probabilities("ghz.code", params={"n": 5})
```

`to_qiskit` goes through OpenQASM 2 (`QuantumCircuit.from_qasm_str`, needs
only Qiskit) unless `qasm_version=3` (`qiskit.qasm3.loads`, needs
`qiskit-qasm3-import`). `from_qiskit` goes through `qiskit.qasm2.dumps`.

For the QDK: `to_qir` bitcode is the base-profile input format QIR
consumers take; the QDK's own OpenQASM 3 compiler (`qsharp.openqasm.compile`)
also accepts `to_qasm(..., version=3)` directly.

## Verification

`tests/test_interop.py` (`python -m unittest discover -s tests`) checks
against the real libraries; each library missing from the environment skips
its tests rather than passing them. Results are recorded in the crate
README (`crates/codira_quantum/README.md`, section 6).
