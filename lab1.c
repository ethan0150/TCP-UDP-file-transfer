#define _GNU_SOURCE
#define BUFLEN 1000
#define BACKLOG 1
#define BUFCLR bzero(buf,BUFLEN);
#include<sys/timeb.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdbool.h>
typedef long double ld;
int lis_sock,psvo_sock,yes=1,act_sock;//lis_sock: passive socket, responsible for listening to new client
socklen_t cli_addrlen;
struct sockaddr_in ser_addr,cli_addr;//psvo_sock: passive open socket, responsible for the actual data transfer
long f_size;//recvsize is in byte;
size_t rd_size,rcv_size;
char buf[BUFLEN],mtime[BUFLEN];
struct timeb start,end;
struct tm * tinfo;
time_t rawtime;
FILE *fp;
bool q1=0,q2=0,q3=0;
static inline void getmtime(){
	time(&rawtime);
	tinfo=localtime(&rawtime);
	strftime(mtime,BUFLEN,"%Y/%m/%d %T",tinfo);
	return;
}
static inline ld trPctg(){
	return (ld)ftell(fp)/(ld)(f_size);
}
int main(int argc, char *argv[]){
	
	bzero(&ser_addr,sizeof(struct sockaddr_in));
	bzero(&cli_addr,sizeof(struct sockaddr_in));
	BUFCLR
	cli_addrlen=sizeof(struct sockaddr_in);
	ser_addr.sin_family=AF_INET;
	ser_addr.sin_port=htons(atoi(argv[4]));
	setenv("TZ", "GMT-8", 0);
	
	if(strcmp("tcp",argv[1])==0 && strcmp("recv",argv[2])==0){
		ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);
		if((lis_sock=socket(AF_INET,SOCK_STREAM,0))==-1){
			perror("socket()");
			return 0;
		}
		if(setsockopt(lis_sock, SOL_SOCKET, SO_REUSEADDR,&yes,sizeof(int)) == -1){
			perror("setsockopt()");
			return 0;
		}
		if(bind(lis_sock,(struct sockaddr*)&ser_addr,(socklen_t)sizeof(struct sockaddr_in))==-1){
			perror("bind()");
			return 0;
		}
		if(listen(lis_sock,BACKLOG)==-1){
			perror("listen()");
			return 0;
		}
		printf("W8ing for connection...\n");
		if((psvo_sock=accept(lis_sock,(struct sockaddr*)&cli_addr,(socklen_t * restrict)&cli_addrlen))==-1){
			perror("accept()");
			return 0;
		}
		//recv filename
		recv(psvo_sock,(void *)buf,BUFLEN,0);
		if((fp=fopen(buf,"wb"))==NULL){
			perror("fopen()");
			return 0;
		}
		//recv f_size
		BUFCLR
		recv(psvo_sock,(void *)buf,BUFLEN,0);
		f_size=atol(buf);
		BUFCLR
		ftime(&start);
		while((rcv_size=recv(psvo_sock,(void *)buf,BUFLEN,0)) > 0){
			if(rcv_size<0)perror("recv()");
			if(trPctg()>0.25 && !q1){
				q1=1;
				getmtime();
				printf("25%% %s\n",mtime);
			}
			if(trPctg()>0.5 && !q2){
				q2=1;
				getmtime();
				printf("50%% %s\n",mtime);
			}
			if(trPctg()>0.75 && !q3){
				q3=1;
				getmtime();
				printf("75%% %s\n",mtime);
			}
			fwrite((const void *)buf,1,rcv_size,fp);
			BUFCLR
		}
		ftime(&end);
		getmtime();
		printf("100%% %s\n",mtime);
		printf("File transmission completed in %d ms.\n",(int)(1000.0*(end.time-start.time)+end.millitm-start.millitm));
		printf("File size: %ld bytes\n",f_size);
		fclose(fp);
		close(psvo_sock);
		close(lis_sock);
	}

	else if(strcmp("tcp",argv[1])==0 && strcmp("send",argv[2])==0){
		ser_addr.sin_addr.s_addr=inet_addr(argv[3]);
		
		if((act_sock=socket(AF_INET,SOCK_STREAM,0))==-1){
			perror("socket()");
			return 0;
		}
		
		if(connect(act_sock,(struct sockaddr *)&ser_addr,sizeof(struct sockaddr))==-1){
			perror("connect()");
			return 0;
		}
		//get filename
		BUFCLR
		strcpy(buf,argv[5]);
		if((fp=fopen(argv[5],"rb"))==NULL){
			perror("fopen()");
			return 0;
		}
		//send filename		
		send(act_sock,(void *)buf,BUFLEN,0);
		//get f_size
		fseek(fp, 0L, SEEK_END);
		f_size=ftell(fp);
		rewind(fp);
		BUFCLR
		sprintf(buf,"%ld",f_size);
		//send f_size
		send(act_sock,(void *)buf,BUFLEN,0);
		while(!feof(fp)){
			BUFCLR
			rd_size=fread((void *)buf,1,BUFLEN,fp);			
			send(act_sock,(const void *)buf,rd_size,0);
		}
		perror("send()");
		close(act_sock);
		fclose(fp);
		printf("All info transferred successfully\n");
		return 0;
	}

	else if(strcmp("tcp",argv[1])==0 && strcmp("recv",argv[2])==0){
		return 0;
	}
	else if(strcmp("tcp",argv[1])==0 && strcmp("recv",argv[2])==0){
		return 0;
	}
	else{
		return 0;
	}
	unsetenv("TZ");
	return 0;
}
