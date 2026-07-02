#include "AttackerTrustAwareAODV.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"

Define_Module(AttackerTrustAwareAODV);


// ============================================================================
// INITIALIZE
// ============================================================================
void AttackerTrustAwareAODV::initialize(int stage) {
    TrustAwareAODV::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        droppingRate = par("droppingRate").intValue();
        if (droppingRate < 0 || droppingRate > 100)
            throw cRuntimeError("droppingRate deve essere tra 0 e 100");

        totalPacketsReceived = 0;
        packetsDropped       = 0;
        packetsForwarded     = 0;

        std::cout << "[ATTACKER] Inizializzato nodo="
                  << getParentModule()->getIndex()
                  << " droppingRate=" << droppingRate << "%"
                  << std::endl;

        // ====================================================================
        // INIZIO STAMPA PER AUTOMAZIONE BASH (NODEID_MAP)
        // ====================================================================
        auto iface = interfaceTable->findInterfaceByName("wlan0");
        if (iface) {
            auto ipData = iface->getProtocolData<Ipv4InterfaceData>();
            if (ipData) {
                Ipv4Address myIp = ipData->getIPAddress();

                int ipId = myIp.getInt() & 0xFF;                   // ID usato dallo Scenario 4 (S4)
                int sumoId = getParentModule()->getIndex();        // ID base di SUMO (0-24)
                int omnetModuleId = getParentModule()->getId();    // Module ID usato dallo Scenario 6 (S6)

                // Stampa formattata ESATTAMENTE per i tuoi script grep/sort
                std::cout << "NODEID_MAP: sumoId=" << sumoId
                          << " ipId=" << ipId
                          << " moduleIndex=" << omnetModuleId
                          << std::endl;
            }
        }
        // ====================================================================
        // FINE STAMPA PER AUTOMAZIONE BASH
        // ====================================================================
    }
}

// ============================================================================
// SHOULD DROP PACKET
// ============================================================================
bool AttackerTrustAwareAODV::shouldDropPacket() {
    if (droppingRate == 0)   return false;
    if (droppingRate == 100) return true;

    int randomValue = intuniform(0, 99);
    return randomValue < droppingRate;
}

// ============================================================================
// CREATE FAKE RREP
// RREP con seqnum=999999 (sempre "migliore" per AODV standard)
// e hopCount=0 (sembra essere a 1 hop dalla destinazione)
// ============================================================================
const Ptr<Rrep> AttackerTrustAwareAODV::createFakeRREP(
    const Ptr<Rreq>& rreq,
    IRoute *destRoute,
    IRoute *originatorRoute,
    const L3Address& lastHopAddr)
{
    auto rrep = makeShared<Rrep>();
    rrep->setPacketType(usingIpv6 ? RREP_IPv6 : RREP);
    rrep->setChunkLength(usingIpv6 ? B(44) : B(20));

    rrep->setDestAddr(rreq->getDestAddr());
    rrep->setOriginatorAddr(rreq->getOriginatorAddr());
    rrep->setDestSeqNum(999999);
    rrep->setHopCount(0);
    rrep->setLifeTime(myRouteTimeout.trunc(SIMTIME_MS));

    std::cout << "[ATTACKER] createFakeRREP:"
              << " dest=" << rreq->getDestAddr()
              << " originator=" << rreq->getOriginatorAddr()
              << " seqNum=999999 hopCount=0 (FALSO)" << std::endl;

    return rrep;
}

