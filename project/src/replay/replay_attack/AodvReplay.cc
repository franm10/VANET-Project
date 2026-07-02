#include "AodvReplay.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address_m.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/Ptr.h"
#include "inet/common/Units.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/physicallayer/unitdisk/UnitDiskPhyHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

Define_Module(AodvReplay);

AodvReplay::AodvReplay(){}

AodvReplay::~AodvReplay()
{
    cancelAndDelete(evStart);
    cancelAndDelete(evForward);
    cancelAndDelete(evPureReplay);
}

void AodvReplay::initialize(int stage)
{
    Aodv::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        cModule* host = getContainingNode(this);
        int idx = host ? host->getIndex() : 0;
        nodeId = "node" + std::to_string(idx);

        EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
        EV_WARN << "[REPLAY-ATTACKER]    Nodo attaccante inizializzato\n";
        EV_WARN << "[REPLAY-ATTACKER]    Node ID: " << nodeId << "\n";
        EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";

        /* read parameters */
        parStart      = par("replayStart");
        EV_WARN << "[REPLAY-ATTACKER] Attacco partirà a t=" << simTime() + parStart << "s\n";

        parLatency    = par("replayLatency");
        EV_WARN << "[REPLAY-ATTACKER] Ritardo tra replay: " << parLatency << "s\n";

        parWindow     = par("bufferWindow");
        EV_WARN << "[REPLAY-ATTACKER]   Finestra buffer: " << parWindow << "s\n";

        parTargetIpv4 = par("targetIpv4");

        enableForgery = par("enableForgery");
        enablePureReplay = par("enablePureReplay");
        pureReplayDelay = par("pureReplayDelay");

        EV_WARN << "[REPLAY-ATTACKER]   Forgery attack: "
                    << (enableForgery ? "ENABLED" : "DISABLED") << "\n";
        EV_WARN << "[REPLAY-ATTACKER]   Pure replay attack: "
                    << (enablePureReplay ? "ENABLED" : "DISABLED") << "\n";
        if (enablePureReplay) {
            EV_WARN << "[REPLAY-ATTACKER]   Pure replay delay: "
                        << pureReplayDelay << "s\n";
        }

        evStart   = new cMessage("replayStart");
        evForward = new cMessage("replayForward");
        evPureReplay = new cMessage("pureReplay");

        if (auto udp = host ? host->getSubmodule("udp") : nullptr) {
            udp->subscribe("packetSentToUpper", this);
            EV_WARN << "[REPLAY-ATTACKER] Tap installato su UDP\n";
        }
    }
    else if(stage == INITSTAGE_APPLICATION_LAYER) {
        L3Address myAddr = getSelfIPAddress();
        EV_WARN << "[REPLAY-ATTACKER] NODEID_MAP nodeId=" << nodeId
                   << " ip=" << myAddr << "\n";
        scheduleAt(simTime() + parStart, evStart);
        EV_WARN << "[REPLAY-ATTACKER] Attack scheduled at t=" << simTime()+parStart << "\n";
    }
}

void AodvReplay::handleMessageWhenUp(cMessage *msg)
{
    Enter_Method_Silent();

    if (msg == evStart){
        EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
        EV_WARN << "[REPLAY-ATTACKER]    ATTACCO INIZIATO a t=" << simTime() << "s\n";
        EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
        startAttack();
        return;
    }

    if (msg == evForward){
        EV_WARN << "[REPLAY-ATTACKER] Reiniezione pacchetto dal buffer\n";
        forwardOne();
        return;
    }

    if (msg == evPureReplay){
        EV_WARN << "[REPLAY-ATTACKER] Invio pacchetto PureReplay \n";
        sendingPureReplay();
        return;
    }

    Aodv::handleMessageWhenUp(msg);
}

