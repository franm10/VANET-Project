package networkprojectvanet.grayhole.grayhole_attack;

import inet.routing.aodv.Aodv;


simple AodvGrayhole extends Aodv
{
    parameters:
        @class(AodvGrayhole);
        int droppingRate = default(50);
}