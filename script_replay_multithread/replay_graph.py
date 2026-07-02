import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import os

import datetime
WORK_DIR = os.path.expanduser('~/NetworkProjectVanet/simulations/replay')
TIMESTAMP = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M')
OUT = os.path.join(WORK_DIR, f'graphs_{TIMESTAMP}')
os.makedirs(OUT, exist_ok=True)

plt.rcParams.update({
    'font.family': 'DejaVu Sans',
    'font.size': 11,
    'axes.titlesize': 13,
    'axes.labelsize': 11,
    'legend.fontsize': 10,
    'figure.dpi': 150,
    'axes.grid': True,
    'grid.alpha': 0.3,
    'lines.linewidth': 2.2,
    'lines.markersize': 7,
})

pcts = [8, 15, 30, 40, 50, 60]
COLORS  = {'S1': '#2E75B6', 'S2': '#C00000', 'S3': '#ED7D31', 'S4': '#70AD47'}
MARKERS = {'S1': 'o',       'S2': 's',       'S3': 'D',       'S4': 'P'}

# ── Dati aggiornati — 50 ripetizioni ─────────────────────────────────────────
data = {
    'Small': {
        'pdr': {
            'S1': 85.15,
            'S2': [87.31, 85.45, 83.68, 81.61, 79.59, 77.65],
            'S3': [85.95, 75.21, 76.90, 79.32, 76.22, 75.98],
            'S4': [85.95, 75.21, 76.90, 79.32, 76.22, 75.98],
        },
        'thr': {
            'S1': 2620.42,
            'S2': [2686.69, 2629.59, 2575.15, 2511.34, 2449.18, 2389.49],
            'S3': [2644.82, 2314.56, 2366.57, 2440.86, 2345.49, 2338.10],
            'S4': [2644.82, 2314.56, 2366.57, 2440.86, 2345.49, 2338.10],
        },
        'dly': {
            'S1': 116.674,
            'S2': [195.539, 124.465, 257.046, 240.420, 235.639, 215.239],
            'S3': [195.017, 353.348, 461.880, 370.506, 336.584, 451.476],
            'S4': [195.017, 353.348, 461.880, 370.506, 336.584, 451.476],
        },
        'accuracy': {
            'S3': [74.35, 84.91, 86.28, 87.61, 91.31, 93.59],
            'S4': [73.79, 84.78, 85.70, 87.59, 91.08, 92.47],
        },
        'fpr': {
            'S3': [35.42, 34.87, 32.33, 33.46, 29.73, 26.67],
            'S4': [36.39, 38.00, 36.13, 38.01, 36.21, 36.51],
        },
        'fnr': {
            'S3': [2.63, 1.48, 1.37, 1.05, 0.61, 0.41],
            'S4': [2.56, 1.26, 1.17, 0.82, 0.45, 0.30],
        },
        'mac': {
            'S1': 2268490,
            'S5': 2282825,
            'S6': 2282825,
        },
    },
    'Medium': {
        'pdr': {
            'S1': 94.70,
            'S2': [82.05, 77.86, 73.21, 69.83, 66.52, 66.82],
            'S3': [88.87, 85.45, 78.12, 71.41, 62.42, 52.63],
            'S4': [88.87, 85.45, 78.12, 71.41, 62.42, 52.63],
        },
        'thr': {
            'S1': 2172.29,
            'S2': [1876.46, 1764.63, 1685.41, 1604.86, 1523.99, 1529.74],
            'S3': [2032.08, 1938.71, 1797.15, 1642.28, 1431.48, 1204.77],
            'S4': [2032.08, 1938.71, 1797.15, 1642.28, 1431.48, 1204.77],
        },
        'dly': {
            'S1': 37.414,
            'S2': [213.030, 282.373, 295.546, 757.276, 606.603, 1241.884],
            'S3': [291.521, 828.903, 731.296, 514.136, 619.639, 762.032],
            'S4': [291.521, 828.903, 731.296, 514.136, 619.639, 762.032],
        },
        'accuracy': {
            'S3': [63.19, 77.92, 91.88, 94.62, 94.84, 95.75],
            'S4': [63.25, 77.92, 90.61, 93.02, 93.28, 94.11],
        },
        'fpr': {
            'S3': [53.94, 46.25, 29.33, 25.25, 28.79, 28.92],
            'S4': [54.86, 48.06, 35.09, 35.44, 40.87, 40.77],
        },
        'fnr': {
            'S3': [2.72, 1.49, 0.82, 0.41, 0.28, 0.19],
            'S4': [2.53, 1.35, 0.73, 0.33, 0.22, 0.16],
        },
        'mac': {
            'S1': 2524610,
            'S5': 2816722,
            'S6': 2816722,
        },
    },
    'Large': {
        'pdr': {
            'S1': 62.99,
            'S2': [62.68, 60.95, 58.10, 54.68, 53.44, 53.29],
            'S3': [46.40, 49.56, 44.33, 42.38, 39.66, 38.97],
            'S4': [46.40, 49.56, 44.33, 42.38, 39.66, 38.97],
        },
        'thr': {
            'S1': 2429.53,
            'S2': [2411.17, 2351.60, 2254.04, 2119.31, 2043.88, 2070.25],
            'S3': [1784.18, 1912.90, 1719.83, 1642.92, 1518.20, 1513.30],
            'S4': [1784.18, 1912.90, 1719.83, 1642.92, 1518.20, 1513.30],
        },
        'dly': {
            'S1': 460.033,
            'S2': [1140.890,  895.543, 1119.360, 1893.529, 1743.246, 1123.871],
            'S3': [1022.739, 1147.294, 1055.603, 1225.631, 1229.667, 1051.234],
            'S4': [1022.739, 1147.294, 1055.603, 1225.631, 1229.667, 1051.234],
        },
        'accuracy': {
            'S3': [94.15, 94.82, 97.04, 97.94, 98.07, 97.98],
            'S4': [93.67, 94.18, 95.24, 95.66, 95.14, 94.59],
        },
        'fpr': {
            'S3': [7.00,  6.49,  5.19,  4.98,  4.95,  5.41],
            'S4': [7.71,  7.44,  9.51, 12.00, 14.22, 16.53],
        },
        'fnr': {
            'S3': [2.46, 1.80, 0.76, 0.39, 0.28, 0.24],
            'S4': [2.38, 1.74, 0.64, 0.33, 0.23, 0.19],
        },
        'mac': {
            'S1': 3042952,
            'S5': 2659038,
            'S6': 2659038,
        },
    },
}

