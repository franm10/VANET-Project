//
// Generated file, do not edit! Created by nedtool 5.6 from replay/replay_mitigation_ecdsa/messages/EcdsaAodvControlPackets.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "EcdsaAodvControlPackets_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}

namespace networkprojectvanet {
namespace replay {
namespace replay_mitigation_ecdsa {

// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacketType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacketType"));
    e->insert(RREQ, "RREQ");
    e->insert(RREP, "RREP");
    e->insert(RERR, "RERR");
    e->insert(RREPACK, "RREPACK");
)

Register_Class(AodvControlPacket)

AodvControlPacket::AodvControlPacket(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

AodvControlPacket::AodvControlPacket(const AodvControlPacket& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

AodvControlPacket::~AodvControlPacket()
{
}

AodvControlPacket& AodvControlPacket::operator=(const AodvControlPacket& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void AodvControlPacket::copy(const AodvControlPacket& other)
{
    this->packetType = other.packetType;
}

void AodvControlPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->packetType);
}

void AodvControlPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->packetType);
}

int AodvControlPacket::getPacketType() const
{
    return this->packetType;
}

void AodvControlPacket::setPacketType(int packetType)
{
    this->packetType = packetType;
}

class AodvControlPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_packetType,
    };
  public:
    AodvControlPacketDescriptor();
    virtual ~AodvControlPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(AodvControlPacketDescriptor)

AodvControlPacketDescriptor::AodvControlPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket)), "omnetpp::cPacket")
{
    propertynames = nullptr;
}

AodvControlPacketDescriptor::~AodvControlPacketDescriptor()
{
    delete[] propertynames;
}

bool AodvControlPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AodvControlPacket *>(obj)!=nullptr;
}

const char **AodvControlPacketDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AodvControlPacketDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AodvControlPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int AodvControlPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_packetType
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *AodvControlPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "packetType",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int AodvControlPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'p' && strcmp(fieldName, "packetType") == 0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AodvControlPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_packetType
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **AodvControlPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_packetType: {
            static const char *names[] = { "enum", "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *AodvControlPacketDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_packetType:
            if (!strcmp(propertyname, "enum")) return "AodvControlPacketType";
            if (!strcmp(propertyname, "enum")) return "networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacketType";
            return nullptr;
        default: return nullptr;
    }
}

int AodvControlPacketDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AodvControlPacket *pp = (AodvControlPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AodvControlPacketDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AodvControlPacket *pp = (AodvControlPacket *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AodvControlPacketDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AodvControlPacket *pp = (AodvControlPacket *)object; (void)pp;
    switch (field) {
        case FIELD_packetType: return enum2string(pp->getPacketType(), "networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacketType");
        default: return "";
    }
}

bool AodvControlPacketDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AodvControlPacket *pp = (AodvControlPacket *)object; (void)pp;
    switch (field) {
        case FIELD_packetType: pp->setPacketType((networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacketType)string2enum(value, "networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacketType")); return true;
        default: return false;
    }
}

const char *AodvControlPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *AodvControlPacketDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AodvControlPacket *pp = (AodvControlPacket *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(Rreq)

Rreq::Rreq(const char *name, short kind) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(name, kind)
{
    this->setPacketType(RREQ);

}

Rreq::Rreq(const Rreq& other) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(other)
{
    copy(other);
}

Rreq::~Rreq()
{
}

Rreq& Rreq::operator=(const Rreq& other)
{
    if (this == &other) return *this;
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::operator=(other);
    copy(other);
    return *this;
}

void Rreq::copy(const Rreq& other)
{
    this->joinFlag = other.joinFlag;
    this->repairFlag = other.repairFlag;
    this->gratuitousRREPFlag = other.gratuitousRREPFlag;
    this->destOnlyFlag = other.destOnlyFlag;
    this->unknownSeqNumFlag = other.unknownSeqNumFlag;
    this->hopCount = other.hopCount;
    this->rreqId = other.rreqId;
    this->destAddr = other.destAddr;
    this->destSeqNum = other.destSeqNum;
    this->originatorAddr = other.originatorAddr;
    this->originatorSeqNum = other.originatorSeqNum;
    this->senderId = other.senderId;
    this->timestamp = other.timestamp;
    this->nonce = other.nonce;
    this->signature = other.signature;
}

void Rreq::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimPack(b);
    doParsimPacking(b,this->joinFlag);
    doParsimPacking(b,this->repairFlag);
    doParsimPacking(b,this->gratuitousRREPFlag);
    doParsimPacking(b,this->destOnlyFlag);
    doParsimPacking(b,this->unknownSeqNumFlag);
    doParsimPacking(b,this->hopCount);
    doParsimPacking(b,this->rreqId);
    doParsimPacking(b,this->destAddr);
    doParsimPacking(b,this->destSeqNum);
    doParsimPacking(b,this->originatorAddr);
    doParsimPacking(b,this->originatorSeqNum);
    doParsimPacking(b,this->senderId);
    doParsimPacking(b,this->timestamp);
    doParsimPacking(b,this->nonce);
    doParsimPacking(b,this->signature);
}

