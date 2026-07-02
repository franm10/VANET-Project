#include "TrustAwareAODV.h"
#include "../grayhole_attackwithtrust/AttackerTrustAwareAODV.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

// Definizioni statiche
std::map<int, int> TrustAwareAODV::flowPacketCounter;
std::map<int, int> TrustAwareAODV::globalPacketPredecessor;
std::map<int, int> TrustAwareAODV::omnetIdToUniqueId;

int globalUniquePacketId = 1;

Define_Module(TrustAwareAODV);

// ============================================================================
// INITIALIZE
// ============================================================================
void TrustAwareAODV::initialize(int stage) {
    Aodv::initialize(stage);



    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        enableTrustRouting = par("enableTrustRouting").boolValue();
        warmupPacketLimit  = par("warmupPacketLimit").intValue();

        // Timer cleanup candidati (ogni activeRouteTimeout secondi)
        candidatesCleanupTimer = new cMessage("candidatesCleanupTimer");
        scheduleAt(simTime() + activeRouteTimeout,
                   candidatesCleanupTimer);

        cModule *node = getParentModule();
        while (node && !node->getSubmodule("trustManager"))
            node = node->getParentModule();

        if (node) {
            trustManager = check_and_cast<TrustManager*>(
                node->getSubmodule("trustManager"));
            trustMonitor = check_and_cast<TrustMonitor*>(
                node->getSubmodule("trustMonitor"));

            trustManager->setAodvModule(this);
            trustManager->setTrustMonitor(trustMonitor);
        }

        std::cout << "[TRUST_AODV] Inizializzato nodo="
                  << getParentModule()->getIndex()
                  << " modulo=" << getParentModule()->getFullName()
                  << " warmupLimit=" << warmupPacketLimit
                  << " enableTrustRouting=" << enableTrustRouting
                  << std::endl;
    }
}

// ============================================================================
// HANDLE MESSAGE
// ============================================================================
void TrustAwareAODV::handleMessage(cMessage *msg) {
    if (msg == candidatesCleanupTimer) {
        cleanupCandidates();
        scheduleAt(simTime() + activeRouteTimeout, candidatesCleanupTimer);
    }
    else {
        Aodv::handleMessageWhenUp(msg);
    }
}

// ============================================================================
// UTILITY
// ============================================================================
int TrustAwareAODV::getNodeIdFromAddress(const L3Address& addr) {
    return addr.toIpv4().getInt() & 0xFF;
}

cModule* TrustAwareAODV::getSafeRoutingModule(int targetNodeId) {
    cModule *sim = getSimulation()->getSystemModule();
    for (cModule::SubmoduleIterator it(sim); !it.end(); ++it) {
        cModule *mod = *it;
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
            ((int)(ipData->getIPAddress().getInt() & 0xFF) == targetNodeId))
            return mod->getSubmodule("routing");
    }
    return nullptr;
}

int TrustAwareAODV::getOrCreatePacketId(Packet *datagram) {
    const auto& ipv4Header = datagram->peekAtFront<Ipv4Header>();
    if (!ipv4Header) return -1;

    uint32_t destIp = ipv4Header->getDestAddress().getInt();
    uint16_t identification = ipv4Header->getIdentification();

    int uniqueKey = (int)((destIp & 0xFF) << 16 | identification);

    auto it = omnetIdToUniqueId.find(uniqueKey);
    if (it != omnetIdToUniqueId.end())
        return it->second;

    int uniqueId = globalUniquePacketId++;
    omnetIdToUniqueId[uniqueKey] = uniqueId;
    return uniqueId;
}

// ============================================================================
// CLEANUP CANDIDATES
// Rimuove entry di neighborCandidates con lastUpdate scaduto
// (freschezza > activeRouteTimeout)
// ============================================================================
void TrustAwareAODV::cleanupCandidates() {
    simtime_t now     = simTime();
    int       removed = 0;

    for (auto destIt = neighborCandidates.begin();
         destIt != neighborCandidates.end(); )
    {
        auto& candidates = destIt->second;
        for (auto candIt = candidates.begin();
             candIt != candidates.end(); )
        {
            if ((now - candIt->second.lastUpdate) > activeRouteTimeout) {
                std::cout << "[TRUST_AODV] cleanup candidato scaduto:"
                          << " dest=" << destIt->first
                          << " neighbor=" << candIt->first << std::endl;
                candIt = candidates.erase(candIt);
                removed++;
            }
            else {
                ++candIt;
            }
        }

        if (candidates.empty())
            destIt = neighborCandidates.erase(destIt);
        else
            ++destIt;
    }

    if (removed > 0) {
        std::cout << "[TRUST_AODV] cleanup: rimossi " << removed
                  << " candidati scaduti" << std::endl;
    }
}

