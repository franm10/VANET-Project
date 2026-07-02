package networkprojectvanet.replay.replay_mitigation_rsa.map_table;

import inet.node.inet.AdhocHost;
import networkprojectvanet.veins_inet.AodvCar;
import inet.routing.aodv.Aodv;

module RsaAodvCar extends AdhocHost
{
    parameters:
        @display("i=veins/node/car;is=vs");
		  routing.attackerNodeIds = default("");
		  
    submodules:
        routing: RsaAodvReplay {
            @display("p=560,310");
        }
    connections allowunconnected:
        routing.socketOut --> at.in++;
        routing.socketIn  <-- at.out++;
}