// ============================================================================
// HANDLE RREQ
// Risponde sempre con un RREP falso invece di propagare la RREQ.
// ============================================================================
void AttackerTrustAwareAODV::handleRREQ(const Ptr<Rreq>& rreq,
                                         const L3Address& sourceAddr,
                                         unsigned int timeToLive) {
    auto blackListIt = blacklist.find(sourceAddr);
    if (blackListIt != blacklist.end())
        return;

    IRoute *previousHopRoute =
        routingTable->findBestMatchingRoute(sourceAddr);
    if (!previousHopRoute || previousHopRoute->getSource() != this) {
        previousHopRoute = createRoute(sourceAddr, sourceAddr, 1, false,
                                       rreq->getOriginatorSeqNum(), true,
                                       simTime() + activeRouteTimeout);
    } else {
        updateRoutingTable(previousHopRoute, sourceAddr, 1, false,
                           rreq->getOriginatorSeqNum(), true,
                           simTime() + activeRouteTimeout);
    }

    RreqIdentifier rreqIdentifier(rreq->getOriginatorAddr(),
                                   rreq->getRreqId());
    auto checkRREQ = rreqsArrivalTime.find(rreqIdentifier);
    if (checkRREQ != rreqsArrivalTime.end() &&
        simTime() - checkRREQ->second <= pathDiscoveryTime)
        return;

    rreqsArrivalTime[rreqIdentifier] = simTime();
    rreq->setHopCount(rreq->getHopCount() + 1);

    IRoute *reverseRoute =
        routingTable->findBestMatchingRoute(rreq->getOriginatorAddr());
    unsigned int hopCount = rreq->getHopCount();
    simtime_t minimalLifeTime = simTime() + 2 * netTraversalTime
                                - 2 * hopCount * nodeTraversalTime;
    simtime_t newLifeTime = std::max(simTime(), minimalLifeTime);
    int rreqSeqNum = rreq->getOriginatorSeqNum();

    if (!reverseRoute || reverseRoute->getSource() != this) {
        reverseRoute = createRoute(rreq->getOriginatorAddr(), sourceAddr,
                                   hopCount, true, rreqSeqNum, true,
                                   newLifeTime);
    } else {
        AodvRouteData *routeData =
            check_and_cast<AodvRouteData*>(reverseRoute->getProtocolData());
        int routeSeqNum   = routeData->getDestSeqNum();
        int newSeqNum     = std::max(routeSeqNum, rreqSeqNum);
        int newHopCount   = rreq->getHopCount();
        int routeHopCount = reverseRoute->getMetric();

        if (rreqSeqNum > routeSeqNum ||
            (rreqSeqNum == routeSeqNum && newHopCount < routeHopCount) ||
            rreq->getUnknownSeqNumFlag())
        {
            updateRoutingTable(reverseRoute, sourceAddr, hopCount, true,
                               newSeqNum, true, newLifeTime);
        }
    }

    IRoute *destRoute =
        routingTable->findBestMatchingRoute(rreq->getDestAddr());
    auto rrep = createFakeRREP(rreq, destRoute, reverseRoute, sourceAddr);
    sendRREP(rrep, rreq->getOriginatorAddr(), 255);

    std::cout << "[ATTACKER] handleRREQ: inviato RREP FALSO"
              << " dest=" << rreq->getDestAddr()
              << " verso=" << rreq->getOriginatorAddr() << std::endl;
}