// ============================================================================
// IS CURRENT ONE HOP NEIGHBOR
// Verifica che neighborId sia raggiungibile con metrica 1
// nella routing table corrente (gestione mobilità)
// ============================================================================
bool TrustAwareAODV::isCurrentOneHopNeighbor(int neighborId) {
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() != this) continue;
        if (route->getMetric() != 1)    continue;

        int routeDestId = getNodeIdFromAddress(
            route->getDestinationAsGeneric());
        if (routeDestId == neighborId)
            return true;
    }
    return false;
}

// ============================================================================
// SELECT BEST CANDIDATE
// Filtra neighborCandidates[destId] per freschezza + 1-hop attuale,
// calcola score = (1 - T_glob(neighbor)) * declaredHop,
// restituisce l'indirizzo del vincitore (minimo score).
// Restituisce L3Address() (unspecified) se lista vuota dopo filtro.
// ============================================================================
L3Address TrustAwareAODV::selectBestCandidate(int destId) {
    auto destIt = neighborCandidates.find(destId);
    if (destIt == neighborCandidates.end())
        return L3Address();

    double   bestScore      = std::numeric_limits<double>::max();
    int      bestNeighborId = -1;

    for (const auto& entry : destIt->second) {
        int neighborId   = entry.first;
        int declaredHop  = entry.second.declaredHopCount;
        simtime_t lastUp = entry.second.lastUpdate;

        // Filtro freschezza
        if ((simTime() - lastUp) > activeRouteTimeout) {
            std::cout << "[TRUST_AODV] selectBestCandidate: neighbor="
                      << neighborId << " scaduto, skip" << std::endl;
            continue;
        }

        // Filtro 1-hop attuale (mobilità)
        if (!isCurrentOneHopNeighbor(neighborId)) {
            std::cout << "[TRUST_AODV] selectBestCandidate: neighbor="
                      << neighborId
                      << " non più a 1-hop, skip" << std::endl;
            continue;
        }

        double tGlob = trustManager->getGlobalTrust(neighborId);
        double score = (1.0 - tGlob) * declaredHop;

        std::cout << "[TRUST_AODV] selectBestCandidate: neighbor="
                  << neighborId
                  << " declaredHop=" << declaredHop
                  << " T_glob=" << tGlob
                  << " score=" << score << std::endl;

        if (score < bestScore) {
            bestScore      = score;
            bestNeighborId = neighborId;
        }
    }

    if (bestNeighborId < 0) {
        std::cout << "[TRUST_AODV] selectBestCandidate: dest=" << destId
                  << " nessun candidato valido, AODV decide" << std::endl;
        return L3Address();
    }

    std::cout << "[TRUST_AODV] selectBestCandidate: dest=" << destId
              << " vincitore=" << bestNeighborId
              << " score=" << bestScore << std::endl;

    // Risolve neighborId → L3Address tramite routing table (rotta 1-hop)
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() != this) continue;
        if (route->getMetric() != 1)    continue;

        if (getNodeIdFromAddress(route->getDestinationAsGeneric())
            == bestNeighborId)
        {
            return route->getDestinationAsGeneric();
        }
    }

    return L3Address();
}

