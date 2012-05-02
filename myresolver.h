#ifndef MYRESOLVER_H_
#define MYRESOLVER_H_

#include <stdlib.h>
#include <stdint.h>

const uint16_t DNS_AA_FALSE = 0x0000;
const uint16_t DNS_AA_BIT_MASK = 0x0400;
const uint16_t DNS_AA_TRUE = 0x0400;
const uint16_t DNS_OPCODE_INVERSE_QUERY = 0x0800;
const uint16_t DNS_OPCODE_BIT_MASK = 0x7800;
const uint16_t DNS_OPCODE_SERVER_STATUS_REQ = 0x1000;
const uint16_t DNS_OPCODE_STANDARD_QUERY = 0x0000;
const uint16_t DNS_PORT_NUMER = 53;
const uint16_t DNS_QR_BIT_MASK = 0x8000;
const uint16_t DNS_QR_TYPE_QUERY = 0x0000;
const uint16_t DNS_QR_TYPE_RESPONSE = 0x8000;
const uint16_t DNS_RA_FALSE = 0x0000;
const uint16_t DNS_RA_BIT_MASK = 0x0080;
const uint16_t DNS_RA_TRUE = 0x0080;
const uint16_t DNS_RCODE_FORMAT_ERROR = 0x0001;
const uint16_t DNS_RCODE_BIT_MASK = 0x000F;
const uint16_t DNS_RCODE_NAME_ERROR = 0x0003;
const uint16_t DNS_RCODE_NOERROR = 0x0000;
const uint16_t DNS_RCODE_NOT_IMPLEMENTED = 0x0004;
const uint16_t DNS_RCODE_REFUSED = 0x0005;
const uint16_t DNS_RCODE_SERVER_FAILURE = 0x0002;
const uint16_t DNS_RD_FALSE = 0x0000;
const uint16_t DNS_RD_BIT_MASK = 0x0100;
const uint16_t DNS_RD_TRUE = 0x0100;
const uint16_t DNS_TRUNC_FALSE = 0x0000;
const uint16_t DNS_TRUNC_BIT_MASK = 0x0200;
const uint16_t DNS_TRUNC_TRUE = 0x0200;
const uint16_t DNS_Z_BIT_MASK = 0x0070;
const uint16_t MAX_UDP_PACKET_SIZE = 9032;

struct DNS_PACKET_HEADER {
    uint16_t dnsHeaderId;
    uint16_t dnsHeaderDescription;
    uint16_t dnsHeaderQuestion_count;
    uint16_t dnsHeaderAnswer_count;
    uint16_t dnsHeaderNameserver_count;
    uint16_t dnsHeaderAdditional_count;
}__attribute__((__packed__));

struct DNS_PACKET_QUESTION {
    uint8_t* qname; 
    RR_TYPE qtype;
    QCLASS qclass;
}__attribute__((__packed__));

struct DNS_PACKET_RESOURCE_RECORD {
    uint8_t* name; 
    RR_TYPE type;
    QCLASS class;
    uint32_t ttl;
    uint16_t rdlength;
    void* rd_data;
}__attribute__((__packed__));

struct DNS_PACKET {
    struct DNS_PACKET_HEADER dnsPacketHeader;
    struct DNS_PACKET_QUESTION* dnsPacketQuestion;
    struct DNS_PACKET_RESOURCE_RECORD* dnsPacketAnswer;
    struct DNS_PACKET_RESOURCE_RECORD* dnsPacketAuthority;
    struct DNS_PACKET_RESOURCE_RECORD* dnsPacketAdditional;
    struct DNS_PACKET_RESOURCE_RECORD* dnsPacketEnds;
}__attribute__((__packed__));

#endif /* MYRESOLVER_H_ */
