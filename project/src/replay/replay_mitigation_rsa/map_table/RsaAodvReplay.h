#ifndef __RSAAODV_REPLAY_H_
#define __RSAAODV_REPLAY_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/common/Ptr.h"
#include "inet/routing/aodv/Aodv.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include <map>
#include <set>
#include <string>
#include <sstream>

using namespace omnetpp;
using namespace inet;
using namespace inet::aodv;

class RsaAodvReplay : public Aodv
{
  protected:
    /* parametri caricati da NED/INI */
    std::string nodeId;
    std::string myPrivPem;
    simtime_t   freshnessWindow;

    const std::string& getNodeId() const   { return nodeId; }

    /* mappa senderId → chiave pubblica PEM */
    std::map<std::string,std::string> pubKeyMap;

    struct SeqEntry {
        int seqNum;
        simtime_t timestamp;

        SeqEntry() : seqNum(-1), timestamp(0) {}
        SeqEntry(int seq, simtime_t ts) : seqNum(seq), timestamp(ts) {}
    };

    /* tabelle anti-replay */
    std::map<L3Address, SeqEntry> lastReqSeq;   // <originator, max seq RREQ>
    std::map<L3Address, SeqEntry> lastRepSeq;   // <dest, max seq RREP>

    /* Garbage Collection */
    simtime_t entryTimeout;
    simtime_t gcInterval;
    cMessage* evGarbageCollection;

    // ── AGGIUNTO: segnali detection ──
    simsignal_t sigTP, sigFP, sigFN, sigTN;

        // ── AGGIUNTO: set attaccanti noti ──
    std::set<std::string> attackerIds;
    bool isAttacker(const std::string& id) const {
        return attackerIds.count(id) > 0;
    }

    /* override */
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleRREQ(const Ptr<Rreq>& rq,
                            const L3Address& src, unsigned int ttl) override;
    virtual void handleRREP(const Ptr<Rrep>& rrep,
                            const L3Address& sourceAddr);

    virtual const Ptr<Rreq> createRREQ(const L3Address& destAddr) override;
    virtual const Ptr<Rrep> createRREP(const Ptr<Rreq>& rreq, IRoute *destRoute, IRoute *originatorRoute, const L3Address& sourceAddr) override;

    virtual void sendRREQ(const Ptr<Rreq>& rreq, const L3Address& destAddr, unsigned int timeToLive) override;

  public:
    virtual ~RsaAodvReplay();

  private:
    int maxCacheSize; //dimensione massima della cache
    /* helpers firma/verifica */
    std::string signPayload(const std::string& pl) const;
    bool verifySig(const std::string& pl,
                   const std::string& sig,
                   const std::string& sender) const;
    std::string getPubPem(const std::string& sender) const;
    void performGarbageCollection();
    void evictOldestFrom(std::map<L3Address, SeqEntry>& cache);
};

#endif
