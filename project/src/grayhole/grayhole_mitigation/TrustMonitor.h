#ifndef TRUSTMONITOR_H_
#define TRUSTMONITOR_H_
#include <omnetpp.h>
#include <map>
#include <set>
#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "TrustManager.h"
#include "messages/IntermediateAck_m_m.h"
using namespace omnetpp;
using namespace inet;

struct ForwardEntry {
    int       packetId;
    int       receivedFromNodeId;
    int       forwardedToNodeId;
    simtime_t forwardTime;
    bool      ackReceived = false;
};

class TrustMonitor : public cSimpleModule {
private:
    TrustManager *trustManager = nullptr;
    std::map<int, ForwardEntry> forwardTable;
    cMessage *cleanupTimer = nullptr;
    double    cleanupTimeout;
    int myNodeId = -1;
    void cleanupExpiredEntries();
    std::set<int> alreadyProcessedAck;

    // --------------------------------------------------------
    // METRICHE
    // --------------------------------------------------------
    // PDR / PER
    simsignal_t pdrSignal;
    simsignal_t perSignal;
    long totalPacketsSent      = 0;
    long totalPacketsDelivered = 0;

    // Throughput
    simsignal_t throughputSignal;
    long totalBytesDelivered = 0;
    int  messageLength       = 512; // bytes, da parametro .ned

    // Delay end-to-end
    simsignal_t              delaySignal;
    std::map<int, simtime_t> packetSendTime; // packetId → timestamp invio

    // Overhead beacon
    simsignal_t beaconOverheadBytesSignal;
    long totalBeaconBytes = 0;

protected:
    virtual int  numInitStages() const override { return 3; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

public:
    void onPacketForwarded(int packetId,
                           int receivedFromNodeId,
                           int forwardedToNodeId,
                           simtime_t forwardTime);
    void onFinalAckReceived(IntermediateAck *ack);
    int  getPredecessor(int packetId);

    // Chiamato dalla sorgente quando genera un pacchetto
    void onPacketSent(int packetId, simtime_t sendTime);

    // Chiamato da TrustAwareAODV quando invia un beacon
    void onBeaconSent(int beaconBytes);
};
#endif // TRUSTMONITOR_H_
