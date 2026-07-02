#include "TrustMonitor.h"
#include "TrustManager.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "../grayhole_attackwithtrust/AttackerTrustAwareAODV.h"

Define_Module(TrustMonitor);

void TrustMonitor::initialize(int stage) {
    if (stage == 0) {
        cleanupTimeout = par("cleanupTimeout").doubleValue();
        messageLength  = par("messageLength").intValue();

        cleanupTimer = new cMessage("cleanupTimer");
        scheduleAt(simTime() + cleanupTimeout, cleanupTimer);

        // Registra segnali
        pdrSignal                 = registerSignal("pdr");
        perSignal                 = registerSignal("per");
        throughputSignal          = registerSignal("throughput");
        delaySignal               = registerSignal("endToEndDelay");
        beaconOverheadBytesSignal = registerSignal("beaconOverheadBytes");

        std::cout << "[TRUST_MONITOR] Inizializzato, cleanupTimeout="
                  << cleanupTimeout << "s" << std::endl;
    }
    else if (stage == 1) {
        cModule *node = getParentModule();
        trustManager = check_and_cast<TrustManager*>(
            node->getSubmodule("trustManager"));
        if (!trustManager)
            throw cRuntimeError("TrustMonitor: TrustManager non trovato");
    }
    else if (stage == 2) {
        myNodeId = getParentModule()->getIndex();
        std::cout << "[TRUST_MONITOR] NodeId=" << myNodeId << std::endl;
    }
}

void TrustMonitor::handleMessage(cMessage *msg) {
    if (msg == cleanupTimer) {
        cleanupExpiredEntries();
        scheduleAt(simTime() + cleanupTimeout, cleanupTimer);
    } else {
        delete msg;
    }
}

// ============================================================================
// ON PACKET SENT
// Chiamato dalla sorgente (TrustAwareAODV::datagramLocalOutHook)
// ============================================================================
void TrustMonitor::onPacketSent(int packetId, simtime_t sendTime) {
    if (packetSendTime.find(packetId) == packetSendTime.end()) {
        packetSendTime[packetId] = sendTime;
        totalPacketsSent++;
    }
}

// ============================================================================
// ON BEACON SENT
// Chiamato da TrustAwareAODV::broadcastTrustBeacon
// ============================================================================
void TrustMonitor::onBeaconSent(int beaconBytes) {
    totalBeaconBytes += beaconBytes;
}

// ============================================================================
// ON PACKET FORWARDED
// ============================================================================
void TrustMonitor::onPacketForwarded(int packetId,
                                      int receivedFromNodeId,
                                      int forwardedToNodeId,
                                      simtime_t forwardTime) {
    if (forwardTable.find(packetId) != forwardTable.end()) {
        std::cout << "[TRUST_MONITOR] onPacketForwarded: packetId="
                  << packetId << " già in forwardTable, skip" << std::endl;
        return;
    }

    ForwardEntry entry;
    entry.packetId           = packetId;
    entry.receivedFromNodeId = receivedFromNodeId;
    entry.forwardedToNodeId  = forwardedToNodeId;
    entry.forwardTime        = forwardTime;
    entry.ackReceived        = false;

    forwardTable[packetId] = entry;

    std::cout << "[TRUST_MONITOR] onPacketForwarded: packetId=" << packetId
              << " receivedFrom=" << receivedFromNodeId
              << " forwardedTo=" << forwardedToNodeId
              << " t=" << forwardTime << std::endl;
}

