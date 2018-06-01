
/*

Пример для курса ФИЛП, УрФУ, 2017-2018. Веретенников А. Б.

Copyright (c) 2018, Alexander B. Veretennikov
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once

#include <setjmp.h>

class CPointerEnumerator
{
public:

	virtual void EnumeratePointer(void **Ptr) = 0;
};

class CGCObject
{
public:

	virtual ~CGCObject()
	{

	}

	virtual void EnumeratePointers(CPointerEnumerator* Enumerator) = 0;

	virtual size_t GetObjectSize() = 0;

	virtual void* GetForwardAddress()
	{
		return 0;
	}
};


class CGCBase
{
protected:

	template <class T>
	struct CList
	{
		T* Item = 0;

		void Push(T* V)
		{
			V->Next = Item;
			Item = V;
		}

		T* Pop()
		{
			T* Result = Item;
			Item = Item->Next;
			return Result;
		}
	};

	template <class T>
	struct CCycleList
	{
		T* First = 0;

		void Clear()
		{
			First = 0;
		};

		void InsertBefore(T* Current, T* ItemForInsert)
		{
			Current->Prev->Next = ItemForInsert;
			ItemForInsert->Prev = Current->Prev;
			Current->Prev = ItemForInsert;
			ItemForInsert->Next = Current;
		}

		void Insert(T* Item)
		{
			if (First)
			{
				InsertBefore(First, Item);
			}
			else
			{
				First = Item;
				Item->Next = Item;
				Item->Prev = Item;
			}
		}

		void Remove(T* Current)
		{
			if (Current == Current->Next)
			{
				First = 0;
			}
			else
			{
				T* Prev = Current->Prev;
				T* Next = Current->Next;
				Prev->Next = Next;
				Next->Prev = Prev;

				if (Current == First)
				{
					First = Next;
				}
			}
		}

		template <class F>
		void ForEach(F f)
		{			
			T* Item = First;
			if (!Item)
				return;

			while (true)
			{
				f(Item);

				Item = Item->Next;
				if (Item == First)
					break;
			}
		}

		size_t Size()
		{
			size_t Result = 0;
			ForEach([&Result](T* Item) {
				Result++;
			});
			return Result;
		};
	};

	template <class CPage>
	struct CPageAllocator
	{
		CList<CPage> Free;

		size_t PageSize = 0;
		char* Start = 0;
		char* Buffer = 0;
		char* Current = 0;
		char* End = 0;

		size_t PagesUsed = 0;
		size_t PagesAllocated = 0;

		bool IsPointer(char* Ptr)
		{
			if ((Ptr >= Start) && (Ptr < Current))
				return true;
			return false;
		}

		~CPageAllocator()
		{
			if (Buffer)
			{
				VirtualFree(Buffer, 0, MEM_RELEASE);
			}
		}

		void Init(size_t BufferSize, size_t APageSize)
		{
			PageSize = APageSize;

			Buffer = (char*)VirtualAlloc(NULL, BufferSize, MEM_COMMIT, PAGE_READWRITE);
			End = Buffer + BufferSize;
			Current = Buffer;
			size_t Offset = ((uintptr_t)Current) % PageSize;
			if (Offset)
			{
				Current += PageSize - Offset;
			}
			Start = Current;
		}

		char* AllocPage()
		{
			char* Result;
			if (Free.Item)
			{				
				Result= (char*)Free.Pop();
			}
			else
			{
				Result = Current;
				char* NewCurrent = Current + PageSize;

				Error(NewCurrent > End);

				Current = NewCurrent;

				PagesAllocated++;
			}
			PagesUsed++;
			return Result;
		}

		void FreePage(CPage* Page)
		{
			PagesUsed--;
			Free.Push(Page);
		}

		void Dump()
		{
			printf("Start %d\r\n", (int)(Start - Buffer));
			printf("Allocated %d\r\n", (int)(Current - Start));
			printf("Pages used %d, allocated %d\r\n", 
				(int)(PagesUsed), (int)PagesAllocated);
		}
	};

	static void Error()
	{
		printf("Memory allocation error");
		TerminateProcess(GetCurrentProcess(), 1);
	}

	static void Error(bool Value)
	{
		if (Value)
			Error();
	}
	

	
};

/*
	Small blocks only.
	Simple Garbage Collector for a single threaded application.
*/
class CGC: public CGCBase, public CPointerEnumerator
{		
	size_t PageSize = 0;
	size_t BufferSize = 0;
		
	enum {Alignment = 16};

	struct CPage;
		
	enum {PageMark = 0xFF01F2FF};

	struct CPage
	{
		uint32_t PageMark;
		CPage* Next;
		CPage* Prev;
		size_t PageSize;
		size_t Count;
		bool Active;
		char* Data;
		char HeaderMark;	
	};

