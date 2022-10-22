#include "duckchat.h"
#include "raw.h"
#include "connection_handler.h"
#include "helper_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define UNUSED __attribute__((unused)) 

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
volatile int writing = 0;
volatile int logout = 0;

struct pargs{
    int socketfd;
    struct sockaddr_in server;
};

void *recv_thread(void *args){
    struct pargs *a = (struct pargs *)args;
    char *buff = (char *)malloc(BUFSIZ * sizeof(char));
    unsigned int len = sizeof(a->server);
    while (!logout)
    {
        if(recvfrom(a->socketfd, buff, BUFSIZ, MSG_DONTWAIT, (struct sockaddr *)&(a->server), &len) >= 0){
            struct text *t = (struct text*) buff;
            pthread_mutex_lock(&lock);
            while (writing == 1) // Wait if the buffer is being written to
            {
                pthread_cond_wait(&cond, &lock);
                if (logout)
                {
                    continue;
                }
                    
            }
            clear_stdout(50); 
            
            switch (t->txt_type)
            {
            case TXT_SAY:{
                struct text_say *ts = (struct text_say *)t;
                say_text_output(ts->txt_channel, ts->txt_username, ts->txt_text);
            }break;
            case TXT_LIST:{
                print_channel_list(t);
                
           }break;
            case TXT_WHO:{
                print_user_list(t);
                
            }break;
            case TXT_ERROR:{
                struct text_error *te = (struct text_error*) buff;
                write(STDOUT_FILENO, te->txt_error, SAY_MAX);
            }break;
            
            default:
                write(STDERR_FILENO, "Invalid request\n", 17);
                break;
            }

            write(STDOUT_FILENO, (char*)"\n> ", 3);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&lock);
        }
    }
    free(buff);
    return NULL;
}

void *send_thread(void *args){
    struct pargs *a = (struct pargs *)args;
    char *buff = (char *)malloc(BUFSIZ * sizeof(char));
    char *active_channel = (char*)malloc(CHANNEL_MAX * sizeof(char));
    strcpy(active_channel, "Common");
    char c;
    int char_ind = 0;
    unsigned int len = sizeof(a->server);
    write(STDOUT_FILENO, (char*)"> ", 2);
    // Main input send loop
    while (!logout)
    {
        char_ind = 0;
        // Read stdin loop
        while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n')
        {
            
            writing = 1;
            pthread_cond_broadcast(&cond);
            buff[char_ind++] = c;
            write(STDOUT_FILENO, &c, 1);

        }
        buff[char_ind + 1] = '\0';
        writing = 0;
        pthread_cond_broadcast(&cond);


        // Decode input and determine correct request type
        switch (decode_input(buff))
        {
        case REQ_JOIN:{ // Join request
            send_join_req(buff, a->socketfd, &a->server, len, active_channel);
        }break;
        case REQ_SAY:{ // Say request
            send_say_req(a->socketfd, &a->server, len, active_channel, buff);
        }break;
        case REQ_LIST:{
            send_list_req(a->socketfd, &a->server, len);
        }break;
        case REQ_WHO:{
            send_who_req(a->socketfd, &a->server, len, buff);

        }break;
        case REQ_LEAVE: {
            send_leave_req(a->socketfd, &a->server, len, buff, active_channel);
        }break;
        case REQ_SWITCH:{
            int chnl_len;
            char *chnl = get_command_arg(buff, &chnl_len);
            strncpy(active_channel, chnl, chnl_len);
        }break;
        case REQ_LOGOUT:{
            /*struct request_logout req_lg;
            req_lg.req_type = REQ_LOGOUT;
            if(sendto(a->socketfd, &req_lg, sizeof(req_lg), 0, (struct sockaddr *)&a->server, len) < 0){
                write(STDOUT_FILENO, "Could not send logout reqeust\n", 30);
            }*/
            send_logout_req(a->socketfd, &a->server, len);
            free(buff);
            free(active_channel);
            logout = 1;
            pthread_cond_broadcast(&cond);
        }break;
        default:
            write(STDOUT_FILENO, "Something went terribly wrong\n", 30);
            break;
        }
        if (!logout)
        {
            pthread_mutex_lock(&lock);
            write(STDOUT_FILENO, (char*)"\n> ", 3);
            memset(buff, '\0', BUFSIZ);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&lock);
        }
    }
    
    return NULL;
}


int main(int argc, char **argv){
    raw_mode();
    if (argc < 4)
    {
        perror("Not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    char *input = (char *)malloc(BUFSIZ * sizeof(char));
    char *usrname = (char *)malloc(USERNAME_MAX * sizeof(char));
    char *server = (char*)malloc(BUFSIZ * sizeof(char));
    //char *active_channel = (char*)malloc(CHANNEL_MAX * sizeof(char));
    uint16_t *port = (uint16_t *)malloc(sizeof(short));


    (void)strcpy(server, argv[1]);
    *port = (uint16_t)atoi(argv[2]);
    (void)strcpy(usrname, argv[3]);

    struct sockaddr_in server_addr;
    //unsigned int server_addrlen;

    Connection_Handler *c = create_handler();
    c->init_socket(c, NULL, 0);

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(*port);
    server_addr.sin_addr.s_addr = inet_addr(server);

    struct request_login req_lg;
    req_lg.req_type = REQ_LOGIN;
    strcpy(req_lg.req_username, usrname);
    c->socket_send(c, (void*)&req_lg, sizeof(req_lg), &server_addr);

    pthread_t recv_t;
    pthread_attr_t rect_attr;
    pthread_attr_init(&rect_attr);
    struct pargs a = {c->get_socketfd(c), server_addr};
    pthread_create(&recv_t, &rect_attr, recv_thread, (void*)&a);

    pthread_t send_t;
    pthread_attr_t send_attr;
    pthread_attr_init(&send_attr);
    pthread_create(&send_t, &send_attr, send_thread, (void*)&a);


    pthread_join(send_t, NULL);
    pthread_join(recv_t, NULL);


    c->destroy(c);

    free(usrname);
    free(port);
    free(server);
    free(input); 
    cooked_mode();
    return 0;
}