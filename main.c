#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h> //Requires C99 standard!

#define MAX_DIGIT 100
#define INITIAL_MAX 10

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b)) //returns biggest
#endif

struct token { //definition
    char *type; //identifier, keyword, integer, parenthesis, eol, comma, string
    char *value;
    int line;
    int column;
};

struct symbol {
    char *name;
    char *value;
};

struct stack {
    int max;
    char *elements;
    int top;
};

char pop(struct stack *st);

void push(struct stack *st, char c);

char *add(char *a, char *b);

char *sub(char *a, char *b);

//shifts strings by n characters
// n=-1 means shift left by one character, n=2 means shift right by two character
char *shiftstr(char *str, int n);

int error(int type, char *info, struct token t);

int stop();

char *valueof(struct token target); //returns decimal value of a token. token can be identifier or integer
char *get(char *target_name); //gets the value of and identifier
int set(char *target_name, char *new_val); // sets the value of and identifier
int compare(char *a, char *b); //compare two decimals, returns -1 if a<b, 0 if a=b, 1 if a>b

struct symbol symbol_table[100];
int symbol_count = 0;

int main(int argc, char *argv[]) {
    char keywords[10][10] = {"int", "move", "add", "to", "sub", "from", "loop", "times", "out", "newline"};
    int line_count = 1;
    char *filename;
    bool arg_handled = false;

    if (argc <= 1) {
        filename = malloc(100 * sizeof(char));
        *(filename) = '\0';
    } else if (argc == 2)
        filename = argv[1];
    else {
        //program working path always passed as first argument
        printf("Too many arguments given. Maximum one argument expected.");
        return stop();
    }

    while (true) { //loop until user enters a correct filename
        if (argc <= 1 || arg_handled) {
            printf("Enter a file name: ");
            scanf(" %[^\n]s", filename); //to accept spaces in file name too
        } else {
            arg_handled = true;
        }
        bool filename_ok = false;
        if (strstr(filename, ".") != NULL) {
            strrev(filename);
            if (filename[0] == 'a' && filename[1] == 'b' && filename[2] == '.') {
                strrev(filename);
                filename_ok = true;
            } else {
                printf("Error: Wrong extension!\n");
            }
        } else { //filename dont have extension, lets add
            int len = (int) strlen(filename);
            filename[len] = '.';
            filename[len + 1] = 'b';
            filename[len + 2] = 'a';
            filename[len + 3] = '\0';
            filename_ok = 1;
        }
        if (filename_ok) {
            if (access(filename, F_OK) == -1) {
                printf("File doesn't exist\n");
            } else {
                break; // file exist, break the while loop
            }
        }
    }

    FILE *source_code = fopen(filename, "r"); //lexical analysis output
    char lexeme[105]; //temporary char array for each lexeme
    struct token tokens[1000];
    int token_count = 0;
    int eof_col = 0;//used for error reporting, not important

    int i = 0, col = 0, start_column = 0;
    bool is_reading_comment = false, is_reading_string = false, is_integer = true;
    while (1) {
        char c = (char) fgetc(source_code); //read one char from source

        if (c == '{' && !is_reading_comment && !is_reading_string) { //don't count parenthesis in strings
            //fputs("{ is a parenthesis\n", output_file);
            is_reading_comment = true;
            continue;
        } else if (c == '}' && is_reading_comment && !is_reading_string) {
            //fputs("} is a parenthesis\n", output_file);
            is_reading_comment = false;
            continue;
        }

        col++;
        if (i == 0) { //its first character of a lexeme
            start_column = col;
        }
        if (c == '\n') {
            eof_col = col; //used for error reporting, not important
            col = 0;
            line_count++;
        }

        if (!is_reading_comment) { //we are not reading comment, lets check lexemes and errors
            if (c == '"') { // WARNING: " “ ” are not same!! print(“str”) is not valid in C!!!
                if (is_reading_string) { //this means we are at the end of a string
                    lexeme[i] = '\0';
                    tokens[token_count].type = "string";
                    tokens[token_count].value = strdup(lexeme); //duplicate lexeme, we will change lexeme later
                    token_count++;
                    i = 0;
                }
                is_reading_string = !is_reading_string;
                continue;
            }
            if (is_reading_string) {
                lexeme[i++] = c;
            }

            if (!is_reading_string) {
                if ((c >= 65 && c <= 91) || (c >= 97 && c <= 123) || (c >= 48 && c <= 57) || (c >= 44 && c <= 46)
                    || c == 93 || c == 125 || c == 32 || c == 9 || c == 10 || c == 95 || c == -1) {
                    //accept A-Z a-z 0-9 , - . [] {} space \t \n _ characters in source code

                    if (c == '[') {
                        tokens[token_count].type = "parenthesis";
                        tokens[token_count].value = "[";
                        tokens[token_count].line = line_count;
                        tokens[token_count].column = start_column;
                        token_count++;
                        continue;
                    } else if (c == ']') {
                        tokens[token_count].type = "parenthesis";
                        tokens[token_count].value = "]";
                        tokens[token_count].line = line_count;
                        tokens[token_count].column = start_column;
                        token_count++;
                        continue;
                    }

                    if (c != ' ' && c != '.' && c != '\t' && c != '\n' && c != ',' && c != EOF) {
                        if ((c < 48 && c != 45) || c > 57) // 45='-'
                            is_integer = false;

                        lexeme[i++] = c; //still reading a lexeme
                    } else {
                        if (i != 0) { //check if lexeme is not null/empty
                            lexeme[i] = '\0';
                            bool is_keyword = false;
                            for (int j = 0; j < 10; j++) { //for loop that checks if lexeme is a keyword
                                if (strcmp(lexeme, keywords[j]) == 0) { //string equality
                                    tokens[token_count].type = "keyword";
                                    tokens[token_count].value = strdup(lexeme);
                                    tokens[token_count].line = line_count;
                                    tokens[token_count].column = start_column;
                                    token_count++;
                                    is_keyword = true;
                                    break;
                                }
                            }

                            if (!is_keyword) {
                                if (is_integer) {
                                    tokens[token_count].type = "integer";
                                    tokens[token_count].value = strdup(lexeme);
                                    tokens[token_count].line = line_count;
                                    tokens[token_count].column = start_column;
                                    token_count++;
                                } else {
                                    tokens[token_count].type = "identifier";
                                    tokens[token_count].value = strdup(lexeme);
                                    tokens[token_count].line = line_count;
                                    tokens[token_count].column = start_column;
                                    token_count++;
                                }
                            }
                        }
                        if (c == '.') {
                            tokens[token_count].type = "eol";
                            tokens[token_count].value = "."; //not necessary, for printing purpose only
                            tokens[token_count].line = line_count;
                            tokens[token_count].column = start_column;
                            token_count++;
                        } else if (c == ',') {
                            tokens[token_count].type = "comma";
                            tokens[token_count].value = ","; //not necessary, for printing purpose only
                            tokens[token_count].line = line_count;
                            tokens[token_count].column = start_column;
                            token_count++;
                        }
                        i = 0;
                        is_integer = true; //reset

                    }
                } else {
                    printf("Unexpected character: %c in line %d, column %d", c, line_count, col);
                    return stop();
                }
            }
        }
        if (c == EOF)
            break;
    }
    fclose(source_code); //we are done with source file

    tokens[token_count].type = "end of file"; //just for error catching
    tokens[token_count].value = "EOF";
    tokens[token_count].line = line_count;
    tokens[token_count].column = eof_col;

    //now we got tokens[] array from lexical analyzer

    struct stack p_stack;
    p_stack.max = INITIAL_MAX;
    p_stack.elements = NULL;
    p_stack.top = -1;
    int open_count = 0, close_count = 0, last_open = 0;

    //initial validation of code
    //it catches wrong integer representations and maximum integer value,
    //  wrong identifier names,  parenthesis matching
    for (int l = 0; l < token_count; l++) {
        if (strcmp(tokens[l].type, "identifier") == 0) {
            if (strlen(tokens[l].value) > 20)
                return error(7, NULL, tokens[l]); //max identifier name lenght exceeded

            if (strstr(tokens[l].value, "-") != NULL) //if identifier contains -
                return error(5, NULL, tokens[l]);// not valid identifier

            if (tokens[l].value[0] >= 48 && tokens[l].value[0] <= 57 ) //if first char is number
                return error(9, NULL, tokens[l]);// not valid identifier

        } else if (strcmp(tokens[l].type, "integer") == 0) {
            int digit_limit;
            if (strstr(tokens[l].value, "-") != NULL) //if its a negative integer
                digit_limit = MAX_DIGIT + 1;
            else
                digit_limit = MAX_DIGIT;

            if (strlen(tokens[l].value) > digit_limit)
                return error(4, NULL, tokens[l]); //Max digit limit exceeded

            int dash_count = 0;
            char *temp = tokens[l].value;
            while (strstr(temp, "-") != NULL) {
                dash_count++;
                temp++;
            }
            if (dash_count > 1)
                return error(6, NULL, tokens[l]); //not valid integer

        } else if (strcmp(tokens[l].type, "parenthesis") == 0) {
            if (strcmp(tokens[l].value, "[") == 0) {
                push(&p_stack, '[');
                open_count++;
                last_open = l;
            } else if (strcmp(tokens[l].value, "]") == 0) {
                char temp = pop(&p_stack);
                if (temp != '[')
                    return error(1, "Expected open parenthesis before using a close parenthesis ", tokens[l]);
                close_count++;
            }
        }
    }
    if (open_count != close_count) {
        printf("Error: Expected a close parenthesis before end of file. Last open parenthesis is on line %d",
               tokens[last_open].line);
        return stop();
    }

    struct token l_vars[100];
    int l_starts[100] = {0}; //loop starting points
    int l_level = -1; //loop level, -1 means we are not in loop
    bool l_block[100] = {false}; //'true' if loop has code block, 'false' if it has one line code
    // included <stdbool.h> for this ^
    i = 0;

    //loop in tokens array. whole loop can be counted as parser
    //it interprets one line of code in every iteration!
    // used tokens[i + 1] or tokens[i + 2] for checking syntax
    // for ex: "move 5 to x." when we are on '5' token, checked next token if its 'to'.
    // then increase i by 2. because we wont do anything with 'to' token.
    while (i < token_count) {
        //start of loop is start of line of code.
        // so everytime we reach here we have to check what type of line of code are we going to read
        if (strcmp(tokens[i].type, "keyword") == 0 || strcmp(tokens[i].value, "]") == 0) {
            if (strcmp(tokens[i].value, "int") == 0) { //new integer declaration -> int x.
                i++;
                if (strcmp(tokens[i].type, "identifier") != 0)
                    return error(1, "Expected an identifier.", tokens[i]);

                if (strcmp(tokens[i + 1].type, "eol") != 0)
                    return error(1, "Expected an end of line character", tokens[i + 1]);

                //declaration syntax is correct
                //get() will return "not declared" if its not declared
                // so if it not returns "not declared" there is an error
                if (strcmp(get(tokens[i].value), "not declared") != 0)
                    return error(3, NULL, tokens[i]);

                symbol_table[symbol_count].name = tokens[i].value;
                symbol_table[symbol_count].value = "0";
                symbol_count++;

                i += 2; //nothing to do with eol
            } else if (strcmp(tokens[i].value, "move") == 0) { //assignment -> move y to x. or move 5 to x.
                i++;
                if (strcmp(tokens[i].type, "identifier") != 0 && strcmp(tokens[i].type, "integer") != 0)
                    return error(1, "Expected identifier or integer", tokens[i]);

                if (strcmp(tokens[i + 1].value, "to") != 0)
                    return error(1, "Expected keyword 'to'", tokens[i + 1]);

                if (strcmp(tokens[i + 2].type, "identifier") != 0)
                    return error(1, "Expected an identifier", tokens[i]);
                    //we can assign values to only identifiers

                if (strcmp(tokens[i + 3].type, "eol") != 0)
                    return error(1, "Expected an end of line character", tokens[i + 3]);

                //assignment syntax is correct
                char *new_val = valueof(tokens[i]);
                if (strcmp(new_val, "not declared") == 0)
                    return error(2, NULL, tokens[i]);

                int found = set(tokens[i + 2].value, new_val); //returns 0 if symbol not found
                if (!found)
                    return error(2, NULL, tokens[i + 2]);

                i += 4; // move x to y. we were on x. skipped "to", "y" and "."
            } else if (strcmp(tokens[i].value, "add") == 0) { //addition
                i++;
                if (strcmp(tokens[i].type, "identifier") != 0 && strcmp(tokens[i].type, "integer") != 0)
                    return error(1, "Expected identifier or integer", tokens[i]);

                if (strcmp(tokens[i + 1].value, "to") != 0)
                    return error(1, "Expected keyword 'to'", tokens[i + 1]);

                if (strcmp(tokens[i + 2].type, "identifier") != 0)
                    return error(1, "Expected an identifier", tokens[i + 2]);
                    // we have to assign to a variable

                if (strcmp(tokens[i + 3].type, "eol") != 0)
                    return error(1, "Expected an end of line character", tokens[i + 3]);
                //addition syntax is correct

                char *new_val = valueof(tokens[i]);
                if (strcmp(new_val, "not declared") == 0)
                    return error(2, NULL, tokens[i]);

                //target accepted! tokens[i + 2] is where to add
                char *old_val = get(tokens[i + 2].value);
                if (strcmp(old_val, "not declared") == 0)
                    return error(2, NULL, tokens[i + 2]);

                char *answer = add(old_val, new_val);
                if (strcmp(answer, "digit limit exceeded") == 0)
                    return error(4, NULL, tokens[i + 2]);

                set(tokens[i + 2].value, answer);

                i += 4; //add x to y. we were on x, skipped "to", "y" and "."
            } else if (strcmp(tokens[i].value, "sub") == 0) { //substraction
                i++;
                if (strcmp(tokens[i].type, "identifier") != 0 && strcmp(tokens[i].type, "integer") != 0)
                    return error(1, "Expected identifier or integer", tokens[i]);

                if (strcmp(tokens[i + 1].value, "from") != 0)
                    return error(1, "Expected keyword 'from'", tokens[i + 1]);

                if (strcmp(tokens[i + 2].type, "identifier") != 0)
                    return error(1, "Expected an identifier", tokens[i + 2]);
                    // we have to assign to a variable

                if (strcmp(tokens[i + 3].type, "eol") != 0)
                    return error(1, "Expected an end of line character", tokens[i + 3]);

                char *new_val = valueof(tokens[i]);
                if (strcmp(new_val, "not declared") == 0)
                    return error(2, NULL, tokens[i]);

                //target accepted! tokens[i + 2] is where to add
                char *old_val = get(tokens[i + 2].value);
                if (strcmp(old_val, "not declared") == 0)
                    return error(2, NULL, tokens[i + 2]);

                char *answer = sub(old_val, new_val);
                if (strcmp(answer, "digit limit exceeded") == 0)
                    return error(4, NULL, tokens[i + 2]);

                set(tokens[i + 2].value, answer);

                i += 4; //"sub x from y." we were on x, skipped "from", "y" and "."
            } else if (strcmp(tokens[i].value, "out") == 0) { //output
                i++;
                while (i < token_count) { //print everything till end of line
                    if (strcmp(tokens[i].type, "string") == 0) {
                        printf(tokens[i].value);
                    } else if (strcmp(tokens[i].type, "identifier") == 0) {
                        char *value = valueof(tokens[i]);
                        if (strcmp(value, "not declared") == 0)
                            return error(2, NULL, tokens[i]);

                        printf(value);
                    } else if (strcmp(tokens[i].value, "newline") == 0) {
                        printf("\n");
                    } else //its not printable
                        return error(1, "Expected string, identifier or 'newline' keyword", tokens[i]);

                    i++;

                    if (strcmp(tokens[i].type, "eol") == 0)
                        break;

                    //if we reached here, we will continue printing. check if theres a comma
                    if (strcmp(tokens[i].type, "comma") != 0)
                        return error(1, "Expected comma or end of line character", tokens[i]);

                    i++; //skipped comma
                }
                i++; //we were on end of line, check while loop condition
            } else if (strcmp(tokens[i].value, "loop") == 0) {
                i++;
                if (strcmp(tokens[i].type, "identifier") != 0 && strcmp(tokens[i].type, "integer") != 0)
                    return error(1, "Expected identifier or integer", tokens[i]);

                if (strcmp(tokens[i + 1].value, "times") != 0)
                    return error(1, "Expected keyword 'times'", tokens[i + 1]);

                char *loop_count = valueof(tokens[i]);
                if (compare(loop_count, "1") == -1)
                    return error(8, NULL, tokens[i]);

                l_level++;
                l_vars[l_level] = tokens[i];

                i += 2; // pass 'times' keyword
                if (strcmp(tokens[i].value, "[") == 0) {
                    i++; // nothing to do with '['
                    l_block[l_level] = true;
                } else if (strcmp(tokens[i].type, "keyword") != 0)
                    return error(1, "Expected open paranthesis or a keyword", tokens[i]);

                l_starts[l_level] = i;
                continue;
            }

            //interpreted a line of code, lets check if it was in loop
            if (l_level >= 0) {
                if (l_block[l_level]) { // we are in loop block
                    if (strcmp(tokens[i].value, "]") == 0) {
                        i++; //if its last iteration of loop, it will continue from next line
                        //this is not used if we keep looping
                    } else {
                        // we are going to interpret atlease one line in current loop
                        continue; //lets go to the next line of code
                    }
                }

                char *old_val = valueof(l_vars[l_level]);
                char *new_val = sub(old_val, "1");

                if (strcmp(l_vars[l_level].type, "identifier") == 0) {
                    set(l_vars[l_level].value, new_val);
                } else {
                    l_vars[l_level].value = new_val;
                }
                if (strcmp(new_val, "0") == 0) {
                    l_starts[l_level] = 0;
                    l_block[l_level] = false;
                    l_level--;
                } else {
                    i = l_starts[l_level]; // go back to start of that loop
                }
            }
        } else {
            //every line of code must start with keyword.
            return error(1, "Expected keyword", tokens[i]);
        }
    }

    printf("\n\nInterpreted succesfully! Press a key to exit...");
    fseek(stdin, 0, SEEK_END); //clear input buffer //thanks to https://stackoverflow.com/a/9750394
    getchar();
    return 0;
}

