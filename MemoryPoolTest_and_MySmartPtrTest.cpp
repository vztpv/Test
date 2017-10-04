
#define _CRT_SECURE_NO_WARNINGS

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
using namespace std;

#include <wiz/global.h> // my own header file
#include <wiz/wizardError.h> // my own header file
#include <wiz/NEWARRAYS.H> // my own header file

namespace wiz {
	template<class T>
	class Pool
	{
	private:
		int num = 0;
		int last = 0;
		char* buffer;
	private:
		Pool(const Pool&) = delete;
		Pool& operator=(const Pool&) = delete;
	public:
		Pool(const int size)
		{
			buffer = (char*)malloc(size);
		}
		virtual ~Pool() 
		{
			free(buffer);
		}
	public:
		T* Allocate(int n = 1) {
			num++;
			last += sizeof(T) * n;
			return (T*)(buffer + last - (sizeof(T) * n));
		}
		void DeAllocate(T* ptr) {
			num--;
			if (0 == num) {
				last = 0;
			}
		}
		void Clear() {
			num = 0;
			last = 0;
		}
		T* ReAllocate(T* source, T* destination, int n) {
			memcpy(destination, source, n);
			return destination;
		}
		T* Buffer() {
			return buffer;
		}
	};
}


namespace wiz {
	class QueueEmptyError : public wiz::Error
	{
	public:
		QueueEmptyError() : wiz::Error("queueEmptyError")
		{

		}
	};
	class QueueFullError : public wiz::Error
	{
	public:
		QueueFullError() : wiz::Error("queueFullError")
		{

		}
	};

	// check array Queue, array stack?
	template <typename T>
	class Queue //: public wizObject
	{
	public:
		class Element
		{
		public:
			Element* next;
			T data;
		public:
			explicit Element(const T&d = T())
			{
				data = d;
				next = nullptr;
			}
		};
	private:
		Element* Head; //
		Element* Rear; //
		Pool<Element>* pool;
	public:
		explicit Queue(Pool<Element> * pool) : Head(nullptr), Rear(nullptr), pool(pool) { Head = new Element(); Rear = Head; }

		virtual ~Queue() {
			clear();
			delete Head; Head = Rear = nullptr;
		}
		void clear()
		{
			pool->Clear();
			Rear = Head;
			//while (!isEmpty()) {
			//	deleteq();
			//}
		}
		bool isEmpty() const
		{
			return nullptr == Head->next; /// size == 0, empty..
		}
		bool empty() const { return isEmpty(); }
		//
		void addq(const T& p) /// push
		{
			Element* temp = pool->Allocate(); // new Element(p); //
			
			temp->data = p;
			Rear->next = temp;//
			temp->next = nullptr;// auto look def of Element
			Rear = Rear->next; //
		}//
		void push(const T& p) { addq(p); }
		T deleteq() { /// pop
			if (isEmpty())
			{
				throw QueueEmptyError();
			}
			//
			Element* temp = Head->next;//
			Head->next = temp->next;
			//Rear 泥섎━
			if (nullptr == Head->next)
				Rear = Head;
			T returnTemp = temp->data;
			//delete temp;
			pool->DeAllocate(temp);
			return returnTemp;
		}
		T pop() { return deleteq(); }
		Queue& operator<<(const T& p) {
			addq(p); //
			return *this;
		}
	private:
		void copy(const Queue<T>& q)
		{
			//this->clear();

			Element* qTemp = q.Head->next;

			while (qTemp != nullptr)
			{
				addq(qTemp->data);
				qTemp = qTemp->next;
			}
		}
	public:
		//
		Queue(const Queue<T>& q) : Head(nullptr), Rear(nullptr)
		{// clear();
			Head = new Element(); Rear = Head;
			copy(q);
		}
		Queue& operator=(const Queue<T>& q)
		{
			if (Head == q.Head) { return *this; }

			clear();

			// head, rear reset..!!
			copy(q);

			return *this;
		}
		Queue(Queue<T>&& q)
		{
			Head->next = q.Head->next;
			Rear = q.Rear;

			/// do-empty..
			q.Head->next = nullptr;
			q.Rear = q.Head;
		}
		Queue<T>& operator=(Queue<T>&& q)
		{
			if (Head == q.Head) { return *this; }
			this->clear();
			Head->next = q.Head->next;
			Rear = q.Rear;

			q.Head->next = nullptr;
			q.Rear = q.Head;
			return *this;
		}
	};

