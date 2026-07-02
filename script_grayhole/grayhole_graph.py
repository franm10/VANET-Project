import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import os
import datetime

# ── Setup Cartelle Dinamiche ──────────────────────────────────────────────────
WORK_DIR = os.path.expanduser('~/NetworkProjectVanet/simulations/grayhole')
TIMESTAMP = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M')
OUT = os.path.join(WORK_DIR, f'graphs_grayhole_finale_{TIMESTAMP}')
os.makedirs(OUT, exist_ok=True)

# ── Stile Globale Accademico ─────────────────────────────────────────────────
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
COLORS  = {'S1': '#2E75B6', 'S2': '#C00000', 'S4': '#ED7D31', 'S6': '#70AD47'}
MARKERS = {'S1': 'o',       'S2': 's',       'S4': 'D',       'S6': 'P'}

# ── Database Ultimi Dati Definitivi (Fix FNR) ────────────────────────────────
data = {
    'Small': {
        'pdr': {
            'S1': 89.06,
            'S2': [62.81, 52.42, 49.92, 44.90, 45.33, 45.58],
            'S4': [59.72, 49.74, 47.61, 44.65, 45.60, 46.30],
            'S6': [59.79, 51.72, 21.14, 46.19, 47.83, 47.53],
        },
        'thr': {
            'S1': 2740.85,
            'S2': [1932.92, 1613.32, 1536.21, 1381.94, 1395.00, 1402.88],
            'S4': [1837.93, 1530.80, 1465.30, 1374.20, 1403.55, 1424.87],
            'S6': [1840.09, 1591.61,  650.66, 1421.66, 1471.98, 1462.78],
        },
        'dly': {
            'S1': 46.943,
            'S2': [37.998, 65.816, 55.901, 260.429, 348.830, 270.607],
            'S4': [119.815, 194.361, 140.076, 190.371, 325.603, 256.550],
            'S6': [300.345, 1040.766, 650.477, 491.799, 389.563, 403.462],
        },
        'accuracy': {
            'S4': [85.01, 72.43, 55.35, 39.52, 25.70, 17.70],
            'S6': [83.24, 60.07, 40.94, 37.39, 27.59, 21.04],
        },
        'fpr': {
            'S4': [3.60, 3.64, 4.98, 7.69, 16.93, 41.30],
            'S6': [6.96, 12.42, 30.30, 14.05, 22.04, 54.78],
        },
        'fnr': {
            'S4': [75.70, 78.79, 85.60, 86.64, 85.85, 85.49],
            'S6': [64.91, 80.74, 74.36, 82.71, 81.63, 80.71],
        },
        'prec': {
            'S6': [58.73, 51.23, 61.36, 80.85, 88.76, 87.76],
        },
        'rec': {
            'S6': [27.43, 19.49, 27.95, 15.26, 17.91, 20.13],
        },
        'mac': {
            'S1': 1745220,
            'S3': 1738729,
            'S5': 1739287,
            'S2_attack': [1540224, 1459861, 1486686, 1528200, 1470137, 1478720],
            'S4_attack': [1418181, 1437879, 1387717, 1449622, 1403560, 1389516],
            'S6_attack': [1644109, 1296066, 1453237, 1658941, 1537810, 1504596],
        }
    },
    'Medium': {
        'pdr': {
            'S1': 95.14,
            'S2': [52.06, 39.05, 31.00, 29.32, 24.86, 27.29],
            'S4': [47.34, 36.01, 29.13, 25.30, 23.16, 23.35],
            'S6': [56.41, 39.75,  9.01,  7.27, 25.63, 23.46],
        },
        'thr': {
            'S1': 2190.44,
            'S2': [1194.76,  889.14,  711.71,  675.48,  571.00,  628.45],
            'S4': [1086.32,  819.83,  668.98,  582.90,  532.02,  537.60],
            'S6': [1294.54,  905.03,  206.98,  167.65,  588.69,  540.14],
        },
        'dly': {
            'S1': 60.102,
            'S2': [21.294, 43.012, 65.280, 100.746, 154.379, 86.425],
            'S4': [14.142, 21.588, 103.189, 146.457, 158.546, 111.528],
            'S6': [19.136, 23.736, 585.994, 490.350, 87.813, 118.488],
        },
        'accuracy': {
            'S4': [84.40, 69.68, 44.58, 31.19, 19.91, 11.98],
            'S6': [84.39, 69.97, 24.48, 10.07, 20.00, 12.55],
        },
        'fpr': {
            'S4': [0.93, 1.01, 3.35, 5.69, 10.19, 21.58],
            'S6': [1.03, 1.26, 13.71, 94.50, 8.99, 22.81],
        },
        'fnr': {
            'S4': [92.67, 92.96, 93.72, 93.64, 92.60, 92.89],
            'S6': [91.79, 91.29, 81.15, 89.87, 92.91, 92.10],
        },
        'prec': {
            'S6': [66.29, 81.18, 95.13, 88.76, 89.83, 90.09],
        },
        'rec': {
            'S6': [9.23, 12.90, 23.34, 12.59, 8.71, 9.41],
        },
        'mac': {
            'S1': 1992295,
            'S3': 1991773,
            'S5': 2030008,
            'S2_attack': [1741599, 2017670, 7831508, 9809922, 9444691, 6280675],
            'S4_attack': [1692253, 2220151, 9212687, 10543419, 8532335, 7104539],
            'S6_attack': [1886012, 2293608, 22291323, 21699961, 7111008, 7470589],
        }
    },
    'Large': {
        'pdr': {
            'S1': 89.50,
            'S2': [49.12, 45.28, 40.40, 37.26, 33.16, 34.19],
            'S4': [52.89, 44.21, 40.22, 38.85, 32.86, 34.92],
            'S6': [51.54, 46.41, 36.90, 27.03, 20.86, 24.57],
        },
        'thr': {
            'S1': 3478.38,
            'S2': [1894.32, 1755.73, 1570.21, 1448.14, 1265.32, 1331.05],
            'S4': [2039.73, 1714.38, 1563.15, 1509.83, 1254.36, 1359.66],
            'S6': [1987.44, 1799.59, 1433.88, 1050.62,  796.78,  956.66],
        },
        'dly': {
            'S1': 227.053,
            'S2': [131.473, 132.213, 109.189, 123.578, 176.434, 167.501],
            'S4': [246.075, 159.382, 172.940, 136.027, 201.304, 166.135],
            'S6': [213.651, 146.207, 271.976, 412.713, 423.894, 306.679],
        },
        'accuracy': {
            'S4': [78.23, 65.79, 39.70, 22.00, 12.51, 10.24],
            'S6': [78.12, 63.33, 23.30, 23.80, 13.94, 16.33],
        },
        'fpr': {
            'S4': [3.06, 3.07, 6.56, 16.32, 56.25, 99.93],
            'S6': [3.05, 3.82, 21.06, 27.73, 88.54, 100.00],
        },
        'fnr': {
            'S4': [84.34, 88.30, 89.67, 90.55, 89.35, 89.44],
            'S6': [84.65, 87.93, 86.00, 84.87, 85.97, 83.02],
        },
        'prec': {
            'S6': [64.38, 70.84, 81.92, 74.48, 81.28, 82.89],
        },
        'rec': {
            'S6': [21.02, 16.37, 15.96, 14.66, 16.55, 16.87],
        },
        'mac': {
            'S1': 3278256,
            'S3': 3304210,
            'S5': 3345200,
            'S2_attack': [3007081, 3152980, 4086237, 4795553, 6518515, 5808458],
            'S4_attack': [3594572, 3426736, 4542690, 5019900, 5881229, 5337719],
            'S6_attack': [3679518, 3374129, 5047829, 5859518, 6451896, 6341148],
        }
    }
}