void AodvReplay::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    auto pk = dynamic_cast<Packet*>(obj);
    if (!pk) return;

    auto udpTag = pk->findTag<TransportProtocolInd>();
    if (!udpTag || udpTag->getProtocol() != &Protocol::udp) return;

    Ptr<const Chunk> body = pk->peekData();

    if (!interesting(body)) return;
    // FIX: scarta RREP con indirizzi non valorizzati (Hello messages)
        if (auto rrep = dynamicPtrCast<const Rrep>(body)) {
            if (rrep->getOriginatorAddr().isUnspecified() ||
                rrep->getDestAddr().isUnspecified()) {
                EV_WARN << "[REPLAY-ATTACKER] RREP Hello ignorato (indirizzo non valorizzato)\n";
                return;
            }
        }

    buf.push_back({simTime(), body->dupShared()});

    EV_WARN << "[REPLAY-ATTACKER]   Pacchetto AODV catturato\n";
    EV_WARN << "[REPLAY-ATTACKER]   Tipo: " << body->getClassName() << "\n";
    EV_WARN << "[REPLAY-ATTACKER]   Buffer size: " << buf.size() << "\n";
    EV_WARN << "[REPLAY-ATTACKER]   Timestamp: " << simTime() << "s\n";

    purgeOld();

    if (active && buf.size() == 1){
        EV_WARN << "[REPLAY-ATTACKER] Primo pacchetto: reiniezione immediata\n";
        scheduleAt(simTime(), evForward);
    }
}

bool AodvReplay::interesting(const Ptr<const Chunk>& c) const
{
    if (!dynamicPtrCast<const Rreq>(c) && !dynamicPtrCast<const Rrep>(c)){
        return false;
    }

    if (parTargetIpv4 < 0){
        return true;
    }

    if (auto rreq = dynamicPtrCast<const Rreq>(c)){
        bool match = rreq->getOriginatorAddr().toIpv4().getInt() == parTargetIpv4 ||
                     rreq->getDestAddr().toIpv4().getInt() == parTargetIpv4;
        return match;
    }

    if (auto rrep = dynamicPtrCast<const Rrep>(c)){
        bool match = rrep->getOriginatorAddr().toIpv4().getInt() == parTargetIpv4 ||
                     rrep->getDestAddr().toIpv4().getInt() == parTargetIpv4;
        return match;
    }

    return false;
}

void AodvReplay::startAttack()
{
    EV_WARN << "[REPLAY-ATTACKER] Modalità replay ATTIVA\n";
    EV_WARN << "[REPLAY-ATTACKER] Buffer corrente: " << buf.size() << " pacchetti\n";

    active = true;

    if (!buf.empty()){
        EV_WARN << "[REPLAY-ATTACKER] Avvio reiniezione immediata\n";
        scheduleAt(simTime(), evForward);
    }
}

