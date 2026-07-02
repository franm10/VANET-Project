#!/bin/bash
cd /home/giada/NetworkProjectVanet/simulations/grayhole

SCENARIOS=(
    "S1_NoAttack_Medium"
    "S2_Attack_Medium_8pct"
    "S2_Attack_Medium_15pct"
    "S2_Attack_Medium_30pct"
    "S2_Attack_Medium_40pct"
    "S2_Attack_Medium_50pct"
    "S2_Attack_Medium_60pct"
    "S3_NoAttack_Trust_Medium"
    "S4_Attack_Trust_Medium_8pct"
    "S4_Attack_Trust_Medium_15pct"
    "S4_Attack_Trust_Medium_30pct"
    "S4_Attack_Trust_Medium_40pct"
    "S4_Attack_Trust_Medium_50pct"
    "S4_Attack_Trust_Medium_60pct"
    "S5_NoAttack_Active_Medium"
    "S6_Attack_Active_Medium_8pct"
    "S6_Attack_Active_Medium_15pct"
    "S6_Attack_Active_Medium_30pct"
    "S6_Attack_Active_Medium_40pct"
    "S6_Attack_Active_Medium_50pct"
    "S6_Attack_Active_Medium_60pct"
)

SIM_TIME=290

# ============================================================
# PARTE 1 — LANCIO SIMULAZIONI
# ============================================================

for scenario in "${SCENARIOS[@]}"; do
    echo "=================================================="
    echo "Avvio: $scenario - $(date)"
    echo "=================================================="
    for ((r=0; r<5; r++)); do
        rm -f results/${scenario}-#${r}.sca
        opp_run -m -u Cmdenv \
          -n ..:../../src:../../../inet/src:../../../inet/examples:../../../inet/tutorials:../../../inet/showcases:../../../veins/examples/veins:../../../veins/src/veins \
          --image-path=../../../inet/images:../../../veins/images \
          -l ../../src/NetworkProjectVanet \
          -l ../../../inet/src/INET \
          -l ../../../veins/src/veins \
          -c $scenario -r $r omnet_medium.ini 2>&1 | tail -1
        echo "  ✓ Run #$r"
    done
    echo "✓ $scenario completato!"
done

echo ""
echo "Tutte le simulazioni completate! Calcolo metriche..."
echo ""

# ============================================================
# PARTE 2 — CALCOLO METRICHE
# ============================================================

METRICS_FILE="results/metriche_medium.txt"
echo "================================================================" > $METRICS_FILE
echo "METRICHE COMPLETE — Medium Topology — $(date)" >> $METRICS_FILE
echo "================================================================" >> $METRICS_FILE

