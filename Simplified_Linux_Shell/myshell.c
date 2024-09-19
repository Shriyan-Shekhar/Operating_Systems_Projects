#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> // For constants that are required in open/read/write/close syscalls
#include <sys/wait.h> // For wait() - suppress warning messages
#include <fcntl.h>    // For open/read/write/close syscalls

#define MYSHELL_WELCOME_MESSAGE "COMP3511 PA1 Myshell (Spring 2024)"


#define TEMPLATE_MYSHELL_START "Myshell (pid=%d) starts\n"
#define TEMPLATE_MYSHELL_END "Myshell (pid=%d) ends\n"
#define TEMPLATE_MYSHELL_CD_ERROR "Myshell cd command error\n"

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LENGTH 256

// Assume that we have at most 8 arguments
#define MAX_ARGUMENTS 8

// Assume that we only need to support 2 types of space characters:
// " " (space) and "\t" (tab)
#define SPACE_CHARS " \t"

// The pipe character
#define PIPE_CHAR "|"

// Assume that we only have at most 8 pipe segements,
// and each segment has at most 256 characters
#define MAX_PIPE_SEGMENTS 8


#define MAX_ARGUMENTS_PER_SEGMENT 9

// Define the standard file descriptor IDs here
#define STDIN_FILENO 0  // Standard input
#define STDOUT_FILENO 1 // Standard output

// This function will be invoked by main()
void show_prompt(char *prompt, char *path)
{
    printf("%s %s> ", prompt, path);
}

