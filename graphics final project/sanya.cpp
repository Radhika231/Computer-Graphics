#include <stdio.h>
int i, j, k;

char reverse_string(char *str, int length)
{
    char code;
    for(i=0;i<length;i++){
       // if(*ptr == '\0')  break;
        str++;
    }
    length = i;
    //str--;
    char str2[length];
    printf("Reversed String: ");
    for(i=length; i>=0; i--){
        printf("%c",*str--);
        //str2[i]=ptr+1;
    }
    int j=0;
    for(k=length;k>=0;k--)
    {
        if(str2[k]==str++)
        {
            code='0';
            break;
        }
        else
        {
            code='1';
            break;
        }
        j++;
    }
    return 0;
    //if(str2==)
}

int main()
{
    
    int len;
    printf("Enter the string length");
    scanf("%d",&len);
    char str[len], *ptr;
    printf("Enter a string: ");
    gets(str);
    ptr = str;
    char return_code=reverse_string(ptr, len);
    printf("Final output code: %s", &return_code);


   /* for(i=0;i<len-1;i++){
       // if(*ptr == '\0')  break;
        ptr++;
    }
    len = i;
    ptr--;
    printf("Reversed String: ");
    for(i=len; i>0; i--){
        printf("%c",*ptr--);
    }*/
    return 0;
}
