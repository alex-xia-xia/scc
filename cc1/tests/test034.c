
/*
name: TEST034
description: Basic test for incomplete structures
error:
test034.c:44: error: declared variable 'bar' of incomplete type
test034.c:44: error: redeclaration of 'bar'
output:
X3	S2	"x
G6	F	"main
{
\
X7	S2	"x
	h	X7	'P	#P0	!I
}
G5	F	"foo
{
\
	X3	M9	.I	#I0	:I
	h	X3	M9	.I
}
X13	S11	"bar2
*/

extern struct X x;
int foo();

int main()
{
	extern struct X x;
	return &x != 0;
}

struct X {int v;};

int foo()
{
	x.v = 0;
	return x.v;
}

typedef struct bar bar;
extern bar bar2;
bar bar;
