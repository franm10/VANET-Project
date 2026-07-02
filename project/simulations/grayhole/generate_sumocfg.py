#!/usr/bin/env python3
"""
Genera un sumocfg per ogni launchd.xml esistente.
"""
import os
import glob

SUMOCFG_TEMPLATE = """<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <input>
        <net-file value="arcavacatamap.net.xml"/>
        <route-files value="{rou_file}"/>
    </input>
    <time>
        <begin value="0"/>
        <end value="320"/>
        <step-length value="0.1"/>
    </time>
    <processing>
        <lanechange.duration value="1.5"/>
        <ignore-route-errors value="true"/>
    </processing>
    <report>
        <xml-validation value="never"/>
        <xml-validation.net value="never"/>
        <no-step-log value="true"/>
    </report>
    <gui_only>
        <start value="true"/>
    </gui_only>
</configuration>
"""

LAUNCHD_TEMPLATE = """<?xml version="1.0"?>
<launch>
    <copy file="arcavacatamap.net.xml"/>
    <copy file="{rou_file}"/>
    <copy file="{sumocfg_file}" type="config"/>
</launch>
"""

folder = "/home/giada/NetworkProjectVanet/simulations/grayhole"
rou_files = glob.glob(os.path.join(folder, "arcavacatamap_*.rou.xml"))

for rou_path in rou_files:
    rou_file = os.path.basename(rou_path)
    base = rou_file.replace(".rou.xml", "")

    # Genera sumocfg
    sumocfg_file = f"{base}.sumocfg"
    sumocfg_path = os.path.join(folder, sumocfg_file)
    with open(sumocfg_path, 'w') as f:
        f.write(SUMOCFG_TEMPLATE.format(rou_file=rou_file))

    # Aggiorna launchd.xml
    launchd_file = f"{base}.launchd.xml"
    launchd_path = os.path.join(folder, launchd_file)
    with open(launchd_path, 'w') as f:
        f.write(LAUNCHD_TEMPLATE.format(
            rou_file=rou_file,
            sumocfg_file=sumocfg_file
        ))

    print(f"✓ {sumocfg_file}")

print(f"\nGenerati {len(rou_files)} sumocfg e launchd.xml")
