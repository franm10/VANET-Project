#include "EcdsaAodvReplay.h"
#include "replay/replay_mitigation_ecdsa/utils/EcdsaUtils.h"
#include "replay/replay_mitigation_ecdsa/utils/json.hpp"
#include "inet/common/ModuleAccess.h"
#include <fstream>
#include <sstream>
#include <chrono>

Define_Module(EcdsaAodvReplay);

/* ***************************************************************** */
/*   Distruttore                                                      */
/* ***************************************************************** */

EcdsaAodvReplay::~EcdsaAodvReplay()
{
    cancelAndDelete(evGarbageCollection);
}

/* ***************************************************************** */
/*   1. initialize()                                                  */
/* ***************************************************************** */

void EcdsaAodvReplay::initialize(int stage)
{
    Aodv::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        nodeId = par("nodeId").stdstringValue();
        std::string privPath    = par("ecdsaPrivateKeyPem").stdstringValue();
        std::string mapPath     = par("ecdsaPubKeyMapJson").stdstringValue();
        freshnessWindow         = par("freshnessWindow");
        entryTimeout            = par("entryTimeout");
        gcInterval              = par("gcInterval");
        maxCacheSize            = par("maxCacheSize");

        // Auto-detect nodeId e private key
        cModule* host = getContainingNode(this);
        int idx = host ? host->getIndex() : 0;

        if (nodeId.empty() || nodeId == "nodeX")
            nodeId = "node" + std::to_string(idx);

        if (privPath.empty())
            privPath = "/home/giada/NetworkProjectVanet/src/replay/"
                       "replay_mitigation_ecdsa/keys/node"
                       + std::to_string(idx) + "_priv_ecdsa.pem";

        myPrivKeyPem = loadPemFile(privPath);
        if (myPrivKeyPem.empty())
            throw cRuntimeError("Cannot load ECDSA private key: %s", privPath.c_str());

        loadPublicKeyMap(mapPath);

        evGarbageCollection = new cMessage("ecdsaGarbageCollection");

        EV_WARN << "[ECDSA-MITIGATION] ═══════════════════════════════════════\n";
        EV_WARN << "[ECDSA-MITIGATION]   Sistema inizializzato\n";
        EV_WARN << "[ECDSA-MITIGATION]   Node ID: " << nodeId << "\n";
        EV_WARN << "[ECDSA-MITIGATION]   Chiave privata: CARICATA (" << myPrivKeyPem.length() << " bytes)\n";
        EV_WARN << "[ECDSA-MITIGATION]   Chiavi pubbliche: " << pubKeyMap.size() << " nodi\n";
        EV_WARN << "[ECDSA-MITIGATION]   Freshness window: " << freshnessWindow << "s\n";
        EV_WARN << "[ECDSA-MITIGATION]   Entry timeout: " << entryTimeout << "s\n";
        EV_WARN << "[ECDSA-MITIGATION]   GC interval: " << gcInterval << "s\n";
        EV_WARN << "[ECDSA-MITIGATION]   Max cache size: " << maxCacheSize << "\n";
        EV_WARN << "[ECDSA-MITIGATION] ═══════════════════════════════════════\n";
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

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        scheduleAt(simTime() + gcInterval, evGarbageCollection);
    }
    if (stage == INITSTAGE_APPLICATION_LAYER) {

        // Stampa mappatura nodeId → IP
        L3Address myAddr = getSelfIPAddress();
        EV_WARN << "[ECDSA-MITIGATION] MAPPATURA: " << nodeId
                << " → IP: " << myAddr << "\n";
    }
}

/* ***************************************************************** */
/*   2. handleMessageWhenUp – Garbage Collection                      */
/* ***************************************************************** */

void EcdsaAodvReplay::handleMessageWhenUp(cMessage* msg)
{
    if (msg == evGarbageCollection) {
        EV_WARN << "[ECDSA-MITIGATION] Garbage Collection - Inizio pulizia\n";

        int beforeRreq = lastReqSeq.size();
        int beforeRrep = lastRepSeq.size();

        performGarbageCollection();

        EV_WARN << "[ECDSA-MITIGATION]   RREQ map: " << beforeRreq << " → " << lastReqSeq.size()
                << " (rimossi: " << (beforeRreq - (int)lastReqSeq.size()) << ")\n";
        EV_WARN << "[ECDSA-MITIGATION]   RREP map: " << beforeRrep << " → " << lastRepSeq.size()
                << " (rimossi: " << (beforeRrep - (int)lastRepSeq.size()) << ")\n";

        scheduleAt(simTime() + gcInterval, evGarbageCollection);
        return;
    }

    Aodv::handleMessageWhenUp(msg);
}