// ============================================================================
// HANDLE RREP
// Popola neighborCandidates per ogni RREP ricevuto,
// indipendentemente da cosa farà Aodv::handleRREP.
// Nessun gate binario sulla trust.
// ============================================================================
void TrustAwareAODV::handleRREP(const Ptr<Rrep>& rrep,
                                 const L3Address& sourceAddr) {
    int hopCount = rrep->getHopCount();
    int destId   = getNodeIdFromAddress(rrep->getDestAddr());
    int neighborId = getNodeIdFromAddress(sourceAddr);

    int myNodeId = -1;
    auto iface = interfaceTable->findInterfaceByName("wlan0");
    if (iface) {
        myNodeId = iface->getProtocolData<Ipv4InterfaceData>()
                       ->getIPAddress().getInt() & 0xFF;
    }

    if (neighborId > 0 && destId > 0) {
        if (neighborId == myNodeId) {
            std::cout << "[TRUST_AODV] handleRREP: ignoro candidato"
                      << " neighborId==myNodeId dest=" << destId
                      << " (eviterebbe loop su se stesso)" << std::endl;
        } else {
            CandidateEntry candidate;
            candidate.declaredHopCount = hopCount + 1;
            candidate.lastUpdate       = simTime();
            neighborCandidates[destId][neighborId] = candidate;
            std::cout << "[TRUST_AODV] handleRREP: candidato aggiunto"
                      << " dest=" << destId
                      << " neighbor=" << neighborId
                      << " hopCount=" << (hopCount + 1) << std::endl;
        }
    }

    Aodv::handleRREP(rrep, sourceAddr);
}
// ============================================================================
// HANDLE RREQ — invariato rispetto ad Aodv
// ============================================================================
void TrustAwareAODV::handleRREQ(const Ptr<Rreq>& rreq,
                                 const L3Address& sourceAddr,
                                 unsigned int timeToLive) {
    Aodv::handleRREQ(rreq, sourceAddr, timeToLive);
}

// ============================================================================
// APPLICA OVERRIDE NEXT-HOP (helper interno)
// Usato da datagramForwardHook e datagramLocalOutHook in fase attiva.
// Modifica tag NextHopAddressReq + IRoute se il candidato migliore
// è diverso dal next-hop corrente.
// ============================================================================
static void applyNextHopOverride(Packet *datagram,
                                  const L3Address& bestNextHop,
                                  IRoutingTable *routingTable,
                                  const L3Address& destAddr,
                                  int bestNeighborId,
                                  int currentNextHopId)
{
    if (bestNeighborId == currentNextHopId)
        return; // già sul candidato migliore

    std::cout << "[TRUST_AODV] override next-hop:"
              << " corrente=" << currentNextHopId
              << " → migliore=" << bestNeighborId << std::endl;

    // 1. Override tag (effetto immediato sul pacchetto corrente)
    datagram->addTagIfAbsent<NextHopAddressReq>()
            ->setNextHopAddress(bestNextHop);

    // 2. Override IRoute (coerenza per pacchetti successivi e RERR)
    IRoute *route = routingTable->findBestMatchingRoute(destAddr);
    if (route) {
        route->setNextHop(bestNextHop);
        std::cout << "[TRUST_AODV] IRoute aggiornata: dest="
                  << destAddr << " nextHop=" << bestNextHop << std::endl;
    }
}

