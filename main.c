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
    FILE *output_file = fopen("lexemes.txt", "w");
    while(1) {
        char c = (char)fgetc(source_code);
        if (c == EOF){
            break;
        }
        if(c == '{' && !is_reading_comment && !is_reading_string){ //don't count parenthesis in strings
            parenthesis_count++;
            //fputs("{ is a parenthesis\n", output_file);
            is_reading_comment = 1;
            continue;
        } else if (c == '}' && is_reading_comment && !is_reading_string) { //don't count parenthesis in strings
            parenthesis_count++;
            //fputs("} is a parenthesis\n", output_file);
            is_reading_comment = 0;
            continue;
        }

        if(!is_reading_comment){ //we are not reading comment, lets check lexemes and errors
            if(c == '"'){ // WARNING: " “ ” are not same!! print(“str”) is not valid in C!!!
                if(is_reading_string){
                    lexeme[i++] = c;
                    lexeme[i] = '\0';
                    fprintf(output_file, "%s is a string\n", lexeme);
                    i = 0;
                }else{
                    lexeme[i++] = c;
                }
                is_reading_string = !is_reading_string;
                continue;
            }
            if(is_reading_string){
                lexeme[i++] = c;
            }

            if(!is_reading_string) {
                if((c>=65 && c<=91) || (c>=97 && c<=123) || (c>=48 && c<=57) || (c>=44 && c<=46)
                   || c==34 || c==93 || c==125 || c==32 || c==9 || c==10){

                    if(c == '['){
                        fputs("[ is a parenthesis\n", output_file);
                        parenthesis_count++;
                        continue;
                    }else if(c == ']'){
                        fputs("] is a parenthesis\n", output_file);
                        parenthesis_count++;
                        continue;
                    }

                    if(c!=' ' && c!='.' && c!='\t' && c!='\n' && c!=','){
                        lexeme[i++] = c;
                    } else {
                        if (i != 0) {
                            lexeme[i] = '\0';
                            for (int j = 0; j < 10; j++) {
                                if(strcmp(lexeme, keywords[j]) == 0){
                                    keyword_count++;
                                    //printf("%s is a keyword\n", lexeme);
                                    fprintf(output_file, "%s is a keyword\n", lexeme);
                                    break;
                                }
                                if(j == 9 &&  (lexeme[0]<48 || lexeme[0]>57)){
                                    identifier_count++;
                                    fprintf(output_file, "%s is a identifier\n", lexeme);
                                    //printf("%s is a identifier\n", lexeme );
                                }else if(j ==9 && lexeme[0]>=48 && lexeme[0]<=57){
                                    fprintf(output_file, "%s is an integer\n", lexeme);
                                }
                            }
                        }
                        if (c == '.') {
                            eol_count++;
                            fputs(". is end of line\n", output_file);
                        }else if(c == ','){
                            fputs(", is a comma\n", output_file);
                        }
                        i = 0;
                    }
                }else{
                    printf("Unexpected character: %c in line %d", c, eol_count);
                    return -1;
                }
            }
        }
    }

    printf("New line count: %d\nKeyword count: %d\nParanthesis count: %d\nIdentifier count: %d ",
           eol_count, keyword_count, parenthesis_count, identifier_count);

    fclose(source_code);
    fclose(output_file);
    return 0;
}