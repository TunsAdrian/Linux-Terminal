#define BUFFER 1048576

int multipleFiles=0;
int setV,setQ;

void printHeader(char *,int);
int n_option(char *,int);

int tail(int argc, char *argv[]){

	char *files[10];
	int n,i;
	int setN=0,once=0,noFile=0;

	setV=0;
	setQ=0;
	for(i=1;i<argc;i++)
	{

		if(strcmp(argv[i],"-n")==0)
		{	
			once=1;
			setN=1;
			n=atoi(argv[i+1]);
		
			if(argv[i+1][0]=='-'){
					fprintf(stderr, "Error: invalid option %s\n",argv[i+1]);
					return 1;
				}
			else if(n==0){
				fprintf(stderr, "Error: missing argument\n");
				return 1;
			}

			else if(n<1){
				fprintf(stderr, "Error: incompatible argument\n");
				return 1;
			}
		}

		else if(strcmp(argv[i],"-v")==0)
			if(setQ==0)
				setV=1;
			else{
				fprintf(stderr, "Error: incompatible argument\n");
				return 1;
			}

		else if(strcmp(argv[i],"-q")==0)
			if(setV==0)
				setQ=1;
			else{
				fprintf(stderr, "Error: incompatible argument\n");
				return 1;
			}

		else if(argv[i][0]=='-'){
			fprintf(stderr, "Error: invalid option %s\n",argv[i]);
			return 1;
		}

		else if(once==1){	
			if(atoi(argv[i])==0)files[noFile++]=strdup(argv[i]); 
			once=0;
		}

		else files[noFile++]=strdup(argv[i]);	
	}

	if(noFile>1)multipleFiles=1;
	if(noFile!=0)

	for(i=0;i<noFile;i++)
	{

//Check if file can be opened for reading
		if(access(files[i],F_OK)==0){//!=-1){
//Check if file can be opened for reading
			if(access(files[i],R_OK)==0){

				printHeader(files[i],i+1);
				
				if(setN==1)
					n_option(files[i],n);

//The number 10 is used because that would be the default value of n, for tail command
				else n_option(files[i],10);
				
				}
//If file can't be opened for reading
			else{    
				fprintf(stderr, "Error: cannot open '%s' for reading: Permission denied\n",files[i]);
			}
		} 
		
//If file doesn't exist
		else{
			fprintf(stderr, "Error: cannot open '%s' for reading: No such file or directory\n",files[i]);			
		}
		  
	}
//Now got to free the memory allocated for the files
	for(i=0;i<noFile;i++)free(files[i]);
	
	return 0;	   
}

// This function checks the global set verbose(setV) and set quiet(setQ) and prints the header accordingly
void printHeader(char *name, int fileNo){

	if(fileNo==1 && setQ==0)
		if(setV==1 || multipleFiles==1)
			printf("==> %s <==\n",name);
	if(fileNo!=1 && setQ!=1)
		printf("\n==> %s <==\n",name);

	return;
}

// This function prints the last n lines of the file
int n_option(char *source, int n){

	FILE *fd;
	char *lines[1000];
	int noLines=0,ok=0;
	int i,j;
	char *buffer=(char*)malloc(sizeof(char)*BUFFER);
// fd is the file descriptor of the source file, opened in read mode
	fd=fopen(source,"r");
// while end of file not reached
	do{
// fgets reads from fd file into buffer and it stops when new line is found
		fgets(buffer,BUFFER-1,fd);
// in the lines char array(matrix) the lines one by on are allocated memory for the BUFFER size
		lines[noLines]=(char*)malloc(sizeof(char)*BUFFER);
// the line read into the buffer is copied into the array lines and the the number of lines is incremented
		strcpy(lines[noLines],buffer);
		noLines++;
	}while(feof(fd)==0);
	
// if there are no linnes, print the error to stderr file
	if(noLines<=0)
		fprintf(stderr, "Error: no lines\n");
	else{
// else compute how the n lines that must be showed on screen and then print them
		if(noLines>n)
			i=noLines-n;
		else i=0;
		
		for(j=i;j<noLines-1;j++){
			printf("%s",lines[j]);
			ok=1;
		}
	}
// free all the files and close the file descriptor
	for(i=0;i<noLines;i++)free(lines[i]);

	fclose(fd);
	
	return ok;
}
