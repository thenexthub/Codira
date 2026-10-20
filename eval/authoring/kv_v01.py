"""KV workload, version 1: the files of the first version, as text.

Imported by author_kv.py. Kept separate so that v01 is one readable block.
"""


def blanks(n):
    return ", ".join(["blank()"] * n)


def ring_init(n):
    # 7 is not a byte-splat: an all-zero or all-0xFF literal makes LLVM emit a
    # memset that the library cannot link (see UEVAL_RESULTS.txt, "toolchain").
    return ", ".join(["7"] * n)


TABLE = f'''public struct Entry {{
    public key: i64,
    public val: i64,
    public state: i64,
}}

// state: 1 = empty, 2 = full, 3 = tombstone
public class Table {{
    public slots: [Entry],
    public cap: i64,
    public used: i64,
    public tomb: i64,
}}

func blank() -> Entry {{
    Entry {{ key: 0, val: 0, state: 1 }}
}}

public func table_new() -> Table {{
    Table {{
        slots: [{blanks(128)}],
        cap: 128,
        used: 0,
        tomb: 0,
    }}
}}

@realtime
func mix(key: i64) -> i64
    ensures result >= 0
{{
    ((key * 2654435761) >> 7) & 2147483647
}}

@realtime
func slot_of(key: i64, mask: i64) -> i64
    requires mask >= 0
    ensures result >= 0 && result <= mask
{{
    mix(key) & mask
}}

@noalloc
func find_slot(t: Table, key: i64) -> i64 {{
    let mask = t.cap - 1
    if mask < 0 {{ return -1 }}
    var i = slot_of(key, mask)
    var n = 0
    while n < t.cap {{
        let s = t.slots[i].state
        if s == 1 {{ return -1 }}
        if s == 2 && t.slots[i].key == key {{ return i }}
        i = (i + 1) & mask
        n += 1
    }}
    -1
}}

public func table_get(t: Table, key: i64) -> i64? {{
    let at = find_slot(t, key)
    if at < 0 {{ return nil }}
    t.slots[at].val
}}

@noalloc
public func table_put(t: Table, key: i64, val: i64) -> bool {{
    let at = find_slot(t, key)
    if at >= 0 {{
        t.slots[at].val = val
        return true
    }}
    if t.used + t.tomb + 1 > t.cap {{ return false }}
    let mask = t.cap - 1
    if mask < 0 {{ return false }}
    var i = slot_of(key, mask)
    while t.slots[i].state == 2 {{
        i = (i + 1) & mask
    }}
    if t.slots[i].state == 3 {{ t.tomb -= 1 }}
    t.slots[i].key = key
    t.slots[i].val = val
    t.slots[i].state = 2
    t.used += 1
    true
}}

@noalloc
public func table_del(t: Table, key: i64) -> bool {{
    let at = find_slot(t, key)
    if at < 0 {{ return false }}
    t.slots[at].state = 1
    true
}}

@noalloc
public func table_scan(t: Table, lo: i64, hi: i64) -> i64 {{
    var sum = 0
    var i = 0
    while i < t.cap {{
        if t.slots[i].state != 1 {{
            let k = t.slots[i].key
            if k >= lo && k <= hi {{
                sum += t.slots[i].val
            }}
        }}
        i += 1
    }}
    sum
}}

@noalloc
public func table_count(t: Table) -> i64 {{
    t.used
}}
'''

WAL = f'''public class Log {{
    public ring: [i64],
    public seq: i64,
    public sum: i64,
}}

public func log_new() -> Log {{
    Log {{
        ring: [{ring_init(64)}],
        seq: 0,
        sum: 0,
    }}
}}

@realtime
func ring_pos(seq: i64, mask: i64) -> i64
    requires mask >= 0
    ensures result >= 0 && result <= mask
{{
    seq & mask
}}

@realtime
func fold(sum: i64, v: i64) -> i64
    ensures result >= 0
{{
    (sum * 31 + v) & 9223372036854775807
}}

@noalloc
public func log_append(l: Log, op: i64, key: i64, val: i64) {{
    let rec = op * 1000003 + val
    let pos = ring_pos(l.seq, 63)
    l.ring[pos] = rec
    l.sum = fold(l.sum, rec)
    l.seq += 1
}}
'''

INSPECT = '''import root.table.{ Table }
import root.wal.{ Log }

// Read-only views the evaluation harness uses to compare logical state
// across versions. Never changed by any update in the history.
public func slot_live(t: Table, i: i64) -> i64 {
    if i < 0 || i >= t.cap { return 0 }
    if t.slots[i].state == 2 { 1 } else { 0 }
}

public func slot_key(t: Table, i: i64) -> i64 {
    if i < 0 || i >= t.cap { return 0 }
    t.slots[i].key
}

public func slot_val(t: Table, i: i64) -> i64 {
    if i < 0 || i >= t.cap { return 0 }
    t.slots[i].val
}

public func table_capacity(t: Table) -> i64 { t.cap }
public func table_used(t: Table) -> i64 { t.used }
public func log_seq(l: Log) -> i64 { l.seq }
public func log_sum(l: Log) -> i64 { l.sum }
'''

MOD = '''import root.table.{ Table, table_new, table_get, table_put, table_del, table_scan, table_count }
import root.wal.{ Log, log_new, log_append }
import root.inspect.{ table_used }

public class Store {
    table: Table,
    log: Log,
}

public func kv_new() -> Store {
    Store { table: table_new(), log: log_new() }
}

public func kv_get(s: Store, key: i64) -> i64 {
    match table_get(s.table, key) {
        nil -> -1,
        v -> v,
    }
}

public func kv_put(s: Store, key: i64, val: i64) -> i64 {
    if table_put(s.table, key, val) {
        log_append(s.log, 1, key, val)
        1
    } else {
        0
    }
}

public func kv_del(s: Store, key: i64) -> i64 {
    if table_del(s.table, key) {
        log_append(s.log, 2, key, 0)
        1
    } else {
        0
    }
}

public func kv_scan(s: Store, lo: i64, hi: i64) -> i64
    ensures result >= 0
{
    let r = table_scan(s.table, lo, hi)
    if r < 0 { 0 } else { r }
}

// Keeps the inspection module loaded with the service (the evaluation harness reads state through it).
public func kv_used(s: Store) -> i64 {
    table_used(s.table)
}

public func kv_count(s: Store) -> i64 {
    table_count(s.table)
}

// One entry point for the request stream: 0 get, 1 put, 2 del, 3 scan, 4 count.
public func kv_handle(s: Store, op: i64, a: i64, b: i64) -> i64 {
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

TOML = '''[package]
name="kv"
authors=[]
version="0.1.0"
'''

FILES = {
    "codira.toml": TOML,
    "src/mod.code": MOD,
    "src/table.code": TABLE,
    "src/wal.code": WAL,
    "src/inspect.code": INSPECT,
}
