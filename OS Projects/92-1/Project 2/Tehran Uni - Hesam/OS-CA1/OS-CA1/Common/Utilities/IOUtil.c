#include "IOUtil.h"
#include "FileUtil.h"
#include "../Consts/Consts.h"

int readLine(char* input) {
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

int writeLine(char* tmp) {
	return fWriteStr(STDOUT, tmp);
}

int writeErr(char* tmp) {
	char buf[BUFFER_SIZE];
	strcpy(buf, tmp);
	strcat(buf, "\n");
	return fWriteStr(STDERR, buf);
}

int writeStr(char* tmp) {
	return fWriteStr(STDOUT,tmp);
}

int writeChar(char c) {
	return fWriteChar(STDOUT, c);
}

int writeInt(int d) {
	return fWriteInt(STDOUT, d);
}

int println(char* first, ...) {
	va_list vl;
	va_start(vl,first);
	char* ch = first;
	for(; *ch; ch++) {
		if( *ch !='%' && *ch != '\\') {
			if( writeChar(*ch) < 0)
				return -1;
		}
		else if( *ch == '%') {
			ch++;
			if( *ch == 0)
				return 0;
			if( *ch == 's' || *ch == 'S') {
				char* tmp = va_arg(vl,char*);
				if(writeStr(tmp) < 0)
					return -1;
			}
			else if( *ch == 'd') {
				int d = va_arg(vl,int);
				if( writeInt(d) < 0)
					return -1;
			}
		}
		else {
			ch++;
			if( *ch == 'n') {
				if( writeChar('\n') < 0)
					return -1;
			}
			else if ( *ch == 't') {
				if( writeChar('\t') < 0)
					return -1;
			}
		}
	}
	va_end(vl);
	write(STDOUT, "\n", 1);
	return 0;
}

int print(char* first, ...) {
	va_list vl;
	va_start(vl,first);
	char* ch = first;
	for(; *ch; ch++) {
		if( *ch !='%' && *ch != '\\') {
			if( writeChar(*ch) < 0)
				return -1;
		}
		else if( *ch == '%') {
			ch++;
			if( *ch == 0)
				return 0;
			if( *ch == 's' || *ch == 'S') {
				char* tmp = va_arg(vl,char*);
				if( writeStr(tmp) < 0)
					return -1;
			}
			else if( *ch == 'd') {
				int d = va_arg(vl,int);
				if( writeInt(d) < 0)
					return -1;
			}
		}
		else {
			ch++;
			if( *ch == 'n') {
				if( writeChar('\n') < 0)
					return -1;
			}
			else if ( *ch == 't') {
				if( writeChar('\t') < 0)
					return -1;
			}
		}
	}
	va_end(vl);
	return 0;
}

int readWord(char* word) {
	return fReadWord(STDIN, word);
}

int nextInt() {
	return fReadInt(STDIN);
}

int scan(char* first, ...) {
	register int i;
	va_list vl;
	va_start(vl,first);
	char* ch = first;
	for(; *ch; ch++) {
		if( *ch =='%') {
			ch++;
			if( *ch == 0)
				return 0;
			if( *ch == 's' || *ch == 'S') {
				char buf[BUFFER_SIZE];
				readWord(*va_arg(vl,char**));
			}
			else if( *ch == 'd') {
				*va_arg(vl,int*) = nextInt();
			}
		}
	}
	va_end(vl);
	return 0;
}