# ── Helpers ───────────────────────────────────────────────────────────────────

def save(fig, name):
    path = f'{OUT}/{name}.png'
    fig.savefig(path, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print(f'Salvato: {path}')


def plot_pdr(topo):
    d = data[topo]['pdr']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]}%)',
            color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2'], label='S2 Solo attacco',    color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S3'], label='S3 RSA + attacco',   color=COLORS['S3'], marker=MARKERS['S3'])
    ax.plot(pcts, d['S4'], label='S4 ECDSA + attacco', color=COLORS['S4'], marker=MARKERS['S4'],
            linestyle=':', linewidth=2)
    ax.set_title(f'PDR vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('PDR (%)')
    ax.set_xticks(pcts)
    ax.set_ylim(0, 105)
    ax.legend()
    save(fig, f'pdr_{topo.lower()}')


def plot_throughput(topo):
    d = data[topo]['thr']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]:.0f} B/s)',
            color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2'], label='S2 Solo attacco',    color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S3'], label='S3 RSA + attacco',   color=COLORS['S3'], marker=MARKERS['S3'])
    ax.plot(pcts, d['S4'], label='S4 ECDSA + attacco', color=COLORS['S4'], marker=MARKERS['S4'],
            linestyle=':', linewidth=2)
    ax.set_title(f'Throughput vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Throughput (byte/s)')
    ax.set_xticks(pcts)
    ax.legend()
    save(fig, f'throughput_{topo.lower()}')


def plot_delay(topo):
    d = data[topo]['dly']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]:.1f} ms)',
            color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2'], label='S2 Solo attacco',    color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S3'], label='S3 RSA + attacco',   color=COLORS['S3'], marker=MARKERS['S3'])
    ax.plot(pcts, d['S4'], label='S4 ECDSA + attacco', color=COLORS['S4'], marker=MARKERS['S4'],
            linestyle=':', linewidth=2)
    ax.set_title(f'End-to-End Delay vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Delay (ms)')
    ax.set_xticks(pcts)
    ax.legend()
    save(fig, f'delay_{topo.lower()}')


def plot_accuracy(topo):
    d = data[topo]['accuracy']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, d['S3'], label='S3 RSA',   color=COLORS['S3'], marker=MARKERS['S3'])
    ax.plot(pcts, d['S4'], label='S4 ECDSA', color=COLORS['S4'], marker=MARKERS['S4'])
    ax.set_title(f'Accuracy vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Accuracy (%)')
    ax.set_xticks(pcts)
    ax.set_ylim(0, 105)
    ax.legend()
    save(fig, f'accuracy_{topo.lower()}')


