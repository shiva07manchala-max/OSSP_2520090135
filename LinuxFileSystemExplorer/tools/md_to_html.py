#!/usr/bin/env python3
"""
md_to_html.py - convert TEAM_GUIDE_A_TO_Z.md (a constrained Markdown subset)
into a standalone, print-friendly HTML file.

Uses ONLY the Python standard library. Supports exactly the constructs used
in the team guide: headings, fenced code blocks, pipe tables, bullet and
numbered lists, blockquotes, horizontal rules, **bold**, *italic*, `code`.

Usage:
    python3 md_to_html.py INPUT.md OUTPUT.html [PAGE_TITLE]
"""

import html
import re
import sys

CSS = """
:root { --accent:#0b5394; --code-bg:#f5f5f5; --line:#cccccc; }
* { box-sizing: border-box; }
body { font-family: Georgia, 'Times New Roman', serif; font-size: 11pt;
       line-height: 1.5; color: #1a1a1a; margin: 0 auto; max-width: 190mm;
       padding: 6mm 2mm; }
h1 { font-family: 'Segoe UI', Arial, sans-serif; font-size: 20pt; color: var(--accent);
     border-bottom: 3px solid var(--accent); padding-bottom: 6px; margin-top: 24px; }
h2 { font-family: 'Segoe UI', Arial, sans-serif; font-size: 15pt; color: var(--accent);
     border-bottom: 1px solid var(--line); padding-bottom: 4px; margin-top: 26px;
     page-break-after: avoid; }
h3 { font-family: 'Segoe UI', Arial, sans-serif; font-size: 12.5pt; color: #333;
     margin-top: 20px; page-break-after: avoid; }
p  { margin: 8px 0; }
code { font-family: Consolas, 'Courier New', monospace; font-size: 9.5pt;
       background: var(--code-bg); padding: 1px 4px; border-radius: 3px;
       border: 1px solid #e0e0e0; }
pre { background: var(--code-bg); border: 1px solid var(--line);
      border-left: 4px solid var(--accent); padding: 10px 12px;
      border-radius: 4px; overflow-x: auto; page-break-inside: avoid; }
pre code { background: none; border: none; padding: 0; font-size: 9pt; line-height: 1.45; }
table { border-collapse: collapse; width: 100%; margin: 12px 0;
        font-size: 10pt; page-break-inside: avoid; }
th, td { border: 1px solid var(--line); padding: 5px 8px; text-align: left;
         vertical-align: top; }
th { background: #eef3f8; font-family: 'Segoe UI', Arial, sans-serif; }
tr:nth-child(even) td { background: #fafafa; }
blockquote { border-left: 4px solid var(--accent); background: #f2f7fc;
             margin: 12px 0; padding: 8px 14px; color: #333;
             font-style: italic; }
blockquote p { margin: 4px 0; }
hr { border: none; border-top: 2px solid var(--line); margin: 22px 0; }
ul, ol { margin: 8px 0; padding-left: 26px; }
li { margin: 4px 0; }
strong { color: #000; }
@page { size: A4; margin: 18mm 16mm; }
@media print { body { max-width: none; padding: 0; } }
"""


def esc(s: str) -> str:
    return html.escape(s, quote=False)


def inline(s: str) -> str:
    """Inline markdown -> HTML: code spans, bold, italic (order matters)."""
    s = esc(s)
    s = re.sub(r"`([^`]+)`", r"<code>\1</code>", s)
    s = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", s)
    s = re.sub(r"(?<!\*)\*([^*\n]+)\*(?!\*)", r"<em>\1</em>", s)
    return s


def split_row(line: str):
    """Split a markdown table row into cells."""
    parts = line.strip().strip("|").split("|")
    return [c.strip() for c in parts]


def is_sep_row(line: str) -> bool:
    """True if the line is a |---|---| table separator."""
    return bool(re.match(r"^\s*\|?\s*:?-{3,}", line)) and "-" in line and "|" in line


