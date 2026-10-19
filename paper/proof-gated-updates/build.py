#!/usr/bin/env python3
"""Builds the paper from one Markdown source.

    python build.py [--pdf]

Inputs:   paper.md (the text), refs.json (the bibliography), figs/*.png
Outputs:  paper.tex, refs.bib (committed), build/paper.html and, with --pdf,
          paper.pdf (printed from the HTML by Microsoft Edge in headless mode).

Why not pandoc or LaTeX: neither is installed on the development machine. The
LaTeX output is plain `article` (two columns, no venue template) so that it can
be moved into the USENIX or ACM template when a venue is chosen.

Source conventions (a small subset on purpose):
  %key: value                    metadata at the top (title, author, date)
  # / ## / ###                   section, subsection, subsubsection
  # Abstract                     the abstract (unnumbered)
  {#label} after a heading       a label; {ref:label} refers to it
  **bold** *italic* `code` $math$ [@key1; @key2]   inline markup
  {{TODO: text}}                 a visible placeholder (red) for a missing result
  {{NOTE: text}}                 an author-only note (grey, remove before submission)
  - item / 1. item               lists
  ```...```                      code block
  $$ ... $$                      display math (alone on lines)
  Table: caption {#label}        caption for the pipe table that follows
  ![caption](figs/x.png){#label} a figure
  > text                         a block quote (used for definitions and theorems)
"""
import html as htmllib
import json
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "paper.md")
REFS = os.path.join(HERE, "refs.json")
OUT_TEX = os.path.join(HERE, "paper.tex")
OUT_BIB = os.path.join(HERE, "refs.bib")
BUILD = os.path.join(HERE, "build")
KATEX = os.environ.get("KATEX_DIST", "")  # a katex/dist directory, for math in the HTML

INLINE = re.compile(
    r"(?P<math>\$[^$\n]+\$)"
    r"|(?P<code>`[^`\n]+`)"
    r"|(?P<todo>\{\{(?:TODO|NOTE)(?::.*?)?\}\})"
    r"|(?P<cite>\[@[^\]]+\])"
    r"|(?P<ref>\{ref:[A-Za-z0-9:_-]+\})"
    r"|(?P<bold>\*\*.+?\*\*)"
    r"|(?P<ital>\*[^*\s][^*]*?\*)"
)


def load_refs():
    return json.load(open(REFS, encoding="utf-8"))


def tex_escape(s):
    out = []
    for ch in s:
        out.append({"&": r"\&", "%": r"\%", "#": r"\#", "_": r"\_", "{": r"\{", "}": r"\}",
                    "~": r"\textasciitilde{}", "^": r"\textasciicircum{}", "\\": r"\textbackslash{}",
                    "<": r"\textless{}", ">": r"\textgreater{}", "|": r"\textbar{}"}.get(ch, ch))
    return "".join(out)


