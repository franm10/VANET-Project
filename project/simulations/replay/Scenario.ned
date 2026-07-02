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

package networkprojectvanet.simulations.replay;

import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.physicallayer.unitdisk.UnitDiskRadioMedium;
import inet.node.inet.INetworkNode;
import networkprojectvanet.veins_inet.AodvCar;
import networkprojectvanet.replay.replay_attack.AttackerAodvReplayCar;
import networkprojectvanet.veins_inet.VeinsInetManager;
import inet.environment.common.PhysicalEnvironment;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;
import org.car2x.veins.visualizer.roads.RoadsOsgVisualizer;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

network Scenario
{
    parameters:
        @display("bgb=4000,5000");
        //bool withStaticAttacker = default(false);
    
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            @display("p=38,200");
        }
        radioMedium: UnitDiskRadioMedium {//Ieee80211ScalarRadioMedium;
            @display("p=38,600");
        }
        manager: VeinsInetManager {
            @display("p=38,1000");
        }
        
        // attacker_car[0]: AttackerAodvReplayCar if withStaticAttacker;
        attacker_car[0]: AttackerAodvReplayCar;
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=38,1400");
        }
        
}
