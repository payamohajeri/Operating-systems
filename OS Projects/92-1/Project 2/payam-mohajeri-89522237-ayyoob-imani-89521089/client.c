/*
*** OS Spring 2013 *** Priject #2 ***
*** Payam Mohajeri *** 89522237   ***
*** Ayyoob Imani   *** 89521089   *** 
*/

#include "client.h"


int main(int argc, char *argv[]) {
	
	int i, len, rc, status, IS_ALIVE = 1;
	char buffer[BUFFER_SIZE];
	if ( argc != 3) {
		msg = "-------ERROR--------\nUSE :\n./client ClientName Directory\n--------------------\n";
		write(STDERR_FILENO, msg, strlen(msg));
		return 0;
	}
	
	strcpy(MY_NAME, argv[1]);
	strcpy(MY_PATH, argv[2]);
	
	max_sd = STDIN; //TODO
	FD_ZERO(&file_descriptors);
	FD_SET(STDIN, &file_descriptors);
	fd_set working_set;

	do {
		memcpy(&working_set, &file_descriptors, sizeof(file_descriptors));
		msg = "Waiting on select()\n";
		write(STDOUT_FILENO, msg, strlen(msg));
		rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);
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
		for (i = 0; i <= max_sd && IS_ALIVE; ++i) {
			if (FD_ISSET(i, &working_set)) {
				if (i == STDIN) {
					memset(buffer, 0, BUFFER_SIZE);
					IO_ReadLine(buffer);
					status = command(buffer);
					if (status == -1)
						IS_ALIVE = FALSE;
				} else {
					//recving a message from server
					custom_receive(buffer);
					write(STDERR_FILENO, buffer, strlen(buffer));
					custom_send("OK");
				}
			}
		}
	} while (IS_ALIVE);
	if (sockfd > 0)
		close(sockfd);
	return 0;
}


int command(char* line) {
	int index = 0;
	char next[BUFFER_SIZE] = "";
	char send_buf[BUFFER_SIZE] = "";
	char recv_buf[BUFFER_SIZE] = "";
	memset(next, 0, BUFFER_SIZE);
	index = ST_Next(line, next, index);
	int k = 100;
	if (strcmp(next, "@quit") == 0)
		k = -1; // quit
	else if (strcmp(next, "@connect") == 0) {
		k = custom_connect(line);
		if (k != -1) {
			custom_send(MY_NAME);
			int len = custom_receive(recv_buf);
			MY_NUMBER = atoi(recv_buf);
			msg="My Number is :\n";
			write(STDOUT_FILENO, msg, strlen(msg));
			write(STDOUT_FILENO, recv_buf, strlen(recv_buf));
			write(STDOUT_FILENO, "\n", 1);
		}
		k = 0;
	} else if (strcmp(next, "@get-clients-list") == 0) {
		if (sockfd < 0) {
			msg="You Are Not Connected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
			return 100;
		}
		k = custom_send(next);
		if (k >= 0) {
			k = custom_receive(recv_buf);
			if (k >= 0) {
				msg="----------currently available clients-----------\n";
				write(STDOUT_FILENO, msg, strlen(msg));
				write(STDOUT_FILENO, recv_buf, strlen(recv_buf));
				write(STDOUT_FILENO, "\n", 1);
			}
		}
		k = 1;
	} else if (strcmp(next, "@share") == 0) {
		if (sockfd < 0) {
			msg="You Are Not Connected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
			return 100;
		}
		send_file(index, next, line);///
		k = 2; // share
	} else if (strcmp(next, "@get-files-list") == 0) {
		if (sockfd < 0) {
			msg="You Are Not Connected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
			return 100;
		}
		k = custom_send(next);
		if (k >= 0) {
			k = custom_receive(recv_buf);
			if (k >= 0) {
				msg="----------shared files-----------\n";
				write(STDOUT_FILENO, msg, strlen(msg));
				write(STDOUT_FILENO, recv_buf, strlen(recv_buf));
				write(STDOUT_FILENO, "\n", 1);
			}
		}
		k = 3; // share
	} else if (strcmp(next, "@get") == 0) {
		custom_send(line);
		k = custom_receive(recv_buf);
		if (strcmp("Privilege granted", recv_buf) == 0) {
			custom_send("getSize");
			custom_receive(recv_buf);
			int totalSize = atoi(recv_buf);

			char completeName[BUFFER_SIZE];
			strcpy(completeName,MY_PATH);
			char fileName[BUFFER_SIZE];
			index = ST_Next(line, fileName, index);
			strcat(completeName, fileName);
			int myFD;
			int k = creat(completeName, S_IRWXU | O_CREAT);
			if (k >= 0) {
				myFD=k;
			}
			else {
				myFD=open(completeName, O_WRONLY);
			}
			custom_send("getFile");
			k = custom_receive(recv_buf);
			while (totalSize >0) {
				totalSize -= k;
				write(myFD, recv_buf, k);
				custom_send("getFile");
				k = custom_receive(recv_buf);
			}
			close(myFD);
		}
		else{
			write(STDOUT_FILENO, recv_buf, strlen(recv_buf));
		}
		k = 4; // share
	} else if (strcmp(next, "@remove") == 0) {
		if (sockfd < 0) {
			msg="You Are Not Connected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
			return 100;
		}
		custom_send(line);
		custom_receive(recv_buf);
		write(STDOUT_FILENO, recv_buf, strlen(recv_buf));
		k = 5; // remove
	} else if (strcmp(next, "@rename") == 0) {
		if (sockfd < 0) {
			msg="You Are Not Connected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
			return 100;
		}
		custom_send(line);
		custom_receive(recv_buf);
		write(STDOUT_FILENO, recv_buf, strlen(recv_buf));
		k = 6; // rename
	} else if (strcmp(next, "@msg") == 0) {
		if (sockfd < 0) {
			msg="You Are Not Connected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
			return 100;
		}
		custom_send(line);
		custom_receive(recv_buf);
		write(STDOUT_FILENO, recv_buf, strlen(recv_buf));
		k = 7; // Msg
	} else if (strcmp(next, "@dc") == 0) {
		if (sockfd > 0) {
			msg="You Are Now DisConnected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
			close(sockfd);
			sockfd = -1;
			max_sd = 0;
		} else {
			msg="You Are Not Connected !\n";
			write(STDERR_FILENO, msg, strlen(msg));
		}
		k = 8; // share
	}
	return k;
}

