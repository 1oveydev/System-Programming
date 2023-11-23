#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smsh.h"

char *next_cmd(char *prompt, FILE *fp){
    char *buf;
    int bufspace = BUFSIZ;
    int pos=0;
    int c;

    // 항상 프롬프트 표시
    printf("%s", prompt);

    buf = emalloc(bufspace);

    while ((c=getc(fp))!=EOF){
        if(pos+1>=bufspace){
            buf=erealloc(buf, bufspace+BUFSIZ);
            bufspace+=BUFSIZ;
        }

        if(c == '\n' || c == ';')
            break;

        buf[pos++]=c;
    }
    
    buf[pos]='\0';

    if(c==EOF && pos==0){
        printf("\n");
        free(buf);
        return next_cmd(prompt, fp);
    }

    return buf;
}

#define is_delim(x) ((x)==' '||(x)=='\t')

char **splitline(char *line){
    char *newstr();
    char **args;
    int spots=0;
    int bufspace=0;
    int argnum=0;
    char *cp=line;
    char *start;
    int len;
    
    if(line==NULL)
        return NULL;
    
    args=emalloc(BUFSIZ);
    bufspace=BUFSIZ;
    spots=BUFSIZ/sizeof(char *);
    args=emalloc(BUFSIZ);
    bufspace=BUFSIZ;
    spots=BUFSIZ/sizeof(char *);

    while (*cp!='\0'){
        while (is_delim(*cp))
            cp++;
        if(*cp=='\0')
            break;
        
        if(argnum+1>=spots){
            args=erealloc(args, bufspace+BUFSIZ);
            bufspace+=BUFSIZ;
            spots+=(BUFSIZ/sizeof(char *));
        }
        start=cp;
        len=1;
        while (*++cp!='\0'&&!(is_delim(*cp)))
            len++;
        args[argnum++]=newstr(start, len);
    }
    args[argnum]=NULL;
    return args;
}

char *newstr(char *s, int l){
    char *rv=emalloc(l+1);
    
    rv[l]='\0';
    strncpy(rv, s, l);
    return rv;
}

void freelist(char **list){
    char **cp=list;
    while (*cp)
        free(*cp++);
    free(list);
}

void *emalloc(size_t n){
    void *rv;
    if((rv=malloc(n))==NULL)
        fatal("out of memory", "", 1);
    return rv;
}

void *erealloc(void *p, size_t n){
    void *rv;
    if ((rv=realloc(p,n))==NULL)
        fatal("realloc() failed", "", 1);
    return rv;
}
