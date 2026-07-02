package networkprojectvanet.replay.replay_mitigation_ecdsa.bloom_filter;

import inet.routing.aodv.Aodv;

simple EcdsaBloomFilterAodvReplay extends Aodv
{
    parameters:
        // Parametri ECDSA
        string nodeId = default("");
        string ecdsaPrivateKeyPem = default("");
        string ecdsaPubKeyMapJson = default("");
        
        // Parametri Bloom Filter
        int bloomFilterSize = default(10000);
        int bloomFilterHashes = default(3);
        double bloomFilterClearInterval @unit(s) = default(10s);
}