#ifndef __ECDSA_BM_AODV_REPLAY_H_
#define __ECDSA_BM_AODV_REPLAY_H_

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

/**
 * AODV con firma ECDSA-256 + Bloom Filter anti-replay
 */
class EcdsaBloomFilterAodvReplay : public Aodv
{
  protected:
    std::string nodeId;
    std::string myPrivKeyPem;
    std::map<std::string, std::string> pubKeyMap;

    // Bloom Filters al posto delle map
    BloomFilter* rreqBloomFilter;
    BloomFilter* rrepBloomFilter;

    // Parametri Bloom Filter
    size_t bloomFilterSize;
    size_t bloomFilterHashes;
    simtime_t bloomFilterClearInterval;

    // Timer per clear periodico
    cMessage* evClearBloomFilter;

  public:
    virtual ~EcdsaBloomFilterAodvReplay();

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleRREQ(const Ptr<Rreq>& rq, const L3Address& src, unsigned int ttl) override;
    virtual void handleRREP(const Ptr<Rrep>& rrep, const L3Address& sourceAddr);
    virtual const Ptr<Rreq> createRREQ(const L3Address& destAddr);
    virtual const Ptr<Rrep> createRREP(const Ptr<Rreq>& rreq, IRoute *destRoute, IRoute *originatorRoute, const L3Address& sourceAddr);
    virtual void sendRREQ(const Ptr<Rreq>& rreq, const L3Address& destAddr, unsigned int timeToLive);

  private:
    std::string loadPemFile(const std::string& path);
    void loadPublicKeyMap(const std::string& jsonPath);
    std::string getPublicKeyPem(const std::string& senderId) const;

    // Helper per costruire chiavi Bloom Filter
    std::string buildRreqKey(const L3Address& orig, int seq) const;
    std::string buildRrepKey(const L3Address& dest, int seq) const;
};

#endif