void Rreq::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->joinFlag);
    doParsimUnpacking(b,this->repairFlag);
    doParsimUnpacking(b,this->gratuitousRREPFlag);
    doParsimUnpacking(b,this->destOnlyFlag);
    doParsimUnpacking(b,this->unknownSeqNumFlag);
    doParsimUnpacking(b,this->hopCount);
    doParsimUnpacking(b,this->rreqId);
    doParsimUnpacking(b,this->destAddr);
    doParsimUnpacking(b,this->destSeqNum);
    doParsimUnpacking(b,this->originatorAddr);
    doParsimUnpacking(b,this->originatorSeqNum);
    doParsimUnpacking(b,this->senderId);
    doParsimUnpacking(b,this->timestamp);
    doParsimUnpacking(b,this->nonce);
    doParsimUnpacking(b,this->signature);
}

bool Rreq::getJoinFlag() const
{
    return this->joinFlag;
}

void Rreq::setJoinFlag(bool joinFlag)
{
    this->joinFlag = joinFlag;
}

bool Rreq::getRepairFlag() const
{
    return this->repairFlag;
}

void Rreq::setRepairFlag(bool repairFlag)
{
    this->repairFlag = repairFlag;
}

bool Rreq::getGratuitousRREPFlag() const
{
    return this->gratuitousRREPFlag;
}

void Rreq::setGratuitousRREPFlag(bool gratuitousRREPFlag)
{
    this->gratuitousRREPFlag = gratuitousRREPFlag;
}

bool Rreq::getDestOnlyFlag() const
{
    return this->destOnlyFlag;
}

void Rreq::setDestOnlyFlag(bool destOnlyFlag)
{
    this->destOnlyFlag = destOnlyFlag;
}

bool Rreq::getUnknownSeqNumFlag() const
{
    return this->unknownSeqNumFlag;
}

void Rreq::setUnknownSeqNumFlag(bool unknownSeqNumFlag)
{
    this->unknownSeqNumFlag = unknownSeqNumFlag;
}

unsigned int Rreq::getHopCount() const
{
    return this->hopCount;
}

void Rreq::setHopCount(unsigned int hopCount)
{
    this->hopCount = hopCount;
}

uint32_t Rreq::getRreqId() const
{
    return this->rreqId;
}

void Rreq::setRreqId(uint32_t rreqId)
{
    this->rreqId = rreqId;
}

const char * Rreq::getDestAddr() const
{
    return this->destAddr.c_str();
}

void Rreq::setDestAddr(const char * destAddr)
{
    this->destAddr = destAddr;
}

uint32_t Rreq::getDestSeqNum() const
{
    return this->destSeqNum;
}

void Rreq::setDestSeqNum(uint32_t destSeqNum)
{
    this->destSeqNum = destSeqNum;
}

const char * Rreq::getOriginatorAddr() const
{
    return this->originatorAddr.c_str();
}

void Rreq::setOriginatorAddr(const char * originatorAddr)
{
    this->originatorAddr = originatorAddr;
}

uint32_t Rreq::getOriginatorSeqNum() const
{
    return this->originatorSeqNum;
}

void Rreq::setOriginatorSeqNum(uint32_t originatorSeqNum)
{
    this->originatorSeqNum = originatorSeqNum;
}

const char * Rreq::getSenderId() const
{
    return this->senderId.c_str();
}

void Rreq::setSenderId(const char * senderId)
{
    this->senderId = senderId;
}

uint64_t Rreq::getTimestamp() const
{
    return this->timestamp;
}

void Rreq::setTimestamp(uint64_t timestamp)
{
    this->timestamp = timestamp;
}

uint32_t Rreq::getNonce() const
{
    return this->nonce;
}

void Rreq::setNonce(uint32_t nonce)
{
    this->nonce = nonce;
}

const char * Rreq::getSignature() const
{
    return this->signature.c_str();
}

void Rreq::setSignature(const char * signature)
{
    this->signature = signature;
}

class RreqDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_joinFlag,
        FIELD_repairFlag,
        FIELD_gratuitousRREPFlag,
        FIELD_destOnlyFlag,
        FIELD_unknownSeqNumFlag,
        FIELD_hopCount,
        FIELD_rreqId,
        FIELD_destAddr,
        FIELD_destSeqNum,
        FIELD_originatorAddr,
        FIELD_originatorSeqNum,
        FIELD_senderId,
        FIELD_timestamp,
        FIELD_nonce,
        FIELD_signature,
    };
  public:
    RreqDescriptor();
    virtual ~RreqDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RreqDescriptor)

RreqDescriptor::RreqDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(networkprojectvanet::replay::replay_mitigation_ecdsa::Rreq)), "networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket")
{
    propertynames = nullptr;
}

RreqDescriptor::~RreqDescriptor()
{
    delete[] propertynames;
}

bool RreqDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Rreq *>(obj)!=nullptr;
}

const char **RreqDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RreqDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RreqDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 15+basedesc->getFieldCount() : 15;
}

