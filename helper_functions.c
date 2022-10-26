#include "helper_functions.h"
#include "duckchat.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void say_text_output(char *channel, char *username, char *text){
    write(STDOUT_FILENO, "[", 1);
    write(STDOUT_FILENO, channel, CHANNEL_MAX);
    write(STDOUT_FILENO, "]", 1);
    write(STDOUT_FILENO, "[", 1);
    write(STDOUT_FILENO, username, USERNAME_MAX);
    write(STDOUT_FILENO, "]: ", 3);
    write(STDOUT_FILENO, text, SAY_MAX);
}

int decode_input(char *inp){
    if (inp[0] != '/')
    {
        return REQ_SAY;
    }else if (strncmp(inp, "/join", 5) == 0)
    {
        return REQ_JOIN;
    }else if (strncmp(inp, "/list", 5) == 0)
    {
        return REQ_LIST;
    }else if(strncmp(inp, "/who", 4) == 0)
    {
        return REQ_WHO;
    }else if (strncmp(inp, "/leave", 6) == 0)
    {
        return REQ_LEAVE;
    }else if(strncmp(inp, "/switch", 7) == 0)
    {
        return REQ_SWITCH;
    }else if(strncmp(inp, "/exit", 5) == 0)
    {
        return REQ_LOGOUT;
    }
    return -1;
}

char *get_command_arg(char *buf, int *chnl_len){
    int i = 0;
    int chnl_start = -1;
    while (buf[i] != '\0')
    {
      if (buf[i] == ' ')
      {
        if (chnl_start == -1)
        {
            chnl_start = i + 1;
        }
      }
      i++;
    }
    *chnl_len = i - chnl_start;
    return chnl_start + buf;
}

void clear_stdout(int clear_chars){
    for (int i = 0; i < clear_chars; i++)
    {
        write(STDOUT_FILENO, "\b", 1);
    }
}

void print_channel_list(struct text *inp_t){
    struct text_list *tl = (struct text_list *)inp_t;
    write(STDOUT_FILENO, "Existing Channels:", 19);
    for (int i = 0; i < tl->txt_nchannels; i++)
    {
        write(STDOUT_FILENO, "\n\t", 2);
        write(STDOUT_FILENO, (tl->txt_channels + i)->ch_channel, CHANNEL_MAX);
    }

}

void print_user_list(struct text *inp_t){
    struct text_who *tl = (struct text_who *)inp_t;
    
    write(STDOUT_FILENO, "All users in ", 14);
    write(STDOUT_FILENO, tl->txt_channel, CHANNEL_MAX);
    for (int i = 0; i < tl->txt_nusernames; i++)
    {
        write(STDOUT_FILENO, "\n\t", 2);
        write(STDOUT_FILENO, (tl->txt_users + i)->us_username, USERNAME_MAX);
    }

}

int send_join_req(char *buff, int socketfd, struct sockaddr_in *server_addr, int len, char *active_channel){
    struct request_join req_jn;
    memset(&req_jn, 0, sizeof(req_jn));
    int chnl_len;
    char *chnl = get_command_arg(buff, &chnl_len);
    strncpy(req_jn.req_channel, chnl, chnl_len);
    req_jn.req_type = REQ_JOIN;

    if(sendto(socketfd, &req_jn, sizeof(req_jn), 0, (struct sockaddr *)server_addr, len) < 0){
        write(STDOUT_FILENO, "Could not send join request\n", 29);
        return 0;
    }
    strcpy(active_channel, chnl);
    return 1;
}

int send_say_req(int socketfd, struct sockaddr_in *s_addr, int len, char *active_channel, char *buff){
    struct request_say rs;
    memset(&rs, 0, sizeof(rs));
    strcpy(rs.req_channel, active_channel);
    rs.req_type = REQ_SAY;
    strcpy(rs.req_text, buff);
    
    if(sendto(socketfd, &rs, sizeof(rs), 0, (struct sockaddr *)s_addr, len) < 0){
        write(STDOUT_FILENO, "Could not send say request\n", 28);
        return 0;
    }
    return 1;
}

