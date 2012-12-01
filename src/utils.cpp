#include "utils.hpp"
#include "alias.h"

int main(int argc, char** argv)
{
    // memdump sample
    uint8_t buffer[] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
                        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
                        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
                        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
                        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88};

    // icmp_checksum:        : 57c3
    // ip header total length: 84 byte
    // ip header size        : 20byte
    uint8_t icmp_sample[] = {0x08, 0x00, 0x00, 0x00, 0x56, 0x58, 0x00, 0x01,
                             0x50, 0x8f, 0x92, 0x1b, 0x00, 0x0c, 0x7c, 0x29,
                             0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                             0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
                             0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                             0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
                             0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};

    memdump(buffer, sizeof(buffer));

    // bytes swapper sample
    uint64_t input64 = 0x123456789ABCDEF0;
    uint64_t output64;
    BSWAP64(input64, output64);
    printf("%llx\n", output64);
    BSWAP64(output64, output64);
    printf("%llx\n", output64);

    uint32_t input32 = 0x12345678;
    uint32_t output32;
    BSWAP32(input32, output32);
    printf("%x\n", output32);
    BSWAP32(output32, output32);
    printf("%x\n", output32);
    
    std::cout << "addr2string" << std::endl;
    std::string s1 = "127.0.0.1";
    std::string s2 = "255.255.255.0";
    uint32_t addr = string2addr(s1);
    uint32_t mask = string2addr(s2);
    std::cout << std::hex << addr << std::endl;
    std::cout << std::hex << mask << std::endl;
    std::cout << addr2string(addr) << std::endl;
    std::cout << addr2string_calc(addr, mask) << std::endl;

    uint16_t a = htons(checksum(icmp_sample, 36, 0));
    uint8_t* b = (uint8_t*)&a;
    printf("%x", b[0]);
    printf("%x\n", b[1]);



    return 0;
}
