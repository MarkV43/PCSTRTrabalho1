//
// Created by marcelo on 08/07/2021.
//

#ifndef TRABALHO1_UDPCLIENT_H
#define TRABALHO1_UDPCLIENT_H

int create_local_socket(void);

struct sockaddr_in create_target_address(char*, int);

void send_message(int, struct sockaddr_in, char*);

long receive_message(int, char*, int);

void setup_sockets(int, char*);

long exchange_message(char*, char*);

#endif //TRABALHO1_UDPCLIENT_H
