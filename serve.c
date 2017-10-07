#pragma once
#include "serpro.h"

int main(int argc, char **argv)
{
	while (!connecttodb()) {
		printf("connect to database error,press any key to retry...\n");
		system("pause>>nul");
	}
#ifndef NDEBUG
	reset("uaa");//重置数据库中的数据
#endif // !NDEBUG

	printf("connect to database succeed!\n");
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		exit(0);
	}

	SOCKET	 listenfd, connfd=-1;
	struct sockaddr_in	servaddr;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(9999);	// daytime server 

	bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	for (;;) {

		if ((connfd = accept(listenfd, (SA *)NULL, NULL)) < 0) {
			if (errno == EINTR)
				continue;
			else {
				printf("accept error");
				//return 0;
			}
		}
		else{
			printf("connected\n");
			str_ser(connfd);
			printf("disconnected!\n");
			closesocket(connfd);
		}
	}

	exit(0);
}
