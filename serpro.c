#include "serpro.h"
#define user_name 'u'
#define snake_body 'b'
#define snake_direction 'd'
#define game_food 'f'
#define game_score 's'
#define game_record 'r'
#define ask_for_body 'B'
#define ask_for_dir 'D'
#define ask_for_food 'F'
#define ask_for_score 'S'
#define ask_for_record 'R'
#ifdef NDEBUG
#define writedb(user) {	char buff[1000];\
						sprintf_s(buff,200, "update snakesdata set body = '%s',food='%s',direction='%s',score='%s',record='%s' where user_id='%s'", body, food, direction, score, record, user);\
						RETCODE retcode;\
						while ((retcode = SQLExecDirect(hstmt1, buff, SQL_NTS)) < 0) {\
							printf("senddata to database error,press any key to retry...\n");\
							system("pause>>nul");\
						}\
						}
#endif // NDEBUG

char body[8] = { 'b',20,20,20,21,20,22 };
char food[4] = { 'f',14,18 };//横坐标一定为偶数
char direction[3]="dd";
char score[3] = { 's',0 };
char record[3] = { 'r',1 };

	SQLHENV  henv = SQL_NULL_HENV;//定义环境句柄
	SQLHDBC  hdbc1 = SQL_NULL_HDBC;//定义数据库连接句柄     
	SQLHSTMT  hstmt1 = SQL_NULL_HSTMT;//定义语句句柄


#ifndef NDEBUG
void writedb(const char *colname, const char *value, const char *user)
{
	char buff[200];
	sprintf_s(buff,200, "update snakesdata set %s = '%s' where user_id='%s'", colname, value, user);
	RETCODE retcode;
	while ((retcode = SQLExecDirect(hstmt1, buff, SQL_NTS)) < 0) {
		printf("senddata to database error,press any key to retry...\n");
		system("pause>>nul");
	}
}
#endif // !NDEBUG


short readdb(const char *user)
{
	char buff[200];
	sprintf_s(buff,200, "select users,codes,body,food,direction,score,record from usersdata join snakesdata on users=user_id where users='%s';" , user);
	RETCODE retcode;
	while ((retcode = SQLExecDirect(hstmt1, buff, SQL_NTS)) < 0) {
		if (retcode == SQL_ERROR) {
			SQLSMALLINT errmsglen;
			SQLINTEGER errnative;
			UCHAR errmsg[255];
			UCHAR errstate[6];
			SQLGetDiagRec(SQL_HANDLE_STMT, hstmt1, 1, errstate, &errnative, errmsg, sizeof(errmsg), &errmsglen);
			if(strcmp("42S02", errstate)==0)//读取不到该数据，即账号不存在
				return 0;
		}
		printf("readdata from database error,press any key to retry...\n");
		system("pause>>nul");
	}
	return 1;
}

void reset(const char *user)
{
#ifndef NDEBUG
	writedb("body", body, user);
	writedb("food", food, user);
	writedb("direction", direction, user);
	writedb("score", score, user);
	writedb("record", record, user);
#endif // !NDEBUG
#ifdef NDEBUG
	writedb(user);
#endif // NDEBUG

}