char *add(char *a, char *b) {
    if (strcmp(a, "0") == 0)
        return b;
    else if (strcmp(b, "0") == 0)
        return a;

    bool negative = false;
    if (a[0] == '-' && b[0] != '-') { // -3 + 5 == 5 - 3
        a = shiftstr(a, -1); //remove negative sign by shifting one char left
        return sub(b, a); //if a is negative, subtract a from b
    } else if (a[0] != '-' && b[0] == '-') { //3 + (-5) == 3 - 5
        b = shiftstr(b, -1);
        return sub(a, b); //if b is negative, subtract b from a
    } else if (a[0] == '-' && b[0] == '-')
        negative = true; //if both are negative, answer will be negative

    char result[MAX_DIGIT + 2], x[MAX_DIGIT + 2], y[MAX_DIGIT + 2];  // +1 character for '-', +1 for '\0'

    for (int k = strlen(a) - 1; k >= 0 ; k--) {
        x[strlen(a) -1 - k] = *(a + k);
    }
    x[strlen(a)] = '\0';

    for (int k = strlen(b) - 1; k >= 0; k--) {
        y[strlen(b) -1 - k] = *(b + k);
    }
    y[strlen(b)] = '\0';

    char carry = '0';
    int i = 0;
    bool x_ended = false, y_ended = false;
    for (;;) { // while(true)

        if (x[i] > 57 || x[i] < 48) { //check if its not a number. x="123", x[3], x[4] is not number
            x[i] = 48; //assign 0
            x[i + 1] = '\0';
            x_ended = true;
        }

        if (y[i] > 57 || y[i] < 48) { //check if its not a number again
            y[i] = 48;
            y[i + 1] = '\0';
            y_ended = true;
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
        if (i > MAX_DIGIT)
            return "digit limit exceeded";
    }

    if (carry == '1') {
        result[i++] = '1';
        if (i > MAX_DIGIT)
            return "digit limit exceeded";
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

char *sub(char *a, char *b) {
    if (strcmp(a, b) == 0)
        return "0";

    bool negative = false;
    if (a[0] != '-' && b[0] == '-') { // +x - (-y) == x + y
        b = shiftstr(b, -1); // shift left by one character, we dont need '-'
        return add(a, b); //"sub - from +" is equal to "add + to +"
    } else if (a[0] == '-' && b[0] != '-') { // -x - x = -x + (-x)
        b = shiftstr(b, 1); // 54 -> 054, shift for adding a space for negative sign
        b[0] = '-';
        return add(a, b); //if b is negative, subtract b from a
    } else if (a[0] == '-' && b[0] == '-') {
        if (strlen(a) > strlen(b)) {
            negative = true;
        } else if (strlen(a) == strlen(b) && strcmp(a, b) > 0) { //-5 - (-3) = -2
            negative = true;
        }
    } else if (strlen(a) < strlen(b)) { //3 - 55 = -52
        negative = true;
    } else if (strlen(b) == strlen(a) && strcmp(b, a) > 0) { // 3 - 5 = -2
        //do 5 - 3 first, then add a negative sign
        negative = true;
        char *temp = b;
        b = a;
        a = temp;
    }

    char result[MAX_DIGIT + 2], x[MAX_DIGIT + 2], y[MAX_DIGIT + 2];


    for (int k = strlen(a) - 1; k >= 0 ; k--) {
        x[strlen(a) -1 - k] = *(a + k);
    }
    x[strlen(a)] = '\0';
    for (int k = strlen(b) - 1; k >= 0; k--) {
        y[strlen(b) -1 - k] = *(b + k);
    }
    y[strlen(b)] = '\0';


    //i is current digit number
    int i = 0;
    bool x_ended = false, y_ended = false;
    while (true) {
        if (x[i] > 57 || x[i] < 48) {  //check if its not a number
            x[i] = '0'; //assign 0, it wont effect calculations
            x[i + 1] = '\0';
            x_ended = true;
        }
        if (y[i] > 57 || y[i] < 48) { //check if its not a number
            y[i] = '0';
            y[i + 1] = '\0';
            y_ended = true;
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
            while (x[j] == '0') { // assign 9 to all zeros, 500000 - 1?
                x[j] = '9';
                j++;
            }
            x[j] = (char) (x[j] - 1);

            //                  13         - 5        = 8
            //                  51   + 10 - 53   + 48 = 56
            result[i] = (char) (x[i] + 10 - y[i] + 48);
        }
        i++;
        if (i > MAX_DIGIT)
            return "digit limit exceeded";
    }

    result[i] = '\0';
    strrev(result);

    if (result[0] == '0' && strlen(result) > 1) { // 0002, lets remove zeros
        int zero_count = 0;
        for (int j = 0; j < strlen(result); j++) { // count how many unwanted zeros at the beginning
            if (result[j] == '0')
                zero_count++;
            else
                break;
        }
        for (int j = 0; j < strlen(result); j++) { // remove zeros by shifting
            result[j] = result[j + zero_count];
        }
    }
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
    if (n < 0) { // 05 -> 5 if n=-1 | 3456 -> 56 if n=-2
        str = str + abs(n);
        return str;
    } else if (n > 0) { // 4 -> 04 if n=1 | 23 -> 00023 if n=3
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
    } else
        return str;
}

int error(int type, char *info, struct token t) {
    system("cls");
    printf("Error on line %d column %d: ", t.line, t.column);
    if (type == 1) // expected ...
        printf("Unexpected %s '%s'. %s.", t.type, t.value, info);
    else if (type == 2)
        printf("'%s' is not declared before.", t.value);
    else if (type == 3)
        printf("'%s' is already declared before.", t.value);
    else if (type == 4)
        printf("Maximum value of an integer is exceeded.");
    else if (type == 5)
        printf("'%s' is not valid variable name. Only alphanumeric characters and underscores accepted.", t.value);
    else if (type == 6)
        printf("'%s' is not valid integer.", t.value); //--23
    else if (type == 7)
        printf("Maximum length of an identifier is exceeded.");
    else if (type == 8)
        printf("Loop variable '%s' must be greater than zero.", t.value);
    else if (type == 9)
        printf("'%s' is not valid variable name. It must start with an alphabetic character.", t.value);

    printf("\nPress enter to exit...");
    fseek(stdin, 0, SEEK_END);
    getchar();
    return -1;
}

int stop() {
    fseek(stdin, 0, SEEK_END);
    getchar();
    return -1;
}

char *valueof(struct token target) { //finds value of a token. it can be integer or identifier
    // if target token is identifier we already know its value
    if (strcmp(target.type, "integer") == 0)
        return target.value;
    else
        return get(target.value);
}

char *get(char *target_name) { //gets value of an identifier
    //traverse through symbol table and return value of target
    for (int j = 0; j < symbol_count; ++j) {
        if (strcmp(symbol_table[j].name, target_name) == 0) {
            return symbol_table[j].value;
        }
    }
    return "not declared";
}

int set(char *target_name, char *new_val) { //sets value of an identifier
    for (int j = 0; j < symbol_count; ++j) {
        if (strcmp(symbol_table[j].name, target_name) == 0) {
            symbol_table[j].value = new_val;
            return 1; //updated succesfully
        }
    }
    return 0; //error
}

void push(struct stack *st, char c) {
    if (st->elements == NULL) { //lets allocate some space
        st->elements = malloc(INITIAL_MAX * sizeof(char));
    } else if (st->top == st->max - 1) { //its full, lets reallocate some more space
        st->max *= 2; //double the capacity
        char *more_elements = realloc(st->elements, st->max * sizeof(char));
        st->elements = more_elements;
    }
    st->top++;
    st->elements[st->top] = c;
}

char pop(struct stack *st) {
    if ((st->max / 2) > (st->top + 10)) { //more than half of it is empty
        st->max /= 2;
        char *less_elements = malloc(st->max * sizeof(char));
        free(st->elements);
        st->elements = less_elements;
    }
    if (st->top == -1) {
        //stack is empty, return null
        return '\0';
    } else {
        char c = st->elements[st->top];
        st->elements[st->top] = '\0'; // for debugging purpose only
        st->top--;
        return c;
    }
}

/* compare(): compares two decimal numbers
 * returns -1 if a<b
 * returns  0 if a=b
 * returns  1 if a>b
 * */
int compare(char *a, char *b) {
    if (strcmp(a, b) == 0)
        return 0;

    if (a[0] == '-' && b[0] != '-') // -a  +b
        return -1;

    if (a[0] != '-' && b[0] == '-') // +a  -b
        return 1;

    if (a[0] == '-' && b[0] == '-') { // -a  -b
        if (strlen(a) == strlen(b)) { // -aaa -bbb
            if (strcmp(a, b) > 0)   //-5  -4
                return -1;
            else                    //-4  -5
                return 1;
        }

        if (strlen(a) > strlen(b))  // -10  -5
            return -1;
        else
            return 1;

    }

    //if we reached here, we know both is positive
    if (strlen(a) == strlen(b)) { //+aaa  +bbb
        if (strcmp(a, b) < 0) // +4  +5
            return -1;
        else                  // +5  +4
            return 1;
    }

    if (strlen(a) < strlen(b))  //+5  +10
        return -1;
    else
        return 1;

}