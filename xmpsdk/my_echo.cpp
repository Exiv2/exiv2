#include <stdio.h>
int main(int argc,const char** argv)
{
    int args = argc;
	while ( --argc ) {
		printf("%s",argv[args-argc]);
		if ( argc > 1 ) printf(" ");
	}
	return 0;
}