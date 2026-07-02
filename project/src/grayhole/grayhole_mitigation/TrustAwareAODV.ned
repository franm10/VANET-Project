package networkprojectvanet.grayhole.grayhole_mitigation;

import inet.routing.aodv.Aodv;

simple TrustAwareAODV extends Aodv
{
    parameters:
        @class(TrustAwareAODV);
        bool enableTrustRouting = default(true);
        int  warmupPacketLimit  = default(30);

        @display("i=block/monitor");
}