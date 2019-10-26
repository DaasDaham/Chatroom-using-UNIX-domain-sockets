#include <stdio.h>
#include <string.h>

int main(void){
    char str[8] = "ab@ucef\0";
    char tmp[14] = "abcd";
    tmp[2] = 'f';
    int f = 6;
    char ff = f+'0';
    printf("%s\n", tmp);
}

void cpy_str(int ind, char a[], char tmp[]){
    for(int i=0;i<ind;i++){
        tmp[i] = a[i];
    }
}

int check_for_rate(char str[]){
    for (int i = 0; i < strlen(str)-1; i++)
    {
        if(str[i]=='@' && (str[i+1]=='u' || str[i+1]=='U')){
            return i;
        }
    }
    return -1;
}
