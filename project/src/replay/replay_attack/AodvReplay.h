#ifndef __NETWORKPROJECTVANET_AODVREPLAY_H_
#define __NETWORKPROJECTVANET_AODVREPLAY_H_

#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/aodv/Aodv.h"
#include "inet/common/Ptr.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/unitdisk/UnitDiskPhyHeader_m.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

#include <deque>

using namespace inet;
using namespace omnetpp;
using namespace inet::aodv;

// Helper: safest way to pick the NIC 0 of the host that owns this module
static cModule *firstNic(cModule *host)
{
    if (!host) return nullptr;
    cModule *nic = host->getSubmodule("wlan", 0);
    if (!nic) nic = host->getSubmodule("eth", 0);   // fallback for wired tests
    return nic;
}

class AodvReplay : public Aodv
{
    private:
       /* ---- parameters set via .ini / .ned ---- */
       simtime_t  parStart      = 15;
       simtime_t  parLatency    = 0.3;
       simtime_t  parWindow     = 3;
       int        parTargetIpv4 = -1;   // -1 → sniff everything

       //Parametro per tipo di attacco
       bool enableForgery = true;
       bool enablePureReplay = true;
       simtime_t  pureReplayDelay = 0.1;


       /* ---- internal state ---- */
       bool       active        = false;
       cMessage  *evStart       = nullptr;
       cMessage  *evForward     = nullptr;
       cMessage  *evPureReplay = nullptr;

       std::deque<Ptr<Chunk>> pendingPureReplayQueue;

       struct BufferEntry {
           simtime_t   ts;      // when captured
           Ptr<Chunk>  aodv;    // RREQ or RREP chunk (shared–dup)
       };
       std::deque<BufferEntry> buf;
    public:
        AodvReplay();
        virtual ~AodvReplay() override;

        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessageWhenUp(cMessage *msg) override;
        virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) override;
        virtual void handleRREP(const Ptr<Rrep>& rrep,const L3Address& sourceAddr) override;

    protected:
        std::string nodeId;
        bool interesting(const Ptr<const Chunk>& c) const;
        void startAttack();
        void forwardOne();
        void purgeOld();
        void sendImmediateAck(const inet::Ipv4Address& dst);
        void sendingPureReplay();

};

#endif /* __VANETPROJECT_AODVREPLAY_H_ */
