//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "AodvGrayhole.h"
#include "inet/networklayer/common/L3Tools.h"

using namespace inet;
using namespace aodv;

Define_Module(AodvGrayhole);

AodvGrayhole::AodvGrayhole() {
    totalPacketsReceived = 0;
    packetsDropped = 0;
    packetsForwarded = 0;
}


void AodvGrayhole::initialize(int stage)
{
    Aodv::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        droppingRate = par("droppingRate");
        if (droppingRate < 0 || droppingRate > 100) {
            throw cRuntimeError("Il parametro droppingRate deve essere compreso tra 0 e 100");
        }
        totalPacketsReceived = 0;
        packetsDropped = 0;
        packetsForwarded = 0;
    }
    // Sposta il NODEID_MAP a uno stage più tardo
    if (stage == INITSTAGE_APPLICATION_LAYER) {
        auto iface = interfaceTable->findInterfaceByName("wlan0");
        if (iface) {
            int myIpNodeId = iface->getProtocolData<inet::Ipv4InterfaceData>()
                                 ->getIPAddress().getInt() & 0xFF;
            EV_WARN << "[NODEID_MAP] modulo=" << getParentModule()->getFullName()
                    << " ipNodeId=" << myIpNodeId << endl;
        }
    }
}

void AodvGrayhole::printRoutingTable()
{   /*
    EV_WARN << "===============================================" << endl;
    EV_WARN << "[GRAYHOLE] TABELLA DI ROUTING AODV" << endl;
    EV_WARN << "[GRAYHOLE] Numero di rotte: " << routingTable->getNumRoutes() << endl;
    EV_WARN << "-----------------------------------------------" << endl;
*/
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);

        // Verifica se la rotta è gestita da AODV
        if (route->getSource() == this) {
            AodvRouteData *routeData = dynamic_cast<AodvRouteData *>(route->getProtocolData());
            /*
            EV_WARN << "[GRAYHOLE] Rotta #" << i << ":" << endl;
            EV_WARN << "[GRAYHOLE]   Destinazione: " << route->getDestinationAsGeneric() << endl;
            EV_WARN << "[GRAYHOLE]   Next hop: " << route->getNextHopAsGeneric() << endl;
            EV_WARN << "[GRAYHOLE]   Interfaccia: " << route->getInterface()->getInterfaceName() << endl;
            EV_WARN << "[GRAYHOLE]   Metrica (hop count): " << route->getMetric() << endl;
            */
            if (routeData) {
                /*
                EV_WARN << "[GRAYHOLE]   Sequence number: " << routeData->getDestSeqNum() << endl;
                EV_WARN << "[GRAYHOLE]   SeqNum valido: " << (routeData->hasValidDestNum() ? "SI" : "NO") << endl;
                EV_WARN << "[GRAYHOLE]   Rotta attiva: " << (routeData->isActive() ? "SI" : "NO") << endl;
                EV_WARN << "[GRAYHOLE]   Lifetime: " << routeData->getLifeTime() << endl;
                */
            }
            //EV_WARN << "-----------------------------------------------" << endl;
        }
    }
    //EV_WARN << "===============================================" << endl;
}

bool AodvGrayhole::shouldDropPacket()
{
    if (droppingRate == 0)
        return false;

    if (droppingRate == 100)
        return true;

    int randomValue = intuniform(0, 99);
    bool shouldDrop = randomValue < droppingRate;

    //EV_DEBUG << "[GRAYHOLE] Decisione: random=" << randomValue << ", soglia=" << droppingRate << ", risultato=" << (shouldDrop ? "SCARTA" : "INOLTRA") << endl;

    return shouldDrop;
}

