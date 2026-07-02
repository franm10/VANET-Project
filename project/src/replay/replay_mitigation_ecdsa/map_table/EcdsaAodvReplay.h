#ifndef __ECDSA_AODV_REPLAY_H_
#define __ECDSA_AODV_REPLAY_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/common/Ptr.h"
#include "inet/routing/aodv/Aodv.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include <map>
#include <set>       // AGGIUNTO
#include <string>
#include <sstream>   // AGGIUNTO
using namespace omnetpp;
using namespace inet;
using namespace inet::aodv;

class EcdsaAodvReplay : public Aodv
{
  protected:
    /* parametri NED/INI */
    std::string nodeId;
    std::string myPrivKeyPem;
    simtime_t   freshnessWindow;

    /* mappa senderId → chiave pubblica PEM */
    std::map<std::string, std::string> pubKeyMap;

    /* ── Anti-replay ── */
    struct SeqEntry {
        int       seqNum;
        simtime_t timestamp;
        SeqEntry() : seqNum(-1), timestamp(0) {}
        SeqEntry(int s, simtime_t t) : seqNum(s), timestamp(t) {}
    };
    std::map<L3Address, SeqEntry> lastReqSeq;   // originator → max seq RREQ
    std::map<L3Address, SeqEntry> lastRepSeq;   // dest       → max seq RREP

    /* ── Garbage Collection ── */
    simtime_t  entryTimeout;
    simtime_t  gcInterval;
    int        maxCacheSize;
    cMessage*  evGarbageCollection = nullptr;

    // ── AGGIUNTO: segnali detection ──
    simsignal_t sigTP, sigFP, sigFN, sigTN;

    // ── AGGIUNTO: set attaccanti noti ──
    std::set<std::string> attackerIds;
    bool isAttacker(const std::string& id) const {
        return attackerIds.count(id) > 0;
    }

  public:
    virtual ~EcdsaAodvReplay();

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage* msg) override;

    virtual void handleRREQ(const Ptr<Rreq>& rq,
                            const L3Address& src, unsigned int ttl) override;
    virtual void handleRREP(const Ptr<Rrep>& rrep,
                            const L3Address& sourceAddr) override;

    virtual const Ptr<Rreq> createRREQ(const L3Address& destAddr) override;
    virtual const Ptr<Rrep> createRREP(const Ptr<Rreq>& rreq,
                                       IRoute* destRoute,
                                       IRoute* originatorRoute,
                                       const L3Address& sourceAddr) override;
    virtual void sendRREQ(const Ptr<Rreq>& rreq,
                          const L3Address& destAddr,
                          unsigned int timeToLive) override;

  private:
    std::string loadPemFile(const std::string& path);
    void        loadPublicKeyMap(const std::string& jsonPath);
    std::string getPublicKeyPem(const std::string& senderId) const;

    void performGarbageCollection();
    void evictOldestFrom(std::map<L3Address, SeqEntry>& cache);
};

#endif
