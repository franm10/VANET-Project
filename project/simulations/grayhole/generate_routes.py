#!/usr/bin/env python3
"""
Genera tutti i file .rou.xml e .launchd.xml per le simulazioni VANET grayhole.
Lancia con: python3 generate_routes.py
"""

import os

# ============================================================
# CONFIGURAZIONE TOPOLOGIE
# ============================================================

ROUTE0 = ("45849800#5 45849800#6 45849800#7 120516818#1 "
          "120516818#2 120516818#3 120516818#4 120516818#5 121823451 30924737#3 "
          "31143664 121823473 30924737#5 30924737#6 30924737#7 121823462 "
          "-30504149#8 -30504149#7 -30504149#6 -30504149#3 -30504149#2 "
          "-30504149#1 -30504149#0 30898807#0 30898807#1")

ROUTE1 = ("121823452 30924737#7 30924737#0 30924737#1 "
          "121823455 -120516818#5 -120516818#4 -120516818#3 -120516818#2 "
          "-120516818#1 -120516818#0 -45849800#6 -45849800#5 -45849800#4 "
          "-45849800#3 -45849800#2 -45849800#1")

# Topologie: nome -> (nodi_totali, period, nodi_per_ondata)
TOPOLOGIES = {
    "small":  {"total": 25,  "period": 2.0},
    "medium": {"total": 50,  "period": 1.0},
    "large":  {"total": 100, "period": 0.5},
}

# Percentuali attaccanti
PERCENTAGES = [0, 8, 15, 30, 40, 50, 60]

# ============================================================
# CALCOLO NODI PER ONDATA
# ============================================================

def compute_nodes(total, pct):
    """
    Calcola slow, normal, attacker per ondata (3 ondate).
    total = nodi totali nella simulazione
    pct   = percentuale attaccanti (0-60)
    """
    attackers_total = round(total * pct / 100)
    normals_total   = total - attackers_total

    # Dividi in 3 ondate il più uniformemente possibile
    att_per_wave = [attackers_total // 3] * 3
    nor_per_wave = [normals_total  // 3] * 3

    # Distribuisci il resto sulla prima ondata
    att_per_wave[0] += attackers_total % 3
    nor_per_wave[0] += normals_total   % 3

    # Dividi i nodi normali in slow e normal (metà e metà)
    waves = []
    for i in range(3):
        slow   = nor_per_wave[i] // 2
        normal = nor_per_wave[i] - slow
        attack = att_per_wave[i]
        waves.append((slow, normal, attack))

    return waves

# ============================================================
# GENERATORI XML
# ============================================================

def generate_rou_xml(topology_name, pct, waves, period):
    """Genera il contenuto del file .rou.xml"""

    lines = []
    lines.append('<?xml version="1.0" encoding="UTF-8"?>')
    lines.append('<routes xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"')
    lines.append('        xsi:noNamespaceSchemaLocation='
                 '"http://sumo.dlr.de/xsd/routes_file.xsd">')
    lines.append('')
    lines.append('    <vType id="normal_car" accel="2.8" decel="4.5" sigma="0.5"')
    lines.append('           length="4.5" minGap="8" maxSpeed="14" color="1,1,0"/>')
    lines.append('    <vType id="slow_car" accel="1.7" decel="3.5" sigma="0.5"')
    lines.append('           length="4.5" minGap="8" maxSpeed="7" color="0,0,1"/>')

    if pct > 0:
        lines.append('    <vType id="attacker_car" accel="2.4" decel="4.5" sigma="0.5"')
        lines.append('           length="4.5" minGap="8" maxSpeed="14" color="1,0,0"/>')

    lines.append('')
    lines.append(f'    <route id="route0" edges="{ROUTE0}"/>')
    lines.append(f'    <route id="route1" edges="{ROUTE1}"/>')
    lines.append('')

    begin_times = [0, 100, 200]

    for i, (begin, wave) in enumerate(zip(begin_times, waves)):
        slow, normal, attack = wave
        lines.append(f'    <!-- ONDATA {i+1} (t={begin}s) -->')

        if slow > 0:
            lines.append(
                f'    <flow id="slow_flow{i}" type="slow_car" route="route0" '
                f'begin="{begin}" period="{period}" number="{slow}" arrivalPos="-1"/>')

        if normal > 0:
            lines.append(
                f'    <flow id="normal_flow{i}" type="normal_car" route="route1" '
                f'begin="{begin}" period="{period}" number="{normal}" arrivalPos="-1"/>')

        if pct > 0 and attack > 0:
            att_begin = begin + 1
            lines.append(
                f'    <flow id="attacker_flow{i}" type="attacker_car" route="route1" '
                f'begin="{att_begin}" period="{period}" number="{attack}" arrivalPos="-1"/>')

        lines.append('')

    lines.append('</routes>')
    return '\n'.join(lines)


def generate_launchd_xml(rou_filename):
    """Genera il contenuto del file .launchd.xml"""
    return f"""<?xml version="1.0"?>
<launch>
    <copy file="arcavacatamap.net.xml"/>
    <copy file="{rou_filename}"/>
    <copy file="arcavacatamap.sumocfg" type="config"/>
</launch>
"""

# ============================================================
# MAIN
# ============================================================

def main():
    output_dir = "."  # cartella corrente (simulations/grayhole)

    generated_rou    = []
    generated_launch = []

    for topo_name, topo_cfg in TOPOLOGIES.items():
        total  = topo_cfg["total"]
        period = topo_cfg["period"]

        for pct in PERCENTAGES:

            # Nome file
            if pct == 0:
                suffix = "noattack"
            else:
                suffix = f"{pct}pct"

            rou_filename    = f"arcavacatamap_{topo_name}_{suffix}.rou.xml"
            launch_filename = f"arcavacatamap_{topo_name}_{suffix}.launchd.xml"

            # Calcola nodi
            waves = compute_nodes(total, pct)

            # Verifica totale
            total_check = sum(s+n+a for s,n,a in waves)
            print(f"[{topo_name:6s}] {pct:3d}%: "
                  f"waves={waves} → totale={total_check} "
                  f"(atteso={total})")

            # Genera .rou.xml
            rou_content = generate_rou_xml(topo_name, pct, waves, period)
            rou_path    = os.path.join(output_dir, rou_filename)
            with open(rou_path, 'w') as f:
                f.write(rou_content)
            generated_rou.append(rou_filename)

            # Genera .launchd.xml
            launch_content = generate_launchd_xml(rou_filename)
            launch_path    = os.path.join(output_dir, launch_filename)
            with open(launch_path, 'w') as f:
                f.write(launch_content)
            generated_launch.append(launch_filename)

    print(f"\n✓ Generati {len(generated_rou)} file .rou.xml")
    print(f"✓ Generati {len(generated_launch)} file .launchd.xml")
    print("\nFile .rou.xml generati:")
    for f in generated_rou:
        print(f"  {f}")

if __name__ == "__main__":
    main()

