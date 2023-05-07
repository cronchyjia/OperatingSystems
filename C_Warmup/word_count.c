#include <stdio.h>
#include <string.h>

#define INPUT_LEN 115

/* Function Declaration*/
int token_words(char line[INPUT_LEN]);


int main(int argc, char *argv[]){
    char line[INPUT_LEN];
    int word = 0;

    FILE* file = fopen(argv[1], "r");

    while(fgets(line, INPUT_LEN, file) != NULL){

        //removes \n char
        line[strcspn(line, "\n")] = 0;

        int number = token_words(line);
        word = word + number;
    }

    fclose(file);

    printf("%d %s\n", word, argv[1]);
    return 0;
}

int token_words(char line[INPUT_LEN]){
    char* token;
    int word = 0;

    token = strtok(line, " ");

    while(token != NULL){
        token = strtok(NULL, " ");
        word++;
    }

    return word;
}
