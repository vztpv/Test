
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>


#define WIZ_STRING_BUFFER_SIZE 5
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
			char* start; // removal?
			char* ptr;
		public:
			explicit Ptr()
			{
				left = nullptr;
				right = nullptr;

				no = 0;
				num = 0;

				start = nullptr;
				ptr = nullptr;
			}
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
						while ((temp->no < 0 || iter_pool != pool + temp->no) && iter_pool + temp->num <= pool + max_size) {
							iter_pool++;
						}
						if (iter_pool + temp->num > pool + max_size) {
							break;
						}

						//for (int k = 0; k < temp->num; ++k) {
						//	pool[count + k] = *(iter_pool + k);
						//}
						memmove(pool + count, iter_pool, temp->num);
						
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
						
						//for (int k = 0; k < max_size; ++k) {
						//	buffer[k] = pool[k];
						//}
						memmove(buffer, pool, max_size);

						Ptr* temp = dump.left;
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

		class StringBuilder //
		{
		private:
			char* buffer_first;
			char* buffer;
			int len;
			int capacity;
		private:
			char* end() const
			{
				return buffer_first + capacity;
			}
		public:
			StringBuilder(const int buffer_size, const char* cstr = "", int len = 0) {
				buffer = (char*)malloc(sizeof(char) * (buffer_size + 1)); // 1 for '\0'
				this->len = len;
				capacity = buffer_size;
				memcpy(buffer, cstr, this->len);
				buffer[this->len] = '\0';
				buffer_first = buffer;
			}
			StringBuilder(const StringBuilder&) = delete;
			StringBuilder(StringBuilder&&) = delete;
			StringBuilder& operator=(const StringBuilder& other)
			{
				if (buffer_first != nullptr) {
					free(buffer_first);
				}
				len = other.len;
				capacity = other.capacity;
				buffer_first = (char*)malloc(sizeof(char)*(len + 1));
				buffer = other.buffer;

				for (int i = 0; i <= len; ++i) {
					buffer_first[i] = other.buffer_first[i];
				}

				return *this;
			}
			StringBuilder& operator=(StringBuilder&& other)
			{
				buffer_first = other.buffer_first;
				buffer = other.buffer;
				len = other.len;
				capacity = other.capacity;

				other.buffer_first = nullptr;
				other.buffer = nullptr;
				other.len = 0;

				return *this;
			}

			virtual ~StringBuilder()
			{
				free(buffer_first);
			}

			StringBuilder& AppendChar(const char ch)
			{
				char temp[2];
				temp[0] = ch; temp[1] = '\0';
				return Append(temp, 1);
			}

			StringBuilder& Append(const char* cstr, const int len)
			{
				if (buffer + this->len + len < end())
				{
					memcpy(buffer + this->len, cstr, len);
					buffer[this->len + len] = '\0';
					this->len = this->len + len;
				}
				else {
					if (buffer_first + this->len + len < end()) {
						memmove(buffer_first, buffer, this->len);
						memcpy(buffer_first + this->len, cstr, len);
						buffer_first[this->len + len] = '\0';
						buffer = buffer_first;
						this->len = this->len + len;
					}
					else {
						char* new_buffer = (char*)malloc(sizeof(char) * (this->len + len + 1));
						memcpy(new_buffer, buffer, this->len);
						memcpy(new_buffer + this->len, cstr, len);
						new_buffer[this->len + len] = '\0';
						free(buffer_first);
						buffer = new_buffer;
						buffer_first = buffer;
						this->len = this->len + len;
					}
				}
				return *this;
			}
			const char* Divide(const int idx) // need to rename!l, chk idx range!
			{
				buffer[idx] = '\0';
				return buffer;
			}
			const char* Str(int* size = nullptr) const {
				if (size) { *size = len; }
				return buffer;
			}
			char* Str(int* size = nullptr) {
				if (size) { *size = len; }
				return buffer;
			}
			void Clear()
			{
				len = 0;
				buffer = buffer_first;
				buffer[0] = '\0';
			}
			int Size() { return len; }
			StringBuilder& LeftShift(const int offset = 1)
			{
				if (offset < 1) { return *this; }

				if (buffer + offset < end()) {
					buffer = buffer + offset;
				}
				else {
					memmove(buffer_first, buffer + offset, len - offset);
					buffer = buffer_first;
					buffer[len - offset] = '\0';
				}
				len = len - offset;
				return *this;
			}

			void SetLength(const int len)
			{
				this->len = len;
			}

			void SetBuffer(const int idx, const char val)
			{
				this->buffer[idx] = val;
			}
		};
		class WizString
		{
		private:
			char buffer[WIZ_STRING_BUFFER_SIZE];
			Ptr str;
			int len;
			int moved;
			Pool* pool;
		private:
			void init(const char* cstr, int n)
			{
				moved = 0;

				if (0 == n) {
					len = 0;
					str.ptr = "";
					buffer[0] = '\0';
				}
				else {
					if (n > WIZ_STRING_BUFFER_SIZE) {
						pool->Alloc(&str, n + 1); 
						strncpy(str.ptr, cstr, n + 1);
						len = n;
					}
					else {
						str.ptr = "";
						strncpy(buffer, cstr, n + 1);
						len = n;
					}
				}
			}
			void remove()
			{
				if (0 != len) {
					if (len > WIZ_STRING_BUFFER_SIZE && 0 == moved) {
						if (pool) {
							pool->DeAlloc(&str);
						}

						str.ptr = "";
						buffer[0] = '\0';
						len = 0;
						moved = -1;
					}
					else if (0 == moved) {
						buffer[0] = '\0';
						str.ptr = "";
						len = 0;
						moved = -2;
					}
				}
			}
		public:
			explicit WizString() : pool(nullptr) {
				init("", 0);
			}
			explicit WizString(Pool* pool, const char* cstr)
				: pool(pool) {
				init(cstr, strlen(cstr));
			}
			WizString(const WizString& other)
			{
				this->pool = other.pool;
				init("", 0);
				AssignNoMove(other);
			}
			WizString(WizString&& other)
			{
				pool = other.pool;
				other.pool = nullptr;

				len = other.len;
				other.len = 0;
				moved = other.moved;
				other.moved = true;
				str = other.str;
				other.str.ptr = "";

				for (int i = 0; i < WIZ_STRING_BUFFER_SIZE; ++i) {
					buffer[i] = other.buffer[i];
				}
			}

			virtual ~WizString()
			{
				remove();
			}
		private:
			// assignment -> DON`T
			void operator=(const WizString&) = delete;
			// move assignment -> DON`T
			void operator=(WizString&&) = delete;
		public:
			bool AssignWithMove(WizString& str) {
				if (str.moved)
				{
					return false;
				}

				if (this->len != str.len || str.moved) {
					this->remove();
					this->len = str.len;
					if (str.len > WIZ_STRING_BUFFER_SIZE) {
						pool->Alloc(&this->str, str.len + 1);
					}
				}
				this->moved = str.moved;
				str.moved = true;
				strncpy(this->CStr(), str.CStr(), str.len + 1);

				return true;
			}
			bool AssignNoMove(const WizString& str) {
				if (false == str.moved)
				{
					return false;
				}

				if (this->len != str.len || str.moved) {
					this->remove();
					this->len = str.len;
					if (str.len > WIZ_STRING_BUFFER_SIZE) {
						pool->Alloc(&this->str, str.len + 1);
					}
				}
				this->moved = str.moved;
				strncpy(this->CStr(), str.CStr(), str.len + 1);

				return true;
			}
			bool Assign(const char* cstr) {
				const int cstr_len = strlen(cstr);

				if (this->len != cstr_len) {
					this->remove();
					this->len = cstr_len;
					if (cstr_len > WIZ_STRING_BUFFER_SIZE) {
						pool->Alloc(&this->str, cstr_len + 1);
					}
				}
				this->moved = false; //
				strncpy(this->CStr(), cstr, cstr_len + 1);

				return true;
			}
		public:
			// +
			// + with const char*
			// + with const char*, friend
			friend WizString operator+(const WizString& str1, const WizString& str2)
			{
				WizString temp;

				temp.len = str1.len + str2.len;
				temp.moved = 0;

				if (temp.len == 0) {
					temp.buffer[0] = '\0';
					temp.str.ptr = "";
				}
				else {
					if (temp.len > WIZ_STRING_BUFFER_SIZE) {
						str1.pool->Alloc(&temp.str, temp.len + 1);
						strncpy(temp.str.ptr, str1.CStr(), str1.len + 1);
						strncpy(temp.str.ptr + str1.len, str2.CStr(), str2.len + 1);
					}
					else {
						strncpy(temp.buffer, str1.CStr(), str1.len + 1);
						strncpy(temp.buffer + str1.len, str2.CStr(), str2.len + 1);
					}
				}

				return temp;
			}
			friend WizString operator+(const WizString& str1, const char* cstr)
			{
				const int cstr_len = strlen(cstr);
				WizString temp;

				temp.len = str1.len + cstr_len;
				temp.moved = 0;

				if (temp.len == 0) {
					temp.buffer[0] = '\0';
					temp.str.ptr = "";
				}
				else {
					if (temp.len > WIZ_STRING_BUFFER_SIZE) {
						str1.pool->Alloc(&temp.str, temp.len + 1);
						strncpy(temp.str.ptr, str1.CStr(), str1.len + 1);
						strncpy(temp.str.ptr + str1.len, cstr, cstr_len + 1);
					}
					else {
						strncpy(temp.buffer, str1.CStr(), str1.len + 1);
						strncpy(temp.buffer + str1.len, cstr, cstr_len + 1);
					}
				}
				return temp;
			}
			friend WizString operator+(const char* cstr, const WizString& str)
			{
				WizString temp;
				temp.AssignNoMove(str + cstr);
				return temp;
			}

			// +=
			void operator+=(const WizString& str)
			{
				const int len = this->len + str.len;
				WizString temp;
				temp.moved = false;

				temp.len = this->len + str.len;
				if (temp.len > WIZ_STRING_BUFFER_SIZE) {
					pool->Alloc(&temp.str, temp.len + 1);
				}
				strncpy(temp.CStr(), this->CStr(), this->len + 1);
				strncpy(temp.CStr() + this->len, str.CStr(), str.len + 1);

				remove();

				this->AssignWithMove(temp);
			}
			// += with const char*
			void operator+=(const char* cstr)
			{
				const int cstr_len = strlen(cstr);

				const int len = this->len + cstr_len;
				WizString temp;
				temp.moved = false;

				temp.len = this->len + cstr_len;
				if (temp.len > WIZ_STRING_BUFFER_SIZE) {
					pool->Alloc(&temp.str, temp.len + 1);
				}
				strncpy(temp.CStr(), this->CStr(), this->len + 1);
				strncpy(temp.CStr() + this->len, cstr, cstr_len + 1);

				remove();

				this->AssignWithMove(temp);
			}
			// ==
			bool operator==(const WizString& str)
			{
				if (this->len != str.len) { return 0; }
				return 0 == strcmp(this->CStr(), str.CStr());
			}
			// == with const char*
			bool operator==(const char* str)
			{
				if (this->len != strlen(str)) { return 0; }
				return 0 == strcmp(this->CStr(), str);
			}
			// == with const char*, friend
			friend bool operator==(const char* str, const WizString& wizStr)
			{
				if (wizStr.len != strlen(str)) { return 0; }
				return 0 == strcmp(wizStr.CStr(), str);
			}
			// !=
			bool operator!=(const WizString& str)
			{
				if (this->len == str.len) { return 0; }
				return 0 != strcmp(this->CStr(), str.CStr());
			}
			// != with const char*
			bool operator!=(const char* str)
			{
				if (this->len == strlen(str)) { return 0; }
				return 0 != strcmp(this->CStr(), str);
			}
			// != with const char*, friend
			friend bool operator!=(const char* str, const WizString& wizStr)
			{
				if (wizStr.len == strlen(str)) { return 0; }
				return 0 != strcmp(wizStr.CStr(), str);
			}
		private:
			static int find(const char* cstr, char x, int before, int n)
			{
				int i = 0;

				for (i = before + 1; i < n; ++i) {
					if (cstr[i] == x) {
						return i;
					}
				}

				return -1;
			}
		public:
			static int BufferSize()
			{
				return WIZ_STRING_BUFFER_SIZE;
			}

			// enterkey is included, return valid?
			static void GetLine(FILE* file, WizString* strVec) {
				GetMultiLine(file, strVec, 1);
			}
			// enterkey is included, return valid? - has bug? if lineNum > 1 !!
			static void GetMultiLine(FILE* file, WizString* strVec, const int lineNum) {
				int real_line_num = -1;
				const int line_max = lineNum;
				const int num = 102400;
				StringBuilder builder(num * 10, "", 0);
				int real_count;
				int i;
				int size = 0;
				const char* builder_cstr = NULL;
				WizString temp;
				int temp2;
				const char* temp3 = NULL;
				int chk = 1;
				int offset = 0; // for backward
				int temp5;
				int fpos = ftell(file);

				i = 0;
				temp2 = -1;
				temp5 = -1;

				while (true) {
					if (chk) {
						StringBuilder temp_builder(num, "", 0);
						real_count = fread((void*)temp_builder.Str(), sizeof(char), num, file);

						temp_builder.SetLength(real_count);
						temp_builder.SetBuffer(real_count, '\0');

						builder.Append(temp_builder.Str(), temp_builder.Size());
					}

					builder_cstr = builder.Str();
					size = builder.Size();

					for (; i < line_max; ++i) {
						int chk_find = WizString::find(builder_cstr, '\n', temp2, size);

						temp5 = chk_find;

						if (chk_find != -1) {
							temp2 = chk_find;
						}
						else {
							chk = 1;
							break;
						}
					}

					// ... enterkey ... 
					// ... find_idx ... 
					if (line_max == i || real_count != num) {
						real_line_num = i;

						if (temp5 == -1) {
							temp2 = builder.Size() - 1;
						}

						builder.Divide(temp2 + 1);

						temp3 = builder.Str();
						temp.init(temp3, strlen(temp3));

						builder.LeftShift(temp2 + 1);
					}
					else {
						continue;
					}

					if (line_max == i) {
						fpos = fpos + temp2 + 1 + i;
						fseek(file, fpos, SEEK_SET);
					}
					else {
						//
					}

					strVec->AssignWithMove(temp);

					break;
				}
			}

			static long long ToInteger(const WizString& str)
			{
				return atoll(str.CStr());
			}
			static long double ToNumber(const WizString& str)
			{
				return strtold(str.CStr(), nullptr);
			}

			static bool ToString(long long x, WizString* str)
			{
				StringBuilder temp(1024, "", 0);
				int size = 1024;

				while (1) {
					int chk = snprintf(temp.Str(), size, "%lld", x);
					if (!(0 <= chk && chk < size)) {
						size = size * 2;
						temp = StringBuilder(size, "", 0);
					}
					else {
						temp.SetLength(chk);
						break;
					}
				}

				str->Assign(temp.Str());
				return true;
			}
			static bool ToString(long double x, WizString* str)
			{
				StringBuilder temp(1024, "", 0);
				int size = 1024;

				while (true) {
					int chk = snprintf(temp.Str(), size, "%Lf", x);
					if (!(0 <= chk && chk < size)) {
						size = size * 2;
						temp = StringBuilder(size, "", 0);
					}
					else {
						temp.SetLength(chk);
						break;
					}
				}

				str->Assign(temp.Str());

				return true;
			}

			bool SubString(WizString* result, int begin, int end)
			{
				WizString temp;

				temp.len = (end - 1) - begin + 1;
				if (temp.len <= 0) { return false; }
				temp.moved = 0;

				if (temp.len > WIZ_STRING_BUFFER_SIZE) {
					pool->Alloc(&temp.str, temp.len + 1);

					strncpy(temp.str.ptr, CStr() + begin, end - begin);
					temp.str.ptr[end - begin] = '\0';
				}
				else {
					strncpy(temp.buffer, CStr() + begin, end - begin);
					temp.buffer[end - begin] = '\0';
				}
				result->AssignWithMove(temp);

				return true;
			}

			bool SubStringAndAssign(int begin, int end)
			{
				const int _len = (end - 1) - begin + 1;
				const int start = begin;
				const int len = Size();
				const int substr_len = _len;

				const int left_num = start;
				const int right_num = len - left_num - substr_len - start;
				wiz::String::Ptr left, right;

				left.num = left_num;
				right.num = right_num;
				str.num = substr_len + 1;

				str.ptr = str.ptr + start;
				str.ptr[substr_len] = '\0';

				left.no = -3;
				right.no = -3;
				str.no += start;


				str.left->right = &left;
				str.right->left = &right;

				right.left = str.left;
				right.right = &str;

				left.left = &str;
				left.right = str.right;

				str.left = &right;
				str.right = &left;

				pool->DeAlloc(&left);
				pool->DeAlloc(&right);

				return true;
			}

			bool Erase(int idx)
			{
				for (int i = idx; i < this->len - 1; ++i) {
					this->CStr()[i] = this->CStr()[i + 1];
				}
				this->CStr()[this->len - 1] = '\0';
				this->len--;

				if (this->len == WIZ_STRING_BUFFER_SIZE) {
					strncpy(this->buffer, this->str.ptr, WIZ_STRING_BUFFER_SIZE + 1);

					pool->DeAlloc(&this->str);

					this->str.ptr = "";
					this->moved = -3;
				}
			}

			const char* CStr() const
			{
				if (len > WIZ_STRING_BUFFER_SIZE) { return str.ptr; }
				return buffer;
			}
			char* CStr()
			{
				if (len > WIZ_STRING_BUFFER_SIZE) { return str.ptr; }
				return buffer;
			}

			bool StartsWith(const WizString& start_str) const
			{
				int result = true;

				if (this->len < start_str.len) {
					return false;
				}

				for (int i = 0; i < start_str.len; ++i) {
					if (this->CStr()[i] == start_str.CStr()[i]) {
						//
					}
					else {
						result = false;
						break;
					}
				}

				return result;
			}
			bool StartsWith(const char* start_cstr) const
			{
				bool result = true;
				const int start_cstr_len = strlen(start_cstr);

				if (this->len < start_cstr_len) {
					return false;
				}

				for (int i = 0; i < start_cstr_len; ++i) {
					if (this->CStr()[i] == start_cstr[i]) {
						//
					}
					else {
						result = false;
						break;
					}
				}

				return result;
			}
			bool LastsWith(const WizString& last_str) const
			{
				bool result = true;
				int j;

				if (this->len < last_str.len) {
					return 0;
				}

				j = last_str.len - 1;
				for (int i = this->len - 1; i >= 0 && j >= 0; --i) {
					if (this->CStr()[i] == last_str.CStr()[j]) {
						--j;
					}
					else {
						result = false;
						break;
					}
				}

				return result;
			}
			bool LastsWith(const char* last_cstr) const
			{
				bool result = true;
				int j;
				const int last_cstr_len = strlen(last_cstr);

				if (this->len < last_cstr_len) {
					return 0;
				}

				j = last_cstr_len - 1;
				for (int i = this->len - 1; i >= 0 && j >= 0; --i) {
					if (this->CStr()[i] == last_cstr[j]) {
						--j;
					}
					else {
						result = false;
						break;
					}
				}

				return result;
			}

			void Reverse()
			{
				char ch;

				for (int i = 0; i < this->len / 2; ++i) {
					ch = this->CStr()[i];
					this->CStr()[i] = this->CStr()[this->len - 1 - i];
					this->CStr()[this->len - 1 - i] = ch;
				}
			}

			int Size()const
			{
				return len;
			}
			bool Empty()const
			{
				return 0 == len;
			}
			int Back() const
			{
				if (len > WIZ_STRING_BUFFER_SIZE) { return str.ptr[len - 1]; }
				return buffer[len - 1];
			}
			int Front() const
			{
				if (len > WIZ_STRING_BUFFER_SIZE) { return str.ptr[0]; }
				return buffer[0];
			}
		private:
			bool comp(const char* x, const char* y, int n)
			{
				for (int i = 0; i < n; ++i) {
					if (x[i] != y[i]) {
						return false;
					}
				}
				return true;
			}
		public:
			bool Replace(const WizString& mark, const WizString& changed, WizString* result, StringBuilder* builder)
			{
				const char* pStr = this->CStr();

				builder->Clear();

				// chk??
				if (mark.Empty()) {
					return false;
				}

				for (int i = 0; i < this->Size(); i++) {
					if (strlen(pStr + i) >= mark.Size()
						&& comp(pStr + i, mark.CStr(), mark.Size()))
					{
						builder->Append(changed.CStr(), changed.Size());
						i = i + mark.Size() - 1;
					}
					else
					{
						builder->AppendChar(this->CStr()[i]);
					}
				}

				result->Assign(builder->Str());

				return true;
			}
			void AddSpace(WizString* result, StringBuilder* builder)
			{
				builder->Clear();

				for (int i = 0; i < this->Size(); ++i)
				{
					if ('=' == this->CStr()[i]) {
						builder->Append(" = ", 3);
					}
					else if ('{' == this->CStr()[i]) {
						builder->Append(" { ", 3);
					}
					else if ('}' == this->CStr()[i]) {
						builder->Append(" } ", 3);
					}
					else {
						builder->AppendChar(this->CStr()[i]);
					}
				}

				result->Assign(builder->Str());
			}
		};
	}
}

