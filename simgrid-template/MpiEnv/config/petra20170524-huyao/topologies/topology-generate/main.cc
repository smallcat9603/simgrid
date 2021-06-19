#include<stdio.h>

int main(void){
	for (int i=0; i< 64; i++)
	for (int j=0; j< 64; j++)
		if(i != j)
			printf("%d %d\n",i,j);	
	return 0;
}

