package networkprojectvanet.replay.replay_mitigation_rsa.bloom_filter;

import inet.routing.aodv.Aodv;

simple RsaBloomFilterAodvReplay extends Aodv
{
    parameters:
        @class(RsaBloomFilterAodvReplay);
        string nodeId             = default("nodeX");
        string privateKeyPem      = default("");
        string pubKeyMapJson      = default("/home/giada/NetworkProjectVanet/src/replay/replay_mitigation_rsa/keys/pubkeys.json");
        double freshnessWindow @unit(s) = default(2s);
        
        int bloomFilterSize = default(10000);           // m bits
        int bloomFilterHashes = default(3);             // k hash functions
        double bloomFilterClearInterval @unit(s) = default(10s);
}