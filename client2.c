#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCK_PATH "relay_socket"
int s, t, len;

void *recmsg(){
    char r_buff[100];
    while(1){
        if ((t=recv(s, r_buff, 100, 0)) > 0) {
            r_buff[t] = '\0';
            printf("%s\n", r_buff);
            fflush(stdout);
        } 
        else {
            if (t < 0) perror("recv");
            else printf("Server closed connection\n");
            exit(1);
        }
    }
}

/*void writemsg(){
    while(1){
        fflush(stdout);
        char str[200];
        while(fgets(str, 200, stdin)!=NULL){
            if(strlen(str)==0){
                fflush(stdout);
            } else {
                break;
            }
        }
        if (send(s, str, strlen(str), 0) == -1) {
            perror("send");
            exit(1);
        }   
    }

}*/

int main(void)
{
    pthread_t receiver_thread;
    struct sockaddr_un remote;
    char str[200];
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    printf("Trying to connect...\n");
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }
    printf("Connected.\n");
    if(pthread_create(&receiver_thread, NULL, (void *)recmsg, NULL)!=0){
        printf("pthread create error\n");
        exit(1);
    }
    while(fgets(str, 200, stdin)>0){
        if(strlen(str)<0){
            break;
        }
        if (send(s, str, strlen(str), 0) == -1) {
            perror("send");
            exit(1);
        }
    }
    pthread_join(receiver_thread, NULL);
    close(s);
    return 0;
}

