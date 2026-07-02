//  RsaAodvReplay.cc
//  ───────────────────────────────────────────────────────────────
//  ‣ Estende il modulo INET AODV per aggiungere:
//    1) Firma RSA dei pacchetti RREQ/RREP (autenticità / integrità)
//    2) Filtro anti-replay: accetta solo seqNum strettamente crescenti
//  Compatibile con: OMNeT++ 5.6 / INET 4.2 / Veins 5.2
//  ----------------------------------------------------------------

#include "RsaAodvReplay.h"

#include "replay/replay_mitigation_rsa/utils/RsaUtils.h"
#include "replay/replay_mitigation_rsa/utils/json.hpp"
#include "inet/common/ModuleAccess.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

using namespace inet;
using namespace aodv;

Define_Module(RsaAodvReplay);

RsaAodvReplay::~RsaAodvReplay() {
    cancelAndDelete(evGarbageCollection);
}

/* ***************************************************************** */
/*   1. initialize()  –  carica parametri, chiavi pubbliche, ecc.    */
/* ***************************************************************** */

void RsaAodvReplay::initialize(int stage)
{
    Aodv::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // Legge parametri
        nodeId          = par("nodeId").stdstringValue();
        myPrivPem       = par("privateKeyPem").stdstringValue();
        freshnessWindow = par("freshnessWindow");

        // Carica mappa chiavi pubbliche da JSON
        std::string mapPath = par("pubKeyMapJson").stdstringValue();
        std::ifstream fin(mapPath);
        if (!fin.is_open())
            throw cRuntimeError("Cannot open pubKey map %s", mapPath.c_str());

        nlohmann::json j;
        fin >> j;

        for (auto& kv : j.items()) {
            std::string sender  = kv.key();
            std::string pemPath = kv.value();

            std::ifstream pemFile(pemPath.c_str());
            if (!pemFile)
                throw cRuntimeError("Cannot open PEM %s", pemPath.c_str());

            std::stringstream ss;
            ss << pemFile.rdbuf();
            pubKeyMap[sender] = ss.str();
        }

        // Auto-detect nodeId e private key
        cModule *host = getContainingNode(this);
        if (!host) host = getParentModule();
        int idx = host ? host->getIndex() : 0;

        if (nodeId.empty() || nodeId == "nodeX") {
            nodeId = "node" + std::to_string(idx);
        }

        if (myPrivPem.empty()) {
            myPrivPem = "/home/giada/NetworkProjectVanet/src/replay/replay_mitigation_rsa/keys/node" + std::to_string(idx) + "_priv_rsa.pem";
        }

        if (myPrivPem.find("-----BEGIN") == std::string::npos) {
            std::ifstream pk(myPrivPem);
            if (!pk.is_open())
                throw cRuntimeError("Cannot open private key %s", myPrivPem.c_str());

            std::stringstream ss;
            ss << pk.rdbuf();
            myPrivPem = ss.str();
        }

        entryTimeout = par("entryTimeout");
        gcInterval = par("gcInterval");
        maxCacheSize = par("maxCacheSize");

        evGarbageCollection = new cMessage("garbageCollection");

        EV_WARN << "[RSA-MITIGATION] ═══════════════════════════════════════\n";
        EV_WARN << "[RSA-MITIGATION]   Sistema inizializzato con successo\n";
        EV_WARN << "[RSA-MITIGATION]    Node ID: " << nodeId << "\n";
        EV_WARN << "[RSA-MITIGATION]    Chiave privata: CARICATA (" << myPrivPem.length() << " bytes)\n";
        EV_WARN << "[RSA-MITIGATION]    Chiavi pubbliche: " << pubKeyMap.size() << " nodi\n";
        EV_WARN << "[RSA-MITIGATION]    Freshness window: " << freshnessWindow << "s\n";
        EV_WARN << "[RSA-MITIGATION]    Entry timeout: " << entryTimeout << " s\n";
        EV_WARN << "[RSA-MITIGATION]    GC interval: " << gcInterval << "s\n";
        EV_WARN << "[RSA-MITIGATION]    Max cache size: " << maxCacheSize << "\n";
        EV_WARN << "[RSA-MITIGATION] ═══════════════════════════════════════\n";

        // ── Registra segnali detection ──
        sigTP = registerSignal("detectionTP");
        sigFP = registerSignal("detectionFP");
        sigFN = registerSignal("detectionFN");
        sigTN = registerSignal("detectionTN");

        // ── Carica attacker node IDs ──
        std::string ids = par("attackerNodeIds").stdstringValue();
        std::istringstream iss(ids);
        std::string token;
        while (iss >> token) {
            attackerIds.insert(token);
        }
        EV_WARN << "[RSA-MITIGATION] Attaccanti noti: " << attackerIds.size() << "\n";



    }

    if( stage == INITSTAGE_ROUTING_PROTOCOLS ) {
        scheduleAt(simTime() + gcInterval, evGarbageCollection);
    }
    if (stage == INITSTAGE_APPLICATION_LAYER) {

        // Stampa mappatura nodeId → IP
        L3Address myAddr = getSelfIPAddress();
        EV_WARN << "[RSA-MITIGATION] MAPPATURA: " << nodeId
                << " → IP: " << myAddr << "\n";
    }
}

