#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
//#include <limits.h>
//#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

int exit_check();
void printbuffer();
int overflowecheck();
void run();
void clear_s();

void systemLauncher();
int execLauncher(char* argv[], int is_blocking);
int parseCommand(char *line, char **argv);
char** splitCmd(char* cmd);
void sig_chld_handler(int snum);
int backgroundProcess[16];
int backgroundProcessNo =0;/* number of background child processes */

int const in = 82;                // a constant for buffer
char s[in];                 // for readig in
int flagwhile = 1;
int const MAX_ARGS = 16;


bool redirect_stdout = false;
bool redirect_stdin = false;
FILE *fp;
int fd;
char filename_stdout[20];
char filename_stdin[20];
char pipecommand;
char * bname, * basec;

int main(int argc, char *argv[])
{

    char* arguments[MAX_ARGS];

    signal(SIGCHLD, sig_chld_handler); //catching sigchld signal, for showing approporiate message when a child process has terminated

    printf("Basic Shell >");
    int counter = 0;
    int is_blocking;
    char** commands;
    while ((flagwhile==1)&&(fgets(s,in,stdin)!=NULL))                /// command to reading from stdin 80 char in S
    {
        commands = splitCmd(s);

        if( overflowecheck() == 1)
        {
            printf("Overflowe!!!");
        }
        else
        {
            counter =0;
            while(commands[counter]!= '\0')
            {
                is_blocking = parseCommand(commands[counter], arguments); // convert command string to an array
                if(arguments[0] !='\0') // check whether the input was an empty string or not
                {
                    if (strcmp(arguments[0], "exit") == 0)
                    {
                        exit(EXIT_SUCCESS);
                    }
                    else if(strcmp(arguments[0], "cd") == 0)
                    {

                        if(chdir(arguments[1]) == -1)
                        {
                            //on error
                            printf("%s\n",strerror(errno));

                        }
                    }
                    else
                    {
							bname = strdup((char *)arguments); 
                            execLauncher((char **)basename(bname), is_blocking);
                    }
                }
                counter++;
            }
        }
        free(commands);// free the memory which allocated for the buffer
        clear_s();
        redirect_stdout = false;
		redirect_stdin = false;
        printf("Basic Shell >");

   }//end while

    return 0;

}// end main

void sig_chld_handler(int snum)
{
    int pid; int status;
    pid = wait(&status);
    int counter=0;
    int is_found = 0;
    while(counter<backgroundProcessNo)
    {
        if(backgroundProcess[counter] == pid)
        {
            printf("\n[%d] %d done\nBasic Shell>", counter + 1, pid);

            is_found = 1;

        }
        else if(is_found==1)
        {
            backgroundProcess[counter - 1] =  backgroundProcess[counter];
        }

        counter++;
    }
    fflush(stdout);

    backgroundProcessNo--;

    signal(SIGCHLD, sig_chld_handler);

}

void clear_s()
{
    char cc[in];
    for (int i = 0; i<in; i++) {

        s[i]=cc[i];
    }

}


void printbuffer()
{
    for (int i = 0 ; i<=strlen(s); i++)
    {               // for printing 80
        fputc(s[i],stdout);
    }
}




int overflowecheck()
{
    int result = 0; //false
    int x =strlen(s);
    int x2 = in-1;
    if (x==x2) {                            // to compare that stak is overflued or notd
        if (s[in-2]!='\n') {
            result = 1; //true
            fflush(stdin);                  // after finding over flow cleaning the stdin
        }
    }

    return result;

}


void systemLauncher()
{
}

int execLauncher(char* argv[], int is_blocking)
{
    pid_t child_pid;
    int child_status;
    child_pid = fork();
//    argv[0] = basename(strdup(argv[0]));
//    printf("command is : %s \n.", basename(strdup(argv[0])));
//    printf("full command is : %s \n.", basename(strdup(*argv)));
    if(child_pid == 0) {
		if(redirect_stdout)
		{
			fd = dup(fileno(stdout));	// Copy of stdout
			fp = fopen(filename_stdout, "w" );	// New Stdout
			dup2(fileno(fp), 1);	// Redirecting
	        /* This is done by the child process. */
	        execvp(argv[0], argv);
	        fflush(stdout);
        	fclose(fp);
        	redirect_stdout = false;
        	memset(&filename_stdout, 0, sizeof(filename_stdout));
        	dup2(fd, 1);
        	close(fd);
        	clearerr(stdout);
        } if(redirect_stdin)
		{
			fd = dup(fileno(stdin));	// Copy of stdout
			fp = fopen(filename_stdin, "r" );	// New Stdout
			dup2(fileno(fp), STDIN_FILENO);	// Redirecting
	        /* This is done by the child process. */
	        execvp(argv[0], argv);
	        fflush(stdin);
        	fclose(fp);
        	redirect_stdin = false;
        	memset(&filename_stdin, 0, sizeof(filename_stdin));
        	dup2(fd, STDIN_FILENO);
        	close(fd);
        	clearerr(stdin);
        } if (pipe)
        {
        	// TODO
        } else  {
            /* This is done by the child process. */
        	execvp(argv[0], argv);
        }
        /* If execvp returns, it must have failed. */
        printf("Unknown command\n");
        exit(0);
    }
    else {
        /* This is run by the parent.  Wait for the child
        to terminate. */
        if(is_blocking == 0)
        {
            printf("[%d] %d\n",backgroundProcessNo +1, child_pid);
            fflush(stdout);
            backgroundProcess[backgroundProcessNo] = child_pid;
            backgroundProcessNo++;
        }

        while (is_blocking == 1 && wait(&child_status) != child_pid )       /* wait for completion  */
                       ;
        return child_status;
    }

}