AodvGrayhole::Result AodvGrayhole::datagramForwardHook(Packet *datagram)
{
    //EV_WARN << "###############################################" << endl;
    //EV_WARN << "[GRAYHOLE] datagramForwardHook CHIAMATO!" << endl;
    //EV_WARN << "[GRAYHOLE] Nome pacchetto: " << datagram->getName() << endl;

    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    const L3Address& sourceAddr = networkHeader->getSourceAddress();

    //EV_WARN << "[GRAYHOLE] Sorgente: " << sourceAddr << endl;
    //EV_WARN << "[GRAYHOLE] Destinazione: " << destAddr << endl;

    // Cerca la rotta per questa destinazione nella tabella
    IRoute *route = routingTable->findBestMatchingRoute(destAddr);
    if (route) {
        //EV_WARN << "[GRAYHOLE] Rotta trovata nella tabella:" << endl;
        //EV_WARN << "[GRAYHOLE]   Next hop: " << route->getNextHopAsGeneric() << endl;
        //EV_WARN << "[GRAYHOLE]   Interfaccia: " << route->getInterface()->getInterfaceName() << endl;
        //EV_WARN << "[GRAYHOLE]   Metrica: " << route->getMetric() << endl;

        AodvRouteData *routeData = dynamic_cast<AodvRouteData *>(route->getProtocolData());
        if (routeData) {
            //EV_WARN << "[GRAYHOLE]   Sequence number: " << routeData->getDestSeqNum() << endl;
            //EV_WARN << "[GRAYHOLE]   Attiva: " << (routeData->isActive() ? "SI" : "NO") << endl;
        }
    } else {
        //EV_WARN << "[GRAYHOLE] ATTENZIONE: Nessuna rotta trovata per " << destAddr << endl;
    }

    // Prima chiama il metodo base per gestire AODV normalmente
    Result baseResult = Aodv::datagramForwardHook(datagram);

    //EV_WARN << "[GRAYHOLE] Risultato AODV base: " << (baseResult == ACCEPT ? "ACCEPT" : baseResult == DROP ? "DROP" : baseResult == QUEUE ? "QUEUE" : "STOLEN") << endl;

    // Se AODV ha già deciso di droppare o mettere in coda, rispetta quella decisione
    if (baseResult != ACCEPT) {
        //EV_WARN << "[GRAYHOLE] AODV ha gestito il pacchetto, non applico attacco" << endl;
        //EV_WARN << "###############################################" << endl;
        return baseResult;
    }

    // Ora il pacchetto dovrebbe essere inoltrato - qui applichiamo l'attacco grayhole
    totalPacketsReceived++;

    if (shouldDropPacket()) {
        packetsDropped++;
        double currentDropRate = packetsDropped * 100.0 / totalPacketsReceived;
/*
        EV_WARN << "***********************************************" << endl;
        EV_WARN << "[GRAYHOLE] ATTACCO: PACCHETTO SCARTATO" << endl;
        EV_WARN << "[GRAYHOLE] -----------------------------------" << endl;
        EV_WARN << "[GRAYHOLE] Pacchetti totali: " << totalPacketsReceived << endl;
        EV_WARN << "[GRAYHOLE] Pacchetti inoltrati: " << packetsForwarded << endl;
        EV_WARN << "[GRAYHOLE] Pacchetti scartati: " << packetsDropped << endl;
        EV_WARN << "[GRAYHOLE] Percentuale attuale: " << currentDropRate << "%" << endl;
        EV_WARN << "[GRAYHOLE] Percentuale configurata: " << droppingRate << "%" << endl;
        EV_WARN << "***********************************************" << endl;
        EV_WARN << "###############################################" << endl;
*/
        return DROP;
    }
    else {
        packetsForwarded++;
        double currentDropRate = totalPacketsReceived > 0 ? (packetsDropped * 100.0 / totalPacketsReceived) : 0;
/*
        EV_WARN << "===============================================" << endl;
        EV_WARN << "[GRAYHOLE] PACCHETTO INOLTRATO" << endl;
        EV_WARN << "[GRAYHOLE] -----------------------------------" << endl;
        EV_WARN << "[GRAYHOLE] Pacchetti totali: " << totalPacketsReceived << endl;
        EV_WARN << "[GRAYHOLE] Pacchetti inoltrati: " << packetsForwarded << endl;
        EV_WARN << "[GRAYHOLE] Pacchetti scartati: " << packetsDropped << endl;
        EV_WARN << "[GRAYHOLE] Percentuale attuale: " << currentDropRate << "%" << endl;
        EV_WARN << "===============================================" << endl;
        EV_WARN << "###############################################" << endl;
*/
        return ACCEPT;
    }
}