void str_ser(int connfd) {
	char user[20];
	char codes[20];
	char body[200] = "\0" ;
	char food[4];
	char direction[3];
	char score[3];
	char record[3];
	RETCODE retcode;

	int n;
	char recvline[MAXLINE];

	for (;;) {
	again:
		if ((n = recv(connfd, recvline, MAXLINE, 0)) > 0) {
			recvline[n] = 0;
			switch (recvline[0])//判断发送数据的头字母，区分不同的数据
			{
			case user_name:

				strcpy(user, recvline);
				if (readdb(user)) {
					SQLLEN columnLen = 0;
					retcode = SQLBindCol(hstmt1, 1, SQL_C_CHAR, user, 200, &columnLen);
					retcode = SQLBindCol(hstmt1, 2, SQL_C_CHAR, codes, 200, &columnLen);
					retcode = SQLBindCol(hstmt1, 3, SQL_C_CHAR, body, 200, &columnLen);
					retcode = SQLBindCol(hstmt1, 4, SQL_C_CHAR, food, 200, &columnLen);
					retcode = SQLBindCol(hstmt1, 5, SQL_C_CHAR, direction, 200, &columnLen);
					retcode = SQLBindCol(hstmt1, 6, SQL_C_CHAR, score, 200, &columnLen);
					retcode = SQLBindCol(hstmt1, 7, SQL_C_CHAR, record, 200, &columnLen);
					retcode = SQLFetch(hstmt1);//从数据库获取数据
					retcode = SQLCloseCursor(hstmt1);//执行select语句会分配一个cursor，下次执行必须先关闭该cursor
					if ((n = recv(connfd, recvline, MAXLINE, 0)) > 0)
					{
						recvline[n] = 0;
						char rpassword[20];
						strcpy(rpassword, recvline);
						if (!strcmp(codes, rpassword)) {
							send(connfd, "y", 1, 0);//验证成功，发送y确认
							break;
						}
					}
				}
				send(connfd, "n", 1, 0);//失败发送n
				break;
			case snake_body:
				strcpy(body, recvline);
				send(connfd, "LWB", 3, 0);
				break;
			case game_food:
				strcpy(food, recvline);
				send(connfd, "LWB", 3, 0);
				break;
			case snake_direction:
				strcpy(direction, recvline);
				send(connfd, "LWB", 3, 0);
				break;
			case game_score:
				strcpy(score, recvline);
				send(connfd, "LWB", 3, 0);
				break;
			case game_record:
				strcpy(record, recvline);
				send(connfd, "LWB", 3, 0);
				break;
			case ask_for_body:
				send(connfd, body, strlen(body), 0);
				break;
			case ask_for_dir:
				send(connfd, direction, strlen(direction), 0);
				break;
			case ask_for_food:
				send(connfd, food, strlen(food), 0);
				break;
			case ask_for_score:
				send(connfd, score, strlen(score), 0);
				break;
			case ask_for_record:
				send(connfd, record, strlen(record), 0);
				break;
			default:
				break;
			}
		}
		if (n < 0 && errno == EINTR)
			goto again;
		if (n <= 0)//客户端连接断开
		{
			if (strlen(body)) {
#ifndef NDEBUG
				writedb("body", body, user);
				writedb("food", food, user);
				writedb("direction", direction, user);
				writedb("score", score, user);
				writedb("record", record, user);
#endif // !NDEBUG
#ifdef NDEBUG
				writedb(user);
#endif // NDEBUG
			}
			break;
		}
	}
}

short connecttodb() {
	RETCODE retcode;//错误返回码
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);//allocate environment handle
	if (retcode < 0)//错误处理 
	{
		printf("allocate ODBC Environment handle errors.\n");
		return 0;
	}
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
		(SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);//set environment handle attributes
	if (retcode < 0) //错误处理
	{
		printf("the  ODBC is not version3.0\n ");
		return 0;
	}
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);//allocate connection handle based on allocated environment handle
	if (retcode < 0) //错误处理
	{
		printf("allocate ODBC connection handle errors.\n");
		return 0;
	}

	char* szDSN = "mysqltest";//添加数据源时，为其起的名字
	char* szUID = "root";
	char* szAuthStr = "VIvi3154";

	retcode = SQLConnect(hdbc1,
		(SQLCHAR*)szDSN,
		(SWORD)strlen(szDSN),
		(SQLCHAR*)szUID,
		(SWORD)strlen(szUID),
		(SQLCHAR*)szAuthStr,
		(SWORD)strlen(szAuthStr));

	if (retcode < 0) //错误处理
	{
		printf("connect to  ODBC datasource errors.\n");
		return 0;
	}

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);  //allocte s
	if (retcode < 0)
	{
		printf("allocate ODBC statement handle errors.\n");
		return 0;
	}
	return 1;
}

