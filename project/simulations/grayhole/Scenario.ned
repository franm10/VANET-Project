package networkprojectvanet.simulations.grayhole;

import inet.physicallayer.unitdisk.UnitDiskRadioMedium;
import inet.node.inet.INetworkNode;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import networkprojectvanet.veins_inet.VeinsInetManager;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;

// Tutti i tipi di nodo necessari per i 6 scenari
import networkprojectvanet.veins_inet.AodvCar;
import networkprojectvanet.grayhole.grayhole_attack.AttackerAodvGrayholeCar;
import networkprojectvanet.grayhole.grayhole_mitigation.TrustAwareAodvCar;
import networkprojectvanet.grayhole.grayhole_attackwithtrust.AttackerTrustAwareAodvCar;


network Scenario
{
    parameters:
        @display("bgb=4000,5000");

    submodules:
        visualizer: IntegratedCanvasVisualizer {
            @display("p=38,200");
        }
        radioMedium: UnitDiskRadioMedium {
            @display("p=38,600");
        }
        manager: VeinsInetManager {
            @display("p=38,1000");
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=38,1400");
        }

        // Attaccanti statici — la dimensione viene configurata
        // dall'omnetpp.ini per ogni scenario:
        // 0 = nessun attaccante, 2 = due attaccanti
        attacker_car[0]: AttackerAodvGrayholeCar;
        attacker_trust_car[0]: AttackerTrustAwareAodvCar;
}