#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#define MAXLINE 200
#define SA struct sockaddr
#define LISTENQ 5
//void str_ser(int connfd);

#include <sql.h>
#include <odbcss.h>
#include <sqlext.h>
#include <Windows.h>

#define NDEBUG
void str_ser(int connfd);

int checkuser(char *, char*);

short connecttodb();