/* ***************************************************************** */
/*   3. createRREQ                                                    */
/* ***************************************************************** */

const Ptr<Rreq> EcdsaAodvReplay::createRREQ(const L3Address& destAddr)
{
    auto rq = Aodv::createRREQ(destAddr);
    rq->setSenderId(nodeId.c_str());
    rq->setSignature("");   // firma aggiunta in sendRREQ
    return rq;
}

/* ***************************************************************** */
/*   4. createRREP – firma + setDestNodeId (FIX)                     */
/* ***************************************************************** */

const Ptr<Rrep> EcdsaAodvReplay::createRREP(const Ptr<Rreq>& rreq,
                                             IRoute* destRoute,
                                             IRoute* originatorRoute,
                                             const L3Address& sourceAddr)
{
    auto rp = Aodv::createRREP(rreq, destRoute, originatorRoute, sourceAddr);

    std::string payload = rp->getDestAddr().str() + "|" +
                          std::to_string(rp->getDestSeqNum());

    rp->setSenderId(nodeId.c_str());
    // FIX: destNodeId necessario per la verifica in handleRREP
    rp->setDestNodeId(nodeId.c_str());

    auto t0 = std::chrono::high_resolution_clock::now();
    std::string sig = EcdsaUtils::signPayload(payload, myPrivKeyPem);
    auto t1 = std::chrono::high_resolution_clock::now();
    double msSign = std::chrono::duration<double, std::milli>(t1 - t0).count();
    EV_WARN << "[ECDSA-MITIGATION] OVERHEAD firma RREP: " << msSign << " ms\n";

    rp->setSignature(sig.c_str());

    EV_WARN << "[ECDSA-MITIGATION] RREP creato e firmato\n";
    EV_WARN << "[ECDSA-MITIGATION]   Sender/DestNodeId: " << nodeId << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Destination: " << rp->getDestAddr() << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Dest Seq: " << rp->getDestSeqNum() << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Payload: " << payload << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Signature: " << sig.size() << " bytes\n";

    return rp;
}

/* ***************************************************************** */
/*   5. sendRREQ – firma + setOriginatorNodeId (FIX)                 */
/* ***************************************************************** */

void EcdsaAodvReplay::sendRREQ(const Ptr<Rreq>& rreq,
                               const L3Address& destAddr,
                               unsigned int timeToLive)
{
    if (!rreq) {
        EV_ERROR << "[ECDSA-MITIGATION] ERRORE: RREQ nullo!\n";
        return;
    }

    rreq->setSenderId(nodeId.c_str());

    if (rreq->getOriginatorAddr() == getSelfIPAddress()) {
        int newSeq = std::max(rreq->getOriginatorSeqNum(), sequenceNum + 1);
        sequenceNum = newSeq;
        rreq->setOriginatorSeqNum(newSeq);

        std::string payload = rreq->getOriginatorAddr().str() + "|" +
                              std::to_string(newSeq);

        // FIX: originatorNodeId necessario per la verifica in handleRREQ
        rreq->setOriginatorNodeId(nodeId.c_str());

        auto t0 = std::chrono::high_resolution_clock::now();
        std::string sig = EcdsaUtils::signPayload(payload, myPrivKeyPem);
        auto t1 = std::chrono::high_resolution_clock::now();
        double msSign = std::chrono::duration<double, std::milli>(t1 - t0).count();
        EV_WARN << "[ECDSA-MITIGATION] OVERHEAD firma RREQ: " << msSign << " ms\n";
        rreq->setSignature(sig.c_str());

        EV_WARN << "[ECDSA-MITIGATION] RREQ firmato\n";
        EV_WARN << "[ECDSA-MITIGATION]   Sender/OriginatorNodeId: " << nodeId << "\n";
        EV_WARN << "[ECDSA-MITIGATION]   Originator: " << rreq->getOriginatorAddr() << "\n";
        EV_WARN << "[ECDSA-MITIGATION]   Seq: " << newSeq << "\n";
        EV_WARN << "[ECDSA-MITIGATION]   Payload: " << payload << "\n";
        EV_WARN << "[ECDSA-MITIGATION]   Signature: " << sig.size() << " bytes\n";
    } else {
        EV_WARN << "[ECDSA-MITIGATION] RREQ forward (non originatore)\n";
        EV_WARN << "[ECDSA-MITIGATION]   Sender aggiornato: " << nodeId << "\n";
    }

    Aodv::sendRREQ(rreq, destAddr, timeToLive);
}

