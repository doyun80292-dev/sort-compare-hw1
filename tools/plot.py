"""plot.py - 측정 결과(csv)랑 그래프(svg)를 report/ 에 만든다.

    make charts          # 또는
    python3 tools/plot.py

main.out --csv, --pivot, --theory, --big 출력을 읽어서 씀. 표준 모듈만 사용.
"""

import csv
import io
import math
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import svgchart  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "src" / "main.out"
OUT = ROOT / "report"
ALGOS = ["mergeSort", "quickSort", "heapSort"]
KINDS = ["random", "sorted", "reversed", "few-unique"]
KIND_LABEL = {"random": "무작위", "sorted": "정렬됨",
              "reversed": "역순", "few-unique": "중복많음"}
EULER_GAMMA = 0.5772156649015329


def run_csv(flag):
    if not BINARY.exists():
        subprocess.run(["make", "src/main.out"], cwd=ROOT, check=True)
    out = subprocess.run([str(BINARY), flag], cwd=ROOT, check=True,
                         capture_output=True, text=True).stdout
    rows = list(csv.DictReader(io.StringIO(out)))
    for r in rows:
        for k, v in r.items():
            try:
                r[k] = int(v)
            except ValueError:
                try:
                    r[k] = float(v)
                except ValueError:
                    pass
    return rows


def save(name, rows):
    with open(OUT / name, "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)


def short(n):
    return svgchart.si(n)


# --- 이론식 ---------------------------------------------------------------

def harmonic(n):
    # H_n 근사: ln n + 감마 + 1/(2n) - 1/(12n^2)
    return math.log(n) + EULER_GAMMA + 1 / (2 * n) - 1 / (12 * n * n)


def theory(algo, n):
    lg = math.log2(n)
    if algo == "mergeSort":
        return n * lg - 1.2645 * n           # n = 2^k일 때 평균 (위키피디아 Merge sort)
    if algo == "quickSort":
        return 2 * (n + 1) * harmonic(n) - 4 * n   # 파티션당 비교 m-1회일 때 평균
    return 2 * n * lg                        # 힙 정렬은 상한


def main():
    OUT.mkdir(exist_ok=True)
    made = []

    # 1. 입력 모양별 + n 두 배씩
    rows = run_csv("--csv")
    save("results.csv", rows)
    kinds = [r for r in rows if r["scope"] == "kinds"]
    growth = [r for r in rows if r["scope"] == "growth"]
    sizes = sorted({r["n"] for r in growth})

    def series(rs, field, keys, key_field):
        return {a: [next(r[field] for r in rs if r["algo"] == a and r[key_field] == k)
                    for k in keys] for a in ALGOS}

    made.append(svgchart.grouped_bar_chart(
        OUT / "input-shapes-time-log.svg",
        "입력 모양에 따른 걸린 시간 (로그 축)",
        "n = 100,000 · 3회 평균 · 눈금 한 칸이 10배다",
        [KIND_LABEL[k] for k in KINDS], series(kinds, "millis", KINDS, "input"),
        "시간 (ms)", log_scale=True, value_label=svgchart.ms))

    # 비교 횟수 / (n log2 n) = 앞에 붙는 상수
    norm = {a: [r / (n * math.log2(n)) for r, n in zip(vals, sizes)]
            for a, vals in series(growth, "compares", sizes, "n").items()}
    made.append(svgchart.line_chart(
        OUT / "growth-normalized.svg",
        "비교 횟수 ÷ (n log2 n) — 같은 O(n log n)의 서로 다른 상수",
        "무작위 입력 · x축만 로그 · 점선은 n → ∞일 때의 이론 상수",
        sizes, norm, "n (원소 개수)", "비교 / (n log2 n)", log_axes=False, log_x=True,
        x_label=short, y_label=lambda v: f"{v:.1f}",
        ref_lines=[(1.0, "병합 → 1", "#0072B2"),
                   (2 * math.log(2), "퀵 → 2 ln 2 ≈ 1.39", "#E69F00"),
                   (2.0, "힙 ≤ 2", "#009E73")]))

    made.append(svgchart.line_chart(
        OUT / "growth-time-log.svg",
        "n이 커질 때 걸린 시간 — 로그-로그 축",
        "무작위 입력 · 5회 평균 · 세 선이 평행하다 = 자라는 속도가 같다, 높이 차이 = 상수 차이",
        sizes, series(growth, "millis", sizes, "n"), "n (원소 개수)", "시간 (ms)",
        x_label=short, annotate_slope=False))

    # 2. 피벗 실험
    piv = run_csv("--pivot")
    save("pivot.csv", piv)
    pn = sorted({r["n"] for r in piv})
    label = {("sorted", "first"): "정렬됨·첫 원소(수업)",
             ("sorted", "random"): "정렬됨·무작위",
             ("few-unique", "random"): "중복많음·무작위"}
    pdata = {lab: [next(r["compares"] for r in piv if r["input"] == i
                        and r["pivot"] == p and r["n"] == n) for n in pn]
             for (i, p), lab in label.items()}
    made.append(svgchart.line_chart(
        OUT / "pivot-compares.svg",
        "퀵 정렬: 입력 · 피벗별 비교 횟수 (로그-로그)",
        "기울기 2 = O(n^2). 무작위 피벗은 정렬된 입력은 구하지만 중복은 못 구한다",
        pn, pdata, "n (원소 개수)", "비교 횟수", x_label=short))

    # 3. 이론값 비교 (시드 10개 평균)
    th = run_csv("--theory")
    for r in th:
        r["theory"] = round(theory(r["algo"], r["n"]), 1)
        r["errorPercent"] = round((r["meanCompares"] - r["theory"]) / r["theory"] * 100, 3)
        r["perNlgN"] = round(r["meanCompares"] / (r["n"] * math.log2(r["n"])), 4)
    save("theory.csv", th)

    # 4. 큰 n
    big = run_csv("--big")
    save("big.csv", big)

    for path in made:
        print(f"wrote {Path(path).relative_to(ROOT)}")
    for name in ("results.csv", "pivot.csv", "theory.csv", "big.csv"):
        print(f"wrote report/{name}")


if __name__ == "__main__":
    main()