	// Queue using Array
	template <class T>
	class ArrayQueue
	{
	public:
		ArrayQueue(const ArrayQueue<T>& aq)
			:que(aq.que), start(aq.start), num(aq.num)
		{
			//
		}
		ArrayQueue(ArrayQueue<T>&& aq)
		{
			que = std::move(aq.que);
			start = aq.start;
			num = aq.num;

			aq.que = Array<T>(2);
			aq.start = 0;
			aq.num = 0;
		}
		ArrayQueue<T>& operator=(ArrayQueue<T>&& aq)
		{
			if (que == aq.que) { return *this; }
			this->que.DoEmpty();
			que = std::move(aq.que);
			start = aq.start;
			num = aq.num;

			aq.que = Array<T>(2);
			aq.start = 0;
			aq.num = 0;
			return *this;
		}
		ArrayQueue<T>& operator=(const ArrayQueue<T>& aq)
		{
			if (que == aq.que) { return *this; }
			que.DoEmpty();
			this->que = aq.que;
			this->start = aq.start;
			this->num = aq.num;
			return *this;
		}
	private:
		Array<T> que;
		int start;
		int num;
	public:
		struct Iter
		{
			T* ptr;
			int pos;

			Iter() { }
			Iter(T* ptr, int pos)
				: ptr(ptr), pos(pos)
			{

			}

			bool operator==(const Iter& other) const {
				return pos == other.pos;
			}
			bool operator!=(const Iter& other) const {
				return pos != other.pos;
			}
		};

		Iter begin(int count = 0) { return Iter(&((*this)[count]), count); }

		Iter end() { return Iter(nullptr, size()); }

		Iter erase(Iter ptr) {
			for (int i = ptr.pos; i < size() - 1; ++i) {
				(*this)[i] = move((*this)[i + 1]);
			}
			//(*this)[size() - 1] = T();

			num--;

			return ptr;
		}
	public:
		explicit ArrayQueue(const int max = 2) : start(0), num(0)
		{
	#ifdef QUEUES_DEBUG
			// max > 0
			wiz::checkUnderError(0, max, 1);
	#endif
			que = Array<T>(max);
		}
		virtual ~ArrayQueue()
		{
		}
	public:
		const T& operator[](const int idx) const
		{
			return que[(start + idx) & (que.size()-1)];
		}
		T& operator[](const int idx)
		{
			return que[(start + idx) & (que.size()-1)];
		}
	public:
		void push(const ArrayQueue<T>& val) {
			if (val.empty()) { return; }
			int newSize = this->que.size();

			while (newSize - num - 1 < val.size()) {
				newSize = newSize * 2;
			}
			if (newSize != this->que.size()) {
				// expand array queue.
				ArrayQueue<T> temp(newSize);
				temp.start = 0;
				//
				for (int i = 0; i < this->size(); ++i) {
					temp[i] = std::move((*this)[i]);
				}

				const int iend = val.num;
				for (int i = 0; i < iend; ++i) {
					temp[i + this->num] = val[i];
				}

				temp.num = this->num + val.num;
				*this = std::move(temp);
			}
			else {
				for (int i = 0; i < val.size(); ++i) {
					this->push(val.que[i]);
				}
			}
		}
		void push(ArrayQueue<T>&& val) { // chk..
			if (val.empty()) { return; }
			int newSize = this->que.size();

			while (newSize - num - 1 < val.size()) {
				newSize = newSize * 2;
			}
			if (newSize != this->que.size()) {
				// expand array queue.
				ArrayQueue temp(newSize);
				temp.start = 0;
				//
				for (int i = 0; i < this->size(); ++i) {
					temp[i] = std::move((*this)[i]);
				}

				const int iend = val.num;
				for (int i = 0; i < iend; ++i) {
					temp[i + this->num] = std::move(val[i]);
				}

				temp.num = this->num + val.num;
				*this = std::move(temp);
			}
			else {
				for (int i = 0; i < val.size(); ++i) {
					this->push(std::move(val[i]));
				}
			}
		}
		void push(const T& val)
		{
			if (isFull())
			{
				// expand array queue.
				ArrayQueue temp(que.size() * 2);
				//
				for (int i = 0; i < que.size(); ++i) {
					temp[i] = std::move(que[(start + i) & (que.size() - 1)]);
				}
				temp.start = 0;
				temp.num = que.size() - 1;

				*this = std::move(temp);
			}
			que[(start + num) & (que.size() - 1)] = val;
			num++;
		}