/* ***************************************************************** */
/*   2. Helper functions                                             */
/* ***************************************************************** */

void RsaAodvReplay::handleMessageWhenUp(cMessage* msg)
{
    if (msg == evGarbageCollection) {
        EV_WARN << "[RSA-MITIGATION] Garbage Collection - Inizio pulizia\n";

        int beforeRreq = lastReqSeq.size();
        int beforeRrep = lastRepSeq.size();

        performGarbageCollection();

        int afterRreq = lastReqSeq.size();
        int afterRrep = lastRepSeq.size();

        EV_WARN << "[RSA-MITIGATION]   RREQ map: " << beforeRreq << " → " << afterRreq
                << " (rimossi: " << (beforeRreq - afterRreq) << ")\n";
        EV_WARN << "[RSA-MITIGATION]   RREP map: " << beforeRrep << " → " << afterRrep
                << " (rimossi: " << (beforeRrep - afterRrep) << ")\n";

        scheduleAt(simTime() + gcInterval, evGarbageCollection);
        return;
    }

    Aodv::handleMessageWhenUp(msg);
}

std::string RsaAodvReplay::getPubPem(const std::string& sender) const
{
    auto it = pubKeyMap.find(sender);
    return (it == pubKeyMap.end()) ? "" : it->second;
}

std::string RsaAodvReplay::signPayload(const std::string& pl) const
{
    return RsaUtils::signPayload(pl, myPrivPem);
}

bool RsaAodvReplay::verifySig(const std::string& pl,
                                 const std::string& sig,
                                 const std::string& sender) const
{
    std::string pubKey = getPubPem(sender);
    if (pubKey.empty()) {
        return false;
    }
    return RsaUtils::verifySignature(pl, sig, pubKey);
}

void RsaAodvReplay::performGarbageCollection()
{
    simtime_t now = simTime();
    simtime_t threshold = now - entryTimeout;

    // Pulizia RREQ map
    auto it = lastReqSeq.begin();
    while (it != lastReqSeq.end()) {
        if (it->second.timestamp < threshold) {
            EV_DEBUG << "[RSA-MITIGATION] GC: Rimosso RREQ entry per " << it->first
                     << " (età: " << (now - it->second.timestamp) << "s)\n";
            it = lastReqSeq.erase(it);
        } else {
            ++it;
        }
    }

    // Pulizia RREP map
    it = lastRepSeq.begin();
    while (it != lastRepSeq.end()) {
        if (it->second.timestamp < threshold) {
            EV_DEBUG << "[RSA-MITIGATION] GC: Rimosso RREP entry per " << it->first
                     << " (età: " << (now - it->second.timestamp) << "s)\n";
            it = lastRepSeq.erase(it);
        } else {
            ++it;
        }
    }
}

/* ***************************************************************** */
/*   3. RREQ Creation                                                */
/* ***************************************************************** */