	CPage* GetPage(void* Item)
	{
		uintptr_t Ptr = (uintptr_t)Item;
		uintptr_t Offset = Ptr % PageSize;
		Ptr -= Offset;
		CPage* Result = (CPage*)Ptr;
		Error(Result->PageMark != PageMark);
		return Result;
	}
		
	CPageAllocator<CPage> PageAllocator;

	char* AllocPage()
	{
		return PageAllocator.AllocPage();
	}

	CCycleList<CPage> Pages;
	CCycleList<CPage> PrevPages;
	CCycleList<CPage> NewPages;
	CPage* CurrentPage = 0;
	char* Current = 0;
	char* End = 0;
	size_t PageIndex = 0;

	char* AllocItem(size_t Size)
	{
		if (Size >= PageSize)
		{
			printf("Cannot allocate item of size %d\r\n", (int)Size);
			Error();
		}

		size_t Available = End - Current;
		if (Available >= (Size + sizeof(size_t)))
		{
			//printf("Page list size %d\r\n", (int)Pages.Size());
			if (!CurrentPage->Active)
			{
				Error();
			}

			char* Result = Current;
			Current += Size;
			CurrentPage->Count++;
			return Result;
		}

		if (CurrentPage)
		{
			if (CurrentPage->Count == 0)
			{
				Error();
			}
		}
		
		char* PageBuffer = AllocPage();
		CPage* Page = InitPage(PageBuffer);
		Pages.Insert(Page);
		
		Current = Page->Data;
		End = PageBuffer + PageSize;
		CurrentPage = Page;
		return AllocItem(Size);
	}
		
	size_t Count = 0;
	size_t RootPages = 0;
	size_t AllocatedSpace;
	size_t AllocatedCount;
	size_t ObjectsFinalized = 0;

	size_t ShiftDown(size_t Value, size_t Base)
	{
		size_t Offset = Value % Base;
		Value -= Offset;
		return Value;
	}

	CPage* InitPage(char* PageBuffer)
	{
		CPage* Result = (CPage*)PageBuffer;
		Result->PageMark = PageMark;
		Result->PageSize = PageSize;
		Result->Active = true;
		Result->Count = 0;

		char* Data = &Result->HeaderMark;
		size_t Shift = ((Data - PageBuffer) % Alignment);
		if (Shift)
			Data += Alignment - Shift;

		Result->Data = Data;
		return Result;
	}
	
	void Init()
	{
	
	}

	void Mark(char* Address)
	{	
		char* Ptr = *(char**)Address;

		if (PageAllocator.IsPointer(Ptr))
		{
			CPage* Page = GetPage(Ptr);
			
			if (Ptr >= Page->Data)
			{
				if (!Page->Active)
				{
					Page->Active = true;
					PrevPages.Remove(Page);
					Pages.Insert(Page);
					RootPages++;
				}
			}
		}
	}
		
	void ClearCurrentPage()
	{
		CurrentPage = 0;
		Current = 0;
		End = 0;
	}

	void Mark()
	{
		printf("Allocated space %d, count %d\r\n",
			(int)AllocatedSpace, (int)AllocatedCount);

		Pages.ForEach([this](CPage* Page) {
			Page->Active = false;
			CheckPage(Page);
		});

		PageAllocator.Dump();

		PrevPages = Pages;
		Pages.Clear();
		NewPages.Clear();
		RootPages = 0;
		ClearCurrentPage();

		AllocatedCount = 0;
		AllocatedSpace = 0;
		ObjectsFinalized = 0;
		PageIndex = 0;

		MarkReg();
		MarkStack();

		printf("Root pages %d\r\n",(int)RootPages);

		Pages.ForEach([this](CPage* Page) {
			CheckAlloc(Page);
		});
	}

	void MarkBuffer(char* Buffer, size_t Size)
	{
		if (Size < sizeof(void*))
		{
			return;
		}
		for (size_t i = 0; i < (Size - sizeof(void*)); i++)
		{
			Mark(&Buffer[i]);
		}
	};

	void MarkReg()
	{
		jmp_buf b = { 0 };
		setjmp(b);
		char* t = (char*)b;

		MarkBuffer(t, sizeof(b));

		printf("Mark reg %d\r\n", (int)sizeof(b));
	}