for scenario in "${SCENARIOS[@]}"; do

    echo "" >> $METRICS_FILE
    echo "=== $scenario ===" >> $METRICS_FILE

    sum_sent=0; sum_recv=0
    sum_bytes_recv=0
    sum_delay=0; count_delay=0
    sum_bytes_mac=0
    sum_tp=0; sum_fp=0; sum_fn=0; sum_tn=0
    sum_tp_bl=0; sum_fp_bl=0; sum_fn_bl=0
    valid_runs=0

    for r in 0 1 2 3 4; do
        FILE="results/${scenario}-#${r}.sca"
        if [ ! -f "$FILE" ]; then continue; fi

        sent=$(grep "packetSent:count" $FILE | grep "app" | grep -v " 0$" | awk '{sum+=$NF} END {print sum}')
        recv=$(grep "packetReceived:count" $FILE | grep "app" | grep -v " 0$" | awk '{sum+=$NF} END {print sum}')
        if [ -z "$sent" ] || [ "$sent" = "0" ]; then continue; fi

        sum_sent=$((sum_sent + sent))
        sum_recv=$((sum_recv + recv))

        bytes=$(grep "packetReceived:sum(packetBytes)" $FILE | grep "app" | grep -v " 0$" | awk '{sum+=$NF} END {print sum+0}')
        sum_bytes_recv=$((sum_bytes_recv + bytes))

        delays=()
        while IFS= read -r val; do
            if [ -n "$val" ] && [ "$val" != "-nan" ] && [ "$val" != "nan" ]; then
                delays+=($val)
            fi
        done < <(grep -A 2 "endToEndDelay:histogram" "$FILE" | grep "field mean" | awk '{print $3}')
        if [ ${#delays[@]} -gt 0 ]; then
            delays_str=$(IFS=,; echo "${delays[*]}")
            delay=$(python3 -c "
vals = [${delays_str}]
print(f'{sum(vals)/len(vals)*1000:.3f}')
" 2>/dev/null)
            if [ -n "$delay" ] && [ "$delay" != "0.000" ]; then
                sum_delay=$(echo "$sum_delay + $delay" | bc)
                count_delay=$((count_delay + 1))
            fi
        fi

        mac=$(grep "sentDownPk:sum(packetBytes)" $FILE | grep "wlan" | awk '{sum+=$NF} END {print sum+0}')
        sum_bytes_mac=$((sum_bytes_mac + mac))

        tp=$(grep "detectionTP:count" $FILE | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
        fp=$(grep "detectionFP:count" $FILE | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
        fn=$(grep "detectionFN:count" $FILE | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
        tn=$(grep "detectionTN:count" $FILE | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
        sum_tp=$((sum_tp + tp))
        sum_fp=$((sum_fp + fp))
        sum_fn=$((sum_fn + fn))
        sum_tn=$((sum_tn + tn))

        tp_bl=$(grep "blacklistTP:count" $FILE | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
        fp_bl=$(grep "blacklistFP:count" $FILE | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
        fn_bl=$(grep "blacklistFN:count" $FILE | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
        sum_tp_bl=$((sum_tp_bl + tp_bl))
        sum_fp_bl=$((sum_fp_bl + fp_bl))
        sum_fn_bl=$((sum_fn_bl + fn_bl))

        valid_runs=$((valid_runs + 1))
    done

    if [ $valid_runs -eq 0 ]; then
        echo "  NESSUN FILE VALIDO" >> $METRICS_FILE
        continue
    fi

    # PDR e PER
    pdr=$(echo "scale=2; $sum_recv*100/$sum_sent" | bc)
    per=$(echo "scale=2; 100 - $pdr" | bc)
    echo "  [1] PDR=$pdr%  PER=$per%" >> $METRICS_FILE

    # Throughput
    throughput=$(echo "scale=2; $sum_bytes_recv / ($valid_runs * $SIM_TIME)" | bc)
    echo "  [2] Throughput=${throughput} byte/s" >> $METRICS_FILE

    # Delay
    if [ $count_delay -gt 0 ]; then
        avg_delay=$(echo "scale=3; $sum_delay / $count_delay" | bc)
        echo "  [3] End-to-end delay=${avg_delay} ms" >> $METRICS_FILE
    else
        echo "  [3] End-to-end delay=N/A" >> $METRICS_FILE
    fi

    # Overhead
    overhead=$(echo "scale=0; $sum_bytes_mac / $valid_runs" | bc)
    echo "  [4] Overhead MAC=${overhead} byte/run" >> $METRICS_FILE

    # Detection trust passiva
    det_total=$((sum_tp + sum_fp + sum_fn + sum_tn))
    if [ $det_total -gt 0 ]; then
        accuracy=$(echo "scale=2; ($sum_tp + $sum_tn)*100/$det_total" | bc)
        fpr_den=$((sum_fp + sum_tn))
        fnr_den=$((sum_fn + sum_tp))
        fpr=0; fnr=0
        if [ $fpr_den -gt 0 ]; then
            fpr=$(echo "scale=2; $sum_fp*100/$fpr_den" | bc)
        fi
        if [ $fnr_den -gt 0 ]; then
            fnr=$(echo "scale=2; $sum_fn*100/$fnr_den" | bc)
        fi
        echo "  [5a] Detection passiva: TP=$sum_tp FP=$sum_fp FN=$sum_fn TN=$sum_tn" >> $METRICS_FILE
        echo "  [5a] Accuracy=$accuracy%  FPR=$fpr%  FNR=$fnr%" >> $METRICS_FILE
    fi

    # Detection blacklist attiva
    bl_total=$((sum_tp_bl + sum_fp_bl + sum_fn_bl))
    if [ $bl_total -gt 0 ]; then
        prec_den=$((sum_tp_bl + sum_fp_bl))
        recall_den=$((sum_tp_bl + sum_fn_bl))
        prec=0; recall=0
        if [ $prec_den -gt 0 ]; then
            prec=$(echo "scale=2; $sum_tp_bl*100/$prec_den" | bc)
        fi
        if [ $recall_den -gt 0 ]; then
            recall=$(echo "scale=2; $sum_tp_bl*100/$recall_den" | bc)
        fi
        echo "  [5b] Detection attiva: TP=$sum_tp_bl FP=$sum_fp_bl FN=$sum_fn_bl" >> $METRICS_FILE
        echo "  [5b] Precision=$prec%  Recall=$recall%" >> $METRICS_FILE
    fi

done

echo "" >> $METRICS_FILE
echo "================================================================" >> $METRICS_FILE
echo "Fine — $(date)" >> $METRICS_FILE



# ============================================================
# OVERHEAD INTRODOTTO DALLA PROPOSTA
# ============================================================
echo "" >> $METRICS_FILE
echo "================================================================" >> $METRICS_FILE
echo "OVERHEAD INTRODOTTO DALLA PROPOSTA" >> $METRICS_FILE
echo "================================================================" >> $METRICS_FILE

# Calcola MAC bytes medi per scenario baseline e proposta
get_mac_avg() {
    local scenario=$1
    local total=0; local count=0
    for r in 0 1 2 3 4; do
        FILE="results/${scenario}-#${r}.sca"
        if [ ! -f "$FILE" ]; then continue; fi
        mac=$(grep "sentDownPk:sum(packetBytes)" $FILE | grep "wlan" | awk '{sum+=$NF} END {print sum+0}')
        if [ -n "$mac" ] && [ "$mac" != "0" ]; then
            total=$(echo "$total + $mac" | bc)
            count=$((count + 1))
        fi
    done
    if [ $count -gt 0 ]; then
        echo "scale=0; $total / $count" | bc
    else
        echo "0"
    fi
}

mac_s1=$(get_mac_avg "S1_NoAttack_Medium")
mac_s3=$(get_mac_avg "S3_NoAttack_Trust_Medium")
mac_s5=$(get_mac_avg "S5_NoAttack_Active_Medium")

overhead_trust=$(echo "scale=0; $mac_s3 - $mac_s1" | bc)
overhead_active=$(echo "scale=0; $mac_s5 - $mac_s1" | bc)

echo "" >> $METRICS_FILE
echo "  Baseline (S1) MAC bytes/run: $mac_s1" >> $METRICS_FILE
echo "  Trust passiva (S3) MAC bytes/run: $mac_s3" >> $METRICS_FILE
echo "  Mitigazione attiva (S5) MAC bytes/run: $mac_s5" >> $METRICS_FILE
echo "" >> $METRICS_FILE
echo "  Overhead trust passiva (S3-S1): ${overhead_trust} byte/run" >> $METRICS_FILE
echo "  Overhead mitigazione attiva (S5-S1): ${overhead_active} byte/run" >> $METRICS_FILE
echo "Metriche salvate in $METRICS_FILE"
cat $METRICS_FILE
