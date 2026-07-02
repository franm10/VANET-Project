#ifndef __RSA_BM_AODV_REPLAY_H_
#define __RSA_BM_AODV_REPLAY_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/common/Ptr.h"
#include "inet/routing/aodv/Aodv.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include <map>
#include <string>

#include "replay/utils/BloomFilter.h"

using namespace omnetpp;
using namespace inet;
using namespace inet::aodv;

class RsaBloomFilterAodvReplay : public Aodv
{
  public:
    virtual ~RsaBloomFilterAodvReplay();

  protected:
    /* parametri caricati da NED/INI */
    std::string nodeId;
    std::string myPrivPem;
    simtime_t   freshnessWindow;

    const std::string& getNodeId() const   { return nodeId; }

    /* mappa senderId → chiave pubblica PEM */
    std::map<std::string,std::string> pubKeyMap;

    /* tabelle anti-replay */
    BloomFilter* rreqBloomFilter;
    BloomFilter* rrepBloomFilter;

    // Parametri configurabili
    size_t bloomFilterSize;
    size_t bloomFilterHashes;
    simtime_t bloomFilterClearInterval;

    // Timer per pulizia periodica
    cMessage* evClearBloomFilter;

    // Helper per costruire chiavi
    std::string buildRreqKey(const L3Address& orig, int seq) const;
    std::string buildRrepKey(const L3Address& dest, int seq) const;

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


  private:
    /* helpers firma/verifica */
    std::string signPayload(const std::string& pl) const;
    bool verifySig(const std::string& pl,
                   const std::string& sig,
                   const std::string& sender) const;
    std::string getPubPem(const std::string& sender) const;
};

#endif