unsigned int RreqDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_joinFlag
        FD_ISEDITABLE,    // FIELD_repairFlag
        FD_ISEDITABLE,    // FIELD_gratuitousRREPFlag
        FD_ISEDITABLE,    // FIELD_destOnlyFlag
        FD_ISEDITABLE,    // FIELD_unknownSeqNumFlag
        FD_ISEDITABLE,    // FIELD_hopCount
        FD_ISEDITABLE,    // FIELD_rreqId
        FD_ISEDITABLE,    // FIELD_destAddr
        FD_ISEDITABLE,    // FIELD_destSeqNum
        FD_ISEDITABLE,    // FIELD_originatorAddr
        FD_ISEDITABLE,    // FIELD_originatorSeqNum
        FD_ISEDITABLE,    // FIELD_senderId
        FD_ISEDITABLE,    // FIELD_timestamp
        FD_ISEDITABLE,    // FIELD_nonce
        FD_ISEDITABLE,    // FIELD_signature
    };
    return (field >= 0 && field < 15) ? fieldTypeFlags[field] : 0;
}

const char *RreqDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "joinFlag",
        "repairFlag",
        "gratuitousRREPFlag",
        "destOnlyFlag",
        "unknownSeqNumFlag",
        "hopCount",
        "rreqId",
        "destAddr",
        "destSeqNum",
        "originatorAddr",
        "originatorSeqNum",
        "senderId",
        "timestamp",
        "nonce",
        "signature",
    };
    return (field >= 0 && field < 15) ? fieldNames[field] : nullptr;
}

int RreqDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'j' && strcmp(fieldName, "joinFlag") == 0) return base+0;
    if (fieldName[0] == 'r' && strcmp(fieldName, "repairFlag") == 0) return base+1;
    if (fieldName[0] == 'g' && strcmp(fieldName, "gratuitousRREPFlag") == 0) return base+2;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destOnlyFlag") == 0) return base+3;
    if (fieldName[0] == 'u' && strcmp(fieldName, "unknownSeqNumFlag") == 0) return base+4;
    if (fieldName[0] == 'h' && strcmp(fieldName, "hopCount") == 0) return base+5;
    if (fieldName[0] == 'r' && strcmp(fieldName, "rreqId") == 0) return base+6;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destAddr") == 0) return base+7;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destSeqNum") == 0) return base+8;
    if (fieldName[0] == 'o' && strcmp(fieldName, "originatorAddr") == 0) return base+9;
    if (fieldName[0] == 'o' && strcmp(fieldName, "originatorSeqNum") == 0) return base+10;
    if (fieldName[0] == 's' && strcmp(fieldName, "senderId") == 0) return base+11;
    if (fieldName[0] == 't' && strcmp(fieldName, "timestamp") == 0) return base+12;
    if (fieldName[0] == 'n' && strcmp(fieldName, "nonce") == 0) return base+13;
    if (fieldName[0] == 's' && strcmp(fieldName, "signature") == 0) return base+14;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RreqDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_joinFlag
        "bool",    // FIELD_repairFlag
        "bool",    // FIELD_gratuitousRREPFlag
        "bool",    // FIELD_destOnlyFlag
        "bool",    // FIELD_unknownSeqNumFlag
        "unsigned int",    // FIELD_hopCount
        "uint32_t",    // FIELD_rreqId
        "string",    // FIELD_destAddr
        "uint32_t",    // FIELD_destSeqNum
        "string",    // FIELD_originatorAddr
        "uint32_t",    // FIELD_originatorSeqNum
        "string",    // FIELD_senderId
        "uint64_t",    // FIELD_timestamp
        "uint32_t",    // FIELD_nonce
        "string",    // FIELD_signature
    };
    return (field >= 0 && field < 15) ? fieldTypeStrings[field] : nullptr;
}

const char **RreqDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RreqDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RreqDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Rreq *pp = (Rreq *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RreqDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Rreq *pp = (Rreq *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RreqDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Rreq *pp = (Rreq *)object; (void)pp;
    switch (field) {
        case FIELD_joinFlag: return bool2string(pp->getJoinFlag());
        case FIELD_repairFlag: return bool2string(pp->getRepairFlag());
        case FIELD_gratuitousRREPFlag: return bool2string(pp->getGratuitousRREPFlag());
        case FIELD_destOnlyFlag: return bool2string(pp->getDestOnlyFlag());
        case FIELD_unknownSeqNumFlag: return bool2string(pp->getUnknownSeqNumFlag());
        case FIELD_hopCount: return ulong2string(pp->getHopCount());
        case FIELD_rreqId: return ulong2string(pp->getRreqId());
        case FIELD_destAddr: return oppstring2string(pp->getDestAddr());
        case FIELD_destSeqNum: return ulong2string(pp->getDestSeqNum());
        case FIELD_originatorAddr: return oppstring2string(pp->getOriginatorAddr());
        case FIELD_originatorSeqNum: return ulong2string(pp->getOriginatorSeqNum());
        case FIELD_senderId: return oppstring2string(pp->getSenderId());
        case FIELD_timestamp: return uint642string(pp->getTimestamp());
        case FIELD_nonce: return ulong2string(pp->getNonce());
        case FIELD_signature: return oppstring2string(pp->getSignature());
        default: return "";
    }
}

bool RreqDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Rreq *pp = (Rreq *)object; (void)pp;
    switch (field) {
        case FIELD_joinFlag: pp->setJoinFlag(string2bool(value)); return true;
        case FIELD_repairFlag: pp->setRepairFlag(string2bool(value)); return true;
        case FIELD_gratuitousRREPFlag: pp->setGratuitousRREPFlag(string2bool(value)); return true;
        case FIELD_destOnlyFlag: pp->setDestOnlyFlag(string2bool(value)); return true;
        case FIELD_unknownSeqNumFlag: pp->setUnknownSeqNumFlag(string2bool(value)); return true;
        case FIELD_hopCount: pp->setHopCount(string2ulong(value)); return true;
        case FIELD_rreqId: pp->setRreqId(string2ulong(value)); return true;
        case FIELD_destAddr: pp->setDestAddr((value)); return true;
        case FIELD_destSeqNum: pp->setDestSeqNum(string2ulong(value)); return true;
        case FIELD_originatorAddr: pp->setOriginatorAddr((value)); return true;
        case FIELD_originatorSeqNum: pp->setOriginatorSeqNum(string2ulong(value)); return true;
        case FIELD_senderId: pp->setSenderId((value)); return true;
        case FIELD_timestamp: pp->setTimestamp(string2uint64(value)); return true;
        case FIELD_nonce: pp->setNonce(string2ulong(value)); return true;
        case FIELD_signature: pp->setSignature((value)); return true;
        default: return false;
    }
}

