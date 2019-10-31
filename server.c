#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>  
#include <arpa/inet.h>  
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>  
#include <sys/un.h>

#define SOCK_PATH "relay_socket" 

void final_str_prep(char tomod[], char buff[], int usr_id, int stop);
void cpy_str_nn(int ind, char tmp[] ,char a[]);
int check_for_rate(char str[]);

int main(int argc , char *argv[]){ 
	int listenSocket, addrlen, new_socket, max_clients = 10, activity, i, valread, sd; 
	int sd_for_select; 
	char buffer[1025];
	struct sockaddr_un address;
	int all_clients[10]; 		
	for (i = 0; i < max_clients; i++){ 
		all_clients[i] = 0; 
	} 
	fd_set fds; 
	if((listenSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == 0){ 
		perror("socket failed"); 
		exit(1); 
	} 
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCK_PATH);
    unlink(address.sun_path);
	if (bind(listenSocket, (struct sockaddr *)&address, sizeof(address))<0){ 
		perror("bind failed"); 
		exit(1); 
	} 
	printf("Listener on port \n"); 
	if (listen(listenSocket, 5) < 0){ 
		perror("listen"); 
		exit(1); 
	} 	
	addrlen = sizeof(address); 
	printf("Waiting for connections ...\n"); 
	char *message = "You've joined the chatroom \n"; 
	while(1){ 	
		FD_ZERO(&fds); 
		FD_SET(listenSocket, &fds); 
		sd_for_select = listenSocket; 
		for ( i = 0 ; i < max_clients ; i++) { 
			sd = all_clients[i]; 
			if(sd > 0){FD_SET( sd , &fds);}
			if(sd > sd_for_select){sd_for_select = sd;}
		} 
		activity = select(sd_for_select+1, &fds , NULL , NULL , NULL); 
		if ((activity < 0) && (errno!=EINTR)){ 
			printf("select error"); 
		} 
		if(FD_ISSET(listenSocket, &fds)){ 
			if ((new_socket = accept(listenSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){ 
				perror("accept"); 
				exit(1); 
			} 
			printf("New connection, socket fd is %d, sock_path is %s \n" , new_socket , address.sun_path); 
            char other_message[18] = "#user : Hey All!\n\0";
			send(new_socket, message, strlen(message), 0);
			puts("Welcome message sent successfully"); 
			int new_user_index;
			for (i = 0; i < max_clients; i++) 
			{ 
				if( all_clients[i] == 0 ) 
				{ 
					all_clients[i] = new_socket; 
					printf("Adding to list of sockets at index = %d\n" , i); 
                    new_user_index = i;
					break; 
				} 
			}
            char app_other_message = new_user_index+'0';
            other_message[5] = app_other_message;
            printf("%s\n", other_message);
            for(i = 0; i < max_clients; i++)
            {
                if(all_clients[i]!=0 && i!=new_user_index){
					send(all_clients[i], other_message, strlen(other_message), 0); 
                }
            }
		}        
		for (i = 0; i < max_clients; i++){ 
			sd = all_clients[i]; 	
			if (FD_ISSET(sd, &fds)){ 
				if ((valread = read( sd , buffer, 1024)) == 0){ 
					printf("Host disconnected\n"); 	
					close( sd ); 
					all_clients[i] = 0; 
				} else { 
                    int index, priv_socket;
                    index = check_for_rate(buffer);
                    if(index!=-1){
						printf("entered priv\n");
                        char tmp[index];
                        cpy_str_nn(index, tmp, buffer);
                        char fin_msg_usr[strlen(tmp)+9];
                        int to_stop = index+8;
                        final_str_prep(fin_msg_usr, tmp, i, to_stop);
                        int priv_msg_index = buffer[index+5]-'0';
						if(priv_msg_index < max_clients){
							priv_socket = all_clients[priv_msg_index];
							if(priv_socket > 0){
							if( send(priv_socket, fin_msg_usr, strlen(fin_msg_usr), 0) != strlen(fin_msg_usr)) 
			            	{ 
				        	    perror("send"); 
			            	} } else{
								char *errormsg2="No such user exists!\n";
							if( send(sd, errormsg2, strlen(errormsg2), 0) != strlen(errormsg2)) 
			            	{ 
				        	    perror("send"); 
			            	}
							}
						} else{
							char *errormsg="No such user exists!\n";
							if( send(sd, errormsg, strlen(errormsg), 0) != strlen(errormsg)) 
			            	{ 
				        	    perror("send"); 
			            	}
						}
						printf("exited priv\n");
						memset(&buffer, 0, sizeof(buffer));
                    }
                    else{
						printf("enetered public\n");
                        char mesg_for_all[valread+10];
                        int toStop = valread+8;
                        final_str_prep(mesg_for_all, buffer, i, toStop);
                        for(int j=0;j<max_clients;j++){
                            if(all_clients[j]!=sd && all_clients[j]!=0){
                                if( send(all_clients[j], mesg_for_all, strlen(mesg_for_all), 0) != strlen(mesg_for_all)) 
			                    { 
				                    perror("send"); 
			                    } 
                            }
                        }
						printf("exited public\n");
						memset(&buffer, 0, sizeof(buffer));
                    }
				} 
			} 
		} 
	} 
		
	return 0; 
} 

void final_str_prep(char tomod[], char buff[], int usr_id, int stop){
    tomod[0] = '#';
    tomod[1] = 'u';
    tomod[2] = 's';
    tomod[3] = 'e';
    tomod[4] = 'r';
    tomod[5] = ' ';
    tomod[6] = ':';
    tomod[7] = ' ';
    printf("check1 \n");
    char usr_id_chr = usr_id+'0';
    tomod[5] = usr_id_chr;
    for(int x=8;x<stop;x++){
        tomod[x] = buff[x-8];
    }
    tomod[stop] = '\n';
    tomod[stop+1]='\0';
}

void cpy_str_nn(int ind, char tmp[] ,char a[]){
    for(int i=0;i<ind;i++){
        tmp[i] = a[i];
    }
}

int check_for_rate(char str[]){
    for (int i = 0; i < strlen(str)-1; i++)
    {
        if(str[i]=='@' && (str[i+1]=='u' || str[i+1]=='U')){
            return i;
        }
    }
    return -1;
}
