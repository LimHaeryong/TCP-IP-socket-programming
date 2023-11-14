#include "../common.h"

#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    uint16_t x1 = 0x1234;
    uint32_t y1 = 0x12345678;
    uint16_t x2;
    uint32_t y2;

    printf("[host byte -> network byte]\n");
	printf("%#x -> %#x\n", x1, x2 = htons(x1));
	printf("%#x -> %#x\n", y1, y2 = htonl(y1));

    printf("[network byte -> host byte]\n");
	printf("%#x -> %#x\n", x2, ntohs(x2));
	printf("%#x -> %#x\n", y2, ntohl(y2));

    return 0;
}