class Doc:
    """Parsed document: metadata, and a list of blocks."""

    def __init__(self, text):
        self.meta, self.blocks = {}, []
        lines = text.split("\n")
        i = 0
        while i < len(lines) and lines[i].startswith("%"):
            k, _, v = lines[i][1:].partition(":")
            self.meta[k.strip()] = v.strip()
            i += 1
        self.parse(lines[i:])

    def parse(self, lines):
        i, n = 0, len(lines)
        para = []

        def flush():
            if para:
                self.blocks.append(("p", " ".join(x.strip() for x in para)))
                para.clear()

        while i < n:
            ln = lines[i]
            if not ln.strip():
                flush(); i += 1; continue
            if ln.startswith("```"):
                flush(); i += 1; code = []
                while i < n and not lines[i].startswith("```"):
                    code.append(lines[i]); i += 1
                i += 1; self.blocks.append(("code", "\n".join(code))); continue
            if ln.strip() == "$$":
                flush(); i += 1; m = []
                while i < n and lines[i].strip() != "$$":
                    m.append(lines[i]); i += 1
                i += 1; self.blocks.append(("math", "\n".join(m))); continue
            h = re.match(r"^(#{1,3}) (.*?)(?: \{#([A-Za-z0-9:_-]+)\})?$", ln)
            if h:
                flush(); self.blocks.append(("h", len(h.group(1)), h.group(2), h.group(3))); i += 1; continue
            if ln.startswith("Table:"):
                flush()
                cap = re.match(r"^Table: (.*?)(?: \{#([A-Za-z0-9:_-]+)\})?$", ln)
                i += 1
                while i < n and not lines[i].strip(): i += 1
                rows = []
                while i < n and lines[i].lstrip().startswith("|"):
                    rows.append([c.strip() for c in lines[i].strip().strip("|").split("|")]); i += 1
                self.blocks.append(("table", cap.group(1), cap.group(2), rows)); continue
            f = re.match(r"^!\[(.*?)\]\((.*?)\)(?:\{#([A-Za-z0-9:_-]+)\})?$", ln)
            if f:
                flush(); self.blocks.append(("fig", f.group(1), f.group(2), f.group(3))); i += 1; continue
            if re.match(r"^(- |\d+\. )", ln):
                flush(); ordered = bool(re.match(r"^\d+\. ", ln)); items = []
                while i < n and (re.match(r"^(- |\d+\. )", lines[i]) or (lines[i].startswith("  ") and items)):
                    if re.match(r"^(- |\d+\. )", lines[i]):
                        items.append(re.sub(r"^(- |\d+\. )", "", lines[i]).strip())
                    else:
                        items[-1] += " " + lines[i].strip()
                    i += 1
                self.blocks.append(("list", ordered, items)); continue
            if ln.startswith(">"):
                flush(); q = []
                while i < n and lines[i].startswith(">"):
                    q.append(lines[i][1:].strip()); i += 1
                self.blocks.append(("quote", " ".join(q))); continue
            para.append(ln); i += 1
        flush()


def number_things(doc):
    """Numbers sections, tables and figures; returns label -> display string."""
    labels, sec = {}, [0, 0, 0]
    t = f = 0
    for b in doc.blocks:
        if b[0] == "h":
            level, title, lab = b[1], b[2], b[3]
            if title == "Abstract":
                continue
            sec[level - 1] += 1
            for j in range(level, 3): sec[j] = 0
            num = ".".join(str(x) for x in sec[:level])
            if lab: labels[lab] = ("Section " + num, num)
        elif b[0] == "table":
            t += 1
            if b[2]: labels[b[2]] = (f"Table {t}", str(t))
        elif b[0] == "fig":
            f += 1
            if b[3]: labels[b[3]] = (f"Figure {f}", str(f))
    return labels


def cite_keys(s):
    return [k.strip().lstrip("@") for k in s[2:-1].split(";")]


def inline_tex(s, refs, labels):
    out, pos = [], 0
    for m in INLINE.finditer(s):
        out.append(tex_escape(s[pos:m.start()])); pos = m.end()
        k, v = m.lastgroup, m.group()
        if k == "math": out.append(v)
        elif k == "code": out.append(r"\texttt{" + tex_escape(v[1:-1]) + "}")
        elif k == "todo":
            kind, _, body = v[2:-2].partition(":")
            if kind == "TODO": out.append(r"\todo{" + tex_escape(body.strip()) + "}")
            else: out.append(r"\authornote{" + tex_escape(body.strip()) + "}")
        elif k == "cite":
            for key in cite_keys(v):
                if key not in refs: raise SystemExit(f"unknown citation: {key}")
            out.append(r"\cite{" + ",".join(cite_keys(v)) + "}")
        elif k == "ref":
            lab = v[5:-1]
            if lab not in labels: raise SystemExit(f"unknown label: {lab}")
            out.append(labels[lab][0].split(" ")[0] + "~\\ref{" + lab + "}")
        elif k == "bold": out.append(r"\textbf{" + inline_tex(v[2:-2], refs, labels) + "}")
        elif k == "ital": out.append(r"\emph{" + inline_tex(v[1:-1], refs, labels) + "}")
    out.append(tex_escape(s[pos:]))
    return "".join(out)


