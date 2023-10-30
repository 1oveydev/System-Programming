#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


#define BUFFERSIZE 4096
#define COPYMODE 0644

void oops(char *, char *);
void oops2(char *, char *);

int main(int ac, char *av[])
{
	int in_fd, out_fd, n_chars;
	char buf[BUFFERSIZE];
	struct stat *buf1;

	if (ac != 3){
		fprintf( stderr, "usage : %s source destination\n", *av);
		exit(1);
	}

	if ( strcmp(av[1], av[2]) == 0 )
		oops2(av[1], av[2]);

	stat(av[2], buf1);

	if ( (in_fd=open(av[1], O_RDONLY)) == -1 )
		oops("Cannot open ", av[1]);

	if (S_ISDIR(buf1->st_mode)){
		char str_cp[] = "/";
		av[2] = strcat(av[2], str_cp);
		av[2] = strcat(av[2], av[1]);
	}

	if ( (out_fd=creat(av[2], COPYMODE)) == -1 )
		oops("Cannot create", av[2]);


	while( (n_chars = read(in_fd, buf, BUFFERSIZE)) > 0){
		if (write(out_fd, buf, n_chars) != n_chars)
			oops("Write error to :", av[2]);
	}

	if (n_chars == -1)
		oops("Read error from ", av[1]);

	if (close(in_fd) == -1 || close(out_fd) == -1)
		oops("Error closing files", "");

	return 0;
}

void oops(char *s1, char *s2)
{
	fprintf(stderr, "Error : %s ", s1);
	perror(s2);
	exit(1);
}

void oops2(char *s1, char *s2){
	fprintf(stderr, "Error : '%s' and '%s' is Same File. Cannot Copy. \n", s1, s2);
	exit(1);
}
