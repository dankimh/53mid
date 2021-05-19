#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
    
#define BUF_SIZE 1024 //max input buffer size
#define RDY_MSG_SIZE 5 //max ready sign length
#define SENDER 0
#define RECEIVER 1
    
void * send_msg(void * arg); //send message to server(for sender)
void * recv_msg(void * arg); //receive message from server(for receiver)
//void error_handling(char *msg);

char msg[BUF_SIZE]; //message buffer
int main(int argc, char *argv[])
{
//client socket descriptor and server address structure
    int sock;
    struct sockaddr_in serv_addr;
    
//thread id for sender&receiver and thread return value(pthread_join)
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
    
    int str_len,ch=0;
    char send_rdy_msg[]="READY: You are a sender\n";
    char recv_rdy_msg[]="READY: You are a receiver\n";
    char ready_msg[RDY_MSG_SIZE];
    
    //argv error handling
    if (argc != 3) {
        printf("usage : %s <ip> <port> \n" , argv[0]);
        exit(1);
    }
    
    //IPv4, TCP socket create
    sock = socket(PF_INET, SOCK_STREAM,0);
    
    //server address information initialization
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
    
    //connect request
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    //waiting for server's ready sign
    while(1){
    	str_len=read(sock,ready_msg,RDY_MSG_SIZE);
    	if(str_len==2){ //if client is sender
    		fputs(send_rdy_msg,stdout);
    		ch=SENDER;
    		break;
    	}
    	else if(str_len==3){ //if client is receiver
    		fputs(recv_rdy_msg,stdout);
    		ch=RECEIVER;
    		break;
    	}
    }
    //thread create
    if(ch==SENDER){
    	pthread_create(&snd_thread,NULL, send_msg, (void*)&sock);
    	pthread_join(snd_thread,&thread_return);
    }
    
    if(ch==RECEIVER){
    	pthread_create(&rcv_thread,NULL, recv_msg, (void*)&sock);
    	pthread_join(rcv_thread,&thread_return);
    }
    
    //client socket connection close
    close(sock);
    return 0;
}
    
//sender
void* send_msg(void* arg){
    
    //client's file descriptor
    int sock = *((int*)arg);
    
//user input
    fgets(msg,BUF_SIZE,stdin);
    
//send message to server
    write(sock,msg,strlen(msg));
    
//socket close
    close(sock);
    return NULL;
}

//receiver
void* recv_msg(void* arg){
    
    //client's file descriptor
    int sock = *((int*)arg);
    char name_msg[BUF_SIZE];
    int str_len;
    
    while (1) {
        
	//read buffer from server
        str_len = read(sock, name_msg ,BUF_SIZE-1);
        if(str_len){ //if message sent from server
        	name_msg[str_len] = '\0';
        	fputs(name_msg, stdout);
        	break;
        }
        else if (str_len == -1) { //fail to read
            return (void*)-1;
        }
        
    }
//socket close
    close(sock);
    return NULL;
}

//error handling
/*void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}*/