// ============================================================================
// DATAGRAM FORWARD HOOK
//
// Struttura corretta:
// 1. Filtri leggeri (broadcast, AODV, proprio IP) → delega a TrustAwareAODV
// 2. Decidi drop/forward PRIMA di toccare globalPacketPredecessor
// 3. FORWARD → delega tutto a TrustAwareAODV (una sola chiamata)
// 4. DROP → gestisci predecessore e fake ACK direttamente
//
// NON chiamare Aodv::datagramForwardHook separatamente:
// nel path FORWARD lo fa TrustAwareAODV internamente,
// nel path DROP il pacchetto viene scartato e non serve.
// ============================================================================
INetfilter::IHook::Result AttackerTrustAwareAODV::datagramForwardHook(
    Packet *datagram)
{
    const auto& ipv4Header = datagram->peekAtFront<Ipv4Header>();
    if (!ipv4Header)
        return TrustAwareAODV::datagramForwardHook(datagram);

    // ---- Filtra pacchetti AODV (porta 654) ----
    if (ipv4Header->getProtocolId() == IP_PROT_UDP) {
        try {
            auto udpHdr = datagram->peekDataAt<UdpHeader>(
                ipv4Header->getChunkLength());
            if (udpHdr->getDestPort() == 654 ||
                udpHdr->getSourcePort() == 654)
                return TrustAwareAODV::datagramForwardHook(datagram);
        } catch (...) {
            return TrustAwareAODV::datagramForwardHook(datagram);
        }
    }

    // ---- Filtra broadcast/multicast ----
    L3Address destAddr = ipv4Header->getDestAddress();
    L3Address srcAddr  = ipv4Header->getSourceAddress();

    if (destAddr.isBroadcast() || destAddr.isMulticast())
        return TrustAwareAODV::datagramForwardHook(datagram);

    // ---- Filtra pacchetti propri ----
    auto iface = interfaceTable->findInterfaceByName("wlan0");
    if (!iface)
        return TrustAwareAODV::datagramForwardHook(datagram);

    Ipv4Address myIp = iface->getProtocolData<Ipv4InterfaceData>()
                           ->getIPAddress();
    if (srcAddr.toIpv4() == myIp)
        return TrustAwareAODV::datagramForwardHook(datagram);

    // ================================================================
    // È un pacchetto dati in transito: decidi PRIMA se droppare
    // ================================================================
    totalPacketsReceived++;

    if (!shouldDropPacket()) {
        // ---- FORWARD ----
        // Delega TUTTO a TrustAwareAODV in un'unica chiamata.
        // TrustAwareAODV gestirà internamente:
        //   - Aodv::datagramForwardHook (check rotta, RERR)
        //   - globalPacketPredecessor (predecessore corretto)
        //   - selectBestCandidate (score trust)
        //   - trustMonitor->onPacketForwarded (registrazione)
        packetsForwarded++;
        double dropRate = totalPacketsReceived > 0
            ? (packetsDropped * 100.0 / totalPacketsReceived) : 0;

        int pktId = getOrCreatePacketId(datagram);
        std::cout << "[ATTACKER] FORWARD packetId=" << pktId
                  << " totRicevuti=" << totalPacketsReceived
                  << " inoltrati=" << packetsForwarded
                  << " dropRate=" << dropRate << "%" << std::endl;

        return TrustAwareAODV::datagramForwardHook(datagram);
    }

    // ---- DROP ----
    // Gestisci predecessore e fake ACK direttamente.
    // Non chiamare Aodv::datagramForwardHook: il pacchetto viene
    // scartato, non serve aggiornare rotte o timer AODV.
    packetsDropped++;

    int packetId  = getOrCreatePacketId(datagram);
    int myNodeId  = myIp.getInt() & 0xFF;
    int srcNodeId = srcAddr.toIpv4().getInt() & 0xFF;

    // Recupera il vero predecessore PRIMA di sovrascrivere
    int previousHopId = srcNodeId;
    auto predIt = globalPacketPredecessor.find(packetId);
    if (predIt != globalPacketPredecessor.end())
        previousHopId = predIt->second;

    // Segna l'attaccante nella catena (per coerenza)
    globalPacketPredecessor[packetId] = myNodeId;

    double dropRate = packetsDropped * 100.0 / totalPacketsReceived;
    std::cout << "[ATTACKER] DROP packetId=" << packetId
              << " totRicevuti=" << totalPacketsReceived
              << " droppati=" << packetsDropped
              << " dropRate=" << dropRate << "%"
              << " → ACK FALSO MIC=0 verso predecessore="
              << previousHopId << std::endl;

    // Genera ACK falso con MIC=0 e propaga a ritroso
    if (previousHopId > 0) {
        IntermediateAck *fakeAck = new IntermediateAck("FakeAck");
        fakeAck->setOriginalPacketId(packetId);
        fakeAck->setMic(false);
        fakeAck->setSenderNodeId(myNodeId);
        fakeAck->setPreviousHopNodeId(previousHopId);
        fakeAck->setTimestamp(simTime().dbl());

        cModule *prevRouting = getSafeRoutingModule(previousHopId);
        if (prevRouting) {
            AttackerTrustAwareAODV *prevAttacker =
                dynamic_cast<AttackerTrustAwareAODV*>(prevRouting);
            if (prevAttacker) {
                prevAttacker->onFinalAckReceived(fakeAck);
            } else {
                TrustAwareAODV *prevAodv =
                    dynamic_cast<TrustAwareAODV*>(prevRouting);
                if (prevAodv) {
                    prevAodv->onFinalAckReceived(fakeAck);
                } else {
                    delete fakeAck;
                }
            }
        } else {
            std::cout << "[ATTACKER] predecessore=" << previousHopId
                      << " non raggiungibile, ACK falso non inviato"
                      << std::endl;
            delete fakeAck;
        }
    }

    return DROP;
}

// ============================================================================
// FINISH
// ============================================================================
void AttackerTrustAwareAODV::finish() {
    TrustAwareAODV::finish();

    double dropRate = totalPacketsReceived > 0
        ? (packetsDropped * 100.0 / totalPacketsReceived) : 0;

    std::cout << "[ATTACKER] ========================================"
              << std::endl;
    std::cout << "[ATTACKER] STATISTICHE FINALI" << std::endl;
    std::cout << "[ATTACKER] Ricevuti:  " << totalPacketsReceived << std::endl;
    std::cout << "[ATTACKER] Inoltrati: " << packetsForwarded << std::endl;
    std::cout << "[ATTACKER] Droppati:  " << packetsDropped << std::endl;
    std::cout << "[ATTACKER] Drop rate effettiva: " << dropRate << "%"
              << std::endl;
    std::cout << "[ATTACKER] Drop rate configurata: " << droppingRate << "%"
              << std::endl;
    std::cout << "[ATTACKER] ========================================"
              << std::endl;
}
