
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "scanner.h"
#include "sender.h"

#define PORT 12345

int main(int argc, char** argv) {
    bool is_server = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            is_server = false;
        } else if (strcmp(argv[i], "-s") == 0) {
            is_server = true;
        } else {
            fprintf(stderr, "Usage: %s [...OPTIONS]\n", argv[0]);
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "   -s   Start as server\n");
            fprintf(stderr, "   -c   Start as client\n");
        }
    }
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket() failed");
    }
    int permission = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &permission, sizeof(permission)) < 0) {
        perror("setsockopt() failed");
    }
    if (is_server) {
        initializeScanner();
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = PORT;
        address.sin_addr.s_addr = INADDR_BROADCAST;
        for (;;) {
            int code;
            char key[64];
            if (getNextScancode(&code)) {
                printf("Pressed %i\n", code);
                sprintf(key, "P%i", code);
                int length = strlen(key);
                if (sendto(sock, key, length, 0, (struct sockaddr*)&address, sizeof(address)) != length) {
                    perror("sendto() failed");
                }
            } else if (code >= 0) {
                printf("Released %i\n", code);
                sprintf(key, "R%i", code);
                int length = strlen(key);
                if (sendto(sock, key, length, 0, (struct sockaddr*)&address, sizeof(address)) != length) {
                    perror("sendto() failed");
                }
            }
        }
    } else {
        initializeSender();
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = PORT;
        address.sin_addr.s_addr = INADDR_BROADCAST;
        if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("bind() failed");
        }
        for (;;) {
            char key[64];
            int length = recvfrom(sock, key, 64, 0, NULL, 0);
            if (length <= 0) {
                perror("recvfrom() failed");
            } else {
                key[length] = 0;
                char type;
                int code;
                sscanf(key, "%c%i", &type, &code);
                if (type == 'P') {
                    printf("Pressed %i\n", code);
                } else {
                    printf("Released %i\n", code);
                }
                /* sendScancode(code, type == 'P'); */
            }
        }
    }
}