		void push(T&& val)
		{
			if (isFull())
			{
				// expand array queue.
				ArrayQueue temp(que.size() * 2);
				//
				for (int i = 0; i < que.size(); ++i) {
					temp[i] = std::move(que[(start + i) & (que.size() - 1)]);
				}
				temp.start = 0;
				temp.num = que.size() - 1;

				*this = std::move(temp);
			}
			que[(start + num) & (que.size() - 1)] = std::move(val);
			num++;
		}

		void pop(T* t = nullptr) {
			if (isEmpty()) { throw QueueEmptyError(); }

			T temp = std::move(que[start]);

			//que[start] = T();

			start = (start + 1) & (que.size() - 1); // % que.size(), 2^n.
			num--;

			if (nullptr != t) {
				*t = move(temp);
			}
		}
		void pop_front(T* t = nullptr) {
			pop(t);
		}

		T pop_back()
		{
			if (isEmpty()) { throw QueueEmptyError(); }
			T temp = std::move((*this)[num - 1]);

			//(*this)[num - 1] = T();

			num--;

			return temp;
		}

		bool isFull()const
		{
			return num >= (que.size() - 1);
		}

		bool isEmpty()const
		{
			return 0 == num;
		}
		int size()const { return num; }
		bool empty()const { return isEmpty(); }
		int getNumMax()const { return que.size(); }
		int getNum()const { return num; }
		T& front() { return (*this)[0]; }
		const T& front()const { return (*this)[0]; }
		Array<T> toArray()const
		{
			Array<T> temp;

			if (num > 0)
			{
				temp = Array<T>(num);
				int count = 0;

				for (int i = 0; i < num; i++)
				{
					temp[i] = que[(start + i) & (que.size() - 1)];
				}
			}

			return temp;
		}

		ArrayQueue<T>& operator<<(const T& data)
		{
			push(data);

			return *this;
		}
	public:
		// size must 2^n!, init?
		bool reserve(const int size)
		{
			que = Array<T>(size);
			start = 0;
			num = 0;

			return true;
		}
	};
}
// my smart ptr, no with thread!
namespace wiz {
	template < class T, class IS_SAME_VALUE = wiz::EQ<T> >
	class WizSmartPtr
	{
	private:
		WizSmartPtr<T, IS_SAME_VALUE>* left;
		WizSmartPtr<T, IS_SAME_VALUE>* right;
		T* ptr;
		int option;
	public:
		const WizSmartPtr<T, IS_SAME_VALUE>* getThis()const { return this; }
	private:
		void doNULL(WizSmartPtr<T, IS_SAME_VALUE>* sp)
		{
			sp->ptr = NULL;
			sp->left = sp;
			sp->right = sp;
		}
		void quit()
		{
			right->left = this->left;
			left->right = this->right;
			doNULL(this);
		}

		void enter(const WizSmartPtr<T, IS_SAME_VALUE>& sp)
		{
			//wizard::assertNotNULL( sp );
			//wizard::assertNULL( this->ptr );

			if (NULL == sp.ptr)
			{
				doNULL(this);
			}
			else if (NULL == this->ptr)
			{
				this->ptr = sp.ptr;
				this->option = sp.option; ///
				this->right = const_cast<WizSmartPtr<T, IS_SAME_VALUE>*>(sp.getThis());
				this->left = sp.left;
				this->left->right = this;
				this->right->left = this;
			}
		}

