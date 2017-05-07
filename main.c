#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    char keywords[10][10] = {"int", "move", "add", "to", "sub", "from", "loop", "times", "out", "newline" };
    int keyword_count = 0, identifier_count = 0, parenthesis_count = 0, eol_count = 0;

    char filename[100];
    filename[0] = '\0';

    while(1) { //until user enters a correct filename
        printf("Enter a file name: ");
        int is_filename_acceptable = 0;
        scanf(" %[^\n]s", filename); //to accept spaces in file name too
        if (strstr(filename, ".") != NULL) {
            strrev(filename);
            if (filename[0] == 'a' && filename[1] == 'b' && filename[2] == '.') {
                strrev(filename);
                is_filename_acceptable = 1;
            } else {
                printf("Error: Wrong extension!\n");
            }
        } else {
            int len = (int)strlen(filename);
            filename[len] = '.';
            filename[len + 1] = 'b';
            filename[len + 2] = 'a';
            filename[len + 3] = '\0';
            is_filename_acceptable = 1;
        }
        if (is_filename_acceptable) {
            if( access(filename, F_OK) == -1 ) {
                printf("File doesn't exist\n");
            } else {
                break; // file exist, break the while loop
            }
        }
    }

    FILE * source_code = fopen (filename, "r");
    char lexeme[100];
    int i = 0;
    int is_reading_comment = 0, is_reading_string = 0;
    while(1) {
        char c = (char)fgetc(source_code);
        if (c == EOF){
            break;
        }

        if(c == '{' && !is_reading_comment && !is_reading_string){ //don't count parenthesis in strings
            parenthesis_count++;
            is_reading_comment = 1;
        } else if (c == '}' && is_reading_comment && !is_reading_string) { //don't count parenthesis in strings
            parenthesis_count++;
            is_reading_comment = 0;
        }

        if(!is_reading_comment){ //we are not reading comment, lets check lexemes and errors
            if(c == '"'){ // WARNING: " “ ” are not same!! print(“str”) is not valid in C!!!
                is_reading_string = !is_reading_string;
            }
            if(!is_reading_string) {
                if(c == '[' || c == ']'){
                    parenthesis_count++;
                }
                if(c != ' ' && c != '.' && c != '\t' && c != '\n'){
                    lexeme[i++] = c;
                } else {
                    if (c == '.') {
                        eol_count++;
                    }
                    lexeme[i] = '\0';
                    for (int j = 0; j < 10; j++) {
                        if(strcmp(lexeme, keywords[j]) == 0){
                            keyword_count++;
                            printf("%s is a keyword\n", lexeme);
                        }
                    }
                    // if it's not keyword its identifier
                    i = 0;
                }
            }
        }
    }

    printf("New line count: %d\nKeyword count: %d\nParanthesis count: %d", eol_count, keyword_count, parenthesis_count);
    fclose(source_code);

    return 0;
}