#include "EcdsaBloomFilterAodvReplay.h"

#include "replay/replay_mitigation_ecdsa/utils/EcdsaUtils.h"
#include "replay/replay_mitigation_ecdsa/utils/json.hpp"
#include "inet/common/ModuleAccess.h"
#include <fstream>
#include <sstream>

Define_Module(EcdsaBloomFilterAodvReplay);

EcdsaBloomFilterAodvReplay::~EcdsaBloomFilterAodvReplay() {
    delete rreqBloomFilter;
    delete rrepBloomFilter;
    cancelAndDelete(evClearBloomFilter);
}

void EcdsaBloomFilterAodvReplay::initialize(int stage) {
    Aodv::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        nodeId = par("nodeId").stdstringValue();
        std::string privPath = par("ecdsaPrivateKeyPem").stdstringValue();
        std::string mapPath = par("ecdsaPubKeyMapJson").stdstringValue();

        // Auto-detect nodeId
        if (nodeId.empty() || nodeId == "nodeX") {
            cModule* host = getContainingNode(this);
            int idx = host ? host->getIndex() : 0;
            nodeId = "node" + std::to_string(idx);
        }

        // Auto-detect private key path
        if (privPath.empty()) {
            cModule* host = getContainingNode(this);
            int idx = host ? host->getIndex() : 0;
            privPath = "/home/giada/NetworkProjectVanet/src/replay/replay_mitigation_ecdsa/keys/node" +
                      std::to_string(idx) + "_priv_ecdsa.pem";
        }

        // Carica chiave privata
        myPrivKeyPem = loadPemFile(privPath);
        if (myPrivKeyPem.empty()) {
            throw cRuntimeError("Cannot load ECDSA private key: %s", privPath.c_str());
        }

        // Carica mappa chiavi pubbliche
        loadPublicKeyMap(mapPath);

        // ✅ NUOVO: Inizializza Bloom Filters
        bloomFilterSize = par("bloomFilterSize");
        bloomFilterHashes = par("bloomFilterHashes");
        bloomFilterClearInterval = par("bloomFilterClearInterval");

        rreqBloomFilter = new BloomFilter(bloomFilterSize, bloomFilterHashes);
        rrepBloomFilter = new BloomFilter(bloomFilterSize, bloomFilterHashes);

        evClearBloomFilter = new cMessage("clearBloomFilter");

        EV_WARN << "[ECDSA-BLOOM] ═══════════════════════════════════════\n";
        EV_WARN << "[ECDSA-BLOOM] 🔒 Sistema inizializzato con successo\n";
        EV_WARN << "[ECDSA-BLOOM]   Node ID: " << nodeId << "\n";
        EV_WARN << "[ECDSA-BLOOM]   Chiave privata: CARICATA (" << myPrivKeyPem.length() << " bytes)\n";
        EV_WARN << "[ECDSA-BLOOM]   Chiavi pubbliche: " << pubKeyMap.size() << " nodi\n";
        EV_WARN << "[ECDSA-BLOOM]   Bloom Filter size: " << bloomFilterSize << " bits\n";
        EV_WARN << "[ECDSA-BLOOM]   Hash functions: " << bloomFilterHashes << "\n";
        EV_WARN << "[ECDSA-BLOOM]   Clear interval: " << bloomFilterClearInterval << "s\n";
        EV_WARN << "[ECDSA-BLOOM] ═══════════════════════════════════════\n";
    }

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        // Schedula primo clear
        scheduleAt(simTime() + bloomFilterClearInterval, evClearBloomFilter);
    }
}

// Gestione timer clear
void EcdsaBloomFilterAodvReplay::handleMessageWhenUp(cMessage* msg) {
    if (msg == evClearBloomFilter) {
        EV_WARN << "[ECDSA-BLOOM] 🧹 Pulizia periodica Bloom Filters\n";
        EV_WARN << "[ECDSA-BLOOM]   RREQ insertions: " << rreqBloomFilter->getInsertCount() << "\n";
        EV_WARN << "[ECDSA-BLOOM]   RREP insertions: " << rrepBloomFilter->getInsertCount() << "\n";
        EV_WARN << "[ECDSA-BLOOM]   FPR RREQ: " << rreqBloomFilter->getEstimatedFPR() << "\n";
        EV_WARN << "[ECDSA-BLOOM]   FPR RREP: " << rrepBloomFilter->getEstimatedFPR() << "\n";

        rreqBloomFilter->clear();
        rrepBloomFilter->clear();

        scheduleAt(simTime() + bloomFilterClearInterval, evClearBloomFilter);
        return;
    }

    Aodv::handleMessageWhenUp(msg);
}

// ==================== CREA RREQ (identico) ====================