		void initFromOther(const WizSmartPtr<T, IS_SAME_VALUE>& sp)
		{
			//wizard::assertNotEquals( this, &sp );
			// if( this == sp.getThis() ) { return; } 

			// delete or quit linked list of this
			if (NULL != this->ptr)
			{
				if (isOnlyOne()) //
				{
					remove();
				}
				else
				{
					quit();
				}
			}

			// enter linked list of sp
			enter(sp);

			return;
		}
	public:
		explicit
			WizSmartPtr(T* ptr = NULL)
			: ptr(ptr), option(0)
		{
			left = this; right = this;
		}
		explicit
			WizSmartPtr(T* ptr, const int option) // option 1,2,..
			: ptr(ptr), option(option)
		{
			left = this; right = this;
		}
		WizSmartPtr(const WizSmartPtr<T, IS_SAME_VALUE>& sp)
			: ptr(NULL), option(sp.option)
		{
			left = this; right = this;

			initFromOther(sp);
		}
		virtual ~WizSmartPtr() /// virtual??
		{
			if (isOnlyOne())
			{
				remove(true);
			}
			else
			{
				remove(false);
			}
		}
	public:
		WizSmartPtr<T, IS_SAME_VALUE>& operator=(const WizSmartPtr<T, IS_SAME_VALUE>& _sp)
		{
			// temp link
			WizSmartPtr<T, IS_SAME_VALUE> tempLink(_sp);

			initFromOther(tempLink);

			return *this;
		}
		T& operator*()
		{
			// wizard::assertNotNULL( ptr );
			return (*ptr);
		}
		const T& operator*()const
		{
			// wizard::assertNotNULL( ptr );
			return (*ptr);
		}
	public:
		bool isOnlyOne()const
		{
			return (this == left) && (this == right); // && NULL != ptr ?
		}
		bool isNULL()const
		{
			return NULL == ptr;
		}
		bool empty()const
		{
			return isNULL();
		}
		/// remove return suceess?
		bool remove()
		{
			return remove(true);
		}
		bool remove(const bool bremove) // make private and delete =true, and make public remove() call remove( true ); - 2012.3.5 todo...
		{
			if (empty()) { return false; }
			if (!bremove && isOnlyOne()) { return false; } /// 2013.08.13 false means "no change"??
			if (NULL != ptr && bremove)
			{
				delete ptr; ptr = NULL;

				// reset all linked list`s ptr -> NULL
				WizSmartPtr<T, IS_SAME_VALUE>* temp = this->right;
				WizSmartPtr<T, IS_SAME_VALUE>* temp2;
				// doNULL others
				while (temp != this)
				{
					temp2 = temp;
					temp = temp->right;
					doNULL(temp2);
				}
				//doNULL this
				doNULL(this);
			}
			else
			{
				//// 2012.3.5 todo  - make function - "quit" linkedList of this, "enter" linkeList of other
				// quit linked list of this
				quit();
			}
			return true;
		}
	public:
		const T* operator->()const
		{
			//wizard::assertNotNULL( ptr );
			return ptr;
		}
		T* operator->()
		{
			//wizard::assertNotNULL( ptr );
			return ptr;
		}
		T* operator&()
		{
			return ptr;
		}
		const T* operator&()const
		{
			return ptr;
		}
		///
	public:
		bool operator!=(const WizSmartPtr<T, IS_SAME_VALUE>& wsp) const
		{
			return (*ptr) != (*(wsp.ptr));
		}
		bool operator==(const WizSmartPtr<T, IS_SAME_VALUE>& wsp) const ///
		{
			return (*ptr) == (*(wsp.ptr));
		}
		bool isSameValue(const WizSmartPtr<T, IS_SAME_VALUE>& wsp) const /// ���� ���� ������ �ִ°�?
		{
			return IS_SAME_VALUE()((*ptr), (*wsp.ptr));
		}
		bool hasSameObject(const WizSmartPtr<T, IS_SAME_VALUE>& wsp) const
		{
			return ptr == wsp.ptr;
		}
	};
}


class WizString // stringview?
{
private:
	char* str;
	wiz::Pool<char>* pool;
public:
	WizString() {
		str = nullptr;
		pool = nullptr;
	}
	WizString(const char* cstr, wiz::Pool<char>* pool)
		: pool(pool)
	{
		str = pool->Allocate(strlen(cstr) + 1);
		strcpy(str, cstr);
	}
	virtual ~WizString()
	{
		if (pool) {
			pool->DeAllocate(str);
		}
	}
};


int main(void)
{
	const int MAX = 15000000;
	int a, b;

	a = clock();
	{
		wiz::Pool<char> chPool(4 * MAX);
		wiz::Pool<wiz::Queue<WizString>::Element> pool(sizeof(wiz::Queue<WizString>::Element) * MAX);
		wiz::Queue<WizString> x(&pool);
		for (int i = 0; i < MAX; ++i) {
			x.push(WizString(to_string(i).c_str(), &chPool));
		}
	}
	b = clock();
	cout << b - a << "ms" << endl;

	a = clock();
	{
		wiz::ArrayQueue<string> x;
		for (int i = 0; i < MAX; ++i) {
			x.push(to_string(i));
		}
	}
	b = clock();
	cout << b - a << "ms" << endl;


	/**
	a = clock();
	for (int i = 0; i < MAX; ++i) {
		shared_ptr<int> x(new int);
		shared_ptr<int> y(x);
	}
	b = clock();

	cout << b - a << "ms" << endl;


	a = clock();
	for (int i = 0; i < MAX; ++i) {
		wiz::WizSmartPtr<int> x(new int);
		wiz::WizSmartPtr<int> y(x);
	}
	b = clock();

	cout << b - a << "ms" << endl;
	*/

	/*
	a = clock();
	for (int i = 0; i < MAX; ++i) {
		int* x = new int;
		int* y = x;

		delete x;
	}
	b = clock();

	cout << b - a << "ms" << endl;
	*/

	return 0;
}

