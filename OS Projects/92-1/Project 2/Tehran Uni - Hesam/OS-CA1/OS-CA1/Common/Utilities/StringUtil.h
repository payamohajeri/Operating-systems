
#ifndef __STRINGUTILITIES_H_
#define __STRINGUTILITIES_H_

int nextToken(char* str, char* output, int index);
int nextTokenDelimiter(char* str, char* output, int index, char d);
int indexOf(char* src, char c);
int convertIntToString(int k, char* output);
int getFileName(char* str, char* output);

#endif
