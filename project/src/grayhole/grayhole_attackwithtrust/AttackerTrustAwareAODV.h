#ifndef ATTACKERTRUSTAWAREAODV_H_
#define ATTACKERTRUSTAWAREAODV_H_

#include "../grayhole_mitigation/TrustAwareAODV.h"
#include "../grayhole_mitigation/messages/TrustBeacon_m.h"
#include "../grayhole_mitigation/messages/IntermediateAck_m_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/packet/Packet.h"

using namespace inet;
using namespace inet::aodv;

// ============================================================================
// AttackerTrustAwareAODV
// Estende TrustAwareAODV (nodo onesto completo) aggiungendo:
// - drop selettivo dei pacchetti (droppingRate configurabile)
// - generazione di RREP falsi (seqnum=999999, hopCount=0)
// - generazione di ACK falso con MIC=0 quando droppa
// Quando NON droppa si comporta esattamente come TrustAwareAODV.
// ============================================================================
class AttackerTrustAwareAODV : public TrustAwareAODV {

private:
    // Parametro drop
    int droppingRate;

    // Statistiche
    long totalPacketsReceived;
    long packetsDropped;
    long packetsForwarded;

    // Metodi privati
    bool shouldDropPacket();
    const Ptr<Rrep> createFakeRREP(const Ptr<Rreq>& rreq,
                                    IRoute *destRoute,
                                    IRoute *originatorRoute,
                                    const L3Address& lastHopAddr);

protected:
    virtual int  numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;

    // Override: drop selettivo + ACK falso MIC=0
    virtual INetfilter::IHook::Result datagramForwardHook(
        Packet *datagram) override;

    // Override: genera RREP falso invece di propagare la RREQ
    virtual void handleRREQ(const Ptr<Rreq>& rreq,
                            const L3Address& sourceAddr,
                            unsigned int timeToLive) override;

    // handleRREP: eredita da TrustAwareAODV senza modifiche
    // (popola neighborCandidates + chiama Aodv::handleRREP)
};

#endif // ATTACKERTRUSTAWAREAODV_H_
