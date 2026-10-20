"""Authors the stream aggregator history (v01..v21) and its manifest.

Every update here is invented for this evaluation. Where a change follows a
well-known class of change, `provenance` says "pattern" and `derivation`
names the class; none is a reproduction of a specific upstream commit.

Run:  python eval/authoring/author_stream.py   (writes eval/workloads/stream)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from authorlib import Update, write_history  # noqa: E402
import stream_v01  # noqa: E402

H = "src/hist.code"
W = "src/window.code"
P = "src/pipeline.code"
M = "src/mod.code"

IMPORT_W = "import root.window.{ Agg, agg_new, agg_pct, agg_window_sum }"
LAST_ELSE = "    } else {\n        -2\n    }\n}"


def bucket_chain(realtime=True):
    bounds = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]
    out = []
    if realtime:
        out.append("@realtime")
    out.append("public func bucket_of(v: i64) -> i64")
    out.append("    ensures result >= 0 && result <= 14")
    out.append("{")
    for k, b in enumerate(bounds):
        head = "    if" if k == 0 else "    } else if"
        out.append(f"{head} v < {b} {{")
        out.append(f"        {k}")
    out.append("    } else {")
    out.append("        14")
    out.append("    }")
    out.append("}")
    return "\n".join(out)


U = []

U.append(Update(
    id="stream-u01", category="bug-fix",
    title="bucket boundaries: v in [2^(k-1), 2^k) lands in bucket k",
    provenance="pattern",
    derivation="Class: off-by-one in a logarithmic bucket function (`x > 1` where `x > 0` is meant), so every value "
               "landed one bucket low and a percentile under-reported.",
    behaviour="changed",
    notes="Counts already in the histogram stay in their old (wrong) buckets.",
    edits=[(H, "    while x > 1 && b < 14\n", "    while x > 0 && b < 14\n")],
))

U.append(Update(
    id="stream-u02", category="bug-fix",
    title="late events are counted and dropped instead of overwriting a newer window",
    provenance="invented",
    derivation="An event whose window is older than the one in its ring slot reset the slot and destroyed the newer window.",
    behaviour="changed",
    edits=[(W, '''    if a.win_id[slot] != w {
        a.win_id[slot] = w
        a.win_sum[slot] = 0
    }
''', '''    if a.win_id[slot] != w {
        if w < a.win_id[slot] {
            a.late += 1
            return
        }
        a.win_id[slot] = w
        a.win_sum[slot] = 0
    }
''')],
))

U.append(Update(
    id="stream-u03", category="bug-fix",
    title="recycling a window slot also resets its event count",
    provenance="invented",
    derivation="Only the sum was reset, so the count of a recycled slot kept growing.",
    behaviour="changed",
    edits=[(W, "        a.win_sum[slot] = 0\n    }\n    a.win_sum[slot] += val", "        a.win_sum[slot] = 0\n        a.win_cnt[slot] = 0\n    }\n    a.win_sum[slot] += val")],
))

U.append(Update(
    id="stream-u04", category="refactor",
    title="extract window_of",
    provenance="invented",
    derivation="Extract-function refactor of `ts / 64`, kept real-time.",
    behaviour="preserved",
    edits=[
        (W, "    let w = ts / 64\n", "    let w = window_of(ts)\n"),
        (W, "// Events are folded into a ring of 8 windows of 64 time units each.\n",
            "@realtime\nfunc window_of(ts: i64) -> i64 {\n    ts / 64\n}\n\n// Events are folded into a ring of 8 windows of 64 time units each.\n"),
    ],
))

U.append(Update(
    id="stream-u05", category="bug-fix",
    title="percentile target rounds up and is at least one event",
    provenance="pattern",
    derivation="Class: floor instead of ceiling in a nearest-rank percentile; small totals returned the lowest bucket.",
    behaviour="changed",
    edits=[(W, "    let target = (a.total * p) / 100\n", "    let want = (a.total * p + 99) / 100\n    let target = if want < 1 { 1 } else { want }\n")],
))

U.append(Update(
    id="stream-u06", category="performance",
    title="four workers instead of three",
    provenance="invented",
    derivation="A tuning change of a constant (worker count) in the pipeline; folding is commutative, so results do not change.",
    behaviour="preserved",
    edits=[
        (P, "        while w < 3 {\n            spawn worker(jobs, results)\n", "        while w < 4 {\n            spawn worker(jobs, results)\n"),
        (P, "        while w < 3 {\n            jobs <- -1\n", "        while w < 4 {\n            jobs <- -1\n"),
    ],
))

U.append(Update(
    id="stream-u07", category="refactor",
    title="extract reduce_one from the batch loop",
    provenance="invented",
    derivation="The unpack-and-fold of one result moves into its own function.",
    behaviour="preserved",
    edits=[
        (P, '''            let r = <-results
            let ts = (r >> 24) - 1073741824
            let b = (r >> 20) & 15
            let v = r & 1048575
            agg_add(a, ts, b, v)
            got += 1
''', '''            let r = <-results
            reduce_one(a, r)
            got += 1
'''),
        (P, "// Folds n events", '''func reduce_one(a: Agg, r: i64) {
    let ts = (r >> 24) - 1073741824
    let b = (r >> 20) & 15
    let v = r & 1048575
    agg_add(a, ts, b, v)
}

// Folds n events'''),
    ],
))

U.append(Update(
    id="stream-u08", category="regression",
    title="ring slot as `w % 8` (reads better than a mask)",
    provenance="pattern",
    derivation="Class: replacing a bit mask by a modulo for readability. For a negative window (an event from a clock "
               "that ran behind) `%` is negative and the slot index is out of bounds, which stops the process.",
    behaviour="changed",
    notes="Seeded bug that looks plausible in review; only negative timestamps trigger it.",
    edits=[(W, "    let slot = w & 7\n    if a.win_id[slot] != w {", "    let slot = w % 8\n    if a.win_id[slot] != w {")],
))

U.append(Update(
    id="stream-u09", category="bug-fix",
    title="revert u08: ring slot masked again",
    provenance="invented",
    derivation="Revert after the first negative timestamp stopped a worker process.",
    behaviour="changed",
    edits=[(W, "    let slot = w % 8\n    if a.win_id[slot] != w {", "    let slot = w & 7\n    if a.win_id[slot] != w {")],
))

U.append(Update(
    id="stream-u10", category="feature",
    title="running sum and mean",
    provenance="invented",
    derivation="Agg gains sum_all (zero is right) and agg_mean; request op 5.",
    behaviour="new-api",
    migrate="zero-default-ok",
    notes="sum_all counts only events after the update for a hot-updated aggregate.",
    edits=[
        (W, "    public newest: i64,\n}", "    public newest: i64,\n    public sum_all: i64,\n}"),
        (W, "        newest: -7,\n    }", "        newest: -7,\n        sum_all: 0,\n    }"),
        (W, "    a.total += 1\n", "    a.total += 1\n    a.sum_all += val\n"),
        (W, "@noalloc\npublic func agg_window_sum", "@noalloc\npublic func agg_mean(a: Agg) -> i64 {\n    if a.total == 0 { return 0 }\n    a.sum_all / a.total\n}\n\n@noalloc\npublic func agg_window_sum"),
        (M, IMPORT_W, "import root.window.{ Agg, agg_new, agg_pct, agg_window_sum, agg_mean }"),
        (M, LAST_ELSE, "    } else if op == 5 {\n        agg_mean(a)\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="stream-u11", category="performance",
    title="bucket_of as a comparison chain instead of a loop",
    provenance="invented",
    derivation="Replaces the shift loop by a 14-way comparison chain with the same contract. Equal for every input "
               "(bit length capped at 14), but that takes a proof, not a diff.",
    behaviour="preserved",
    edits=[(H, ("func", "bucket_of"), bucket_chain())],
))

U.append(Update(
    id="stream-u12", category="feature",
    title="sum of the most recent k windows",
    provenance="invented",
    derivation="New agg_recent and request op 6.",
    behaviour="new-api",
    edits=[
        (W, "@noalloc\npublic func agg_window_sum", '''@noalloc
public func agg_recent(a: Agg, k: i64) -> i64 {
    var sum = 0
    for i in 0..8 {
        let id = a.win_id[i]
        if id <= a.newest && id > a.newest - k {
            sum += a.win_sum[i]
        }
    }
    sum
}

@noalloc
public func agg_window_sum'''),
        (M, "agg_window_sum, agg_mean }", "agg_window_sum, agg_mean, agg_recent }"),
        (M, LAST_ELSE, "    } else if op == 6 {\n        agg_recent(a, x)\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="stream-u13", category="contract-change",
    title="bucket_of now promises only result <= 15",
    provenance="invented",
    derivation="Weakening a published postcondition (14 to 15) in preparation for a wider histogram. The code is "
               "unchanged, so behaviour is identical; any caller that relied on <= 14 (the histogram index) is "
               "no longer covered by the contract.",
    behaviour="preserved",
    contract="weakens ensures of bucket_of (result <= 14 becomes result <= 15); code unchanged",
    edits=[(H, "    ensures result >= 0 && result <= 14\n{\n    if v < 1 {", "    ensures result >= 0 && result <= 15\n{\n    if v < 1 {")],
))

U.append(Update(
    id="stream-u14", category="bug-fix",
    title="window_sum returns -1 for a window that is not in the ring",
    provenance="invented",
    derivation="The accessor returned whatever window happened to share the slot.",
    behaviour="changed",
    edits=[(W, ("func", "agg_window_sum"), '''@noalloc
public func agg_window_sum(a: Agg, w: i64) -> i64 {
    let slot = w & 7
    if a.win_id[slot] != w { return -1 }
    a.win_sum[slot]
}''')],
))

U.append(Update(
    id="stream-u15", category="feature",
    title="minimum and maximum value",
    provenance="invented",
    derivation="Agg gains min_val and max_val; request op 8 reads them. max_val is right at zero; min_val must start "
               "high, so live aggregates stay stuck at a minimum of 0 unless migrated.",
    behaviour="new-api",
    migrate="needs-migrate: min_val = 1099511627776 for every live Agg",
    edits=[
        (W, "    public sum_all: i64,\n}", "    public sum_all: i64,\n    public min_val: i64,\n    public max_val: i64,\n}"),
        (W, "        sum_all: 0,\n    }", "        sum_all: 0,\n        min_val: 1099511627776,\n        max_val: 0,\n    }"),
        (W, "    a.sum_all += val\n", "    a.sum_all += val\n    if val < a.min_val {\n        a.min_val = val\n    }\n    if val > a.max_val {\n        a.max_val = val\n    }\n"),
        (W, "@noalloc\npublic func agg_window_sum", "@noalloc\npublic func agg_extreme(a: Agg, which: i64) -> i64 {\n    if which == 0 { a.min_val } else { a.max_val }\n}\n\n@noalloc\npublic func agg_window_sum"),
        (M, "agg_mean, agg_recent }", "agg_mean, agg_recent, agg_extreme }"),
        (M, LAST_ELSE, "    } else if op == 8 {\n        agg_extreme(a, x)\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="stream-u16", category="refactor",
    title="reorder Agg fields, for loop in agg_pct",
    provenance="invented",
    derivation="Field reorder (matched by name by hot reload) and a for loop with an early return; no behaviour change.",
    behaviour="preserved",
    edits=[
        (W, "    public hist: [i64],\n    public win_id: [i64],\n    public win_sum: [i64],\n    public win_cnt: [i64],\n    public total: i64,\n    public late: i64,\n    public newest: i64,\n    public sum_all: i64,\n    public min_val: i64,\n    public max_val: i64,\n}",
            "    public total: i64,\n    public late: i64,\n    public newest: i64,\n    public sum_all: i64,\n    public min_val: i64,\n    public max_val: i64,\n    public hist: [i64],\n    public win_id: [i64],\n    public win_sum: [i64],\n    public win_cnt: [i64],\n}"),
        # Struct literals list fields in declaration order (toolchain defect, see UEVAL_RESULTS.txt).
        (W, "        hist: [" + stream_v01.zeros(15) + "],\n        win_id: [" + stream_v01.minus7(8) + "],\n        win_sum: [" + stream_v01.zeros(8) + "],\n        win_cnt: [" + stream_v01.zeros(8) + "],\n        total: 0,\n        late: 0,\n        newest: -7,\n        sum_all: 0,\n        min_val: 1099511627776,\n        max_val: 0,\n",
            "        total: 0,\n        late: 0,\n        newest: -7,\n        sum_all: 0,\n        min_val: 1099511627776,\n        max_val: 0,\n        hist: [" + stream_v01.zeros(15) + "],\n        win_id: [" + stream_v01.minus7(8) + "],\n        win_sum: [" + stream_v01.zeros(8) + "],\n        win_cnt: [" + stream_v01.zeros(8) + "],\n"),
        (W, '''    var seen = 0
    var b = 0
    while b < 15 {
        seen += a.hist[b]
        if seen >= target {
            return 1 << b
        }
        b += 1
    }
    1 << 14
''', '''    var seen = 0
    for b in 0..15 {
        seen += a.hist[b]
        if seen >= target {
            return 1 << b
        }
    }
    1 << 14
'''),
    ],
))

U.append(Update(
    id="stream-u17", category="contract-change",
    title="percentile requires 1 <= p <= 100; the request path rejects other values with -3",
    provenance="invented",
    derivation="Strengthening a published precondition on agg_pct and guarding the request path. p = 0 used to answer "
               "with the smallest bucket; it now answers -3.",
    behaviour="changed",
    contract="strengthens requires of agg_pct (1 <= p <= 100)",
    edits=[
        (W, "public func agg_pct(a: Agg, p: i64) -> i64 {\n", "public func agg_pct(a: Agg, p: i64) -> i64\n    requires p >= 1 && p <= 100\n{\n"),
        (M, "    } else if op == 1 {\n        agg_pct(a, x)\n", "    } else if op == 1 {\n        if x < 1 || x > 100 { -3 } else { agg_pct(a, x) }\n"),
    ],
))

U.append(Update(
    id="stream-u18", category="feature",
    title="clear",
    provenance="invented",
    derivation="New agg_clear resets every window, bucket and counter; request op 7.",
    behaviour="new-api",
    edits=[
        (W, "@noalloc\npublic func agg_window_sum", '''@noalloc
public func agg_clear(a: Agg) {
    for i in 0..15 {
        a.hist[i] = 0
    }
    for i in 0..8 {
        a.win_id[i] = -7
        a.win_sum[i] = 0
        a.win_cnt[i] = 0
    }
    a.total = 0
    a.late = 0
    a.newest = -7
    a.sum_all = 0
    a.min_val = 1099511627776
    a.max_val = 0
}

@noalloc
public func agg_window_sum'''),
        (M, "agg_recent, agg_extreme }", "agg_recent, agg_extreme, agg_clear }"),
        (M, LAST_ELSE, "    } else if op == 7 {\n        agg_clear(a)\n        0\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="stream-u19", category="effect-change",
    title="bucket_of logs unusually large values (no longer real-time, now prints)",
    provenance="invented",
    derivation="A debug print is added to the worker's classification step and @realtime comes off bucket_of for the "
               "package to build. The function was allocation-free and bounded; now a print can allocate and block. "
               "Results are unchanged.",
    behaviour="preserved",
    effect="bucket_of: @realtime removed; println on values above 60000 (allocates, blocks on stdout)",
    edits=[(H, ("func", "bucket_of"), bucket_chain(realtime=False).replace(
        "{\n    if v < 1 {", "{\n    if v > 60000 {\n        println(\"large value \", v)\n    }\n    if v < 1 {"))],
))

HANDLE_ENUM = '''enum Op {
    Batch,
    Pct,
    WindowSum,
    Total,
    Late,
    Mean,
    Recent,
    Clear,
    Extreme,
    Unknown,
}

func decode(code: i64) -> Op {
    match code {
        0 -> Op.Batch,
        1 -> Op.Pct,
        2 -> Op.WindowSum,
        3 -> Op.Total,
        4 -> Op.Late,
        5 -> Op.Mean,
        6 -> Op.Recent,
        7 -> Op.Clear,
        8 -> Op.Extreme,
        _ -> Op.Unknown,
    }
}

public func stream_handle(a: Agg, op: i64, x: i64, y: i64, z: i64) -> i64 {
    match decode(op) {
        Op.Batch -> if x < 0 || x > 60 { -1 } else { process_batch(a, x, y, z) },
        Op.Pct -> if x < 1 || x > 100 { -3 } else { agg_pct(a, x) },
        Op.WindowSum -> agg_window_sum(a, x),
        Op.Total -> a.total,
        Op.Late -> a.late,
        Op.Mean -> agg_mean(a),
        Op.Recent -> agg_recent(a, x),
        Op.Clear -> {
            agg_clear(a)
            0
        },
        Op.Extreme -> agg_extreme(a, x),
        Op.Unknown -> -2,
    }
}'''

U.append(Update(
    id="stream-u20", category="refactor",
    title="request dispatch through an enum and match",
    provenance="invented",
    derivation="The if-chain over op codes becomes an enum and an exhaustive match; same codes, same guards.",
    behaviour="preserved",
    edits=[
        (M, "import root.window.{ Agg, agg_new, agg_pct, agg_window_sum, agg_mean, agg_recent, agg_extreme, agg_clear }",
            "import root.window.{ Agg, agg_new, agg_pct, agg_window_sum, agg_mean, agg_recent, agg_extreme, agg_clear }"),
        (M, ("func", "stream_handle"), HANDLE_ENUM),
    ],
))

assert len(U) == 20, len(U)

if __name__ == "__main__":
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.join(here, "..", "workloads", "stream")
    write_history(
        os.path.normpath(root), "stream",
        "Stream aggregator: a bounded channel feeds worker tasks that classify values into latency buckets; a ring "
        "of windowed sums and a histogram give sums and percentiles. Authored for this evaluation; not a "
        "production system.",
        stream_v01.FILES, U,
        extra={"entry_functions": "stream_new,stream_handle"},
    )
    print("wrote", os.path.normpath(root))