const Ptr<Rreq> EcdsaBloomFilterAodvReplay::createRREQ(const L3Address& destAddr) {
    auto rq = Aodv::createRREQ(destAddr);
    rq->setSenderId(nodeId.c_str());
    rq->setSignature("");
    return rq;
}

// ==================== CREA RREP (identico) ====================

const Ptr<Rrep> EcdsaBloomFilterAodvReplay::createRREP(const Ptr<Rreq>& rreq,
                                                  IRoute *destRoute,
                                                  IRoute *originatorRoute,
                                                  const L3Address& sourceAddr) {
    auto rp = Aodv::createRREP(rreq, destRoute, originatorRoute, sourceAddr);

    std::string payload = rp->getDestAddr().str() + "|" +
                          std::to_string(rp->getDestSeqNum());

    rp->setSenderId(nodeId.c_str());

    std::string sig = EcdsaUtils::signPayload(payload, myPrivKeyPem);
    rp->setSignature(sig.c_str());

    EV_WARN << "[ECDSA-BLOOM] 📤 RREP creato e firmato\n";
    EV_WARN << "[ECDSA-BLOOM]   Sender: " << nodeId << "\n";
    EV_WARN << "[ECDSA-BLOOM]   Destination: " << rp->getDestAddr() << "\n";
    EV_WARN << "[ECDSA-BLOOM]   Dest Seq: " << rp->getDestSeqNum() << "\n";

    return rp;
}

// ==================== FIRMA RREQ (identico) ====================

void EcdsaBloomFilterAodvReplay::sendRREQ(const Ptr<Rreq>& rreq,
                                    const L3Address& destAddr,
                                    unsigned int timeToLive) {

    if (!rreq) {
        EV_ERROR << "[ECDSA-BLOOM] ❌ ERRORE: RREQ nullo!\n";
        return;
    }

    L3Address originator = rreq->getOriginatorAddr();
    rreq->setSenderId(nodeId.c_str());

    if (originator == getSelfIPAddress()) {
        std::string payload = originator.str() + "|" +
                              std::to_string(rreq->getOriginatorSeqNum());

        std::string sig = EcdsaUtils::signPayload(payload, myPrivKeyPem);
        rreq->setSignature(sig.c_str());

        EV_WARN << "[ECDSA-BLOOM] 📤 RREQ creato e firmato\n";
        EV_WARN << "[ECDSA-BLOOM]   Sender: " << nodeId << "\n";
        EV_WARN << "[ECDSA-BLOOM]   Seq Number: " << rreq->getOriginatorSeqNum() << "\n";
    }

    Aodv::sendRREQ(rreq, destAddr, timeToLive);
}

// ==================== VERIFICA RREQ CON BLOOM FILTER ====================

void EcdsaBloomFilterAodvReplay::handleRREQ(const Ptr<Rreq>& rq,
                                      const L3Address& src,
                                      unsigned int ttl) {

    EV_WARN << "[ECDSA-BLOOM] ════════════════════════════════════\n";
    EV_WARN << "[ECDSA-BLOOM] 📥 RREQ ricevuto - Inizio verifica\n";
    EV_WARN << "[ECDSA-BLOOM]   Source IP: " << src << "\n";
    EV_WARN << "[ECDSA-BLOOM]   Sender ID: " << rq->getSenderId() << "\n";
    EV_WARN << "[ECDSA-BLOOM]   Seq Number: " << rq->getOriginatorSeqNum() << "\n";

    // Costruisci payload
    std::string payload = rq->getOriginatorAddr().str() + "|" +
                          std::to_string(rq->getOriginatorSeqNum());

    // Trova chiave pubblica
    std::string pubKey = getPublicKeyPem(rq->getSenderId());
    if (pubKey.empty()) {
        EV_WARN << "[ECDSA-BLOOM] ❌ DROP: Chiave pubblica non trovata\n";
        return;
    }

    // Verifica firma ECDSA
    bool valid = EcdsaUtils::verifySignature(payload, rq->getSignature(), pubKey);

    if (!valid) {
        EV_WARN << "[ECDSA-BLOOM] ❌ DROP: FIRMA ECDSA INVALIDA\n";
        EV_WARN << "[ECDSA-BLOOM]   ⚠️  POSSIBILE REPLAY ATTACK!\n";
        return;
    }

    // Check Bloom Filter anti-replay
    L3Address orig = rq->getOriginatorAddr();
    int seq = rq->getOriginatorSeqNum();
    std::string key = buildRreqKey(orig, seq);

    bool isReplay = rreqBloomFilter->contains(key);

    EV_WARN << "[ECDSA-BLOOM]   Bloom Filter check:\n";
    EV_WARN << "[ECDSA-BLOOM]     Key: " << key << "\n";
    EV_WARN << "[ECDSA-BLOOM]     FPR: " << rreqBloomFilter->getEstimatedFPR() << "\n";

    if (isReplay) {
        EV_WARN << "[ECDSA-BLOOM] ❌ DROP: REPLAY RILEVATO (Bloom Filter)\n";
        EV_WARN << "[ECDSA-BLOOM]   ⚠️  Possibile replay o false positive\n";
        return;
    }

    // Inserisci nel Bloom Filter
    rreqBloomFilter->insert(key);
    EV_WARN << "[ECDSA-BLOOM]   ✅ Chiave aggiunta al Bloom Filter\n";

    EV_WARN << "[ECDSA-BLOOM] ✅ PACCHETTO VALIDO E NUOVO\n";
    EV_WARN << "[ECDSA-BLOOM]   Firma ECDSA verificata\n";
    EV_WARN << "[ECDSA-BLOOM]   Nessun replay rilevato\n";
    EV_WARN << "[ECDSA-BLOOM] ════════════════════════════════════\n";

    Aodv::handleRREQ(rq, src, ttl);
}

