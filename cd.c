int cd(char *argv[]){

	char *home_dir="/home";
	int ok=0;

//Checks if it's not the case that goes to home directory
	if((argv[1]==NULL)||(strcmp(argv[1],"~")==0)||(strcmp(argv[1],"--")==0))
		chdir(home_dir);

//else it will call chdir -> specified path; in case of failure, return the error
	else {
		ok=chdir(argv[1]);
		if(ok<0)
			perror("Error: ");
			return 1;
	}

	return 0;
}
