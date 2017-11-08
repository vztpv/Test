

#ifdef _DEBUG 
#include <vld.h>
#endif

#include <iostream>


template <class T>
class LocalPtr // removal?
{
public:
	T* ptr;
	int size;
public:
	explicit LocalPtr() : ptr(nullptr), size(0) { }
	explicit LocalPtr(T* ptr, int size) : ptr(ptr), size(size) {

	}
public:

	bool Expand()
	{
		if (0 == size) {
			return false;
		}

		T* temp = new T[2 * size];
		for (int i = 0; i < size; ++i) {
			temp[i] = ptr[i];
		}
		delete[] ptr;
		ptr = temp;
		size = 2 * size;

		return true;
	}
};


// base for vec, arr, ...
template <class T>
class LocalBase
{
public:
	LocalPtr<T> buf;
	bool isOwner;
private:
	LocalBase(const LocalBase&) = delete;
	LocalBase& operator=(const LocalBase&) = delete;
public:
	explicit LocalBase() : buf(nullptr, 0), isOwner(true) {}
	virtual ~LocalBase() {
		if (buf.ptr) {
			delete[] buf.ptr;
		}
	}

	explicit LocalBase(int size) {
		buf.ptr = new T[size];
		buf.size = size;
		isOwner = true;
	}

	LocalBase(LocalBase&& aaa) {
		if (aaa.isOwner == false) {
			throw "aaa.isOwner is false in move constructor";
		}
		buf = aaa.buf;
		isOwner = true;
		aaa.isOwner = false;
		aaa.buf.ptr = nullptr;
	}
	void operator=(LocalBase&& aaa) {
		if (this == &aaa) {
			return;
		}

		if (aaa.isOwner == false) {
			throw "aaa.isOwner is false in =";
		}
		if (buf) { delete[] buf; }
		buf = aaa.buf;
		aaa.buf.ptr = nullptr;
		isOwner = true;
		aaa.isOwner = false;
	}

	bool Expand()
	{
		return buf.Expand();
	}
};  


template <class T>
class LocalVector
{
private:
	LocalBase<T> aaa;
	int size;
	int capacity;
public:
	LocalVector() : aaa(1), size(0), capacity(1)
	{
		//
	}
	LocalVector(LocalVector<T>&& other) {
		aaa = std::move(other.aaa);
		size = other.size;
		capacity = other.capacity;
	}

	int Size() {
		return size;
	}
	int Capacity() {
		return capacity;
	}

	void PushBack(T data) {
		if (size >= capacity) {
			// expand
			aaa.Expand();
			capacity = 2 * capacity;
		}
		aaa.buf.ptr[size] = data;
		size++;
	}
	void PopBack(T& data = nullptr) {
		size--;
		if (data) {
			data = aaa.buf.ptr[size + 1];
		}
	}

	bool Empty()
	{
		return 0 == size;
	}

	virtual ~LocalVector() { }
};


#define let
#define mut

#include <vector>
#include <ctime>

int main(void)
{
	const int MAX = 5000000;
	int a, b;
	a = clock();
	{
		let LocalVector<std::string*> test;
		let LocalVector<std::string*> test2;
		let LocalVector<std::string*> test3;

		std::vector<std::string> vec;

		for (int i = 0; i < MAX; ++i) {
			vec.push_back("1234567890");
			test.PushBack(&vec[i]);
		}
		for (int i = 0; i < MAX; ++i) {
			std::string* x;
			test.PopBack(x);
			test2.PushBack(x);
		}
		for (int i = 0; i < MAX; ++i) {
			std::string* x;
			test2.PopBack(x);
			test3.PushBack(x);
		}
	}
	b = clock();

	std::cout << b - a << std::endl;

	a = clock();
	{
		std::vector<std::string> vec;
		std::vector<std::string> vec2;
		std::vector<std::string> vec3;

		for (int i = 0; i < MAX; ++i) {
			vec.push_back("1234567890");
		}
		for (int i = 0; i < MAX; ++i) {
			std::string x;
			x = vec.back();
			vec.pop_back();
			vec2.push_back(std::move(x));
		}
		for (int i = 0; i < MAX; ++i) {
			std::string x;
			x = vec2.back();
			vec2.pop_back();
			vec3.push_back(std::move(x));
		}
	}
	b = clock();

	std::cout << b - a << std::endl;
	

	return 0;
}

