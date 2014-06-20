/*
*** OS Spring 2013 *** Priject #2 ***
*** Payam Mohajeri *** 89522237   ***
***
*/

#include "server.h"

int main(int argc, char *argv[]) {

	if (argc != 3) {
		msg = "-------ERROR--------\nUSE :\n./server Directory Port\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
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
		
	FD_ZERO(&master_set);
	FD_SET(STDIN, &master_set);
	msg = "Listening on given port.\n";
	write( STDOUT_FILENO, msg, strlen(msg));
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);
	do {
		memcpy(&working_set, &master_set, sizeof(master_set));
		rc = select(max_sd + 1, &working_set, NULL, NULL, NULL );
		if (rc < 0) {
			msg="-------ERROR--------\nselect() FAILED !!!\n";
			write(STDERR_FILENO, msg, strlen(msg));
			break;
		}
		if (rc == 0) {
			msg="-------ERROR--------\nselect() TIMED OUT.\nclosing the program !! :(\n";
			write(STDERR_FILENO, msg, strlen(msg));
			break;
		}
		desc_ready = rc;
		for (i = 0; i <= max_sd && desc_ready > 0; ++i) {
			if (FD_ISSET(i, &working_set)) {
				desc_ready--;
				if (i == listen_sd) {
					msg = "--------\nListening socket is readable\n---------\nwaiting on given port.\n";
					write( STDOUT_FILENO, msg, strlen(msg));
					//DO_ACCEPT
					/**********************/
					new_sd = accept(listen_sd, NULL, NULL );
					if (new_sd < 0) {
						if (errno != EWOULDBLOCK) {
							msg="-------ERROR--------\naccept() failed.\nclosing the program !! :(\n";
							write(STDERR_FILENO, msg, strlen(msg));
							end_server = TRUE;
						}
						break;
					}
					msg = "New incoming connection:  ";
					write( STDOUT_FILENO, msg, strlen(msg));
					convertIntToString(new_sd, temp );
					write( STDOUT_FILENO, temp, strlen(temp));
					write( STDOUT_FILENO, "\n", 1);
					rc = recv(new_sd, buffer, sizeof(buffer), 0);
					int q = 0;
					int flag = 0;
					for (; q < clientCount; q++) {
						if (clients[q].fd == -1) {
							flag = 1;
							clients[q].fd = new_sd;
							strcpy(clients[q].name, buffer);
							clients[q].id = q;
							msg = "cli#";
							write( STDOUT_FILENO, msg, strlen(msg));
							convertIntToString(q, temp);
							write( STDOUT_FILENO, temp, strlen(temp));
							write( STDOUT_FILENO, buffer, strlen(buffer));
							write( STDOUT_FILENO, "\n", 1);
							break;
						}
					}
					if (flag == 0) {
						q = clientCount;
						clients[q].fd = new_sd;
						strcpy(clients[q].name, buffer);
						clients[q].id = q;
						msg = "cli#";
						write( STDOUT_FILENO, msg, strlen(msg));
						convertIntToString(q, temp);
						write( STDOUT_FILENO, temp, strlen(temp));
						write( STDOUT_FILENO, buffer, strlen(buffer));
						write( STDOUT_FILENO, "\n", 1);
						clientCount++;
					}
					len = convertIntToString(q, buffer);
					rc = send(new_sd, buffer, len + 1, 0);
					msg = "It has been sent\n";
					write( STDOUT_FILENO, msg, strlen(msg));
					/**********************/
					//END DO_ACCEPT
					FD_SET(new_sd, &master_set);
					if (new_sd > max_sd)
						max_sd = new_sd;
				} else if (i == STDIN) {
					memset(buffer, 0, BUFFER_SIZE);
					IO_ReadLine(buffer);

					int index = 0;
					char next[BUFFER_SIZE];
					memset(next, 0, BUFFER_SIZE);
					index = ST_Next(buffer, next, index);
					if (strcmp("@kick", next) == 0) {
						strcpy(buffer, buffer + 9);
						char tmpInt[BUFFER_SIZE] = "";
						index = ST_Next(buffer, tmpInt, 0);
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
					close_conn = FALSE;
					rc = recv(i, buffer, sizeof(buffer), 0);
					if (rc < 0){
						msg="-------ERROR--------\nrecv() failed\n";
						write(STDERR_FILENO, msg, strlen(msg));
					}
					if (rc == 0) {
						msg="Connection closed\n";
						write(STDOUT_FILENO, msg, strlen(msg));
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

int init() {
	clientCount = 0;
	int i = 0;
	for (; i < CLIENT_SIZE; i++)
		resetClient(&clients[i]);
	int listen_sd, on = 1;
	struct sockaddr_in addr;
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0) {
		msg = "-------ERROR--------\nsocket() FAILED.\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		return -1;
	}
	int rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
	if (rc < 0) {
		msg = "-------ERROR--------\nsetsockopt() failed\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		close(listen_sd);
		return -1;
	}
	rc = ioctl(listen_sd, FIONBIO, (char *) &on);
	if (rc < 0) {
		msg = "-------ERROR--------\nioctl() failed\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		close(listen_sd);
		return -1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY );
	addr.sin_port = htons(SERVER_PORT);
	rc = bind(listen_sd, (struct sockaddr *) &addr, sizeof(addr));
	if (rc < 0) {
		msg = "-------ERROR--------\nbind() failed\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		close(listen_sd);
		return -1;
	}
	rc = listen(listen_sd, 5);
	if (rc < 0) {
		msg = "-------ERROR--------\nlisten() failed\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		close(listen_sd);
		return -1;
	}
	return listen_sd;
}

int custom_send(char* line, int sockfd) {
	int len = send(sockfd, line, strlen(line) + 1, 0);
	if (len != strlen(line) + 1) {
		msg = "-------ERROR--------\nError is sent.\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		return -1;
	}
	return 0;
}
int custom_send_size(char* line, int size, int sockfd) {
	int len = send(sockfd, line, size, 0);
	if (len != size) {
		msg = "-------ERROR--------\nError is Sent.\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		return -1;
	}
	return 0;
}

int custom_receive(char* output, int sockfd) {
	memset(output, 0, sizeof(char) * BUFFER_SIZE);
	return recv(sockfd, output, sizeof(char) * BUFFER_SIZE, 0);
}

///////////////////////////////


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
	custom_send(message, clients[num].fd);
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
	custom_send(message, clients[num].fd);
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
		//println("There is no space for this file"); TODO
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
		//println("This operation cannot be done"); TODO
		custom_send("Prob: server is busy", clients[num].fd);
		return 1;
	}
	char recv_buf[BUFFER_SIZE];
	char completeName[BUFFER_SIZE] = ""; // = "__shared__";
	strcpy(completeName, MY_PATH);
	char fileName[BUFFER_SIZE];
	index = ST_Next(line, fileName, index);
	strcat(completeName, fileName);
	if (isThisFileExist(fileName) != 0) {
		//println("This file '%s' already exists", fileName); TODO
		custom_send("Prob: File exists", clients[num].fd);
		return -1;
	}
	int myFD;
	int k = creat(completeName, S_IRWXU | O_CREAT);
	if (k >= 0){
		myFD = k;
	}
	else {
		myFD = open(completeName, O_WRONLY);
	}
	if (myFD < 0) {
		//println("I can't make this file %s", fileName); TODO
		custom_send("Problem:", clients[num].fd);
		return -1;
	}
	clients[num].myFD = myFD;
	lock_id = num;
	clients[num].send = 1;
	custom_send("Privilege granted", clients[num].fd);
	custom_receive(recv_buf, clients[num].fd);
	clients[num].totalSize = atoi(recv_buf);
	clients[num].ts2 = atoi(recv_buf);
	clients[num].startTime = time(NULL );
	custom_send(recv_buf, clients[num].fd);
	strcpy(clients[num].currentFileName, fileName);
	return 1;
}

int sendFile(int index, int num, char* line) {
	if (lock_id != -1) {
		//println("This operation cannot be done"); TODO
		custom_send("Prob: server is busy", clients[num].fd);
		return 1;
	}
	char recv_buf[BUFFER_SIZE];
	char completeName[BUFFER_SIZE] = ""; // "__shared__";
	strcpy(completeName, MY_PATH);
	char fileName[BUFFER_SIZE];
	index = ST_Next(line, fileName, index);
	strcat(completeName, fileName);
	char pass[BUFFER_SIZE];
	index = ST_Next(line, pass, index);
	int q = 0;
	for (; q < clientCount; q++) {
		if (clients[q].fd != -1 && checkPassword(fileName, pass, q) != 0) {

			q = -1;
			break;
		}
	}
	if (q != -1) {
		//println("This file '%s' doesn't exist or wrong password", fileName); TODO
		custom_send("Prob: File doesn't exist or wrong password", clients[num].fd);
		return -1;
	}

	int myFD = open(completeName, O_RDONLY);
	if (myFD < 0) {
		//println("I can't make this file %s", fileName); TODO
		custom_send("Problem:", clients[num].fd);
		return -1;
	}
	clients[num].myFD = myFD;
	lock_id = num;
	clients[num].startTime = time(NULL );
	clients[num].send = 0;
	stat(completeName, &clients[num].info);
	msg="did you make it?";
	write(STDERR_FILENO, msg, strlen(msg));
	custom_send("Privilege granted", clients[num].fd);
	custom_receive(recv_buf, clients[num].fd);
	char tmpInt[BUFFER_SIZE] = "";
	convertIntToString(clients[num].info.st_size, tmpInt);
	custom_send(tmpInt, clients[num].fd);
	return 1;
}

void renameFile(int index, char next[BUFFER_SIZE], int num,
		char recv_buf[BUFFER_SIZE], Client clients[CLIENT_SIZE], char* line) {
	index = ST_Next(line, next, index);
	if (checkAuthority(next, num) == 1) {
		char fileName[BUFFER_SIZE] = ""; //"__shared__";
		strcpy(fileName, MY_PATH);
		char newFileName[BUFFER_SIZE] = ""; //"__shared__";
		strcpy(newFileName, MY_PATH);
		index = ST_Next(line, recv_buf, index);
		strcat(fileName, next);
		strcat(newFileName, recv_buf);
		rename(fileName, newFileName);
		renameFileFromClient(next, recv_buf, num);
		custom_send("Rename was successful!", clients[num].fd);
	} else {
		msg="Unauthorized access or file not found";
		write(STDOUT_FILENO, msg, strlen(msg));
		custom_send("Unauthorized access or file not found", clients[num].fd);
	}
}

void removeFile(int index, char next[BUFFER_SIZE], int num,
		Client clients[CLIENT_SIZE], char* line) {
	index = ST_Next(line, next, index);
	if (checkAuthority(next, num) == 1) {
		char fileName[BUFFER_SIZE] = ""; //;"__shared__";
		strcpy(fileName, MY_PATH);
		strcat(fileName, next);
		unlink(fileName);
		removeFileFromClient(next, num);
		custom_send("Deletion was successful!", clients[num].fd);
	} else {
		msg="Unauthorized access or file not found";
		write(STDOUT_FILENO, msg, strlen(msg));
		custom_send("Unauthorized access or file not found", clients[num].fd);
	}
}

void sendMessage(int num, Client clients[CLIENT_SIZE], int targetNum,
		char recv_buf[BUFFER_SIZE], char* line) {
	char Smsg[BUFFER_SIZE] = "Cli#";
	char tmpInt2[BUFFER_SIZE];
	char tmpInt3[BUFFER_SIZE];
	convertIntToString(num, tmpInt2);
	convertIntToString(targetNum, tmpInt3);
	strcat(Smsg, tmpInt2);
	strcat(Smsg, ":");
	strcat(Smsg, clients[num].name);
	strcat(Smsg, " said:\n");
	strcat(Smsg, line);
	msg = "cli#";
	write(STDOUT_FILENO, msg, strlen(msg));
	write(STDOUT_FILENO, tmpInt2, strlen(tmpInt2));
	write(STDOUT_FILENO, ":", 1);
	write(STDOUT_FILENO, clients[num].name, strlen(clients[num].name));
	msg = " to cli#";
	write(STDOUT_FILENO, msg, strlen(msg));
	write(STDOUT_FILENO, tmpInt3, strlen(tmpInt2));
	write(STDOUT_FILENO, ":", 1);
	write(STDOUT_FILENO, clients[targetNum].name, strlen(clients[targetNum].name));
	write(STDOUT_FILENO, line, strlen(line));
	custom_send(Smsg, clients[targetNum].fd);
	custom_receive(recv_buf, clients[targetNum].fd);
	if (strcmp(recv_buf, "OK") == 0)
		custom_send("Message has been sent.", clients[num].fd);
	else
		custom_send("Connection was interrupted. Try again later.",
				clients[num].fd);
}

int do_command(char* line, int msgLen, int num) {
	char send_buf[BUFFER_SIZE] = "";
	char recv_buf[BUFFER_SIZE] = "";
	if (lock_id == num) {
		if (clients[num].send == 1) {
			if (clients[num].totalSize > 0) {
				clients[num].totalSize -= msgLen;
				write(clients[num].myFD, line, msgLen);
				custom_send("OK", clients[num].fd);
			} else {
				msg = "Password: ";
				write(STDOUT_FILENO, msg, strlen(msg));
				write(STDOUT_FILENO, line, strlen(line));
				write(STDOUT_FILENO, "\n", 1);
				custom_send("Complete", clients[num].fd);
				char fileName[BUFFER_SIZE];
				strcpy(fileName, clients[num].currentFileName);
				if (addThisFile(fileName, line, num) != 0) {
					msg = "There is no space for ";
					write(STDOUT_FILENO, msg, strlen(msg));
					write(STDOUT_FILENO, fileName, strlen(fileName));
					write(STDOUT_FILENO, "\n", 1);
					custom_send("Prob: no space", clients[num].fd);
					return 2;
				}
				msg="Receiving file ";
				write(STDOUT_FILENO, msg, strlen(msg));
				write(STDOUT_FILENO, fileName, strlen(fileName));
				msg="was completed.\n";
				write(STDOUT_FILENO, msg, strlen(msg));
				lock_id = -1;
				close(clients[num].myFD);
			}
		} else {
			if (fEndOfFile(clients[num].myFD) == 0) {
				int len = fReadSome(clients[num].myFD, send_buf,
				BUFFER_SIZE - 5);
				custom_send_size(send_buf, len, clients[num].fd);
			} else {
				msg="I sent all the file\n";
				write(STDOUT_FILENO, msg, strlen(msg));
				custom_send("Complete", clients[num].fd);
				lock_id = -1;
				int dif = (time(NULL ) - clients[num].startTime);
				if (dif == 0)
					dif = 1;
//				write("Average transmit speed: %d (KB/Sec)",
//						(clients[num].info.st_size / dif) / 1000);
				close(clients[num].myFD);
			}
		}
		return 1;
	}
	int index = 0;
	char next[BUFFER_SIZE];
	memset(next, 0, BUFFER_SIZE);
	index = ST_Next(line, next, index);
	int k = -1;
	msg="next token is ";
	write(STDOUT_FILENO, msg, strlen(msg));
	write(STDOUT_FILENO, next, strlen(next));
	write(STDOUT_FILENO, "\n", 1);
	if (strcmp(next, "@quit") == 0)
		k = -1; // quit
	else if (strcmp(next, "@get-clients-list") == 0) {
		sendClientsList(num);
		k = 0; // send "get-clients-list"
	} else if (strcmp(next, "@share") == 0) {
		getFile(index, num, line);
	} else if (strcmp(next, "@get-files-list") == 0) {
		sendFileList(num);
		k = 3; // share
	} else if (strcmp(next, "@get") == 0) {
		k = sendFile(index, num, line);
//		k =  4; // share
	} else if (strcmp(next, "@remove") == 0) {
		removeFile(index, next, num, clients, line);
		k = 5; // remove
	} else if (strcmp(next, "@rename") == 0) {
		renameFile(index, next, num, recv_buf, clients, line);
		k = 6; // rename
	} else if (strcmp(next, "@msg") == 0) {
		strcpy(line, line + 8);
		char tmpInt[BUFFER_SIZE] = "";
		index = ST_Next(line, tmpInt, 0);
		strcpy(line, line + index);
		int targetNum = atoi(tmpInt);
		if (lock_id != targetNum && targetNum < clientCount
				&& clients[targetNum].fd != -1) {
			sendMessage(num, clients, targetNum, recv_buf, line);
		} else
			custom_send("Your target may be busy. Try again later!",clients[num].fd);
		k = 7; // msg
	} else if (strcmp(next, "@dc") == 0) {
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

