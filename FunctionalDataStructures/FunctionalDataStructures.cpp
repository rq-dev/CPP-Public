// FunctionalDataStructures.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include <vector>

using namespace std;

#include "List.h"
#include "Queue.h"
#include "RandomAccessList.h"
#include "BinominalHeap.h"
#include "BinaryNumber.h"

int main()
{
	CListTest Test;
	Test.Test();

	CQueueTest QueueTest;
	QueueTest.Test();

	CRandomAccessListTest RandomAccessListTest;
	RandomAccessListTest.Test();

	CBinominalHeapTest BinominalHeapTest;
	BinominalHeapTest.Test();

	CBinaryNumberTest BinaryNumberTest;
	BinaryNumberTest.Test();

//	getchar();
    return 0;
}

