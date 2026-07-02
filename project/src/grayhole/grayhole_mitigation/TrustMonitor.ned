package networkprojectvanet.grayhole.grayhole_mitigation;

simple TrustMonitor
{
    parameters:
        double cleanupTimeout @unit(s) = default(10s);
        int messageLength = default(512);
        double warmupDuration = default(10.0);

        @display("i=block/monitor");

        @signal[pdr](type=double);
        @signal[per](type=double);
        @signal[throughput](type=double);
        @signal[endToEndDelay](type=double);
        @signal[beaconOverheadBytes](type=double);

        @statistic[pdr](title="Packet Delivery Ratio";
            source=pdr; record=last);
        @statistic[per](title="Packet Error Rate";
            source=per; record=last);
        @statistic[throughput](title="Throughput (bytes/s)";
            source=throughput; record=last);
        @statistic[endToEndDelay](title="End-to-End Delay";
            source=endToEndDelay; record=vector,stats);
        @statistic[beaconOverheadBytes](title="Beacon Overhead (bytes)";
            source=beaconOverheadBytes; record=last);
}