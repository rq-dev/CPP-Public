// TextMap.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <map>

using namespace std;

class CStringComparator
{
public:

	/*
		A < B --> true
	*/
	bool operator()(const char* A, const char* B) const
	{
		while (true)
		{
			if (A[0] == B[0])
			{
				//A = B
				if (!A[0])
					return false;

				A++;
				B++;
			}
			else
			{
				return A[0] < B[0];
			}
		}
	}
};

template <class T>
class CMyAllocator
{
public:
	typedef typename T value_type;
	
	CMyAllocator()
	{

	}

	template <class U>
	CMyAllocator(const CMyAllocator<U> &V)
	{

	}

	T* allocate(size_t Count)
	{
		//printf("Allocate %d\n", (int)(Count * sizeof(T)));

		return (T*)malloc(sizeof(T) * Count);
	}

	void deallocate(T* V, size_t Count)
	{
		//printf("Free %d\n", (int)(Count * sizeof(T)));

		free(V);
	}
};

void TextMapTest()
{
	map<char*, size_t, CStringComparator, CMyAllocator<char*>> Map;

	char* Words[] = { "Who", "Are", "You", "Who" };
	char* Word;

	for (size_t i = 0; i < sizeof(Words) / sizeof(Words[0]); i++)
	{
		Word = Words[i];
		auto It = Map.find(Word);
		if (It == Map.end())
		{
			Map.insert(make_pair(Word, 1));
		}
		else
		{
			It->second++;
		}
	}

	for (auto Entry : Map)
	{
		printf("Word %s, count %I64d\n", Entry.first, (uint64_t)Entry.second);
	}

}

#include <windows.h>

ULONGLONG GetCurrentTimeMs()
{
	SYSTEMTIME S = { 0 };
	FILETIME F = { 0 };
	GetSystemTime(&S);
	SystemTimeToFileTime(&S, &F);
	LARGE_INTEGER Int;
	Int.HighPart = F.dwHighDateTime;
	Int.LowPart = F.dwLowDateTime;
	return Int.QuadPart / 10000;
}


int main()
{
	ULONGLONG Start = GetCurrentTimeMs();

	TextMapTest();
	
	ULONGLONG End = GetCurrentTimeMs();
	
	printf("Time (ms) %d\n", (int)(End - Start));

	getchar();

	//C++, where is your LinkedHashMap?
    return 0;
}