void AodvGrayhole::handleRREQ(const Ptr<Rreq>& rreq, const L3Address& sourceAddr, unsigned int timeToLive)
{
    /*
    EV_WARN << "-----------------------------------------------" << endl;
    EV_WARN << "[GRAYHOLE] RICEZIONE ROUTE REQUEST (RREQ)" << endl;
    EV_WARN << "[GRAYHOLE] Sorgente: " << sourceAddr << endl;
    EV_WARN << "[GRAYHOLE] Originatore: " << rreq->getOriginatorAddr() << endl;
    EV_WARN << "[GRAYHOLE] Destinazione: " << rreq->getDestAddr() << endl;
    EV_WARN << "[GRAYHOLE] Hop count: " << rreq->getHopCount() << endl;
    EV_WARN << "[GRAYHOLE] RREQ ID: " << rreq->getRreqId() << endl;
    EV_WARN << "-----------------------------------------------" << endl;
*/
    auto blackListIt = blacklist.find(sourceAddr);
    if (blackListIt != blacklist.end()) {
       // EV_WARN << "[GRAYHOLE] RREQ ignorato (blacklist)" << endl;
        return;
    }

    IRoute *previousHopRoute = routingTable->findBestMatchingRoute(sourceAddr);

    if (!previousHopRoute || previousHopRoute->getSource() != this) {
        previousHopRoute = createRoute(sourceAddr, sourceAddr, 1, false, rreq->getOriginatorSeqNum(), true, simTime() + activeRouteTimeout);
        //EV_WARN << "[GRAYHOLE] Creata rotta verso hop precedente: " << sourceAddr << endl;
    }
    else {
        updateRoutingTable(previousHopRoute, sourceAddr, 1, false, rreq->getOriginatorSeqNum(), true, simTime() + activeRouteTimeout);
        //EV_WARN << "[GRAYHOLE] Aggiornata rotta verso hop precedente: " << sourceAddr << endl;
    }

    RreqIdentifier rreqIdentifier(rreq->getOriginatorAddr(), rreq->getRreqId());
    auto checkRREQArrivalTime = rreqsArrivalTime.find(rreqIdentifier);
    if (checkRREQArrivalTime != rreqsArrivalTime.end() && simTime() - checkRREQArrivalTime->second <= pathDiscoveryTime) {
        //EV_WARN << "[GRAYHOLE] RREQ duplicato, scarto" << endl;
        return;
    }

    rreqsArrivalTime[rreqIdentifier] = simTime();
    rreq->setHopCount(rreq->getHopCount() + 1);

    IRoute *reverseRoute = routingTable->findBestMatchingRoute(rreq->getOriginatorAddr());
    unsigned int hopCount = rreq->getHopCount();
    simtime_t minimalLifeTime = simTime() + 2 * netTraversalTime - 2 * hopCount * nodeTraversalTime;
    simtime_t newLifeTime = std::max(simTime(), minimalLifeTime);
    int rreqSeqNum = rreq->getOriginatorSeqNum();

    if (!reverseRoute || reverseRoute->getSource() != this) {
        reverseRoute = createRoute(rreq->getOriginatorAddr(), sourceAddr, hopCount, true, rreqSeqNum, true, newLifeTime);
        //EV_WARN << "[GRAYHOLE] Creata rotta di ritorno verso: " << rreq->getOriginatorAddr() << endl;
        //EV_WARN << "[GRAYHOLE]   Next hop: " << sourceAddr << endl;
        //EV_WARN << "[GRAYHOLE]   Hop count: " << hopCount << endl;
        //EV_WARN << "[GRAYHOLE]   SeqNum: " << rreqSeqNum << endl;
    }
    else {
        AodvRouteData *routeData = check_and_cast<AodvRouteData *>(reverseRoute->getProtocolData());
        int routeSeqNum = routeData->getDestSeqNum();
        int newSeqNum = std::max(routeSeqNum, rreqSeqNum);
        int newHopCount = rreq->getHopCount();
        int routeHopCount = reverseRoute->getMetric();

        if (rreqSeqNum > routeSeqNum ||
            (rreqSeqNum == routeSeqNum && newHopCount < routeHopCount) ||
            rreq->getUnknownSeqNumFlag())
        {
            updateRoutingTable(reverseRoute, sourceAddr, hopCount, true, newSeqNum, true, newLifeTime);
            //EV_WARN << "[GRAYHOLE] Aggiornata rotta di ritorno verso: " << rreq->getOriginatorAddr() << endl;
        } else {
            //EV_WARN << "[GRAYHOLE] Rotta di ritorno esistente e' migliore, non aggiorno" << endl;
        }
    }

    IRoute *destRoute = routingTable->findBestMatchingRoute(rreq->getDestAddr());
/*
    EV_WARN << "===============================================" << endl;
    EV_WARN << "[GRAYHOLE] ATTACCO: Generazione RREP FALSO" << endl;
    EV_WARN << "[GRAYHOLE] Per destinazione: " << rreq->getDestAddr() << endl;
    EV_WARN << "[GRAYHOLE] Verso originatore: " << rreq->getOriginatorAddr() << endl;
*/
    auto rrep = createFakeRREP(rreq, destRoute, reverseRoute, sourceAddr);
/*
    EV_WARN << "[GRAYHOLE] RREP falso creato:" << endl;
    EV_WARN << "[GRAYHOLE]   SeqNum (falso): " << rrep->getDestSeqNum() << endl;
    EV_WARN << "[GRAYHOLE]   Hop count (falso): " << rrep->getHopCount() << endl;
    EV_WARN << "[GRAYHOLE] L'originatore pensera' di avere la rotta migliore attraverso questo nodo!" << endl;
    EV_WARN << "===============================================" << endl;
*/
    sendRREP(rrep, rreq->getOriginatorAddr(), 255);
/*
    EV_WARN << "[HOP] ATTACKER invia RREP falso:"
    << " dest=" << rreq->getDestAddr()
    << " verso=" << rreq->getOriginatorAddr()
    << " hopCount_dichiarato=" << rrep->getHopCount()
    << " seqNum_dichiarato=" << rrep->getDestSeqNum() << endl;


    // Stampa la routing table dopo aver processato il RREQ
    EV_WARN << "[GRAYHOLE] Routing table dopo RREQ:" << endl;
*/
    printRoutingTable();
}

