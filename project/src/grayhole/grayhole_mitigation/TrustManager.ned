package networkprojectvanet.grayhole.grayhole_mitigation;

simple TrustManager
{
    parameters:
        double initialTrust = default(1.0);
        double trustThreshold = default(0.5);
        string attackerNodeIds = default("");

        @display("i=block/control");

        @signal[trustLocal](type=double);
        @signal[trustGlobal](type=double);
        @signal[detectionTP](type=double);
        @signal[detectionFP](type=double);
        @signal[detectionFN](type=double);
        @signal[detectionTN](type=double);

        @statistic[trustLocal](title="Local Trust";
            source=trustLocal; record=vector,stats);
        @statistic[trustGlobal](title="Global Trust";
            source=trustGlobal; record=vector,stats);
        @statistic[detectionTP](title="True Positives";
            source=detectionTP; record=sum);
        @statistic[detectionFP](title="False Positives";
            source=detectionFP; record=sum);
        @statistic[detectionFN](title="False Negatives";
            source=detectionFN; record=sum);
        @statistic[detectionTN](title="True Negatives";
            source=detectionTN; record=sum);
}