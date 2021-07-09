//
// Created by marcelo on 08/07/2021.
//

#include "udpclient.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define FAIL 1

int create_local_socket(void) {
    int local_socket;
    local_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (local_socket < 0) {
        perror("socket");
        return -1;
    }
    return local_socket;
}

struct sockaddr_in create_target_address(char *target, int target_port) {
    struct sockaddr_in server;
    struct hostent *internet_target;
    struct in_addr target_ip;

    if (inet_aton(target, &target_ip))
        internet_target = gethostbyaddr((char *) &target_ip, sizeof(target_ip), AF_INET);
    else
        internet_target = gethostbyname(target);

    if (internet_target == NULL) {
        fprintf(stderr, "Invalid network address\n");
        exit(FAIL);
    }

    memset((char *) &server, 0, sizeof(server));
    memcpy(&server.sin_addr, internet_target->h_addr_list[0], sizeof(server.sin_addr));
    server.sin_family = AF_INET;
    server.sin_port = htons(target_port);

    return server;
}

void send_message(int local_socket, struct sockaddr_in target_address, char *message) {
    if (sendto(local_socket, message, strlen(message) + 1, 0, (struct sockaddr *) &target_address, sizeof(target_address)) < 0) {
        perror("sendto");
    }
}

long receive_message(int local_socket, char *buffer, int BUFFER_SIZE) {
    long received_bytes;

    received_bytes = recvfrom(local_socket, buffer, BUFFER_SIZE, 0, NULL, 0);
    if (received_bytes < 0) {
        perror("recvfrom");
    }
    return received_bytes;
}

int local_socket;
struct sockaddr_in target_address;

void setup_sockets(int port, char* address) {
    local_socket = create_local_socket();
    target_address = create_target_address(address, port);

}

long exchange_message(char* message, char* response) {
    send_message(local_socket, target_address, message);

    long size = receive_message(local_socket, response, 1000);
    response[size] = '\0';

    return size;
}
