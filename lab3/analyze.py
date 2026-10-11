#!/usr/bin/env python3
"""results/*.csv-ээс лабораторийн 5 хүснэгт ба 2 графикийг үүсгэнэ.

Гаралт:
    results/tables.md       — Хүснэгт 1–5 (Markdown)
    results/speedup.png     — График 1
    results/efficiency.png  — График 2
"""
import csv
from collections import defaultdict
from pathlib import Path
from statistics import mean

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

RES = Path(__file__).resolve().parent / "results"
THREADS = [1, 2, 4, 8]
EXPECTED = 240_000_000.0


def load(path):
    times = defaultdict(list)   # (variant, threads) -> [time, ...]
    values = defaultdict(list)  # (variant, threads) -> [value, ...]
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            key = (row["variant"], int(row["threads"]))
            times[key].append(float(row["time"]))
            values[key].append(float(row["value"]))
    return times, values


def avg(times, variant, p):
    return mean(times[(variant, p)])


def main():
    times, values = load(RES / "results.csv")
    _, values_o0 = load(RES / "race_O0.csv")

    # Tseq-ийг OMP_NUM_THREADS=1 программын 3 ажиллуулалтаас авна. Олон урсгалтай
    # программд ажилгүй OpenMP урсгалууд spin хийж turbo давтамжийг бууруулдаг тул
    # тэнд хэмжсэн A1/B1-ийг хуваарьт ашиглахгүй.
    tseq_a = avg(times, "A1", 1)
    tseq_b = avg(times, "B1", 1)

    lines = []
    out = lines.append

    out("### Хүснэгт 1. Суурь хэмжилт — дараалсан хувилбарууд (Tseq)\n")
    out("| Дараалсан хувилбар | Хугацаа (сек) |")
    out("|---|---|")
    out(f"| A1 — Вектор нэмэх: c[i] = a[i] + b[i] | {tseq_a:.4f} |")
    out(f"| B1 — Нийлбэр олох: sum += c[i] | {tseq_b:.4f} |")
    out("")

    out("### Хүснэгт 2. Вектор нэмэх: параллель хувилбарууд\n")
    out("| Урсгалын тоо | Tprag (сек) | Tft (сек) | Tft-ийн параллель инициализаци (сек) |")
    out("|---|---|---|---|")
    for p in THREADS:
        out(f"| {p} | {avg(times, 'A2', p):.4f} | {avg(times, 'A3', p):.4f} "
            f"| {avg(times, 'INIT_PAR', p):.4f} |")
    out("")

    out("### Хүснэгт 3. Нийлбэр олох: параллель хувилбарууд\n")
    out("| Урсгалын тоо | Trace (сек) | Tcrit (сек) | Tred (сек) |")
    out("|---|---|---|---|")
    for p in THREADS:
        out(f"| {p} | {avg(times, 'B2', p):.4f} | {avg(times, 'B3', p):.4f} "
            f"| {avg(times, 'B4', p):.4f} |")
    out("")

    def race_table(title, vals):
        out(title + "\n")
        out("| Урсгалын тоо | Ажиллуулалт 1 | Ажиллуулалт 2 | Ажиллуулалт 3 | Зөв хариу |")
        out("|---|---|---|---|---|")
        for p in [2, 4, 8]:
            v = vals[("B2", p)]
            out(f"| {p} | " + " | ".join(f"{x:,.0f}" for x in v) + f" | {EXPECTED:,.0f} |")
        out("")

    race_table("### Хүснэгт 4. Race condition-ийн үр дүн (-O3, 3 удаа ажиллуулсан)", values)
    race_table("### Хүснэгт 4б. Race condition-ийн үр дүн (-O0, асуулт 6)", values_o0)

    s_crit = {p: tseq_b / avg(times, "B3", p) for p in THREADS}
    s_red = {p: tseq_b / avg(times, "B4", p) for p in THREADS}
    e_crit = {p: s_crit[p] / p for p in THREADS}
    e_red = {p: s_red[p] / p for p in THREADS}

    out("### Хүснэгт 5. Speedup ба Efficiency (Tseq = B1)\n")
    out("| Урсгалын тоо | S(crit) | E(crit) | S(red) | E(red) |")
    out("|---|---|---|---|---|")
    for p in THREADS:
        out(f"| {p} | {s_crit[p]:.4f} | {e_crit[p] * 100:.2f}% "
            f"| {s_red[p]:.2f} | {e_red[p] * 100:.1f}% |")
    out("")

    (RES / "tables.md").write_text("\n".join(lines), encoding="utf-8")
    print("\n".join(lines))

    # ---------------- График 1: Speedup ----------------
    fig, ax = plt.subplots(figsize=(7, 4.8))
    ax.plot(THREADS, THREADS, "k--", label="Төгс scaling (S = P)")
    ax.plot(THREADS, [s_red[p] for p in THREADS], "o-", color="tab:green",
            label="reduction (Tred)")
    ax.plot(THREADS, [s_crit[p] for p in THREADS], "s-", color="tab:red",
            label="critical (Tcrit)")
    for p in THREADS:
        ax.annotate(f"{s_red[p]:.2f}", (p, s_red[p]), textcoords="offset points",
                    xytext=(0, 8), ha="center", color="tab:green", fontsize=9)
        ax.annotate(f"{s_crit[p]:.3f}", (p, s_crit[p]), textcoords="offset points",
                    xytext=(0, -15), ha="center", color="tab:red", fontsize=9)
    ax.axhline(1.0, color="gray", lw=0.8, alpha=0.6)
    ax.set_yscale("log")
    ax.set_ylim(min(s_crit.values()) / 2.5, 15)
    ax.set_xticks(THREADS)
    ax.set_xlabel("Урсгалын тоо (P)")
    ax.set_ylabel("Speedup S(P) = Tseq / TP  (лог масштаб)")
    ax.set_title(f"График 1. Нийлбэр олох — Speedup (N = 80M, Tseq = {tseq_b:.4f} с)")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(loc="center right")
    fig.tight_layout()
    fig.savefig(RES / "speedup.png", dpi=150)

    # ---------------- График 2: Efficiency ----------------
    fig, ax = plt.subplots(figsize=(7, 4.8))
    ax.plot(THREADS, [100] * len(THREADS), "k--", label="Төгс (100%)")
    ax.plot(THREADS, [e_red[p] * 100 for p in THREADS], "o-", color="tab:green",
            label="reduction (Tred)")
    ax.plot(THREADS, [e_crit[p] * 100 for p in THREADS], "s-", color="tab:red",
            label="critical (Tcrit)")
    for p in THREADS:
        ax.annotate(f"{e_red[p] * 100:.1f}%", (p, e_red[p] * 100),
                    textcoords="offset points", xytext=(0, -16), ha="center",
                    color="tab:green", fontsize=9)
        ax.annotate(f"{e_crit[p] * 100:.2f}%", (p, e_crit[p] * 100),
                    textcoords="offset points", xytext=(0, 8), ha="center",
                    color="tab:red", fontsize=9)
    ax.set_xticks(THREADS)
    ax.set_ylim(-5, 120)
    ax.set_xlabel("Урсгалын тоо (P)")
    ax.set_ylabel("Efficiency E(P) = S(P) / P  (%)")
    ax.set_title("График 2. Нийлбэр олох — Efficiency")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="center left")
    fig.tight_layout()
    fig.savefig(RES / "efficiency.png", dpi=150)

    print(f"\nГрафикууд: {RES / 'speedup.png'}, {RES / 'efficiency.png'}")


if __name__ == "__main__":
    main()
