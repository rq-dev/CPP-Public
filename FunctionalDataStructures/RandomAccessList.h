
/*

Пример для курса ФИЛП, УрФУ, 2017-2018. Веретенников А. Б.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/	

template <class T>
class CRandomAccessList
{
	struct CNode;

	typedef shared_ptr<CNode> PNode;

	struct CNode
	{
		T Value;
		PNode Left, Right;

		CNode(PNode ALeft, PNode ARight): Left(ALeft), Right(ARight)
		{

		}

		CNode(T AValue): Value(AValue)
		{

		}
	};

	typedef CList<PNode> CNodeList;

	CNodeList List;

	CNodeList Add(CNodeList List, PNode Other) const
	{
		if (List.IsEmpty())
		{
			return List.PushFront(Other);
		}

		PNode Current = List.Front();
		if (Current)
		{
			return Add(List.Tail(), make_shared<CNode>(Other, Current)).PushFront(PNode());
		}
		else
		{
			return List.Tail().PushFront(Other);
		}
	}

	T LookupTree(size_t Position, PNode Current, size_t Index) const
	{
		size_t CurrentSize = 1 << Position;
		if (Position == 0)
		{
			return Current->Value;
		}
		CurrentSize >>= 1;
		if (Index < CurrentSize)
		{
			return LookupTree(Position - 1, Current->Left, Index);
		}
		else
		{
			return LookupTree(Position - 1, Current->Right, Index - CurrentSize);
		}
	}

	T Lookup(size_t Position, CNodeList List, size_t Index) const
	{
		if (List.IsEmpty())
		{
			return T();
		}

		PNode Current = List.Front();
		if (Current)
		{
			size_t CurrentSize = 1 << Position;
			if (Index < CurrentSize)
			{
				return LookupTree(Position, Current, Index);
			}
			else
			{
				return Lookup(Position + 1, List.Tail(), Index - CurrentSize);
			}
		}
		else
		{
			return Lookup(Position + 1, List.Tail(), Index);
		}
	}
	
	PNode UpdateTree(size_t Position, PNode Current, size_t Index, T Value) const
	{
		size_t CurrentSize = 1 << Position;
		if (Position == 0)
		{
			return make_shared<CNode>(Value);
		}
		CurrentSize >>= 1;
		if (Index < CurrentSize)
		{
			return make_shared<CNode>(UpdateTree(Position - 1, Current->Left, Index, Value), Current->Right);
		}
		else
		{
			return make_shared<CNode>(Current->Left, UpdateTree(Position - 1, Current->Right, Index - CurrentSize, Value));
		}
	}

	CNodeList Update(size_t Position, CNodeList List, size_t Index, T Value) const
	{
		PNode Current = List.Front();
		if (Current)
		{
			size_t CurrentSize = 1 << Position;
			if (Index < CurrentSize)
			{
				return List.Tail().PushFront(UpdateTree(Position, Current, Index, Value));
			}
			else
			{
				return Update(Position + 1, List.Tail(), Index - CurrentSize, Value).PushFront(Current);
			}
		}
		else
		{
			return Update(Position + 1, List.Tail(), Index, Value).PushFront(Current);
		}

	}

	CRandomAccessList(CNodeList AList) : List(AList)
	{

	}

	pair<PNode, CNodeList> Borrow(CNodeList List) const
	{
		PNode Front = List.Front();
		CNodeList Tail = List.Tail();
		if (Tail.IsEmpty())
		{
			if (Front)
			{
				return make_pair(Front, Tail);
			}
			else
			{
				//Error
				return make_pair(PNode(), Tail);
			}
		}

		if (Front) 
		{
			return make_pair(Front, Tail.PushFront(PNode()));
		}
		else
		{
			pair<PNode, CNodeList> Pair = Borrow(Tail);
			return make_pair(Pair.first->Left, Pair.second.PushFront(Pair.first->Right));
		}
	}

	size_t Count(size_t Position, CNodeList List) const
	{
		if (List.IsEmpty())
		{
			return 0;
		}
		PNode Front = List.Front();
		CNodeList Tail = List.Tail();
		size_t CurrentSize = 0;
		if (Front)
		{
			CurrentSize = 1 << Position;
		}
		return CurrentSize + Count(Position + 1, Tail);
	}

public:

	CRandomAccessList()
	{

	}

	CRandomAccessList PushFront(T V)
	{
		return CRandomAccessList(Add(List, make_shared<CNode>(V)));
	}

	T Lookup(size_t Index)
	{
		return Lookup(0, List, Index);
	}

	CRandomAccessList Update(size_t Index, T Value)
	{
		return CRandomAccessList(Update(0, List, Index, Value));
	}

	T Head()
	{
		if (List.IsEmpty())
		{
			return T();
		}
		
		pair<PNode, CNodeList> Pair = Borrow(List);
		return Pair.first->Value;
	}

	CRandomAccessList Tail()
	{
		pair<PNode, CNodeList> Pair = Borrow(List);
		return CRandomAccessList(Pair.second);
	}

	size_t Count() const
	{
		return Count(0, List);
	}

	bool IsEmpty() const
	{
		return List.IsEmpty();
	}

	CList<T> ToList() const
	{
		CList<T> Result;
		CRandomAccessList Current = *this;

		while (!Current.IsEmpty())
		{
			Result = Result.PushFront(Current.Head());
			Current = Current.Tail();
		}
		return Result.Reverse();
	}

	string ToString() const
	{
		return ToList().ToString();
	}
};

class CRandomAccessListTest
{
public:

	void Test()
	{
		printf("Random Access List\r\n");

		CRandomAccessList<int> List;
		List = List.PushFront(10);
		List = List.PushFront(7);
		List = List.PushFront(3);
		List = List.PushFront(12);
		List = List.PushFront(14);
		
		printf("\r\n");

		printf("%d\r\n", List.Lookup(0));
		printf("%d\r\n", List.Lookup(1));
		printf("%d\r\n", List.Lookup(2));
		printf("%d\r\n", List.Lookup(3));
		printf("%d\r\n", List.Lookup(4));

		printf("\r\n");

		List = List.Update(2, 15);

		printf("%d\r\n", List.Lookup(0));
		printf("%d\r\n", List.Lookup(1));
		printf("%d\r\n", List.Lookup(2));
		printf("%d\r\n", List.Lookup(3));
		printf("%d\r\n", List.Lookup(4));
		printf("%d\r\n", List.Lookup(5));
		printf("Count: %d\r\n", (int)List.Count());

		printf("\r\n");
		printf("%s\r\n",List.ToString().c_str());
		printf("\r\n");

		for (size_t i = 0; i < 5; i++)
		{
			printf("%d\r\n", List.Head());
			List = List.Tail();
		}
	}
};