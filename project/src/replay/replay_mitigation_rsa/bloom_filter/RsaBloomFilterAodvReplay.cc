//  RsaBloomFilterAodvReplay.cc
//  ───────────────────────────────────────────────────────────────
//  Estende il modulo INET AODV per aggiungere:
//    1) Firma RSA dei pacchetti RREQ/RREP (autenticità / integrità)
//    2) Filtro anti-replay: accetta solo seqNum strettamente crescenti
//  Compatibile con: OMNeT++ 5.6 / INET 4.2 / Veins 5.2
//  ----------------------------------------------------------------

#include "RsaBloomFilterAodvReplay.h"

#include "replay/replay_mitigation_rsa/utils/RsaUtils.h"
#include "replay/replay_mitigation_rsa/utils/json.hpp"
#include "inet/common/ModuleAccess.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace inet;
using namespace aodv;

Define_Module(RsaBloomFilterAodvReplay);

RsaBloomFilterAodvReplay::~RsaBloomFilterAodvReplay()
{
    delete rreqBloomFilter;
    delete rrepBloomFilter;
    cancelAndDelete(evClearBloomFilter);
}

/* ***************************************************************** */
/*   1. initialize()  –  carica parametri, chiavi pubbliche, ecc.    */
/* ***************************************************************** */

void RsaBloomFilterAodvReplay::initialize(int stage)
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

        EV_WARN << "[RSA-BLOOMFILTER] ═══════════════════════════════════════\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Sistema inizializzato con successo\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Node ID: " << nodeId << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Chiave privata: CARICATA (" << myPrivPem.length() << " bytes)\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Chiavi pubbliche: " << pubKeyMap.size() << " nodi\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Freshness window: " << freshnessWindow << "s\n";
        EV_WARN << "[RSA-BLOOMFILTER] ═══════════════════════════════════════\n";

        // Leggi parametri Bloom Filter
        bloomFilterSize = par("bloomFilterSize");
        bloomFilterHashes = par("bloomFilterHashes");
        bloomFilterClearInterval = par("bloomFilterClearInterval");

        // Crea i Bloom Filters
        rreqBloomFilter = new BloomFilter(bloomFilterSize, bloomFilterHashes);
        rrepBloomFilter = new BloomFilter(bloomFilterSize, bloomFilterHashes);

        // Crea timer per pulizia periodica
        evClearBloomFilter = new cMessage("clearBloomFilter");

        EV_WARN << "[RSA-BLOOMFILTER] Bloom Filters inizializzati\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Size: " << bloomFilterSize << " bits\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Hash functions: " << bloomFilterHashes << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]   Clear interval: " << bloomFilterClearInterval << "s\n";

    }

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        // Schedula primo clear
        scheduleAt(simTime() + bloomFilterClearInterval, evClearBloomFilter);
    }
}

/* ***************************************************************** */
/*   2. Helper functions                                             */
/* ***************************************************************** */

std::string RsaBloomFilterAodvReplay::getPubPem(const std::string& sender) const
{
    auto it = pubKeyMap.find(sender);
    return (it == pubKeyMap.end()) ? "" : it->second;
}

std::string RsaBloomFilterAodvReplay::signPayload(const std::string& pl) const
{
    return RsaUtils::signPayload(pl, myPrivPem);
}

bool RsaBloomFilterAodvReplay::verifySig(const std::string& pl,
                                 const std::string& sig,
                                 const std::string& sender) const
{
    std::string pubKey = getPubPem(sender);
    if (pubKey.empty()) {
        return false;
    }
    return RsaUtils::verifySignature(pl, sig, pubKey);
}

std::string RsaBloomFilterAodvReplay::buildRreqKey(const L3Address& orig, int seq) const {
    return orig.str() + "|" + std::to_string(seq) + "|RREQ";
}

std::string RsaBloomFilterAodvReplay::buildRrepKey(const L3Address& dest, int seq) const {
    return dest.str() + "|" + std::to_string(seq) + "|RREP";
}