def convert(md_text: str) -> str:
    lines = md_text.split("\n")
    out = []
    i = 0
    para = []

    def flush_para():
        if para:
            out.append("<p>%s</p>" % inline(" ".join(para)))
            para.clear()

    while i < len(lines):
        line = lines[i]

        # fenced code block
        if line.startswith("```"):
            flush_para()
            i += 1
            code = []
            while i < len(lines) and not lines[i].startswith("```"):
                code.append(lines[i])
                i += 1
            i += 1  # skip closing fence
            out.append("<pre><code>%s</code></pre>" % esc("\n".join(code)))
            continue

        # blank line
        if line.strip() == "":
            flush_para()
            i += 1
            continue

        # heading
        m = re.match(r"^(#{1,6})\s+(.*)$", line)
        if m:
            flush_para()
            level = len(m.group(1))
            out.append("<h%d>%s</h%d>" % (level, inline(m.group(2)), level))
            i += 1
            continue

        # horizontal rule
        if re.match(r"^\s*-{3,}\s*$", line):
            flush_para()
            out.append("<hr>")
            i += 1
            continue

        # table (header row followed by separator row)
        if line.lstrip().startswith("|") and i + 1 < len(lines) \
                and is_sep_row(lines[i + 1]):
            flush_para()
            header = split_row(line)
            i += 2
            rows = []
            while i < len(lines) and lines[i].lstrip().startswith("|"):
                rows.append(split_row(lines[i]))
                i += 1
            t = ["<table>", "<tr>"]
            for c in header:
                t.append("<th>%s</th>" % inline(c))
            t.append("</tr>")
            for r in rows:
                t.append("<tr>")
                for c in r:
                    t.append("<td>%s</td>" % inline(c))
                t.append("</tr>")
            t.append("</table>")
            out.append("".join(t))
            continue

        # blockquote
        if line.lstrip().startswith(">"):
            flush_para()
            quote = []
            while i < len(lines) and lines[i].lstrip().startswith(">"):
                quote.append(re.sub(r"^\s*>\s?", "", lines[i]))
                i += 1
            out.append("<blockquote><p>%s</p></blockquote>"
                       % inline(" ".join(quote)))
            continue

        # unordered list
        if re.match(r"^\s*[-*]\s+", line):
            flush_para()
            items = []
            while i < len(lines) and re.match(r"^\s*[-*]\s+", lines[i]):
                items.append(re.sub(r"^\s*[-*]\s+", "", lines[i]))
                i += 1
            out.append("<ul>%s</ul>" % "".join(
                "<li>%s</li>" % inline(it) for it in items))
            continue

        # ordered list
        if re.match(r"^\s*\d+\.\s+", line):
            flush_para()
            items = []
            while i < len(lines) and re.match(r"^\s*\d+\.\s+", lines[i]):
                items.append(re.sub(r"^\s*\d+\.\s+", "", lines[i]))
                i += 1
            out.append("<ol>%s</ol>" % "".join(
                "<li>%s</li>" % inline(it) for it in items))
            continue

        # paragraph text (accumulate)
        para.append(line.strip())
        i += 1

    flush_para()
    return "\n".join(out)


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: md_to_html.py INPUT.md OUTPUT.html [TITLE]")
        return 2
    src, dst = sys.argv[1], sys.argv[2]
    title = sys.argv[3] if len(sys.argv) > 3 else "Team Guide"
    with open(src, "r", encoding="utf-8") as f:
        md_text = f.read()
    body = convert(md_text)
    doc = ("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
           "<meta charset=\"utf-8\">\n"
           "<title>%s</title>\n<style>%s</style>\n</head>\n<body>\n%s\n"
           "</body>\n</html>\n" % (esc(title), CSS, body))
    with open(dst, "w", encoding="utf-8") as f:
        f.write(doc)
    print("Wrote %s" % dst)
    return 0


if __name__ == "__main__":
    sys.exit(main())
