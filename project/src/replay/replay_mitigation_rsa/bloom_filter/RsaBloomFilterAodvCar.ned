package networkprojectvanet.replay.replay_mitigation_rsa.bloom_filter;

import inet.node.inet.AdhocHost;
import networkprojectvanet.veins_inet.AodvCar;
import inet.routing.aodv.Aodv;


module RsaBloomFilterAodvCar extends AdhocHost
{
    parameters:
        @display("i=veins/node/car;is=vs");

    submodules:
        routing: RsaBloomFilterAodvReplay {
            @display("p=560,310");
        }
    connections allowunconnected:
        routing.socketOut --> at.in++;
        routing.socketIn  <-- at.out++;
}