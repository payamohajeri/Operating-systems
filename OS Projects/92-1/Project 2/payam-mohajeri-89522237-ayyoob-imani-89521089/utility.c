/*
*** OS Spring 2013 *** Priject #2 ***
*** Payam Mohajeri *** 89522237   ***
*** Ayyoob Imani   *** 89521089   *** 
*/
#include "utility.h"


int IO_ReadLine(char* input) {
	char buf = 0;
	int size = 0;
	strcpy(input, "");

	int rc = read(STDIN, &buf, 1);
	if (rc < 0)
		return rc;

	while (buf != 13 && buf != 10) {
		input[size++] = buf;
		rc = read(STDIN, &buf, 1);
		if (rc < 0)
			return rc;
	}
	input[size] = 0;
	return size == strlen(input) ? size : -1;
}

//ASCII 10 : New Line Feed , 13 : Carrige Return
int ST_Next(char* str, char* ans, int index) {
	memset(ans, 0, BUFFER_SIZE);
	for(; str[index] && str[index] != 10 && str[index] != ' ' && str[index] != '\t' && str[index] != 13; index++)
	{
		strncat(ans, &(str[index]), 1);
	}
	return index + 1;
}

int ST_Next_D(char* str, char* output, int index, char d) {
	strcpy(output, "");
	for(; str[index] && str[index] != d; index++) {
		strncat(output, &(str[index]), 1);
	}
	return index + 1;
}

int ST_getFileName(char* str, char* output) {
	int size = strlen(str);
	for(; size>=0; size--)
		if( str[size] == '/')
			break;
	strcpy(output, str + size+1);
}

// itoa
int convertIntToString(int d, char* output) {
		//TODO negative doesn't work
		int i = 0;
		output[1] = 0;
		if (d == 0) {
			output[i++] = '0';
			output[i] = 0;
			return i;
		}
		if (d < 0) {
			d *= -1;
		}
		while (d > 0) {
			output[i++] = (d % 10) + '0';
			d /= 10;
		}
		output[i] = 0;
		i--;
		int j = 0;
		for (; j * 2 < i; j++) {
			char c = output[j];
			output[j] = output[i - j];
			output[i - j] = c;
		}
		return i;
}

// Checks if EOF or not
int fEndOfFile(int fd) {
	char buf;
	if (read(fd, &buf, 1) != 1)
		return 1;
	lseek(fd, -1, SEEK_CUR);
	return 0;
}

int fReadSome(int fd, char* str, int count) {
	char buf;
	int size = 0;
	strcpy(str, "");
	if (read(fd, &buf, 1) != 1)
		return -1;
	str[size++] = buf;
	while (size < count && fEndOfFile(fd) == 0) {
		if (read(fd, &buf, 1) != 1)
			break;
		str[size++] = buf;
	}
	str[size] = 0;
	return size <= count ? size : -1;
}

void resetClient( Client* cli)
{
	(*cli).fileCount=0;
	(*cli).name[0] = 0;
	(*cli).currentFileName[0] = 0;
	(*cli).id = -1;
	(*cli).fd = -1;
	(*cli).myFD = -1;
	(*cli).send = 0;

	int q =0;
	for(; q < FILE_SIZE; q++)
	{
		(*cli).isFileExist[q] = 0;

	}
}

