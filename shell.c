#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include "tail.c"
#include "cd.c"

int fd;
int checkOutputRedirection, checkInputRedirection;
char prompt[512], cwd[1024];
char *argv[512], *cmd_exec[512];
char *buff;
char *outputRedirectionFile, *inputRedirectionFile;
pid_t pid;

int executeCommands(char *,int, int, int);

// This function is used to handle the CTRL+C signal, and to prevent closing the terminal
void handleCtrlC(){

	printf("\nCannot close the terminal with CTRL+C\n");
	//fflush(stdout);	
}

void initializeShell(){

//clear the variables
	fd=0;
	cwd[0]='\0';
	prompt[0]='\0';
 	pid=0;
	checkOutputRedirection=0;
	checkInputRedirection=0;

//get current working directory, with getcwd that copies an absolute pathname of the current working directory to the cwd array, with length of cwd array and set the terminal 
	getcwd(cwd,sizeof(cwd));
	if(cwd!='\0')
	{
		strcpy(prompt,"");
		strcat(prompt,cwd);
		strcat(prompt,"$ ");
	}
//perror is used because it writes a message with the last error encountered during a call to a system or library function
	else perror("Error: ");
	return;
}

void help(){

	printf("\n***This is the help section***\n"
		"\nThe following commands and features are supported:"
		"\n-tail with the following parameters: -n, -q, -v"
		"\n-cd"
		"\n-pipe handling"
		"\n-output and input redirection"
		"\n-improper space handling\n\n");
	return;
}

void version()
{
	printf("\n***This is the version section***\n"
		"\nThe author of this shell project is Adrian-Ioan Tuns\n"
		"\nHere are some useful information about your system: \n");
		system("lsb_release -a");
	printf("\n");

	return;
}

// This function tokenizes the string by every space found, into the argv, leaving the last argv[m] with a NULL element
void tokenizeSpace(char *str){
	
	int n=1;
	argv[0]=strtok(str," ");
	while((argv[n]=strtok(NULL," "))!=NULL)n++;

	return;
}

// This function tokenizes the input buffer by every | found, into the cmd exec array, the last element being a NULL element
void tokenizePipe(){

	int i,n=1,input=0,first=1;
	
	cmd_exec[0]=strtok(buff,"|");
	while((cmd_exec[n]=strtok(NULL,"|"))!=NULL)n++;

// If there was at least one pipe occurence, then it will enter the for loop, that will afterwards call the first part of command execution
	for(i=0;i<n-1;i++){
		input=executeCommands(cmd_exec[i],input,first,0);
		first=0;
	}
// If n=1, which means that no pipe was found or if the for loop has finished, which means that all but the last command was executed, it will enter this code part, which calls with the last parameter set on 1
	input=executeCommands(cmd_exec[i],input,first,1);

	return;
}

// This function is used to skip the whites from str input
char *removeWhiteSpaces(char* str){

	int index,i;
	
	index=0;
// Find last index of whitespace character 
	while(str[index]==' ')index++;
// Shift all trailing characters to its left 
	if(index!=0)
	{
		i=0;
		while(str[i+index]!='\0')
		{
			str[i]=str[i+index];
			i++;
		}
		str[i]='\0';
	}
	return str;
}
   
// This function is used to parse the input when the output redirection sign '>' was found 
void tokenizeOutputRedirection (char *cmd_exec) {

	char *val[128];
	char *cmd_exec2,*s1;
	cmd_exec2=strdup(cmd_exec);

	int n=1;
	val[0]=strtok(cmd_exec2,">");
	while((val[n]=strtok(NULL,">"))!=NULL)n++;
	s1=strdup(val[1]);
	
	outputRedirectionFile=removeWhiteSpaces(s1);
	tokenizeSpace(val[0]);

	return;
}

// This function is used to parse the input when the input redirection sign '<' was found 
void tokenizeInputRedirection (char *cmd_exec) {

	char *val[128];
	char *cmd_exec2,*s1;
	cmd_exec2=strdup(cmd_exec);

	int n= 1;
	val[0]=strtok(cmd_exec2,"<");
	while((val[n]=strtok(NULL,"<"))!=NULL)n++;

	s1=strdup(val[1]);

	inputRedirectionFile=removeWhiteSpaces(s1);
	tokenizeSpace(val[0]);

	return;
}

// This function is used to parse the input when both input and output redirection signs '<', '>' were found 
void tokenizeInputOutputRedirection(char *cmd_exec){

	char *val[128];
	char *cmd_exec2,*s1,*s2;
	cmd_exec2=strdup(cmd_exec);

	int n=1;
	val[0]=strtok(cmd_exec2,"<");
	while((val[n]=strtok(NULL,">"))!=NULL)n++;

	s1=strdup(val[1]);
	s2=strdup(val[2]);

	inputRedirectionFile=removeWhiteSpaces(s1);
	outputRedirectionFile=removeWhiteSpaces(s2);

	tokenizeSpace(val[0]);

	return;
}

