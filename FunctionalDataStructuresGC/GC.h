
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
	
	class CFinalizer
	{
	public:

		virtual void Execute() = 0;
	};

	template <class T>
	class CFinalizerT : public CFinalizer
	{
		T* Item = 0;
	public:

		CFinalizerT()
		{

		}

		void Init(T* AItem)
		{
			Item = AItem;
		}

		virtual void Execute()
		{
			if (Item)
				Item->~T();
		}
	};
};

/*
	Small blocks only.
	Simple Garbage Collector for a single threaded application.
*/
class CGC: public CGCBase
{
	

	struct CBitmap
	{
		char* Ptr;

		void Set(size_t Index, bool V)
		{
			size_t BitIndex = Index & 7;
			size_t ByteIndex = Index >> 3;
			char Byte = Ptr[ByteIndex];
			if (V)
				Byte |= 1 << BitIndex; else
				Byte &= ~(1 << BitIndex);
			Ptr[ByteIndex] = Byte;
		}

		bool SetEx(size_t Index, bool V)
		{
			size_t BitIndex = Index & 7;
			size_t ByteIndex = Index >> 3;
			char Byte = Ptr[ByteIndex];
			if (V)
				Byte |= 1 << BitIndex; else
				Byte &= ~(1 << BitIndex);
			bool R = Ptr[ByteIndex] != Byte;
			Ptr[ByteIndex] = Byte;
			return R;
		}

		bool Get(size_t Index)
		{
			size_t BitIndex = Index & 7;
			size_t ByteIndex = Index >> 3;
			char Byte = Ptr[ByteIndex];
			return (Byte & (1 << BitIndex)) != 0;
		}

		void Clear(size_t ByteSize)
		{
			memset(Ptr, 0, ByteSize);
		}

		void SetAll(size_t ByteSize)
		{
			for (size_t i = 0; i < ByteSize; i++)
				Ptr[i] = (char)(unsigned char)0xFF;
		}

		bool operator[](size_t Index)
		{
			return Get(Index);
		}
	};

	size_t PageSize = 0;
	size_t BufferSize = 0;
		
	enum {Alignment = 16};

	struct CPage;

	struct CItem
	{
		CItem* Next;
	};
		
	enum {PageMark = 0xFF01F2FF};

	struct CPage
	{
		uint32_t PageMark;
		CPage* Next;
		CPage* Prev;
		size_t PageSize;
		size_t ItemSize;
		size_t ItemCount;
		size_t BitmapByteSize;
		size_t UsedItems;
		CList<CItem> Free;
		CBitmap FreeBitmap;
		CBitmap MarkBitmap;
		CBitmap FinalizerBitmap;
		char* Data;
		char HeaderMark;
				
		CItem* GetItem(size_t i)
		{
			return (CItem*)(Data + (ItemSize * i));
		}

		size_t GetIndex(CItem* Item)
		{
			char* Ptr = (char*)Item;
			size_t Delta = Ptr - Data;
			return Delta / ItemSize;
		}
	};

	CPage* GetPage(CItem* Item)
	{
		uintptr_t Ptr = (uintptr_t)Item;
		uintptr_t Offset = Ptr % PageSize;
		Ptr -= Offset;
		CPage* Result = (CPage*)Ptr;
		Error(Result->PageMark != PageMark);
		return Result;
	}

	CPage* GetPage(char* Ptr)
	{
		return GetPage((CItem*)Ptr);
	}
	
	CPageAllocator<CPage> PageAllocator;

	char* AllocPage()
	{
		return PageAllocator.AllocPage();
	}

	struct CPageList
	{
		CCycleList<CPage> Pages;
		CCycleList<CPage> FullPages;
		size_t ItemSize;
	};

	CItem* AllocItem(CPageList* List)
	{
		if (!List->Pages.First)
		{
			char* PageBuffer = AllocPage();
			CPage* Page = InitPage(PageBuffer, List->ItemSize);
			List->Pages.Insert(Page);
		}
		
		CItem* Result = List->Pages.First->Free.Pop();
		CPage* Page = GetPage(Result);
		Page->UsedItems++;
		if (Page->UsedItems == Page->ItemCount)
		{
			List->Pages.Remove(Page);
			List->FullPages.Insert(Page);
		}
		return Result;
	}

	CItem* Alloc(CPageList* List)
	{
		CItem* Item = AllocItem(List);
		CPage* Page = GetPage(Item);
		size_t ItemIndex = Page->GetIndex(Item);

		Error(!Page->FreeBitmap.Get(ItemIndex));

		Page->FreeBitmap.Set(ItemIndex, false);
		return Item;
	}

