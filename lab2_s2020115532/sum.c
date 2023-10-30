#include <stdio.h>

int sum(int num){
	int result = 0;

	for (int i=1; i<=num; i++){
		result += i;
	}

	return result;

}

void main(){

	printf("The sum of 1 to 500 is %d \n", sum(500));

}
