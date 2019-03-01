// VariablesInMemory.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/**
_alloca <malloc.h>

The _alloca routine returns a void pointer to the allocated space,
which is guaranteed to be suitably aligned for storage of any type of object.
If size is 0, _alloca allocates a zero-length item and returns a valid pointer to that item.

A stack overflow exception is generated if the space cannot be allocated.
The stack overflow exception is not a C++ exception; it is a structured exception.
Instead of using C++ exception handling, you must use Structured Exception Handling (SEH).

*/
#include <malloc.h>

#include <Windows.h>

int GlobalInt = -1;

typedef int(*CMainPtr)();

int main()
{	
	int c;
	char* ch;
	char word[100];

	printf("Local: %d\r\n", (int)&c);
	printf("Global: %d\r\n", (int)&GlobalInt);
	printf("Local array element: %d\r\n", (int)&word[1]);

	ch = (char*)alloca(10);
	printf("alloca: %d\r\n", (int)&ch[1]);

	ch = new char[10];
	printf("new char: %d\r\n", (int)&ch[1]);

	delete[] ch;

	ch = (char*)malloc(10);
	printf("malloc: %d\r\n", (int)&ch[1]);

	free(ch);

	ch = (char*)VirtualAlloc(0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	printf("VirtualAlloc: %d\r\n", (int)&ch[1]);

	VirtualFree(ch, 0, MEM_RELEASE);

	CMainPtr MainPtr = main;
	printf("Main: %d\r\n", (int)MainPtr);

	int V = 0;
	int V2;
	_asm
	{
		mov V, esp;
		mov V2, ebp;
	}
	
	printf("ESP %d\r\n", V);
	printf("EBP %d\r\n", V2);

    return 0;
}

