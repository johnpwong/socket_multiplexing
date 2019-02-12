#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <sys/select.h>
#include <sys/time.h>


#define CN 4 //connect num


int connfd[CN];		//save the connect sock id 
fd_set rset;

void routine() //be responsible for block select
{
	struct timeval t = {0, 500};	//set timeout
	
	char *msg = malloc(100);
	while(1) {
		FD_ZERO(&rset);
		int maxfd = -1;
		for(int i=0; i<CN; i++) {
			if(connfd[i] > 0) {
				FD_SET(connfd[i], &rset);	
				maxfd = connfd[i] > maxfd ? connfd[i] : maxfd;
			}
		}

		if(select(maxfd+1, &rset, NULL, NULL, &t) > 0) { 
		
			for(int i=0; i<CN; i++) {
				if(connfd[i] > 0) {
					if(FD_ISSET(connfd[i], &rset)) {
						bzero(msg, 100);
						int n = read(connfd[i], msg, 100);
						if(n == 0){
							connfd[i] = -1;
							printf("disconnect\n");
							break;
						}
						else if(n>0) {
							printf("get info: %s", msg);
							for(int j=0; j<CN; j++) {
								if(connfd[j]>0 && j!=i)
									write(connfd[j], msg, strlen(msg));
							}
						}
					}
				}
			}
		}
		

		

	}
}

int main(int argc, char *argv[])
{
	for(int i=0; i<CN; i++) {
		connfd[i] = -1;
	}

	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1) {
		perror("socket");
		exit(-1);
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[1]));
	bind(sfd, (struct sockaddr *)&addr, sizeof(addr));
	
	listen(sfd, CN);

	pthread_t tid;
	pthread_create(&tid, NULL, (void *)routine, NULL);

	struct sockaddr_in cli_addr;
	socklen_t len = sizeof(cli_addr);

	while(1){		//main pthread is responsible for accept socket
		bzero(&cli_addr, len);

		int index;
		for(index=0; index<CN; index++) {
			if(connfd[index]==-1) {
				break;
			}
		}

		connfd[index] = accept(sfd, (struct sockaddr *)&cli_addr, &len);
		printf("accept %s\n", inet_ntoa(cli_addr.sin_addr));
	}
	
	
	return 0;

}

