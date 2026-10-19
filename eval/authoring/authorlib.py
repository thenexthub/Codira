"""Shared machinery for authoring a version history.

A history is v01 (a dict path -> text) plus a list of Update records. Each
update is a list of edits (path, old, new) applied to the previous version;
`old` must occur exactly once, so an edit cannot silently miss. The script
writes one directory per version and a manifest.toml with the labels. The
derived facts (which functions changed, which type declarations changed) are
computed from the texts, never typed in by hand.
"""
import os
import re
import shutil
from dataclasses import dataclass, field

CATEGORIES = (
    "bug-fix",
    "refactor",
    "performance",
    "feature",
    "regression",
    "contract-change",
    "effect-change",
)


@dataclass
class Update:
    id: str
    category: str
    title: str
    provenance: str  # "invented" | "pattern"
    derivation: str
    edits: list = field(default_factory=list)  # (path, old, new)
    new_files: dict = field(default_factory=dict)  # path -> text
    # What an author would declare as intended for the update checker.
    behaviour: str = "preserved"  # "preserved" | "changed" | "new-api"
    contract: str = "none"  # free text: how a published contract changes
    effect: str = "none"  # free text: how an effect summary changes
    migrate: str = "none"  # "none" | "zero-default-ok" | "needs-migrate: ..."
    notes: str = ""


FUNC_RE = re.compile(r"^(?:@[^\n]*\n)*\s*(?:public\s+|internal\s+)?func\s+([A-Za-z_][A-Za-z0-9_]*)", re.M)
TYPE_RE = re.compile(r"^\s*(?:public\s+|internal\s+)?(struct|class|enum)\s+([A-Za-z_][A-Za-z0-9_]*)", re.M)


def split_items(text):
    """Split a module into top-level items keyed by ('func'|'type', name).

    Items start at column 0; attribute and comment lines directly above an
    item belong to it. Imports and blank lines belong to nothing.
    """
    lines = text.split("\n")
    start_re = re.compile(r"^(?:public\s+|internal\s+)?(func|struct|class|enum|extend)\s+([A-Za-z_][A-Za-z0-9_]*)")
    starts = []
    for i, line in enumerate(lines):
        m = start_re.match(line)
        if m:
            j = i
            while j > 0 and (lines[j - 1].startswith("@") or lines[j - 1].startswith("//")):
                j -= 1
            kind = "func" if m.group(1) == "func" else "type"
            starts.append((j, i, (kind, m.group(2))))
    items = {}
    for n, (j, i, key) in enumerate(starts):
        end = starts[n + 1][0] if n + 1 < len(starts) else len(lines)
        items[key] = "\n".join(lines[j:end]).strip()
    return items


def summarize_diff(old_files, new_files):
    changed, added, removed, types_changed, types_added, files = [], [], [], [], [], []
    for path in sorted(set(old_files) | set(new_files)):
        if not path.endswith(".code"):
            if old_files.get(path) != new_files.get(path):
                files.append(path)
            continue
        o, n = old_files.get(path), new_files.get(path)
        if o == n:
            continue
        files.append(path)
        oi = split_items(o) if o is not None else {}
        ni = split_items(n) if n is not None else {}
        mod = os.path.splitext(os.path.basename(path))[0]
        for key in sorted(set(oi) | set(ni)):
            kind, name = key
            q = f"{mod}.{name}"
            if key not in oi:
                (added if kind == "func" else types_added).append(q)
            elif key not in ni:
                (removed if kind == "func" else types_changed).append(q)
            elif oi[key] != ni[key]:
                (changed if kind == "func" else types_changed).append(q)
    return files, changed, added, removed, types_changed, types_added


def apply_edits(files, edits):
    """Apply (path, old, new) edits in order.

    `old` is either exact text that must occur once, or a pair
    ("func" | "type", name) meaning "replace the whole top-level item of that
    name (with its attributes and comments) by `new`".
    """
    files = dict(files)
    for path, old, new in edits:
        assert path in files, f"no such file {path}"
        if isinstance(old, tuple):
            items = split_items(files[path])
            assert old in items, f"{path} has no item {old}"
            old = items[old]
            new = new.strip()
        n = files[path].count(old)
        assert n == 1, f"edit in {path} matches {n} times: {old[:60]!r}"
        files[path] = files[path].replace(old, new)
    return files


def q(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def qlist(xs):
    return "[" + ", ".join(q(x) for x in xs) + "]"


def write_history(root, name, description, v1, updates, extra=None):
    """Write root/v01..vNN and root/manifest.toml. Returns the version list."""
    if os.path.isdir(root):
        shutil.rmtree(root)
    os.makedirs(root)
    versions = [dict(v1)]
    for u in updates:
        assert u.category in CATEGORIES, u.category
        cur = apply_edits(versions[-1], u.edits)
        cur.update(u.new_files)
        versions.append(cur)
    for i, files in enumerate(versions, 1):
        vdir = os.path.join(root, f"v{i:02d}")
        for path, text in files.items():
            full = os.path.join(vdir, path)
            os.makedirs(os.path.dirname(full), exist_ok=True)
            with open(full, "w", newline="\n") as f:
                f.write(text)
    out = []
    out.append("# Generated by eval/authoring/author_%s.py -- edit that script, not this file." % name)
    out.append("[workload]")
    out.append(f"name = {q(name)}")
    out.append(f"description = {q(description)}")
    out.append(f"versions = {len(versions)}")
    for k, v in (extra or {}).items():
        out.append(f"{k} = {q(v)}")
    for i, u in enumerate(updates):
        files, changed, added, removed, types_changed, types_added = summarize_diff(versions[i], versions[i + 1])
        layout = bool(types_changed)
        out.append("")
        out.append("[[update]]")
        out.append(f"id = {q(u.id)}")
        out.append(f"from = {q('v%02d' % (i + 1))}")
        out.append(f"to = {q('v%02d' % (i + 2))}")
        out.append(f"category = {q(u.category)}")
        out.append(f"title = {q(u.title)}")
        out.append(f"provenance = {q(u.provenance)}")
        out.append(f"derivation = {q(u.derivation)}")
        out.append(f"behaviour = {q(u.behaviour)}")
        out.append(f"contract = {q(u.contract)}")
        out.append(f"effect = {q(u.effect)}")
        out.append(f"migrate = {q(u.migrate)}")
        out.append(f"layout_change = {'true' if layout else 'false'}")
        out.append(f"files = {qlist(files)}")
        out.append(f"changed_functions = {qlist(changed)}")
        out.append(f"added_functions = {qlist(added)}")
        out.append(f"removed_functions = {qlist(removed)}")
        out.append(f"changed_types = {qlist(types_changed)}")
        out.append(f"added_types = {qlist(types_added)}")
        if u.notes:
            out.append(f"notes = {q(u.notes)}")
    with open(os.path.join(root, "manifest.toml"), "w", newline="\n") as f:
        f.write("\n".join(out) + "\n")
    return versions
