#!/usr/bin/env python3
"""
progress.py - measure how much of the original code has been reimplemented and
update the progress section of README.md.

A method counts as done when its original address appears in a source comment
under src/game/ (the project convention is `// 0xADDR` above each reimplemented
function) and is not inside a `TODO(0xADDR, ...)` marker.

Inputs:
  * the method lists written by `il2cpp_inventory.py --assemblies all` (local,
    derived from your copy of the game; never committed)
  * the source tree

Outputs (committed; contain only counts):
  * docs/progress.json
  * the block between <!-- progress:start --> and <!-- progress:end --> in README.md

Without --inventory, README.md is regenerated from docs/progress.json.

Usage:
    python3 tools/progress.py --inventory build/inventory
"""
import argparse
import datetime
import glob
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROGRESS_JSON = os.path.join(ROOT, "docs", "progress.json")
README = os.path.join(ROOT, "README.md")
START, END = "<!-- progress:start -->", "<!-- progress:end -->"

# Assemblies written by the game's developer; everything else shipped with the
# game is third-party code (which is also being reimplemented).
GAME_PREFIXES = ("quantum.code", "GameClasses", "GameData", "Assembly-CSharp", "Fidelio",
                 "FidelioFramework", "Ohbibi", "OhBibi", "OhbibiCore", "AFuncs", "com.ohbibi")

ADDRESS = re.compile(r"0x[0-9a-fA-F]{6,}")
TODO_ADDRESS = re.compile(r"TODO\((0x[0-9a-fA-F]{6,})")


def referenced_addresses():
    done, todo = set(), set()
    for path in glob.glob(os.path.join(ROOT, "src", "game", "**", "*.*"), recursive=True):
        if not path.endswith((".h", ".hpp", ".cpp")):
            continue
        with open(path, encoding="utf-8", errors="replace") as f:
            for line in f:
                todo.update(int(a, 16) for a in TODO_ADDRESS.findall(line))
                if "TODO(" in line:
                    continue
                done.update(int(a, 16) for a in ADDRESS.findall(line))
    return done - todo


def measure(inventory_dir):
    done_addresses = referenced_addresses()
    assemblies = []
    for path in sorted(glob.glob(os.path.join(inventory_dir, "*.methods.tsv"))):
        name = os.path.basename(path)[: -len(".methods.tsv")]
        total = done = 0
        with open(path, encoding="utf-8") as f:
            next(f)  # header
            for line in f:
                cols = line.rstrip("\n").split("\t")
                total += 1
                address = int(cols[1], 16)
                if address and address in done_addresses:
                    done += 1
        if total:
            assemblies.append({"assembly": name, "methods": total, "done": done,
                               "game": name.startswith(GAME_PREFIXES)})
    return {"updated": datetime.date.today().isoformat(), "assemblies": assemblies}


def pct(done, total):
    return 100.0 * done / total if total else 0.0


def bar(fraction, width=30):
    filled = round(fraction * width)
    return "█" * filled + "░" * (width - filled)


def render(data):
    rows = data["assemblies"]
    total = sum(r["methods"] for r in rows)
    done = sum(r["done"] for r in rows)
    game = [r for r in rows if r["game"]]
    third = [r for r in rows if not r["game"]]
    g_total, g_done = sum(r["methods"] for r in game), sum(r["done"] for r in game)
    t_total, t_done = sum(r["methods"] for r in third), sum(r["done"] for r in third)

    lines = [
        START,
        f"**Overall: {pct(done, total):.2f}%** — {done:,} of {total:,} original methods reimplemented",
        "",
        f"`{bar(done / total if total else 0)}`",
        "",
        "| Part | Done | Methods | Progress |",
        "|---|---:|---:|---:|",
        f"| Game code | {g_done:,} | {g_total:,} | {pct(g_done, g_total):.2f}% |",
        f"| Third-party libraries | {t_done:,} | {t_total:,} | {pct(t_done, t_total):.2f}% |",
        "",
        "<details><summary>Per assembly (started ones first, then largest)</summary>",
        "",
        "| Assembly | Done | Methods | Progress |",
        "|---|---:|---:|---:|",
    ]
    ordered = sorted(rows, key=lambda r: (r["done"] == 0, -pct(r["done"], r["methods"]), -r["methods"]))
    for r in ordered[:40]:
        lines.append(f"| {r['assembly']} | {r['done']:,} | {r['methods']:,} | "
                     f"{pct(r['done'], r['methods']):.1f}% |")
    if len(ordered) > 40:
        rest = ordered[40:]
        lines.append(f"| *{len(rest)} more* | {sum(r['done'] for r in rest):,} | "
                     f"{sum(r['methods'] for r in rest):,} | |")
    lines += [
        "",
        "</details>",
        "",
        f"<sub>Counted by `tools/progress.py` on {data['updated']}: a method is done when its "
        "original address is cited above a reimplementation under `src/game/`. The engine "
        "layer (platform, renderers, asset readers) is new code and isn't counted.</sub>",
        END,
    ]
    return "\n".join(lines)


def update_readme(block):
    with open(README, encoding="utf-8") as f:
        text = f.read()
    if START in text and END in text:
        before = text[: text.index(START)]
        after = text[text.index(END) + len(END):]
        text = before + block + after
    else:
        text = text.replace("## Status\n", "## Status\n\n" + block + "\n", 1)
    with open(README, "w", encoding="utf-8") as f:
        f.write(text)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--inventory", help="folder with <assembly>.methods.tsv for all assemblies")
    args = ap.parse_args()

    if args.inventory:
        data = measure(args.inventory)
        if not data["assemblies"]:
            ap.error("no method lists found; run il2cpp_inventory.py --assemblies all first")
        with open(PROGRESS_JSON, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=1)
            f.write("\n")
    else:
        with open(PROGRESS_JSON, encoding="utf-8") as f:
            data = json.load(f)

    update_readme(render(data))
    rows = data["assemblies"]
    total = sum(r["methods"] for r in rows)
    done = sum(r["done"] for r in rows)
    print(f"{done} / {total} methods ({pct(done, total):.2f}%)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
