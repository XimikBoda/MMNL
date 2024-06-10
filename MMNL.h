#pragma once
#include "Module/Module.h"
#include "PacketIndexWatcher.h"
#include "DeviceWatcher.h"

//Header: 
//[d adr][s adr][f][i][l]{d}{m}{l}[payload]
//[ 2 b ][ 2 b ][1][1][1]{1}{1}[1}[n bytes]

//#define FIND_NET_FLAG               0b10000000
#define DIRECT_RETRANSLATION_FLAG   0b01000000
#define MODULE_RETRANSLATION_FLAG   0b00100000
#define NETWORK_RETRANSLATION_FLAG  0b00010000
#define FRAGMENTATION_FLAG          0b00001000
#define NEED_ANSVER_FLAG            0b00000100
#define SYSTEM_FLAG                 0b00000010
#define ANSVER_FLAG                 0b00000001

#define MMNL_FIND_NET   0x01
#define MMNL_NEW_DEV    0x02
#define MMNL_KEEP_ADR   0x03
#define MMNL_KEEP_ALIVE 0x04
#define MMNL_PING       0x05
#define MMNL_FIND_NAME  0x06

#define MMNL_GET_ADR(x)         ((x) & 0b0000011111111111)
#define MMNL_GET_NETWORK(x)     ((x) >> (16 - 5))
#define MMNL_GET_BROADCAST(x)   ((x) & 0b1111100000000000)


typedef void (*packet_callback)(uint16_t adr, void* buf, int len, bool broadcast);

namespace MMNL{
    enum State : uint8_t {
        Init,
        WaitNetwork,
        WaitForAddressVerification,
        Idle,
    };

    class MMNL {
        State state;

        uint16_t local_ard = 0;
        uint8_t mmnl_i = 0;

        uint32_t mmnl_tim = 0;
        uint32_t rnd_v = 0;

        const char* dev_name = 0;


        packet_callback user_packet_callback;

        Module::Base** mlist = NULL;
        uint8_t mlist_size = 0;

        uint8_t cur_ind = 0;

        uint8_t MMNL_buf[256];
        uint16_t MMNL_buf_size = 0;

        uint8_t MMNL_inbuf[256];
        uint16_t MMNL_inbuf_size = 0;

        PacketIndexWatcher piw;
    public:
        DeviceWatcher dv;

        bool init(Module::Base** mlist_, uint8_t mlist_size_, const char* device_name, packet_callback pc);
        void update();

        void send(uint16_t adr, void* buf, int l, uint8_t dttl = 0, uint8_t mttl = 0, uint8_t nttl = 0);

        uint16_t get_my_address();

        State get_state();

    private:
        void pcocess_packet();

        void send_buf();

        void make_packet(uint16_t adr, uint8_t f, uint8_t i, const void* buf, int l, uint8_t dttl = 4, uint8_t mttl = 4, uint8_t nttl = 2);
    };
};