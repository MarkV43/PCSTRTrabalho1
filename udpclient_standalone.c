#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr,"Usage: udpclient_standalone <address> <port> <message> \n");
        fprintf(stderr,"Where <address> is the server address\n");
        fprintf(stderr,"<port> is the server port\n");
        fprintf(stderr,"<message> is the message to be sent to the server\n");
        fprintf(stderr,"Example:\n");
        fprintf(stderr,"\tudpclient_standalone localhost 1234 \"Hello World\"\n");
        exit(FAIL);
    }

    int target_port = atoi(argv[2]);
    int local_socket = create_local_socket();
    struct sockaddr_in target_address = create_target_address(argv[1], target_port);

    int i = 0;
    char msg_sent[1000];
    char msg_recv[1000];
    long nrec;

    do {
        printf("Try %d\n", i);
        strcat(msg_sent, argv[3]);

        printf("%s\n", msg_sent);

        send_message(local_socket, target_address, msg_sent);

        nrec = receive_message(local_socket, msg_recv, 1000);
        msg_recv[nrec] = '\0';

        printf("Response with %ld bytes >>> %s\n", nrec, msg_recv);

        sleep(1);
        ++i;
    } while (i < 5000);
}