// ==================== VERIFICA RREP CON BLOOM FILTER ====================

void EcdsaBloomFilterAodvReplay::handleRREP(const Ptr<Rrep>& rrep,
                                      const L3Address& sourceAddr) {

    EV_WARN << "[ECDSA-BLOOM] ════════════════════════════════════\n";
    EV_WARN << "[ECDSA-BLOOM] 📥 RREP ricevuto - Inizio verifica\n";
    EV_WARN << "[ECDSA-BLOOM]   Source IP: " << sourceAddr << "\n";
    EV_WARN << "[ECDSA-BLOOM]   Sender ID: " << rrep->getSenderId() << "\n";

    // Costruisci payload
    std::string payload = rrep->getDestAddr().str() + "|" +
                          std::to_string(rrep->getDestSeqNum());

    // Trova chiave pubblica
    std::string pubKey = getPublicKeyPem(rrep->getSenderId());
    if (pubKey.empty()) {
        EV_WARN << "[ECDSA-BLOOM] ❌ DROP: Chiave pubblica non trovata\n";
        return;
    }

    // Verifica firma ECDSA
    bool valid = EcdsaUtils::verifySignature(payload, rrep->getSignature(), pubKey);

    if (!valid) {
        EV_WARN << "[ECDSA-BLOOM] ❌ DROP: FIRMA ECDSA INVALIDA\n";
        return;
    }

    // Check Bloom Filter anti-replay
    L3Address dest = rrep->getDestAddr();
    int seq = rrep->getDestSeqNum();
    std::string key = buildRrepKey(dest, seq);

    bool isReplay = rrepBloomFilter->contains(key);

    if (isReplay) {
        EV_WARN << "[ECDSA-BLOOM] ❌ DROP: REPLAY RILEVATO (RREP)\n";
        return;
    }

    rrepBloomFilter->insert(key);
    EV_WARN << "[ECDSA-BLOOM] ✅ RREP aggiunto al Bloom Filter\n";

    EV_WARN << "[ECDSA-BLOOM] ✅ PACCHETTO VALIDO\n";
    EV_WARN << "[ECDSA-BLOOM] ════════════════════════════════════\n";

    Aodv::handleRREP(rrep, sourceAddr);
}

// ==================== UTILITY (identiche) ====================

std::string EcdsaBloomFilterAodvReplay::loadPemFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void EcdsaBloomFilterAodvReplay::loadPublicKeyMap(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        throw cRuntimeError("Cannot open: %s", jsonPath.c_str());
    }

    nlohmann::json j;
    file >> j;

    for (auto& kv : j.items()) {
        std::string senderId = kv.key();
        std::string pemPath = kv.value();
        std::string pemContent = loadPemFile(pemPath);

        if (pemContent.empty()) {
            throw cRuntimeError("Cannot load: %s", pemPath.c_str());
        }

        pubKeyMap[senderId] = pemContent;
    }

    EV_WARN << "[ECDSA-BLOOM] Caricate " << pubKeyMap.size() << " chiavi pubbliche\n";
}

std::string EcdsaBloomFilterAodvReplay::getPublicKeyPem(const std::string& senderId) const {
    auto it = pubKeyMap.find(senderId);
    return (it != pubKeyMap.end()) ? it->second : "";
}

// Helper per costruire chiavi Bloom Filter
std::string EcdsaBloomFilterAodvReplay::buildRreqKey(const L3Address& orig, int seq) const {
    return orig.str() + "|" + std::to_string(seq) + "|RREQ";
}

std::string EcdsaBloomFilterAodvReplay::buildRrepKey(const L3Address& dest, int seq) const {
    return dest.str() + "|" + std::to_string(seq) + "|RREP";
}