# ── Helper Funzioni Grafiche ─────────────────────────────────────────────────

def save(fig, name):
    path = f'{OUT}/{name}.png'
    fig.savefig(path, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print(f'Salvato: {path}')

def plot_pdr(topo):
    d = data[topo]['pdr']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]}%)', color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2'], label='S2 Solo Attacco', color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S4'], label='S4 Trust Passiva', color=COLORS['S4'], marker=MARKERS['S4'])
    ax.plot(pcts, d['S6'], label='S6 Mitigazione Attiva', color=COLORS['S6'], marker=MARKERS['S6'], linewidth=2.5)
    ax.set_title(f'Packet Delivery Ratio vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('PDR (%)')
    ax.set_xticks(pcts)
    ax.set_ylim(0, 105)
    ax.legend()
    save(fig, f'pdr_{topo.lower()}')

def plot_throughput(topo):
    d = data[topo]['thr']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]:.0f} B/s)', color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2'], label='S2 Solo Attacco', color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S4'], label='S4 Trust Passiva', color=COLORS['S4'], marker=MARKERS['S4'])
    ax.plot(pcts, d['S6'], label='S6 Mitigazione Attiva', color=COLORS['S6'], marker=MARKERS['S6'], linewidth=2.5)
    ax.set_title(f'Throughput vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Throughput (byte/s)')
    ax.set_xticks(pcts)
    ax.legend()
    save(fig, f'throughput_{topo.lower()}')

