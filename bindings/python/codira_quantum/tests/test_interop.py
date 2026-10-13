# Copyright (c) 2026 Omnira CJSC
# Author: Tunjay Akbarli
# Date: October 13, 2026
#
# Functionality:
# - Cross-checks Codira's quantum support against the real libraries:
#   Qiskit (exact statevector probabilities, both import directions, OpenQASM
#   2 and 3), pyqir (parses and verifies Codira's QIR bitcode), qir-runner
#   (executes it), and the QDK (`qsharp`: compiles Codira's OpenQASM 3 to
#   its own QIR, which Codira imports and must agree with exactly).
#
# Run with the `codira` binary on PATH or CODIRA=/path/to/codira:
#   python -m unittest discover -s bindings/python/codira_quantum/tests
# Each library is optional; a missing one skips its tests, never passes them.

import importlib.util
import math
import os
import random
import re
import sys
import tempfile
import unittest
import warnings
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import codira_quantum as cq  # noqa: E402

warnings.simplefilter("ignore", DeprecationWarning)


def have(module):
    return importlib.util.find_spec(module) is not None


PRELUDE = None


def prelude():
    global PRELUDE
    if PRELUDE is None:
        PRELUDE = cq._codira("prelude")
    return PRELUDE


def write_kernel(directory, name, body):
    path = Path(directory) / f"{name}.code"
    path.write_text(prelude() + "\n" + body)
    return path


BELL = """
@target(qpu)
func bell() {
    h(0);
    cx(0, 1);
    measure(0, 0);
    measure(1, 1);
}
"""

GHZ = """
@target(qpu)
func ghz(n: i64) {
    h(0);
    let mut i = 1;
    while i < n {
        cx(i - 1, i);
        i += 1;
    }
    let mut j = 0;
    while j < n {
        measure(j, j);
        j += 1;
    }
}
"""

# Every gate of the Codira surface, with angles, on 4 qubits.
EVERYTHING = """
@target(qpu)
func everything(theta: f64) {
    h(0); x(1); y(2); z(3); s(0); sdg(1); t(2); tdg(3); sx(0);
    rx(theta, 1); ry(theta * 2.0, 2); rz(0.0 - theta, 3); p(theta / 3.0, 0);
    u(theta, 0.25, -0.75, 1);
    cx(0, 1); cy(1, 2); cz(2, 3); ch(3, 0); swap(0, 2);
    cp(theta, 1, 3); crx(theta, 2, 0); cry(theta, 3, 1); crz(theta, 0, 2);
    ccx(0, 1, 2); cswap(3, 0, 1);
    h(1); cx(1, 3);
    measure(0, 0); measure(1, 1); measure(2, 2); measure(3, 3);
}
"""


def qiskit_exact_outcomes(circuit):
    """Exact outcome distribution of a measurement-terminal Qiskit circuit,
    computed from Qiskit's own Statevector, keyed like get_counts()."""
    from qiskit.quantum_info import Statevector

    measures = []
    for inst in circuit.data:
        if inst.operation.name == "measure":
            q = circuit.find_bit(inst.qubits[0]).index
            c = circuit.find_bit(inst.clbits[0]).index
            measures.append((q, c))
    unitary_part = circuit.remove_final_measurements(inplace=False)
    probs = Statevector.from_instruction(unitary_part).probabilities()
    sizes = [creg.size for creg in circuit.cregs]
    out = {}
    for index, p in enumerate(probs):
        if p < 1e-12:
            continue
        bits = [0] * circuit.num_clbits
        for q, c in measures:
            bits[c] = (index >> q) & 1
        parts, start = [], 0
        for size in sizes:
            parts.append("".join(str(bits[i]) for i in reversed(range(start, start + size))))
            start += size
        key = " ".join(reversed(parts))
        out[key] = out.get(key, 0.0) + p
    return out


def assert_same_distribution(test, a, b, tol=1e-9):
    keys = set(a) | set(b)
    for k in keys:
        test.assertAlmostEqual(a.get(k, 0.0), b.get(k, 0.0), delta=tol, msg=f"outcome {k}")