int send_list_req(int socketfd, struct sockaddr_in *s_addr, int len){
    struct request_list rl;
    memset(&rl, 0, sizeof(rl));
    rl.req_type = REQ_LIST;
    if(sendto(socketfd, &rl, sizeof(rl), 0, (struct sockaddr *)s_addr, len) < 0){
        write(STDOUT_FILENO, "Could not send list request\n", 28);
        return 0;
    }
    return 1;
}

int send_who_req(int socketfd, struct sockaddr_in *s_addr, int len, char *buff){
    int chnl_len;
    char *chnl_name = get_command_arg(buff, &chnl_len);
    struct request_who re_who;
    memset(&re_who, 0, sizeof(re_who));
    re_who.req_type = REQ_WHO;
    strncpy(re_who.req_channel, chnl_name, chnl_len);
    if(sendto(socketfd, &re_who, sizeof(re_who), 0, (struct sockaddr *)s_addr, len) < 0){
        write(STDOUT_FILENO, "Could not send who request\n", 28);
        return 0;
    }
    return 1;
}

int send_leave_req(int socketfd, struct sockaddr_in *s_addr, int len, char *buff, char *active_channel){
    struct request_leave req_l;
    memset(&req_l, 0, sizeof(req_l));
    req_l.req_type = REQ_LEAVE;
    int chnl_len;
    char *chnl = get_command_arg(buff, &chnl_len);
    strncpy(req_l.req_channel, chnl, chnl_len);
    if (strncmp(active_channel, chnl, chnl_len) == 0)
    {
        strcpy(active_channel, "Common");
    }
    
    if(sendto(socketfd, &req_l, sizeof(req_l), 0, (struct sockaddr *)s_addr, len) < 0){
        write(STDOUT_FILENO, "Could not send who request\n", 28);
        return 0;
    }
    return 1;
}

int send_logout_req(int socketfd, struct sockaddr_in *s_addr, int len){
    struct request_logout req_lg;
    memset(&req_lg, 0, sizeof(req_lg));
    req_lg.req_type = REQ_LOGOUT;
    if(sendto(socketfd, &req_lg, sizeof(req_lg), 0, (struct sockaddr *)s_addr, len) < 0){
        write(STDOUT_FILENO, "Could not send logout reqeust\n", 30);
        return 0;
    }
    return 1;
}

Channel *find_channel(Channel **chnls, int chnls_len, char* channel_name){
    for (int i = 0; i < chnls_len; i++)
    {
        if (strcmp(chnls[i]->chnl_name, channel_name) == 0)
        {
            return chnls[i];
        }
        
    }
    return NULL;
}

User *find_user(User **usrs, int usrs_len, struct sockaddr_in addr, int *position){
    for (int i = 0; i < usrs_len; i++)
    {
        if (comp_sockaddr(usrs[i]->address, addr))
        {
            if (position != NULL)
            {
                *position = i;
            }
            return usrs[i];
        }
    }
    return NULL;
}

Channel *add_chnl(Channel **chnls, int *num_chnls, char *chnl_name){
    Channel *chnl = create_channel(chnl_name);
    chnls[(*num_chnls)++] = chnl;
    return chnl;
}

Channel *remove_chnl(Channel **chnls, int *num_chnls, char *chnl_name){
    Channel *chnl;
    int move = 0;
    int i;
    for (i = 0; i < *num_chnls; i++)
    {
        if (move)
        {
            chnls[i-1] = chnls[i];

        }else if(strcmp(chnls[i]->chnl_name, chnl_name) == 0){
            chnl = chnls[i];
            move = 1;
            printf("Destroyed channel\n");
        }
        
    }
    chnls[i] = NULL;
    *num_chnls -= 1;
    return chnl;

}
