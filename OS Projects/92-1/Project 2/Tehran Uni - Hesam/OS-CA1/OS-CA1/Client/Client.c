#include "../Common/Consts/Consts.h"
#include "../Common/Utilities/IOUtil.h"
#include "../Common/Utilities/FileUtil.h"
#include "../Common/Utilities/StringUtil.h"

int sockfd = -1;
int MY_NUMBER = -1;
char MY_NAME[BUFFER_SIZE] = "";
char MY_PATH[BUFFER_SIZE] = "";
fd_set file_descriptors;
int max_sd;
int init();

int do_connect(char* line) {
	int locIndex = 0;
	char sIP[BUFFER_SIZE];
	memset(sIP, 0, BUFFER_SIZE);
	locIndex = nextTokenDelimiter(line, sIP, 0, ':');
	char sPORT[BUFFER_SIZE];
	memset(sPORT, 0, BUFFER_SIZE);
	nextToken(line, sPORT, locIndex);
	int rc = init(sIP, sPORT);
	return rc;
}

int init(char* SERVER_IP, char* SERVER_PORT) {

	// Allready connected
	if (sockfd > 0)
		return 1;

	int len, rc;
	struct sockaddr_in addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Socket couldn't be created
	if (sockfd < 0) {
		writeErr("Error: Socket cannot be established");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
	addr.sin_port = htons(atoi(SERVER_PORT));

	// Tring to connect
	rc = connect(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

	// If the attempt was unsuccessful
	if (rc < 0) {
		writeErr("Error: Unable to connect");
		close(sockfd);
		sockfd = -1;
		return -1;
	}
	// Adds this new connectiong to file descriptors set
	FD_SET(sockfd, &file_descriptors);

	// Sets the maximum descriptor to this fd, STDIN is 0.
	max_sd = sockfd;
	return 0;
}

// Sends a line of char via sockfd
int do_send(char* line) {
	int len = send(sockfd, line, strlen(line) + 1, 0);
	if (len != strlen(line) + 1) {
		writeErr("Error is send");
		return -1;
	}
	return 0;
}

// Sends a char* with it's size
int do_send_size(char* line, int size) {
	int len = send(sockfd, line, size, 0);
	if (len != size) {
		writeErr("Error is send");
		return -1;
	}
	return 0;
}

// Receives a char* with maximum of BUFFER_SIZE via sockfd
int do_receive(char* output) {
	memset(output, 0, sizeof(char) * BUFFER_SIZE);
	return recv(sockfd, output, sizeof(char) * BUFFER_SIZE, 0);
}

// Sends a file, used in sharing
int send_file(int index, char next[BUFFER_SIZE], char* line) {
	// Our protocol to tell server that client is gonna start to share a file, starts with "share"
	char send_buf[BUFFER_SIZE] = "share ";
	char recv_buf[BUFFER_SIZE] = "";
	char fileName[BUFFER_SIZE] = "";
	char password[BUFFER_SIZE] = "";

	// Reads the file name
	index = nextToken(line, next, index);
	int myFD = getROFile(next);

	// File not found
	if (myFD == -1) {
		println("This file does not exists");
		return -1;
	}

	// Now reads password
	index = nextToken(line, password, index);

	// Removes file's parents from it's name
	getFileName(next, fileName);

	// Concats the filename to end of send buffer
	strcat(send_buf, fileName);
	int k = do_send(send_buf);
	if (k >= 0) {
		k = do_receive(recv_buf);

		// The first line that recieves, is permission status from server
		if (strcmp("Privilege granted", recv_buf) == 0) {
			char tmpInt[BUFFER_SIZE]="";
			struct stat info;
			stat(fileName,&info);
			convertIntToString(info.st_size,tmpInt);
			do_send(tmpInt);
			do_receive(recv_buf);
			if( strcmp(recv_buf,tmpInt) != 0)
			{
				println("Something's wrong");
				return 1;
			}
			while (fEndOfFile(myFD) == 0) {
				// Reads from file and sends the buffer
				int len = fReadSome(myFD, send_buf, BUFFER_SIZE - 5);
				do_send_size(send_buf,len);
				k = do_receive(recv_buf);
				if (k >= 0 && strcmp("OK", recv_buf) != 0) {
					println("interrupt from server");
					close(myFD);
					return -1;
				}
			}
//			k = do_send("BAZINGA");
//			if (k < 0) {
//				println("I Can't send BAZINGA");
//				return -1;
//			}
//
//			// Checks if server requested the password or not
//			k = do_receive(recv_buf);
//			if (strcmp("give me Password", recv_buf) != 0) {
//				println("problem with finalize");
//				return -1;
//			}

			k = do_send(password);
			if (k < 0) {
				println("I Can't send Password");
				return -1;
			}

			// Finilize
			k = do_receive(recv_buf);
			if (strcmp("Complete", recv_buf) != 0) {
				println("problem with sending password");
				return -1;
			}
		}
	}
}

// Runs commands, depending on them
int do_command(char* line) {
	int index = 0;
	char next[BUFFER_SIZE] = "";
	char send_buf[BUFFER_SIZE] = "";
	char recv_buf[BUFFER_SIZE] = "";
	memset(next, 0, BUFFER_SIZE);
	index = nextToken(line, next, index);
	int k = 100;
	if (strcmp(next, "quit") == 0)
		k = -1; // quit
	else if (strcmp(next, "connect") == 0) {
		k = do_connect(line);

		if (k != -1) {
			do_send(MY_NAME);
			int len = do_receive(recv_buf);
			MY_NUMBER = atoi(recv_buf);
			println("My Number is '%d'", MY_NUMBER);
		}
		k = 0;
	} else if (strcmp(next, "get-clients-list") == 0) {
		if (sockfd < 0) {
			println("You are not connected.");
			return 100;
		}
		k = do_send(next);
		if (k >= 0) {
			k = do_receive(recv_buf);
			if (k >= 0) {
				println("----------currently available clients-----------");
				println("%s", recv_buf);
			}
		}
		k = 1;
	} else if (strcmp(next, "share") == 0) {
		if (sockfd < 0) {
			println("You are not connected.");
			return 100;
		}
		send_file(index, next, line);
		k = 2; // share
	} else if (strcmp(next, "get-files-list") == 0) {
		if (sockfd < 0) {
			println("You are not connected.");
			return 100;
		}
		k = do_send(next);
		if (k >= 0) {
			k = do_receive(recv_buf);
			if (k >= 0) {
				println("----------shared files-----------");
				println("%s", recv_buf);
			}
		}
		k = 3; // share
	} else if (strcmp(next, "get") == 0) {
		do_send(line);
		k = do_receive(recv_buf);
		if (strcmp("Privilege granted", recv_buf) == 0) {
			do_send("getSize");
			do_receive(recv_buf);
			int totalSize = atoi(recv_buf);

			char completeName[BUFFER_SIZE];
			strcpy(completeName,MY_PATH);
			char fileName[BUFFER_SIZE];
			index = nextToken(line, fileName, index);
			strcat(completeName, fileName);
			int myFD = getWRFile(completeName);
			do_send("getFile");
			k = do_receive(recv_buf);
			while (totalSize >0) {
				totalSize -= k;
				fWriteStrSize(myFD, recv_buf, k);
				do_send("getFile");
				k = do_receive(recv_buf);
			}
			close(myFD);
		}
		else{
			println(recv_buf);
		}
		k = 4; // share
	} else if (strcmp(next, "remove") == 0) {
		if (sockfd < 0) {
			println("You are not connected.");
			return 100;
		}
		do_send(line);
		do_receive(recv_buf);
		println(recv_buf);
		k = 5; // remove
	} else if (strcmp(next, "rename") == 0) {
		if (sockfd < 0) {
			println("You are not connected.");
			return 100;
		}
		do_send(line);
		do_receive(recv_buf);
		println(recv_buf);
		k = 6; // rename
	} else if (strcmp(next, "msg") == 0) {
		if (sockfd < 0) {
			println("You are not connected.");
			return 100;
		}
		do_send(line);
		do_receive(recv_buf);
		println(recv_buf);
		k = 7; // Msg
	} else if (strcmp(next, "dc") == 0) {
		if (sockfd > 0) {
			println("You are now disconnected.");
			close(sockfd);
			sockfd = -1;
			max_sd = 0;
		} else {
			println("You are not connected.");
		}
		k = 8; // share
	}
	return k;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		writeErr("Error: Use ./client clientName directory");
		return -1;
	}

	strcpy(MY_NAME, argv[1]);
	strcpy(MY_PATH, argv[2]);

	int len, rc, status, IS_ALIVE = 1;
	int i;
	char buffer[BUFFER_SIZE];

	max_sd = STDIN;
	FD_ZERO(&file_descriptors);
	FD_SET(STDIN, &file_descriptors);
	fd_set working_set;

	do {
		memcpy(&working_set, &file_descriptors, sizeof(file_descriptors));
		println("Waiting on select()");

		rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);

		if (rc < 0) {
			writeErr("  select() failed");
			break;
		}

		if (rc == 0) {
			writeErr("  select() timed out.  End program.");
			break;
		}

		for (i = 0; i <= max_sd && IS_ALIVE; ++i) {
			if (FD_ISSET(i, &working_set)) {
				if (i == STDIN) {
					memset(buffer, 0, BUFFER_SIZE);
					readLine(buffer);
					status = do_command(buffer);
					if (status == -1)
						IS_ALIVE = FALSE;
				} else {
					// Socket is hot, should now recv a message from server
					do_receive(buffer);
					println("%s", buffer);
					do_send("OK");
				}
			}
		}
	} while (IS_ALIVE);
	if (sockfd > 0)
		close(sockfd);
	return 0;
}
