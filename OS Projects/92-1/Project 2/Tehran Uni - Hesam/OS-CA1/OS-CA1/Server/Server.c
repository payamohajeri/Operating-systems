#include "../Common/Consts/Consts.h"
#include "../Common/Utilities/StringUtil.h"
#include "../Common/Objects/DataObjects.h"
#include "../Common/Utilities/FileUtil.h"

//#define SERVER_PORT  12345

char MY_PATH[BUFFER_SIZE];
int SERVER_PORT;

Client clients[CLIENT_SIZE];
int clientCount;
int lock_id = -1;

int init() {
	clientCount = 0;
	int i = 0;
	for (; i < CLIENT_SIZE; i++)
		resetClient(&clients[i]);
	int listen_sd, on = 1;
	struct sockaddr_in addr;
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		writeErr("socket() failed");
		return -1;
	}

	int rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char*) &on,
			sizeof(on));
	if (rc < 0) {
		writeErr("setsockopt() failed");
		close(listen_sd);
		return -1;
	}

	rc = ioctl(listen_sd, FIONBIO, (char *) &on);
	if (rc < 0) {
		writeErr("ioctl() failed");
		close(listen_sd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY );
	addr.sin_port = htons(SERVER_PORT);
	rc = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
	if (rc < 0) {
		println("bind() failed");
		close(listen_sd);
		return -1;
	}

	rc = listen(listen_sd, 5);
	if (rc < 0) {
		writeErr("listen() failed");
		close(listen_sd);
		return -1;
	}
	return listen_sd;
}

int isRCValid(int rc) {
	if (rc < 0)
		writeErr("  select() failed");
	if (rc == 0)
		writeErr("  select() timed out.  End program.");
	return rc;
}

int do_send(char* line, int sockfd) {
	int len = send(sockfd, line, strlen(line) + 1, 0);
	if (len != strlen(line) + 1) {
		writeErr("Error is send");
		return -1;
	}
	return 0;
}
int do_send_size(char* line, int size, int sockfd) {
	int len = send(sockfd, line, size, 0);
	if (len != size) {
		writeErr("Error is send");
		return -1;
	}
	return 0;
}

int do_recieve(char* output, int sockfd) {
	memset(output, 0, sizeof(char) * BUFFER_SIZE);
	return recv(sockfd, output, sizeof(char) * BUFFER_SIZE, 0);
}

void sendClientsList(int num) {
	char message[BUFFER_SIZE] = "";
	int q = 0;
	for (; q < clientCount; q++) {
		if (clients[q].fd != -1) {
			strcat(message, "cli");
			char tmpInt[BUFFER_SIZE] = "";
			convertIntToString(clients[q].id, tmpInt);
			strcat(message, tmpInt);
			strcat(message, ":");
			strcat(message, clients[q].name);
			strcat(message, "\n");
		}
	}
	do_send(message, clients[num].fd);
}

void sendFileList(int num) {
	char message[BUFFER_SIZE] = "";
	int q = 0;
	for (; q < clientCount; q++) {
		if (clients[q].fd != -1) {
			int i = 0;
			for (; i < clients[q].fileCount; i++) {
				if (clients[q].isFileExist[i] == 0)
					continue;

				strcat(message, clients[q].fileNames[i]);
				strcat(message, "\towner:cli#");
				char tmpInt[BUFFER_SIZE] = "";
				convertIntToString(clients[q].id, tmpInt);
				strcat(message, tmpInt);
				strcat(message, ":");
				strcat(message, clients[q].name);
				strcat(message, "\n");
			}
		}
	}
	do_send(message, clients[num].fd);
}

int isThisFileExist(char* fileName) {
	int q = 0;
	for (; q < clientCount; q++) {
		if (clients[q].fd == -1)
			continue;
		int i = 0;
		for (; i < clients[q].fileCount; i++)
			if (clients[q].isFileExist[i]
					&& strcmp(fileName, clients[q].fileNames[i]) == 0)
				return 1;
	}
	return 0;
}

