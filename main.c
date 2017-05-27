#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_DIGIT 100

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b)) //returns biggest
#endif

struct token;
struct symbol;

void printToken(struct token t);

char *addDecimals(char *a, char *b);

char *subDecimals(char *a, char *b);

char *shiftstr(char *str, int n);

struct token { //definition
    char *type; //identifier, keyword, integer, parenthesis, eol, comma, string
    char *value;
};

struct symbol {
    char *name;
    char *value;
};

int main() {
    char keywords[10][10] = {"int", "move", "add", "to", "sub", "from", "loop", "times", "out", "newline"};
    int eol_count = 0;
    char filename[100];
    filename[0] = '\0';

    //for testing purpose
//    char *result;
//    char *a = "-35";
//    char *b = "100";
//    result = addDecimals(a, b);
//    printf("%s\n", result);

    while (1) { //loop until user enters a correct filename
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
        } else { //filename dont have extension, lets add
            int len = (int) strlen(filename);
            filename[len] = '.';
            filename[len + 1] = 'b';
            filename[len + 2] = 'a';
            filename[len + 3] = '\0';
            is_filename_acceptable = 1;
        }
        if (is_filename_acceptable) {
            if (access(filename, F_OK) == -1) {
                printf("File doesn't exist\n");
            } else {
                break; // file exist, break the while loop
            }
        }
    }

    FILE *source_code = fopen(filename, "r"); //lexical analysis output
    char lexeme[MAX_DIGIT + 1]; //temporary char array for each lexeme
    struct token tokens[100];
    int token_count = 0;

    int i = 0;
    int is_reading_comment = 0, is_reading_string = 0;
    while (1) {
        char c = (char) fgetc(source_code); //read one char from source
        if (c == EOF) {
            break;
        }
        if (c == '{' && !is_reading_comment && !is_reading_string) { //don't count parenthesis in strings
            //fputs("{ is a parenthesis\n", output_file);
            is_reading_comment = 1;
            continue;
        } else if (c == '}' && is_reading_comment && !is_reading_string) {
            //fputs("} is a parenthesis\n", output_file);
            is_reading_comment = 0;
            continue;
        }

        if (!is_reading_comment) { //we are not reading comment, lets check lexemes and errors
            if (c == '"') { // WARNING: " “ ” are not same!! print(“str”) is not valid in C!!!
                if (is_reading_string) { //this means we are at the end of a string
                    lexeme[i++] = c;
                    lexeme[i] = '\0';
                    tokens[token_count].type = "string";
                    tokens[token_count].value = strdup(lexeme); //duplicate lexeme, we will change lexeme later
                    token_count++;
                    i = 0;
                } else {
                    lexeme[i++] = c;
                }
                is_reading_string = !is_reading_string;
                continue;
            }
            if (is_reading_string) {
                lexeme[i++] = c;
            }

            if (!is_reading_string) {
                if ((c >= 65 && c <= 91) || (c >= 97 && c <= 123) || (c >= 48 && c <= 57) || (c >= 44 && c <= 46)
                    || c == 93 || c == 125 || c == 32 || c == 9 || c == 10) {
                    //accept A-Z a-z 0-9 , - . [] {} space \t \n characters in source code

                    if (c == '[') {
                        tokens[token_count].type = "parenthesis";
                        tokens[token_count].value = "[";
                        token_count++;
                        continue;
                    } else if (c == ']') {
                        tokens[token_count].type = "parenthesis";
                        tokens[token_count].value = "]";
                        token_count++;
                        continue;
                    }

                    if (c != ' ' && c != '.' && c != '\t' && c != '\n' && c != ',') {
                        lexeme[i++] = c; //still reading a lexeme
                    } else {
                        if (i != 0) { //check if lexeme is not null/empty
                            lexeme[i] = '\0';
                            for (int j = 0; j < 10; j++) { //for loop that checks if lexeme is a keyword
                                if (strcmp(lexeme, keywords[j]) == 0) { //string equality
                                    tokens[token_count].type = "keyword";
                                    tokens[token_count].value = strdup(lexeme);
                                    token_count++;
                                    break;
                                }
                                if (j == 9 && (lexeme[0] < 48 || lexeme[0] > 57) && lexeme[0] != '-') {//end of loop
                                    tokens[token_count].type = "indentifier";
                                    tokens[token_count].value = strdup(lexeme);
                                    token_count++;
                                } else if ((j == 9 && lexeme[0] >= 48 && lexeme[0] <= 57) ||
                                           (j == 9 && lexeme[0] == '-')) {
                                    tokens[token_count].type = "integer";
                                    tokens[token_count].value = strdup(lexeme);
                                    token_count++;
                                }
                            }
                        }
                        if (c == '.') {
                            eol_count++;
                            tokens[token_count].type = "eol";
                            tokens[token_count].value = "."; //not necessary, for printing purpose only
                            token_count++;
                        } else if (c == ',') {
                            tokens[token_count].type = "comma";
                            tokens[token_count].value = ","; //not necessary, for printing purpose only
                            token_count++;
                        }
                        i = 0;
                    }
                } else {
                    printf("Unexpected character: %c in line %d", c, eol_count + 1);
                    return -1;
                }
            }
        }
    }
    fclose(source_code); //we are done with source file
    //now we got tokens[] array from lexical analyzer

    //testing purpose only
    for (int k = 0; k < token_count; k++) {
        printToken(tokens[k]);
    }

    struct symbol symbol_table[100];
    int symbol_count = 0;
    for (i = 0; i < token_count; ++i) {
        if (strcmp(tokens[i].type, "keyword") == 0) {
            if (strcmp(tokens[i].value, "int") == 0) { //new integer declaration
                symbol_table[symbol_count].name = tokens[++i].value;
                symbol_table[symbol_count].value = "0";
            }
        }
    }

    return 0;
}

