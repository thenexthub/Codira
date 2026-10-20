"""Ledger / rate-limiter workload, version 1: the files of the first version.

Accounts with balances, a token bucket per account, transfers between
accounts. The money supply (`total`) is an invariant that `audit` checks.
Imported by author_ledger.py.
"""


def fresh(n):
    return ", ".join(["fresh()"] * n)


ACCOUNTS = f'''import root.limiter.{{ refill, rate, burst }}

type Amount = i64 {{ a | a > 0 && a <= 1000000 }}

public struct Acct {{
    public bal: i64,
    public frozen: i64,
    public tokens: i64,
    public last: i64,
}}

public class Ledger {{
    public accts: [Acct],
    public n: i64,
    public total: i64,
    public ok: i64,
    public denied: i64,
}}

func fresh() -> Acct {{
    Acct {{ bal: 1000, frozen: 0, tokens: 10, last: 0 }}
}}

public func ledger_new() -> Ledger {{
    Ledger {{
        accts: [{fresh(32)}],
        n: 32,
        total: 32000,
        ok: 0,
        denied: 0,
    }}
}}

@realtime
func debit(bal: i64, amt: Amount) -> i64
    requires bal >= amt
    ensures result == bal - amt && result >= 0
{{
    bal - amt
}}

@realtime
func credit(bal: i64, amt: Amount) -> i64
    requires bal >= 0 && bal <= 4000000000000
    ensures result == bal + amt && result > 0
{{
    bal + amt
}}

@noalloc
public func deposit(l: Ledger, acct: i64, amt: i64) -> i64 {{
    if acct < 0 || acct >= l.n {{ return 4 }}
    let b = l.accts[acct].bal
    l.accts[acct].bal = b + amt
    l.total += amt
    0
}}

@noalloc
public func withdraw(l: Ledger, acct: i64, amt: i64) -> i64 {{
    if acct < 0 || acct >= l.n {{ return 4 }}
    if amt <= 0 || amt > 1000000 {{ return 5 }}
    if l.accts[acct].frozen == 1 {{ return 3 }}
    let b = l.accts[acct].bal
    if b < amt {{ return 1 }}
    l.accts[acct].bal = debit(b, amt)
    l.total -= amt
    0
}}

@noalloc
public func transfer(l: Ledger, from: i64, to: i64, amt: i64, now: i64) -> i64 {{
    if from < 0 || from >= l.n || to < 0 || to >= l.n {{ return 4 }}
    if amt <= 0 || amt > 1000000 {{ return 5 }}
    if l.accts[from].frozen == 1 {{ return 3 }}
    let tk = refill(l.accts[from].tokens, l.accts[from].last, now, rate(), burst())
    if tk < 1 {{
        l.denied += 1
        return 2
    }}
    l.accts[from].tokens = tk - 1
    l.accts[from].last = now
    let fb = l.accts[from].bal
    if fb <= amt {{
        l.denied += 1
        return 1
    }}
    let tb = l.accts[to].bal
    if tb < 0 || tb > 4000000000000 {{ return 7 }}
    l.accts[from].bal = debit(fb, amt)
    l.accts[to].bal = credit(tb, amt)
    l.ok += 1
    0
}}

@noalloc
public func set_frozen(l: Ledger, acct: i64, flag: i64) -> i64 {{
    if acct < 0 || acct >= l.n {{ return 4 }}
    l.accts[acct].frozen = if flag == 0 {{ 0 }} else {{ 1 }}
    0
}}

// Sum of all balances minus the money supply: zero while the books balance.
@noalloc
public func audit(l: Ledger) -> i64 {{
    var sum = 0
    var i = 0
    while i < l.n {{
        sum += l.accts[i].bal
        i += 1
    }}
    sum - l.total
}}
'''

LIMITER = '''// Token bucket arithmetic. Pure, so every function here can be proven in
// isolation and called from a real-time path.

@realtime
public func rate() -> i64
    ensures result == 1
{ 1 }

@realtime
public func burst() -> i64
    ensures result == 10
{ 10 }

@realtime
public func refill(tokens: i64, last: i64, now: i64, rate: i64, burst: i64) -> i64
    requires rate >= 0
{
    tokens + (now - last) * rate
}
'''

INSPECT = '''import root.accounts.{ Ledger }

// Read-only views the evaluation harness uses to compare logical state
// across versions. Never changed by any update in the history.
public func acct_bal(l: Ledger, i: i64) -> i64 {
    if i < 0 || i >= l.n { return 0 }
    l.accts[i].bal
}

public func acct_frozen(l: Ledger, i: i64) -> i64 {
    if i < 0 || i >= l.n { return 0 }
    l.accts[i].frozen
}

public func acct_tokens(l: Ledger, i: i64) -> i64 {
    if i < 0 || i >= l.n { return 0 }
    l.accts[i].tokens
}

public func acct_last(l: Ledger, i: i64) -> i64 {
    if i < 0 || i >= l.n { return 0 }
    l.accts[i].last
}

public func ledger_n(l: Ledger) -> i64 { l.n }
public func ledger_total(l: Ledger) -> i64 { l.total }
public func ledger_ok(l: Ledger) -> i64 { l.ok }
public func ledger_denied(l: Ledger) -> i64 { l.denied }
'''

MOD = '''import root.accounts.{ Ledger, ledger_new, deposit, withdraw, transfer, set_frozen, audit }

public func ledger_stat(l: Ledger, which: i64) -> i64 {
    if which == 0 {
        l.ok
    } else {
        l.denied
    }
}

// One entry point for the request stream:
// 0 transfer(a -> b, c), 1 deposit(a, b), 2 withdraw(a, b), 3 balance(a),
// 4 freeze(a, b), 5 audit, 6 stat(a).
public func ledger_handle(l: Ledger, op: i64, a: i64, b: i64, c: i64, now: i64) -> i64 {
    if op == 0 {
        transfer(l, a, b, c, now)
    } else if op == 1 {
        deposit(l, a, b)
    } else if op == 2 {
        withdraw(l, a, b)
    } else if op == 3 {
        if a < 0 || a >= l.n { -1 } else { l.accts[a].bal }
    } else if op == 4 {
        set_frozen(l, a, b)
    } else if op == 5 {
        audit(l)
    } else if op == 6 {
        ledger_stat(l, a)
    } else {
        -2
    }
}
'''

TOML = '''[package]
name="ledger"
authors=[]
version="0.1.0"
'''

FILES = {
    "codira.toml": TOML,
    "src/mod.code": MOD,
    "src/accounts.code": ACCOUNTS,
    "src/limiter.code": LIMITER,
    "src/inspect.code": INSPECT,
}
