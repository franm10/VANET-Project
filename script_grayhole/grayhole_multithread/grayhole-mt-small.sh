#!/bin/bash

# ============================================================
# 1. IMPOSTAZIONE PERCORSI E VERIFICA AMBIENTE
# ============================================================
WORK_DIR="$HOME/NetworkProjectVanet/simulations/grayhole"
cd "$WORK_DIR" || { echo "Errore: cartella $WORK_DIR non trovata!"; exit 1; }

# ============================================================
# 2. CONTROLLO MANUALE DI SUMO (SEMAFORO)
# ============================================================
echo "Controllo stato di SUMO (veins_launchd)..."

if ! pgrep -f veins_launchd > /dev/null; then
    echo "--------------------------------------------------------"
    echo "❌ ERRORE: SUMO (veins_launchd) NON è in esecuzione!"
    echo "Devi avviare il demone prima di lanciare le simulazioni."
    echo ""
    echo "👉 Comando per AVVIARE SUMO (eseguilo in un altro terminale):"
    echo "   python3 $HOME/veins/bin/veins_launchd -v"
    echo ""
    echo "👉 Comando per FERMARE SUMO (quando hai finito di lavorare):"
    echo "   pkill -f veins_launchd"
    echo "--------------------------------------------------------"
    echo "Interruzione dello script. Nessuna simulazione avviata."
    exit 1
fi

echo "✅ SUMO rilevato in esecuzione. Procedo con la preparazione..."

# ============================================================
# 3. CREAZIONE CARTELLE RISULTATI DINAMICHE
# ============================================================
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M")
RESULTS_DIR="$WORK_DIR/results_parallel_${TIMESTAMP}_small"
SCA_DIR="$RESULTS_DIR/simulations_sca"
LOG_DIR="$RESULTS_DIR/simulations_log"

# Crea l'albero delle directory
mkdir -p "$SCA_DIR" "$LOG_DIR"
cp omnet_small.ini "$RESULTS_DIR/omnet_small.ini"

# ============================================================
# 4. CONFIGURAZIONE SIMULAZIONE E TOLLERANZA ERRORI
# ============================================================
REPEAT=50                 # numero di ripetizioni per scenario
MAX_JOBS=10               # thread paralleli massimi
SIM_TIME=290
MAX_ERRORS_PER_GROUP=20   # Errori tollerati per ogni gruppo

SCENARIOS=(
    "S1_NoAttack_Small_8pct"
    "S1_NoAttack_Small_15pct"
    "S1_NoAttack_Small_30pct"
    "S1_NoAttack_Small_40pct"
    "S1_NoAttack_Small_50pct"
    "S1_NoAttack_Small_60pct"
    "S2_Attack_Small_8pct"
    "S2_Attack_Small_15pct"
    "S2_Attack_Small_30pct"
    "S2_Attack_Small_40pct"
    "S2_Attack_Small_50pct"
    "S2_Attack_Small_60pct"
    "S3_NoAttack_Trust_Small_8pct"
    "S3_NoAttack_Trust_Small_15pct"
    "S3_NoAttack_Trust_Small_30pct"
    "S3_NoAttack_Trust_Small_40pct"
    "S3_NoAttack_Trust_Small_50pct"
    "S3_NoAttack_Trust_Small_60pct"
    "S4_Attack_Trust_Small_8pct"
    "S4_Attack_Trust_Small_15pct"
    "S4_Attack_Trust_Small_30pct"
    "S4_Attack_Trust_Small_40pct"
    "S4_Attack_Trust_Small_50pct"
    "S4_Attack_Trust_Small_60pct"

)

OPP_ARGS=(
    -m -u Cmdenv
    -n ..:../../src:../../../inet/src:../../../inet/examples:../../../inet/tutorials:../../../inet/showcases:../../../veins/examples/veins:../../../veins/src/veins
    --image-path=../../../inet/images:../../../veins/images
    -l ../../src/NetworkProjectVanet
    -l ../../../inet/src/INET
    -l ../../../veins/src/veins
    --cmdenv-log-level=warn
    --result-dir="$SCA_DIR"
)

# ============================================================
# PARTE 1 — LANCIO SIMULAZIONI IN PARALLELO
# ============================================================