int custom_connect(char* line) {
	int locIndex = 0;
	char sIP[BUFFER_SIZE]; //Server IP
	memset(sIP, 0, BUFFER_SIZE);
	locIndex = ST_Next_D(line, sIP, 0, ':');
	char sPORT[BUFFER_SIZE]; //Server PORT
	memset(sPORT, 0, BUFFER_SIZE);
	ST_Next(line, sPORT, locIndex);
	int rc = init(sIP, sPORT);
	return rc;
}

int init(char* SERVER_IP, char* SERVER_PORT) {

	if (sockfd > 0)
		return 1;
	int len, rc;
	struct sockaddr_in addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		msg = "-------ERROR--------\ncannot create socket.\n--------------------\n";
		write( STDERR_FILENO, msg, strlen(msg));
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
	addr.sin_port = htons(atoi(SERVER_PORT));
	rc = connect(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if (rc < 0) {
		msg = "-------ERROR--------\nUnable to connect.\n--------------------\n";
		write( STDERR_FILENO, msg, strlen(msg));
		close(sockfd);
		sockfd = -1;
		return -1;
	}
	// Adds this new connection to file descriptors set
	FD_SET(sockfd, &file_descriptors);

	// Sets the maximum descriptor to this fd, STDIN is 0.
	max_sd = sockfd;
	return 0;
}

int custom_send(char* line) {
	int len = send(sockfd, line, strlen(line) + 1, 0);
	if (len != strlen(line) + 1) {
		msg = "-------ERROR--------\nError is send.\n--------------------\n";
		write( STDERR_FILENO, msg, strlen(msg));
		return -1;
	}
	return 0;
}

int custom_receive (char* output) {
	memset(output, 0, sizeof(char) * BUFFER_SIZE);
	return recv(sockfd, output, sizeof(char) * BUFFER_SIZE, 0);
}

int send_file(int index, char next[BUFFER_SIZE], char* line) {
	//client is going to start for sharing a file
	char send_buf[BUFFER_SIZE] = "share ";
	char recv_buf[BUFFER_SIZE] = "";
	char fileName[BUFFER_SIZE] = "";
	char password[BUFFER_SIZE] = "";

	index = ST_Next(line, next, index);
	int myFD = open(next, O_RDONLY);

	if (myFD == -1) {
		msg = "-------ERROR--------\nThis File Doesn't exist.\n--------------------\n";
		write( STDERR_FILENO, msg, strlen(msg));
		return -1;
	}

	index = ST_Next(line, password, index);

	ST_getFileName(next, fileName);

	strcat(send_buf, fileName);
	int k = custom_send(send_buf);
	if (k >= 0) {
		k = custom_receive(recv_buf);
		if (strcmp("Privilege granted", recv_buf) == 0) {
			char tmpInt[BUFFER_SIZE]="";
			struct stat info;
			stat(fileName,&info);
			convertIntToString(info.st_size,tmpInt);
			custom_send(tmpInt);
			custom_receive(recv_buf);
			if( strcmp(recv_buf,tmpInt) != 0)
			{
				msg = "-------ERROR--------\nSomething is Wrong.\n--------------------\n";
				write( STDERR_FILENO, msg, strlen(msg));
				return 1;
			}
			while (fEndOfFile(myFD) == 0) {
				// Reads from file and sends the buffer
				int len = fReadSome(myFD, send_buf, BUFFER_SIZE - 5);
				do_send_size(send_buf,len);
				k = custom_receive(recv_buf);
				if (k >= 0 && strcmp("OK", recv_buf) != 0) {
					msg = "-------ERROR--------\nServer Intrupted.\n--------------------\n";
					write( STDERR_FILENO, msg, strlen(msg));
					close(myFD);
					return -1;
				}
			}

			k = custom_send(password);
			if (k < 0) {
				msg = "-------ERROR--------\nCan't send password !.\n--------------------\n";
				write( STDERR_FILENO, msg, strlen(msg));
				return -1;
			}

			// Finilize
			k = custom_receive(recv_buf);
			if (strcmp("Complete", recv_buf) != 0) {
				msg = "-------ERROR--------\nProblem with sending password.\n--------------------\n";
				write( STDERR_FILENO, msg, strlen(msg));
				return -1;
			}
		}
	}
}

int do_send_size (char* line, int size) {
	int len = send(sockfd, line, size, 0);
	if (len != size) {
		msg = "-------ERROR--------\nError is Send.\n--------------------\n";
		write( STDERR_FILENO, msg, strlen(msg));
		return -1;
	}
	return 0;
}
