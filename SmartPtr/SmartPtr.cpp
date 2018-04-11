// SmartPtr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "SmartPtrTestObjectCollection.h"

namespace SimpleSmartPtr {

	#include "SimpleSmartPtr.h"

}

namespace SimpleSmartPtr2 {

	#include "SimpleSmartPtr2.h"

}
namespace SimpleSmartPtrWithStrategy {

	#include "SimpleSmartPtrWithStrategy.h"

}

namespace SimplePtr
{
	#include "SimplePtr.h"
}

#include "SmartPtr.h"

#include "StandardSmartPtr.h"

int main()
{
	CSmartPtrDetailedTester SmartPtrTester;
	SmartPtrTester.Test();

	printf("\r\nSimple Smart Pointer Test\r\n\r\n");
	SimpleSmartPtr::CSimpleSmartPtrTester SimpleSmartPtrTester;
	SimpleSmartPtrTester.Test();
	printf("\r\n");

	printf("\r\nSimple Smart Pointer/Strategy Test\r\n\r\n");
	SimpleSmartPtrWithStrategy::CSimpleSmartPtrTester SimpleSmartPtrTester1;
	SimpleSmartPtrTester1.Test();
	printf("\r\n");

	CStandardSmartPtrTester StandardSmartPtrTester;
	StandardSmartPtrTester.Test();
	
    return 0;
}

