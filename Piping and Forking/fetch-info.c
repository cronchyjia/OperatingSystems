/* getstats.c 
 *
 * CSC 360, Spring 2024
 *
 * - If run without an argument, prints information about 
 *   the computer to stdout.
 *
 * - If run with a process number created by the current user, 
 *   prints information about that process to stdout.
 *
 * Please change the following before submission:
 *
 * Author: Din Grogu
 * Login:  babyyoda@uvic.ca 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Note: You are permitted, and even encouraged, to add other
 * support functions in order to reduce duplication of code, or
 * to increase the clarity of your solution, or both.
 */

/*
 * Macros 
 */
#define LINE_LEN 500

/*
 * Function Declatarions
 */
void convert_uptime(FILE *file);
char* grab_last(char* line);
char* wanted_line(FILE *file, const char *needle);


char* grab_last(char* line){
    char* last_tok = NULL;
    char* token;

    token = strtok(line, " \t\n");

    while(token){
        last_tok = token;
        token = strtok(NULL, " \t\n");
    }
    return last_tok;
}

// Loop through lines in the file
char* wanted_line(FILE *file, const char* needle){
    char haystack[LINE_LEN];
    char* line = NULL;
    
    while (fgets(haystack, LINE_LEN, file) != NULL) {
        if(strstr(haystack, needle)){
            line = strstr(haystack, needle);
            return line;
        }
    }
    return line;
}


void convert_uptime(FILE *file){
    int uptime, days, hours, minutes, seconds;

    if (fscanf(file, "%i", &uptime) != 1){
        return;
    }

    days = uptime/(24 * 3600);

    uptime = uptime % (24 * 3600);
    hours = uptime/3600;

    uptime %= 3600;
    minutes = uptime/60;

    uptime %= 60;
    seconds = uptime;

    printf("Uptime: %i days, %i hours, %i minutes, %i seconds\n", days, hours, minutes, seconds);   
}


void print_process_info(char * process_num) {
    FILE *fptr;
    FILE *fptr_comm;
    char* output_line;
    int num1;
    int num2;
    char filepath[LINE_LEN];

    
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", process_num);
    fptr = fopen(filepath, "r");

    if (fptr == NULL){
        printf("Process number %s not found.\n", process_num);
        return;
    }

    printf("Process number: %s\n", process_num);
    
    output_line = wanted_line(fptr, "Name:");
    printf("%s", output_line);

    snprintf(filepath, sizeof(filepath), "/proc/%s/comm", process_num);
    fptr_comm = fopen(filepath, "r");
    fscanf(fptr_comm, "%s", filepath);
    
    printf("Filename (if any): %s\n", filepath);
    
    output_line = wanted_line(fptr, "Threads");
    printf("%s", output_line);


    num1 = atoi(grab_last(wanted_line(fptr, "voluntary_ctxt_switches")));
    num2 = atoi(grab_last(wanted_line(fptr, "nonvoluntary_ctxt_switches")));
    printf("Total context switches: %i\n", num1 + num2);
    
    fclose(fptr);
    fclose(fptr_comm);
} 


void print_full_info() {
    FILE *fptr_info;
    FILE *fptr_version;
    FILE *fptr_meminfo;
    FILE *fptr_uptime;
    char* output_line;
    
    fptr_info = fopen("/proc/cpuinfo", "r");
    fptr_version = fopen("/proc/version", "r");
    fptr_meminfo = fopen("/proc/meminfo", "r");
    fptr_uptime = fopen("/proc/uptime", "r");

    if (fptr_info == NULL){
        printf("File cannot be found :(\n");
        return;
    }

    output_line = wanted_line(fptr_info, "model name");
    printf("%s", output_line);
    
    output_line = wanted_line(fptr_info, "cpu cores");
    printf("%s", output_line);

    output_line = wanted_line(fptr_version, "version");
    printf("%s", output_line);

    output_line = wanted_line(fptr_meminfo, "MemTotal");
    printf("%s", output_line);

    convert_uptime(fptr_uptime);
    
    fclose(fptr_info);
    fclose(fptr_version);
    fclose(fptr_meminfo);
    fclose(fptr_uptime);
}

int main(int argc, char ** argv) {  
    if (argc == 1) {
        print_full_info();
    } else {
        print_process_info(argv[1]);
    }
}