def plot_delay(topo):
    d = data[topo]['dly']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]:.1f} ms)', color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2'], label='S2 Solo Attacco', color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S4'], label='S4 Trust Passiva', color=COLORS['S4'], marker=MARKERS['S4'])
    ax.plot(pcts, d['S6'], label='S6 Mitigazione Attiva', color=COLORS['S6'], marker=MARKERS['S6'], linewidth=2.5)
    ax.set_title(f'End-to-End Delay vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Delay (ms)')
    ax.set_xticks(pcts)
    ax.legend()
    save(fig, f'delay_{topo.lower()}')

def plot_precision_recall(topo):
    prec = data[topo]['prec']['S6']
    rec = data[topo]['rec']['S6']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, prec, label='S6 Precision (Attiva)', color='#8E44AD', marker='*', linewidth=2)
    ax.plot(pcts, rec, label='S6 Recall (Attiva)', color='#E67E22', marker='x', linewidth=2, linestyle='--')
    ax.set_title(f'Precision & Recall — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Tasso (%)')
    ax.set_xticks(pcts)
    ax.set_ylim(0, 105)
    ax.legend()
    save(fig, f'precision_recall_{topo.lower()}')

def plot_fpr_fnr(topo):
    d_fpr = data[topo]['fpr']
    d_fnr = data[topo]['fnr']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, d_fpr['S4'], label='S4 FPR (Passiva)', color=COLORS['S4'], marker=MARKERS['S4'])
    ax.plot(pcts, d_fpr['S6'], label='S6 FPR (Attiva)', color=COLORS['S6'], marker=MARKERS['S6'])
    ax.plot(pcts, d_fnr['S4'], label='S4 FNR (Passiva)', color=COLORS['S4'], marker=MARKERS['S4'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d_fnr['S6'], label='S6 FNR (Attiva)', color=COLORS['S6'], marker=MARKERS['S6'], linestyle='--', alpha=0.7)
    ax.set_title(f'Tassi di Errore (FPR / FNR) vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Tasso (%)')
    ax.set_xticks(pcts)
    ax.set_ylim(0, 105)
    ax.legend(loc='lower right')
    save(fig, f'fpr_fnr_{topo.lower()}')

def plot_mac_attack(topo):
    d = data[topo]['mac']
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(pcts, [d['S1']] * 6, label=f'S1 Baseline ({d["S1"]:,} B)', color=COLORS['S1'], marker=MARKERS['S1'], linestyle='--', alpha=0.7)
    ax.plot(pcts, d['S2_attack'], label='S2 Solo Attacco', color=COLORS['S2'], marker=MARKERS['S2'])
    ax.plot(pcts, d['S4_attack'], label='S4 Trust Passiva', color=COLORS['S4'], marker=MARKERS['S4'])
    ax.plot(pcts, d['S6_attack'], label='S6 Mitigazione Attiva', color=COLORS['S6'], marker=MARKERS['S6'], linewidth=2.5)
    ax.set_title(f'Overhead MAC Totale vs % Attaccanti — {topo}')
    ax.set_xlabel('% Attaccanti')
    ax.set_ylabel('Byte MAC / run')
    ax.set_xticks(pcts)
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{x:,.0f}'))
    ax.legend()
    save(fig, f'mac_attack_{topo.lower()}')

def plot_overhead_clean(topo):
    d = data[topo]['mac']
    labels = ['S1 Baseline', 'S3 Passiva\n(No Attack)', 'S5 Attiva\n(No Attack)']
    vals   = [d['S1'], d['S3'], d['S5']]
    colors_bar = [COLORS['S1'], COLORS['S4'], COLORS['S6']]
    fig, ax = plt.subplots(figsize=(7, 5))
    bars = ax.bar(labels, vals, color=colors_bar, width=0.45, edgecolor='white', linewidth=1.5)
    for bar, val in zip(bars, vals):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + max(vals) * 0.01, f'{val:,.0f}', ha='center', va='bottom', fontsize=9)
    ax.set_title(f'Analisi dell\'Overhead MAC Intrinseco — {topo}')
    ax.set_ylabel('Byte / run')
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{x:,.0f}'))
    save(fig, f'overhead_clean_{topo.lower()}')

# ── Esecuzione ───────────────────────────────────────────────────────────────
for topo in ['Small', 'Medium', 'Large']:
    print(f'Generazione grafici per topologia: {topo}...')
    plot_pdr(topo)
    plot_throughput(topo)
    plot_delay(topo)
    plot_precision_recall(topo)
    plot_fpr_fnr(topo)
    plot_mac_attack(topo)
    plot_overhead_clean(topo)

print(f'\n [COMPLETATO] Grafici salvati in: {OUT}')
