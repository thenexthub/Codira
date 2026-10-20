"""Authors the ledger / rate-limiter history (v01..v21) and its manifest.

Every update here is invented for this evaluation. Where a change follows a
well-known class of change, `provenance` says "pattern" and `derivation`
names the class; none is a reproduction of a specific upstream commit.

Run:  python eval/authoring/author_ledger.py   (writes eval/workloads/ledger)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from authorlib import Update, write_history  # noqa: E402
import ledger_v01  # noqa: E402

A = "src/accounts.code"
L = "src/limiter.code"
M = "src/mod.code"


def transfer(exact=False, take=True, same=False, dest_frozen=False, fee=False, enum=False,
             txs=False, strict=False, helper=False):
    """The text of `transfer` at a given stage of the history."""
    def r(name, code):
        return f"code(Status.{name})" if enum else str(code)

    lines = ["@noalloc"]
    if strict:
        lines.append("public func transfer(l: Ledger, from: i64, to: i64, amt: i64, now: i64) -> i64")
        lines.append("    requires amt > 0 && amt <= 1000000")
        lines.append("{")
    else:
        lines.append("public func transfer(l: Ledger, from: i64, to: i64, amt: i64, now: i64) -> i64 {")
    lines.append(f"    if from < 0 || from >= l.n || to < 0 || to >= l.n {{ return {r('BadAccount', 4)} }}")
    if not strict:
        if helper:
            lines.append(f"    if bad_amount(amt) {{ return {r('BadAmount', 5)} }}")
        else:
            lines.append(f"    if amt <= 0 || amt > 1000000 {{ return {r('BadAmount', 5)} }}")
    if same:
        lines.append(f"    if from == to {{ return {r('SameAccount', 6)} }}")
    if dest_frozen:
        lines.append(f"    if l.accts[from].frozen == 1 || l.accts[to].frozen == 1 {{ return {r('Frozen', 3)} }}")
    else:
        lines.append(f"    if l.accts[from].frozen == 1 {{ return {r('Frozen', 3)} }}")
    assert take
    lines.append("    if !take_token(l, from, now) {")
    lines.append("        l.denied += 1")
    lines.append(f"        return {r('RateLimited', 2)}")
    lines.append("    }")
    if fee:
        lines.append("    let fee = fee_of(amt, l.fee_shift)")
    lines.append("    let fb = l.accts[from].bal")
    need = "amt + fee" if fee else "amt"
    cmp_ = "<" if exact else "<="
    lines.append(f"    if fb {cmp_} {need} {{")
    lines.append("        l.denied += 1")
    lines.append(f"        return {r('Insufficient', 1)}")
    lines.append("    }")
    lines.append("    let tb = l.accts[to].bal")
    lines.append(f"    if tb < 0 || tb > 4000000000000 {{ return {r('Overflow', 7)} }}")
    if fee:
        lines.append("    let rest = debit(fb, amt)")
        lines.append("    l.accts[from].bal = if fee > 0 { debit(rest, fee) } else { rest }")
        lines.append("    l.total -= fee")
    else:
        lines.append("    l.accts[from].bal = debit(fb, amt)")
    lines.append("    l.accts[to].bal = credit(tb, amt)")
    if txs:
        lines.append("    l.accts[from].txs += 1")
        lines.append("    l.accts[to].txs += 1")
    lines.append("    l.ok += 1")
    lines.append(f"    {r('Ok', 0)}")
    lines.append("}")
    return "\n".join(lines) + "\n"


def take_token(lt0=False, rate_field=False, burst_field=False, metrics=False, noalloc=True):
    out = []
    if noalloc:
        out.append("@noalloc")
    out.append("func take_token(l: Ledger, i: i64, now: i64) -> bool {")
    r, b = "rate()", "burst()"
    if rate_field:
        out.append("    let r = l.accts[i].rate")
        out.append("    if r < 0 { return false }")
        r = "r"
    if burst_field:
        out.append("    let b = l.accts[i].burst")
        out.append("    if b < 0 { return false }")
        b = "b"
    out.append(f"    let tk = refill(l.accts[i].tokens, l.accts[i].last, now, {r}, {b})")
    out.append(f"    if tk {'< 0' if lt0 else '< 1'} {{ return false }}")
    if metrics:
        out.append("    publish_metric(tk)")
    out.append("    l.accts[i].tokens = tk - 1")
    out.append("    l.accts[i].last = now")
    out.append("    true")
    out.append("}")
    return "\n".join(out) + "\n"


U = []

U.append(Update(
    id="ledger-u01", category="bug-fix",
    title="a transfer of the exact balance is allowed",
    provenance="pattern",
    derivation="Class: off-by-one in a balance check (`<=` where `<` is meant). Invented instance.",
    behaviour="changed",
    edits=[(A, "    if fb <= amt {\n", "    if fb < amt {\n")],
))

U.append(Update(
    id="ledger-u02", category="refactor",
    title="extract take_token from transfer",
    provenance="invented",
    derivation="Extract-function refactor of the rate-limit step; token and timestamp are updated exactly as before.",
    behaviour="preserved",
    edits=[
        (A, '''    let tk = refill(l.accts[from].tokens, l.accts[from].last, now, rate(), burst())
    if tk < 1 {
        l.denied += 1
        return 2
    }
    l.accts[from].tokens = tk - 1
    l.accts[from].last = now
''', '''    if !take_token(l, from, now) {
        l.denied += 1
        return 2
    }
'''),
        (A, "@noalloc\npublic func transfer", take_token() + "\n@noalloc\npublic func transfer"),
    ],
))

U.append(Update(
    id="ledger-u03", category="bug-fix",
    title="a transfer to the same account is refused",
    provenance="pattern",
    derivation="Class: read-modify-write on two accounts that may alias. Both balances were read first, so a self "
               "transfer wrote back a stale credit and created money; `audit` stops returning 0.",
    behaviour="changed",
    notes="State damaged by v01..v02 (audit != 0) stays damaged.",
    edits=[(A, "    if l.accts[from].frozen == 1 { return 3 }\n", "    if from == to { return 6 }\n    if l.accts[from].frozen == 1 { return 3 }\n")],
))

U.append(Update(
    id="ledger-u04", category="bug-fix",
    title="a frozen destination account refuses incoming transfers",
    provenance="invented",
    derivation="Only the sender was checked for the frozen flag.",
    behaviour="changed",
    edits=[(A, "    if l.accts[from].frozen == 1 { return 3 }\n", "    if l.accts[from].frozen == 1 || l.accts[to].frozen == 1 { return 3 }\n")],
))

U.append(Update(
    id="ledger-u05", category="bug-fix",
    title="refill is capped at the burst size and ignores a clock that went backwards",
    provenance="pattern",
    derivation="Class: token bucket without an upper bound and with unsigned-style elapsed time. Two defects fixed "
               "together; the contract gains `ensures result >= 0 && result <= burst` and `requires burst >= 0` "
               "(every caller passes the constant burst()).",
    behaviour="changed",
    contract="strengthens ensures of refill (0 <= result <= burst); requires gains burst >= 0",
    edits=[(L, ("func", "refill"), '''@realtime
public func refill(tokens: i64, last: i64, now: i64, rate: i64, burst: i64) -> i64
    requires rate >= 0 && burst >= 0
    ensures result >= 0 && result <= burst
{
    let dt = if now > last { now - last } else { 0 }
    let t = tokens + dt * rate
    if t < 0 { 0 } else if t > burst { burst } else { t }
}''')],
))

U.append(Update(
    id="ledger-u06", category="performance",
    title="audit hoists the account count out of the loop",
    provenance="invented",
    derivation="Loop-invariant hoisting by hand: `l.n` read once.",
    behaviour="preserved",
    edits=[(A, ("func", "audit"), '''// Sum of all balances minus the money supply: zero while the books balance.
@noalloc
public func audit(l: Ledger) -> i64 {
    var sum = 0
    var i = 0
    let n = l.n
    while i < n {
        sum += l.accts[i].bal
        i += 1
    }
    sum - l.total
}''')],
))

U.append(Update(
    id="ledger-u07", category="bug-fix",
    title="deposit validates the amount and uses the proven credit",
    provenance="pattern",
    derivation="Class: a missing sign check lets a 'deposit' of a negative amount withdraw without any balance check.",
    behaviour="changed",
    edits=[(A, ("func", "deposit"), '''@noalloc
public func deposit(l: Ledger, acct: i64, amt: i64) -> i64 {
    if acct < 0 || acct >= l.n { return 4 }
    if amt <= 0 || amt > 1000000 { return 5 }
    let b = l.accts[acct].bal
    if b < 0 || b > 4000000000000 { return 7 }
    l.accts[acct].bal = credit(b, amt)
    l.total += amt
    0
}''')],
))

FEE = '''@realtime
public func fee_of(amt: i64, shift: i64) -> i64
    requires amt >= 0 && amt <= 1000000
    ensures result >= 0 && result <= amt
{
    if shift <= 0 || shift > 40 { 0 } else { amt >> shift }
}

'''

U.append(Update(
    id="ledger-u08", category="feature",
    title="transfer fees as amt / 2^shift, burned",
    provenance="invented",
    derivation="Ledger gains fee_shift (zero = no fee, the old behaviour) and a proven fee_of; transfer debits the fee "
               "from the sender and removes it from the money supply so audit still balances.",
    behaviour="new-api",
    migrate="zero-default-ok",
    edits=[
        (A, "    public denied: i64,\n}", "    public denied: i64,\n    public fee_shift: i64,\n}"),
        (A, "        denied: 0,\n    }", "        denied: 0,\n        fee_shift: 0,\n    }"),
        (A, "@realtime\nfunc debit", FEE + "@realtime\nfunc debit"),
        (A, ("func", "transfer"), transfer(exact=True, same=True, dest_frozen=True, fee=True)),
        (M, "import root.accounts.{ Ledger, ledger_new, deposit, withdraw, transfer, set_frozen, audit }",
            "import root.accounts.{ Ledger, ledger_new, deposit, withdraw, transfer, set_frozen, audit }"),
        (M, "    } else {\n        -2\n    }\n}", "    } else if op == 10 {\n        l.fee_shift = if a < 0 { 0 } else { a }\n        0\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="ledger-u09", category="regression",
    title="rate limiter admits a request when the bucket is empty (looks like a cleanup of '< 1')",
    provenance="pattern",
    derivation="Class: off-by-one in a threshold that is easy to read as equivalent (`tk < 0` for 'none left'). "
               "A transfer at zero tokens is admitted and leaves -1 behind; the next refill hides it.",
    behaviour="changed",
    notes="Seeded bug that looks plausible in review.",
    edits=[(A, ("func", "take_token"), take_token(lt0=True))],
))

U.append(Update(
    id="ledger-u10", category="bug-fix",
    title="revert u09: an empty bucket refuses the transfer again",
    provenance="invented",
    derivation="Revert after the rate-limit test failed in staging.",
    behaviour="changed",
    edits=[(A, ("func", "take_token"), take_token(lt0=False))],
))

STATUS = '''enum Status {
    Ok,
    Insufficient,
    RateLimited,
    Frozen,
    BadAccount,
    BadAmount,
    SameAccount,
    Overflow,
}

func code(s: Status) -> i64 {
    match s {
        Status.Ok -> 0,
        Status.Insufficient -> 1,
        Status.RateLimited -> 2,
        Status.Frozen -> 3,
        Status.BadAccount -> 4,
        Status.BadAmount -> 5,
        Status.SameAccount -> 6,
        Status.Overflow -> 7,
    }
}

'''

U.append(Update(
    id="ledger-u11", category="refactor",
    title="status codes through an enum",
    provenance="invented",
    derivation="transfer returns code(Status.X) instead of bare integers; the integer values are unchanged.",
    behaviour="preserved",
    edits=[
        (A, "@noalloc\nfunc take_token", STATUS + "@noalloc\nfunc take_token"),
        (A, ("func", "transfer"), transfer(exact=True, same=True, dest_frozen=True, fee=True, enum=True)),
    ],
))

U.append(Update(
    id="ledger-u12", category="feature",
    title="per-account transaction counter",
    provenance="invented",
    derivation="Acct gains txs; transfer counts both sides; a read accessor and request op 7. New field is right at zero.",
    behaviour="new-api",
    migrate="zero-default-ok",
    edits=[
        (A, "    public last: i64,\n}", "    public last: i64,\n    public txs: i64,\n}"),
        (A, "Acct { bal: 1000, frozen: 0, tokens: 10, last: 0 }", "Acct { bal: 1000, frozen: 0, tokens: 10, last: 0, txs: 0 }"),
        (A, ("func", "transfer"), transfer(exact=True, same=True, dest_frozen=True, fee=True, enum=True, txs=True)),
        (A, "@noalloc\npublic func set_frozen", "public func acct_txs(l: Ledger, i: i64) -> i64 {\n    if i < 0 || i >= l.n { return -1 }\n    l.accts[i].txs\n}\n\n@noalloc\npublic func set_frozen"),
        (M, "import root.accounts.{ Ledger, ledger_new, deposit, withdraw, transfer, set_frozen, audit }",
            "import root.accounts.{ Ledger, ledger_new, deposit, withdraw, transfer, set_frozen, audit, acct_txs }"),
        (M, "    } else {\n        -2\n    }\n}", "    } else if op == 7 {\n        acct_txs(l, a)\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="ledger-u13", category="contract-change",
    title="burst() is loosened to 'between 10 and 20' and set to 12",
    provenance="invented",
    derivation="Tuning a constant while loosening its published postcondition from an equality to a range. "
               "Every caller only needs burst >= 0, so the package still verifies; behaviour changes.",
    behaviour="changed",
    contract="weakens ensures of burst (result == 10 becomes 10 <= result <= 20); value 12",
    edits=[(L, ("func", "burst"), '''@realtime
public func burst() -> i64
    ensures result >= 10 && result <= 20
{ 12 }''')],
))

U.append(Update(
    id="ledger-u14", category="performance",
    title="refill rate stored per account instead of a constant",
    provenance="pattern",
    derivation="Class: turning a constant into a per-object field so it can be tuned. Acct gains `rate`; fresh accounts "
               "get 1. Accounts that exist at update time have rate 0 unless migrated, so they never refill.",
    behaviour="preserved",
    migrate="needs-migrate: rate = 1 for every live Acct",
    notes="Admitted and then every transfer is rate limited once the initial tokens are used.",
    edits=[
        (A, "    public txs: i64,\n}", "    public txs: i64,\n    public rate: i64,\n}"),
        (A, "last: 0, txs: 0 }", "last: 0, txs: 0, rate: 1 }"),
        (A, ("func", "take_token"), take_token(rate_field=True)),
    ],
))

U.append(Update(
    id="ledger-u15", category="refactor",
    title="reorder Ledger fields, audit with a for loop",
    provenance="invented",
    derivation="Field reorder (matched by name by hot reload) and a for loop; no behaviour change.",
    behaviour="preserved",
    edits=[
        (A, "    public accts: [Acct],\n    public n: i64,\n    public total: i64,\n    public ok: i64,\n    public denied: i64,\n    public fee_shift: i64,\n}",
            "    public n: i64,\n    public total: i64,\n    public fee_shift: i64,\n    public ok: i64,\n    public denied: i64,\n    public accts: [Acct],\n}"),
        # Struct literals list fields in declaration order (toolchain defect, see UEVAL_RESULTS.txt).
        (A, "        accts: [" + ledger_v01.fresh(32) + "],\n        n: 32,\n        total: 32000,\n        ok: 0,\n        denied: 0,\n        fee_shift: 0,\n",
            "        n: 32,\n        total: 32000,\n        fee_shift: 0,\n        ok: 0,\n        denied: 0,\n        accts: [" + ledger_v01.fresh(32) + "],\n"),
        (A, ("func", "audit"), '''// Sum of all balances minus the money supply: zero while the books balance.
@noalloc
public func audit(l: Ledger) -> i64 {
    var sum = 0
    for i in 0..l.n {
        sum += l.accts[i].bal
    }
    sum - l.total
}'''),
    ],
))

U.append(Update(
    id="ledger-u16", category="feature",
    title="per-account burst size",
    provenance="invented",
    derivation="Acct gains `burst` (fresh accounts 10) with a setter (request op 8). Live accounts have burst 0 unless "
               "migrated, which refuses every transfer.",
    behaviour="new-api",
    migrate="needs-migrate: burst = 10 for every live Acct",
    edits=[
        (A, "    public rate: i64,\n}", "    public rate: i64,\n    public burst: i64,\n}"),
        (A, "txs: 0, rate: 1 }", "txs: 0, rate: 1, burst: 10 }"),
        (A, ("func", "take_token"), take_token(rate_field=True, burst_field=True)),
        (A, "@noalloc\npublic func set_frozen", "public func set_burst(l: Ledger, i: i64, b: i64) -> i64 {\n    if i < 0 || i >= l.n { return 4 }\n    if b < 0 || b > 1000 { return 5 }\n    l.accts[i].burst = b\n    0\n}\n\n@noalloc\npublic func set_frozen"),
        (M, "audit, acct_txs }", "audit, acct_txs, set_burst }"),
        (M, "    } else {\n        -2\n    }\n}", "    } else if op == 8 {\n        set_burst(l, a, b)\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="ledger-u17", category="contract-change",
    title="transfer requires a valid amount; the request path checks it",
    provenance="invented",
    derivation="Strengthening a published precondition (`requires amt > 0 && amt <= 1000000`), moving the check to "
               "the request path. Through ledger_handle the observable result is unchanged; a direct caller that "
               "passed an invalid amount and got status 5 now violates the contract.",
    behaviour="preserved",
    contract="strengthens requires of transfer (amt in 1..=1000000)",
    edits=[
        (A, ("func", "transfer"), transfer(exact=True, same=True, dest_frozen=True, fee=True, enum=True, txs=True, strict=True)),
        (M, "    if op == 0 {\n        transfer(l, a, b, c, now)\n", "    if op == 0 {\n        if c <= 0 || c > 1000000 { 5 } else { transfer(l, a, b, c, now) }\n"),
    ],
))

BATCH = '''@noalloc
public func batch_transfer(l: Ledger, from: i64, first: i64, count: i64, amt: i64, now: i64) -> i64 {
    if amt <= 0 || amt > 1000000 { return -5 }
    var paid = 0
    var k = 0
    while k < count && k < 8 {
        let st = transfer(l, from, first + k, amt, now)
        if st != 0 { return 0 - st }
        paid += 1
        k += 1
    }
    paid
}

'''

U.append(Update(
    id="ledger-u18", category="feature",
    title="batch transfer to consecutive accounts",
    provenance="invented",
    derivation="New batch_transfer (at most 8 payees, stops at the first refusal and returns its status negated); request op 9.",
    behaviour="new-api",
    edits=[
        (A, "@noalloc\npublic func set_frozen", BATCH + "@noalloc\npublic func set_frozen"),
        (M, "audit, acct_txs, set_burst }", "audit, acct_txs, set_burst, batch_transfer }"),
        (M, "    } else {\n        -2\n    }\n}", "    } else if op == 9 {\n        batch_transfer(l, a, b, 3, c, now)\n    } else {\n        -2\n    }\n}"),
    ],
))

U.append(Update(
    id="ledger-u19", category="effect-change",
    title="every admitted transfer publishes a metric through a channel (the real-time path now blocks)",
    provenance="invented",
    derivation="A metrics hook is added to take_token. Sending on a channel blocks and allocates, so @noalloc has to "
               "come off take_token and transfer for the package to build; batch_transfer loses it too. Nothing "
               "else in the diff looks like a behaviour change.",
    behaviour="preserved",
    effect="take_token, transfer, batch_transfer: @noalloc removed; blocking channel use added on the transfer path",
    edits=[
        (A, ("func", "take_token"), take_token(rate_field=True, burst_field=True, metrics=True, noalloc=False)),
        (A, "func take_token", '''func put_metric(c: Channel[i64], v: i64) {
    c <- v
}

func publish_metric(v: i64) {
    scope {
        let m = channel(1)
        put_metric(m, v)
        let echo = <-m
    }
}

func take_token'''),
        (A, "@noalloc\npublic func transfer", "public func transfer"),
        (A, "@noalloc\npublic func batch_transfer", "public func batch_transfer"),
    ],
))

U.append(Update(
    id="ledger-u20", category="refactor",
    title="extract the amount check",
    provenance="invented",
    derivation="`bad_amount` replaces the inline range test in deposit and withdraw; same range, same status.",
    behaviour="preserved",
    edits=[
        (A, "    if amt <= 0 || amt > 1000000 { return 5 }\n    if l.accts[acct].frozen == 1 { return 3 }\n", "    if bad_amount(amt) { return 5 }\n    if l.accts[acct].frozen == 1 { return 3 }\n"),
        (A, "    if amt <= 0 || amt > 1000000 { return 5 }\n    let b = l.accts[acct].bal\n    if b < 0", "    if bad_amount(amt) { return 5 }\n    let b = l.accts[acct].bal\n    if b < 0"),
        (A, "@realtime\nfunc debit", '''@realtime
func bad_amount(amt: i64) -> bool
    ensures result == (amt <= 0 || amt > 1000000)
{
    amt <= 0 || amt > 1000000
}

@realtime
func debit'''),
    ],
))

assert len(U) == 20, len(U)

if __name__ == "__main__":
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.join(here, "..", "workloads", "ledger")
    write_history(
        os.path.normpath(root), "ledger",
        "Ledger and rate limiter: accounts with balances, a token bucket per account, transfers, fees, an "
        "invariant (money supply) audited by the service. Authored for this evaluation; not a production system.",
        ledger_v01.FILES, U,
        extra={"entry_functions": "ledger_new,ledger_handle"},
    )
    print("wrote", os.path.normpath(root))
