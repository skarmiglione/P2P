/*
 * FunctionArray.hpp
 *
 *  Created on: Dec 26, 2016
 *      Author: dragos
 */

#ifndef FUNCTIONARRAY_HPP_
#define FUNCTIONARRAY_HPP_

#include <vector>
#include <string>
#include <set>
#include <deque>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define MAX_COMMAND_SIZE 1024
#define FUNCTION pair<string, void(*)(char *)>

struct Peer
{
	string ip;
	uint16_t port;
	string fileID;
	int requestSocket;

	Peer(string ip, uint16_t port, string fileID = "", int requestSocket = 0)
	{
		this->ip = ip;
		this->port = port;
		this->fileID = fileID;
		this->requestSocket = requestSocket;
	}
};

struct DownloadProcedureParameter
{
	char fileID[16];
	bool type;
	unsigned long long int startOffset, endOffset;

	DownloadProcedureParameter(char fileID[16], bool type, unsigned long long int startOffset = 0, unsigned long long int endOffset = 0)
	{
		strcpy(this->fileID, fileID);
		this->type = type;
		this->startOffset = startOffset;
		this->endOffset = endOffset;
	}
};

struct PeerComparator
{
	int operator()(Peer peer1, Peer peer2)
	{
		unsigned int difference = peer1.ip.compare(peer2.ip);
		if(difference == 0)
			return peer1.port < peer2.port;
		else
			return difference < 0;
	}
};

struct ActiveObject
{
	string fileName;
	string status;
	double percentage;
	string fileID;
	mutable unsigned int downloadCount;

	ActiveObject(string fileName, unsigned int downloadCount = 1, string status = "seeding", double percentage = 0.0, string fileID = "")
	{
		this->fileName = fileName;
		this->status = status;
		this->percentage = percentage;
		this->fileID = fileID;
		this->downloadCount = downloadCount;
	}
};

struct ActiveObjectComparator
{
	int operator()(ActiveObject obj1, ActiveObject obj2)
	{
		int difference = obj1.fileID.compare(obj2.fileID);
		if(difference == 0)
		{
			difference = obj1.fileName.compare(obj2.fileName);
			return difference < 0;
		}
		return difference < 0;
	}
};

struct ActiveObjectThread
{
	string fileID;
	pthread_t thread;
	bool active;
	unsigned long long int startOffset, endOffset;

	ActiveObjectThread(string fileID, pthread_t thread, unsigned long long int startOffset = 0, unsigned long long int endOffset = 0, bool active = true)
	{
		this->fileID = fileID;
		this->thread = thread;
		this->startOffset = startOffset;
		this->endOffset = endOffset;
		this->active = active;
	}
};

class FunctionArray
{
private:
	static int client, servent;
	vector<FUNCTION> function;
	static deque<ActiveObjectThread> activeDownload;
	static set<Peer, PeerComparator> alreadyConnected;
	static deque<DownloadProcedureParameter> unfinishedDownload;
	static set<ActiveObject, ActiveObjectComparator> activeList;
	static short DOWNLOAD, FIND, QUIT, DOWNLOAD_FINISHED;
	static bool continueDownloads;

	static void sendInfoToServer(void * data, unsigned int dataSize);
	static void displayActiveList(char command[MAX_COMMAND_SIZE]);
	static void find(char command[MAX_COMMAND_SIZE]);
	static void download(char fileID[MAX_COMMAND_SIZE]);
	static void pause(char command[MAX_COMMAND_SIZE]);
	static void resume(char command[MAX_COMMAND_SIZE]);
	static void deleteFromActiveList(char command[MAX_COMMAND_SIZE]);
	static void quit(char command[MAX_COMMAND_SIZE]);
	static void readError();
	static void writeError();
	static void quitSignalHandler(int signal);
	static void * downloadFileChunk(void * args);
	static void initFileTransfer(vector<Peer> peer, char fileName[100], unsigned int fileNameSize, unsigned long long int startOffset, unsigned long long int endOffset, char * fileID);
	static void sendFileChunk(int &peer, char fileName[100], unsigned long long int startOffset, unsigned long long int endOffset);
	static void * startDownloadProcedure(void * args);
	static bool downloadFinished(char fileID[16]);
	static void downloadAcknowledgement(char fileID[16]);
	static void deleteFromActiveDownload(ActiveObjectThread obj);
	static ActiveObjectThread * getActiveObjectThread(pthread_t threadID);
	static void saveUnfinishedDownloads();

public:
	FunctionArray();
	virtual ~FunctionArray();
	unsigned int size();
	string getName(unsigned int index);
	int exists(string commandArray, int lowerBound, int upperBound);
	void execute(unsigned int functionIndex, char command[MAX_COMMAND_SIZE]);
	static void setClient(int clientSD);
	static void setServent(int serventSD);
	static void setSignalHandler();
	static int getServent();
	static void * solveDownloadRequest(void * args);
	static void finishDownloads();
	static void resumeUnfinishedDownloads();
};

struct DownloadParameter
{
	int peer;
	sockaddr_in from;

	DownloadParameter(int peer, sockaddr_in from)
	{
		this->peer = peer;
		this->from = from;
	}
};

#endif /* FUNCTIONARRAY_HPP_ */
