

#include <iostream>
#include <cstdlib>


namespace wiz {
	namespace String {
		class Ptr
		{
		public:
			Ptr* left;
			Ptr* right;
			int no;
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
				pool = (char*)malloc(sizeof(char) * 1024);
				max_size = 1024;
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
			void Alloc(Ptr* p) {
				if (n >= max_size) {
					// expand, change pool !
					Ptr* temp = dump.left;
					char* iter_pool = pool;
					int count = 0;

					while (temp != &dump) {
						while (iter_pool != pool + temp->no && iter_pool < pool + max_size) {
							iter_pool++;
						}
						if (iter_pool >= pool + max_size) {
							break;
						}

						pool[count] = *iter_pool;

						count++;
						iter_pool++;
						temp = temp->left;
					}

					char* buffer;
					
					if (count >= n) {
						buffer = (char*)malloc(sizeof(char) * 2 * max_size);

						free(pool);
						pool = buffer;
						max_size = 2 * max_size;
					}

					n = count;
				}

				Ptr* temp = dump.right;

				p->left = &dump;
				temp->left = p;
				p->right = temp;
				dump.right = p;

				p->no = n;

				
				n++;
			}
			void DeAlloc(Ptr* p) {
				p->right->left = p->left;
				p->left->right = p->right;

				p->left = p;
				p->right = p;
				p->no = -2;

				n--;
			}
		};
	}
}


int main(void)
{
	wiz::String::Pool pool;
	wiz::String::Ptr p[4096];

	for (int i = 0; i < 4096; ++i) {
		pool.Alloc(&p[i]);

		int x = rand() % 5;
		if (x) {
			pool.DeAlloc(&p[i]);
		}
	}

	return 0;
}