const Ptr<Rrep> AodvGrayhole::createFakeRREP(const Ptr<Rreq>& rreq, IRoute *destRoute, IRoute *originatorRoute, const L3Address& lastHopAddr)
{
    auto rrep = makeShared<Rrep>();
    rrep->setPacketType(usingIpv6 ? RREP_IPv6 : RREP);
    rrep->setChunkLength(usingIpv6 ? B(44) : B(20));

    rrep->setDestAddr(rreq->getDestAddr());
    rrep->setOriginatorAddr(rreq->getOriginatorAddr());
    rrep->setDestSeqNum(999999);  // SeqNum altissimo per sembrare la rotta migliore
    rrep->setHopCount(0);          // Hop count basso per sembrare la rotta più corta
    rrep->setLifeTime(myRouteTimeout.trunc(SIMTIME_MS));

/*
    EV_WARN << "[OP] ATTACKER createFakeRREP:"
    << " dest=" << rrep->getDestAddr()
    << " hopCount=" << rrep->getHopCount()
    << " (reale sarebbe molto più alto)" << endl;
*/
    return rrep;
}
void AodvGrayhole::finish() {
    cSimpleModule::finish();

    double dropRate = totalPacketsReceived > 0 ?
        (packetsDropped * 100.0 / totalPacketsReceived) : 0;
/*
    EV_WARN << "===============================================" << endl;
    EV_WARN << "[GRAYHOLE] STATISTICHE FINALI" << endl;
    EV_WARN << "[GRAYHOLE] Ricevuti: " << totalPacketsReceived << endl;
    EV_WARN << "[GRAYHOLE] Inoltrati: " << packetsForwarded << endl;
    EV_WARN << "[GRAYHOLE] Scartati: " << packetsDropped << endl;
    EV_WARN << "[GRAYHOLE] Drop rate effettiva: " << dropRate << "%" << endl;
    EV_WARN << "[GRAYHOLE] Drop rate configurata: "
            << droppingRate << "%" << endl;
    EV_WARN << "===============================================" << endl;
*/
}

AodvGrayhole::~AodvGrayhole() {
    // Distruttore completamente vuoto
    // Tutta la logica è stata spostata in finish()
}
