#ifndef __VANETPROJECT_AODVGRAYHOLE_H_
#define __VANETPROJECT_AODVGRAYHOLE_H_

#include "inet/routing/aodv/Aodv.h"
#include "inet/networklayer/contract/INetfilter.h"

using namespace inet;
using namespace aodv;

class AodvGrayhole : public Aodv
{
    public:
        AodvGrayhole();
        virtual ~AodvGrayhole();

    protected:
        virtual void finish() override;
        // Override dei metodi
        virtual void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

        // Metodi AODV da sovrascrivere
        virtual void handleRREQ(const Ptr<Rreq>& rreq, const L3Address& sourceAddr, unsigned int timeToLive) override;

        // Override del metodo per intercettare pacchetti in forward
        virtual Result datagramForwardHook(Packet *datagram) override;

        // Metodo helper per creare RREP falsi
        virtual const Ptr<Rrep> createFakeRREP(const Ptr<Rreq>& rreq, IRoute *destRoute, IRoute *originatorRoute, const L3Address& lastHopAddr);

        // Metodo per decidere se droppare un pacchetto
        virtual bool shouldDropPacket();

        // Metodo per stampare la routing table
        virtual void printRoutingTable();

    private:
        // Parametro di configurazione: percentuale di pacchetti da droppare (0-100)
        int droppingRate;

        // Statistiche
        long totalPacketsReceived;
        long packetsDropped;
        long packetsForwarded;
};

#endif // ifndef __VANETPROJECT_AODVGRAYHOLE_H_
