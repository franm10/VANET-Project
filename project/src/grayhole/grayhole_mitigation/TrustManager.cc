#include "TrustManager.h"
#include "TrustMonitor.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include <sstream>
#include <cmath>

Define_Module(TrustManager);

// ============================================================================
// INITIALIZE
// ============================================================================
void TrustManager::initialize(int stage) {
    if (stage == 0) {
        initialTrust = par("initialTrust").doubleValue();

        // Segnali statistici
        trustLocalSignal  = registerSignal("trustLocal");
        trustGlobalSignal = registerSignal("trustGlobal");

        trustThreshold = par("trustThreshold").doubleValue();

        // NON leggiamo più attackerNodeIds dal parametro .ned:
        // gli ID IP cambiano ad ogni run perché SUMO assegna gli IP
        // in ordine variabile. Li rileveremo automaticamente in finish()
        // cercando i moduli con nome "attacker_car".

        // Registra segnali detection
        detectionTPSignal = registerSignal("detectionTP");
        detectionFPSignal = registerSignal("detectionFP");
        detectionFNSignal = registerSignal("detectionFN");
        detectionTNSignal = registerSignal("detectionTN");

        std::cout << "[TRUST_MANAGER] Inizializzato, initialTrust="
                  << initialTrust << std::endl;
    }
    else if (stage == 1) {
        cModule *node = getParentModule();

        trustMonitor = check_and_cast<TrustMonitor*>(
            node->getSubmodule("trustMonitor"));
        if (!trustMonitor)
            throw cRuntimeError("TrustManager: TrustMonitor non trovato");
    }
}

// ============================================================================
// HANDLE MESSAGE
// ============================================================================
void TrustManager::handleMessage(cMessage *msg) {
    EV_WARN << "[TRUST_MANAGER] Messaggio inatteso ricevuto: "
            << msg->getName() << endl;
    delete msg;
}

// ============================================================================
// GET LOCAL TRUST
// ============================================================================
double TrustManager::getLocalTrust(int neighborId) {
    auto it = linkStats.find(neighborId);
    if (it == linkStats.end() || it->second.sent == 0) {
        return initialTrust;
    }

    double tLoc = (double)it->second.ackedLegit / it->second.sent;

    std::cout << "[TRUST_MANAGER] getLocalTrust: neighbor=" << neighborId
              << " sent=" << it->second.sent
              << " ackedLegit=" << it->second.ackedLegit
              << " T_loc=" << tLoc << std::endl;

    return tLoc;
}

// ============================================================================
// GET GLOBAL TRUST
// ============================================================================
double TrustManager::getGlobalTrust(int neighborId) {
    double tLoc = getLocalTrust(neighborId);

    auto extIt = receivedTrust.find(neighborId);
    if (extIt == receivedTrust.end() || extIt->second.empty()) {
        std::cout << "[TRUST_MANAGER] getGlobalTrust: neighbor=" << neighborId
                  << " nessun dato esterno → T_glob=T_loc=" << tLoc
                  << std::endl;
        return tLoc;
    }

    double numerator   = tLoc;
    double denominator = 1.0;

    std::cout << "[TRUST_MANAGER] getGlobalTrust: neighbor=" << neighborId
              << " T_loc=" << tLoc << std::endl;

    for (const auto& entry : extIt->second) {
        int    reporterId    = entry.first;
        double tRecv         = entry.second.trustValue;
        double tMeReporter   = getLocalTrust(reporterId);

        numerator   += tMeReporter * tRecv;
        denominator += 1.0;

        std::cout << "[TRUST_MANAGER]   reporter=" << reporterId
                  << " T_loc(me,k)=" << tMeReporter
                  << " T_recv(k,j)=" << tRecv
                  << " contributo=" << (tMeReporter * tRecv) << std::endl;
    }

    double tGlob = numerator / denominator;

    std::cout << "[TRUST_MANAGER] getGlobalTrust: neighbor=" << neighborId
              << " numeratore=" << numerator
              << " denominatore=" << denominator
              << " T_glob=" << tGlob << std::endl;

    emit(trustGlobalSignal, tGlob);
    return tGlob;
}

// ============================================================================
// ON FINAL ACK UPDATE
// ============================================================================
void TrustManager::onFinalAckUpdate(int neighborId, bool mic) {
    LinkStats& stats = linkStats[neighborId];
    stats.sent++;
    if (mic) stats.ackedLegit++;

    double tLoc = getLocalTrust(neighborId);
    emit(trustLocalSignal, tLoc);

    double tGlob = getGlobalTrust(neighborId);
    emit(trustGlobalSignal, tGlob);

    if (aodvModule) {
        aodvModule->broadcastTrustBeacon(neighborId, tLoc);
    }
}