def plot_fpr_fnr(topo):
    d_fpr = data[topo]['fpr']
    d_fnr = data[topo]['fnr']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, d_fpr['S3'], label='FPR S3 RSA',
            color=COLORS['S3'], marker=MARKERS['S3'])
    ax.plot(pcts, d_fpr['S4'], label='FPR S4 ECDSA',
            color=COLORS['S4'], marker=MARKERS['S4'])
    ax.plot(pcts, d_fnr['S3'], label='FNR S3 RSA',
            color=COLORS['S3'], marker=MARKERS['S3'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d_fnr['S4'], label='FNR S4 ECDSA',
            color=COLORS['S4'], marker=MARKERS['S4'], linestyle='--', alpha=0.7)
    ax.set_title(f'FPR e FNR vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Tasso (%)')
    ax.set_xticks(pcts)
    # scala automatica per non tagliare valori alti (Medium ha FPR > 50%)
    max_val = max(max(d_fpr['S3']), max(d_fpr['S4']))
    ax.set_ylim(0, min(max_val * 1.15 + 2, 65))
    ax.legend()
    save(fig, f'fpr_fnr_{topo.lower()}')


def plot_overhead(topo):
    d = data[topo]['mac']
    labels = ['S1 Baseline', 'S5 RSA\nNo Attack', 'S6 ECDSA\nNo Attack']
    vals   = [d['S1'], d['S5'], d['S6']]
    colors_bar = [COLORS['S1'], COLORS['S3'], COLORS['S4']]
    fig, ax = plt.subplots(figsize=(7, 5))
    bars = ax.bar(labels, vals, color=colors_bar, width=0.5,
                  edgecolor='white', linewidth=1.5)
    for bar, val in zip(bars, vals):
        ax.text(bar.get_x() + bar.get_width() / 2,
                bar.get_height() + max(vals) * 0.01,
                f'{val:,.0f}', ha='center', va='bottom', fontsize=9)
    ax.set_title(f'Overhead MAC (senza attacco) — {topo}')
    ax.set_ylabel('Byte / run')
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{x:,.0f}'))
    save(fig, f'overhead_{topo.lower()}')


def plot_mac_attack(topo):
    """Overhead MAC al variare degli attaccanti (S1 vs S2 vs S3 vs S4)."""
    # Usiamo i dati MAC dei run con attacco dai file metriche
    mac_data = {
        'Small': {
            'S1': 2268490,
            'S2': [3228231, 4220213, 4605267, 5665148, 7398416, 7156132],
            'S3': [2539675, 6056165, 5806882, 4511103, 4675215, 5250851],
            'S4': [2539675, 6056165, 5806882, 4511103, 4675215, 5250851],
        },
        'Medium': {
            'S1': 2524610,
            'S2': [5165068, 7392592, 9880380, 10396549, 13107476, 13588694],
            'S3': [2828788, 3127326, 5825493,  9668397, 21929194, 28232979],
            'S4': [2828788, 3127326, 5825493,  9668397, 21929194, 28232979],
        },
        'Large': {
            'S1': 3042952,
            'S2': [5587850,  6693622,  9298891, 11920292, 12043849, 13439294],
            'S3': [2865395,  3043788,  3629504,  4661455,  5203138,  5447031],
            'S4': [2865395,  3043788,  3629504,  4661455,  5203138,  5447031],
        },
    }
    d = mac_data[topo]
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]:,} B)',
            color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2'], label='S2 Solo attacco',    color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S3'], label='S3 RSA + attacco',   color=COLORS['S3'], marker=MARKERS['S3'])
    ax.plot(pcts, d['S4'], label='S4 ECDSA + attacco', color=COLORS['S4'], marker=MARKERS['S4'],
            linestyle=':', linewidth=2)
    ax.set_title(f'Overhead MAC vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Byte MAC / run')
    ax.set_xticks(pcts)
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{x:,.0f}'))
    ax.legend()
    save(fig, f'mac_attack_{topo.lower()}')


# ── Genera tutti i grafici ────────────────────────────────────────────────────
for topo in ['Small', 'Medium', 'Large']:
    plot_pdr(topo)
    plot_throughput(topo)
    plot_delay(topo)
    plot_accuracy(topo)
    plot_fpr_fnr(topo)
    plot_overhead(topo)
    plot_mac_attack(topo)

print('Tutti i grafici generati in:', OUT)
