package networkprojectvanet.replay.replay_mitigation_ecdsa.map_table;

import inet.node.inet.AdhocHost;

module EcdsaAodvCar extends AdhocHost
{
    parameters:
        @display("i=veins/node/car;is=vs");
		routing.attackerNodeIds = default("");
		
    submodules:
        routing: EcdsaAodvReplay {
            @display("p=558,307");
        }

    connections allowunconnected:
        routing.socketOut --> at.in++;
        routing.socketIn  <-- at.out++;
}