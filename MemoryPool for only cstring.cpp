
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdlib>


#define WIZ_STRING_POOL_SIZE 1024

namespace wiz {
	namespace String {
		class Ptr
		{
		public:
			Ptr* left;
			Ptr* right;
			int no; // const?
			int num; // const?
			char* start;
			char* ptr;
		};

		class Pool // for String of char
		{
		private:
			Pool(const Pool&) = delete;
			Pool& operator=(const Pool&) = delete;
		private:
			char* pool;
			int max_size;
			int n;
			Ptr dump; // temp?
		public:
			explicit Pool()
			{
				pool = (char*)malloc(sizeof(char) * WIZ_STRING_POOL_SIZE);
				max_size = WIZ_STRING_POOL_SIZE;
				n = 0;
				dump.left = &dump;
				dump.right = &dump;
				dump.no = -1;
			}
			virtual ~Pool()
			{
				free(pool);
			}
		public:
			void Alloc(Ptr* p, int num = 1) {
				if (n + num > max_size) {
					// expand, change pool !
					Ptr* temp = dump.left;
					char* iter_pool = pool;
					int count = 0;

					while (temp != &dump) {
						while (iter_pool != pool + temp->no && iter_pool + temp->num <= pool + max_size) {
							iter_pool++;
						}
						if (iter_pool + temp->num > pool + max_size) {
							break;
						}

						for (int k = 0; k < temp->num; ++k) {
							pool[count + k] = *(iter_pool + k);
						}
						
						temp->no = count;
						count += temp->num;
						iter_pool += temp->num;
						
						temp = temp->left;
					}

					char* buffer;
					
					if (count + num > max_size) {
						int multiple = 2;
						while (multiple * max_size < count + num) {
							multiple++;
						}
						buffer = (char*)malloc(sizeof(char) * multiple * max_size);
						Ptr* temp = dump.left;
						
						for (int k = 0; k < max_size; ++k) {
							buffer[k] = pool[k];
						}
						while (temp != &dump) {
							temp->start = buffer;
							temp->ptr = buffer + temp->no;

							temp = temp->left;
						}
						free(pool);
						pool = buffer;
						max_size = multiple * max_size;
					}

					n = count;
				}

				Ptr* temp = dump.right;

				p->left = &dump;
				temp->left = p;
				p->right = temp;
				dump.right = p;

				p->no = n;
				p->start = pool;
				p->ptr = pool + n;
				p->num = num;
				
				n += num;
			}
			void DeAlloc(Ptr* p) {
				p->right->left = p->left;
				p->left->right = p->right;

				p->left = p;
				p->right = p;
				p->no = -2;

				n -= p->num;
			}
		};
	}
}

#include <ctime>

int main(void)
{
	int a, b;
	const int MAX = 100000000;


	a = clock();
	{
		wiz::String::Pool pool;
		for (int t = 0; t < MAX; ++t) {
			do {
				wiz::String::Ptr str;

				pool.Alloc(&str, 33);
				strcpy(str.ptr, "abcdefghabcdefghabcdefghabcdefgh");

				wiz::String::Ptr left, right;

				str.ptr[str.num - 2] = '\0';
				left.num = 1;
				right.num = 2;
				str.num -= 3;

				left.ptr = str.ptr - 1;
				right.ptr = str.ptr + str.num + 1;
				str.ptr = str.ptr + 1;

				str.no = 1;
				left.no = 0;
				right.no = 2;

				str.left->right = &left;
				str.right->left = &right;

				left.left = str.left;
				left.right = &str;

				right.left = &str;
				right.right = str.right;

				str.left = &left;
				str.right = &right;


				pool.DeAlloc(&left);
				pool.DeAlloc(&right);

				pool.DeAlloc(&str);
				//std::cout << str.ptr << std::endl;
			} while (false);
		}
	}
	b = clock();
	std::cout << b - a << std::endl;

	a = clock();
	for (int t = 0; t < MAX; ++t) {
		do {
			std::string test = "abcdefghabcdefghabcdefghabcdefgh";
			std::string sol = test.substr(1, 30); // number 6
		} while (false);
	}
	b = clock();
	std::cout << b - a << std::endl;


	while (false) {
		wiz::String::Pool pool;
		wiz::String::Ptr p[4096];

		for (int i = 0; i < 4096; ++i) {
			pool.Alloc(&p[i], 3);

			strcpy(p[i].ptr, "ab");

			if (rand() % 2) {
				pool.DeAlloc(&p[i]);
				p[i].ptr = nullptr;
			}
		}
		for (int i = 0; i < 4; ++i) {
			if (p[i].ptr != nullptr) {
				std::cout << p[i].ptr << std::endl;
			}
		}
	}

	return 0;
}

