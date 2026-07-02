#ifndef TRUSTAWAREAODV_H_
#define TRUSTAWAREAODV_H_

#include "inet/routing/aodv/Aodv.h"
#include "TrustManager.h"
#include "TrustMonitor.h"
#include "messages/IntermediateAck_m_m.h"
#include "inet/common/packet/Packet.h"
#include "messages/TrustBeacon_m.h"
#include "ITrustBeaconSender.h"

using namespace inet;
using namespace inet::aodv;

// ============================================================================
// Struttura per un candidato next-hop verso una destinazione
// ============================================================================
struct CandidateEntry {
    int       declaredHopCount;
    simtime_t lastUpdate;
};

// ============================================================================
// TrustAwareAODV
// ============================================================================
class TrustAwareAODV : public Aodv, public ITrustBeaconSender {

private:
    TrustManager *trustManager = nullptr;
    TrustMonitor *trustMonitor = nullptr;

    // Parametri
    bool enableTrustRouting;   // false in warmup, true in fase attiva
    int  warmupPacketLimit;    // soglia warmup (default 30)

    // Candidati next-hop per destinazione
    // destinationId → { neighborId → CandidateEntry }
    std::map<int, std::map<int, CandidateEntry>> neighborCandidates;

    // Timer per cleanup periodico di neighborCandidates
    cMessage *candidatesCleanupTimer = nullptr;

    // Metodi privati
    void cleanupCandidates();
    int  getNodeIdFromAddress(const L3Address& addr);

    // Selezione next-hop basata su score (1 - T_glob) * declaredHop
    // Restituisce L3Address::UNSPECIFIED se lista vuota dopo filtro
    L3Address selectBestCandidate(int destId);

    // Verifica se il candidato è attualmente un vicino a 1-hop
    bool isCurrentOneHopNeighbor(int neighborId);

protected:
    virtual int  numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // AODV overrides
    virtual void handleRREP(const Ptr<Rrep>& rrep,
                            const L3Address& sourceAddr) override;
    virtual void handleRREQ(const Ptr<Rreq>& rreq,
                            const L3Address& sourceAddr,
                            unsigned int timeToLive) override;

    // Netfilter hooks
    virtual INetfilter::IHook::Result datagramForwardHook(
        Packet *datagram) override;
    virtual INetfilter::IHook::Result datagramLocalInHook(
        Packet *datagram) override;
    virtual INetfilter::IHook::Result datagramLocalOutHook(
        Packet *datagram) override;

public:
    // --------------------------------------------------------
    // Contatori di flusso (statici, condivisi tra tutti i nodi)
    // flowKey = (srcLastOctet << 8 | destLastOctet)
    // --------------------------------------------------------
    static std::map<int, int> flowPacketCounter;

    // Mapping packetId → predecessore (per propagazione ACK)
    static std::map<int, int> globalPacketPredecessor;

    // Mapping omnetId → uniquePacketId
    static std::map<int, int> omnetIdToUniqueId;

    // --------------------------------------------------------
    // ACK finale (MIC 0/1) — chiamato da TrustMonitor
    // --------------------------------------------------------
    void onFinalAckReceived(IntermediateAck *ack);

    // --------------------------------------------------------
    // Trust beacon
    // --------------------------------------------------------
    void broadcastTrustBeacon(int aboutNodeId,
                              double trustValue) override;
    void receiveTrustBeacon(TrustBeacon *beacon);

    // --------------------------------------------------------
    // Utility pubblica (usata anche da AttackerTrustAwareAODV)
    // --------------------------------------------------------
    cModule* getSafeRoutingModule(int targetNodeId);
    int      getOrCreatePacketId(Packet *datagram);
};

#endif // TRUSTAWAREAODV_H_