const char *RreqDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RreqDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Rreq *pp = (Rreq *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(Rrep)

Rrep::Rrep(const char *name, short kind) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(name, kind)
{
    this->setPacketType(RREP);

}

Rrep::Rrep(const Rrep& other) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(other)
{
    copy(other);
}

Rrep::~Rrep()
{
}

Rrep& Rrep::operator=(const Rrep& other)
{
    if (this == &other) return *this;
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::operator=(other);
    copy(other);
    return *this;
}

void Rrep::copy(const Rrep& other)
{
    this->repairFlag = other.repairFlag;
    this->ackRequiredFlag = other.ackRequiredFlag;
    this->prefixSize = other.prefixSize;
    this->hopCount = other.hopCount;
    this->destAddr = other.destAddr;
    this->destSeqNum = other.destSeqNum;
    this->originatorAddr = other.originatorAddr;
    this->lifeTime = other.lifeTime;
    this->senderId = other.senderId;
    this->timestamp = other.timestamp;
    this->nonce = other.nonce;
    this->signature = other.signature;
}

void Rrep::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimPack(b);
    doParsimPacking(b,this->repairFlag);
    doParsimPacking(b,this->ackRequiredFlag);
    doParsimPacking(b,this->prefixSize);
    doParsimPacking(b,this->hopCount);
    doParsimPacking(b,this->destAddr);
    doParsimPacking(b,this->destSeqNum);
    doParsimPacking(b,this->originatorAddr);
    doParsimPacking(b,this->lifeTime);
    doParsimPacking(b,this->senderId);
    doParsimPacking(b,this->timestamp);
    doParsimPacking(b,this->nonce);
    doParsimPacking(b,this->signature);
}

void Rrep::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->repairFlag);
    doParsimUnpacking(b,this->ackRequiredFlag);
    doParsimUnpacking(b,this->prefixSize);
    doParsimUnpacking(b,this->hopCount);
    doParsimUnpacking(b,this->destAddr);
    doParsimUnpacking(b,this->destSeqNum);
    doParsimUnpacking(b,this->originatorAddr);
    doParsimUnpacking(b,this->lifeTime);
    doParsimUnpacking(b,this->senderId);
    doParsimUnpacking(b,this->timestamp);
    doParsimUnpacking(b,this->nonce);
    doParsimUnpacking(b,this->signature);
}

bool Rrep::getRepairFlag() const
{
    return this->repairFlag;
}

void Rrep::setRepairFlag(bool repairFlag)
{
    this->repairFlag = repairFlag;
}

bool Rrep::getAckRequiredFlag() const
{
    return this->ackRequiredFlag;
}

void Rrep::setAckRequiredFlag(bool ackRequiredFlag)
{
    this->ackRequiredFlag = ackRequiredFlag;
}

unsigned int Rrep::getPrefixSize() const
{
    return this->prefixSize;
}

void Rrep::setPrefixSize(unsigned int prefixSize)
{
    this->prefixSize = prefixSize;
}

unsigned int Rrep::getHopCount() const
{
    return this->hopCount;
}

void Rrep::setHopCount(unsigned int hopCount)
{
    this->hopCount = hopCount;
}

const char * Rrep::getDestAddr() const
{
    return this->destAddr.c_str();
}

void Rrep::setDestAddr(const char * destAddr)
{
    this->destAddr = destAddr;
}

uint32_t Rrep::getDestSeqNum() const
{
    return this->destSeqNum;
}

void Rrep::setDestSeqNum(uint32_t destSeqNum)
{
    this->destSeqNum = destSeqNum;
}

const char * Rrep::getOriginatorAddr() const
{
    return this->originatorAddr.c_str();
}

