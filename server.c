//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h> //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> //close 
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <sys/un.h>

#define TRUE 1 
#define FALSE 0 
#define SOCK_PATH "relay_socket" 
void final_str_prep(char tomod[], char buff[], int usr_id, int stop);
void cpy_str_nn(int ind, char tmp[] ,char a[]);
int check_for_rate(char str[]);

	
int main(int argc , char *argv[]) 
{ 
	int opt = TRUE; 
	int master_socket , addrlen , new_socket , client_socket[30] , 
		max_clients = 30 , activity, i , valread , sd; 
	int max_sd; 
	struct sockaddr_un address; 
		
	char buffer[1025]; //data buffer of 1K 
		
	//set of socket descriptors 
	fd_set wfds; 
		
	//a message 
	char *message = "You've joined the chatroom \r\n"; 
	
	//initialise all client_socket[] to 0 so not checked 
	for (i = 0; i < max_clients; i++) 
	{ 
		client_socket[i] = 0; 
	} 
		
	//create a master socket 
	if( (master_socket = socket(AF_UNIX , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//set master socket to allow multiple connections , 
	//this is just a good habit, it will work without this 
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
		sizeof(opt)) < 0 ) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCK_PATH);
    unlink(address.sun_path);
	//type of socket created
		
	//bind the socket to localhost port 8888 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	printf("Listener on port \n"); 
		
	//try to specify maximum of 3 pending connections for the master socket 
	if (listen(master_socket, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
		
	//accept the incoming connection 
	addrlen = sizeof(address); 
	puts("Waiting for connections ..."); 
		
	while(TRUE) 
	{ 
		//clear the socket set 
		FD_ZERO(&wfds); 
	
		//add master socket to set 
		FD_SET(master_socket, &wfds); 
		max_sd = master_socket; 
			
		//add child sockets to set 
		for ( i = 0 ; i < max_clients ; i++) 
		{ 
			//socket descriptor 
			sd = client_socket[i]; 
				
			//if valid socket descriptor then add to read list 
			if(sd > 0) 
				FD_SET( sd , &wfds); 
				
			//highest file descriptor number, need it for the select function 
			if(sd > max_sd) 
				max_sd = sd; 
		} 
	
		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &wfds , NULL , NULL , NULL); 
	
		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
			
		//If something happened on the master socket , 
		//then its an incoming connection 
		if (FD_ISSET(master_socket, &wfds)) 
		{ 
			if ((new_socket = accept(master_socket, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			
			//inform user of socket number - used in send and receive commands 
			printf("New connection , socket fd is %d , sock_path is %s \n" , new_socket , address.sun_path); 
            char other_message[18] = "#user : Hey All!\n\0";
		
			//send new connection greeting message 
			if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
			{ 
				perror("send"); 
			} 
				
			puts("Welcome message sent successfully"); 
			int new_user_index;
			//add new socket to array of sockets 
			for (i = 0; i < max_clients; i++) 
			{ 
				//if position is empty 
				if( client_socket[i] == 0 ) 
				{ 
					client_socket[i] = new_socket; 
					printf("Adding to list of sockets as %d\n" , i); 
                    new_user_index = i;
					break; 
				} 
			}
            char app_other_message = new_user_index+'0';
            other_message[5] = app_other_message;
            printf("%s\n", other_message);
            for(i = 0; i < max_clients; i++)
            {
                if(client_socket[i]!=0 && i!=new_user_index){
                    if( send(client_socket[i], other_message, strlen(other_message), 0) != strlen(other_message) ) 
			        { 
				        perror("send"); 
			        } 
                }
            }
		} 
			
       
		//else its some IO operation on some other socket 
		for (i = 0; i < max_clients; i++) 
		{ 
			sd = client_socket[i]; 
				
			if (FD_ISSET( sd , &wfds)) 
			{ 
				//Check if it was for closing , and also read the 
				//incoming message 
				if ((valread = read( sd , buffer, 1024)) == 0) 
				{ 
					//Somebody disconnected , get his details and print 
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen); 
					printf("Host disconnected\n"); 
						
					//Close the socket and mark as 0 in list for reuse 
					close( sd ); 
					client_socket[i] = 0; 
				} 
					
				//Echo back the message that came in 
				else
				{ 
                    int index, tit;
                    index = check_for_rate(buffer);
                    if(index!=-1){
                        char tmp[index];
                        cpy_str_nn(index, tmp, buffer);
                        char fin_msg_usr[strlen(tmp)+9];
                        int to_stop = index+8;
                        final_str_prep(fin_msg_usr, tmp, i, to_stop);
                        int lol = buffer[index+5]-'0';
                        tit = client_socket[lol];
                        printf("%s\n", fin_msg_usr);
                        printf("%d\n", tit);
                        if( send(tit, fin_msg_usr, strlen(fin_msg_usr), 0) != strlen(fin_msg_usr)) 
			            { 
				            perror("send"); 
			            } 
                    }
                    else{
                        //buffer[valread] = '\0'; 
                        int tonton = valread+10;
                        char mesg_for_all[tonton];
                        int toStop = valread+8;
                        final_str_prep(mesg_for_all, buffer, i, toStop);
                        for(int j=0;j<max_clients;j++){
                            if(j!=sd && client_socket[j]!=0){
                                if( send(client_socket[j], mesg_for_all, strlen(mesg_for_all), 0) != strlen(mesg_for_all)) 
			                    { 
				                    perror("send"); 
			                    } 
                            }
                        }
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