void AodvReplay::forwardOne()
{
    Enter_Method_Silent();
    purgeOld();

    if (buf.empty()) {
        EV_WARN << "[REPLAY-ATTACKER] Buffer vuoto, nessun replay\n";
        return;
    }

    auto entry = buf.front();
    buf.pop_front();

    // --- RREQ REPLAY ---
    if (auto oldRreq = dynamicPtrCast<const Rreq>(entry.aodv)) {
        if( enableForgery ) {
            auto rreqForgery = makeShared<Rreq>(*oldRreq);

            int oldHop = oldRreq->getHopCount();
            int oldSeq = oldRreq->getOriginatorSeqNum();
            int oldId  = oldRreq->getRreqId();

            // Modifica parametri per replay
            rreqForgery->setHopCount(0);
            rreqForgery->setOriginatorSeqNum(oldSeq + 1);
            rreqForgery->setRreqId(oldId + 1);
            rreqForgery->setSenderId(nodeId.c_str());

            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
            EV_WARN << "[REPLAY-ATTACKER]   FORGERY ATTACK - Modifica in corso\n";
            EV_WARN << "[REPLAY-ATTACKER]   Sender ID: " << nodeId << " (ATTACKER)\n";
            EV_WARN << "[REPLAY-ATTACKER]   Originator: " << rreqForgery->getOriginatorAddr() << "\n";
            EV_WARN << "[REPLAY-ATTACKER]   Hop: " << oldHop << " → 0\n";
            EV_WARN << "[REPLAY-ATTACKER]   Seq: " << oldSeq << " → " << rreqForgery->getOriginatorSeqNum() << "\n";
            EV_WARN << "[REPLAY-ATTACKER]   RREQ ID: " << oldId << " → " << rreqForgery->getRreqId() << "\n";
            EV_WARN << "[REPLAY-ATTACKER]   FIRMA ORIGINALE MANTENUTA (INVALIDA!)\n";
            EV_WARN << "[REPLAY-ATTACKER]   Expected: DROP da verifica firma\n";

            unsigned int ttl = (unsigned int) std::max(1, (int)netDiameter);
            EV_WARN << "[REPLAY-ATTACKER]   TTL: " << ttl << "\n";

            pendingPureReplayQueue.push_back(rreqForgery->dupShared());
            if (!evPureReplay->isScheduled()) {
                scheduleAt(simTime() + uniform(0.001, 0.005), evPureReplay);
            }
            EV_WARN << "[REPLAY-ATTACKER]   FORGERY RREQ schedulato con jitter\n";

            EV_WARN << "[REPLAY-ATTACKER]   FORGERY RREQ trasmesso in broadcast\n";
            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
        }
        if( enablePureReplay ) {
            auto rreqPure = makeShared<Rreq>(*oldRreq);

            // NO MODIFICHE ai parametri critici
            // Mantiene: seq, hop, rreqId, signature ORIGINALI

            // OPZIONALE: Cambia solo senderId per logging
            rreqPure->setSenderId(nodeId.c_str());

            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
            EV_WARN << "[REPLAY-ATTACKER]   PURE REPLAY ATTACK (no modifiche)\n";
            EV_WARN << "[REPLAY-ATTACKER]      Sender ID: " << nodeId << " (ATTACKER)\n";
            EV_WARN << "[REPLAY-ATTACKER]      Originator: " << rreqPure->getOriginatorAddr() << "\n";
            EV_WARN << "[REPLAY-ATTACKER]      Seq: " << rreqPure->getOriginatorSeqNum() << " (INVARIATO)\n";
            EV_WARN << "[REPLAY-ATTACKER]      Hop: " << rreqPure->getHopCount() << " (INVARIATO)\n";
            EV_WARN << "[REPLAY-ATTACKER]      RREQ ID: " << rreqPure->getRreqId() << " (INVARIATO)\n";
            EV_WARN << "[REPLAY-ATTACKER]   FIRMA ORIGINALE (VALIDA!)\n";
            EV_WARN << "[REPLAY-ATTACKER]   Expected: PASS firma, DROP anti-replay\n";

            unsigned int ttl = (unsigned int) std::max(1, (int)netDiameter);

            // RITARDO per evitare collisione con forgery
            pendingPureReplayQueue.push_back(rreqPure->dupShared());
            if (!evPureReplay->isScheduled()) {
                scheduleAt(simTime() + pureReplayDelay, evPureReplay);
            }
            /*
            scheduleAfter(pureReplayDelay, [this, rreqPure, ttl]() {
                sendRREQ(rreqPure, addressType->getBroadcastAddress(), ttl);
                EV_WARN << "[REPLAY-ATTACKER]   ✅ Pure replay RREQ trasmesso\n";
            });
            */

            EV_WARN << "[REPLAY-ATTACKER]   Invio schedulato tra " << pureReplayDelay << "s\n";
            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
        }
    }
    // --- RREP REPLAY ---
    else if (auto oldRrep = dynamicPtrCast<const Rrep>(entry.aodv)) {
        if (oldRrep->getOriginatorAddr().isUnspecified() ||
                oldRrep->getDestAddr().isUnspecified()) {
                EV_WARN << "[REPLAY-ATTACKER] RREP con indirizzi non valorizzati, scartato\n";
                if (!buf.empty() && active) {
                    scheduleAt(simTime() + parLatency, evForward);
                }
                return;
            }

        if( enableForgery ) {
            auto rrepForgery = makeShared<Rrep>(*oldRrep);

            int oldHop = oldRrep->getHopCount();
            int oldSeq = oldRrep->getDestSeqNum();

            rrepForgery->setHopCount(0);
            rrepForgery->setDestSeqNum(oldSeq + 1);
            rrepForgery->setSenderId(nodeId.c_str());

            bool gRrep = rrepForgery->getDestAddr().isUnspecified();
            if (gRrep) {
                rrepForgery->setDestAddr(oldRrep->getOriginatorAddr());
            }

            L3Address dst = oldRrep->getOriginatorAddr();

            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
            EV_WARN << "[REPLAY-ATTACKER]   RREP REPLAY - Modifica in corso\n";
            EV_WARN << "[REPLAY-ATTACKER]      Sender ID: " << nodeId << " (ATTACKER)\n";
            EV_WARN << "[REPLAY-ATTACKER]      Destination: " << dst << "\n";
            EV_WARN << "[REPLAY-ATTACKER]      Hop: " << oldHop << " → 0\n";
            EV_WARN << "[REPLAY-ATTACKER]      Dest Seq: " << oldSeq << " → " << rrepForgery->getDestSeqNum() << "\n";
            EV_WARN << "[REPLAY-ATTACKER]   FIRMA ORIGINALE MANTENUTA (INVALIDA!)\n";
            EV_WARN << "[REPLAY-ATTACKER]   Expected: DROP da firma\n";

            inet::InterfaceEntry *ie = nullptr;
            for (int i = 0; i < interfaceTable->getNumInterfaces(); ++i) {
                auto cand = interfaceTable->getInterface(i);
                if (cand && !cand->isLoopback()) { ie = cand; break; }
            }
            int ifId = ie ? ie->getInterfaceId() : -1;
            unsigned int ttl = (unsigned int) std::max(1, (int)netDiameter);

            EV_WARN << "[REPLAY-ATTACKER]   TTL: " << ttl << ", Interface ID: " << ifId << "\n";

            pendingPureReplayQueue.push_back(rrepForgery->dupShared());
            if (!evPureReplay->isScheduled()) {
                scheduleAt(simTime() + uniform(0.001, 0.005), evPureReplay);
            }
            EV_WARN << "[REPLAY-ATTACKER]   FORGERY RREP schedulato con jitter\n";

            EV_WARN << "[REPLAY-ATTACKER]   FORGERY RREP trasmesso a " << dst << "\n";
            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
        }
        if (enablePureReplay) {
            auto rrepPure = makeShared<Rrep>(*oldRrep);

            // ✅ NO MODIFICHE
            rrepPure->setSenderId(nodeId.c_str());

            L3Address dst = oldRrep->getOriginatorAddr();

            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
            EV_WARN << "[REPLAY-ATTACKER] 🟡 PURE REPLAY ATTACK (RREP)\n";
            EV_WARN << "[REPLAY-ATTACKER]   Destination: " << dst << "\n";
            EV_WARN << "[REPLAY-ATTACKER]   Seq: " << rrepPure->getDestSeqNum() << " (INVARIATO)\n";
            EV_WARN << "[REPLAY-ATTACKER]   Expected: ✅ PASS firma, ❌ DROP anti-replay\n";

            inet::InterfaceEntry *ie = nullptr;
            for (int i = 0; i < interfaceTable->getNumInterfaces(); ++i) {
                auto cand = interfaceTable->getInterface(i);
                if (cand && !cand->isLoopback()) { ie = cand; break; }
            }
            int ifId = ie ? ie->getInterfaceId() : -1;
            unsigned int ttl = (unsigned int) std::max(1, (int)netDiameter);

            // RITARDO per evitare collisione con forgery
            pendingPureReplayQueue.push_back(rrepPure->dupShared());
            if (!evPureReplay->isScheduled()) {
                scheduleAt(simTime() + pureReplayDelay, evPureReplay);
            }
            /*
            scheduleAfter(pureReplayDelay, [this, rreqPure, ttl]() {
                sendRREQ(rreqPure, addressType->getBroadcastAddress(), ttl);
                EV_WARN << "[REPLAY-ATTACKER]   ✅ Pure replay RREQ trasmesso\n";
            });
            */

            EV_WARN << "[REPLAY-ATTACKER]     Invio schedulato tra " << pureReplayDelay << "s\n";
            EV_WARN << "[REPLAY-ATTACKER] ════════════════════════════════════\n";
        }
    }
    else {
        EV_WARN << "[REPLAY-ATTACKER] Chunk sconosciuto nel buffer, scartato\n";
    }

    // Schedule next replay
    if (!buf.empty() && active) {
        scheduleAt(simTime() + parLatency + uniform(0.0, 0.005), evForward);
    }
}