void Rrep::setOriginatorAddr(const char * originatorAddr)
{
    this->originatorAddr = originatorAddr;
}

omnetpp::simtime_t Rrep::getLifeTime() const
{
    return this->lifeTime;
}

void Rrep::setLifeTime(omnetpp::simtime_t lifeTime)
{
    this->lifeTime = lifeTime;
}

const char * Rrep::getSenderId() const
{
    return this->senderId.c_str();
}

void Rrep::setSenderId(const char * senderId)
{
    this->senderId = senderId;
}

uint64_t Rrep::getTimestamp() const
{
    return this->timestamp;
}

void Rrep::setTimestamp(uint64_t timestamp)
{
    this->timestamp = timestamp;
}

uint32_t Rrep::getNonce() const
{
    return this->nonce;
}

void Rrep::setNonce(uint32_t nonce)
{
    this->nonce = nonce;
}

const char * Rrep::getSignature() const
{
    return this->signature.c_str();
}

void Rrep::setSignature(const char * signature)
{
    this->signature = signature;
}

class RrepDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_repairFlag,
        FIELD_ackRequiredFlag,
        FIELD_prefixSize,
        FIELD_hopCount,
        FIELD_destAddr,
        FIELD_destSeqNum,
        FIELD_originatorAddr,
        FIELD_lifeTime,
        FIELD_senderId,
        FIELD_timestamp,
        FIELD_nonce,
        FIELD_signature,
    };
  public:
    RrepDescriptor();
    virtual ~RrepDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RrepDescriptor)

RrepDescriptor::RrepDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(networkprojectvanet::replay::replay_mitigation_ecdsa::Rrep)), "networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket")
{
    propertynames = nullptr;
}

RrepDescriptor::~RrepDescriptor()
{
    delete[] propertynames;
}

bool RrepDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Rrep *>(obj)!=nullptr;
}

const char **RrepDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RrepDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RrepDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 12+basedesc->getFieldCount() : 12;
}

unsigned int RrepDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_repairFlag
        FD_ISEDITABLE,    // FIELD_ackRequiredFlag
        FD_ISEDITABLE,    // FIELD_prefixSize
        FD_ISEDITABLE,    // FIELD_hopCount
        FD_ISEDITABLE,    // FIELD_destAddr
        FD_ISEDITABLE,    // FIELD_destSeqNum
        FD_ISEDITABLE,    // FIELD_originatorAddr
        0,    // FIELD_lifeTime
        FD_ISEDITABLE,    // FIELD_senderId
        FD_ISEDITABLE,    // FIELD_timestamp
        FD_ISEDITABLE,    // FIELD_nonce
        FD_ISEDITABLE,    // FIELD_signature
    };
    return (field >= 0 && field < 12) ? fieldTypeFlags[field] : 0;
}

const char *RrepDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "repairFlag",
        "ackRequiredFlag",
        "prefixSize",
        "hopCount",
        "destAddr",
        "destSeqNum",
        "originatorAddr",
        "lifeTime",
        "senderId",
        "timestamp",
        "nonce",
        "signature",
    };
    return (field >= 0 && field < 12) ? fieldNames[field] : nullptr;
}

int RrepDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'r' && strcmp(fieldName, "repairFlag") == 0) return base+0;
    if (fieldName[0] == 'a' && strcmp(fieldName, "ackRequiredFlag") == 0) return base+1;
    if (fieldName[0] == 'p' && strcmp(fieldName, "prefixSize") == 0) return base+2;
    if (fieldName[0] == 'h' && strcmp(fieldName, "hopCount") == 0) return base+3;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destAddr") == 0) return base+4;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destSeqNum") == 0) return base+5;
    if (fieldName[0] == 'o' && strcmp(fieldName, "originatorAddr") == 0) return base+6;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lifeTime") == 0) return base+7;
    if (fieldName[0] == 's' && strcmp(fieldName, "senderId") == 0) return base+8;
    if (fieldName[0] == 't' && strcmp(fieldName, "timestamp") == 0) return base+9;
    if (fieldName[0] == 'n' && strcmp(fieldName, "nonce") == 0) return base+10;
    if (fieldName[0] == 's' && strcmp(fieldName, "signature") == 0) return base+11;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RrepDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_repairFlag
        "bool",    // FIELD_ackRequiredFlag
        "unsigned int",    // FIELD_prefixSize
        "unsigned int",    // FIELD_hopCount
        "string",    // FIELD_destAddr
        "uint32_t",    // FIELD_destSeqNum
        "string",    // FIELD_originatorAddr
        "omnetpp::simtime_t",    // FIELD_lifeTime
        "string",    // FIELD_senderId
        "uint64_t",    // FIELD_timestamp
        "uint32_t",    // FIELD_nonce
        "string",    // FIELD_signature
    };
    return (field >= 0 && field < 12) ? fieldTypeStrings[field] : nullptr;
}

