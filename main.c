#include <stdio.h>
#include <string.h>

int main() {
    char filename[100];
    filename[0] = '\0';

    while(1) { //until a correct filename
        printf("Enter a file name: ");
        scanf(" %[^\n]s", filename); //to accept spaces too

        if (strstr(filename, ".") != NULL) {
            strrev(filename);
            if (filename[0] == 'a' && filename[1] == 'b' && filename[2] == '.') {
                strrev(filename);
                break;
            } else {
                printf("Error: Wrong extension!\n");
            }
        } else {
            int len = (int)strlen(filename);
            filename[len] = '.';
            filename[len + 1] = 'b';
            filename[len + 2] = 'a';
            filename[len + 3] = '\0';
            break;
        }
    }
    return 0;
}