
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
class CQueue1
{
	CList<T> First;
	
	CQueue1(CList<T> AFirst) : First(AFirst)
	{

	}

public:

	CQueue1()
	{

	}

	CQueue1 PopFront() const
	{
		return CQueue1(First.Tail());
	}

	T Front() const
	{
		return First.Front();
	}

	CQueue1 PushBack(T V) const
	{
		return CQueue1(First.Concat(CList<T>(V)));
	}

	bool IsEmpty() const
	{
		return First.IsEmpty();
	}

	CQueue1(initializer_list<T> L)
	{
		for (auto it = rbegin(L); it != rend(L); it++)
		{
			First = CList<T>(*it, First);
		};
	}

	string ToString() const
	{
		return First.ToString();
	}
};

template <class T>
class CQueue
{
	CList<T> First;
	CList<T> Second;

	CQueue(CList<T> AFirst, CList<T> ASecond) : First(AFirst), Second(ASecond)
	{

	}

	CQueue Check()
	{
		if (First.IsEmpty())
		{
			return CQueue(Second.Reverse(), First);
		}
		return *this;
	}

public:

	CQueue()
	{

	}

	CQueue PopFront() const
	{
		return CQueue(First.Tail(), Second).Check();
	}

	T Front() const
	{
		return First.Front();
	}

	CQueue PushBack(T V) const
	{
		return CQueue(First, Second.PushFront(V)).Check();
	}

	bool IsEmpty() const
	{
		return First.IsEmpty();
	}

	CQueue(initializer_list<T> L)
	{
		for (auto it = rbegin(L); it != rend(L); it++)
		{
			First = CList<T>(*it, First);
		};
	}

	string ToString() const
	{
		return First.ToString() + " / " + Second.ToString();
	}
};

template <template <class T> class CQueue>
class CQueueTester1
{
public:
	
	void PrintQueue(CQueue<int> Queue)
	{
		while (!Queue.IsEmpty())
		{
			//printf("\r\n%s\r\n", Queue.ToString().c_str());

			printf("%d ", Queue.Front());
			Queue = Queue.PopFront();
		}
		printf("\r\n");
	}

	void Test()
	{
		printf("Queue\r\n");

		CQueue<int> Queue1;

		Queue1 = Queue1.PushBack(1);
		Queue1 = Queue1.PushBack(2);
		Queue1 = Queue1.PushBack(3);

		PrintQueue(Queue1);

		CQueue<int> Queue({ 1,2,3 });

		PrintQueue(Queue);

		printf("\r\n");
	}
};

class CQueueTest
{
public:

	void Test()
	{
		CQueueTester1<CQueue1> Tester1;
		Tester1.Test();

		CQueueTester1<CQueue> Tester2;
		Tester2.Test();
	}
};