#ifndef TRUSTMANAGER_H_
#define TRUSTMANAGER_H_

#include <omnetpp.h>
#include <map>
#include <set>
#include "inet/common/INETDefs.h"
#include "ITrustBeaconSender.h"

using namespace omnetpp;

class TrustMonitor;

// ============================================================================
// Statistiche per un singolo link (me → neighbor)
// ============================================================================
struct LinkStats {
    int sent      = 0;
    int ackedLegit = 0;
};

// ============================================================================
// Entry per trust ricevuta da un terzo nodo via beacon
// ============================================================================
struct ReceivedTrustEntry {
    double    trustValue;
    simtime_t timestamp;
};

// ============================================================================
// TrustManager
// ============================================================================
class TrustManager : public cSimpleModule {

private:
    std::map<int, LinkStats> linkStats;
    std::map<int, std::map<int, ReceivedTrustEntry>> receivedTrust;

    double initialTrust;

    ITrustBeaconSender *aodvModule  = nullptr;
    TrustMonitor       *trustMonitor = nullptr;

    simsignal_t trustLocalSignal;
    simsignal_t trustGlobalSignal;

    // Detection
    simsignal_t detectionTPSignal;
    simsignal_t detectionFPSignal;
    simsignal_t detectionFNSignal;
    simsignal_t detectionTNSignal;

    std::set<int> attackerNodeIds; // popolato automaticamente in finish()
    double        trustThreshold;

    // Auto-detect attaccanti dal tipo di modulo
    void discoverAttackerNodeIds();

protected:
    virtual int  numInitStages() const override { return 2; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

public:
    double getLocalTrust(int neighborId);
    double getGlobalTrust(int neighborId);
    void onFinalAckUpdate(int neighborId, bool mic);
    void updateFromDistributedInfo(int reportingNodeId,
                                   int aboutNodeId,
                                   double reportedTrust);
    bool hasSentData(int neighborId) const;

    void setAodvModule(ITrustBeaconSender *aodv) { aodvModule = aodv; }
    void setTrustMonitor(TrustMonitor *tm)        { trustMonitor = tm; }

    // Mantenuto per compatibilità ma non più necessario
    void setAttackerNodeIds(const std::set<int>& ids) { attackerNodeIds = ids; }
};

#endif // TRUSTMANAGER_H_
