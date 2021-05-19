#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define BUF_SIZE 1024 //max input buffer size
#define MAX_CLNT 100 //max acceptable client number
#define GOAL_CLNT 2 //two clients(sender, receiver)
#define SENDER 0
#define RECEIVER 1
void *handle_clnt(void* arg); //client handler
void send_msg(char* msg, int len); //send sender's message to receiver
//number of client on the server
int clnt_cnt = 0;
//client's file descriptor array
int clnt_socks[MAX_CLNT];
//mutex variable for thread syncronization
pthread_mutex_t mutx;
int main(int argc, char * argv[]){
    
	//server socket, client socket & server address, client address
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr , clnt_adr;
	//addrlen(accept)
    socklen_t clnt_adr_sz;
	//thread id(pthread_create)
    pthread_t t_id;
    
	//sign of determining client type(server, receiver)
    char send_rdy_msg[]="0";//"READY: You are a sender\n";
    char recv_rdy_msg[]="00";//"READY: You are a receiver\n";
  
    //argv error handling
    if (argc != 2) {
        printf("usage : %s <port> \n" , argv[0]);
        exit(1);
    }
    
    //initialize mutex
    pthread_mutex_init(&mutx, NULL);
    
    //create IPv4, TCP socket
    serv_sock = socket(PF_INET, SOCK_STREAM,0);
    
    //initialize server address information
    memset(&serv_adr , 0 , sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    
    //allocate server address
    bind(serv_sock,(struct sockaddr*)&serv_adr, sizeof(serv_adr));
    
    //listen for client(max number of 5)
    listen(serv_sock , 5);
    while (1) {
        
        clnt_adr_sz = sizeof(clnt_adr);
        
        //accept client connection request & create new socket for client-server transmission
        clnt_sock = accept(serv_sock , (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        pthread_mutex_lock(&mutx);  //mutex lock
        
        clnt_socks[clnt_cnt++] = clnt_sock; //assign client file descriptor & client count
        pthread_mutex_unlock(&mutx); //mutex unlock
        
		//print client number
        printf("clnt_cnt : %d \n" , clnt_cnt);
        //send ready sign to client when second client is online 
        pthread_mutex_lock(&mutx); //mutex lock
        if(clnt_cnt>=GOAL_CLNT){ //if number of client in server is more than 2
        	write(clnt_socks[SENDER],send_rdy_msg,sizeof(send_rdy_msg)); //send ready message to sender
        	write(clnt_socks[RECEIVER],recv_rdy_msg,sizeof(recv_rdy_msg)); //send ready message to receiver
        }
        pthread_mutex_unlock(&mutx); //mutex unlock
        
        //create thread
        pthread_create(&t_id , NULL, handle_clnt, (void*)&clnt_sock);
        
        //detach thread when socket closed
        pthread_detach(t_id);
        
    }
    return 0;
}
//client handling
void* handle_clnt(void* arg){
	//file descriptor of client
    int clnt_sock = *((int*)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];
    
    //read msg from client(sender)
    while ( (str_len = read(clnt_sock , msg , sizeof(msg))) !=0) {
        send_msg(msg, str_len);
    }
    
    pthread_mutex_lock(&mutx);
    //delete disconnected client
    for (i = 0; i < clnt_cnt; i++) {
        
        if (clnt_sock == clnt_socks[i]) {
            while (i < clnt_cnt -1) {
                clnt_socks[i] = clnt_socks[i+1];
                i++;
            }
            break;
        }
    }
    
    clnt_cnt--; //decrease client number
    pthread_mutex_unlock(&mutx);
	//close client socket
    close(clnt_sock);
    
    printf("clnt_cnt : %d\n",clnt_cnt);
    return NULL;
}
//send to receiver
void send_msg(char*msg, int len){
    
    pthread_mutex_lock(&mutx); //mutex lock(clnt_socks)
    write(clnt_socks[RECEIVER], msg,len); //broadcast message to receiver
    pthread_mutex_unlock(&mutx); //mutex unlock
    return;
}

//error handling
/*void error_handling(char *message){
    
    fputs(message , stderr);
    fputc('\n' , stderr);
    exit(-1);
    
}*/