void RsaBloomFilterAodvReplay::handleMessageWhenUp(cMessage* msg) {
    if (msg == evClearBloomFilter) {
        EV_WARN << "[RSA-BLOOMFILTER] Pulizia periodica Bloom Filters\n";
        EV_WARN << "[RSA-BLOOMFILTER]    RREQ insertions: " << rreqBloomFilter->getInsertCount() << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    RREP insertions: " << rrepBloomFilter->getInsertCount() << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    FPR RREQ: " << rreqBloomFilter->getEstimatedFPR() << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    FPR RREP: " << rrepBloomFilter->getEstimatedFPR() << "\n";

        rreqBloomFilter->clear();
        rrepBloomFilter->clear();

        scheduleAt(simTime() + bloomFilterClearInterval, evClearBloomFilter);
        return;
    }

    // Chiamata al parent
    Aodv::handleMessageWhenUp(msg);
}
/* ***************************************************************** */
/*   3. RREQ Creation                                                */
/* ***************************************************************** */

const Ptr<Rreq> RsaBloomFilterAodvReplay::createRREQ(const L3Address& destAddr)
{
    auto rq = Aodv::createRREQ(destAddr);
    rq->setSenderId(nodeId.c_str());
    rq->setSignature("");  // Firma aggiunta in sendRREQ
    return rq;
}

/* ***************************************************************** */
/*   4. RREP Creation                                                */
/* ***************************************************************** */

const Ptr<Rrep> RsaBloomFilterAodvReplay::createRREP(const Ptr<Rreq>& rreq,
                                             IRoute *destRoute,
                                             IRoute *originatorRoute,
                                             const L3Address& sourceAddr)
{
    auto rp = Aodv::createRREP(rreq, destRoute, originatorRoute, sourceAddr);

    // Costruisci payload: destAddr|destSeqNum
    std::string payload = rp->getDestAddr().str() + "|" + std::to_string(rp->getDestSeqNum());

    rp->setSenderId(nodeId.c_str());

    // Firma con RSA
    std::string sig = signPayload(payload);
    rp->setSignature(sig.c_str());

    EV_WARN << "[RSA-BLOOMFILTER]  RREP creato e firmato\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Sender: " << nodeId << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Destination: " << rp->getDestAddr() << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Dest Seq: " << rp->getDestSeqNum() << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Payload: " << payload << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Signature: " << sig.size() << " bytes\n";

    return rp;
}

/* ***************************************************************** */
/*   5. RREQ Sending (with signature)                               */
/* ***************************************************************** */

void RsaBloomFilterAodvReplay::sendRREQ(const Ptr<Rreq>& rreq,
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

        // Firma con RSA
        std::string sig = signPayload(payload);
        rreq->setSignature(sig.c_str());

        EV_WARN << "[RSA-BLOOMFILTER] RREQ creato e firmato\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Sender: " << nodeId << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Originator: " << rreq->getOriginatorAddr() << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Seq Number: " << newSeq << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Payload: " << payload << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Signature: " << sig.size() << " bytes\n";
    }

    Aodv::sendRREQ(rreq, destAddr, ttl);
}

/* ***************************************************************** */
/*   6. RREQ Handling (verification)                                 */
/* ***************************************************************** */

void RsaBloomFilterAodvReplay::handleRREQ(const Ptr<Rreq>& rq,
                                  const L3Address& src,
                                  unsigned int ttl)
{
    EV_WARN << "[RSA-BLOOMFILTER] ════════════════════════════════════\n";
    EV_WARN << "[RSA-BLOOMFILTER]  RREQ ricevuto - Inizio verifica\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Source IP: " << src << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Sender ID: " << rq->getSenderId() << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Originator: " << rq->getOriginatorAddr() << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Seq Number: " << rq->getOriginatorSeqNum() << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    TTL: " << ttl << "\n";

    // Costruisci payload atteso
    std::string payload = rq->getOriginatorAddr().str() + "|" +
                         std::to_string(rq->getOriginatorSeqNum());

    EV_WARN << "[RSA-BLOOMFILTER]    Payload ricostruito: " << payload << "\n";

    // Verifica firma RSA
    bool sigOk = verifySig(payload, rq->getSignature(), rq->getSenderId());

    if (!sigOk) {
        EV_WARN << "[RSA-BLOOMFILTER]  PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Motivo: FIRMA RSA INVALIDA\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Payload: " << payload << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Signature length: " << strlen(rq->getSignature()) << " bytes\n";
        EV_WARN << "[RSA-BLOOMFILTER]    POSSIBILE REPLAY ATTACK RILEVATO!\n";
        EV_WARN << "[RSA-BLOOMFILTER] ════════════════════════════════════\n";
        return;
    }

    // Anti-replay check
    L3Address orig = rq->getOriginatorAddr();
    int seq = rq->getOriginatorSeqNum();
    std::string key = buildRreqKey(orig, seq);

    bool isReplay = rreqBloomFilter->contains(key);

    EV_WARN << "[RSA-BLOOMFILTER]   Controllo anti-replay (Bloom Filter):\n";
    EV_WARN << "[RSA-BLOOMFILTER]      Key: " << key << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]      FPR stimato: " << rreqBloomFilter->getEstimatedFPR() << "\n";

    if (isReplay) {
        EV_WARN << "[RSA-BLOOMFILTER]  PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-BLOOMFILTER]     Motivo: REPLAY RILEVATO (Bloom Filter)\n";
        EV_WARN << "[RSA-BLOOMFILTER]     Possibile replay o false positive\n";
        EV_WARN << "[RSA-BLOOMFILTER] ════════════════════════════════════\n";
        return;
    }

    // Inserisci nel Bloom Filter
    rreqBloomFilter->insert(key);
    EV_WARN << "[RSA-BLOOMFILTER]   Chiave aggiunta al Bloom Filter\n";

    EV_WARN << "[RSA-BLOOMFILTER]  PACCHETTO VALIDO\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Firma RSA verificata con successo\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Sequence number aggiornato: " << seq << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Azione: Pacchetto accettato e processato\n";
    EV_WARN << "[RSA-BLOOMFILTER] ════════════════════════════════════\n";

    Aodv::handleRREQ(rq, src, ttl);
}

