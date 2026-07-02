//
// ACK finale per il meccanismo di trust basato su delivery ratio
//
cplusplus {{
#include "inet/common/INETDefs.h"
}}

packet IntermediateAck
{
    int    originalPacketId;   // ID del pacchetto dati originale
    bool   mic;                // MIC=1 ACK legittimo, MIC=0 ACK falso (drop)
    int    senderNodeId;       // Chi invia questo ACK
    int    previousHopNodeId;  // A chi propagare l'ACK (predecessore nel path)
    double timestamp;          // Timestamp di creazione (per log/debug)
}