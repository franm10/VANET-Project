//
// Generated file, do not edit! Created by nedtool 5.6 from grayhole/grayhole_mitigation/messages/IntermediateAck_m.msg.
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
#include "IntermediateAck_m_m.h"

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

Register_Class(IntermediateAck)

IntermediateAck::IntermediateAck(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

IntermediateAck::IntermediateAck(const IntermediateAck& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

IntermediateAck::~IntermediateAck()
{
}

IntermediateAck& IntermediateAck::operator=(const IntermediateAck& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void IntermediateAck::copy(const IntermediateAck& other)
{
    this->originalPacketId = other.originalPacketId;
    this->mic = other.mic;
    this->senderNodeId = other.senderNodeId;
    this->previousHopNodeId = other.previousHopNodeId;
    this->timestamp = other.timestamp;
}

void IntermediateAck::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->originalPacketId);
    doParsimPacking(b,this->mic);
    doParsimPacking(b,this->senderNodeId);
    doParsimPacking(b,this->previousHopNodeId);
    doParsimPacking(b,this->timestamp);
}

void IntermediateAck::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->originalPacketId);
    doParsimUnpacking(b,this->mic);
    doParsimUnpacking(b,this->senderNodeId);
    doParsimUnpacking(b,this->previousHopNodeId);
    doParsimUnpacking(b,this->timestamp);
}

int IntermediateAck::getOriginalPacketId() const
{
    return this->originalPacketId;
}

void IntermediateAck::setOriginalPacketId(int originalPacketId)
{
    this->originalPacketId = originalPacketId;
}

bool IntermediateAck::getMic() const
{
    return this->mic;
}

void IntermediateAck::setMic(bool mic)
{
    this->mic = mic;
}

int IntermediateAck::getSenderNodeId() const
{
    return this->senderNodeId;
}

void IntermediateAck::setSenderNodeId(int senderNodeId)
{
    this->senderNodeId = senderNodeId;
}

int IntermediateAck::getPreviousHopNodeId() const
{
    return this->previousHopNodeId;
}

void IntermediateAck::setPreviousHopNodeId(int previousHopNodeId)
{
    this->previousHopNodeId = previousHopNodeId;
}

double IntermediateAck::getTimestamp() const
{
    return this->timestamp;
}

void IntermediateAck::setTimestamp(double timestamp)
{
    this->timestamp = timestamp;
}

class IntermediateAckDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_originalPacketId,
        FIELD_mic,
        FIELD_senderNodeId,
        FIELD_previousHopNodeId,
        FIELD_timestamp,
    };
  public:
    IntermediateAckDescriptor();
    virtual ~IntermediateAckDescriptor();

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

Register_ClassDescriptor(IntermediateAckDescriptor)

IntermediateAckDescriptor::IntermediateAckDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(IntermediateAck)), "omnetpp::cPacket")
{
    propertynames = nullptr;
}

IntermediateAckDescriptor::~IntermediateAckDescriptor()
{
    delete[] propertynames;
}

bool IntermediateAckDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<IntermediateAck *>(obj)!=nullptr;
}

const char **IntermediateAckDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *IntermediateAckDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int IntermediateAckDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int IntermediateAckDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_originalPacketId
        FD_ISEDITABLE,    // FIELD_mic
        FD_ISEDITABLE,    // FIELD_senderNodeId
        FD_ISEDITABLE,    // FIELD_previousHopNodeId
        FD_ISEDITABLE,    // FIELD_timestamp
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *IntermediateAckDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "originalPacketId",
        "mic",
        "senderNodeId",
        "previousHopNodeId",
        "timestamp",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int IntermediateAckDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'o' && strcmp(fieldName, "originalPacketId") == 0) return base+0;
    if (fieldName[0] == 'm' && strcmp(fieldName, "mic") == 0) return base+1;
    if (fieldName[0] == 's' && strcmp(fieldName, "senderNodeId") == 0) return base+2;
    if (fieldName[0] == 'p' && strcmp(fieldName, "previousHopNodeId") == 0) return base+3;
    if (fieldName[0] == 't' && strcmp(fieldName, "timestamp") == 0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *IntermediateAckDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_originalPacketId
        "bool",    // FIELD_mic
        "int",    // FIELD_senderNodeId
        "int",    // FIELD_previousHopNodeId
        "double",    // FIELD_timestamp
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **IntermediateAckDescriptor::getFieldPropertyNames(int field) const
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

const char *IntermediateAckDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int IntermediateAckDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    IntermediateAck *pp = (IntermediateAck *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *IntermediateAckDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    IntermediateAck *pp = (IntermediateAck *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string IntermediateAckDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    IntermediateAck *pp = (IntermediateAck *)object; (void)pp;
    switch (field) {
        case FIELD_originalPacketId: return long2string(pp->getOriginalPacketId());
        case FIELD_mic: return bool2string(pp->getMic());
        case FIELD_senderNodeId: return long2string(pp->getSenderNodeId());
        case FIELD_previousHopNodeId: return long2string(pp->getPreviousHopNodeId());
        case FIELD_timestamp: return double2string(pp->getTimestamp());
        default: return "";
    }
}

bool IntermediateAckDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    IntermediateAck *pp = (IntermediateAck *)object; (void)pp;
    switch (field) {
        case FIELD_originalPacketId: pp->setOriginalPacketId(string2long(value)); return true;
        case FIELD_mic: pp->setMic(string2bool(value)); return true;
        case FIELD_senderNodeId: pp->setSenderNodeId(string2long(value)); return true;
        case FIELD_previousHopNodeId: pp->setPreviousHopNodeId(string2long(value)); return true;
        case FIELD_timestamp: pp->setTimestamp(string2double(value)); return true;
        default: return false;
    }
}

const char *IntermediateAckDescriptor::getFieldStructName(int field) const
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

void *IntermediateAckDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    IntermediateAck *pp = (IntermediateAck *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