int parseCommand(char *line, char **argv)
{
    int is_blocking = 1; //true
    if(strlen(line)>0 && line[strlen(line)-1]=='\n')
    {
        line[strlen(line)-1]='\0'; //remove the \n from the end of command
    }

     if( strlen(line)-2 >=0 && line[strlen(line)-1]=='&')
     {
         is_blocking = false;
         line[strlen(line)-1]='\0';
     }

     int comma = 0;
     int end_comma = 0;

     int location = 0;
     int argNo = 0;

     int lenght = strlen(line);
	 int count=0;
     for(int i=0; i<lenght; i++)
     {
     	if(line[i] == '>')
     	{
     		for(int j=i+1; j<lenght; j++)
     		{
     			if(line[j]==' ')
     			{
     				continue;
     			}
     			filename_stdout[count]=line[j];
     			count++;
     		}
     		filename_stdout[lenght]='\0';
     		line[i]='\0';
     		lenght = strlen(line);
     		redirect_stdout = true;
     	}
     	count = 0;
     	if(line[i] == '<')
     	{
     		for(int j=i+1; j<lenght; j++)
     		{
     			if(line[j]==' ')
     			{
     				continue;
     			}
     			filename_stdin[count]=line[j];
     			count++;
     		}
     		filename_stdin[lenght]='\0';
     		line[i]='\0';
     		lenght = strlen(line);
     		redirect_stdin = true;
     	}
     	if(line[i] == '|')
     	{
     		for(int j=i+1; j<lenght; j++)
     		{
     			if(line[j]==' ')
     			{
     				continue;
     			}
     			filename_stdin[count]=line[j];
     			count++;
     		}
     		filename_stdin[lenght]='\0';
     		line[i]='\0';
     		lenght = strlen(line);
     		redirect_stdin = true;
     	}
     }
     
     while (location<lenght) {       /* if not the end of line ....... */
     	
         //if we didn't see start comma before
         if(comma == 0)
         {
            while (line[location]  == ' ' || line[location]  == '\t' || line[location]  == '\n')
            {
               line[location] = '\0';     /* replace white spaces with null    */
               location++;
            }
            //if the current character is " not \" (the current comma is the starting comma
            if(line[location] == '"' && !(location>0 && line[location -1] =='\\'))
            {
                comma = 1; //set
                location++;
                argv[argNo] = &line[location];
                argNo++;

            }
            else if(line[location] != '\0')
            {
                argv[argNo]= &line[location];          /* save the argument position     */
                location++;
                argNo++;
            }


            while (line[location] != '\0' && line[location]  != ' ' && line[location]  != '\t' && line[location]  != '\n' && line[location]  != '"')
            {
               location++;
            }
         }
         else if (comma == 1) // if we saw a starting comma
         {
                          //if the current character is " not \" (so we are seeing an ending comma, a match)
             if(line[location] == '"' && !(location>0 && line[location -1] =='\\'))
             {
                 comma = 0; //reset

                 line[location]  ='\0';
                 location++;

             }
             else
             {
                location++; //go to the next character in the command string
             }
         }
     }
     argv[argNo] = '\0';                 /* mark the end of argument list  */
     return is_blocking;
}

char** splitCmd(char* cmd)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = cmd;
    char* last_comma = 0;
    char delim[2];
    delim[0] = ';';
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (';' == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (cmd + strlen(cmd) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**)malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(cmd, delim);

        while (token)
        {
            //assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        //assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}


void run()
{

    //system("clear");

    char *ar[] = {"cat","-ne",NULL};
    //char *vv="cat";
    execvp(ar[0] , ar);

    //system(s);
}
