#ifndef CNNCT_HNDLR
#define CNNCT_HNDLR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct self{
    uint16_t port;
    struct sockaddr_in addr;
    int addrlen;
    int socket_fd;
} Self;

typedef struct connection_handler
{
    void *self;
    int (*init_socket)(struct connection_handler* cnnct_hndlr, char *addr, uint16_t prt);
    int (*socket_listen)(struct connection_handler* cnnct_hndlr, int mx_cnncts);
    int (*socket_accept)(struct connection_handler*, struct sockaddr_in *client_addr, int *client_addrlen);
    int (*socket_connect)(struct connection_handler*, char *server_name, uint16_t port, struct sockaddr_in* s_addr, socklen_t *len);
    int (*socket_send)(struct connection_handler*, void *out_message, int out_msglen, struct sockaddr_in *dest_addr);
    int (*socket_recv)(struct connection_handler*, void *buffer, int buff_len, struct sockaddr_in *cli_addr, socklen_t *cli_addrlen, int block_type);
    int (*get_socketfd)(struct connection_handler*);
    void (*destroy)(struct connection_handler*cnnct_hndlr);
} Connection_Handler;

Connection_Handler *create_handler();

#endif