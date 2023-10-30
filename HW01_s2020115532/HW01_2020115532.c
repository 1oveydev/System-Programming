// 2020115532 컴퓨터학부 정서현 
// 시스템 프로그래밍 HW 01

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// --all, --human-readable 여부
int option_a = 0, option_h = 0;

long long printFirstFileUsageAndPath(char *, int, int);
long long printUsageAndPath(char *);
void getinfo(char *, long long, int);
long long sizeChange(long long);

int main(int argc, char *argv[]){
    // 디렉토리 정보를 담음
    struct dirent *dirent_p;
    DIR *dir_p;

    // 현재 파일의 이름
    char cur_name[BUFSIZ];
    // strncpy(cur_name, ".", BUFSIZ);

    dir_p = opendir(".");
    if (dir_p == NULL){
        perror(".");
        exit(1);
    }

    if (argc == 1){ // 인자로 아무것도 받지 않음 (ex: ./du)
        long long result = printUsageAndPath(".");
    }

    else{ 
        int checkPath = 0; // 1이면 인자에 경로 존재
        
        // 첫번째 반복문 : 인자에서 옵션과 경로 가려내기
        for (int i = 1; i < argc; i++){
            if (argv[i][0] == '-'){ // 옵션?
                for (int j = 1; j < (int)sizeof(argv[i]) ; j++){
                    if (argv[i][j] == 'a')
                        option_a = 1;
                    if (argv[i][j] == 'h')
                        option_h = 1;
                }
            }

            else
                checkPath = 1; 
        }

        if (checkPath == 1){
            for (int i = 1; i < argc; i++){
                if (argv[i][0] != '-'){ // 옵션이 아님
                    struct stat first;
                    char temp[BUFSIZ];
                    strncpy(temp, cur_name, BUFSIZ);
                    strncat(temp, argv[i], BUFSIZ);
                    stat(temp, &first);
                    
                    if (!S_ISDIR(first.st_mode)) // 처음부터 파일이 들어오면?
                        printFirstFileUsageAndPath(temp, option_a, option_h);
                    else
                        printUsageAndPath(temp);
                }
            }
        }
        else{
            printUsageAndPath(".");
        }
        
    }
    closedir(dir_p);
    return 0;
}


long long printFirstFileUsageAndPath(char* path, int a, int h){
    struct stat path_stat;

    if (stat(path, &path_stat) == -1){
        fprintf(stderr, "du: cannot access '%s'", path);
        perror(" ");
        exit(1);
    }

    long long size = path_stat.st_size;

    if (a == 1){
        if (h == 1)
            getinfo(path, size, 1);
        else
            getinfo(path, size, 0);
    }

    else{
        if (h == 1)
            getinfo(path, size, 1);
        else
            getinfo(path, size, 0);
    }

    return sizeChange(size);
}

long long printUsageAndPath(char* dirPath) {
    struct stat path_stat;
    if (stat(dirPath, &path_stat) != 0) {
        fprintf(stderr, "du: cannot access '%s'", dirPath);
        perror(" ");
        exit(1);
    }

    if (S_ISREG(path_stat.st_mode)) { // 파일인 경우
        long long size = path_stat.st_size;
        if (option_a == 1){
                if (option_h == 1)
                    getinfo(dirPath, size, 1);
                else
                    getinfo(dirPath, size, 0);
        }
        return sizeChange(size);
    } 

    else if (S_ISDIR(path_stat.st_mode)) { // 디렉토리인 경우
        DIR* dir_p;
        struct dirent *direntp;
        struct stat buf;
        char pathOfDir[BUFSIZ];

        dir_p = opendir(dirPath);
        if (dir_p == NULL) {
            perror(dirPath);
            exit(1);
        }

        long long sizeOfDir = 4096;
        while ((direntp = readdir(dir_p)) != NULL) {
            if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
                continue;

            strcpy(pathOfDir, dirPath);
            if (pathOfDir[strlen(pathOfDir)-1] != '/')
                strcat(pathOfDir, "/");
            strncat(pathOfDir, direntp->d_name, BUFSIZ);
            stat(pathOfDir, &buf);

            long long result = 0;

            result += sizeChange(printUsageAndPath(pathOfDir));

            if (result == -1) {
                fprintf(stderr, "Error: %s is not a valid directory or file.\n", pathOfDir);
            }

            sizeOfDir += result;
        }
        
        if (option_h == 1) {
            getinfo(dirPath, sizeOfDir, 1);
        } else {
            getinfo(dirPath, sizeOfDir, 0);
        }

        closedir(dir_p);
        return sizeChange(sizeOfDir);

    } else {
        fprintf(stderr, "Error: %s is not a valid directory or file.\n", dirPath);
        return -1; // 올바르지 않은 경로나 파일
    }
}


void getinfo(char * dirPath, long long sizeOfDir, int option_h){
    sizeOfDir = sizeChange(sizeOfDir);

    if (option_h){
        if (sizeOfDir >= (long long)10*1024*1024*1024)
            printf("%lldG\t%s\n", sizeOfDir/(1024*1024*1024), dirPath);
        else if (sizeOfDir >= 1024*1024*1024)
            printf("%.1fG\t%s\n", (double)(sizeOfDir/(1024*1024*1024)), dirPath);
        else if (sizeOfDir >= 10*1024*1024)
            printf("%lldM\t%s\n", sizeOfDir/(1024*1024), dirPath);
        else if (sizeOfDir >= 1024*1024)
            printf("%.1fM\t%s\n", (double)sizeOfDir/(1024*1024), dirPath);
        else if (sizeOfDir >= 10*1024)
            printf("%lldK\t%s\n", sizeOfDir/(1024), dirPath);
        else if (sizeOfDir >= 1024)
            printf("%.1fK\t%s\n", (double)sizeOfDir/(1024), dirPath);
        else if (sizeOfDir == 0)
            printf("%lld\t%s\n", sizeOfDir, dirPath);
        else
            printf("%lldB\t%s\n", sizeOfDir, dirPath);
    }
    else
        printf("%lld\t%s\n",sizeOfDir/(1024), dirPath);
}

long long sizeChange(long long sizeOfDir){
    if (sizeOfDir % 4096)
        return (sizeOfDir/4096 + 1) * 4096;
    
    else
        return sizeOfDir;
}