// ============================================================================
// UPDATE FROM DISTRIBUTED INFO
// ============================================================================
void TrustManager::updateFromDistributedInfo(int reportingNodeId,
                                              int aboutNodeId,
                                              double reportedTrust) {
    ReceivedTrustEntry entry;
    entry.trustValue = reportedTrust;
    entry.timestamp  = simTime();

    receivedTrust[aboutNodeId][reportingNodeId] = entry;

    std::cout << "[TRUST_MANAGER] updateFromDistributedInfo:"
              << " reporter=" << reportingNodeId
              << " about=" << aboutNodeId
              << " T_recv=" << reportedTrust
              << " @ t=" << simTime() << std::endl;
}

// ============================================================================
// HAS SENT DATA
// ============================================================================
bool TrustManager::hasSentData(int neighborId) const {
    auto it = linkStats.find(neighborId);
    return it != linkStats.end() && it->second.sent > 0;
}

// ============================================================================
// DISCOVER ATTACKER NODE IDS
// Scansiona tutti i moduli nella simulazione e trova quelli con nome
// "attacker_car". Per ciascuno, legge l'ultimo ottetto dell'IP wlan0
// e lo inserisce in attackerNodeIds.
// Chiamato in finish() quando tutti gli IP sono sicuramente assegnati.
// ============================================================================
void TrustManager::discoverAttackerNodeIds() {
    attackerNodeIds.clear();

    cModule *sim = getSimulation()->getSystemModule();
    for (cModule::SubmoduleIterator it(sim); !it.end(); ++it) {
        cModule *mod = *it;

        // Controlla se il nome del modulo contiene "attacker_car"
        std::string modName = mod->getName();
        if (modName.find("attacker_car") == std::string::npos)
            continue;

        // Trova l'interfaccia wlan0 e leggi l'IP
        cModule *ifTableMod = mod->getSubmodule("interfaceTable");
        if (!ifTableMod) continue;

        IInterfaceTable *ift = dynamic_cast<IInterfaceTable*>(ifTableMod);
        if (!ift) continue;

        auto iface = ift->findInterfaceByName("wlan0");
        if (!iface) continue;

        auto ipData = iface->getProtocolData<Ipv4InterfaceData>();
        if (!ipData) continue;

        int ipId = ipData->getIPAddress().getInt() & 0xFF;
        attackerNodeIds.insert(ipId);

        std::cout << "[TRUST_MANAGER] discoverAttackerNodeIds: trovato "
                  << mod->getFullName()
                  << " IP last octet=" << ipId << std::endl;
    }

    std::cout << "[TRUST_MANAGER] discoverAttackerNodeIds: totale "
              << attackerNodeIds.size() << " attaccanti rilevati"
              << std::endl;
}

// ============================================================================
// FINISH
// ============================================================================
void TrustManager::finish() {
    std::cout << "[TRUST_MANAGER] ========================================"
              << std::endl;
    std::cout << "[TRUST_MANAGER] STATISTICHE FINALI LINK" << std::endl;

    // --------------------------------------------------------
    // AUTO-DETECT: scopri gli ID degli attaccanti dal tipo di modulo
    // Questo funziona per qualsiasi run, indipendentemente dagli IP
    // --------------------------------------------------------
    discoverAttackerNodeIds();

    // --------------------------------------------------------
    // DETECTION: classifica ogni vicino osservato
    // --------------------------------------------------------
    int tp = 0, fp = 0, fn = 0, tn = 0;

    for (const auto& entry : linkStats) {
        int    neighborId = entry.first;
        double tGlob      = getGlobalTrust(neighborId);
        bool   classified = (tGlob < trustThreshold);
        bool   isAttacker = (attackerNodeIds.count(neighborId) > 0);

        if (classified && isAttacker)  tp++;
        else if (classified && !isAttacker) fp++;
        else if (!classified && isAttacker) fn++;
        else                                tn++;
    }

    emit(detectionTPSignal, (double)tp);
    emit(detectionFPSignal, (double)fp);
    emit(detectionFNSignal, (double)fn);
    emit(detectionTNSignal, (double)tn);

    std::cout << "[TRUST_MANAGER] Detection: TP=" << tp
              << " FP=" << fp << " FN=" << fn << " TN=" << tn << std::endl;

    for (const auto& entry : linkStats) {
        int    neighborId  = entry.first;
        int    sent        = entry.second.sent;
        int    acked       = entry.second.ackedLegit;
        double tLoc        = (sent > 0) ? (double)acked / sent : initialTrust;
        bool   isAttacker  = (attackerNodeIds.count(neighborId) > 0);

        std::cout << "[TRUST_MANAGER]   neighbor=" << neighborId
                  << " sent=" << sent
                  << " ackedLegit=" << acked
                  << " T_loc=" << tLoc
                  << (isAttacker ? " [ATTACKER]" : " [ONESTO]")
                  << std::endl;
    }

    std::cout << "[TRUST_MANAGER] ========================================"
              << std::endl;
}
