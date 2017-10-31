
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


int main(void)
{
	wiz::String::Pool pool;
	wiz::String::Ptr p[4096];

	for (int i = 0; i < 4096; ++i) {
		pool.Alloc(&p[i], 100);
		strcpy(p[i].ptr, "abc ");
	}
	for (int i = 0; i < 4; ++i) {
		std::cout << p[i].ptr << std::endl;
	}

	return 0;
}