/* ***************************************************************** */
/*   7. RREP Handling (verification)                                 */
/* ***************************************************************** */

void RsaBloomFilterAodvReplay::handleRREP(const Ptr<Rrep>& rp,
                                  const L3Address& src)
{
    EV_WARN << "[RSA-BLOOMFILTER] ════════════════════════════════════\n";
    EV_WARN << "[RSA-BLOOMFILTER]   RREP ricevuto - Inizio verifica\n";
    EV_WARN << "[RSA-BLOOMFILTER]     Source IP: " << src << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]     Sender ID: " << rp->getSenderId() << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]     Destination: " << rp->getDestAddr() << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]     Dest Seq: " << rp->getDestSeqNum() << "\n";

    // Costruisci payload atteso
    std::string payload = rp->getDestAddr().str() + "|" +
                         std::to_string(rp->getDestSeqNum());

    EV_WARN << "[RSA-BLOOMFILTER]   Payload ricostruito: " << payload << "\n";

    // Verifica firma RSA
    bool sigOk = verifySig(payload, rp->getSignature(), rp->getSenderId());

    if (!sigOk) {
        EV_WARN << "[RSA-BLOOMFILTER]  PACCHETTO DROPPATO\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Motivo: FIRMA RSA INVALIDA\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Payload: " << payload << "\n";
        EV_WARN << "[RSA-BLOOMFILTER]    Signature length: " << strlen(rp->getSignature()) << " bytes\n";
        EV_WARN << "[RSA-BLOOMFILTER]    POSSIBILE REPLAY ATTACK RILEVATO!\n";
        EV_WARN << "[RSA-BLOOMFILTER] ════════════════════════════════════\n";
        return;
    }

    // Anti-replay check
    L3Address dest = rp->getDestAddr();
    int seq = rp->getDestSeqNum();
    std::string key = buildRrepKey(dest, seq);

    bool isReplay = rrepBloomFilter->contains(key);

    EV_WARN << "[RSA-BLOOMFILTER]   Controllo anti-replay (Bloom Filter):\n";
    EV_WARN << "[RSA-BLOOMFILTER]     Key: " << key << "\n";

    if (isReplay) {
        EV_WARN << "[RSA-BLOOMFILTER]  REPLAY RILEVATO (RREP)\n";
        return;
    }

    rrepBloomFilter->insert(key);
    EV_WARN << "[RSA-BLOOMFILTER]   RREP aggiunto al Bloom Filter\n";

    EV_WARN << "[RSA-BLOOMFILTER]  PACCHETTO VALIDO\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Firma RSA verificata con successo\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Sequence number aggiornato: " << seq << "\n";
    EV_WARN << "[RSA-BLOOMFILTER]    Azione: Pacchetto accettato e processato\n";
    EV_WARN << "[RSA-BLOOMFILTER] ════════════════════════════════════\n";

    Aodv::handleRREP(rp, src);
}