def to_tex(doc, refs, labels):
    pre = r"""\documentclass[10pt,twocolumn]{article}
\usepackage[margin=0.75in]{geometry}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{amsmath,amssymb,booktabs,graphicx,xcolor,url}
\usepackage[colorlinks=true,linkcolor=blue!50!black,citecolor=blue!50!black,urlcolor=blue!50!black]{hyperref}
\newcommand{\todo}[1]{{\color{red}\textbf{[TODO: #1]}}}
\newcommand{\authornote}[1]{{\color{gray}\textit{[note: #1]}}}
\sloppy
"""
    m = doc.meta
    pre += "\\title{" + tex_escape(m.get("title", "")) + "}\n\\author{" + tex_escape(m.get("author", "")) + "}\n"
    pre += "\\date{" + tex_escape(m.get("date", "")) + "}\n\\begin{document}\n\\maketitle\n"
    out = [pre]
    for b in doc.blocks:
        k = b[0]
        if k == "h":
            level, title, lab = b[1], b[2], b[3]
            if title == "Abstract":
                out.append("\\begin{abstract}\n"); continue
            cmd = ["section", "subsection", "subsubsection"][level - 1]
            out.append("\\" + cmd + "{" + inline_tex(title, refs, labels) + "}" + (f"\\label{{{lab}}}" if lab else "") + "\n")
        elif k == "p": out.append(inline_tex(b[1], refs, labels) + "\n\n")
        elif k == "code": out.append("\\begin{small}\\begin{verbatim}\n" + b[1] + "\n\\end{verbatim}\\end{small}\n")
        elif k == "math": out.append("\\begin{equation*}\n" + b[1] + "\n\\end{equation*}\n")
        elif k == "list":
            env = "enumerate" if b[1] else "itemize"
            out.append("\\begin{" + env + "}\n" + "".join("\\item " + inline_tex(x, refs, labels) + "\n" for x in b[2]) + "\\end{" + env + "}\n")
        elif k == "quote": out.append("\\begin{quote}\n" + inline_tex(b[1], refs, labels) + "\n\\end{quote}\n")
        elif k == "table":
            rows = b[3]; ncol = len(rows[0])
            aligns = [c for c in rows[1]]
            spec = "".join("r" if a.endswith(":") and not a.startswith(":") else "c" if a.startswith(":") and a.endswith(":") else "l" for a in aligns)
            body = [rows[0]] + rows[2:]
            lab = f"\\label{{{b[2]}}}" if b[2] else ""
            out.append("\\begin{table*}[t]\\centering\\small\n\\begin{tabular}{" + spec + "}\\toprule\n")
            for ri, r in enumerate(body):
                out.append(" & ".join(inline_tex(c, refs, labels) for c in r) + " \\\\\n")
                if ri == 0: out.append("\\midrule\n")
            out.append("\\bottomrule\n\\end{tabular}\n\\caption{" + inline_tex(b[1], refs, labels) + "}" + lab + "\n\\end{table*}\n")
        elif k == "fig":
            lab = f"\\label{{{b[3]}}}" if b[3] else ""
            out.append("\\begin{figure*}[t]\\centering\n\\includegraphics[width=0.85\\textwidth]{" + b[2] + "}\n\\caption{" + inline_tex(b[1], refs, labels) + "}" + lab + "\n\\end{figure*}\n")
    out.append("\\end{abstract}\n" if False else "")
    out.append("\\bibliographystyle{plain}\n\\bibliography{refs}\n\\end{document}\n")
    text = "".join(out)
    # the abstract environment must close before the first numbered section
    first = text.find("\\section{")
    if "\\begin{abstract}" in text and first != -1:
        text = text[:first] + "\\end{abstract}\n" + text[first:]
    return text


def to_bib(refs):
    out = []
    for key, r in refs.items():
        t = r.get("type", "inproceedings")
        fields = [("author", r.get("author")), ("title", "{" + r["title"] + "}"),
                  ("booktitle" if t == "inproceedings" else "journal" if t == "article" else "howpublished", r.get("venue")),
                  ("year", r.get("year")), ("url", r.get("url")), ("note", r.get("note"))]
        body = ",\n".join(f"  {k} = {{{v}}}" for k, v in fields if v)
        out.append(f"@{t}{{{key},\n{body}\n}}\n")
    return "\n".join(out)