int addThisFile(char* fileName, char* pass, int num) {
	int i = 0;
	for (; i < clients[num].fileCount; i++)
		if (clients[num].isFileExist[i] == 0) {
			clients[num].isFileExist[i] = 1;
			strcpy(clients[num].fileNames[i], fileName);
			strcpy(clients[num].password[i], pass);
			return 0;
		}
	if (clients[num].fileCount >= FILE_SIZE) {
		println("There is no space for this file");
		return -1;
	}
	clients[num].isFileExist[clients[num].fileCount] = 1;
	strcpy(clients[num].password[clients[num].fileCount], pass);
	strcpy(clients[num].fileNames[clients[num].fileCount++], fileName);

	return 0;
}

int checkAuthority(char* fileName, int num) {
	int i = 0;
	for (; i < clients[num].fileCount; i++)
		if (clients[num].isFileExist[i] == 1
				&& strcmp(clients[num].fileNames[i], fileName) == 0)
			return 1;
	return 0;
}

int checkPassword(char* fileName, char* password, int num) {
	int i = 0;
	for (; i < clients[num].fileCount; i++)
		if (clients[num].isFileExist[i] == 1
				&& strcmp(clients[num].fileNames[i], fileName) == 0
				&& strcmp(clients[num].password[i], password) == 0)
			return 1;
	return 0;
}

int removeFileFromClient(char* fileName, int num) {
	int i = 0;
	for (; i < clients[num].fileCount; i++)
		if (clients[num].isFileExist[i] == 1
				&& strcmp(clients[num].fileNames[i], fileName) == 0) {
			clients[num].isFileExist[i] = 0;
			return 1;
		}
	return 0;
}

int renameFileFromClient(char* fileName, char* newFileName, int num) {
	int i = 0;
	for (; i < clients[num].fileCount; i++)
		if (clients[num].isFileExist[i] == 1
				&& strcmp(clients[num].fileNames[i], fileName) == 0) {
			strcpy(clients[num].fileNames[i], newFileName);
			return 1;
		}
	return 0;
}

int getFile(int index, int num, char* line) {
	if (lock_id != -1) {
		println("This operation cannot be done");
		do_send("Prob: server is busy", clients[num].fd);
		return 1;
	}
	char recv_buf[BUFFER_SIZE];
	char completeName[BUFFER_SIZE] = ""; // = "__shared__";
	strcpy(completeName, MY_PATH);
	char fileName[BUFFER_SIZE];
	index = nextToken(line, fileName, index);
	strcat(completeName, fileName);
	if (isThisFileExist(fileName) != 0) {
		println("This file '%s' already exists", fileName);
		do_send("Prob: File exists", clients[num].fd);
		return -1;
	}
	int myFD = getWRFile(completeName);
	if (myFD < 0) {
		println("I can't make this file %s", fileName);
		do_send("Problem:", clients[num].fd);
		return -1;
	}
	clients[num].myFD = myFD;
	lock_id = num;
	clients[num].send = 1;
	do_send("Privilege granted", clients[num].fd);
	do_recieve(recv_buf, clients[num].fd);
	clients[num].totalSize = atoi(recv_buf);
	clients[num].ts2 = atoi(recv_buf);
	clients[num].startTime = time(NULL );
	do_send(recv_buf, clients[num].fd);
	strcpy(clients[num].currentFileName, fileName);
	return 1;
}

