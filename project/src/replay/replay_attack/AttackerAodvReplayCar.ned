//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package networkprojectvanet.replay.replay_attack;

import inet.node.inet.AdhocHost;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;
import networkprojectvanet.replay.replay_attack.AodvReplay;

//Nodo wireless che monta AodvReplay al posto dello stack Aodv tradizionale.

module AttackerAodvReplayCar extends AdhocHost
{
    parameters:
        @display("i=veins/node/car,red;is=vs");

    submodules:
        routing: AodvReplay {
            @display("p=558,307");
        }
    connections allowunconnected:
        routing.socketOut --> at.in++;
        routing.socketIn  <-- at.out++;
   
        //routing.socketOut --> udp.ipIn;
        //routing.socketIn  <-- udp.ipOut;
}
