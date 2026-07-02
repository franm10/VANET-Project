package networkprojectvanet.replay.replay_mitigation_ecdsa.map_table;

import inet.routing.aodv.Aodv;

simple EcdsaAodvReplay extends Aodv
{
    parameters:
        @class(EcdsaAodvReplay);
 		string attackerNodeIds = default("");
        string nodeId = default("nodeX");
        string ecdsaPrivateKeyPem = default("");
        string ecdsaPubKeyMapJson = default(
            "/home/giada/NetworkProjectVanet/src/replay/replay_mitigation_ecdsa/keys/pubkeys_ecdsa.json");

        double freshnessWindow @unit(s) = default(2s);
        double clockTolerance  @unit(s) = default(0.1s);
        double entryTimeout    @unit(s) = default(30s);
        double gcInterval      @unit(s) = default(10s);
        int    maxCacheSize             = default(1000);

        @display("i=block/routing");
        @signal[detectionTP](type=long);
        @signal[detectionFP](type=long);
        @signal[detectionFN](type=long);
        @signal[detectionTN](type=long);

        @statistic[detectionTP](source=detectionTP; record=count);
        @statistic[detectionFP](source=detectionFP; record=count);
        @statistic[detectionFN](source=detectionFN; record=count);
        @statistic[detectionTN](source=detectionTN; record=count);
}