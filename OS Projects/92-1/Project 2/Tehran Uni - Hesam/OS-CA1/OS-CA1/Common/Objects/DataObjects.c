#include"DataObjects.h"

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
