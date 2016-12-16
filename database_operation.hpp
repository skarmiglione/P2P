/*
 * database_operation.h
 *
 *  Created on: Dec 12, 2016
 *      Author: dragos
 */

#ifndef DATABASE_OPERATION_HPP_
#define DATABASE_OPERATION_HPP_

#include <mysql/my_global.h>
#include <mysql/mysql.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <netinet/in.h>

#define HOST "localhost"
#define PASSWORD "password"
#define USER "root"
#define DB "P2P"

class DatabaseQueryParameters
{
private:
	MYSQL * database;
	int * client;
	struct sockaddr_in clientInfo;

public:
	DatabaseQueryParameters(MYSQL * database, int * client, struct sockaddr_in clientInfo)
	{
		this->database = database;
		this->client = client;
		this->clientInfo = clientInfo;
	}

	MYSQL * getDatabase() {
		return this->database;
	}

	int * getClient() {
		return this->client;
	}

	struct sockaddr_in getClientInfo() {
		return this->clientInfo;
	}
};

void readError();

void writeError();

void databaseConnectionError() {
	perror("Error while connecting to database...");
}

void databaseQueryError() {
	perror("Query error");
}

void connectToDatabase(MYSQL *& databaseConnection)
{
	databaseConnection = mysql_init(NULL);
	if(databaseConnection == NULL)
		databaseConnectionError();
	if(mysql_real_connect(databaseConnection, HOST, USER, PASSWORD, DB, 0, NULL, 0) == NULL)
		databaseConnectionError();
}

MYSQL_RES * query(MYSQL * databaseConnection, char * sqlInstruction)
{
	printf("SQL : %s\n", sqlInstruction);
	if(mysql_query(databaseConnection, sqlInstruction))
		databaseQueryError();
	MYSQL_RES * queryResult = mysql_store_result(databaseConnection);
	return queryResult;
}

void updateUserStatus(MYSQL * database, struct sockaddr_in clientInfo, char * id)
{
	char sqlCommand[100];
	sprintf(sqlCommand, "update UserStatus set status = 'online', ip = '%s' where id = %d", inet_ntoa(clientInfo.sin_addr), atoi(id));
	query(database, sqlCommand);
}

void dropUserAvailableFiles(MYSQL * database, char * id)
{
	char sqlCommand[100];
	sprintf(sqlCommand, "delete from Files where id = %d", atoi(id));
	query(database, sqlCommand);
}

void sendDownloadPathToClient(MYSQL * database, int &client, char * id)
{
	char sqlCommand[100];
	MYSQL_RES * result;
	MYSQL_ROW outputRow;
	sprintf(sqlCommand, "select DownloadPath from UserInfo where id = %d", atoi(id));
	result = query(database, sqlCommand);
	outputRow = mysql_fetch_row(result);
	if(write(client, outputRow[0], strlen(outputRow[0])) == -1)
		writeError();
}

void insertUserAvailableFiles(MYSQL * database, int &client, char * id)
{
	char sqlCommand[100];
	char sizeStr[30], fileName[100], fileHash[64];
	int sizeOfFile, readBytes, sizeOfFileName;
	while((readBytes = read(client, &sizeOfFileName, 4)) > 0 &&
			(readBytes = read(client, fileName, sizeOfFileName)) > 0 &&
			(readBytes = read(client, &sizeOfFile, 4)) > 0 &&
			(readBytes = read(client, fileHash, 64)) > 0)
	{
		sprintf(sqlCommand, "insert into Files value (%d, '%s', %d, '%s')", atoi(id), fileName, sizeOfFile, fileHash);
		query(database, sqlCommand);
	}
	if(readBytes == -1)
		readError();
}

void insertInUserInfo(MYSQL * database, char username[50], char hashPassword[64], char downloadPath[512])
{
	char sqlCommand[1024];
	sprintf(sqlCommand, "insert into UserInfo value (NULL, '%s', '%s', '%s')", username, hashPassword, downloadPath);
	query(database, sqlCommand);
}

void insertInUserStatus(MYSQL * database, struct sockaddr_in clientInfo)
{
	char sqlCommand[100];
	sprintf(sqlCommand, "insert into UserStatus value (NULL, 'offline', '%s')", inet_ntoa(clientInfo.sin_addr));
	query(database, sqlCommand);
}

#endif /* DATABASE_OPERATION_HPP_ */