const Ptr<Rreq> RsaAodvReplay::createRREQ(const L3Address& destAddr)
{
    auto rq = Aodv::createRREQ(destAddr);
    rq->setSenderId(nodeId.c_str());
    rq->setSignature("");  // Firma aggiunta in sendRREQ
    return rq;
}

/* ***************************************************************** */
/*   4. RREP Creation                                                */
/* ***************************************************************** */

const Ptr<Rrep> RsaAodvReplay::createRREP(const Ptr<Rreq>& rreq,
                                             IRoute *destRoute,
                                             IRoute *originatorRoute,
                                             const L3Address& sourceAddr)
{
    auto rp = Aodv::createRREP(rreq, destRoute, originatorRoute, sourceAddr);

    // Costruisci payload: destAddr|destSeqNum
    std::string payload = rp->getDestAddr().str() + "|" + std::to_string(rp->getDestSeqNum());

    rp->setSenderId(nodeId.c_str());
    rp->setDestNodeId(nodeId.c_str());

    // Firma con RSA
    std::string sig = signPayload(payload);
    rp->setSignature(sig.c_str());

    EV_WARN << "[RSA-MITIGATION] RREP creato e firmato\n";
    EV_WARN << "[RSA-MITIGATION]   Sender: " << nodeId << "\n";
    EV_WARN << "[RSA-MITIGATION]   Destination: " << rp->getDestAddr() << "\n";
    EV_WARN << "[RSA-MITIGATION]   Dest Seq: " << rp->getDestSeqNum() << "\n";
    EV_WARN << "[RSA-MITIGATION]   Payload: " << payload << "\n";
    EV_WARN << "[RSA-MITIGATION]   Signature: " << sig.size() << " bytes\n";

    return rp;
}

/* ***************************************************************** */
/*   5. RREQ Sending (with signature)                               */
/* ***************************************************************** */

void RsaAodvReplay::sendRREQ(const Ptr<Rreq>& rreq,
                                const L3Address& destAddr,
                                unsigned int ttl)
{
    if (rreq->getOriginatorAddr() == getSelfIPAddress()) {
        int newSeq = std::max(rreq->getOriginatorSeqNum(), sequenceNum + 1);
        sequenceNum = newSeq;
        rreq->setOriginatorSeqNum(newSeq);

        // Costruisci payload: origAddr|seq
        std::string payload = rreq->getOriginatorAddr().str() + "|" + std::to_string(newSeq);

        rreq->setSenderId(nodeId.c_str());
        rreq->setOriginatorNodeId(nodeId.c_str());

        // Firma con RSA
        std::string sig = signPayload(payload);
        rreq->setSignature(sig.c_str());

        EV_WARN << "[RSA-MITIGATION] RREQ creato e firmato\n";
        EV_WARN << "[RSA-MITIGATION]   Sender: " << nodeId << "\n";
        EV_WARN << "[RSA-MITIGATION]   Originator: " << rreq->getOriginatorAddr() << "\n";
        EV_WARN << "[RSA-MITIGATION]   Seq Number: " << newSeq << "\n";
        EV_WARN << "[RSA-MITIGATION]   Payload: " << payload << "\n";
        EV_WARN << "[RSA-MITIGATION]   Signature: " << sig.size() << " bytes\n";
    }

    Aodv::sendRREQ(rreq, destAddr, ttl);
}

/* ***************************************************************** */
/*   6. RREQ Handling (verification)                                 */
/* ***************************************************************** */

