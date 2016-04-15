#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

using namespace std;

#define BUFFER_SIZE 1024
#define S_PORT 5555

int main()
{
	/* Variables */
	int sock;
	struct sockaddr_in server;
	int mysock;

	char buff[BUFFER_SIZE];
	int readSize;

	/*Create Socket*/
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		perror("Failed to Create Socket");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(S_PORT);

	/* Bind */
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)))
	{
		perror("Bind Failed");
		exit(1);
	}

	/*Listen*/
	listen(sock, 5);
	cout << "Start Listening..." << endl;

	/*Accept*/
	do
	{
		mysock = accept(sock, (struct sockaddr*) 0, 0);
		if (mysock == -1)
		{
			perror("Accept failed");
		}
		else
		{
			bzero(buff, BUFFER_SIZE);
			//Main Loop: show message received from client w/ length
			while ((readSize = recv(mysock, buff, BUFFER_SIZE, 0)))
			{
				if (readSize < 0)
				{
					perror("Reading stream message error");
				}
				else if (readSize == 0)
					printf("Ending Connection\n");
				else
				{
					printf("Got the Message - Length:%d\n", readSize);
					printf("Content: %s\n", buff);
				}
			}
			close(mysock);
		}
	} while (1);

	return 0;
}