// ============================================================================
// DATAGRAM FORWARD HOOK
// Pacchetti in transito (nodi intermedi)
// ============================================================================
INetfilter::IHook::Result TrustAwareAODV::datagramForwardHook(
    Packet *datagram)
{
    // Prima la logica AODV base
    auto baseResult = Aodv::datagramForwardHook(datagram);
    if (baseResult != ACCEPT) return baseResult;

    const auto& ipv4Header = datagram->peekAtFront<Ipv4Header>();
    if (!ipv4Header) return ACCEPT;

    // Filtra AODV (porta 654) e broadcast/multicast
    if (ipv4Header->getProtocolId() == IP_PROT_UDP) {
        try {
            auto udpHdr = datagram->peekDataAt<UdpHeader>(
                ipv4Header->getChunkLength());
            if (udpHdr->getDestPort() == 654 ||
                udpHdr->getSourcePort() == 654)
                return ACCEPT;
        } catch (...) { return ACCEPT; }
    }

    L3Address destAddr = ipv4Header->getDestAddress();
    L3Address srcAddr  = ipv4Header->getSourceAddress();

    if (destAddr.isBroadcast() || destAddr.isMulticast()) return ACCEPT;

    auto iface = interfaceTable->findInterfaceByName("wlan0");
    if (!iface) return ACCEPT;

    Ipv4Address myIp = iface->getProtocolData<Ipv4InterfaceData>()
                           ->getIPAddress();
    if (srcAddr.toIpv4() == myIp) return ACCEPT;

    int packetId    = getOrCreatePacketId(datagram);
    int myNodeId    = myIp.getInt() & 0xFF;
    int srcNodeId   = getNodeIdFromAddress(srcAddr);
    int destId      = getNodeIdFromAddress(destAddr);
    int incomingTTL = ipv4Header->getTimeToLive();

    // Aggiorna predecessore globale
    int previousHopId = srcNodeId;
    auto predIt = globalPacketPredecessor.find(packetId);
    if (predIt != globalPacketPredecessor.end())
        previousHopId = predIt->second;
    globalPacketPredecessor[packetId] = myNodeId;

    // Rotta corrente
    IRoute *route = routingTable->findBestMatchingRoute(destAddr);
    if (!route) return ACCEPT;

    L3Address currentNextHop = route->getNextHopAsGeneric();
    if (currentNextHop.isUnspecified()) currentNextHop = destAddr;
    int currentNextHopId = getNodeIdFromAddress(currentNextHop);

    // Determina numero pacchetto del flusso (src/dest)
    int flowKey = (int)(((srcAddr.toIpv4().getInt() & 0xFF) << 8) |
                        (destAddr.toIpv4().getInt() & 0xFF));
    int flowNum = flowPacketCounter.count(flowKey)
                  ? flowPacketCounter[flowKey] : 0;



    // Fase attiva: applica selezione basata su score
    int actualNextHopId = currentNextHopId;

    if (enableTrustRouting && flowNum > warmupPacketLimit) {
        L3Address bestNextHop = selectBestCandidate(destId);
        if (!bestNextHop.isUnspecified()) {
            int bestId = getNodeIdFromAddress(bestNextHop);
            applyNextHopOverride(datagram, bestNextHop, routingTable,
                                 destAddr, bestId, currentNextHopId);
            actualNextHopId = bestId;
        }
    }

    if (trustMonitor) {
        trustMonitor->onPacketForwarded(packetId, previousHopId,
                                        actualNextHopId, simTime());
    }

    return ACCEPT;
}

// ============================================================================
// DATAGRAM LOCAL OUT HOOK
// Pacchetti generati da questo nodo (sorgente)
// ============================================================================
INetfilter::IHook::Result TrustAwareAODV::datagramLocalOutHook(
    Packet *datagram)
{
    auto baseResult = Aodv::datagramLocalOutHook(datagram);
    if (baseResult != ACCEPT) return baseResult;
    const auto& ipv4Header = datagram->peekAtFront<Ipv4Header>();
    if (!ipv4Header) return ACCEPT;
    if (ipv4Header->getProtocolId() == IP_PROT_UDP) {
        try {
            auto udpHdr = datagram->peekDataAt<UdpHeader>(
                ipv4Header->getChunkLength());
            if (udpHdr->getDestPort() == 654 ||
                udpHdr->getSourcePort() == 654)
                return ACCEPT;
        } catch (...) { return ACCEPT; }
    }
    L3Address destAddr = ipv4Header->getDestAddress();
    if (destAddr.isBroadcast() || destAddr.isMulticast()) return ACCEPT;
    auto iface = interfaceTable->findInterfaceByName("wlan0");
    if (!iface) return ACCEPT;
    Ipv4Address myIp = iface->getProtocolData<Ipv4InterfaceData>()
                           ->getIPAddress();

    int packetId = getOrCreatePacketId(datagram);
    int destId   = getNodeIdFromAddress(destAddr);
    int flowKey = (int)(((myIp.getInt() & 0xFF) << 8) |
                        (destAddr.toIpv4().getInt() & 0xFF));

    // Incrementa il flow counter solo per pacchetti nuovi
    static std::set<int> countedPacketIds;
    if (countedPacketIds.find(packetId) == countedPacketIds.end()) {
        countedPacketIds.insert(packetId);
        flowPacketCounter[flowKey]++;
    }

    int flowNum = flowPacketCounter[flowKey];
    std::cout << "[TRUST_AODV] datagramLocalOutHook: src="
              << (myIp.getInt() & 0xFF)
              << " dest=" << destId
              << " flowNum=" << flowNum
              << (flowNum <= warmupPacketLimit ? " [WARMUP]" : " [ATTIVA]")
              << std::endl;
    IRoute *route = routingTable->findBestMatchingRoute(destAddr);
    if (!route) return ACCEPT;
    L3Address currentNextHop = route->getNextHopAsGeneric();
    if (currentNextHop.isUnspecified()) currentNextHop = destAddr;
    int currentNextHopId = getNodeIdFromAddress(currentNextHop);

    // Registra timestamp invio (indipendente dal next-hop)
    if (trustMonitor) {
        trustMonitor->onPacketSent(packetId, simTime());
    }

    // Fase attiva: applica override basato su trust
    int actualNextHopId = currentNextHopId;
    if (enableTrustRouting && flowNum > warmupPacketLimit) {
        L3Address bestNextHop = selectBestCandidate(destId);
        if (!bestNextHop.isUnspecified()) {
            int bestId = getNodeIdFromAddress(bestNextHop);
            applyNextHopOverride(datagram, bestNextHop, routingTable,
                                 destAddr, bestId, currentNextHopId);
            actualNextHopId = bestId;
        }
    }

    // Registra il forward DOPO l'override, con il next-hop effettivo
    // -1 = sono la sorgente (nessun predecessore)
    if (trustMonitor) {
        trustMonitor->onPacketForwarded(packetId, -1,
                                        actualNextHopId, simTime());
    }

    return ACCEPT;
}

