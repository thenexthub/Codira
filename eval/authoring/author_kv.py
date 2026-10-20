"""Authors the key-value store history (v01..v21) and its manifest.

Every update here is invented for this evaluation. Where a change follows a
well-known class of change (for example the tombstone bug in open addressing),
`provenance` says "pattern" and `derivation` names the class; none of them is
a reproduction of a specific upstream commit, and the paper must not say so.

Run:  python eval/authoring/author_kv.py   (writes eval/workloads/kv)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from authorlib import Update, write_history  # noqa: E402
import kv_v01  # noqa: E402

T = "src/table.code"
W = "src/wal.code"
M = "src/mod.code"

U = []

U.append(Update(
    id="kv-u01", category="bug-fix",
    title="deleting a key leaves a tombstone instead of an empty slot",
    provenance="pattern",
    derivation="Class: open-addressing deletion that clears the slot and so cuts the probe chain of "
               "colliding keys. Invented instance; the fix marks the slot as a tombstone.",
    behaviour="changed",
    migrate="none",
    notes="State built by v01 may already have lost keys behind cleared slots; the fix cannot resurrect them.",
    edits=[(T, "    t.slots[at].state = 1\n    true\n", "    t.slots[at].state = 3\n    t.tomb += 1\n    true\n")],
))

U.append(Update(
    id="kv-u02", category="refactor",
    title="extract next_slot from the two probe loops",
    provenance="invented",
    derivation="Extract-function refactor of `(i + 1) & mask`, with a contract so the callers keep their proofs.",
    behaviour="preserved",
    edits=[
        (T, "@noalloc\nfunc find_slot", '''@realtime
func next_slot(i: i64, mask: i64) -> i64
    requires mask >= 0
    ensures result >= 0 && result <= mask
{
    (i + 1) & mask
}

@noalloc
func find_slot'''),
        (T, "        i = (i + 1) & mask\n        n += 1\n", "        i = next_slot(i, mask)\n        n += 1\n"),
        (T, "    while t.slots[i].state == 2 {\n        i = (i + 1) & mask\n    }\n",
            "    while t.slots[i].state == 2 {\n        i = next_slot(i, mask)\n    }\n"),
    ],
))

U.append(Update(
    id="kv-u03", category="bug-fix",
    title="scan sums only live entries",
    provenance="invented",
    derivation="Scan treated every non-empty slot as live, so after u01 tombstones' stale values were summed.",
    behaviour="changed",
    edits=[(T, "        if t.slots[i].state != 1 {\n", "        if t.slots[i].state == 2 {\n")],
))

U.append(Update(
    id="kv-u04", category="bug-fix",
    title="delete decrements the live-entry count",
    provenance="invented",
    derivation="`used` was never decremented, so a table with churn reported full early (put returns 0).",
    behaviour="changed",
    edits=[(T, "    t.tomb += 1\n    true\n", "    t.tomb += 1\n    t.used -= 1\n    true\n")],
))

U.append(Update(
    id="kv-u05", category="performance",
    title="cache the probe mask in the table",
    provenance="pattern",
    derivation="Class: precomputing a derived field on a hot path. The new field `mask` is derived from `cap`, "
               "so objects that exist at update time need `mask = cap - 1`; a zero default makes every key hash "
               "to slot 0 and the insert probe loop can spin forever.",
    behaviour="preserved",
    migrate="needs-migrate: mask = cap - 1 for every live Table",
    notes="The dangerous case for unverified hot reload: admitted, and state-dependent hangs follow.",
    edits=[
        (T, "    public cap: i64,\n", "    public cap: i64,\n    public mask: i64,\n"),
        (T, "        cap: 128,\n", "        cap: 128,\n        mask: 127,\n"),
        (T, "func find_slot(t: Table, key: i64) -> i64 {\n    let mask = t.cap - 1\n",
            "func find_slot(t: Table, key: i64) -> i64 {\n    let mask = t.mask\n"),
        (T, "    if t.used + t.tomb + 1 > t.cap { return false }\n    let mask = t.cap - 1\n",
            "    if t.used + t.tomb + 1 > t.cap { return false }\n    let mask = t.mask\n"),
    ],
))

U.append(Update(
    id="kv-u06", category="feature",
    title="operation counters in the store",
    provenance="invented",
    derivation="Adds gets/hits/puts/dels counters to Store and a read accessor. New fields start at zero, "
               "which is the right initial value, so no migration is needed.",
    behaviour="new-api",
    migrate="zero-default-ok",
    edits=[
        (M, "    table: Table,\n    log: Log,\n}", "    table: Table,\n    log: Log,\n    public gets: i64,\n    public hits: i64,\n    public puts: i64,\n    public dels: i64,\n}"),
        (M, "    Store { table: table_new(), log: log_new() }\n",
            "    Store { table: table_new(), log: log_new(), gets: 0, hits: 0, puts: 0, dels: 0 }\n"),
        (M, "public func kv_get(s: Store, key: i64) -> i64 {\n    match table_get(s.table, key) {\n        nil -> -1,\n        v -> v,\n    }\n}",
            "public func kv_get(s: Store, key: i64) -> i64 {\n    s.gets += 1\n    match table_get(s.table, key) {\n        nil -> -1,\n        v -> {\n            s.hits += 1\n            v\n        },\n    }\n}"),
        (M, "    if table_put(s.table, key, val) {\n        log_append(s.log, 1, key, val)\n        1\n",
            "    if table_put(s.table, key, val) {\n        s.puts += 1\n        log_append(s.log, 1, key, val)\n        1\n"),
        (M, "    if table_del(s.table, key) {\n        log_append(s.log, 2, key, 0)\n        1\n",
            "    if table_del(s.table, key) {\n        s.dels += 1\n        log_append(s.log, 2, key, 0)\n        1\n"),
        (M, "public func kv_count(s: Store) -> i64 {\n    table_count(s.table)\n}\n",
            "public func kv_count(s: Store) -> i64 {\n    table_count(s.table)\n}\n\npublic func kv_stat(s: Store, which: i64) -> i64 {\n    if which == 0 {\n        s.gets\n    } else if which == 1 {\n        s.hits\n    } else if which == 2 {\n        s.puts\n    } else {\n        s.dels\n    }\n}\n"),
    ],
))

U.append(Update(
    id="kv-u07", category="bug-fix",
    title="put rejects negative values",
    provenance="invented",
    derivation="Negative values collide with the -1 'missing' answer of get; the store now refuses them.",
    behaviour="changed",
    edits=[(M, "public func kv_put(s: Store, key: i64, val: i64) -> i64 {\n    if table_put",
                "public func kv_put(s: Store, key: i64, val: i64) -> i64 {\n    if val < 0 { return 0 }\n    if table_put")],
))

U.append(Update(
    id="kv-u08", category="regression",
    title="stop probing at the first non-live slot (looks like a shorter probe)",
    provenance="pattern",
    derivation="Class: 'optimisation' that stops a probe chain at a tombstone. Looks like a cleanup of the two "
               "conditions; lookups of keys behind a deleted neighbour now miss, and put then inserts duplicates.",
    behaviour="changed",
    notes="Seeded bug that looks plausible in review.",
    edits=[(T, "        if s == 1 { return -1 }\n", "        if s != 2 { return -1 }\n")],
))

U.append(Update(
    id="kv-u09", category="bug-fix",
    title="revert u08: keys behind a tombstone are found again",
    provenance="invented",
    derivation="Revert of the u08 regression after a user report. The state it damaged stays damaged.",
    behaviour="changed",
    edits=[(T, "        if s != 2 { return -1 }\n", "        if s == 1 { return -1 }\n")],
))

HANDLE_OLD = '''public func kv_handle(s: Store, op: i64, a: i64, b: i64) -> i64 {
    if op == 0 {
        kv_get(s, a)
    } else if op == 1 {
        kv_put(s, a, b)
    } else if op == 2 {
        kv_del(s, a)
    } else if op == 3 {
        kv_scan(s, a, b)
    } else if op == 4 {
        kv_count(s)
    } else {
        -2
    }
}
'''
HANDLE_ENUM = '''enum Op {
    Get,
    Put,
    Del,
    Scan,
    Count,
    Unknown,
}

func decode(code: i64) -> Op {
    match code {
        0 -> Op.Get,
        1 -> Op.Put,
        2 -> Op.Del,
        3 -> Op.Scan,
        4 -> Op.Count,
        _ -> Op.Unknown,
    }
}

public func kv_handle(s: Store, op: i64, a: i64, b: i64) -> i64 {
    match decode(op) {
        Op.Get -> kv_get(s, a),
        Op.Put -> kv_put(s, a, b),
        Op.Del -> kv_del(s, a),
        Op.Scan -> kv_scan(s, a, b),
        Op.Count -> kv_count(s),
        Op.Unknown -> -2,
    }
}
'''
U.append(Update(
    id="kv-u10", category="refactor",
    title="request dispatch through an enum and match",
    provenance="invented",
    derivation="Replaces the if-chain over integer op codes by an enum and an exhaustive match; same codes.",
    behaviour="preserved",
    edits=[(M, HANDLE_OLD, HANDLE_ENUM)],
))

U.append(Update(
    id="kv-u11", category="contract-change",
    title="scan drops its non-negative result guarantee and signals bad ranges with -1",
    provenance="invented",
    derivation="Weakening a published postcondition: `ensures result >= 0` is removed and an in-band error value "
               "appears. Compiles, because no caller inside the package relied on the clause.",
    behaviour="changed",
    contract="weakens ensures of kv_scan (result >= 0 removed; lo > hi now returns -1)",
    edits=[(M, '''public func kv_scan(s: Store, lo: i64, hi: i64) -> i64
    ensures result >= 0
{
    let r = table_scan(s.table, lo, hi)
    if r < 0 { 0 } else { r }
}
''', '''public func kv_scan(s: Store, lo: i64, hi: i64) -> i64 {
    if lo > hi { return -1 }
    table_scan(s.table, lo, hi)
}
''')],
))

U.append(Update(
    id="kv-u12", category="feature",
    title="increment operation",
    provenance="invented",
    derivation="New operation kv_incr and a new variant Op.Incr (op code 5). Existing codes keep their meaning.",
    behaviour="new-api",
    edits=[
        (M, "public func kv_count(s: Store) -> i64 {", '''public func kv_incr(s: Store, key: i64, delta: i64) -> i64 {
    let cur = match table_get(s.table, key) {
        nil -> 0,
        v -> v,
    }
    let next = cur + delta
    if next < 0 { return -1 }
    if table_put(s.table, key, next) {
        s.puts += 1
        log_append(s.log, 3, key, next)
        next
    } else {
        -1
    }
}

public func kv_count(s: Store) -> i64 {'''),
        (M, "    Count,\n    Unknown,\n}", "    Count,\n    Incr,\n    Unknown,\n}"),
        (M, "        4 -> Op.Count,\n", "        4 -> Op.Count,\n        5 -> Op.Incr,\n"),
        (M, "        Op.Count -> kv_count(s),\n", "        Op.Count -> kv_count(s),\n        Op.Incr -> kv_incr(s, a, b),\n"),
    ],
))

U.append(Update(
    id="kv-u13", category="performance",
    title="scan returns early on an empty table",
    provenance="invented",
    derivation="`used == 0` implies no live slot, so the loop is skipped. True only because of a data-structure "
               "invariant (used equals the number of live slots) that a function-local equivalence proof does not know.",
    behaviour="preserved",
    edits=[(T, "public func table_scan(t: Table, lo: i64, hi: i64) -> i64 {\n    var sum = 0\n",
                "public func table_scan(t: Table, lo: i64, hi: i64) -> i64 {\n    if t.used == 0 { return 0 }\n    var sum = 0\n")],
))

U.append(Update(
    id="kv-u14", category="feature",
    title="expiry times on entries",
    provenance="invented",
    derivation="Entry gains `exp` (0 = never) and Store gains a logical clock advanced once per handled request; "
               "kv_expire sets an expiry, get treats an expired entry as missing. Both new fields are right at zero.",
    behaviour="new-api",
    migrate="zero-default-ok",
    notes="The clock restarts at 0 for a hot-updated store; entries only expire relative to the new clock.",
    edits=[
        (T, "    public state: i64,\n}", "    public state: i64,\n    public exp: i64,\n}"),
        (T, "    Entry { key: 0, val: 0, state: 1 }\n", "    Entry { key: 0, val: 0, state: 1, exp: 0 }\n"),
        (T, "public func table_get(t: Table, key: i64) -> i64? {\n    let at = find_slot(t, key)\n    if at < 0 { return nil }\n    t.slots[at].val\n}\n",
            '''public func table_get(t: Table, key: i64, now: i64) -> i64? {
    let at = find_slot(t, key)
    if at < 0 { return nil }
    let e = t.slots[at].exp
    if e != 0 && e <= now { return nil }
    t.slots[at].val
}

@noalloc
public func table_expire(t: Table, key: i64, at_time: i64) -> bool {
    let at = find_slot(t, key)
    if at < 0 { return false }
    t.slots[at].exp = at_time
    true
}
'''),
        (M, "import root.table.{ Table, table_new, table_get, table_put, table_del, table_scan, table_count }",
            "import root.table.{ Table, table_new, table_get, table_put, table_del, table_scan, table_count, table_expire }"),
        (M, "    public dels: i64,\n}", "    public dels: i64,\n    public clock: i64,\n}"),
        (M, "puts: 0, dels: 0 }", "puts: 0, dels: 0, clock: 0 }"),
        (M, "    match table_get(s.table, key) {\n        nil -> -1,\n        v -> {\n            s.hits += 1\n",
            "    match table_get(s.table, key, s.clock) {\n        nil -> -1,\n        v -> {\n            s.hits += 1\n"),
        (M, "    let cur = match table_get(s.table, key) {", "    let cur = match table_get(s.table, key, s.clock) {"),
        (M, "public func kv_count(s: Store) -> i64 {\n    table_count(s.table)\n}\n",
            "public func kv_count(s: Store) -> i64 {\n    table_count(s.table)\n}\n\npublic func kv_expire(s: Store, key: i64, at_time: i64) -> i64 {\n    if table_expire(s.table, key, at_time) { 1 } else { 0 }\n}\n"),
        (M, "public func kv_handle(s: Store, op: i64, a: i64, b: i64) -> i64 {\n    match decode(op) {\n",
            "public func kv_handle(s: Store, op: i64, a: i64, b: i64) -> i64 {\n    s.clock += 1\n    match decode(op) {\n"),
        (M, "    Incr,\n    Unknown,\n}", "    Incr,\n    Expire,\n    Unknown,\n}"),
        (M, "        5 -> Op.Incr,\n", "        5 -> Op.Incr,\n        6 -> Op.Expire,\n"),
        (M, "        Op.Incr -> kv_incr(s, a, b),\n", "        Op.Incr -> kv_incr(s, a, b),\n        Op.Expire -> kv_expire(s, a, b),\n"),
    ],
))

U.append(Update(
    id="kv-u15", category="refactor",
    title="reorder Table fields, scan with a for loop",
    provenance="invented",
    derivation="Field reorder (hot reload matches fields by name) and `while` to `for` in scan. No behaviour change.",
    behaviour="preserved",
    notes="Layout change that the heap migration handles by name.",
    edits=[
        (T, "    public slots: [Entry],\n    public cap: i64,\n    public mask: i64,\n    public used: i64,\n    public tomb: i64,\n}",
            "    public cap: i64,\n    public mask: i64,\n    public tomb: i64,\n    public used: i64,\n    public slots: [Entry],\n}"),
        # A struct literal must list its fields in declaration order (a toolchain
        # defect, see UEVAL_RESULTS.txt), so the literal follows the new order.
        (T, "        slots: [" + kv_v01.blanks(128) + "],\n        cap: 128,\n        mask: 127,\n        used: 0,\n        tomb: 0,\n",
            "        cap: 128,\n        mask: 127,\n        tomb: 0,\n        used: 0,\n        slots: [" + kv_v01.blanks(128) + "],\n"),
        (T, '''    var sum = 0
    var i = 0
    while i < t.cap {
        if t.slots[i].state == 2 {
            let k = t.slots[i].key
            if k >= lo && k <= hi {
                sum += t.slots[i].val
            }
        }
        i += 1
    }
    sum
''', '''    var sum = 0
    for i in 0..t.cap {
        if t.slots[i].state == 2 {
            let k = t.slots[i].key
            if k >= lo && k <= hi {
                sum += t.slots[i].val
            }
        }
    }
    sum
'''),
    ],
))

U.append(Update(
    id="kv-u16", category="effect-change",
    title="log_append records an audit object (no longer allocation-free)",
    provenance="invented",
    derivation="Adds a per-append audit object and drops `@noalloc` from log_append so the file still builds. "
               "The function was allocation-free; now every write allocates. No caller is annotated, so the build "
               "does not object.",
    behaviour="preserved",
    effect="log_append: @noalloc removed, allocates one Audit object per call",
    edits=[
        (W, "@noalloc\npublic func log_append(l: Log, op: i64, key: i64, val: i64) {\n    let rec = op * 1000003 + val\n",
            "public class Audit {\n    op: i64,\n    key: i64,\n    val: i64,\n}\n\npublic func log_append(l: Log, op: i64, key: i64, val: i64) {\n    let a = Audit { op: op, key: key, val: val }\n    let rec = a.op * 1000003 + a.val\n"),
    ],
))

U.append(Update(
    id="kv-u17", category="contract-change",
    title="put requires a non-negative key; the request path rejects negative keys",
    provenance="invented",
    derivation="Strengthening a published precondition (`requires key >= 0` on kv_put). The request path is guarded "
               "so the package still verifies; negative keys that used to work now return -3.",
    behaviour="changed",
    contract="strengthens requires of kv_put (key >= 0)",
    edits=[
        (M, "public func kv_put(s: Store, key: i64, val: i64) -> i64 {\n", "public func kv_put(s: Store, key: i64, val: i64) -> i64\n    requires key >= 0\n{\n"),
        (M, "        Op.Put -> kv_put(s, a, b),\n", "        Op.Put -> if a < 0 { -3 } else { kv_put(s, a, b) },\n"),
    ],
))

U.append(Update(
    id="kv-u18", category="feature",
    title="range delete",
    provenance="invented",
    derivation="New table_del_range and kv_del_range (op code 7); tombstones every live key in [lo, hi].",
    behaviour="new-api",
    edits=[
        (T, "@noalloc\npublic func table_scan", '''@noalloc
public func table_del_range(t: Table, lo: i64, hi: i64) -> i64 {
    var removed = 0
    for i in 0..t.cap {
        if t.slots[i].state == 2 {
            let k = t.slots[i].key
            if k >= lo && k <= hi {
                t.slots[i].state = 3
                t.tomb += 1
                t.used -= 1
                removed += 1
            }
        }
    }
    removed
}

@noalloc
public func table_scan'''),
        (M, "table_scan, table_count, table_expire }", "table_scan, table_count, table_expire, table_del_range }"),
        (M, "public func kv_count(s: Store) -> i64 {", "public func kv_del_range(s: Store, lo: i64, hi: i64) -> i64 {\n    if lo > hi { return 0 }\n    let n = table_del_range(s.table, lo, hi)\n    if n > 0 {\n        s.dels += n\n        log_append(s.log, 4, lo, hi)\n    }\n    n\n}\n\npublic func kv_count(s: Store) -> i64 {"),
        (M, "    Expire,\n    Unknown,\n}", "    Expire,\n    DelRange,\n    Unknown,\n}"),
        (M, "        6 -> Op.Expire,\n", "        6 -> Op.Expire,\n        7 -> Op.DelRange,\n"),
        (M, "        Op.Expire -> kv_expire(s, a, b),\n", "        Op.Expire -> kv_expire(s, a, b),\n        Op.DelRange -> kv_del_range(s, a, b),\n"),
    ],
))

U.append(Update(
    id="kv-u19", category="performance",
    title="cheaper hash function",
    provenance="pattern",
    derivation="Class: swapping the hash function of a live table. Drops a multiply-shift for an xor-shift; "
               "slot placement changes, so entries stored by the old function are not found by the new one "
               "unless the table is rehashed on update.",
    behaviour="preserved",
    migrate="needs-migrate: rehash every live Table with the new mix",
    notes="Equivalent as a new program; not a safe transition for a table that already holds data.",
    edits=[(T, "    ((key * 2654435761) >> 7) & 2147483647\n", "    ((key * 40503) ^ (key >> 9)) & 2147483647\n")],
))

U.append(Update(
    id="kv-u20", category="bug-fix",
    title="the log record covers the key",
    provenance="invented",
    derivation="The record hashed op and value only, so two writes of the same value to different keys were "
               "indistinguishable in the log checksum.",
    behaviour="changed",
    notes="The checksum of a hot-updated store differs from a fresh start by construction: it folds the old records.",
    edits=[(W, "    let rec = a.op * 1000003 + a.val\n", "    let rec = (a.op * 1000003 + a.key) * 31 + a.val\n")],
))

assert len(U) == 20, len(U)

if __name__ == "__main__":
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.join(here, "..", "workloads", "kv")
    write_history(
        os.path.normpath(root), "kv",
        "Key-value store, integer keys, open-addressing hash table with tombstones, write-ahead style log ring. "
        "Authored for this evaluation; not a production system.",
        kv_v01.FILES, U,
        extra={"entry_functions": "kv_new,kv_handle"},
    )
    print("wrote", os.path.normpath(root))