void printToken(struct token t) {
    printf("%s is a %s\n", t.value, t.type);
}

char *addDecimals(char *a, char *b) {
    int negative = 0;
    if (a[0] == '-' && b[0] != '-') { // -3 + 5 == 5 - 3
        a = shiftstr(a, -1); //remove negative sign by shifting one char left
        return subDecimals(b, a); //if a is negative, subtract a from b
    } else if (a[0] != '-' && b[0] == '-') { //3 + (-5) == 3 - 5
        char positive_b[MAX_DIGIT];
        for (int j = 0; j < strlen(b); j++) { // shift left by one character, we dont need '-'
            positive_b[j] = b[j + 1];
        }
        positive_b[strlen(b) - 1] = '\0';
        return subDecimals(a, positive_b); //if b is negative, subtract b from a
    } else if (a[0] == '-' && b[0] == '-')
        negative = 1; //if both are negative, answer will be negative

    char result[MAX_DIGIT + 1], x[MAX_DIGIT + 1], y[MAX_DIGIT + 1], carry = '0';
    int k;
    for (k = 0; k < strlen(a); ++k) {
        x[k] = *(a + k);
    }
    x[k] = '\0';
    for (k = 0; k < strlen(b); ++k) {
        y[k] = *(b + k);
    }
    y[k] = '\0';

    strrev(x);
    strrev(y);

    int i = 0, x_ended = 0, y_ended = 0;
    for (;;) { // wtf? // while(1)
        if (x[i] > 57 || x[i] < 48) { //check if its not a number. x="123", x[3], x[4] is not number
            x[i] = 48; //assign 0
            x[i + 1] = '\0';
            x_ended = 1;
        }

        if (y[i] > 57 || y[i] < 48) { //check if its not a number
            y[i] = 48;
            y[i + 1] = '\0';
            y_ended = 1;
        }
        if (x_ended && y_ended)
            break;

        // http://www.asciitable.com/
        //   3   +  5  +   1   <=  9
        //  51   + 53  +  49   <= 153
        if (x[i] + y[i] + carry <= 153) {  // check if it doesnt exceeds 9
            result[i] = (char) (x[i] + y[i] + carry - 96); //take i'th char from
            carry = '0';
        } else {    //it exceeds 9, there will be carry
            result[i] = (char) (x[i] + y[i] + carry - 106);
            carry = '1';
        }
        i++;
    }

    if (carry == '1')
        result[i++] = '1';

    result[i] = '\0';
    strrev(result);
    if (negative) { // lets add '-' at the start of result
        for (int j = strlen(result); j > 0; j--) { // shift right by one character
            result[j] = result[j - 1];
        }
        result[0] = '-';
        result[i + 1] = '\0'; //update end of string
    }
    return strdup(result);
}

