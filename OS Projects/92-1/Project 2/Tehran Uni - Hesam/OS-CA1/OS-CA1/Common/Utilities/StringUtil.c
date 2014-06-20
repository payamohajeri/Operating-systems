#include "FileUtil.h"
#include "../Consts/Consts.h"

// String tokenizer
int nextToken(char* str, char* ans, int index) {
	memset(ans, 0, BUFFER_SIZE);
	for(; str[index] && str[index] != 10 && str[index] != ' ' && str[index] != '\t' && str[index] != 13; index++)
	{
		strncat(ans, &(str[index]), 1);
	}
	return index + 1;
}

// Tokenizer with custom delimiter
int nextTokenDelimiter(char* str, char* output, int index, char d) {
	strcpy(output, "");
	for(; str[index] && str[index] != d; index++) {
		strncat(output, &(str[index]), 1);
	}
	return index + 1;
}

// returns index of a char in a string
int indexOf(char* src, char c) {
	int len = strlen(src);
	int i = 0;
	for (; i < len; i++) {
		if (src[i] == c)
			return i;
	}
	return -1;
}

// itoa
int convertIntToString(int d, char* output)
{
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

// Removes the parent dirs
int getFileName(char* str, char* output) {
	int size = strlen(str);
	for(; size>=0; size--)
		if( str[size] == '/')
			break;
	strcpy(output, str + size+1);
}