int sendFile(int index, int num, char* line) {
	if (lock_id != -1) {
		println("This operation cannot be done");
		do_send("Prob: server is busy", clients[num].fd);
		return 1;
	}
	char recv_buf[BUFFER_SIZE];
	char completeName[BUFFER_SIZE] = ""; // "__shared__";
	strcpy(completeName, MY_PATH);
	char fileName[BUFFER_SIZE];
	index = nextToken(line, fileName, index);
	strcat(completeName, fileName);
	char pass[BUFFER_SIZE];
	index = nextToken(line, pass, index);
	int q = 0;
	for (; q < clientCount; q++) {
		if (clients[q].fd != -1 && checkPassword(fileName, pass, q) != 0) {

			q = -1;
			break;
		}
	}
	if (q != -1) {
		println("This file '%s' doesn't exist or wrong password", fileName);
		do_send("Prob: File doesn't exist or wrong password", clients[num].fd);
		return -1;
	}

	int myFD = getROFile(completeName);
	if (myFD < 0) {
		println("I can't make this file %s", fileName);
		do_send("Problem:", clients[num].fd);
		return -1;
	}
	clients[num].myFD = myFD;
	lock_id = num;
	clients[num].startTime = time(NULL );
	clients[num].send = 0;
	stat(completeName, &clients[num].info);
	writeErr("did you make it?");
	do_send("Privilege granted", clients[num].fd);
	do_recieve(recv_buf, clients[num].fd);
	char tmpInt[BUFFER_SIZE] = "";
	convertIntToString(clients[num].info.st_size, tmpInt);
	do_send(tmpInt, clients[num].fd);
	return 1;
}

void renameFile(int index, char next[BUFFER_SIZE], int num,
		char recv_buf[BUFFER_SIZE], Client clients[CLIENT_SIZE], char* line) {
	index = nextToken(line, next, index);
	if (checkAuthority(next, num) == 1) {
		char fileName[BUFFER_SIZE] = ""; //"__shared__";
		strcpy(fileName, MY_PATH);
		char newFileName[BUFFER_SIZE] = ""; //"__shared__";
		strcpy(newFileName, MY_PATH);
		index = nextToken(line, recv_buf, index);
		strcat(fileName, next);
		strcat(newFileName, recv_buf);
		rename(fileName, newFileName);
		renameFileFromClient(next, recv_buf, num);
		do_send("Rename was successful!", clients[num].fd);
	} else {
		println("Unauthorized access or file not found");
		do_send("Unauthorized access or file not found", clients[num].fd);
	}
}

void removeFile(int index, char next[BUFFER_SIZE], int num,
		Client clients[CLIENT_SIZE], char* line) {
	index = nextToken(line, next, index);
	if (checkAuthority(next, num) == 1) {
		char fileName[BUFFER_SIZE] = ""; //;"__shared__";
		strcpy(fileName, MY_PATH);
		strcat(fileName, next);
		unlink(fileName);
		removeFileFromClient(next, num);
		do_send("Deletion was successful!", clients[num].fd);
	} else {
		println("Unauthorized access or file not found");
		do_send("Unauthorized access or file not found", clients[num].fd);
	}
}

void sendMessage(int num, Client clients[CLIENT_SIZE], int targetNum,
		char recv_buf[BUFFER_SIZE], char* line) {
	char msg[BUFFER_SIZE] = "Cli#";
	char tmpInt2[BUFFER_SIZE];
	convertIntToString(num, tmpInt2);
	strcat(msg, tmpInt2);
	strcat(msg, ":");
	strcat(msg, clients[num].name);
	strcat(msg, " said:\n");
	strcat(msg, line);
	println("cli%d:%s to cli%d:%s:", num, clients[num].name, targetNum,
			clients[targetNum].name);
	println(line);
	do_send(msg, clients[targetNum].fd);
	do_recieve(recv_buf, clients[targetNum].fd);
	if (strcmp(recv_buf, "OK") == 0)
		do_send("Message has been sent.", clients[num].fd);
	else
		do_send("Connection was interrupted. Try again later.",
				clients[num].fd);

//	{
//		println("problem in sending message from %d to %d.", num, targetNum);
//		do_send("problem in sending message.", clients[num].fd);
//	}
}

