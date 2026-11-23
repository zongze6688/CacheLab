#!/usr/bin/env python3
#
# driver.py - Runs ./test-csim (Part A), ./test-trans on three matrix
#     sizes (Part B), ./test-mul (Part C) and ./test-solver (Bonus),
#     then prints the score out of 110. Run `make` first.
#
import argparse
import json
import re
import subprocess

INVALID = 2**31 - 1

# Part A: scale the raw test-csim points without changing the test weights.
CSIM_RAW_MAX = 27
CSIM_MAX = 35

# Part B (CMU thresholds): (name, M, N, full-credit misses, zero-credit misses, points)
TRANS_TESTS = [
    ("32x32", 32, 32, 300, 600, 12),
    ("64x64", 64, 64, 1300, 2000, 12),
    ("61x67", 61, 67, 2000, 3000, 16),
]

# Part C: keep the original full-credit cutoff; the naive version had 35,909 misses.
MUL_TESTS = [
    ("32x32", 32, 32, 3999, 35000, 25),
]

# Bonus (reverse-engineer a cache): three weighted parts, 10 points in all
SOLVER_GEOMETRY = 4     # b, s and E right, over the 8 caches
SOLVER_WA = 1           # write-allocate right (and geometry), over the 8 caches
SOLVER_POLICY = 5       # policy right (and geometry), over the 6 caches with E > 1


def miss_score(miss, lower, upper, full_score):
    """Full score at or below lower, zero at or above upper, linear between."""
    if miss <= lower:
        return full_score
    if miss >= upper:
        return 0
    return (1 - (miss - lower) / (upper - lower)) * full_score


def run(cmd):
    return subprocess.run(cmd, shell=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True).stdout


def parse_result(output, tag):
    """Return (correct, misses) from a TEST_*_RESULTS=c:m line."""
    m = re.search(tag + r"=(\d+):(\d+)", output)
    if not m:
        return 0, INVALID
    correct, misses = int(m.group(1)), int(m.group(2))
    return correct, (misses if correct else INVALID)


def solver_score(output):
    """Return (points, [geometry, wa, policy]) from TEST_SOLVER_RESULTS.

    The line is TEST_SOLVER_RESULTS=ok:geom:wa:policy:policy_total:runs.
    ok is 0 when the static checks of solver.c failed, which scores 0.
    """
    m = re.search(r"TEST_SOLVER_RESULTS=(\d+):(\d+):(\d+):(\d+):(\d+):(\d+)", output)
    if not m:
        return 0.0, [0, 0, 0]
    v = [int(x) for x in m.groups()]
    ok, geom, wa, pol, pol_total, nrun = v
    if not ok or nrun == 0:
        return 0.0, [0, 0, 0]
    pts = (SOLVER_GEOMETRY * geom / nrun + SOLVER_WA * wa / nrun +
           SOLVER_POLICY * pol / pol_total if pol_total else 0)
    return round(pts, 1), [geom, wa, pol]


def main():
    p = argparse.ArgumentParser()
    p.add_argument("-A", action="store_true", dest="autograde",
                   help="emit an autoresult JSON line")
    p.add_argument("-B", action="store_true", dest="bonus_only",
                   help="run only Bonus (no valgrind needed)")
    p.add_argument("-i", default="00000000000", help="11-digit student ID (Bonus)")
    p.add_argument("-K", default=None, help="Bonus key file (TAs only)")
    args = p.parse_args()

    rows = []
    misses_all = []

    if not args.bonus_only:
        print("Part A: Testing cache simulator")
        print("Running ./test-csim")
        out = run("./test-csim")
        m = re.search(r"TEST_CSIM_RESULTS=(\d+)", out)
        csim_raw_score = int(m.group(1)) if m else 0
        csim_score = csim_raw_score * CSIM_MAX / CSIM_RAW_MAX
        print("\n".join(l for l in out.splitlines()
                        if not l.startswith("TEST_CSIM_RESULTS")))

        rows.append(("Csim correctness", csim_score, CSIM_MAX, ""))

        print("Part B: Testing transpose function")
        for name, M, N, lo, hi, pts in TRANS_TESTS:
            print("Running ./test-trans -M %d -N %d" % (M, N))
            correct, miss = parse_result(run("./test-trans -M %d -N %d" % (M, N)),
                                         "TEST_TRANS_RESULTS")
            score = round(miss_score(miss, lo, hi, pts), 1) if correct else 0
            rows.append(("Trans perf " + name, score, pts, miss))
            misses_all.append(miss)

        print("Part C: Testing multiply function")
        for name, M, N, lo, hi, pts in MUL_TESTS:
            print("Running ./test-mul -M %d -N %d" % (M, N))
            correct, miss = parse_result(run("./test-mul -M %d -N %d" % (M, N)),
                                         "TEST_MUL_RESULTS")
            score = miss_score(miss, lo, hi, pts) if correct else 0
            rows.append(("Mul perf " + name, score, pts, miss))
            misses_all.append(miss)

    print("Bonus: Testing cache reverse-engineering")
    cmd = "./test-solver -q -i %s" % args.i + (" -K %s" % args.K if args.K else "")
    print("Running " + cmd)
    out = run(cmd)
    print("\n".join(l for l in out.splitlines()
                    if not l.startswith("TEST_SOLVER_RESULTS")))
    bonus, parts = solver_score(out)
    bonus_max = SOLVER_GEOMETRY + SOLVER_WA + SOLVER_POLICY
    rows.append(("Reverse-engineering", bonus, bonus_max, ""))
    total = round(sum(r[1] for r in rows), 3)

    if args.bonus_only:
        print("%-22s%8.1f%10d" % ("Reverse-engineering", bonus, bonus_max))
    else:
        print("\nCache Lab summary:")
        print("%-22s%8s%10s%12s" % ("", "Points", "Max pts", "Misses"))
        for label, score, pts, miss in rows:
            shown = "" if miss == "" else ("invalid" if miss == INVALID else str(miss))
            print("%-22s%8.3f%10d%12s" % (label, score, pts, shown))
        print("%22s%8.3f%10d" % ("Total points", total, sum(r[2] for r in rows)))

    if args.autograde:
        print(json.dumps({"scores": {"Autograded": total}, "id": args.i,
                          "scoreboard": [total, bonus] + misses_all + parts}))


if __name__ == "__main__":
    main()
