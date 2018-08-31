#include <stdio.h>

int main(int,const char**)
{
	const char* vs[] = { "unknown" , "2005", "2008", "2010", "2012", "2013", "2015","2017" };
	int         v    =  (_MSC_VER-1300)/100;
	if (  v < 0 || v >= (sizeof(vs)/sizeof(vs[0])) ) v = 0;
	if (  v == 6 && _MSC_VER >= 1910  ) v++ ; // 2017 _MSC_VAR  == 1910!

	fprintf(stderr,"_%s = %d (%s) sizeof(void*) = %d\n","MSC_VER",_MSC_VER,vs[v],(int)sizeof(void*));
	return 0;
}