// ============================================================================
// ON FINAL ACK RECEIVED
//
// FIX: OGNI nodo (sorgente e intermedio) aggiorna la trust del proprio
// forwardedTo in base al MIC ricevuto.
//
// Esempio con percorso: Sorgente(115) → Nodo(117) → Attaccante(75) → DROP
//
// PRIMA del fix:
//   - Nodo 117 riceve ACK MIC=0, propaga a 115 SENZA aggiornare trust
//   - Sorgente 115 aggiorna trust di 117 (onesto!) con MIC=0
//   - Attaccante 75 mai penalizzato
//
// DOPO il fix:
//   - Nodo 117 riceve ACK MIC=0, aggiorna trust di 75 (attaccante!), poi propaga
//   - Sorgente 115 aggiorna trust di 117 con MIC=0
//   - Attaccante 75 correttamente penalizzato da 117
// ============================================================================
void TrustMonitor::onFinalAckReceived(IntermediateAck *ack) {
    int  packetId = ack->getOriginalPacketId();
    bool mic      = ack->getMic();

    if (alreadyProcessedAck.count(packetId)) {
        delete ack;
        return;
    }
    alreadyProcessedAck.insert(packetId);

    auto it = forwardTable.find(packetId);
    if (it == forwardTable.end()) {
        delete ack;
        return;
    }

    ForwardEntry& entry = it->second;
    int predecessoreId = entry.receivedFromNodeId;
    int forwardedTo    = entry.forwardedToNodeId;
    forwardTable.erase(it);

    // ================================================================
    // FIX: TUTTI i nodi aggiornano la trust del proprio next-hop.
    // Questa riga era prima SOLO dentro il blocco predecessoreId==-1.
    // Spostandola qui, anche i nodi intermedi penalizzano il loro
    // forwardedTo quando ricevono un ACK MIC=0.
    // ================================================================
    trustManager->onFinalAckUpdate(forwardedTo, mic);

    std::cout << "[TRUST_MONITOR] onFinalAckReceived: packetId=" << packetId
              << " forwardedTo=" << forwardedTo
              << " MIC=" << mic
              << " predecessore=" << predecessoreId
              << " → trust aggiornata per " << forwardedTo << std::endl;

    if (predecessoreId == -1) {
        // Sono la sorgente: gestisco anche PDR/delay/throughput
        if (mic) {
            totalPacketsDelivered++;
            totalBytesDelivered += messageLength;

            auto sendIt = packetSendTime.find(packetId);
            if (sendIt != packetSendTime.end()) {
                simtime_t delay = simTime() - sendIt->second;
                emit(delaySignal, delay);
                packetSendTime.erase(sendIt);
            }
        }

        delete ack;
        return;
    }

    // Nodo intermedio: propago l'ACK al predecessore
    // (la trust del mio forwardedTo è già stata aggiornata sopra)
    cModule *sim = getSimulation()->getSystemModule();
    cModule *predecessoreRouting = nullptr;

    for (cModule::SubmoduleIterator it2(sim); !it2.end(); ++it2) {
        cModule *mod = *it2;
        if (std::string(mod->getName()).find("car") == std::string::npos)
            continue;
        cModule *ifTableMod = mod->getSubmodule("interfaceTable");
        if (!ifTableMod) continue;
        IInterfaceTable *ift = dynamic_cast<IInterfaceTable*>(ifTableMod);
        if (!ift) continue;
        auto iface = ift->findInterfaceByName("wlan0");
        if (!iface) continue;
        auto ipData = iface->getProtocolData<Ipv4InterfaceData>();
        if (ipData &&
            ((int)(ipData->getIPAddress().getInt() & 0xFF) == predecessoreId))
        {
            predecessoreRouting = mod->getSubmodule("routing");
            break;
        }
    }

    if (!predecessoreRouting) {
        delete ack;
        return;
    }

    IntermediateAck *propagatedAck = new IntermediateAck("IntermediateAck");
    propagatedAck->setOriginalPacketId(packetId);
    propagatedAck->setMic(mic);
    propagatedAck->setSenderNodeId(myNodeId);
    propagatedAck->setPreviousHopNodeId(predecessoreId);
    propagatedAck->setTimestamp(ack->getTimestamp());

    AttackerTrustAwareAODV *prevAttacker =
        dynamic_cast<AttackerTrustAwareAODV*>(predecessoreRouting);
    if (prevAttacker) {
        prevAttacker->onFinalAckReceived(propagatedAck);
        delete ack;
        return;
    }

    TrustAwareAODV *prevAodv =
        dynamic_cast<TrustAwareAODV*>(predecessoreRouting);
    if (prevAodv) {
        prevAodv->onFinalAckReceived(propagatedAck);
        delete ack;
        return;
    }

    delete propagatedAck;
    delete ack;
}


int TrustMonitor::getPredecessor(int packetId) {
    auto it = forwardTable.find(packetId);
    if (it == forwardTable.end()) return -1;
    return it->second.receivedFromNodeId;
}

// ============================================================================
// CLEANUP EXPIRED ENTRIES
// Rimuove entry scadute SENZA penalità trust.
// I pacchetti persi per mobilità/collisione/route break non vengono
// conteggiati nella formula T_loc = ackedLegit / sent.
// Solo i pacchetti con ACK esplicito (MIC=1 o MIC=0) influenzano la trust.
// ============================================================================
void TrustMonitor::cleanupExpiredEntries() {
    simtime_t now     = simTime();
    int       removed = 0;

    for (auto it = forwardTable.begin(); it != forwardTable.end(); ) {
        ForwardEntry& entry = it->second;
        if (!entry.ackReceived &&
            (now - entry.forwardTime).dbl() > cleanupTimeout)
        {
            std::cout << "[TRUST_MONITOR] cleanup: rimozione packetId="
                      << entry.packetId
                      << " forwardedTo=" << entry.forwardedToNodeId
                      << " età=" << (now - entry.forwardTime) << "s"
                      << " (nessuna penalità trust)" << std::endl;
            it = forwardTable.erase(it);
            removed++;
        } else {
            ++it;
        }
    }

    if (removed > 0)
        std::cout << "[TRUST_MONITOR] cleanup: rimosse " << removed
                  << " entry scadute" << std::endl;
}

// ============================================================================
// FINISH — emette tutte le metriche aggregate
// ============================================================================
void TrustMonitor::finish() {
    if (cleanupTimer) {
        if (cleanupTimer->isScheduled()) cancelEvent(cleanupTimer);
        drop(cleanupTimer);
        delete cleanupTimer;
        cleanupTimer = nullptr;
    }

    // PDR e PER
    if (totalPacketsSent > 0) {
        double pdr = (double)totalPacketsDelivered / totalPacketsSent * 100.0;
        double per = 100.0 - pdr;
        emit(pdrSignal, pdr);
        emit(perSignal, per);
    }

    // Throughput: byte consegnati / durata simulazione (escluso warmup)
    double warmupDuration = par("warmupDuration").doubleValue();
    double simDuration = (simTime() - SimTime(warmupDuration, SIMTIME_S)).dbl();
    if (simDuration > 0 && totalBytesDelivered > 0) {
        double throughput = (double)totalBytesDelivered / simDuration;
        emit(throughputSignal, throughput);
    }

    // Overhead beacon
    if (totalBeaconBytes > 0)
        emit(beaconOverheadBytesSignal, (double)totalBeaconBytes);

    std::cout << "[TRUST_MONITOR] finish:"
              << " sent=" << totalPacketsSent
              << " delivered=" << totalPacketsDelivered
              << " bytes=" << totalBytesDelivered
              << " beaconBytes=" << totalBeaconBytes
              << std::endl;
}
