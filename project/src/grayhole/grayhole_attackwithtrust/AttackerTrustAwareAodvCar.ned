package networkprojectvanet.grayhole.grayhole_attackwithtrust;

import inet.node.inet.AdhocHost;
import networkprojectvanet.grayhole.grayhole_attackwithtrust.AttackerTrustAwareAODV;
import networkprojectvanet.grayhole.grayhole_mitigation.TrustManager;
import networkprojectvanet.grayhole.grayhole_mitigation.TrustMonitor;

module AttackerTrustAwareAodvCar extends AdhocHost
{
    parameters:
        @display("i=device/cellphone");
        routing.typename = "AttackerTrustAwareAODV";

    submodules:
        routing: AttackerTrustAwareAODV {
            @display("p=825,226");
        }
        trustManager: TrustManager {
            @display("p=700,300");
        }
        trustMonitor: TrustMonitor {
            @display("p=850,300");
        }

    connections allowunconnected:
        routing.socketOut --> at.in++;
        routing.socketIn <-- at.out++;
}