// This function creates the pipe and checks for redirection of input or output and then calls the commands with execvp
int executeCommands(char *cmd_exec, int input, int first, int last){
	
	int pipefd[2],check,outputfd,inputfd;

	char *cmd_exec2;
	cmd_exec2=strdup(cmd_exec);

	tokenizeSpace(cmd_exec);

// Check if it's one of the function that have to be called in the main process
	if(strcmp(argv[0],"cd")==0){
			return cd(argv);
		}
	if(strcmp(argv[0],"help")==0){
			help();
			return 0;
		}
		
	if(strcmp(argv[0],"version")==0){
		version();
		return 0;
	}

//else creates the pipes and does the work in the child process
	check=pipe(pipefd);
	if(check==-1){
		perror("Pipe error: ");
		return 1;
	}

	pid=fork();
	
	if(pid==0){//if in child
	
//if first and not the last and input is empty then we'll need a duplicate of the pipe's writing end
		if(first==1 && last==0 && input==0){
//pipefd[1] is the old file descriptor, 1 is the new file descriptor used by dup2 to create a copy
//everything that is taken to stdout (having 1 descriptor) will be written to pipdfd[1](the pipe's writing end) instead of being shown on the screen
			dup2(pipefd[1],1);
		}
//if not first and not the last and input isn't empty then we'll need a duplicate of the input and of the pipe's writing end
		else if(first==0 && last==0 && input!=0){
//dup2(input,0) will therefore redirect the input from stdin (having 0 descriptor) to the input file descriptor
//for the second one, everything that is taken to stdout (having 1 descriptor) will be written to pipdfd[1](the pipe's writing end) instead of being shown on the screen
			dup2(input,0);
			dup2(pipefd[1],1);
		}
//in all other cases, just take the stdin to input file descriptor
		else{
			dup2(input,0);
		}

//now check if both input and output redirection was found, the checks becomes true, and then the cmd exec is tokenized properly
		if((strchr(cmd_exec2,'<')!=0) && (strchr(cmd_exec2,'>')!=0)){

			checkInputRedirection=1;
			checkOutputRedirection=1;
			tokenizeInputOutputRedirection(cmd_exec2);
		}
//now check if input redirection was found, check becomes true, and then the cmd exec is tokenized properly
		else if (strchr(cmd_exec2, '<')!=0) {

			checkInputRedirection=1;
			tokenizeInputRedirection(cmd_exec2);
		}
//else check if output redirection was found, check becomes true, and then the cmd exec is tokenized properly
		else if (strchr(cmd_exec2, '>')!=0) {

			checkOutputRedirection=1;
			tokenizeOutputRedirection(cmd_exec2);
		}
		

//if there is output redirection, try to open the file if exists, if not then create it
		if (checkOutputRedirection==1) {
//take the filed descriptor of the file into outpufd; 0644 - file, read/write, read, read
			outputfd=open(outputRedirectionFile,O_RDWR|O_CREAT,0644);
			if(outputfd<0){

				fprintf(stderr, "Failed to open %s for writing\n",outputRedirectionFile);
				exit(1);
			}
//redirect stdout to the output file, then close the file descriptor of the file and unmark the check
			dup2(outputfd,1);
			close(outputfd);
			checkOutputRedirection=0;
		}

//if there is input redirection, try to open the file if exists
		if (checkInputRedirection==1) {

			inputfd=open(inputRedirectionFile,O_RDONLY,0);
			if (inputfd<0){

				fprintf(stderr, "Failed to open %s for reading\n",inputRedirectionFile);
				exit(1);
			}
//redirect stding to the input file, then close the file descriptor of the file and unmark the check
			dup2(inputfd,0);
			close(inputfd);
			checkInputRedirection=0;
		}
		
		
//check if tail command was invoked and call it
		if(strcmp(argv[0],"tail")==0){
			int argc,check;
			for(argc=0;argv[argc]!=NULL;argc++);
//if tail doesn't return error, signal it and exit the child
			check=tail(argc,argv);
			if(check!=0){

				fprintf(stderr, "\nTail command unsuccessful\n");	
  				exit(1);
			} 
			
		}

//else for any other command, call execvp
		else if(execvp(argv[0],argv)<0){
			
			fprintf(stderr, "%s: Command not found\n",argv[0]);
			exit(1);
		}
		exit(0);
	}

// If in parent process, then wait for the child process
	else{
		wait(NULL);
	}
	
//if the command was the last of the argv array, close the pipe's reading end
	if(last==1)
		close(pipefd[0]);
//if the input is not empty, close the input
	if(input!=0)
		close(input);
	close(pipefd[1]);

	return pipefd[0];
}


int isEmpty(char *s){
	
	int i=0;
	while(s[i]!='\0'){
//if it's not a white space character(e.g. \n, \t, ' ') it returns 1, else 0
		if(isspace(s[i])==0)
			return 1;
		i++;	
	}
	return 0;
}

int main(){

	system("clear");
// using_history() marks the beginning of session where history my be used
	using_history();
// Used to handle the CTRL+C 
	signal(SIGINT,handleCtrlC); 

	do{
		initializeShell();
// readline() prints a prompt and then reads and returns a single line of text from the user
		buff=readline(prompt);

		if(strncmp(buff,"exit",4)==0){
			printf("\nThank you. The shell will now close.\n");
			exit(0);
		}
// If line contaions only spaces or is just a new line, it won't be added to history
		if(isEmpty(buff)==0)
			continue;		
	
		add_history(buff);
		
		tokenizePipe();

	}while(1);


	return 0;
}
