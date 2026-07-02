//
// EcdsaAodvControlPackets.msg
// AODV Control Packets con campi ECDSA (timestamp + nonce)
//

namespace networkprojectvanet::replay::replay_mitigation_ecdsa;

enum AodvControlPacketType
{
    RREQ = 1;
    RREP = 2;
    RERR = 3;
    RREPACK = 4;
};

//
// Base class for AODV Control Packets
//
packet AodvControlPacket
{
    int packetType @enum(AodvControlPacketType);
}

//
// RREQ with ECDSA fields
//
packet Rreq extends AodvControlPacket
{
    packetType = RREQ;
    
    // AODV standard fields (string per evitare problemi di import)
    bool joinFlag = false;
    bool repairFlag = false;
    bool gratuitousRREPFlag = false;
    bool destOnlyFlag = false;
    bool unknownSeqNumFlag = false;
    unsigned int hopCount = 0;
    uint32_t rreqId;
    string destAddr;              // L3Address come string
    uint32_t destSeqNum;
    string originatorAddr;        // L3Address come string
    uint32_t originatorSeqNum;
    
    // ECDSA security fields
    string senderId;
    uint64_t timestamp;
    uint32_t nonce;
    string signature;
}

//
// RREP with ECDSA fields
//
packet Rrep extends AodvControlPacket
{
    packetType = RREP;
    
    // AODV standard fields
    bool repairFlag = false;
    bool ackRequiredFlag = false;
    unsigned int prefixSize = 0;
    unsigned int hopCount = 0;
    string destAddr;              // L3Address come string
    uint32_t destSeqNum;
    string originatorAddr;        // L3Address come string
    simtime_t lifeTime;
    
    // ECDSA security fields
    string senderId;
    uint64_t timestamp;
    uint32_t nonce;
    string signature;
}

//
// RERR (minimal, no ECDSA)
//
packet Rerr extends AodvControlPacket
{
    packetType = RERR;
    bool noDeleteFlag = false;
    unsigned int destCount = 0;
    string unreachableNodes[];
    uint32_t unreachableSeqNums[];
}

//
// RREPACK (minimal, no ECDSA)
//
packet RrepAck extends AodvControlPacket
{
    packetType = RREPACK;
}