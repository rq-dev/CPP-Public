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


class CSmartPtrTestObjectCollection
{
	intptr_t Objects = 0;

public:

	class CTestObject
	{
		int V;
		CSmartPtrTestObjectCollection* Owner;
	public:

		intptr_t Count = 0;

		CTestObject(const CTestObject& T)
		{
			V = T.V;
			Owner = T.Owner;
			Owner->Objects++;

			printf("Test object has been copied %d\r\n", V);
		}

		CTestObject& operator=(const CTestObject& T)
		{
			V = T.V;
			Owner = T.Owner;
			printf("Test object has been assignmented %d\r\n", V);
			return *this;
		}

		CTestObject(int T, CSmartPtrTestObjectCollection* AOwner) : V(T), Owner(AOwner)
		{
			printf("Test object has been created %d\r\n", V);

			Owner->Objects++;
		}

		~CTestObject()
		{
			Owner->Objects--;
			printf("Test object has been destroyed %d, objects %d\r\n", V, (int)Owner->Objects);
		}

		int Value() const
		{
			return V;
		}
	};
};


template <class CSmartPtr_>
class CSmartPtrTestObjectTester
{
	CSmartPtr_ f1(CSmartPtr_ V)
	{
		return V;
	}

	void f2(CSmartPtr_ V)
	{

	}

	typedef CSmartPtrTestObjectCollection::CTestObject CTestObject;

public:

	void Test(CSmartPtrTestObjectCollection* SmartPtrTestObjectCollection)
	{
		CSmartPtr_ Ptr(new CTestObject(7, SmartPtrTestObjectCollection));
		printf("Size of smart ptr: %d\r\n", sizeof(Ptr));
		CSmartPtr_ Ptr1 = Ptr;
		CSmartPtr_ Ptr2 = Ptr;
		CSmartPtr_ Ptr3 = f1(Ptr);
		f2(Ptr);

		int q = Ptr1->Value();
	}



};
