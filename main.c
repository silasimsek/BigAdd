#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    char keywords[10][10] = {"int", "move", "add", "to", "sub", "from", "loop", "times", "out", "newline" };
    int keyword_count = 0, identifier_count = 0, parenthesis_count = 0, newline_count = 0;
    char parenthesis[] = {'(', ')', '{', '}', '[', ']'};

    char filename[100];
    filename[0] = '\0';

    while(1) { //until a correct filename
        printf("Enter a file name: ");
        scanf(" %[^\n]s", filename); //to accept spaces too

        if (strstr(filename, ".") != NULL) {
            strrev(filename);
            if (filename[0] == 'a' && filename[1] == 'b' && filename[2] == '.') {
                strrev(filename);
                if( access(filename, F_OK) == -1 ) {
                    printf("File doesn't exist\n");
                } else {
                    break; // file exist
                }
            } else {
                printf("Error: Wrong extension!\n");
            }
        } else {
            int len = (int)strlen(filename);
            filename[len] = '.';
            filename[len + 1] = 'b';
            filename[len + 2] = 'a';
            filename[len + 3] = '\0';
            if( access(filename, F_OK) == -1 ) {
                printf("File doesn't exist\n");
            } else {
                break; // file exist
            }
        }
    }

    FILE * source_code = fopen (filename, "r");
    char lexeme[100];
    int i = 0;

    while(1) {
        char c = fgetc(source_code);
        if (c == EOF){
            break;
        }

        //check for errors
        for(int j = 0; j < 6; j++){
            if(c == parenthesis[j]){
                parenthesis_count++;
            }
        }

        if(c != ' ' && c != '\n' ){
            lexeme[i++] = c;
        } else {
            if (c == '\n' ) {
                newline_count++;
            }
            lexeme[i] = '\0';
            for (int j = 0; j < 10; j++) {
                if(strcmp(lexeme, keywords[j]) == 0){
                    keyword_count++; //currently it counts keywords in comments too, it will be fixed soon :)
                    printf("%s is a keyword\n", lexeme);
                }
            }
            // if it's not keyword its identifier
            i = 0;
        }
    }

    printf("New line count: %d\nKeyword count: %d\nParanthesis count: %d", newline_count, keyword_count, parenthesis_count);
    fclose(source_code);

    return 0;
}