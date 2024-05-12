/* gopipe.c
 *
 * CSC 360, Spring 2024
 *
 * Execute up to four instructions, piping the output of each into the
 * input of the next.
 *
 * Please change the following before submission:
 *
 * Author: Din Grogu
 * Login:  babyyoda@uvic.ca 
 */


/* Note: The following are the **ONLY** header files you are
 * permitted to use for this assignment! */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

/*
 * Macros 
 */
#define MAX_ARGUMENTS 7
#define MAX_COMMANDS 4
#define MAX_LINE_LEN 80 


/*
 * Function Declatarions
 */
void create_children(char arg[][MAX_LINE_LEN], int fd[], int num_of_inputs);
void execute(char* command);

void execute(char* command){
    char* argument[MAX_LINE_LEN];
    char* token;
    int i = 0;

    token = strtok(command, " ");
    while(token){
        argument[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    //Makes sure the array ends with a NULL
    argument[i] = NULL;
    execvp(argument[0], argument);

    exit(1);
}

void create_children(char arg[][MAX_LINE_LEN], int fd[], int num_of_inputs){
    pid_t pid;
    
    for (int i = 0; i < num_of_inputs; i++){
        pipe(fd);
        pid = fork();

        //Forks a child
        if (pid < 0){
            char* message = "Forking had failed";
            write(1, message, strlen(message));
        }
        //Redirects output to the pipe
        else if (pid == 0){
            dup2(fd[1], 1); //Makes output go to the pipe
            close(fd[0]); //Close unused read end
            execute(arg[i]);
        }
        //Parent Process
        else{
            wait(&pid);
            close(fd[1]);
            dup2(fd[0], 0);
            close(fd[0]);
        }
    }
    execute("cat");
}

int main() {
    char arguments[MAX_COMMANDS][MAX_LINE_LEN];
    int fd[2], nbytes, usr_input;

    for (usr_input = 0; usr_input < MAX_COMMANDS; usr_input++){
        nbytes = read(STDOUT_FILENO, arguments[usr_input], MAX_LINE_LEN);
        if (nbytes == 0 || arguments[usr_input][0] == '\n'){
            break;
        }
        arguments[usr_input][nbytes - 1] = '\0';
    }

    create_children(arguments, fd, usr_input);

    return 0;
}