const char **RrepDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RrepDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RrepDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Rrep *pp = (Rrep *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RrepDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Rrep *pp = (Rrep *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RrepDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Rrep *pp = (Rrep *)object; (void)pp;
    switch (field) {
        case FIELD_repairFlag: return bool2string(pp->getRepairFlag());
        case FIELD_ackRequiredFlag: return bool2string(pp->getAckRequiredFlag());
        case FIELD_prefixSize: return ulong2string(pp->getPrefixSize());
        case FIELD_hopCount: return ulong2string(pp->getHopCount());
        case FIELD_destAddr: return oppstring2string(pp->getDestAddr());
        case FIELD_destSeqNum: return ulong2string(pp->getDestSeqNum());
        case FIELD_originatorAddr: return oppstring2string(pp->getOriginatorAddr());
        case FIELD_lifeTime: return simtime2string(pp->getLifeTime());
        case FIELD_senderId: return oppstring2string(pp->getSenderId());
        case FIELD_timestamp: return uint642string(pp->getTimestamp());
        case FIELD_nonce: return ulong2string(pp->getNonce());
        case FIELD_signature: return oppstring2string(pp->getSignature());
        default: return "";
    }
}

bool RrepDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Rrep *pp = (Rrep *)object; (void)pp;
    switch (field) {
        case FIELD_repairFlag: pp->setRepairFlag(string2bool(value)); return true;
        case FIELD_ackRequiredFlag: pp->setAckRequiredFlag(string2bool(value)); return true;
        case FIELD_prefixSize: pp->setPrefixSize(string2ulong(value)); return true;
        case FIELD_hopCount: pp->setHopCount(string2ulong(value)); return true;
        case FIELD_destAddr: pp->setDestAddr((value)); return true;
        case FIELD_destSeqNum: pp->setDestSeqNum(string2ulong(value)); return true;
        case FIELD_originatorAddr: pp->setOriginatorAddr((value)); return true;
        case FIELD_senderId: pp->setSenderId((value)); return true;
        case FIELD_timestamp: pp->setTimestamp(string2uint64(value)); return true;
        case FIELD_nonce: pp->setNonce(string2ulong(value)); return true;
        case FIELD_signature: pp->setSignature((value)); return true;
        default: return false;
    }
}

const char *RrepDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RrepDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Rrep *pp = (Rrep *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(Rerr)

Rerr::Rerr(const char *name, short kind) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(name, kind)
{
    this->setPacketType(RERR);

}

Rerr::Rerr(const Rerr& other) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(other)
{
    copy(other);
}

Rerr::~Rerr()
{
    delete [] this->unreachableNodes;
    delete [] this->unreachableSeqNums;
}

Rerr& Rerr::operator=(const Rerr& other)
{
    if (this == &other) return *this;
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::operator=(other);
    copy(other);
    return *this;
}

void Rerr::copy(const Rerr& other)
{
    this->noDeleteFlag = other.noDeleteFlag;
    this->destCount = other.destCount;
    delete [] this->unreachableNodes;
    this->unreachableNodes = (other.unreachableNodes_arraysize==0) ? nullptr : new omnetpp::opp_string[other.unreachableNodes_arraysize];
    unreachableNodes_arraysize = other.unreachableNodes_arraysize;
    for (size_t i = 0; i < unreachableNodes_arraysize; i++) {
        this->unreachableNodes[i] = other.unreachableNodes[i];
    }
    delete [] this->unreachableSeqNums;
    this->unreachableSeqNums = (other.unreachableSeqNums_arraysize==0) ? nullptr : new uint32_t[other.unreachableSeqNums_arraysize];
    unreachableSeqNums_arraysize = other.unreachableSeqNums_arraysize;
    for (size_t i = 0; i < unreachableSeqNums_arraysize; i++) {
        this->unreachableSeqNums[i] = other.unreachableSeqNums[i];
    }
}

void Rerr::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimPack(b);
    doParsimPacking(b,this->noDeleteFlag);
    doParsimPacking(b,this->destCount);
    b->pack(unreachableNodes_arraysize);
    doParsimArrayPacking(b,this->unreachableNodes,unreachableNodes_arraysize);
    b->pack(unreachableSeqNums_arraysize);
    doParsimArrayPacking(b,this->unreachableSeqNums,unreachableSeqNums_arraysize);
}

void Rerr::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->noDeleteFlag);
    doParsimUnpacking(b,this->destCount);
    delete [] this->unreachableNodes;
    b->unpack(unreachableNodes_arraysize);
    if (unreachableNodes_arraysize == 0) {
        this->unreachableNodes = nullptr;
    } else {
        this->unreachableNodes = new omnetpp::opp_string[unreachableNodes_arraysize];
        doParsimArrayUnpacking(b,this->unreachableNodes,unreachableNodes_arraysize);
    }
    delete [] this->unreachableSeqNums;
    b->unpack(unreachableSeqNums_arraysize);
    if (unreachableSeqNums_arraysize == 0) {
        this->unreachableSeqNums = nullptr;
    } else {
        this->unreachableSeqNums = new uint32_t[unreachableSeqNums_arraysize];
        doParsimArrayUnpacking(b,this->unreachableSeqNums,unreachableSeqNums_arraysize);
    }
}

bool Rerr::getNoDeleteFlag() const
{
    return this->noDeleteFlag;
}

void Rerr::setNoDeleteFlag(bool noDeleteFlag)
{
    this->noDeleteFlag = noDeleteFlag;
}

