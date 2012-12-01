
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int main(int argc, char** argv)
{
    struct sockaddr_in sin_divert;
    sin_divert.sin_family = AF_INET;
    sin_divert.sin_port = htons(8668);

    int sock;
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_DIVERT);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    if (bind(sock, (struct sockaddr*)&sin_divert, sizeof(sin_divert)) < 0) {
        perror("bind");
        exit(1);
    }
    size_t size;
    char buf[65535];
    struct sockaddr_in recv_addr;
    socklen_t addr_size = sizeof(recv_addr);

    for (;;) {
        size = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&recv_addr, &addr_size);
        if (size < 0) {
            perror("recvfrom");
            exit(1);
        }
        size = sendto(sock, buf, size, 0, (struct sockaddr*)&recv_addr, addr_size);
    }

    return 0;
}