run_one() {
    local scenario=$1
    local r=$2
    local group=$(echo "$scenario" | cut -d'_' -f1)
    
    if [ -f "$RESULTS_DIR/.sim_error" ]; then return 1; fi

    rm -f "$SCA_DIR/${scenario}-#${r}.sca"
    
    opp_run "${OPP_ARGS[@]}" -c "$scenario" -r "$r" omnet_small.ini \
        > "$LOG_DIR/${scenario}-#${r}.log" 2>&1
    
    if [ $? -ne 0 ]; then
        touch "$RESULTS_DIR/.err_${group}_${scenario}_${r}"
        local err_count=$(find "$RESULTS_DIR" -maxdepth 1 -name ".err_${group}_*" 2>/dev/null | wc -l)
        
        if [ "$err_count" -gt "$MAX_ERRORS_PER_GROUP" ]; then
            echo "  ❌ ERRORE CRITICO: Superato il limite di $MAX_ERRORS_PER_GROUP errori nel gruppo $group! Blocco tutto."
            touch "$RESULTS_DIR/.sim_error"
        else
            echo "  ⚠️ ATTENZIONE: $scenario run #$r fallita. (Errore $err_count/$MAX_ERRORS_PER_GROUP tollerato per $group)"
        fi
    else
        echo "  ✓ $scenario run #$r completata"
    fi
}

export -f run_one
export OPP_ARGS
export RESULTS_DIR
export SCA_DIR
export LOG_DIR
export MAX_ERRORS_PER_GROUP

echo "========================================================"
echo "Lancio simulazioni GRAYHOLE con max $MAX_JOBS thread."
echo "Tolleranza errori: max $MAX_ERRORS_PER_GROUP per gruppo."
echo "Cartella Principale: $RESULTS_DIR"
echo "  -> Metriche e flag salvati qui."
echo "  -> File SCA salvati in: simulations_sca/"
echo "  -> File LOG salvati in: simulations_log/"
echo "Inizio: $(date)"
echo "========================================================"
echo ""

job_count=0
current_group=""

for scenario in "${SCENARIOS[@]}"; do
    
    if [ -f "$RESULTS_DIR/.sim_error" ]; then break; fi

    group=$(echo "$scenario" | cut -d'_' -f1)

    if [[ -n "$current_group" && "$current_group" != "$group" ]]; then
        echo "--------------------------------------------------------"
        echo "⏳ Attendo il completamento del gruppo $current_group..."
        wait
        if [ -f "$RESULTS_DIR/.sim_error" ]; then break; fi
        job_count=0
        echo "✅ Gruppo $current_group completato! Inizio gruppo $group..."
        echo "--------------------------------------------------------"
    fi
    current_group="$group"

    for ((r=0; r<REPEAT; r++)); do
        if [ -f "$RESULTS_DIR/.sim_error" ]; then break 2; fi

        run_one "$scenario" "$r" &
        job_count=$(( job_count + 1 ))
        
        if (( job_count >= MAX_JOBS )); then
            wait -n 2>/dev/null || wait
            job_count=$(( job_count - 1 ))
        fi
    done
done

wait
echo "--------------------------------------------------------"

# ============================================================
# PARTE 2 — CALCOLO METRICHE (SPECIFICHE GRAYHOLE)
# ============================================================

if [ -f "$RESULTS_DIR/.sim_error" ]; then
    echo "🚨 SIMULAZIONI INTERROTTE."
    echo "È stato superato il limite massimo di $MAX_ERRORS_PER_GROUP errori per un singolo gruppo."