int do_command(char* line, int msgLen, int num) {
	char send_buf[BUFFER_SIZE] = "";
	char recv_buf[BUFFER_SIZE] = "";
	if (lock_id == num) {
		if (clients[num].send == 1) {
			if (clients[num].totalSize > 0) {
				clients[num].totalSize -= msgLen;
				fWriteStrSize(clients[num].myFD, line, msgLen);
				do_send("OK", clients[num].fd);
			} else {
//				do_send("give me Password", clients[num].fd);
//				do_recieve(recv_buf, clients[num].fd);
				println("Password: %s", line);
				do_send("Complete", clients[num].fd);
				char fileName[BUFFER_SIZE];
				strcpy(fileName, clients[num].currentFileName);
				if (addThisFile(fileName, line, num) != 0) {
					println("There is no space for '%s'", fileName);
					do_send("Prob: no space", clients[num].fd);
					return 2;
				}
				println("Receiving file \"%s\" was completed", fileName);
				lock_id = -1;
				close(clients[num].myFD);
			}
		} else {
			if (fEndOfFile(clients[num].myFD) == 0) {
				int len = fReadSome(clients[num].myFD, send_buf,
				BUFFER_SIZE - 5);
				do_send_size(send_buf, len, clients[num].fd);
			} else {
				println("I sent all the file");
				do_send("Complete", clients[num].fd);
				lock_id = -1;
				int dif = (time(NULL ) - clients[num].startTime);
				if (dif == 0)
					dif = 1;
				println("Average transmit speed: %d (KB/Sec)",
						(clients[num].info.st_size / dif) / 1000);
				close(clients[num].myFD);
			}
		}
		return 1;
	}
	int index = 0;
	char next[BUFFER_SIZE];
	memset(next, 0, BUFFER_SIZE);
	index = nextToken(line, next, index);
	int k = -1;
	println("next token is '%s'", next);
	if (strcmp(next, "quit") == 0)
		k = -1; // quit
	else if (strcmp(next, "get-clients-list") == 0) {
		sendClientsList(num);
		k = 0; // send "get-clients-list"
	} else if (strcmp(next, "share") == 0) {
		getFile(index, num, line);
	} else if (strcmp(next, "get-files-list") == 0) {
		sendFileList(num);
		k = 3; // share
	} else if (strcmp(next, "get") == 0) {
		k = sendFile(index, num, line);
//		k =  4; // share
	} else if (strcmp(next, "remove") == 0) {
		removeFile(index, next, num, clients, line);
		k = 5; // remove
	} else if (strcmp(next, "rename") == 0) {
		renameFile(index, next, num, recv_buf, clients, line);
		k = 6; // rename
	} else if (strcmp(next, "msg") == 0) {
		strcpy(line, line + 8);
		char tmpInt[BUFFER_SIZE] = "";
		index = nextToken(line, tmpInt, 0);
		strcpy(line, line + index);
		int targetNum = atoi(tmpInt);
		if (lock_id != targetNum && targetNum < clientCount
				&& clients[targetNum].fd != -1) {
			sendMessage(num, clients, targetNum, recv_buf, line);
		} else
			do_send("Your target may be busy. Try again later!",clients[num].fd);
		k = 7; // msg
	} else if (strcmp(next, "dc") == 0) {
		int q = 0;
		k = 8; // share
	}
	return k;
}

