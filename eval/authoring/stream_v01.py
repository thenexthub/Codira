"""Stream aggregator workload, version 1: the files of the first version.

A batch of events goes through a bounded channel to three worker tasks that
classify each value into a latency bucket; the host thread folds the results
into a windowed sum and a histogram. Imported by author_stream.py.
"""


def zeros(n):
    return ", ".join(["0"] * n)


def minus7(n):
    # -7 is not a byte-splat; an all-zero or all-0xFF literal of 16 or more
    # elements makes LLVM emit a memset the library cannot link.
    return ", ".join(["-7"] * n)


HIST = '''// Latency buckets: bucket k holds 2^(k-1) <= v < 2^k, bucket 0 holds v < 1,
// and everything from 2^13 up lands in bucket 14. Pure and bounded, so it is
// safe on a real-time path and cheap to prove.

@realtime
public func bucket_of(v: i64) -> i64
    ensures result >= 0 && result <= 14
{
    var b = 0
    var x = v
    while x > 1 && b < 14
        invariant b >= 0 && b <= 14
        decreases 14 - b
    {
        x = x >> 1
        b += 1
    }
    b
}

// Upper bound of a bucket, used to report a percentile.
@realtime
public func bucket_bound(b: i64) -> i64
    requires b >= 0 && b <= 14
    ensures result >= 1
{
    1 << b
}
'''

WINDOW = f'''public class Agg {{
    public hist: [i64],
    public win_id: [i64],
    public win_sum: [i64],
    public win_cnt: [i64],
    public total: i64,
    public late: i64,
    public newest: i64,
}}

public func agg_new() -> Agg {{
    Agg {{
        hist: [{zeros(15)}],
        win_id: [{minus7(8)}],
        win_sum: [{zeros(8)}],
        win_cnt: [{zeros(8)}],
        total: 0,
        late: 0,
        newest: -7,
    }}
}}

// Events are folded into a ring of 8 windows of 64 time units each.
@noalloc
public func agg_add(a: Agg, ts: i64, bucket: i64, val: i64) {{
    let w = ts / 64
    let slot = w & 7
    if a.win_id[slot] != w {{
        a.win_id[slot] = w
        a.win_sum[slot] = 0
    }}
    a.win_sum[slot] += val
    a.win_cnt[slot] += 1
    a.hist[bucket] += 1
    a.total += 1
    if w > a.newest {{
        a.newest = w
    }}
}}

// The smallest bucket bound below which at least p percent of the events fall.
@noalloc
public func agg_pct(a: Agg, p: i64) -> i64 {{
    let target = (a.total * p) / 100
    var seen = 0
    var b = 0
    while b < 15 {{
        seen += a.hist[b]
        if seen >= target {{
            return 1 << b
        }}
        b += 1
    }}
    1 << 14
}}

@noalloc
public func agg_window_sum(a: Agg, w: i64) -> i64 {{
    a.win_sum[w & 7]
}}
'''

PIPELINE = '''import root.hist.{ bucket_of }
import root.window.{ Agg, agg_add }

func worker(jobs: Channel[i64], results: Channel[i64]) {
    var running = true
    while running {
        let x = <-jobs
        if x < 0 {
            running = false
        } else {
            let tsx = x >> 20
            let v = x & 1048575
            let b = bucket_of(v)
            results <- (tsx << 24) | (b << 20) | v
        }
    }
}

func event_value(seed: i64, i: i64) -> i64 {
    (((seed + i) * 2654435761) >> 7) & 65535
}

// Folds n events (n <= 60: the channels hold 64) through three workers.
public func process_batch(a: Agg, n: i64, seed: i64, ts0: i64) -> i64
    requires n >= 0 && n <= 60
{
    scope {
        let jobs = channel(64)
        let results = channel(64)
        var w = 0
        while w < 3 {
            spawn worker(jobs, results)
            w += 1
        }
        var i = 0
        while i < n {
            jobs <- ((ts0 + i * 3 + 1073741824) << 20) | event_value(seed, i)
            i += 1
        }
        w = 0
        while w < 3 {
            jobs <- -1
            w += 1
        }
        var got = 0
        while got < n {
            let r = <-results
            let ts = (r >> 24) - 1073741824
            let b = (r >> 20) & 15
            let v = r & 1048575
            agg_add(a, ts, b, v)
            got += 1
        }
        n
    }
}
'''

INSPECT = '''import root.window.{ Agg }

// Read-only views the evaluation harness uses to compare logical state
// across versions. Never changed by any update in the history.
public func hist_at(a: Agg, i: i64) -> i64 {
    if i < 0 || i >= 15 { return 0 }
    a.hist[i]
}

public func win_id_at(a: Agg, i: i64) -> i64 {
    if i < 0 || i >= 8 { return 0 }
    a.win_id[i]
}

public func win_sum_at(a: Agg, i: i64) -> i64 {
    if i < 0 || i >= 8 { return 0 }
    a.win_sum[i]
}

public func win_cnt_at(a: Agg, i: i64) -> i64 {
    if i < 0 || i >= 8 { return 0 }
    a.win_cnt[i]
}

public func agg_total(a: Agg) -> i64 { a.total }
public func agg_late(a: Agg) -> i64 { a.late }
public func agg_newest(a: Agg) -> i64 { a.newest }
'''

MOD = '''import root.window.{ Agg, agg_new, agg_pct, agg_window_sum }
import root.pipeline.{ process_batch }

public func stream_new() -> Agg { agg_new() }

// One entry point for the request stream:
// 0 batch(n=x, seed=y, ts0=z), 1 percentile(p=x), 2 window sum (window id x),
// 3 total events, 4 late events.
public func stream_handle(a: Agg, op: i64, x: i64, y: i64, z: i64) -> i64 {
    if op == 0 {
        if x < 0 || x > 60 { -1 } else { process_batch(a, x, y, z) }
    } else if op == 1 {
        agg_pct(a, x)
    } else if op == 2 {
        agg_window_sum(a, x)
    } else if op == 3 {
        a.total
    } else if op == 4 {
        a.late
    } else {
        -2
    }
}
'''

TOML = '''[package]
name="stream"
authors=[]
version="0.1.0"
'''

FILES = {
    "codira.toml": TOML,
    "src/mod.code": MOD,
    "src/hist.code": HIST,
    "src/window.code": WINDOW,
    "src/pipeline.code": PIPELINE,
    "src/inspect.code": INSPECT,
}
