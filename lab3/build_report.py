#!/usr/bin/env python3
"""Lab3_tailan.md → Lab3_tailan.pdf (markdown + weasyprint)."""
from pathlib import Path

import markdown
from weasyprint import HTML

HERE = Path(__file__).resolve().parent
SRC = HERE / "Lab3_tailan.md"
OUT = HERE / "Lab3_tailan.pdf"

CSS = """
@page { size: A4; margin: 18mm 16mm; }
body { font-family: "DejaVu Sans", "Noto Sans", sans-serif; font-size: 10pt; line-height: 1.4; }
h1 { font-size: 18pt; border-bottom: 2px solid #333; padding-bottom: 4px; }
h2 { font-size: 14pt; margin-top: 18px; border-bottom: 1px solid #999; }
h3 { font-size: 11.5pt; margin-top: 14px; }
table { border-collapse: collapse; margin: 6px 0 10px 0; }
th, td { border: 1px solid #888; padding: 3px 8px; }
th { background: #e8e8e8; }
td { text-align: left; }
code { font-family: "DejaVu Sans Mono", monospace; font-size: 9pt; background: #f2f2f2; padding: 0 2px; }
pre { background: #f2f2f2; padding: 6px 8px; border-left: 3px solid #999; }
pre code { background: none; padding: 0; }
img { width: 85%; display: block; margin: 8px auto; }
h2, h3 { page-break-after: avoid; }
table, pre, img { page-break-inside: avoid; }
"""


def main():
    body = markdown.markdown(SRC.read_text(encoding="utf-8"),
                             extensions=["tables", "fenced_code"])
    html = f"<html><head><meta charset='utf-8'><style>{CSS}</style></head><body>{body}</body></html>"
    HTML(string=html, base_url=str(HERE)).write_pdf(OUT)
    print(f"Үүсгэлээ: {OUT}")


if __name__ == "__main__":
    main()
