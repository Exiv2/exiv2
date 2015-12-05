#include <stdio.h>
#include <windows.h>

int main(int,const char**)
{
	printf("%d sizeof(void*) = %d\n",_MSC_VER,sizeof(void*));
	return 0;
}