/* ***************************************************************** */
/*   6. handleRREQ – verifica ECDSA + anti-replay (FIX)              */
/* ***************************************************************** */

void EcdsaAodvReplay::handleRREQ(const Ptr<Rreq>& rq,
                                 const L3Address& src,
                                 unsigned int ttl)
{
    EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
    EV_WARN << "[ECDSA-MITIGATION] RREQ ricevuto - Inizio verifica\n";
    EV_WARN << "[ECDSA-MITIGATION]   Source IP: " << src << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Sender ID: " << rq->getSenderId() << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Originator: " << rq->getOriginatorAddr() << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Seq Number: " << rq->getOriginatorSeqNum() << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   TTL: " << ttl << "\n";

    // FIX: usa originatorNodeId (chi ha firmato), non senderId (potrebbe essere un forwarder)
    std::string originatorNodeId = rq->getOriginatorNodeId();
    if (originatorNodeId.empty()) {
        std::string senderId = rq->getSenderId();
        if (attackerIds.empty())
            emit(sigTN, 1L);
        else if (isAttacker(senderId))
            emit(sigTP, 1L);
        else
            emit(sigFP, 1L);
        EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[ECDSA-MITIGATION]   Motivo: ORIGINATOR NODE ID MANCANTE\n";
        return;
    }

    std::string payload = rq->getOriginatorAddr().str() + "|" +
                          std::to_string(rq->getOriginatorSeqNum());

    EV_WARN << "[ECDSA-MITIGATION]   Originator Node ID: " << originatorNodeId << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Payload ricostruito: " << payload << "\n";

    // ── STEP 1: Verifica firma ECDSA ──
    std::string pubKey = getPublicKeyPem(originatorNodeId);
    if (pubKey.empty()) {
        EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[ECDSA-MITIGATION]   Motivo: Chiave pubblica non trovata per '" << originatorNodeId << "'\n";
        EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    bool sigOk = EcdsaUtils::verifySignature(payload, rq->getSignature(), pubKey);
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
        EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[ECDSA-MITIGATION]   Motivo: FIRMA ECDSA INVALIDA\n";
        EV_WARN << "[ECDSA-MITIGATION]   Payload: " << payload << "\n";
        EV_WARN << "[ECDSA-MITIGATION]   POSSIBILE REPLAY ATTACK (forgery)\n";
        EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    L3Address orig = rq->getOriginatorAddr();
    int seq = rq->getOriginatorSeqNum();

    auto it = lastReqSeq.find(orig);

    // ── STEP 2: Lazy TTL Eviction ──
    if (it != lastReqSeq.end()) {
        simtime_t age = simTime() - it->second.timestamp;
        if (age > entryTimeout) {
            EV_INFO << "[ECDSA-MITIGATION] Entry RREQ scaduta per " << orig
                    << " (età: " << age << "s)\n";
            lastReqSeq.erase(it);
            it = lastReqSeq.end();
        }
    }

    // ── STEP 3: Anti-replay ──
    EV_WARN << "[ECDSA-MITIGATION]   Controllo anti-replay:\n";

    if (it != lastReqSeq.end()) {
        EV_WARN << "[ECDSA-MITIGATION]     Seq corrente: " << seq << "\n";
        EV_WARN << "[ECDSA-MITIGATION]     Seq precedente: " << it->second.seqNum << "\n";
        EV_WARN << "[ECDSA-MITIGATION]     Età entry: " << (simTime() - it->second.timestamp) << "s\n";

        if (seq <= it->second.seqNum) {
            std::string senderId = rq->getSenderId();
            if (isAttacker(senderId))
                    emit(sigTP, 1L);
                else
                    emit(sigTN, 1L);

            EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
            EV_WARN << "[ECDSA-MITIGATION]   Motivo: SEQUENCE NUMBER REPLAY\n";
            EV_WARN << "[ECDSA-MITIGATION]   Seq ricevuto (" << seq
                    << ") <= Seq precedente (" << it->second.seqNum << ")\n";
            EV_WARN << "[ECDSA-MITIGATION]   REPLAY ATTACK CONFERMATO (pure replay)\n";
            EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
            return;
        }
    } else {
        EV_WARN << "[ECDSA-MITIGATION]     Prima volta (o entry scaduta)\n";
    }

    // ── STEP 4: Aggiorna/Inserisci entry ──
    if (it != lastReqSeq.end()) {
        it->second.seqNum    = seq;
        it->second.timestamp = simTime();
    } else {
        lastReqSeq[orig] = SeqEntry(seq, simTime());
    }

    // ── STEP 5: DoS Protection ──
    if ((int)lastReqSeq.size() > maxCacheSize) {
        EV_WARN << "[ECDSA-MITIGATION] Cache RREQ piena (" << lastReqSeq.size()
                << " > " << maxCacheSize << ")\n";
        evictOldestFrom(lastReqSeq);
    }

    EV_WARN << "[ECDSA-MITIGATION] PACCHETTO VALIDO\n";
    EV_WARN << "[ECDSA-MITIGATION]   Firma ECDSA verificata\n";
    EV_WARN << "[ECDSA-MITIGATION]   Sequence number aggiornato: " << seq << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Azione: accettato e processato\n";
    EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";

    std::string senderId = rq->getSenderId();
    if (isAttacker(senderId))
        emit(sigFN, 1L);
    else
        emit(sigTN, 1L);

    Aodv::handleRREQ(rq, src, ttl);
}

/* ***************************************************************** */
/*   7. handleRREP – verifica ECDSA + anti-replay (FIX)              */
/* ***************************************************************** */

void EcdsaAodvReplay::handleRREP(const Ptr<Rrep>& rrep,
                                 const L3Address& src)
{
    EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
    EV_WARN << "[ECDSA-MITIGATION] RREP ricevuto - Inizio verifica\n";
    EV_WARN << "[ECDSA-MITIGATION]   Source IP: " << src << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Sender ID: " << rrep->getSenderId() << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Destination: " << rrep->getDestAddr() << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Dest Seq: " << rrep->getDestSeqNum() << "\n";

    // FIX: usa destNodeId (chi ha firmato l'RREP), non senderId
    std::string destNodeId = rrep->getDestNodeId();
    if (destNodeId.empty()) {
        std::string senderId = rrep->getSenderId();
        if (attackerIds.empty())
            emit(sigTN, 1L);
        else if (isAttacker(senderId))
            emit(sigTP, 1L);
        else
            emit(sigFP, 1L);
        EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[ECDSA-MITIGATION]   Motivo: DEST NODE ID MANCANTE\n";
        return;
    }

    std::string payload = rrep->getDestAddr().str() + "|" +
                          std::to_string(rrep->getDestSeqNum());

    EV_WARN << "[ECDSA-MITIGATION]   Dest Node ID: " << destNodeId << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Payload ricostruito: " << payload << "\n";

    // ── STEP 1: Verifica firma ECDSA ──
    std::string pubKey = getPublicKeyPem(destNodeId);
    if (pubKey.empty()) {
        EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[ECDSA-MITIGATION]   Motivo: Chiave pubblica non trovata per '" << destNodeId << "'\n";
        EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    bool sigOk = EcdsaUtils::verifySignature(payload, rrep->getSignature(), pubKey);
    if (!sigOk) {
        std::string senderId = rrep->getSenderId();  // chi ha trasmesso = attaccante
        if (attackerIds.empty()) {
            // Nessun attaccante configurato → drop legittimo → TN
            emit(sigTN, 1L);
        } else if (isAttacker(senderId)) {
            emit(sigTP, 1L);
        } else {
            emit(sigFP, 1L);
        }
        EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
        EV_WARN << "[ECDSA-MITIGATION]   Motivo: FIRMA ECDSA INVALIDA\n";
        EV_WARN << "[ECDSA-MITIGATION]   Payload: " << payload << "\n";
        EV_WARN << "[ECDSA-MITIGATION]   POSSIBILE REPLAY ATTACK (forgery)\n";
        EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
        return;
    }

    L3Address dest = rrep->getDestAddr();
    int seq = rrep->getDestSeqNum();

    auto it = lastRepSeq.find(dest);

    // ── STEP 2: Lazy TTL Eviction ──
    if (it != lastRepSeq.end()) {
        simtime_t age = simTime() - it->second.timestamp;
        if (age > entryTimeout) {
            EV_INFO << "[ECDSA-MITIGATION] Entry RREP scaduta per " << dest
                    << " (età: " << age << "s)\n";
            lastRepSeq.erase(it);
            it = lastRepSeq.end();
        }
    }

    // ── STEP 3: Anti-replay ──
    EV_WARN << "[ECDSA-MITIGATION]   Controllo anti-replay:\n";

    if (it != lastRepSeq.end()) {
        EV_WARN << "[ECDSA-MITIGATION]     Seq corrente: " << seq << "\n";
        EV_WARN << "[ECDSA-MITIGATION]     Seq precedente: " << it->second.seqNum << "\n";
        EV_WARN << "[ECDSA-MITIGATION]     Età entry: " << (simTime() - it->second.timestamp) << "s\n";

        if (seq <= it->second.seqNum) {
            std::string senderId = rrep->getSenderId();
            if (isAttacker(senderId))
                emit(sigTP, 1L);  // attaccante bloccato
            else
                emit(sigTN, 1L);
           // ── FINE AGGIUNTO ──
            EV_WARN << "[ECDSA-MITIGATION] PACCHETTO DROPPATO\n";
            EV_WARN << "[ECDSA-MITIGATION]   Motivo: SEQUENCE NUMBER REPLAY\n";
            EV_WARN << "[ECDSA-MITIGATION]   Seq ricevuto (" << seq
                    << ") <= Seq precedente (" << it->second.seqNum << ")\n";
            EV_WARN << "[ECDSA-MITIGATION]   REPLAY ATTACK CONFERMATO (pure replay)\n";
            EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";
            return;
        }
    } else {
        EV_WARN << "[ECDSA-MITIGATION]     Prima volta (o entry scaduta)\n";
    }

    // ── STEP 4: Aggiorna/Inserisci entry ──
    if (it != lastRepSeq.end()) {
        it->second.seqNum    = seq;
        it->second.timestamp = simTime();
    } else {
        lastRepSeq[dest] = SeqEntry(seq, simTime());
    }

    // ── STEP 5: DoS Protection ──
    if ((int)lastRepSeq.size() > maxCacheSize) {
        EV_WARN << "[ECDSA-MITIGATION] Cache RREP piena (" << lastRepSeq.size()
                << " > " << maxCacheSize << ")\n";
        evictOldestFrom(lastRepSeq);
    }

    EV_WARN << "[ECDSA-MITIGATION] PACCHETTO VALIDO\n";
    EV_WARN << "[ECDSA-MITIGATION]   Firma ECDSA verificata\n";
    EV_WARN << "[ECDSA-MITIGATION]   Sequence number aggiornato: " << seq << "\n";
    EV_WARN << "[ECDSA-MITIGATION]   Azione: accettato e processato\n";
    EV_WARN << "[ECDSA-MITIGATION] ════════════════════════════════════\n";

    std::string senderId = rrep->getSenderId();
    if (isAttacker(senderId))
        emit(sigFN, 1L);
    else
        emit(sigTN, 1L);
    Aodv::handleRREP(rrep, src);
}

/* ***************************************************************** */
/*   8. Garbage Collection                                            */
/* ***************************************************************** */

void EcdsaAodvReplay::performGarbageCollection()
{
    simtime_t threshold = simTime() - entryTimeout;

    for (auto it = lastReqSeq.begin(); it != lastReqSeq.end(); ) {
        if (it->second.timestamp < threshold)
            it = lastReqSeq.erase(it);
        else
            ++it;
    }

    for (auto it = lastRepSeq.begin(); it != lastRepSeq.end(); ) {
        if (it->second.timestamp < threshold)
            it = lastRepSeq.erase(it);
        else
            ++it;
    }
}

void EcdsaAodvReplay::evictOldestFrom(std::map<L3Address, SeqEntry>& cache)
{
    if (cache.empty()) return;

    auto oldest = cache.begin();
    for (auto it = cache.begin(); it != cache.end(); ++it)
        if (it->second.timestamp < oldest->second.timestamp)
            oldest = it;

    EV_INFO << "[ECDSA-MITIGATION] Evicting oldest: " << oldest->first
            << " (età: " << (simTime() - oldest->second.timestamp) << "s)\n";
    cache.erase(oldest);
}

/* ***************************************************************** */
/*   9. Utility                                                       */
/* ***************************************************************** */

std::string EcdsaAodvReplay::loadPemFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

void EcdsaAodvReplay::loadPublicKeyMap(const std::string& jsonPath)
{
    std::ifstream file(jsonPath);
    if (!file.is_open())
        throw cRuntimeError("Cannot open: %s", jsonPath.c_str());

    nlohmann::json j;
    file >> j;

    for (auto& kv : j.items()) {
        std::string pem = loadPemFile(kv.value());
        if (pem.empty())
            throw cRuntimeError("Cannot load: %s", ((std::string)kv.value()).c_str());
        pubKeyMap[kv.key()] = pem;
    }

    EV_WARN << "[ECDSA-MITIGATION] Caricate " << pubKeyMap.size() << " chiavi pubbliche\n";
}

std::string EcdsaAodvReplay::getPublicKeyPem(const std::string& senderId) const
{
    auto it = pubKeyMap.find(senderId);
    return (it != pubKeyMap.end()) ? it->second : "";
}