char *subDecimals(char *a, char *b) {
    if (strcmp(a, b) == 0)
        return "0";

    int negative = 0;
    if (a[0] != '-' && b[0] == '-') { // +x - (-y) == x + y
        b = shiftstr(b, -1); // shift left by one character, we dont need '-'
        return addDecimals(a, b); //"sub - from +" is equal to "add + to +"
    } else if (a[0] == '-' && b[0] != '-') { // -x - x = -x + (-x)
        b = shiftstr(b, 1);
        b[0] = '-';
        return addDecimals(a, b); //if b is negative, subtract b from a
    } else if (a[0] == '-' && b[0] == '-') {
        if (strlen(a) > strlen(b))
            negative = 1;
        else if (strlen(a) == strlen(b) && strcmp(a, b) > 0) { //-5 - (-3) = -2
            negative = 1;
        }

    } else if (strlen(b) > strlen(a)) //3 - 55 = -52
        negative = 1;
    else if (strlen(b) == strlen(a) && strcmp(b, a) > 0) { // 3 - 5 = -2
        negative = 1;
        char *temp = b;
        b = a;
        a = temp;
    }

    char result[MAX_DIGIT + 1], x[MAX_DIGIT + 1], y[MAX_DIGIT + 1];
    int k;
    for (k = 0; k < strlen(a); ++k) {
        x[k] = *(a + k);
    }
    x[k] = '\0';
    for (k = 0; k < strlen(b); ++k) {
        y[k] = *(b + k);
    }
    y[k] = '\0';

    strrev(x);
    strrev(y);

    //i is current digit number
    int i = 0, x_ended = 0, y_ended = 0;
    while (1) {
        if (x[i] > 57 || x[i] < 48) {  //check if its not a number
            x[i] = 48; //assign 0
            x[i + 1] = '\0';
            x_ended = 1;
        }
        if (y[i] > 57 || y[i] < 48) { //check if its not a number
            y[i] = 48;
            y[i + 1] = '\0';
            y_ended = 1;
        }
        if (x_ended && y_ended)
            break;

        // http://www.asciitable.com/
        //   5   -  3   > 0
        //  53   - 51   > 0
        if (x[i] - y[i] >= 0) {  // check if it doesnt exceeds 9
            result[i] = (char) (x[i] - y[i] + 48); // 48 is ascii of '0'
        } else {    // 3 - 5?
            int j = i + 1;
            while (x[j] == '0') { // assign 9 to all zeros, 100000 - 1?
                x[j] = '9';
                j++;
            }
            x[j] = (char) (x[j] - 1);

            //                  13         - 5        = 8
            //                  51   + 10 - 53   + 48 = 56
            result[i] = (char) (x[i] + 10 - y[i] + 48);
        }
        i++;
    }

    result[i] = '\0';
    strrev(result);

    if (negative) { // lets add '-' at the start of result
        for (int j = strlen(result); j > 0; j--) { // shift right by one character
            result[j] = result[j - 1];
        }
        result[0] = '-';
        result[i + 1] = '\0'; //update end of string
    }
    return strdup(result);
}

char *shiftstr(char *str, int n) {
    if (n < 0) { // shift right
        str = str + abs(n);
        return str;
    } else if (n > 0) {
        char *new_str = malloc(sizeof(char) * (strlen(str) + n));
        for (int i = 0; i < strlen(str) + n; ++i) {
            if (i < n)
                *(new_str + i) = '0';
            else
                *(new_str + i) = str[i - n];
        }
        *(new_str + strlen(str) + n) = '\0';
        free(str);
        return new_str;
    }

}