def inline_html(s, refs, labels, order):
    out, pos = [], 0
    for m in INLINE.finditer(s):
        out.append(htmllib.escape(s[pos:m.start()])); pos = m.end()
        k, v = m.lastgroup, m.group()
        if k == "math": out.append(htmllib.escape(v))
        elif k == "code": out.append("<code>" + htmllib.escape(v[1:-1]) + "</code>")
        elif k == "todo":
            kind, _, body = v[2:-2].partition(":")
            cls = "todo" if kind == "TODO" else "note"
            out.append(f'<span class="{cls}">[{kind}: {htmllib.escape(body.strip())}]</span>')
        elif k == "cite":
            nums = []
            for key in cite_keys(v):
                if key not in refs: raise SystemExit(f"unknown citation: {key}")
                if key not in order: order.append(key)
                nums.append(f'<a href="#ref-{key}">{order.index(key) + 1}</a>')
            out.append("[" + ", ".join(nums) + "]")
        elif k == "ref":
            lab = v[5:-1]
            out.append(f'<a href="#{lab}">{labels[lab][0]}</a>')
        elif k == "bold": out.append("<b>" + inline_html(v[2:-2], refs, labels, order) + "</b>")
        elif k == "ital": out.append("<i>" + inline_html(v[1:-1], refs, labels, order) + "</i>")
    out.append(htmllib.escape(s[pos:]))
    return "".join(out)


def to_html(doc, refs, labels):
    order, body, sec = [], [], [0, 0, 0]
    tcount = fcount = 0
    in_abs = False
    for b in doc.blocks:
        k = b[0]
        if k == "h":
            level, title, lab = b[1], b[2], b[3]
            if title == "Abstract":
                body.append('<div class="abstract"><h2>Abstract</h2>'); in_abs = True; continue
            if in_abs: body.append("</div>"); in_abs = False
            sec[level - 1] += 1
            for j in range(level, 3): sec[j] = 0
            num = ".".join(str(x) for x in sec[:level])
            idattr = f' id="{lab}"' if lab else ""
            body.append(f"<h{level + 1}{idattr}>{num} {inline_html(title, refs, labels, order)}</h{level + 1}>")
        elif k == "p": body.append("<p>" + inline_html(b[1], refs, labels, order) + "</p>")
        elif k == "code": body.append("<pre>" + htmllib.escape(b[1]) + "</pre>")
        elif k == "math": body.append('<div class="math">$$' + b[1] + "$$</div>")
        elif k == "list":
            tag = "ol" if b[1] else "ul"
            body.append(f"<{tag}>" + "".join("<li>" + inline_html(x, refs, labels, order) + "</li>" for x in b[2]) + f"</{tag}>")
        elif k == "quote": body.append("<blockquote>" + inline_html(b[1], refs, labels, order) + "</blockquote>")
        elif k == "table":
            tcount += 1
            rows = b[3]; idattr = f' id="{b[2]}"' if b[2] else ""
            hdr = "".join("<th>" + inline_html(c, refs, labels, order) + "</th>" for c in rows[0])
            trs = "".join("<tr>" + "".join("<td>" + inline_html(c, refs, labels, order) + "</td>" for c in r) + "</tr>" for r in rows[2:])
            body.append(f'<table{idattr}><caption>Table {tcount}: {inline_html(b[1], refs, labels, order)}</caption><thead><tr>{hdr}</tr></thead><tbody>{trs}</tbody></table>')
        elif k == "fig":
            fcount += 1
            idattr = f' id="{b[3]}"' if b[3] else ""
            body.append(f'<figure{idattr}><img src="../{b[2]}" alt=""><figcaption>Figure {fcount}: {inline_html(b[1], refs, labels, order)}</figcaption></figure>')
    if in_abs: body.append("</div>")
    bib = ["<h2>References</h2><ol class='refs'>"]
    for key in order:
        r = refs[key]
        status = f' <span class="note">[{htmllib.escape(r["status"])}]</span>' if r.get("status", "").startswith("check") else ""
        bib.append(f'<li id="ref-{key}">{htmllib.escape(r.get("author", ""))}. {htmllib.escape(r["title"])}. <i>{htmllib.escape(r.get("venue", ""))}</i>, {r.get("year", "")}.{status}</li>')
    bib.append("</ol>")
    m = doc.meta
    katex = ""
    if KATEX:
        katex = ('<link rel="stylesheet" href="katex/katex.min.css"><script defer src="katex/katex.min.js"></script>'
                 '<script defer src="katex/contrib/auto-render.min.js" onload="renderMathInElement(document.body,{delimiters:[{left:\'$$\',right:\'$$\',display:true},{left:\'$\',right:\'$\',display:false}]})"></script>')
    css = """body{font-family:Georgia,'Times New Roman',serif;max-width:46em;margin:2em auto;padding:0 1em;line-height:1.45;color:#111}
h1{font-size:1.7em;text-align:center;margin-bottom:.2em}.byline{text-align:center;color:#444;margin-bottom:1.5em}
h2{font-size:1.25em;margin-top:1.6em}h3{font-size:1.08em}h4{font-size:1em;font-style:italic}
.abstract{margin:1.5em 2em;font-size:.95em}.abstract h2{text-align:center;font-size:1em;margin:0 0 .4em}
pre{background:#f5f5f5;padding:.6em;overflow-x:auto;font-size:.82em;border-left:3px solid #ccc}code{font-size:.9em}
blockquote{border-left:3px solid #888;margin:1em 0;padding:.1em 1em;background:#fafafa}
table{border-collapse:collapse;margin:1em auto;font-size:.85em}th,td{border-top:1px solid #aaa;border-bottom:1px solid #aaa;padding:.25em .6em}caption{caption-side:top;font-style:italic;padding:.3em}
figure{margin:1.2em 0;text-align:center}figure img{max-width:100%}figcaption{font-size:.9em;font-style:italic}
.todo{color:#b00020;font-weight:bold;background:#fdecea;padding:0 .2em}.note{color:#666;background:#eee;padding:0 .2em;font-size:.9em}
.refs{font-size:.88em}.math{overflow-x:auto}
@media print{body{max-width:none;margin:0;font-size:10.5pt}pre{white-space:pre-wrap}}"""
    return (f"<!doctype html><html><head><meta charset='utf-8'><title>{htmllib.escape(m.get('title', ''))}</title>{katex}<style>{css}</style></head><body>"
            f"<h1>{htmllib.escape(m.get('title', ''))}</h1><div class='byline'>{htmllib.escape(m.get('author', ''))}<br>{htmllib.escape(m.get('date', ''))}</div>"
            + "\n".join(body) + "\n".join(bib) + "</body></html>")