#include <vector>
#include <ctime>

int main(void)
{
	int a, b;
	const int MAX = 10000000;

	a = clock();
	{
		wiz::String::Pool pool;
		for (int t = 0; t < MAX; ++t) {
			wiz::String::WizString str(&pool, "abcdefghabcdefghabcdefghabcdefgh");
			str.SubStringAndAssign(5, 26);
		}
	}
	b = clock();
	std::cout << b - a << std::endl;

	a = clock();
	{
		wiz::String::Pool pool;
		//std::vector<wiz::String::Ptr> str(MAX);
		for (int t = 0; t < MAX; ++t) {
			do {
				wiz::String::Ptr str;
				pool.Alloc(&str, 33);
				strcpy(str.ptr, "abcdefghabcdefghabcdefghabcdefgh");

				constexpr int start = 5;
				constexpr int len = 32;
				constexpr int substr_len = 25;
				
				constexpr int left_num = start;	
				constexpr int right_num = len - left_num - substr_len - start; 
				wiz::String::Ptr left, right;

				left.num = left_num;
				right.num = right_num;
				str.num = substr_len + 1;

				str.ptr = str.ptr + start;
				str.ptr[substr_len] = '\0';

				left.no = -3;
				right.no = -3;
				str.no += start;


				str.left->right = &left;
				str.right->left = &right;

				right.left = str.left;
				right.right = &str;

				left.left = &str;
				left.right = str.right;

				str.left = &right;
				str.right = &left;

				pool.DeAlloc(&left);
				pool.DeAlloc(&right);

				pool.DeAlloc(&str);
			} while (false);
		}
	}
	b = clock();
	std::cout << b - a << std::endl;

	a = clock();
	for (int t = 0; t < MAX; ++t) {
		do {
			std::string test = "abcdefghabcdefghabcdefghabcdefgh";
			std::string sol = test.substr(5, 25); // number 6
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