@unittest.skipUnless(have("qiskit"), "qiskit is not installed")
class QiskitTests(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.tmp = self.dir.name

    def tearDown(self):
        self.dir.cleanup()

    def test_codira_kernels_agree_with_qiskit_exactly(self):
        cases = [
            (write_kernel(self.tmp, "bell", BELL), {}),
            (write_kernel(self.tmp, "ghz", GHZ), {"n": 5}),
            (write_kernel(self.tmp, "everything", EVERYTHING), {"theta": 0.7}),
        ]
        for path, params in cases:
            with self.subTest(kernel=path.stem):
                ours = cq.probabilities(path, params=params)
                theirs = qiskit_exact_outcomes(cq.to_qiskit(path, params=params))
                assert_same_distribution(self, ours, theirs)

    @unittest.skipUnless(have("qiskit_qasm3_import"), "qiskit-qasm3-import is not installed")
    def test_openqasm3_path_into_qiskit(self):
        path = write_kernel(self.tmp, "everything", EVERYTHING)
        ours = cq.probabilities(path, params={"theta": 1.3})
        theirs = qiskit_exact_outcomes(cq.to_qiskit(path, params={"theta": 1.3}, qasm_version=3))
        assert_same_distribution(self, ours, theirs)

    def test_qiskit_circuits_come_into_codira(self):
        from qiskit import ClassicalRegister, QuantumCircuit, QuantumRegister
        from qiskit.circuit.library import QFTGate

        rng = random.Random(1234)

        def angle():
            return rng.uniform(-math.pi, math.pi)

        # Qiskit's standard gates, including the ones Codira imports through
        # its built-in library, over several registers.
        qr, anc = QuantumRegister(3, "q"), QuantumRegister(2, "anc")
        a, b = ClassicalRegister(2, "a"), ClassicalRegister(3, "b")
        qc = QuantumCircuit(qr, anc, a, b, name="mixed")
        for q in range(5):
            qc.h(q)
        qc.rzz(angle(), 0, 1)
        qc.rxx(angle(), 1, 2)
        qc.ryy(angle(), 2, 3)
        qc.rzx(angle(), 3, 4)
        qc.cu(angle(), angle(), angle(), angle(), 4, 0)
        qc.csx(0, 2)
        qc.sxdg(1)
        qc.ccz(0, 1, 2)
        qc.iswap(3, 4)
        qc.ecr(1, 3)
        qc.dcx(2, 4)
        qc.cs(0, 4)
        qc.csdg(1, 4)
        qc.r(angle(), angle(), 2)
        qc.u(angle(), angle(), angle(), 3)
        qc.p(angle(), 0)
        qc.cp(angle(), 1, 0)
        qc.crx(angle(), 2, 1)
        qc.cry(angle(), 3, 2)
        qc.crz(angle(), 4, 3)
        qc.ch(0, 4)
        qc.cswap(1, 2, 3)
        qc.ccx(4, 0, 1)
        qc.barrier()
        qc.measure([0, 1, 2, 3, 4], [b[2], a[0], b[0], a[1], b[1]])

        qft = QuantumCircuit(4, 4, name="qft_on_9")
        qft.x(0)
        qft.x(3)
        qft.append(QFTGate(4), range(4))
        qft = qft.decompose()
        qft.measure(range(4), range(4))

        for circuit in [qc, qft]:
            with self.subTest(circuit=circuit.name):
                code = cq.from_qiskit(circuit, path=Path(self.tmp) / f"{circuit.name}.code")
                self.assertIn("@target(qpu)", code.read_text())
                ours = cq.probabilities(code)
                # The generated kernel has one flat classical register; its
                # bits are Qiskit's registers concatenated in declaration
                # order, so the keys agree once Qiskit's separators go.
                theirs = {}
                for key, p in qiskit_exact_outcomes(circuit).items():
                    flat = key.replace(" ", "")
                    theirs[flat] = theirs.get(flat, 0.0) + p
                assert_same_distribution(self, ours, theirs)

    def test_sampled_counts_are_consistent_with_qiskit(self):
        path = write_kernel(self.tmp, "everything", EVERYTHING)
        theirs = qiskit_exact_outcomes(cq.to_qiskit(path, params={"theta": 0.4}))
        shots = 20000
        counts = cq.run(path, shots=shots, seed=11, params={"theta": 0.4})
        self.assertEqual(sum(counts.values()), shots)
        self.assertEqual(counts, cq.run(path, shots=shots, seed=11, params={"theta": 0.4}))
        for outcome, n in counts.items():
            self.assertIn(outcome, theirs)
        for outcome, p in theirs.items():
            sigma = math.sqrt(p * (1 - p) / shots)
            self.assertLess(abs(counts.get(outcome, 0) / shots - p), 5 * sigma + 1e-9, outcome)


@unittest.skipUnless(have("pyqir"), "pyqir is not installed")
class PyqirTests(unittest.TestCase):
    def test_bitcode_parses_verifies_and_declares_the_base_profile(self):
        import pyqir

        with tempfile.TemporaryDirectory() as d:
            path = write_kernel(d, "ghz", GHZ)
            bitcode = cq.to_qir(path, params={"n": 3})
        module = pyqir.Module.from_bitcode(pyqir.Context(), bitcode)
        self.assertIsNone(module.verify())
        entries = [f for f in module.functions if pyqir.is_entry_point(f)]
        self.assertEqual(len(entries), 1)
        entry = entries[0]
        self.assertEqual(entry.name, "ghz")
        self.assertEqual(pyqir.required_num_qubits(entry), 3)
        self.assertEqual(pyqir.required_num_results(entry), 3)
        self.assertEqual(entry.attributes.func["qir_profiles"].string_value, "base_profile")
        flags = {
            name: module.get_flag(name)
            for name in [
                "qir_major_version",
                "qir_minor_version",
                "dynamic_qubit_management",
                "dynamic_result_management",
            ]
        }
        for name, flag in flags.items():
            self.assertIsNotNone(flag, name)
        names = {f.name for f in module.functions}
        for needed in [
            "__quantum__rt__initialize",
            "__quantum__qis__h__body",
            "__quantum__qis__cx__body",
            "__quantum__qis__mz__body",
            "__quantum__rt__array_record_output",
            "__quantum__rt__result_record_output",
        ]:
            self.assertIn(needed, names)


def parse_runner_output(lines):
    """Shots from qir-runner's output: one list of RESULT bits per shot."""
    shots, current = [], None
    for line in "".join(str(item) for item in lines).splitlines():
        fields = line.split("\t")
        if fields[0] == "START":
            current = []
        elif fields[0] == "OUTPUT" and fields[1] == "RESULT":
            current.append(int(fields[2]))
        elif fields[0] == "END":
            shots.append(current)
    return shots


@unittest.skipUnless(have("qirrunner"), "qirrunner is not installed")
class QirRunnerTests(unittest.TestCase):
    def run_bitcode(self, bitcode, shots):
        import qirrunner

        with tempfile.TemporaryDirectory() as d:
            bc = Path(d) / "kernel.bc"
            bc.write_bytes(bitcode)
            out = []
            qirrunner.run(str(bc), shots=shots, rng_seed=5, output_fn=out.append)
        return parse_runner_output(out)

    def test_deterministic_kernel_gives_the_same_bits(self):
        body = """
@target(qpu)
func pattern() {
    x(0); x(2); h(3); h(3);
    cx(2, 3); swap(0, 1); ccx(1, 2, 4);
    measure(0, 0); measure(1, 1); measure(2, 2); measure(3, 3); measure(4, 4);
}
"""
        with tempfile.TemporaryDirectory() as d:
            path = write_kernel(d, "pattern", body)
            expected = cq.probabilities(path)
            bitcode = cq.to_qir(path)
        self.assertEqual(len(expected), 1)
        (key,) = expected
        bits = [int(b) for b in reversed(key)]  # result 0 first, as QIR records
        for shot in self.run_bitcode(bitcode, shots=8):
            self.assertEqual(shot, bits)

    def test_bell_shots_are_correlated_and_both_outcomes_appear(self):
        with tempfile.TemporaryDirectory() as d:
            bitcode = cq.to_qir(write_kernel(d, "bell", BELL))
        shots = self.run_bitcode(bitcode, shots=200)
        self.assertEqual(len(shots), 200)
        self.assertTrue(all(s[0] == s[1] for s in shots))
        self.assertEqual({tuple(s) for s in shots}, {(0, 0), (1, 1)})

    def test_everything_kernel_matches_exact_distribution_statistically(self):
        with tempfile.TemporaryDirectory() as d:
            path = write_kernel(d, "everything", EVERYTHING)
            expected = cq.probabilities(path, params={"theta": 0.9})
            bitcode = cq.to_qir(path, params={"theta": 0.9})
        n = 4000
        shots = self.run_bitcode(bitcode, shots=n)
        counts = {}
        for s in shots:
            key = "".join(str(b) for b in reversed(s))
            counts[key] = counts.get(key, 0) + 1
        for outcome in counts:
            self.assertIn(outcome, expected)
        for outcome, p in expected.items():
            sigma = math.sqrt(p * (1 - p) / n)
            self.assertLess(abs(counts.get(outcome, 0) / n - p), 5 * sigma + 1e-9, outcome)


@unittest.skipUnless(have("qsharp") or have("qdk"), "qsharp (QDK) is not installed")
class QdkTests(unittest.TestCase):
    def test_qdk_compiles_codira_openqasm3_to_qir_that_codira_reads_back_identically(self):
        import qsharp
        import qsharp.openqasm as oq

        with tempfile.TemporaryDirectory() as d:
            path = write_kernel(d, "everything", EVERYTHING)
            ours = cq.probabilities(path, params={"theta": 0.55})
            qasm3 = cq.to_qasm(path, version=3, params={"theta": 0.55})
            qir = str(oq.compile(qasm3, target_profile=qsharp.TargetProfile.Base))
            self.assertIn("__quantum__qis__", qir)
            ll = Path(d) / "from_qdk.ll"
            ll.write_text(qir)
            through_qdk = cq.probabilities(ll)
        assert_same_distribution(self, ours, through_qdk)

    def test_qdk_simulator_sees_the_bell_correlation(self):
        import qsharp.openqasm as oq

        with tempfile.TemporaryDirectory() as d:
            qasm3 = cq.to_qasm(write_kernel(d, "bell", BELL), version=3)
        results = oq.run(qasm3, shots=100, seed=3)
        self.assertEqual(len(results), 100)
        for shot in results:
            self.assertEqual(str(shot[0]), str(shot[1]))


if __name__ == "__main__":
    unittest.main()