	void MarkStack()
	{
		uintptr_t V;

#ifdef _WIN64
		char D[1024];
		V = (uintptr_t)&D[1];
#else
		//32 bit case
		int V1 = 0;
		_asm
		{
			mov V1, esp;
		}

		V = V1;
#endif
		size_t StackSize = 1024 * 1024;

		char* Ptr = (char*)V;
		size_t i = 0;
		__try
		{
			for (i = 0; i < StackSize; i++)
			{
				Mark(Ptr + i);
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		};

		printf("Mark stack %d, current esp %d\r\n", (int)i,(int)V);
	}


	void* Alloc(size_t Size)
	{				
		AllocatedCount++;
		AllocatedSpace += Size;

		Size = max(Size, sizeof(CMovedGCObject));

		return AllocItem(Size);
	}

	class CMovedGCObject : public CGCObject
	{
	public:

		void* ForwardAddress = 0;
		size_t ObjectSize;

		virtual void EnumeratePointers(CPointerEnumerator* Enumerator) override
		{
			
		}

		virtual size_t GetObjectSize() override
		{
			return ObjectSize;
		}

		virtual void* GetForwardAddress() override
		{
			return ForwardAddress;
		}

	};

	size_t GetObjectSize(CGCObject* Object)
	{
		return max(Object->GetObjectSize(), sizeof(CMovedGCObject));
	}

	void EnumeratePointer(void **PtrVar)
	{
		void* Ptr = *PtrVar;
		if (!Ptr)
			return;

		CPage* Page = GetPage(Ptr);
		/*
			Ссылка из рута на другой рут.
		*/
		if (Page->Active)
		{
			return;
		}

		CGCObject* Object = (CGCObject*)Ptr;
		char* NewPtr = (char*)Object->GetForwardAddress();
		
		if (!NewPtr)
		{
			size_t Size = GetObjectSize(Object);

			NewPtr = (char*)Alloc(Size);
			memcpy(NewPtr, Object, Size);

			CMovedGCObject* MovedGCObject = new (Ptr) CMovedGCObject();
			MovedGCObject->ForwardAddress = NewPtr;
			MovedGCObject->ObjectSize = Size;
		}	

		*PtrVar = NewPtr;
	}
	
	void Compact(CGCObject* Object, size_t Size)
	{
		Object->EnumeratePointers(this);
	}
	
	template <class F>
	void EnumObjects(CPage* Page, F f)
	{
		char* Buffer = Page->Data;
		size_t Size;
		size_t i = 0;
		while (i < Page->Count)
		{
			CGCObject* Object = (CGCObject*)Buffer;
			Size = GetObjectSize(Object);
			f(Object, Size);
			Buffer += Size;
			i++;
		}
	}

	void CheckPage(CPage* Page)
	{
		EnumObjects(Page, [Page, this](CGCObject* Object, size_t Size) {
			if (Size >= PageSize)
			{
				printf("Invalid object size %d, count %d\r\n",
					(int)Size, (int)Page->Count);
				Error();
			}
		});
	}

	void Compact(CPage* Page)
	{
		PageIndex++;

		EnumObjects(Page, [this](CGCObject* Object, size_t Size) {
			Compact(Object, Size);
		});
		
		//printf("Compact page \r\n");

		NewPages.Insert(Page);

		if (Page == CurrentPage)
		{
			printf("Reset current page %d\r\n",(int)Pages.Size());
			ClearCurrentPage();
		}
	}

	void CheckAlloc(CPage* Page)
	{
		EnumObjects(Page, [this](CGCObject* Object, size_t Size) {
			AllocatedCount++;
			AllocatedSpace += Size;
		});
	}

	void FinalizeObject(CGCObject* Object)
	{
		if (Object->GetForwardAddress() == 0)
		{
			Object->~CGCObject();
			ObjectsFinalized++;
		}
	}

	void FinalizePage(CPage* Page)
	{
		EnumObjects(Page, [this](CGCObject* Object, size_t Size) {
			FinalizeObject(Object);
		});
	}

	void Compact()
	{
		CPage* Page;
		while (Pages.First)
		{
			Page = Pages.First;
			Pages.Remove(Page);
			Compact(Page);
		}

		while (PrevPages.First)
		{
			Page = PrevPages.First;
			PrevPages.Remove(Page);
			FinalizePage(Page);
			PageAllocator.FreePage(Page);
		}

		Pages = NewPages;

		printf("Objects finalized %d, Allocated space %d, count %d\r\n", 
			(int)ObjectsFinalized, (int)AllocatedSpace, (int)AllocatedCount);

		PageAllocator.Dump();
	}

public:

	CGC(const CGC& GC) = delete;

	CGC& operator=(const CGC& GC) = delete;

	CGC()
	{
		PageSize = 4096;
		BufferSize = 1024 * 1024 * 64;
		PageAllocator.Init(BufferSize, PageSize);
		
		Init();
	}

	~CGC()
	{		
		
	}
	
	template <class T, class ... Args>
	T* Make(Args... args)
	{
		T* Result = (T*)Alloc(sizeof(T));
		new (Result) T(args...);
		return Result;
	}

	void GC()
	{
		//Scan Stack and Registers to Search Root.

		Mark();
		Compact();
	}
};

static CGC GC;