else
    echo "Tutte le simulazioni completate! — $(date)"
    echo "Calcolo metriche Grayhole in corso..."
    echo ""

    METRICS_FILE="$RESULTS_DIR/metriche_grayhole_small.txt"

    echo "================================================================" > "$METRICS_FILE"
    echo "METRICHE COMPLETE — Grayhole Attack — Small Topology" >> "$METRICS_FILE"
    echo "  Data e Ora: $(date)" >> "$METRICS_FILE"
    echo "  Ripetizioni per scenario richieste: $REPEAT" >> "$METRICS_FILE"
    echo "================================================================" >> "$METRICS_FILE"

    for scenario in "${SCENARIOS[@]}"; do
        echo "" >> "$METRICS_FILE"
        echo "=== $scenario ===" >> "$METRICS_FILE"

        sum_sent=0; sum_recv=0; sum_bytes_recv=0
        sum_delay=0; count_delay=0; sum_bytes_mac=0
        sum_tp=0; sum_fp=0; sum_fn=0; sum_tn=0
        sum_tp_bl=0; sum_fp_bl=0; sum_fn_bl=0
        valid_runs=0

        for ((r=0; r<REPEAT; r++)); do
            FILE="$SCA_DIR/${scenario}-#${r}.sca"
            
            if [ ! -f "$FILE" ]; then continue; fi

            sent=$(grep "packetSent:count" "$FILE" | grep "app" | grep -v " 0$" | awk '{sum+=$NF} END {print sum}')
            recv=$(grep "packetReceived:count" "$FILE" | grep "app" | grep -v " 0$" | awk '{sum+=$NF} END {print sum}')
            if [ -z "$sent" ] || [ "$sent" = "0" ]; then continue; fi

            sum_sent=$((sum_sent + sent))
            sum_recv=$((sum_recv + recv))

            bytes=$(grep "packetReceived:sum(packetBytes)" "$FILE" | grep "app" | grep -v " 0$" | awk '{sum+=$NF} END {print sum+0}')
            sum_bytes_recv=$((sum_bytes_recv + bytes))

            delays=()
            while IFS= read -r val; do
                if [ -n "$val" ] && [ "$val" != "-nan" ] && [ "$val" != "nan" ]; then
                    delays+=($val)
                fi
            done < <(grep -A 2 "endToEndDelay:histogram" "$FILE" | grep "field mean" | awk '{print $3}')
            
            if [ ${#delays[@]} -gt 0 ]; then
                delays_str=$(IFS=,; echo "${delays[*]}")
                delay=$(python3 -c "vals = [${delays_str}]; print(f'{sum(vals)/len(vals)*1000:.3f}')" 2>/dev/null)
                if [ -n "$delay" ] && [ "$delay" != "0.000" ]; then
                    sum_delay=$(echo "$sum_delay + $delay" | bc)
                    count_delay=$((count_delay + 1))
                fi
            fi

            mac=$(grep "sentDownPk:sum(packetBytes)" "$FILE" | grep "wlan" | awk '{sum+=$NF} END {print sum+0}')
            sum_bytes_mac=$((sum_bytes_mac + mac))

            # Detection Passiva
            tp=$(grep "detectionTP:sum" "$FILE" | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
            fp=$(grep "detectionFP:sum" "$FILE" | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
            fn=$(grep "detectionFN:sum" "$FILE" | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
            tn=$(grep "detectionTN:sum" "$FILE" | grep "normal_car" | awk '{sum+=$NF} END {print sum+0}')
            sum_tp=$((sum_tp + tp)); sum_fp=$((sum_fp + fp))
            sum_fn=$((sum_fn + fn)); sum_tn=$((sum_tn + tn))

            # Detection Attiva (Blacklist)
            tp_bl=$(grep "blacklistTP:sum" "$FILE" | awk '{sum+=$NF} END {print sum+0}')
            fp_bl=$(grep "blacklistFP:sum" "$FILE" | awk '{sum+=$NF} END {print sum+0}')
            fn_bl=$(grep "blacklistFN:sum" "$FILE" | awk '{sum+=$NF} END {print sum+0}')
            sum_tp_bl=$((sum_tp_bl + tp_bl))
            sum_fp_bl=$((sum_fp_bl + fp_bl))
            sum_fn_bl=$((sum_fn_bl + fn_bl))

            valid_runs=$((valid_runs + 1))
        done

        if [ $valid_runs -eq 0 ]; then
            echo "  NESSUN FILE VALIDO" >> "$METRICS_FILE"
            continue
        fi

        echo "  Run valide analizzate: $valid_runs/$REPEAT" >> "$METRICS_FILE"
        pdr=$(echo "scale=2; $sum_recv*100/$sum_sent" | bc)
        per=$(echo "scale=2; 100 - $pdr" | bc)
        echo "  [1] PDR=$pdr%  PER=$per%" >> "$METRICS_FILE"

        throughput=$(echo "scale=2; $sum_bytes_recv / ($valid_runs * $SIM_TIME)" | bc)
        echo "  [2] Throughput=${throughput} byte/s" >> "$METRICS_FILE"

        if [ $count_delay -gt 0 ]; then
            avg_delay=$(echo "scale=3; $sum_delay / $count_delay" | bc)
            echo "  [3] End-to-end delay=${avg_delay} ms" >> "$METRICS_FILE"
        else
            echo "  [3] End-to-end delay=N/A" >> "$METRICS_FILE"
        fi

        overhead=$(echo "scale=0; $sum_bytes_mac / $valid_runs" | bc)
        echo "  [4] Overhead MAC=${overhead} byte/run" >> "$METRICS_FILE"

       

        det_total=$((sum_tp + sum_fp + sum_fn + sum_tn))
        if [ $det_total -gt 0 ]; then
            # --- INIZIO PROTEZIONE DATI ---
            # Se TN scende sottozero (saturazione), lo forziamo a 0
            if [ $sum_tn -lt 0 ]; then sum_tn=0; fi
            
            # Ricalcoliamo il totale dopo la correzione
            det_total=$((sum_tp + sum_fp + sum_fn + sum_tn))
            
            accuracy=$(echo "scale=2; ($sum_tp + $sum_tn)*100/$det_total" | bc)
            fpr_den=$((sum_fp + sum_tn))
            fnr_den=$((sum_fn + sum_tp))
            fpr="0.00"
            fnr="0.00"
            
            # Calcolo FPR protetto con awk (no errori bc)
            if [ $fpr_den -gt 0 ]; then 
                fpr=$(awk "BEGIN { 
                    val = ($sum_fp * 100) / $fpr_den;
                    if (val > 100.00) val = 100.00;
                    printf \"%.2f\", val
                }")
            fi
            
            if [ $fnr_den -gt 0 ]; then 
                fnr=$(awk "BEGIN {printf \"%.2f\", $sum_fn * 100 / $fnr_den}")
            fi
            # --- FINE PROTEZIONE DATI ---

            echo "  [5a] Detection passiva: TP=$sum_tp FP=$sum_fp FN=$sum_fn TN=$sum_tn" >> "$METRICS_FILE"
            echo "  [5a] Accuracy=$accuracy%  FPR=$fpr%  FNR=$fnr%" >> "$METRICS_FILE"
        else
            echo "  [5a] Detection passiva: N/A" >> "$METRICS_FILE"
        fi

        bl_total=$((sum_tp_bl + sum_fp_bl + sum_fn_bl))
        if [ $bl_total -gt 0 ]; then
            prec_den=$((sum_tp_bl + sum_fp_bl)); recall_den=$((sum_tp_bl + sum_fn_bl))
            prec=0; recall=0
            [ $prec_den -gt 0 ] && prec=$(echo "scale=2; $sum_tp_bl*100/$prec_den" | bc)
            [ $recall_den -gt 0 ] && recall=$(echo "scale=2; $sum_tp_bl*100/$recall_den" | bc)
            echo "  [5b] Detection attiva: TP=$sum_tp_bl FP=$sum_fp_bl FN=$sum_fn_bl" >> "$METRICS_FILE"
            echo "  [5b] Precision=$prec%  Recall=$recall%" >> "$METRICS_FILE"
        fi
    done

    # ============================================================
    # OVERHEAD MITIGAZIONE GRAYHOLE
    # ============================================================
    echo "" >> "$METRICS_FILE"
    echo "================================================================" >> "$METRICS_FILE"
    echo "OVERHEAD INTRODOTTO DALLA PROPOSTA" >> "$METRICS_FILE"
    echo "================================================================" >> "$METRICS_FILE"

    get_mac_avg() {
        local scenario=$1
        local total=0; local count=0
        for ((r=0; r<REPEAT; r++)); do
            FILE="$SCA_DIR/${scenario}-#${r}.sca"
            if [ ! -f "$FILE" ]; then continue; fi
            mac=$(grep "sentDownPk:sum(packetBytes)" "$FILE" | grep "wlan" | awk '{sum+=$NF} END {print sum+0}')
            if [ -n "$mac" ] && [ "$mac" != "0" ]; then
                total=$(echo "$total + $mac" | bc)
                count=$((count + 1))
            fi
        done
        [ $count -gt 0 ] && echo "scale=0; $total / $count" | bc || echo "0"
    }

    mac_s1=$(get_mac_avg "S1_NoAttack_Small")
    mac_s3=$(get_mac_avg "S3_NoAttack_Trust_Small")
    mac_s5=$(get_mac_avg "S5_NoAttack_Active_Small")

    echo "  Baseline (S1) MAC bytes/run: $mac_s1" >> "$METRICS_FILE"
    echo "  Trust passiva (S3) MAC bytes/run: $mac_s3" >> "$METRICS_FILE"
    echo "  Mitigazione attiva (S5) MAC bytes/run: $mac_s5" >> "$METRICS_FILE"
    echo "" >> "$METRICS_FILE"
    echo "  Overhead trust passiva (S3-S1): $(echo "scale=0; $mac_s3 - $mac_s1" | bc) byte/run" >> "$METRICS_FILE"
    echo "  Overhead mitigazione attiva (S5-S1): $(echo "scale=0; $mac_s5 - $mac_s1" | bc) byte/run" >> "$METRICS_FILE"

    echo ""
    echo "✅ Metriche salvate in: $METRICS_FILE"
    echo ""
    cat "$METRICS_FILE"
fi