void RsaAodvReplay::handleRREQ(const Ptr<Rreq>& rq,
                                  const L3Address& src,
                                  unsigned int ttl)
{
    EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
    EV_WARN << "[RSA-MITIGATION]  RREQ ricevuto - Inizio verifica\n";
    EV_WARN << "[RSA-MITIGATION]    Source IP: " << src << "\n";
    EV_WARN << "[RSA-MITIGATION]    Sender ID: " << rq->getSenderId() << "\n";
    EV_WARN << "[RSA-MITIGATION]    Originator: " << rq->getOriginatorAddr() << "\n";
    EV_WARN << "[RSA-MITIGATION]    Seq Number: " << rq->getOriginatorSeqNum() << "\n";
    EV_WARN << "[RSA-MITIGATION]    TTL: " << ttl << "\n";

    // Costruisci payload atteso
    std::string payload = rq->getOriginatorAddr().str() + "|" +
                         std::to_string(rq->getOriginatorSeqNum());

    std::string originatorNodeId = rq->getOriginatorNodeId();
    if (originatorNodeId.empty()) {
        EV_WARN << "[RSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-MITIGATION]    Motivo: ORIGINATOR NODE ID MANCANTE\n";
        EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    EV_WARN << "[RSA-MITIGATION]    Originator Node ID: " << originatorNodeId << "\n";
    EV_WARN << "[RSA-MITIGATION]    Payload ricostruito: " << payload << "\n";

    // Verifica firma RSA
    bool sigOk = verifySig(payload, rq->getSignature(), originatorNodeId);

    if (!sigOk) {
        std::string senderId = rq->getSenderId();  // chi ha trasmesso = attaccante
        if (attackerIds.empty()) {
            // Nessun attaccante configurato → drop legittimo → TN
            emit(sigTN, 1L);
        } else if (isAttacker(senderId)) {
            emit(sigTP, 1L);
        } else {
            emit(sigFP, 1L);
        }
        EV_WARN << "[RSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-MITIGATION]    Motivo: FIRMA RSA INVALIDA\n";
        EV_WARN << "[RSA-MITIGATION]    Payload: " << payload << "\n";
        EV_WARN << "[RSA-MITIGATION]    Signature length: " << strlen(rq->getSignature()) << " bytes\n";
        EV_WARN << "[RSA-MITIGATION]    POSSIBILE REPLAY ATTACK RILEVATO!\n";
        EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    L3Address orig = rq->getOriginatorAddr();
    int seq = rq->getOriginatorSeqNum();

    auto it = lastReqSeq.find(orig);

    // STEP 1: Lazy TTL Eviction
    if (it != lastReqSeq.end()) {
        simtime_t age = simTime() - it->second.timestamp;

        if (age > entryTimeout) {
            EV_INFO << "[RSA-MITIGATION] Entry RREQ scaduta per " << orig
                    << " (età: " << age << "s > " << entryTimeout << "s)\n";

            lastReqSeq.erase(it);
            it = lastReqSeq.end();  // Reset iteratore
        }
    }

    // STEP 2: Anti-replay Check
    bool isReplay = false;

    EV_WARN << "[RSA-MITIGATION]   Controllo anti-replay:\n";

    if (it != lastReqSeq.end()) {
        // Entry esiste e valida
        EV_WARN << "[RSA-MITIGATION]     Seq corrente: " << seq << "\n";
        EV_WARN << "[RSA-MITIGATION]     Seq precedente: " << it->second.seqNum << "\n";
        EV_WARN << "[RSA-MITIGATION]     Età entry: " << (simTime() - it->second.timestamp) << "s\n";

        if (seq <= it->second.seqNum) {
            isReplay = true;
        }
    } else {
        EV_WARN << "[RSA-MITIGATION]     Prima volta per questo nodo (o entry scaduta)\n";
    }

    if (isReplay) {
        std::string senderId = rq->getSenderId();
        if (isAttacker(senderId))
           emit(sigTP, 1L);  // attaccante bloccato
       else
           emit(sigTN, 1L);
        EV_WARN << "[RSA-MITIGATION]  PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-MITIGATION]    Motivo: SEQUENCE NUMBER REPLAY\n";
        EV_WARN << "[RSA-MITIGATION]    Seq ricevuto (" << seq << ") <= Seq precedente (" << it->second.seqNum << ")\n";
        EV_WARN << "[RSA-MITIGATION]    REPLAY ATTACK CONFERMATO!\n";
        EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
        return;

        simtime_t age = simTime() - it->second.timestamp;
        if (age < 1.0) {  // <1s = probabilmente duplicate AODV
            EV_WARN << "[RSA-MITIGATION]    Età entry: " << age << "s\n";
            EV_WARN << "[RSA-MITIGATION]    Probabile DUPLICATE AODV (multi-path)\n";
        } else {  // >1s = replay sospetto
            EV_WARN << "[RSA-MITIGATION]    Età entry: " << age << "s\n";
            EV_WARN << "[RSA-MITIGATION]    POSSIBILE REPLAY ATTACK!\n";
        }

        EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    // STEP 3: Aggiorna/Inserisci Entry
    if (it != lastReqSeq.end()) {
        // Aggiorna entry esistente
        it->second.seqNum = seq;
        it->second.timestamp = simTime();
    } else {
        // Inserisci nuova entry
        lastReqSeq[orig] = SeqEntry(seq, simTime());
    }

    // STEP 4: DoS Protection - MaxSize Check
    if (lastReqSeq.size() > (size_t)maxCacheSize) {
        EV_WARN << "[RSA-MITIGATION]  Cache RREQ piena ("
                << lastReqSeq.size() << " > " << maxCacheSize << ")\n";
        evictOldestFrom(lastReqSeq);
    }

    EV_WARN << "[RSA-MITIGATION] PACCHETTO VALIDO\n";
    EV_WARN << "[RSA-MITIGATION]   Firma RSA verificata con successo\n";
    EV_WARN << "[RSA-MITIGATION]   Sequence number aggiornato: " << seq << "\n";
    EV_WARN << "[RSA-MITIGATION]   Timestamp: " << simTime() << "\n";
    EV_WARN << "[RSA-MITIGATION]   Azione: Pacchetto accettato e processato\n";
    EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";

    std::string senderId = rq->getSenderId();
    if (isAttacker(senderId))
        emit(sigFN, 1L);
    else
        emit(sigTN, 1L);

    Aodv::handleRREQ(rq, src, ttl);
}

/* ***************************************************************** */
/*   7. RREP Handling (verification)                                 */
/* ***************************************************************** */

void RsaAodvReplay::handleRREP(const Ptr<Rrep>& rp,
                                  const L3Address& src)
{
    EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
    EV_WARN << "[RSA-MITIGATION]  RREP ricevuto - Inizio verifica\n";
    EV_WARN << "[RSA-MITIGATION]    Source IP: " << src << "\n";
    EV_WARN << "[RSA-MITIGATION]    Sender ID: " << rp->getSenderId() << "\n";

    EV_WARN << "[RSA-MITIGATION]    Destination: " << rp->getDestAddr() << "\n";
    EV_WARN << "[RSA-MITIGATION]    Dest Seq: " << rp->getDestSeqNum() << "\n";

    // Costruisci payload atteso
    std::string payload = rp->getDestAddr().str() + "|" +
                         std::to_string(rp->getDestSeqNum());
    std::string destNodeId = rp->getDestNodeId();
    EV_WARN << "[RSA-MITIGATION]    Payload ricostruito: " << payload << "\n";

    // Verifica firma RSA
    bool sigOk = verifySig(payload, rp->getSignature(), destNodeId);

    if (!sigOk) {
        std::string senderId = rp->getSenderId();  // chi ha trasmesso = attaccante
        if (attackerIds.empty()) {
            // Nessun attaccante configurato → drop legittimo → TN
            emit(sigTN, 1L);
        } else if (isAttacker(senderId)) {
            emit(sigTP, 1L);
        } else {
            emit(sigFP, 1L);
        }
        EV_WARN << "[RSA-MITIGATION]  PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-MITIGATION]    Motivo: FIRMA RSA INVALIDA\n";
        EV_WARN << "[RSA-MITIGATION]    Payload: " << payload << "\n";
        EV_WARN << "[RSA-MITIGATION]    Signature length: " << strlen(rp->getSignature()) << " bytes\n";
        EV_WARN << "[RSA-MITIGATION]    POSSIBILE REPLAY ATTACK RILEVATO!\n";
        EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    L3Address dest = rp->getDestAddr();
    int seq = rp->getDestSeqNum();

    auto it = lastRepSeq.find(dest);

    // STEP 1: Lazy TTL Eviction
    if (it != lastRepSeq.end()) {
        simtime_t age = simTime() - it->second.timestamp;

        if (age > entryTimeout) {
            EV_INFO << "[RSA-MITIGATION] Entry RREP scaduta per " << dest
                    << " (età: " << age << "s > " << entryTimeout << "s)\n";

            lastRepSeq.erase(it);
            it = lastRepSeq.end();  // Reset iteratore
        }
    }

    // STEP 2: Anti-replay Check
    bool isReplay = false;

    EV_WARN << "[RSA-MITIGATION]   Controllo anti-replay:\n";

    if (it != lastRepSeq.end()) {
        // Entry esiste e valida
        EV_WARN << "[RSA-MITIGATION]     Seq corrente: " << seq << "\n";
        EV_WARN << "[RSA-MITIGATION]     Seq precedente: " << it->second.seqNum << "\n";
        EV_WARN << "[RSA-MITIGATION]     Età entry: " << (simTime() - it->second.timestamp) << "s\n";

        if (seq <= it->second.seqNum) {
            isReplay = true;
        }
    } else {
        EV_WARN << "[RSA-MITIGATION]     Prima volta per questo nodo (o entry scaduta)\n";
    }

    if (isReplay) {
        std::string senderId = rp->getSenderId();
        if (isAttacker(senderId))
            emit(sigTP, 1L);  // attaccante bloccato
        else
            emit(sigTN, 1L);
        EV_WARN << "[RSA-MITIGATION]    PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-MITIGATION]    Motivo: SEQUENCE NUMBER REPLAY\n";
        EV_WARN << "[RSA-MITIGATION]    Seq ricevuto (" << seq << ") <= Seq precedente (" << it->second.seqNum << ")\n";
        EV_WARN << "[RSA-MITIGATION]    REPLAY ATTACK CONFERMATO!\n";
        EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    // STEP 3: Aggiorna/Inserisci Entry
    if (it != lastRepSeq.end()) {
        // Aggiorna entry esistente
        it->second.seqNum = seq;
        it->second.timestamp = simTime();
    } else {
        // Inserisci nuova entry
        lastRepSeq[dest] = SeqEntry(seq, simTime());
    }

    // STEP 4: DoS Protection - MaxSize Check
    if (lastRepSeq.size() > (size_t)maxCacheSize) {
        EV_WARN << "[RSA-MITIGATION]  Cache RREP piena ("
                << lastRepSeq.size() << " > " << maxCacheSize << ")\n";
        evictOldestFrom(lastRepSeq);
    }

    EV_WARN << "[RSA-MITIGATION]  PACCHETTO VALIDO\n";
    EV_WARN << "[RSA-MITIGATION]    Firma RSA verificata con successo\n";
    EV_WARN << "[RSA-MITIGATION]    Sequence number aggiornato: " << seq << "\n";
    EV_WARN << "[RSA-MITIGATION]    Azione: Pacchetto accettato e processato\n";
    EV_WARN << "[RSA-MITIGATION] ════════════════════════════════════\n";

    std::string senderId = rp->getSenderId();
    if (isAttacker(senderId))
        emit(sigFN, 1L);
    else
        emit(sigTN, 1L);
    Aodv::handleRREP(rp, src);
}

/* ***************************************************************** */
/*   Evict Oldest Entry (DoS Protection)                             */
/* ***************************************************************** */

void RsaAodvReplay::evictOldestFrom(std::map<L3Address, SeqEntry>& cache)
{
    if (cache.empty()) return;

    // Trova entry con timestamp più vecchio
    auto oldest = cache.begin();

    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if (it->second.timestamp < oldest->second.timestamp) {
            oldest = it;
        }
    }

    EV_INFO << "[RSA-MITIGATION] Cache piena, evicting oldest: "
            << oldest->first
            << " (età: " << (simTime() - oldest->second.timestamp) << "s)\n";

    cache.erase(oldest);
}