	void Free(CItem* Item)
	{
		CPage* Page = GetPage(Item);
		size_t ItemIndex = Page->GetIndex(Item);

		Error(Page->FreeBitmap.Get(ItemIndex));
		
		Page->FreeBitmap.Set(ItemIndex, true);		
		if (Page->FinalizerBitmap.Get(ItemIndex))
		{
			CFinalizer* Finalizer = (CFinalizer*)((char*)Item + Page->ItemSize - sizeof(CFinalizerT<int>));
			Finalizer->Execute();
			Page->FinalizerBitmap.Set(ItemIndex, false);
		}
		Page->UsedItems--;
		Page->Free.Push(Item);

		//AllocatedSize -= Size;
		AllocatedCount--;
		AllocatedSpace -= Page->ItemSize;
	}

	CPageList* Lists = 0;
	size_t Count = 0;
	size_t MarkCount = 0;
	size_t MarkFreeCount = 0;
	size_t MarkSize = 0;
	size_t FreeCount = 0;
	size_t FreeSize = 0;
	size_t AllocatedSize = 0;
	size_t AllocatedSpace = 0;
	size_t AllocatedCount = 0;

	size_t ShiftDown(size_t Value, size_t Base)
	{
		size_t Offset = Value % Base;
		Value -= Offset;
		return Value;
	}

	CPage* InitPage(char* PageBuffer, size_t ItemSize)
	{
		size_t MaxItems = PageSize / ItemSize;
		size_t BitmapByteSize = (MaxItems / 8) + 1;

		CPage* Result = (CPage*)PageBuffer;
		Result->PageMark = PageMark;
		Result->PageSize = PageSize;
		Result->ItemSize = ItemSize;
		
		Result->FreeBitmap.Ptr = (&Result->HeaderMark);
		Result->MarkBitmap.Ptr = Result->FreeBitmap.Ptr + BitmapByteSize;
		Result->FinalizerBitmap.Ptr = Result->MarkBitmap.Ptr + BitmapByteSize;

		Result->BitmapByteSize = BitmapByteSize;
		Result->UsedItems = 0;
		char* Data = Result->FinalizerBitmap.Ptr + BitmapByteSize;
		size_t Shift = ((Data - PageBuffer) % Alignment);
		if (Shift)
			Data += Alignment - Shift;
		Result->Data = Data;
		size_t HeaderSize = Data - PageBuffer;
		Result->ItemCount = (PageSize - HeaderSize) / ItemSize;

		Result->MarkBitmap.Clear(BitmapByteSize);
		Result->FreeBitmap.SetAll(BitmapByteSize);
		Result->FinalizerBitmap.Clear(BitmapByteSize);

		for (size_t i = 0; i < Result->ItemCount; i++)
		{
			Result->Free.Push(Result->GetItem(i));
		}
		return Result;
	}

	template<class T>
	void SetFinalizer(T* Result)
	{
		CItem* Item = (CItem*)Result;
		CPage* Page = GetPage(Item);
		size_t Index = Page->GetIndex(Item);

		char* Ptr = ((char*)Item) + Page->ItemSize - sizeof(CFinalizerT<T>);
		new (Ptr) CFinalizerT<T>();

		CFinalizerT<T>* Finalizer = (CFinalizerT<T>*)Ptr;
		Finalizer->Init(Result);

		Page->FinalizerBitmap.Set(Index, true);
	};

	void Init()
	{
		size_t Level = 0;
		size_t Size;
		while (true)
		{
			Size = 1 << Level;
			if (Size > (PageSize / 2))
				break;
			Level++;
		}
		
		Count = Level;
		Lists = new CPageList[Count];
		for (size_t i = 0; i < Count; i++)
			Lists[i].ItemSize = 1 << i;
	}

	void Mark(char* Address)
	{
		char* Ptr = *(char**)Address;

		if (PageAllocator.IsPointer(Ptr))
		{
			CPage* Page = GetPage(Ptr);
			
			if (Ptr >= Page->Data)
			{
				size_t Delta = Ptr - Page->Data;
				Delta = ShiftDown(Delta, Page->ItemSize);
				size_t ItemIndex = Delta / Page->ItemSize;
				if (ItemIndex < Page->ItemCount)
				{
					if (Page->FreeBitmap.Get(ItemIndex))
					{
						MarkFreeCount++;
					} else
					if (!Page->MarkBitmap.Get(ItemIndex))
					{
						Page->MarkBitmap.Set(ItemIndex, true);
						MarkCount++;
						MarkSize += Page->ItemSize;

						MarkBuffer((char*)Page->GetItem(ItemIndex), Page->ItemSize);
					}
				}
			}
		}
	}

