#!/usr/bin/env python3
"""
compare_renderers.py - render the same frames with every backend and compare.

Runs RangerRumble once per renderer with a fixed frame count, captures the final
frame of each, and reports how many pixels differ. Small differences along edges
are normal (rasterisation rules differ slightly between drivers); large ones
point at a backend bug.

Usage:
    python3 tools/compare_renderers.py <path/to/RangerRumble> [--frames N] [--out DIR]
Exit status is non-zero if any pair differs in more than --tolerance of pixels.
"""
import argparse
import os
import subprocess
import sys


def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    # Header: P6 \n width height \n 255 \n
    fields = data.split(b"\n", 3)
    if fields[0] != b"P6":
        raise ValueError(f"{path}: not a binary PPM")
    width, height = map(int, fields[1].split())
    return width, height, fields[3]


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("exe")
    ap.add_argument("--frames", type=int, default=20)
    ap.add_argument("--out", default="renderer-compare")
    ap.add_argument("--tolerance", type=float, default=0.005,
                    help="max fraction of pixels allowed to differ (default 0.5%%)")
    args = ap.parse_args()

    renderers = subprocess.run([args.exe, "--list-renderers"], capture_output=True,
                               text=True, check=True).stdout.split()
    if len(renderers) < 2:
        print(f"only {renderers} available; nothing to compare")
        return 0

    os.makedirs(args.out, exist_ok=True)
    images = {}
    for name in renderers:
        shot = os.path.join(args.out, f"{name}.ppm")
        result = subprocess.run([args.exe, "--renderer", name, "--frames", str(args.frames),
                                 "--screenshot", shot], capture_output=True, text=True)
        print(result.stdout.strip())
        if result.returncode != 0:
            print(result.stderr, file=sys.stderr)
            return 1
        images[name] = read_ppm(shot)

    failed = False
    base_name = renderers[0]
    bw, bh, base = images[base_name]
    for name in renderers[1:]:
        w, h, other = images[name]
        if (w, h) != (bw, bh):
            print(f"{base_name} vs {name}: size mismatch {bw}x{bh} vs {w}x{h}")
            failed = True
            continue
        differing = sum(
            1 for i in range(0, len(base), 3)
            if max(abs(base[i] - other[i]), abs(base[i + 1] - other[i + 1]),
                   abs(base[i + 2] - other[i + 2])) > 8)
        fraction = differing / (w * h)
        status = "ok" if fraction <= args.tolerance else "MISMATCH"
        print(f"{base_name} vs {name}: {differing} of {w * h} pixels differ "
              f"({fraction:.3%}) {status}")
        failed |= fraction > args.tolerance
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
