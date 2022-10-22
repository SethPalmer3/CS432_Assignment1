#include "channels.h"
#include "duckchat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Find user with username username in chnl
// Return NULL if cannot find
User *find_users(Channel *chnl, char *username){
    for (int i = 0; i < chnl->num_users; i++)
    {
        if (strcmp(username, chnl->connected_users[i]->username) == 0)
        {
            return chnl->connected_users[i];
        }
        
    }
    return NULL;
}

// Add an already existing user to channel chnl
void add_user(Channel *chnl, User*usr){
    chnl->connected_users[chnl->num_users++] = usr;
}

// Create a user with name username and with address addr
User *create_user(Channel *chnl, char *username, struct sockaddr_in *addr, unsigned int len){
    User *new_u = (User *)malloc(sizeof(User));
    strcpy(new_u->username, username);
    new_u->address = *addr;
    new_u->addresslen = len;
    chnl->connected_users[chnl->num_users] = new_u;
    chnl->num_users++;
    return new_u;
}

// Remove a user with username username from chnl and return that user
User *remove_user(Channel *chnl, char *username){
    int move = 0;
    User *ret;
    for (int i = 0; i < chnl->num_users; i++)
    {
        if (move) // Move the later elements back to fill gap
        {
            chnl->connected_users[i-1] = chnl->connected_users[i];
            chnl->connected_users[i] = NULL;
        }else if (strcmp(chnl->connected_users[i]->username, username) == 0) // Find user to remove
        {
            ret = chnl->connected_users[i];
            move = 1;
        }
        
    }
    chnl->num_users--;
    return ret;
}

// Compare two addresses
int comp_sockaddr(struct sockaddr_in a, struct sockaddr_in b){
    return (a.sin_addr.s_addr == b.sin_addr.s_addr) &&
            (a.sin_port == b.sin_port);
}

// Find a user in chnl with address addr
User *find_byaddr(Channel *chnl, struct sockaddr_in addr){
    for (int i = 0; i < chnl->num_users; i++)
    {
        if (comp_sockaddr(chnl->connected_users[i]->address, addr))
        {
            return  chnl->connected_users[i];
        }
        
    }
    return NULL;
    
}

// Destroy chnl
void destroy(Channel *chnl){
   
   free(chnl);
}

void init_channel(Channel *chnl){
    chnl->create_user = create_user;
    chnl->find_user = find_users;
    chnl->add_user = add_user;
    chnl->remove_user = remove_user;
    chnl->find_byaddr = find_byaddr;
    chnl->destroy = destroy;
    chnl->num_users = 0;
}

// Create a channel with the channel name chnl_name
Channel *create_channel(char *chnl_name){
    Channel *chnl = (Channel *)malloc(sizeof(Channel));

    chnl->create_user = create_user;
    chnl->find_user = find_users;
    chnl->add_user = add_user;
    chnl->remove_user = remove_user;
    chnl->find_byaddr = find_byaddr;
    chnl->destroy = destroy;
    chnl->num_users = 0;
    strcpy(chnl->chnl_name, chnl_name);
    return chnl;
}