// ============================================================================
// DATAGRAM LOCAL IN HOOK
// Pacchetti arrivati a destinazione: genera ACK finale con MIC=1
// ============================================================================
INetfilter::IHook::Result TrustAwareAODV::datagramLocalInHook(
    Packet *datagram)
{
    auto baseResult = Aodv::datagramLocalInHook(datagram);
    if (baseResult != ACCEPT) return baseResult;

    const auto& ipv4Header = datagram->peekAtFront<Ipv4Header>();
    if (!ipv4Header) return ACCEPT;

    if (ipv4Header->getProtocolId() == IP_PROT_UDP) {
        try {
            auto udpHdr = datagram->peekDataAt<UdpHeader>(
                ipv4Header->getChunkLength());
            if (udpHdr->getDestPort() == 654 ||
                udpHdr->getSourcePort() == 654)
                return ACCEPT;
        } catch (...) { return ACCEPT; }
    }

    L3Address srcAddr  = ipv4Header->getSourceAddress();
    L3Address destAddr = ipv4Header->getDestAddress();

    if (destAddr.isBroadcast() || destAddr.isMulticast()) return ACCEPT;

    auto iface = interfaceTable->findInterfaceByName("wlan0");
    if (!iface) return ACCEPT;

    Ipv4Address myIp = iface->getProtocolData<Ipv4InterfaceData>()
                           ->getIPAddress();
    if (destAddr.toIpv4() != myIp) return ACCEPT;
    if (srcAddr.toIpv4()  == myIp) return ACCEPT;

    int packetId = getOrCreatePacketId(datagram);
    int myNodeId = myIp.getInt() & 0xFF;

    // Trova il predecessore diretto (chi ha mandato il pacchetto a me)
    int predecessoreId = srcAddr.toIpv4().getInt() & 0xFF;
    auto predIt = globalPacketPredecessor.find(packetId);
    if (predIt != globalPacketPredecessor.end())
        predecessoreId = predIt->second;

    std::cout << "[TRUST_AODV] datagramLocalInHook: packetId=" << packetId
              << " destinazione raggiunta! Invio ACK finale MIC=1"
              << " verso predecessore=" << predecessoreId << std::endl;

    // Costruisce e invia ACK finale con MIC=1
    IntermediateAck *ack = new IntermediateAck("FinalAck");
    ack->setOriginalPacketId(packetId);
    ack->setMic(true);
    ack->setSenderNodeId(myNodeId);
    ack->setPreviousHopNodeId(predecessoreId);
    ack->setTimestamp(simTime().dbl());

    // Invia al predecessore
    cModule *prevRouting = getSafeRoutingModule(predecessoreId);
    if (!prevRouting) {
        std::cout << "[TRUST_AODV] predecessore=" << predecessoreId
                  << " non trovato (mobilità?)" << std::endl;
        delete ack;
        return ACCEPT;
    }

    AttackerTrustAwareAODV *prevAttacker =
        dynamic_cast<AttackerTrustAwareAODV*>(prevRouting);
    if (prevAttacker) {
        prevAttacker->onFinalAckReceived(ack);
        return ACCEPT;
    }

    TrustAwareAODV *prevAodv =
        dynamic_cast<TrustAwareAODV*>(prevRouting);
    if (prevAodv) {
        prevAodv->onFinalAckReceived(ack);
        return ACCEPT;
    }

    delete ack;
    return ACCEPT;
}

