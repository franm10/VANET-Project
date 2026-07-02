package networkprojectvanet.replay.replay_mitigation_rsa.map_table;

import inet.routing.aodv.Aodv;

simple RsaAodvReplay extends Aodv
{
    parameters:
        @class(RsaAodvReplay);
        string attackerNodeIds = default("");
        string nodeId             = default("nodeX");
        string privateKeyPem      = default("");
        string pubKeyMapJson      = default("/home/giada/NetworkProjectVanet/src/replay/replay_mitigation_rsa/keys/pubkeys.json");
        double freshnessWindow @unit(s) = default(2s);
        double entryTimeout @unit(s) = default(30s);
        double gcInterval @unit(s) = default(10s);
        int maxCacheSize = default(1000);
		 @signal[detectionTP](type=long);
        @signal[detectionFP](type=long);
        @signal[detectionFN](type=long);
        @signal[detectionTN](type=long);

        @statistic[detectionTP](source=detectionTP; record=count);
        @statistic[detectionFP](source=detectionFP; record=count);
        @statistic[detectionFN](source=detectionFN; record=count);
        @statistic[detectionTN](source=detectionTN; record=count);
}