	void ClearMarks(CCycleList<CPage> PageList)
	{
		PageList.ForEach([](CPage* Page) {
			Page->MarkBitmap.Clear(Page->BitmapByteSize);
		});
	}

	void ClearMarks(CPageList* PageList)
	{
		ClearMarks(PageList->Pages);
		ClearMarks(PageList->FullPages);
	}

	void ClearMarks()
	{
		MarkCount = 0;
		MarkSize = 0;
		MarkFreeCount = 0;

		FreeCount = 0;
		FreeSize = 0;

		for (size_t i = 0; i < Count; i++)
			ClearMarks(&Lists[i]);
	}

	void Sweep(CPageList* PageList)
	{
		CCycleList<CPage> NewPages;
		CCycleList<CPage> NewFullPages;

		Sweep(PageList->Pages, &NewPages, &NewFullPages);
		Sweep(PageList->FullPages, &NewPages, &NewFullPages);
		
		PageList->Pages = NewPages;
		PageList->FullPages = NewFullPages;
	}

	void Sweep(CCycleList<CPage> List, CCycleList<CPage>* Pages, CCycleList<CPage>* NewFullPages)
	{
		CPage* Page;
		while (List.First)
		{
			Page = List.First;
			List.Remove(Page);

			for (size_t i = 0; i < Page->ItemCount; i++)
			{
				if (!Page->MarkBitmap[i])
				{
					if (!Page->FreeBitmap[i])
					{
						Free(Page->GetItem(i));
						FreeCount++;
						FreeSize += Page->ItemSize;
					}
				}
			}

			if (Page->UsedItems == 0)
			{
				PageAllocator.FreePage(Page);
			}
			else
			if (Page->ItemCount == Page->UsedItems)
			{
				NewFullPages->Insert(Page);
			}
			else
			{
				Pages->Insert(Page);
			}
		}
	}

	void Sweep()
	{
		for (size_t i = 0; i < Count; i++)
			Sweep(&Lists[i]);

		printf("Sweep free size %d, count %d, allocated space %d, allocated count %d\r\n", 
			(int)FreeSize, (int)FreeCount, (int)AllocatedSpace, (int)AllocatedCount);
	}

	void Mark()
	{
		ClearMarks();

		PageAllocator.Dump();

		MarkReg();
		MarkStack();

		printf("Marked size %d, count %d, mark free count %d\r\n", 
			(int)MarkSize, (int)MarkCount, (int)MarkFreeCount);
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

		printf("Allocated space %d, count %d\r\n",
			(int)AllocatedSpace, (int)AllocatedCount);
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
		delete[] Lists;
	}

	void* Alloc(size_t Size)
	{
		Size = max(Size, sizeof(CItem*));

		DWORD Index = 0;
		BitScanReverse(&Index, Size);
		if (Size != (1 << Index))
		{
			Index++;
		}
		CItem* Item = Alloc(&Lists[Index]);

		//AllocatedSize += Size;
		AllocatedCount++;
		AllocatedSpace+= 1 << Index;

		return (char*)Item;
	}
	
	template <class T, class ... Args>
	T* Make(Args... args)
	{
		T* Result = (T*)Alloc(sizeof(T) + sizeof(CFinalizerT<T>));
		new (Result) T(args...);
		SetFinalizer(Result);
		return Result;
	}

	void GC()
	{
		//Scan Stack and Registers to Search Root.

		Mark();
		Sweep();
	}
};

static CGC GC;

void* GC_Alloc(size_t Size)
{
	return (void*)GC.Alloc(Size);
}


template <class T>
class CGCAllocator
{
public:
	typedef typename T value_type;

	CGCAllocator()
	{

	}

	template <class U>
	CGCAllocator(const CGCAllocator<U> &V)
	{

	}

	T* allocate(size_t Count)
	{
		return (T*)GC_Alloc(sizeof(T) * Count);
	}

	void deallocate(T* V, size_t Count)
	{
		//
	}

	template <class U>
	bool operator ==(const CGCAllocator<U> &V) const
	{
		return this == &V;
	}

	template <class U>
	bool operator !=(const CGCAllocator<U> &V) const
	{
		return this != &V;
	}
};

