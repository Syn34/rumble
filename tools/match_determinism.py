#!/usr/bin/env python3
"""
match_determinism.py - runs the same headless offline match twice and checks that the
simulation produced the same checksums (one every 60 ticks).

    python3 tools/match_determinism.py build/src/app/RangerRumble --match q_ffa_megalopolis_01b \
        --ticks 600 [--args="--approx-math --content ... --engine-lib ... --data ..."]

Content paths can also come from userdata/settings.ini (RangerRumble --save-content).
Prints numbers only. Exit code 0 when both runs agree and at least one checksum was seen.
"""
import argparse
import re
import shlex
import subprocess
import sys

LINE = re.compile(r"^tick (\d+) checksum ([0-9a-f]{16}) entities (\d+)$")


def run(binary, match, ticks, extra):
    cmd = [binary, "--match", match, "--headless", "--quiet", "--ticks", str(ticks)] + extra
    result = subprocess.run(cmd, capture_output=True, text=True)
    lines = [m.groups() for m in map(LINE.match, result.stdout.splitlines()) if m]
    return result.returncode, lines, result.stderr


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary")
    ap.add_argument("--match", required=True)
    ap.add_argument("--ticks", type=int, default=600)
    ap.add_argument("--args", default="", help="extra arguments for RangerRumble")
    args = ap.parse_args()
    extra = shlex.split(args.args)

    runs = []
    for i in range(2):
        code, lines, err = run(args.binary, args.match, args.ticks, extra)
        if code != 0:
            sys.stderr.write(err)
            print(f"run {i + 1}: exit code {code}")
            return 1
        runs.append(lines)
        print(f"run {i + 1}: {len(lines)} checksums")
    if not runs[0]:
        print("no checksums were printed")
        return 1
    if runs[0] != runs[1]:
        for a, b in zip(runs[0], runs[1]):
            if a != b:
                print(f"first difference at tick {a[0]}: {a[1]} vs {b[1]}")
                break
        else:
            print("the runs printed different numbers of checksums")
        return 1
    print(f"deterministic: {len(runs[0])} checksums match (last tick {runs[0][-1][0]}: {runs[0][-1][1]})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
