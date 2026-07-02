package networkprojectvanet.replay.replay_mitigation_ecdsa.bloom_filter;

import inet.node.inet.AdhocHost;

module EcdsaBloomFilterAodvCar extends AdhocHost
{
    parameters:
        @display("i=veins/node/car;is=vs");
        
    submodules:
        routing: EcdsaBloomFilterAodvReplay {
            @display("p=558,307");
        }
        
    connections allowunconnected:
        routing.socketOut --> at.in++;
        routing.socketIn  <-- at.out++;
}