def main():
    refs = load_refs()
    doc = Doc(open(SRC, encoding="utf-8").read())
    labels = number_things(doc)
    open(OUT_TEX, "w", encoding="utf-8", newline="\n").write(to_tex(doc, refs, labels))
    open(OUT_BIB, "w", encoding="utf-8", newline="\n").write(to_bib(refs))
    os.makedirs(BUILD, exist_ok=True)
    if KATEX and os.path.isdir(KATEX):
        dst = os.path.join(BUILD, "katex")
        if not os.path.isdir(dst): shutil.copytree(KATEX, dst)
    html_path = os.path.join(BUILD, "paper.html")
    open(html_path, "w", encoding="utf-8", newline="\n").write(to_html(doc, refs, labels))
    words = sum(len(b[1].split()) for b in doc.blocks if b[0] == "p")
    todos = open(SRC, encoding="utf-8").read().count("{{TODO")
    print(f"wrote paper.tex, refs.bib, build/paper.html  ({words} words of body text, {len(refs)} references, {todos} TODO placeholders)")
    if "--pdf" in sys.argv:
        edge = next((p for p in (r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe", r"C:\Program Files\Microsoft\Edge\Application\msedge.exe") if os.path.exists(p)), None)
        if not edge: raise SystemExit("Microsoft Edge not found; open build/paper.html and print to PDF")
        pdf = os.path.join(HERE, "paper.pdf")
        subprocess.run([edge, "--headless=new", "--disable-gpu", "--no-pdf-header-footer", "--virtual-time-budget=15000",
                        f"--print-to-pdf={pdf}", "file:///" + html_path.replace("\\", "/")], check=True, capture_output=True, timeout=180)
        print("wrote paper.pdf", os.path.getsize(pdf), "bytes")


if __name__ == "__main__":
    main()