// This function will be invoked by main()
int get_cmd_line(char *cmdline)
{
    int i;
    int n;
    if (!fgets(cmdline, MAX_CMDLINE_LENGTH, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(cmdline);
    cmdline[--n] = '\0';
    i = 0;
    while (i < n && cmdline[i] == ' ')
    {
        ++i;
    }
    if (i == n)
    {
        // Empty command
        return -1;
    }
    return 0;
}

// parse_arguments function is given
// This function helps you parse the command line

void parse_arguments(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

void process_cmd(char *cmdline)
{  
    char* pipes[MAX_PIPE_SEGMENTS];
    int num_segments = 0;
    parse_arguments(pipes, cmdline, &num_segments, "|"); //following the comment section for parse
    
    int fd[2]; 
    pid_t pid;
    //int fd_temp= 0;

    int totalSegments = MAX_ARGUMENTS + 1; 
    char * argument [MAX_ARGUMENTS_PER_SEGMENT] = {NULL};
    char * cmd [MAX_PIPE_SEGMENTS + 1][MAX_ARGUMENTS_PER_SEGMENT] = {NULL};
    char * splitCmd [MAX_PIPE_SEGMENTS + 1][MAX_ARGUMENTS_PER_SEGMENT] = {NULL};
    
    int total; //total segments
    int i;
    int j;
    int k;
    for (i = 0; i < num_segments; i++) {
        for(k = 0; k < MAX_ARGUMENTS_PER_SEGMENT; k++){
            argument[k] = NULL;
        }
        

        parse_arguments(argument, pipes[i], &total, " \t");
        
        for (j = 0; j < MAX_ARGUMENTS_PER_SEGMENT; j++){
            splitCmd[i][j] = argument[j];
        }
        
        //part 3 handling < and >
        for (int l = 0; l < total; l++){
            if (strcmp (argument[l], "<") == 0){
                int inputFd = open (argument[l+1], O_RDONLY); //case2: command < input
                dup2(inputFd, STDIN_FILENO);
                close (inputFd);
            
                for (int j = l; j < total; j++){// case 4: command <input> output
                    if (strcmp (argument[j], ">") == 0){
                        int outputFd2 = open (argument[j+1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
                        dup2(outputFd2, STDOUT_FILENO);
                        close (outputFd2);
                        break;
                    }
                }
                cmd [0][l] = NULL;
                execvp (cmd[0][0], cmd[0]);
                l = total;
                break;
            }
        
            if (strcmp (argument[l], ">") == 0){      
                     
                int outputFd = open (argument[l+1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR); //case 3: command > output
                dup2(outputFd, STDOUT_FILENO);
                close (outputFd);
            
                for (int k = l; k < total; k++){
                    if (strcmp (argument[k], "<") == 0){ //case 5: command > output < input
                        int inputFd2 = open (argument[k+1], O_RDONLY);
                        dup2(inputFd2, STDIN_FILENO);
                        close (inputFd2);
                        break;
                    }   
                }
                cmd [0][l] = NULL; 
                execvp (cmd[0][0], cmd[0]);
                l = total;
                break;
            }

            //case 1 here and outside loop - case 1: command
            cmd [0][l] = argument[l];
            
        }
        argument[num_segments] = NULL;
       
        if ((num_segments == 1) || (i == num_segments - 1)) //case 1 handling and last command handling
        {
            execvp(splitCmd[i][0], splitCmd[i]);
        }
        else if (i < num_segments - 1) //pipe handling
        {
            pipe(fd);
            pid_t pid = fork(); // fork for child process

            if (pid > 0) // Parent process
            {
                close(0);     
                dup2(fd[0], 0); // stdin as output
                //fd_temp = fd[0];
                close(fd[1]);  // close output
                wait(0);  // wait for child
                
            }
            else if (pid == 0)
            {
                close(1);  
                //dup2(fd_temp, 0);    
                dup2(fd[1], 1); //  stdout as input 
                close(fd[0]);   //close input
                close(fd[1]);   //close output more for safety
                execvp(splitCmd[i][0], splitCmd[i]); // execution of command
            }
        }
        
    }

   
    exit (0); // final exit 
}

/* The main function implementation */
int main()
{
    char *prompt = "username"; //change this to your own username
    char cmdline[MAX_CMDLINE_LENGTH];
    char path[256]; // assume path has at most 256 characters

    printf("%s\n\n", MYSHELL_WELCOME_MESSAGE);
    printf(TEMPLATE_MYSHELL_START, getpid());

    // The main event loop
    while (1)
    {
        getcwd(path, 256);
        show_prompt(prompt, path);

        if (get_cmd_line(cmdline) == -1)
            continue; // empty line handling, continue and do not run process_cmd

        
        if (strcmp("exit", cmdline) == 0){
            printf ("Myshell (pid=%d) ends\n", getpid());
            exit (0);
        }

        int i = 0;
        for (i = 0; i < strlen(cmdline) - 1; i++){ // included to handle spaces for cd
            if (cmdline [i] == 'c' && cmdline [i+1] == 'd'){
                break;
            }
        }
        
        if ((cmdline [i] == 'c') && (cmdline [i+1] == 'd') && (cmdline [i+2] == ' ')){
            char *chgeDir = cmdline + i + 3;
            char modifiedDir[strlen(chgeDir) + 1]; // Create a new string to store the modified directory

            int j = 0;
            for (int k = 0; k < strlen(chgeDir); k++) {
                if (chgeDir[k] != ' ' && chgeDir[k] != '\t') {
                    modifiedDir[j] = chgeDir[k];
                    j++;
                }
            }
            modifiedDir[j] = '\0'; // Add the null-terminating character to the new string
            
            if (strcmp (modifiedDir, ".") == 0){ 
                continue; // condition of .
            }
            else if (strcmp (modifiedDir, "..") == 0){
                chdir (".."); // condition of ..
                continue;
            }
            else{
                if (chdir (modifiedDir) == -1){ //failure
                    printf (TEMPLATE_MYSHELL_CD_ERROR);
                    continue;
                }
                else{
                    continue;
                }
            }
            continue;
        }
        
        

        pid_t pid = fork();
        if (pid == 0)
        {
            // the child process handles the command
            process_cmd(cmdline);
        }
        else
        {
            // the parent process simply wait for the child and do nothing
            wait(0);
        }
    }

    return 0;
}