// ============================================================================
// ON FINAL ACK RECEIVED
// Delega a TrustMonitor che aggiorna TrustManager e propaga a ritroso
// ============================================================================
void TrustAwareAODV::onFinalAckReceived(IntermediateAck *ack) {
    if (trustMonitor) {
        trustMonitor->onFinalAckReceived(ack);
    } else {
        delete ack;
    }
}

// ============================================================================
// BROADCAST TRUST BEACON
// Invia T_loc(me, aboutNodeId) a tutti i vicini a 1-hop
// ============================================================================
void TrustAwareAODV::broadcastTrustBeacon(int aboutNodeId,
                                           double trustValue) {
    if (!routingTable) return;
    auto iface = interfaceTable->findInterfaceByName("wlan0");
    if (!iface) return;
    int myNodeId = iface->getProtocolData<Ipv4InterfaceData>()
                       ->getIPAddress().getInt() & 0xFF;
    int sentCount = 0;
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getMetric() != 1) continue;
        L3Address nextHop = route->getNextHopAsGeneric();
        if (nextHop.isUnspecified()) continue;
        int neighborId = getNodeIdFromAddress(nextHop);
        if (neighborId <= 0 || neighborId == myNodeId) continue;
        if (neighborId == aboutNodeId)                  continue;
        cModule *neighborRouting = getSafeRoutingModule(neighborId);
        if (!neighborRouting) continue;
        TrustBeacon *beacon = new TrustBeacon("TrustBeacon");
        beacon->setSenderNodeId(myNodeId);
        beacon->setAboutNodeId(aboutNodeId);
        beacon->setTrustValue(trustValue);
        AttackerTrustAwareAODV *neighborAttacker =
            dynamic_cast<AttackerTrustAwareAODV*>(neighborRouting);
        if (neighborAttacker) {
            neighborAttacker->receiveTrustBeacon(beacon);
            sentCount++;
            continue;
        }
        TrustAwareAODV *neighborAodv =
            dynamic_cast<TrustAwareAODV*>(neighborRouting);
        if (neighborAodv) {
            neighborAodv->receiveTrustBeacon(beacon);
            sentCount++;
            continue;
        }
        delete beacon;
    }

    // NUOVO: notifica overhead beacon al TrustMonitor
    // Dimensione stimata beacon: 12 byte payload + 28 byte UDP/IP header = 40 byte
    if (trustMonitor && sentCount > 0) {
        trustMonitor->onBeaconSent(sentCount * 40);
    }

    std::cout << "[TRUST_AODV] broadcastTrustBeacon: about=" << aboutNodeId
              << " trustValue=" << trustValue
              << " beacon inviati=" << sentCount << std::endl;
}
// ============================================================================
// RECEIVE TRUST BEACON
// ============================================================================
void TrustAwareAODV::receiveTrustBeacon(TrustBeacon *beacon) {
    if (!trustManager) {
        delete beacon;
        return;
    }

    std::cout << "[TRUST_AODV] receiveTrustBeacon: sender="
              << beacon->getSenderNodeId()
              << " about=" << beacon->getAboutNodeId()
              << " T_loc=" << beacon->getTrustValue() << std::endl;

    trustManager->updateFromDistributedInfo(
        beacon->getSenderNodeId(),
        beacon->getAboutNodeId(),
        beacon->getTrustValue());

    delete beacon;
}

// ============================================================================
// FINISH
// ============================================================================
void TrustAwareAODV::finish() {
    if (candidatesCleanupTimer) {
        if (candidatesCleanupTimer->isScheduled())
            cancelEvent(candidatesCleanupTimer);
        drop(candidatesCleanupTimer);
        delete candidatesCleanupTimer;
        candidatesCleanupTimer = nullptr;
    }

    std::cout << "[TRUST_AODV] finish: neighborCandidates per "
              << neighborCandidates.size() << " destinazioni" << std::endl;
}
