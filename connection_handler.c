#include "connection_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


void init_socket(Connection_Handler* ch, char *addr, uint16_t port){
    Self *s = (Self *)ch->self;
    if((s->socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket\n");
    }
    if (port == 0)
    {
        return;
    }
    
    
    memset(&(s->addr), 0, sizeof(s->addr));

    s->port = port;

    s->addr.sin_family = AF_INET;
    if (addr == NULL)
    {
        /* code */
        s->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }else{
        inet_pton(AF_INET, addr, &(s->addr.sin_addr));
    }
    
    s->addr.sin_port = htons(port);
    
    s->addrlen = sizeof(s->addr);

    int bind_err;
    // Bind address to the socket
    if((bind_err = bind(s->socket_fd, (struct sockaddr*)&(s->addr), s->addrlen)) < 0){
        printf("Cannot bind create socket: %d\n", bind_err);
    }
    unsigned int namelen = sizeof(s->addr);
    if (getsockname(s->socket_fd, (struct sockaddr *) &s->addr, &namelen) < 0)
    {
        printf("getsockname()");
        exit(3);
    }
    printf("Port assigned is %d\n", ntohs(s->addr.sin_port));

}

int socket_listen(Connection_Handler *ch, int max_connections){
    Self *s = (Self *)ch->self;
    if(listen(s->socket_fd, max_connections) < 0){
        perror("Cannot listen\n");
        return -1;
    }
    return 0;
}

int socket_accept(Connection_Handler *ch, struct sockaddr_in *client_addr, int *client_addrlen){
    Self *s = (Self *)ch->self;
    int client_sock;

    if((client_sock = accept(s->socket_fd, (struct sockaddr*)client_addr, (socklen_t*)client_addrlen)) < 0) {
        fprintf(stdout, "Cannot accept\n");
    }
    return client_sock;
}

//Connect this socket to the address server_name at port port and
//returns the servers file descripter and places the servers address in srvr_addr and length
//in srvr_addrlen
int socket_connect(Connection_Handler *ch, char *server_name, uint16_t port, struct sockaddr_in *srvr_addr, socklen_t *srvr_addrlen){
    Self *s = (Self *)ch->self;
    struct sockaddr_in server_addr;
    int server_addrlen = sizeof(server_addr), server_fd;

    memset(&server_addr, 0, server_addrlen);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_name);

    if((server_fd = connect(s->socket_fd, (struct sockaddr*)&server_addr, server_addrlen)) < 0) {
        fprintf(stdout, "Cannot connect\n");
    }
        
    *srvr_addr = server_addr;
    *srvr_addrlen = server_addrlen;
    return server_fd;
}

//Send a message msg to dest_addr
int socket_send(Connection_Handler *ch, void *msg, int msg_len, struct sockaddr_in *dest_addr){
    Self *s = (Self *)ch->self;
    return sendto(s->socket_fd, msg, msg_len, 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr));
}

int socket_recv(Connection_Handler *ch, void *buffer, int buff_len, struct sockaddr_in *cli_addr, socklen_t *cli_addrlen, int block_type){
    Self *s = (Self *)ch->self;
    int ret = recvfrom(s->socket_fd, buffer, buff_len, block_type, (struct sockaddr *)cli_addr, cli_addrlen);
    return ret;
}

void destroy(Connection_Handler *ch){
    Self *s = (Self *)ch->self;
    shutdown(s->socket_fd, SHUT_RDWR);
    free(ch->self); 
    free(ch);
}

int get_socketfd(Connection_Handler *ch){
    Self *s = (Self *)ch->self;
    return s->socket_fd;
}

Connection_Handler tmp = {NULL, init_socket, socket_listen, socket_accept, socket_connect, socket_send, socket_recv, get_socketfd, destroy};

Connection_Handler *create_handler(){
    Self *s = (Self*)malloc(sizeof(Self));
    if (s == NULL)
    {
        return NULL;
    }

    Connection_Handler *ch = (Connection_Handler*)malloc(sizeof(Connection_Handler));
    if (ch == NULL)
    {
        free(s);
        return ch;
    }
    
    *ch = tmp;
    ch->self = (void*)s;
    return ch;
}
