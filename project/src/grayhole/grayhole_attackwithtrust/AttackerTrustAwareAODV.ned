package networkprojectvanet.grayhole.grayhole_attackwithtrust;

import networkprojectvanet.grayhole.grayhole_mitigation.TrustAwareAODV;

simple AttackerTrustAwareAODV extends TrustAwareAODV
{
    parameters:
        @class(AttackerTrustAwareAODV);
        int droppingRate = default(50);  // percentuale drop (0-100)

        @display("i=block/monitor");
}