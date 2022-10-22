#ifndef HLPRFUNCS
#define HLPRFUNCS

#include "duckchat.h"
#include "channels.h"

#define REQ_SWITCH 8

// Show a say user message to stdout
void say_text_output(char *channel, char *username, char *text);

// Decode the users input and return the correct response code
int decode_input(char *inp);

// Get the second argument of the users command
char *get_command_arg(char *buf, int *arg_len);

// Clearout the stdout line
void clear_stdout(int clear_chars);

// List all channels from datagram
void print_channel_list(struct text *inp_t);

// List all joined users in channel
void print_user_list(struct text *inp_t);

// Send a join request
int send_join_req(char *buff, int socketfd, struct sockaddr_in *server_addr, int len, char *active_channel);

// Send a say request
int send_say_req(int socketfd, struct sockaddr_in *s_addr, int len, char *active_channel, char *buff);

// Send a list request
int send_list_req(int socketfd, struct sockaddr_in *s_addr, int len);

// Send a who request
int send_who_req(int socketfd, struct sockaddr_in *s_addr, int len, char *buff);

// Send a leave request
int send_leave_req(int socketfd, struct sockaddr_in *s_addr, int len, char *buff, char *active_channel);

// Send a logout request
int send_logout_req(int socketfd, struct sockaddr_in *s_addr, int len);

#endif