void AodvReplay::purgeOld()
{
    int removed = 0;
    while (!buf.empty() && simTime() - buf.front().ts > parWindow){
        buf.pop_front();
        removed++;
    }

    if (removed > 0) {
        EV_WARN << "[REPLAY-ATTACKER] Rimossi " << removed
                << " pacchetti vecchi (buffer=" << buf.size() << ")\n";
    }
}

void AodvReplay::sendingPureReplay()
{
    if (pendingPureReplayQueue.empty()) {
        EV_WARN << "[REPLAY-ATTACKER] Nessun pacchetto in attesa per pure replay\n";
        return;
    }

    auto pendingPureReplay = pendingPureReplayQueue.front();
    pendingPureReplayQueue.pop_front();

    if (auto rreq = dynamicPtrCast<Rreq>(pendingPureReplay)) {
        unsigned int ttl = (unsigned int) std::max(1, (int)netDiameter);
        sendRREQ(rreq, addressType->getBroadcastAddress(), ttl);
        EV_WARN << "[REPLAY-ATTACKER]    Pure replay RREQ trasmesso\n";
    }
    else if (auto rrep = dynamicPtrCast<Rrep>(pendingPureReplay)) {
        if (rrep->getOriginatorAddr().isUnspecified() ||
               rrep->getDestAddr().isUnspecified()) {
               EV_WARN << "[REPLAY-ATTACKER] RREP Hello scartato (indirizzi non valorizzati)\n";
               // Schedula il prossimo se ci sono altri in coda
               if (!pendingPureReplayQueue.empty()) {
                   scheduleAt(simTime() + pureReplayDelay, evPureReplay);
               }
               return;
           }
        L3Address dst = rrep->getOriginatorAddr();
        inet::InterfaceEntry *ie = nullptr;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); ++i) {
            auto cand = interfaceTable->getInterface(i);
            if (cand && !cand->isLoopback()) { ie = cand; break; }
        }
        int ifId = ie ? ie->getInterfaceId() : -1;
        unsigned int ttl = (unsigned int) std::max(1, (int)netDiameter);

        sendAODVPacket(rrep, dst, ttl, ifId);
        EV_WARN << "[REPLAY-ATTACKER]    Pure replay RREP trasmesso\n";
    }

    // Se ci sono ancora pacchetti in coda, schedula il prossimo
    if (!pendingPureReplayQueue.empty()) {
        scheduleAt(simTime() + pureReplayDelay, evPureReplay);
    }
}

void AodvReplay::handleRREP(const Ptr<Rrep>& rrep,
                              const L3Address& sourceAddr)
{
    // Scarta RREP malformati prima che la classe base tenti
    // di aggiungere route con indirizzi non valorizzati
    if (rrep->getOriginatorAddr().isUnspecified() ||
        rrep->getDestAddr().isUnspecified()       ||
        sourceAddr.isUnspecified()) {
        EV_WARN << "[REPLAY-ATTACKER] RREP malformato ignorato "
                << "(addr non valorizzato)\n";
        return;
    }
    Aodv::handleRREP(rrep, sourceAddr);
}

void AodvReplay::sendImmediateAck(const Ipv4Address& dst)
{
    auto ack = createRREPACK();
    sendRREPACK(ack, dst);
    EV_WARN << "[REPLAY-ATTACKER] RREP-ACK inviato a " << dst << "\n";
}
