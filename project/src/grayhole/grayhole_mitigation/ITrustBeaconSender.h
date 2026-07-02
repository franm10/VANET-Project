#ifndef ITRUSTBEACONSENDER_H

#define ITRUSTBEACONSENDER_H

class ITrustBeaconSender {

public:

    virtual void broadcastTrustBeacon(int aboutNodeId, double trustValue) = 0;

    virtual ~ITrustBeaconSender() {}

};

#endif
