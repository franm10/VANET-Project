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

import inet.routing.aodv.Aodv;

simple AodvReplay extends Aodv
{
    parameters:
        @class(AodvReplay);

        //istante in cui parte la fase di replay 
        double replayStart @unit(s) = default(15s);

        // latenza introdotta fra 2 pacchetti consecutivi */
        double replayLatency @unit(s) = default(0.3s);

        // finestra (in secondi) di bufferizzazione */
        double bufferWindow @unit(s) = default(3s);

        // opzionale: IPv4 numerico del bersaglio (-1 → tutti) */
        int targetIpv4 = default(-1);
        
        bool enableForgery = default(true);
        bool enablePureReplay = default(true);
        double pureReplayDelay @unit(s) = default(0.1s);

     
          
}