int getNum(int fd) {
	int q = 0;
	for (; q < clientCount; q++)
		if (clients[q].fd == fd)
			return q;
	return -1;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		writeErr("Error: Use ./server directory PORT");
		return -1;
	}
	strcpy(MY_PATH, argv[1]);
	SERVER_PORT = atoi(argv[2]);
	int i, len, rc;
	int desc_ready, end_server = FALSE;
	int close_conn;
	char buffer[BUFFER_SIZE];
	fd_set master_set, working_set;
	int max_sd, new_sd;

	int listen_sd = init();
	if (listen_sd == -1)
		return -1;
	println("list");
	FD_ZERO(&master_set);
	FD_SET(STDIN, &master_set);
	println("listening");
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);
	do {
		memcpy(&working_set, &master_set, sizeof(master_set));
//		println("waiting for listen...");
		rc = select(max_sd + 1, &working_set, NULL, NULL, NULL );
		if (isRCValid(rc) <= 0)
			break;
		desc_ready = rc;
		for (i = 0; i <= max_sd && desc_ready > 0; ++i) {
			if (FD_ISSET(i, &working_set)) {
				desc_ready--;
				if (i == listen_sd) {
					print("  Listening socket is readable\n");
					//DO_ACCEPT
					/**********************/
					new_sd = accept(listen_sd, NULL, NULL );
					if (new_sd < 0) {
						if (errno != EWOULDBLOCK) {
							writeErr("  accept() failed");
							end_server = TRUE;
						}
						break;
					}
					print("  New incoming connection: %d\n", new_sd);
					rc = recv(new_sd, buffer, sizeof(buffer), 0);
					int q = 0;
					int flag = 0;
					for (; q < clientCount; q++) {
						if (clients[q].fd == -1) {
							flag = 1;
							clients[q].fd = new_sd;
							strcpy(clients[q].name, buffer);
							clients[q].id = q;
							println("cli#%d:%s", q, buffer);
							break;
						}
					}
					if (flag == 0) {
						q = clientCount;
						clients[q].fd = new_sd;
						strcpy(clients[q].name, buffer);
						clients[q].id = q;
						println("cli#%d:%s", q, buffer);
						clientCount++;
					}
					len = convertIntToString(q, buffer);
					rc = send(new_sd, buffer, len + 1, 0);
					println("It has been sent");
					/**********************/
					//END DO_ACCEPT
					FD_SET(new_sd, &master_set);
					if (new_sd > max_sd)
						max_sd = new_sd;

				} else if (i == STDIN) {
					memset(buffer, 0, BUFFER_SIZE);
					readLine(buffer);

					int index = 0;
					char next[BUFFER_SIZE];
					memset(next, 0, BUFFER_SIZE);
					index = nextToken(buffer, next, index);
					if (strcmp("kick", next) == 0) {
						strcpy(buffer, buffer + 9);
						char tmpInt[BUFFER_SIZE] = "";
						index = nextToken(buffer, tmpInt, 0);
						int q = atoi(tmpInt);
						int i = clients[q].fd;
						int j = 0;
						for (; j < clients[q].fileCount; j++) {
							char fileName[BUFFER_SIZE] = "";
							strcpy(fileName, MY_PATH);
							strcat(fileName, clients[q].fileNames[j]);
							unlink(fileName);
						}
						resetClient(&clients[q]);
						close(i);
						FD_CLR(i, &master_set);
						if (i == max_sd) {
							while (FD_ISSET(max_sd, &master_set) == FALSE)
								max_sd -= 1;
						}
					}
				} else {
//					print("  Descriptor %d is readable\n", i);
					close_conn = FALSE;
					rc = recv(i, buffer, sizeof(buffer), 0);
					if (rc < 0)
						writeErr("  recv() failed");

					if (rc == 0) {
						print("  Connection closed\n");

						close_conn = TRUE;
					}

					len = rc;
					do_command(buffer, len, getNum(i));

					if (close_conn) {
						int q = 0;
						int flag = 0;
						for (; q < clientCount; q++) {
							if (clients[q].fd == i) {
								int j = 0;
								for (; j < clients[q].fileCount; j++) {
									char fileName[BUFFER_SIZE] = "";
									strcpy(fileName, MY_PATH);
									strcat(fileName, clients[q].fileNames[j]);
									unlink(fileName);
								}
								resetClient(&clients[q]);
								break;
							}
						}
						close(i);
						FD_CLR(i, &master_set);
						if (i == max_sd) {
							while (FD_ISSET(max_sd, &master_set) == FALSE)
								max_sd -= 1;
						}
					}
				}
			}
		}
	} while (end_server == FALSE);

	for (i = 0; i <= max_sd; ++i) {
		if (FD_ISSET(i, &master_set))
			close(i);
	}
}