unsigned int Rerr::getDestCount() const
{
    return this->destCount;
}

void Rerr::setDestCount(unsigned int destCount)
{
    this->destCount = destCount;
}

size_t Rerr::getUnreachableNodesArraySize() const
{
    return unreachableNodes_arraysize;
}

const char * Rerr::getUnreachableNodes(size_t k) const
{
    if (k >= unreachableNodes_arraysize) throw omnetpp::cRuntimeError("Array of size unreachableNodes_arraysize indexed by %lu", (unsigned long)k);
    return this->unreachableNodes[k].c_str();
}

void Rerr::setUnreachableNodesArraySize(size_t newSize)
{
    omnetpp::opp_string *unreachableNodes2 = (newSize==0) ? nullptr : new omnetpp::opp_string[newSize];
    size_t minSize = unreachableNodes_arraysize < newSize ? unreachableNodes_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        unreachableNodes2[i] = this->unreachableNodes[i];
    delete [] this->unreachableNodes;
    this->unreachableNodes = unreachableNodes2;
    unreachableNodes_arraysize = newSize;
}

void Rerr::setUnreachableNodes(size_t k, const char * unreachableNodes)
{
    if (k >= unreachableNodes_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    this->unreachableNodes[k] = unreachableNodes;
}

void Rerr::insertUnreachableNodes(size_t k, const char * unreachableNodes)
{
    if (k > unreachableNodes_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = unreachableNodes_arraysize + 1;
    omnetpp::opp_string *unreachableNodes2 = new omnetpp::opp_string[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        unreachableNodes2[i] = this->unreachableNodes[i];
    unreachableNodes2[k] = unreachableNodes;
    for (i = k + 1; i < newSize; i++)
        unreachableNodes2[i] = this->unreachableNodes[i-1];
    delete [] this->unreachableNodes;
    this->unreachableNodes = unreachableNodes2;
    unreachableNodes_arraysize = newSize;
}

void Rerr::insertUnreachableNodes(const char * unreachableNodes)
{
    insertUnreachableNodes(unreachableNodes_arraysize, unreachableNodes);
}

void Rerr::eraseUnreachableNodes(size_t k)
{
    if (k >= unreachableNodes_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = unreachableNodes_arraysize - 1;
    omnetpp::opp_string *unreachableNodes2 = (newSize == 0) ? nullptr : new omnetpp::opp_string[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        unreachableNodes2[i] = this->unreachableNodes[i];
    for (i = k; i < newSize; i++)
        unreachableNodes2[i] = this->unreachableNodes[i+1];
    delete [] this->unreachableNodes;
    this->unreachableNodes = unreachableNodes2;
    unreachableNodes_arraysize = newSize;
}

size_t Rerr::getUnreachableSeqNumsArraySize() const
{
    return unreachableSeqNums_arraysize;
}

uint32_t Rerr::getUnreachableSeqNums(size_t k) const
{
    if (k >= unreachableSeqNums_arraysize) throw omnetpp::cRuntimeError("Array of size unreachableSeqNums_arraysize indexed by %lu", (unsigned long)k);
    return this->unreachableSeqNums[k];
}

void Rerr::setUnreachableSeqNumsArraySize(size_t newSize)
{
    uint32_t *unreachableSeqNums2 = (newSize==0) ? nullptr : new uint32_t[newSize];
    size_t minSize = unreachableSeqNums_arraysize < newSize ? unreachableSeqNums_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        unreachableSeqNums2[i] = this->unreachableSeqNums[i];
    for (size_t i = minSize; i < newSize; i++)
        unreachableSeqNums2[i] = 0;
    delete [] this->unreachableSeqNums;
    this->unreachableSeqNums = unreachableSeqNums2;
    unreachableSeqNums_arraysize = newSize;
}

void Rerr::setUnreachableSeqNums(size_t k, uint32_t unreachableSeqNums)
{
    if (k >= unreachableSeqNums_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    this->unreachableSeqNums[k] = unreachableSeqNums;
}

void Rerr::insertUnreachableSeqNums(size_t k, uint32_t unreachableSeqNums)
{
    if (k > unreachableSeqNums_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = unreachableSeqNums_arraysize + 1;
    uint32_t *unreachableSeqNums2 = new uint32_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        unreachableSeqNums2[i] = this->unreachableSeqNums[i];
    unreachableSeqNums2[k] = unreachableSeqNums;
    for (i = k + 1; i < newSize; i++)
        unreachableSeqNums2[i] = this->unreachableSeqNums[i-1];
    delete [] this->unreachableSeqNums;
    this->unreachableSeqNums = unreachableSeqNums2;
    unreachableSeqNums_arraysize = newSize;
}

void Rerr::insertUnreachableSeqNums(uint32_t unreachableSeqNums)
{
    insertUnreachableSeqNums(unreachableSeqNums_arraysize, unreachableSeqNums);
}

void Rerr::eraseUnreachableSeqNums(size_t k)
{
    if (k >= unreachableSeqNums_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = unreachableSeqNums_arraysize - 1;
    uint32_t *unreachableSeqNums2 = (newSize == 0) ? nullptr : new uint32_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        unreachableSeqNums2[i] = this->unreachableSeqNums[i];
    for (i = k; i < newSize; i++)
        unreachableSeqNums2[i] = this->unreachableSeqNums[i+1];
    delete [] this->unreachableSeqNums;
    this->unreachableSeqNums = unreachableSeqNums2;
    unreachableSeqNums_arraysize = newSize;
}

class RerrDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_noDeleteFlag,
        FIELD_destCount,
        FIELD_unreachableNodes,
        FIELD_unreachableSeqNums,
    };
  public:
    RerrDescriptor();
    virtual ~RerrDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RerrDescriptor)

RerrDescriptor::RerrDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(networkprojectvanet::replay::replay_mitigation_ecdsa::Rerr)), "networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket")
{
    propertynames = nullptr;
}

RerrDescriptor::~RerrDescriptor()
{
    delete[] propertynames;
}

bool RerrDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Rerr *>(obj)!=nullptr;
}

const char **RerrDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RerrDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RerrDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int RerrDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_noDeleteFlag
        FD_ISEDITABLE,    // FIELD_destCount
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_unreachableNodes
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_unreachableSeqNums
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *RerrDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "noDeleteFlag",
        "destCount",
        "unreachableNodes",
        "unreachableSeqNums",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int RerrDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'n' && strcmp(fieldName, "noDeleteFlag") == 0) return base+0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destCount") == 0) return base+1;
    if (fieldName[0] == 'u' && strcmp(fieldName, "unreachableNodes") == 0) return base+2;
    if (fieldName[0] == 'u' && strcmp(fieldName, "unreachableSeqNums") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RerrDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_noDeleteFlag
        "unsigned int",    // FIELD_destCount
        "string",    // FIELD_unreachableNodes
        "uint32_t",    // FIELD_unreachableSeqNums
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **RerrDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RerrDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RerrDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Rerr *pp = (Rerr *)object; (void)pp;
    switch (field) {
        case FIELD_unreachableNodes: return pp->getUnreachableNodesArraySize();
        case FIELD_unreachableSeqNums: return pp->getUnreachableSeqNumsArraySize();
        default: return 0;
    }
}

const char *RerrDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Rerr *pp = (Rerr *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RerrDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Rerr *pp = (Rerr *)object; (void)pp;
    switch (field) {
        case FIELD_noDeleteFlag: return bool2string(pp->getNoDeleteFlag());
        case FIELD_destCount: return ulong2string(pp->getDestCount());
        case FIELD_unreachableNodes: return oppstring2string(pp->getUnreachableNodes(i));
        case FIELD_unreachableSeqNums: return ulong2string(pp->getUnreachableSeqNums(i));
        default: return "";
    }
}

bool RerrDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Rerr *pp = (Rerr *)object; (void)pp;
    switch (field) {
        case FIELD_noDeleteFlag: pp->setNoDeleteFlag(string2bool(value)); return true;
        case FIELD_destCount: pp->setDestCount(string2ulong(value)); return true;
        case FIELD_unreachableNodes: pp->setUnreachableNodes(i,(value)); return true;
        case FIELD_unreachableSeqNums: pp->setUnreachableSeqNums(i,string2ulong(value)); return true;
        default: return false;
    }
}

const char *RerrDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *RerrDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Rerr *pp = (Rerr *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(RrepAck)

RrepAck::RrepAck(const char *name, short kind) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(name, kind)
{
    this->setPacketType(RREPACK);
}

RrepAck::RrepAck(const RrepAck& other) : ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket(other)
{
    copy(other);
}

RrepAck::~RrepAck()
{
}

RrepAck& RrepAck::operator=(const RrepAck& other)
{
    if (this == &other) return *this;
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::operator=(other);
    copy(other);
    return *this;
}

void RrepAck::copy(const RrepAck& other)
{
}

void RrepAck::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimPack(b);
}

void RrepAck::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket::parsimUnpack(b);
}

class RrepAckDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    RrepAckDescriptor();
    virtual ~RrepAckDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(RrepAckDescriptor)

RrepAckDescriptor::RrepAckDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(networkprojectvanet::replay::replay_mitigation_ecdsa::RrepAck)), "networkprojectvanet::replay::replay_mitigation_ecdsa::AodvControlPacket")
{
    propertynames = nullptr;
}

RrepAckDescriptor::~RrepAckDescriptor()
{
    delete[] propertynames;
}

bool RrepAckDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RrepAck *>(obj)!=nullptr;
}

const char **RrepAckDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *RrepAckDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int RrepAckDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int RrepAckDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *RrepAckDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int RrepAckDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *RrepAckDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **RrepAckDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RrepAckDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RrepAckDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    RrepAck *pp = (RrepAck *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *RrepAckDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RrepAck *pp = (RrepAck *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RrepAckDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    RrepAck *pp = (RrepAck *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool RrepAckDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    RrepAck *pp = (RrepAck *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *RrepAckDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *RrepAckDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    RrepAck *pp = (RrepAck *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

} // namespace replay_mitigation_ecdsa
} // namespace replay
} // namespace networkprojectvanet

