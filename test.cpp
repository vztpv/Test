
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <set>
#include <list>
#include <vector>
#include <thread>
#include <deque>


#include <cstdlib>
#include <cstring>

int log_result = 0;
#include "njp.h"

//#include <conio.h> // _getch

namespace wiz {
	template <typename T> /// T <- char, int, long, long long...
	std::string toStr(const T x, const int base) /// chk!!
	{
		if (base < 2 || base > 16) { return "base is not valid"; }
		T i = x;

		const int INT_SIZE = sizeof(T) << 3; ///*8
		char* temp = new char[INT_SIZE + 1 + 1]; /// 1 NULL, 1 minus
		std::string tempString;
		int k;
		bool isMinus = (i < 0);
		temp[INT_SIZE + 1] = '\0'; 

		for (k = INT_SIZE; k >= 1; k--) {
			T val = pos_1<T>(i, base); /// 0 ~ base-1
									   /// number to ['0'~'9'] or ['A'~'F']
			if (val < 10) { temp[k] = val + '0'; }
			else { temp[k] = val - 10 + 'A'; }

			i /= base;

			if (0 == i) { 
				k--;
				break;
			}
		}

		if (isMinus) {
			temp[k] = '-';
			tempString = std::string(temp + k);//
		}
		else {
			tempString = std::string(temp + k + 1); //
		}
		delete[] temp;

		return tempString;
	}

	/// chk.... need more thinking..., ToDo...
	template <typename T> /// T <- char, int, long, long long...
	std::string toStr2(const T x, const int str_space, const int base) /// chk!!
	{
		if (base < 2 || base > 16) { return "base is not valid"; }
		T i = x;
		T k2 = 0;

		const int INT_SIZE = sizeof(T) << 3; ///*8
		char* temp = new char[INT_SIZE + 1 + 1]; /// 1 NULL, 1 minus
		for (int i = 0; i < INT_SIZE + 2; ++i) { temp[i] = '0'; }//
		std::string tempString;
		int k;
		bool isMinus = (i < 0);
		temp[INT_SIZE + 1] = '\0'; 

		for (k = INT_SIZE; k >= 1; k--) {
			T val = pos_1<T>(i, base); /// 0 ~ base-1
									   /// number to ['0'~'9'] or ['A'~'F']
			if (val < 10) { temp[k] = val + '0'; }
			else { temp[k] = val - 10 + 'A'; }

			i /= base;

			if (0 == i) { 
				k--;
				break;
			}
		}

		if (isMinus) {
			temp[k] = '-';
			tempString = std::string(temp + k);//
		}
		else {
			if (INT_SIZE + 1 - (k + 1) + 1 < str_space + 1)
			{
				k2 = str_space + 1 - (INT_SIZE + 1 - (k + 1) + 1);
			}
			else
			{
				k2 = 0;
			}
			tempString = std::string(temp + k + 1 - k2); //
		}
		delete[] temp;

		return tempString;
	}

	std::pair<bool, int> Reserve2_3(std::ifstream& inFile, VECTOR<Token2>* aq, const int num, bool* isFirst, const wiz::LoadDataOption& option, int thr_num, char*& _buffer)
	{
		if (inFile.eof()) {
			return { false, 0 };
		}

		int a = clock();

		//int count = 0;
		std::string temp;
		char* buffer = nullptr;// = new char[length + 1]; // 
		std::vector<int> start(thr_num + 1, 0);
		std::vector<int> last(thr_num + 1, 0);

		if (thr_num > 0) {
			inFile.seekg(0, inFile.end);
			const unsigned long long length = inFile.tellg();
			inFile.seekg(0, inFile.beg);

			buffer = new char[length + 1]; // 

										   // read data as a block:
			inFile.read(buffer, length);
			inFile.seekg(0, inFile.end);
			char temp;
			inFile >> temp;

			buffer[length] = '\0';

			start[0] = 0;

			for (int i = 1; i < thr_num; ++i) {
				start[i] = length / thr_num * i;
				for (int x = start[i]; x <= length; ++x) {
					if ('\r' == (buffer[x]) || '\n' == buffer[x] || '\0' == buffer[x]) {
						start[i] = x;
						//	std::cout << "start " << start[i] << std::endl;
						break;
					}
				}
			}
			for (int i = 0; i < thr_num - 1; ++i) {
				last[i] = start[i + 1] - 1;
				for (int x = last[i]; x <= length; ++x) {
					if ('\r' == (buffer[x]) || '\n' == buffer[x] || '\0' == buffer[x]) {
						last[i] = x;
						//	std::cout << "start " << start[i] << std::endl;
						break;
					}
				}
			}
			last[thr_num - 1] = length;
			//	std::cout << last[thr_num - 1] << std::endl;
		}
		else {
			inFile.seekg(0, inFile.end);
			const unsigned long long length = inFile.tellg();
			inFile.seekg(0, inFile.beg);

			buffer = new char[length + 1]; // 

										   // read data as a block:
			inFile.read(buffer, length);
			buffer[length] = '\0';

			inFile.seekg(0, inFile.end);
			char temp;
			inFile >> temp;

			start[0] = 0;
			last[0] = length;
		}

		//int a = clock();

		if (thr_num > 0) {
			std::vector<VECTOR<Token2>> partial_list(thr_num, VECTOR<Token2>());
			std::vector<std::thread> thr(thr_num);

			for (int i = 0; i < thr_num; ++i) {
				//	std::cout << last[i] - start[i] << std::endl;
				partial_list[i].reserve((last[i] - start[i]) / 10);
				thr[i] = std::thread(DoThread3(buffer + start[i], buffer + last[i], &partial_list[i], &option));
			}

			for (int i = 0; i < thr_num; ++i) {
				thr[i].join();
			}

			int new_size = aq->size() + 2;
			for (int i = 0; i < thr_num; ++i) {
				new_size = new_size + partial_list[i].size();
			}

			aq->reserve(new_size);

			for (int i = 0; i < thr_num; ++i) {
				aq->insert(aq->end(), std::make_move_iterator(partial_list[i].begin()), std::make_move_iterator(partial_list[i].end()));
			}
		}
		else {
			VECTOR<Token2> temp;

			temp.reserve((last[0] - start[0]) / 10);

			DoThread3 dothr(buffer + start[0], buffer + last[0], &temp, &option);

			dothr();

			aq->insert(aq->end(), make_move_iterator(temp.begin()), make_move_iterator(temp.end()));
		}

		//delete[] buffer;

		log_result = log_result + clock() - a;
		//std::cout << "lexing " << clock() - a << "ms" << std::endl;

		_buffer = buffer;

		return{ true, 1 };
	}

	bool __LoadData3(VECTOR<Token2>* _strVec, int start_idx, int last_idx, UserType* _global, const wiz::LoadDataOption* _option,
		int start_state, int last_state, UserType** next) // first, strVec.empty() must be true!!
	{
		if (start_idx > last_idx) {
			return false;
		}

		VECTOR<Token2>& strVec = *_strVec;
		UserType& global = *_global;
		const wiz::LoadDataOption& option = *_option;

		int state = start_state;
		int braceNum = 0;
		std::vector< UserType* > nestedUT(1);
		std::string var1, var2, val;


		nestedUT[0] = &global;

		//	for (int i = 0; i < strVec.size(); ++i) {
		//		cout << strVec[i].str << endl;
		//	}
		int i = start_idx;

		while (false == strVec.empty() && i <= last_idx) {
			
		//	for (const char* x = strVec[i].str; x < strVec[i].str + strVec[i].len; x = x + 1)
		//	{
		//		std::cout << (*x);
		//	}
		//	std::cout << std::endl;
		//	_getch();

			switch (state)
			{
			case 0:
			{
				const std::string top(strVec[i].str, strVec[i].len);
				if (top.size() == 1 && -1 != Equal(option.Left, top[0])) {
					state = 1;
				}
				else {
					std::pair<bool, std::string> bsPair;
					if (i < last_idx) {
						bsPair = std::make_pair(true, std::string(strVec[i + 1].str, strVec[i + 1].len));
					}
					else {
						bsPair = std::make_pair(false, "");
					}

					if (bsPair.first) {
						if (bsPair.second.size() == 1 && -1 != Equal(option.Assignment, bsPair.second[0])) {
							var2 = std::string(strVec[i].str, strVec[i].len); // Utility::Pop(strVec, &var2, nestedUT[braceNum], option);
																			  //Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
							state = 1;
							i += 2;
						}
						else {
							if (i <= last_idx) {
								//Utility::Pop(strVec, &var1, nestedUT[braceNum], option)) {
								var1 = std::string(strVec[i].str, strVec[i].len);

								nestedUT[braceNum]->AddItem("", move(var1));
								state = 0;
								i += 1;
							}
						}
					}
					else {
						if (i <= last_idx) {
							//Utility::Pop(strVec, &var1, nestedUT[braceNum], option)) {
							var1 = std::string(strVec[i].str, strVec[i].len);

							nestedUT[braceNum]->AddItem("", move(var1));
							i += 1;
							state = 0;
						}
					}
				}
			}
			break;
			case 1:
			{
				const std::string top(strVec[i].str, strVec[i].len);
				if (top.size() == 1 && -1 != Equal(option.Left, top[0])) {
					//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
					i += 1;

					///
					nestedUT[braceNum]->AddUserTypeItem(UserType(var2));
					UserType* pTemp = nullptr;
					nestedUT[braceNum]->GetLastUserTypeItemRef(var2, pTemp);

					braceNum++;

					/// new nestedUT
					if (nestedUT.size() == braceNum) /// changed 2014.01.23..
						nestedUT.push_back(nullptr);

					/// initial new nestedUT.
					nestedUT[braceNum] = pTemp;
					///
					state = 2;
				}
				else {
					if (i <= last_idx) {
						//Utility::Pop(strVec, &val, nestedUT[braceNum], option)) {
						val = std::string(strVec[i].str, strVec[i].len);

						nestedUT[braceNum]->AddItem(move(var2), move(val));
						i += 1;
						var2 = "";
						val = "";

						state = 0;
					}
				}
			}
			break;
			case 2:
			{
				const std::string top(strVec[i].str, strVec[i].len);
				if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
					//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
					i += 1;

					if (braceNum == 0) {
						UserType ut;
						ut.AddUserTypeItem(UserType("#"));
						UserType* pTemp = nullptr;
						ut.GetLastUserTypeItemRef("#", pTemp);
						int utCount = 0;
						int itCount = 0;

						for (auto i = 0; i < nestedUT[braceNum]->GetIListSize(); ++i) {
							if (nestedUT[braceNum]->IsUserTypeList(utCount)) {
								ut.AddUserTypeItem(std::move(*(nestedUT[braceNum]->GetUserTypeList(utCount))));
								utCount++;
							}
							else {
								ut.AddItemList(std::move(nestedUT[braceNum]->GetItemList(itCount)));
								itCount++;
							}
						}

						nestedUT[braceNum]->Remove();
						nestedUT[braceNum]->AddUserTypeItem(std::move(*(ut.GetUserTypeList(0))));

						braceNum++;
					}

					if (braceNum < nestedUT.size()) {
						nestedUT[braceNum] = nullptr;
					}
					braceNum--;

					state = 0;
				}
				else {
					{
						/// uisng struct
						//state_reserve++;
						state = 3;
					}
					//else
					{
						//	throw  "syntax error 2 ";
					}
				}
			}
			break;
			case 3:
			{
				//const std::string top(strVec[i].str, strVec[i].len);

				if (strVec[i].len == 1 && -1 != Equal(option.Left, strVec[i].str[0])) {
					//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
					i += 1;
					UserType temp("");

					nestedUT[braceNum]->AddUserTypeItem(temp);
					UserType* pTemp = nullptr;
					nestedUT[braceNum]->GetLastUserTypeItemRef("", pTemp);

					braceNum++;

					/// new nestedUT
					if (nestedUT.size() == braceNum) /// changed 2014.01.23..
						nestedUT.push_back(nullptr);

					/// initial new nestedUT.
					nestedUT[braceNum] = pTemp;
					///
					//}


					state = 4;

					// test for reserve.
					long long count_brace = 1;
					long long count_it = 0;
					long long count_ut = 0;

					for (size_t x = i + 1; x < strVec.size(); ++x) {
						if (count_brace == 0) {
							break;
						}
						if (-1 != Equal(option.Left, strVec[x].str[0]) && strVec[x].len == 1) {
							break;
						}
						else if (-1 != Equal(option.Right, strVec[x].str[0]) && strVec[x].len == 1) {
							break;
						}
						count_it++;
					}
					nestedUT[braceNum]->ReserveItemList(count_it);
				}
				else if (strVec[i].len == 1 && -1 != Equal(option.Right, strVec[i].str[0])) {
					//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
					i += 1;

					state = 3;
					//state = isState0(state_reserve) ? 0 : 4;
					//state_reserve--;


					if (braceNum == 0) {
						UserType ut;
						ut.AddUserTypeItem(UserType("#"));
						UserType* pTemp = nullptr;
						ut.GetLastUserTypeItemRef("#", pTemp);
						int utCount = 0;
						int itCount = 0;

						for (auto i = 0; i < nestedUT[braceNum]->GetIListSize(); ++i) {
							if (nestedUT[braceNum]->IsUserTypeList(utCount)) {
								ut.AddUserTypeItem(std::move(*(nestedUT[braceNum]->GetUserTypeList(utCount))));
								utCount++;
							}
							else {
								ut.AddItemList(std::move(nestedUT[braceNum]->GetItemList(itCount)));
								itCount++;
							}
						}

						nestedUT[braceNum]->Remove();
						nestedUT[braceNum]->AddUserTypeItem(std::move(*(ut.GetUserTypeList(0))));

						braceNum++;
					}

					{
						if (braceNum < nestedUT.size()) {
							nestedUT[braceNum] = nullptr;
						}
						braceNum--;
					}
				}
				else {
					std::pair<bool, Token2> bsPair;

					if (i < last_idx)
					{
						bsPair = std::make_pair(true, strVec[i+1]);
					}
					else {
						bsPair = std::make_pair(false, Token2());
					}

					if (bsPair.first) {
						if (bsPair.second.len == 1 && -1 != Equal(option.Assignment, bsPair.second.str[0])) {
							// var2
							//Utility::Pop(strVec, &var2, nestedUT[braceNum], option);
							//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option); // pass EQ_STR
							var2 = std::string(strVec[i].str, strVec[i].len);
							state = 5;
							i += 1;
							i += 1;
						}
						else {
							// var1
							if (i <= last_idx) {
								//Utility::Pop(strVec, &var1, nestedUT[braceNum], option)) {
								var1 = std::string(strVec[i].str, strVec[i].len);

								nestedUT[braceNum]->AddItem("", move(var1));
								var1 = "";

								state = 3;
								i += 1;
							}
						}
					}
					else
					{
						// var1
						if (i <= last_idx) //Utility::Pop(strVec, &var1, nestedUT[braceNum], option))
						{
							var1 = std::string(strVec[i].str, strVec[i].len);
							nestedUT[braceNum]->AddItem("", move(var1));
							var1 = "";

							state = 3;
							i += 1;
						}
					}
				}
			}
			break;
			case 4:
			{
				//const std::string top(strVec[i].str, strVec[i].len);
				if (strVec[i].len == 1 && -1 != Equal(option.Right, strVec[i].str[0])) {
					//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);

					if (braceNum == 0) {
						UserType ut;
						ut.AddUserTypeItem(UserType("#"));
						UserType* pTemp = nullptr;
						ut.GetLastUserTypeItemRef("#", pTemp);
						int utCount = 0;
						int itCount = 0;

						for (auto i = 0; i < nestedUT[braceNum]->GetIListSize(); ++i) {
							if (nestedUT[braceNum]->IsUserTypeList(utCount)) {
								ut.AddUserTypeItem(std::move(*(nestedUT[braceNum]->GetUserTypeList(utCount))));
								utCount++;
							}
							else {
								ut.AddItemList(std::move(nestedUT[braceNum]->GetItemList(itCount)));
								itCount++;
							}
						}

						nestedUT[braceNum]->Remove();
						nestedUT[braceNum]->AddUserTypeItem(std::move(*(ut.GetUserTypeList(0))));

						braceNum++;
					}

					//if (flag1 == 0) {
					if (braceNum < nestedUT.size()) {
						nestedUT[braceNum] = nullptr;
					}
					braceNum--;
					// }
					//
					state = 3;
					i += 1;
				}
				else {
					int idx = -1;
					int num = -1;


					{
						/// uisng struct
						//state_reserve++;
						state = 3;
					}
					//else
					{
						//	throw "syntax error 4  ";
					}
				}
			}
			break;
			case 5:
			{
				//std::string top(strVec[i].str, strVec[i].len);
				if (strVec[i].len == 1 && -1 != Equal(option.Left, strVec[i].str[0])) {
					//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
					i += 1;
					///
					{
						nestedUT[braceNum]->AddUserTypeItem(UserType(var2));
						UserType* pTemp = nullptr;
						nestedUT[braceNum]->GetLastUserTypeItemRef(var2, pTemp);
						var2 = "";
						braceNum++;

						/// new nestedUT
						if (nestedUT.size() == braceNum) /// changed 2014.01.23..
							nestedUT.push_back(nullptr);

						/// initial new nestedUT.
						nestedUT[braceNum] = pTemp;


					}
					///
					state = 6;
				}
				else {
					if (i <= last_idx) { //Utility::Pop(strVec, &val, nestedUT[braceNum], option)) {
						val = std::string(strVec[i].str, strVec[i].len);

						i += 1;
						nestedUT[braceNum]->AddItem(move(var2), move(val));
						var2 = ""; val = "";

						const std::string top = std::string(strVec[i].str, strVec[i].len);

						if (strVec.empty())
						{
							//
						}
						else if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
							//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
							i += 1;

							if (braceNum == 0) {
								UserType ut;
								ut.AddUserTypeItem(UserType("#"));
								UserType* pTemp = nullptr;
								ut.GetLastUserTypeItemRef("#", pTemp);
								int utCount = 0;
								int itCount = 0;

								for (auto i = 0; i < nestedUT[braceNum]->GetIListSize(); ++i) {
									if (nestedUT[braceNum]->IsUserTypeList(utCount)) {
										ut.AddUserTypeItem(std::move(*(nestedUT[braceNum]->GetUserTypeList(utCount))));
										utCount++;
									}
									else {
										ut.AddItemList(std::move(nestedUT[braceNum]->GetItemList(itCount)));
										itCount++;
									}
								}

								nestedUT[braceNum]->Remove();
								nestedUT[braceNum]->AddUserTypeItem(std::move(*(ut.GetUserTypeList(0))));

								braceNum++;
							}
							{
								state = 3;
								//state = isState0(state_reserve) ? 0 : 4;
								//state_reserve--;

								{
									if (braceNum < nestedUT.size()) {
										nestedUT[braceNum] = nullptr;
									}
									braceNum--;
								}
							}
							{
								//state = 3;
							}
						}
						else {
							state = 3;
						}
					}
				}
			}
			break;
			case 6:
			{
				//const std::string top(strVec[i].str, strVec[i].len);
				if (strVec[i].len == 1 && -1 != Equal(option.Right, strVec[i].str[0])) {
					//Utility::Pop(strVec, nullptr, nestedUT[braceNum], option);
					//
					i += 1;

					if (braceNum == 0) {
						UserType ut;
						ut.AddUserTypeItem(UserType("#"));
						UserType* pTemp = nullptr;
						ut.GetLastUserTypeItemRef("#", pTemp);
						int utCount = 0;
						int itCount = 0;

						for (auto i = 0; i < nestedUT[braceNum]->GetIListSize(); ++i) {
							if (nestedUT[braceNum]->IsUserTypeList(utCount)) {
								ut.AddUserTypeItem(std::move(*(nestedUT[braceNum]->GetUserTypeList(utCount))));
								utCount++;
							}
							else {
								ut.AddItemList(std::move(nestedUT[braceNum]->GetItemList(itCount)));
								itCount++;
							}
						}

						nestedUT[braceNum]->Remove();
						nestedUT[braceNum]->AddUserTypeItem(std::move(*(ut.GetUserTypeList(0))));

						braceNum++;
					}
					if (braceNum < nestedUT.size()) {
						nestedUT[braceNum] = nullptr;
					}
					braceNum--;
					//
					state = 3;
				}
				else {
					int idx = -1;
					int num = -1;

					{
						/// uisng struct
						//state_reserve++;

						state = 3;
					}
					//else
					{
						//throw "syntax error 5 ";
					}
				}
			}
			break;
			default:
				// syntax err!!

				throw "syntax error 6 ";
				break;
			}
		}

		if (next) {
			*next = nestedUT[braceNum];
		}
		
		if (state != last_state) { // 0 : empty or 4 : else?
			throw std::string("error final state is not last_state!  : ") + toStr(state);
		}
		if (i > last_idx + 1) {
			throw std::string("error i > last_idx + 1: " + toStr(i) + " , " + toStr(last_idx));
		}
		return true;
	}

	int FindRight(VECTOR<Token2>& strVec, int start, int last, const wiz::LoadDataOption& option)
	{
		for (int i = last; i >= start; --i) {
			if (strVec[i].len == 1 && -1 != Equal(option.Right, strVec[i].str[0])) {
				return i;
			}
		}

		return -1;
	}
	void Merge(UserType* next, UserType* ut, UserType** ut_next)
	{
		//check!!
		while (ut->GetIListSize() >= 1 && ut->GetUserTypeListSize() >= 1
			&& (ut->GetUserTypeList(0)->GetName() == "#"))
		{
			ut = ut->GetUserTypeList(0);
		}

		//int chk = 0;
		while (true) {
			int itCount = 0;
			int utCount = 0;

			UserType* _ut = ut;
			UserType* _next = next;

			if (ut_next && _ut == *ut_next) {
				*ut_next = _next;
			}

			for (int i = 0; i < _ut->GetIListSize(); ++i) {
				if (_ut->IsUserTypeList(i)) {
					if (_ut->GetUserTypeList(utCount)->GetName() == "#") {
						_ut->GetUserTypeList(utCount)->SetName("");
					}
					else {
						{
							_next->LinkUserType(_ut->GetUserTypeList(utCount));
							_ut->GetUserTypeList(utCount) = nullptr;
						}
						//chk++;
					}
					utCount++;
				}
				else if (_ut->IsItemList(i)) {
					_next->AddItemList(std::move(_ut->GetItemList(itCount)));
					itCount++;
				}
			}
			_ut->Remove();
			if (ut->GetParent() && next->GetParent()) {
				ut = ut->GetParent();
				next = next->GetParent();
			}
			else {
				break;
			}
		}

	}

	
	//// ToDo : # => option.~?
	template <class Reserver>
	bool _LoadData3(VECTOR<Token2>& strVec, Reserver& reserver, UserType& global, const wiz::LoadDataOption& option, const int lex_thr_num, const int parse_num) // first, strVec.empty() must be true!!
	{
		const int pivot_num = parse_num - 1;
		char left[] = "{";
		char right[] = "}";
		char* buffer = nullptr;

		strVec.emplace_back(left, 1, false);
		strVec.emplace_back(left, 1, false);

		bool end = false;
		{
			end = !reserver(&strVec, option, lex_thr_num, buffer);
			/*
			while (strVec.empty())
			{
			end = !reserver(&strVec, option, lex_thr_num);
			if (
			strVec.empty() &&
			end
			) {
			return false; // throw "Err nextToken2 does not exist"; // cf) or empty file or empty std::string!
			}
			}
			*/
		}

		UserType* before_next = nullptr;
		UserType _global;

		bool first = true;
		int sum = 0;

		while (true) {
			end = true; //!reserver(&strVec, option, lex_thr_num);

			if (end) {
				strVec.emplace_back(right, 1, false);
				strVec.emplace_back(right, 1, false);
			}

			int a = clock();


			std::set<int> _pivots;
			std::vector<int> pivots;
			const int last_idx = FindRight(strVec, 0, strVec.size() - 1, option);

			if (pivot_num > 0) {
				//int c1 = clock();
				std::vector<int> pivot;

				for (int i = 0; i < pivot_num; ++i) {
					pivot.push_back(FindRight(strVec, (last_idx / (pivot_num + 1)) * (i), (last_idx / (pivot_num + 1)) * (i + 1) - 1, option));
				}

				for (int i = 0; i < pivot.size(); ++i) {
					if (pivot[i] != -1) {
						_pivots.insert(pivot[i]);
					}
				}

				//std::cout << _pivots.size() << std::endl;
				for (auto& x : _pivots) {
					pivots.push_back(x);
					//	std::cout << x << " ";
				}
				//std::cout << last_idx << std::endl;
				//std::cout << std::endl;
			}


			//std::cout << pivot << "\t" << pivot3 << "\t" << pivot2 << "\t" << last_idx << "\t" << strVec.size() - 1 << std::endl;
			//std::cout << strVec[pivot + 1].str << "\t";
			//std::cout << strVec[pivot3 + 1].str << "\t";
			//std::cout << strVec[pivot2 + 1].str << "\t";
			//std::cout << std::endl;


			std::vector<UserType*> next(pivots.size() + 1, nullptr);

			{
				std::vector<UserType> __global(pivots.size() + 1);

				std::vector<std::thread> thr(pivots.size() + 1);

				if (first) {
					int idx = pivots.empty() ? last_idx : pivots[0];
					thr[0] = std::thread(__LoadData3, &strVec, 0, idx, &__global[0], &option, 0, 3, &next[0]);
				}
				else {
					int idx = pivots.empty() ? last_idx : pivots[0];
					thr[0] = std::thread(__LoadData3, &strVec, 0, idx, &__global[0], &option, 3, 3, &next[0]);
				}

				for (int i = 1; i < pivots.size(); ++i) {
					thr[i] = std::thread(__LoadData3, &strVec, pivots[i - 1] + 1, pivots[i], &__global[i], &option, 3, 3, &next[i]);
				}

				if (pivots.size() >= 1) {
					thr[pivots.size()] = std::thread(__LoadData3, &strVec, pivots.back() + 1, last_idx, &__global[pivots.size()],
						&option, 3, 3, &next[pivots.size()]);
				}

				// wait
				for (int i = 0; i < thr.size(); ++i) {
					thr[i].join();
				}

				//Merge
				//int merge_start = clock();

				if (first) {
					Merge(&_global, &__global[0], &next[0]);
					first = false;
				}
				else {
					Merge(before_next, &__global[0], &next[0]);
				}

				for (int i = 1; i < pivots.size() + 1; ++i) {
					Merge(next[i - 1], &__global[i], &next[i]);
				}

				//std::cout << "merge " << clock() - merge_start << "ms" << std::endl;

				//int pop_start = clock();
				//
				//for (int i = 0; i <= last_idx; ++i) {
				//	strVec.pop_front();
				//}

				strVec.erase(strVec.begin(), strVec.begin() + last_idx + 1);

				//std::cout << "pop " << clock() - pop_start << "ms" << std::endl;

				before_next = next.back();
			}

			int b = clock();
			sum += b - a; //std::cout << b - a << "ms" << std::endl;

			if (!end) { // end : true, !end : false
						/*
						end = !reserver(&strVec, option, lex_thr_num);

						while (!end && strVec.empty())
						{
						end = !reserver(&strVec, option, lex_thr_num);
						if (
						strVec.empty() &&
						end
						) {
						break; // throw "Err nextToken2 does not exist"; // cf) or empty file or empty std::string!
						}
						}*/
			}
			else {
				break;
			}
		}

		delete[] buffer;

		global = std::move(*(_global.GetUserTypeList(0)->GetUserTypeList(0)));

		std::cout << "lexing " << log_result << "ms" << std::endl;
		log_result = 0;
		std::cout << "parse " << sum << "ms" << std::endl;
		return true;
	}

	bool LoadDataFromFileFastJson(const std::string& fileName, UserType& global, int lex_thr_num, int parse_num) /// global should be empty
	{
		bool success = true;
		std::ifstream inFile;
		inFile.open(fileName, std::ios::binary);


		if (true == inFile.fail())
		{
			inFile.close(); return false;
		}
		UserType globalTemp;
		VECTOR<Token2> strVec; // remove - O(n) ?

		try {
			InFileReserver3 ifReserver(inFile);
			wiz::LoadDataOption option;
			option.Removal.push_back(',');
			option.Assignment.push_back(':');
			option.Left.push_back('{');
			option.Left.push_back('[');
			option.Right.push_back('}');
			option.Right.push_back(']');
			//option.MuitipleLineCommentStart.push_back("###");
			//option.MuitipleLineCommentEnd.push_back("###");
			//option.LineComment.push_back("#");

			ifReserver.Num = 10000;
			//	strVec.reserve(ifReserver.Num);
			// cf) empty file..
			if (false == _LoadData3(strVec, ifReserver, globalTemp, option, lex_thr_num, parse_num))
			{
				inFile.close();
				return false; // return true?
			}

			inFile.close();
		}
		catch (const char* err) { std::cout << err << std::endl; inFile.close(); return false; }
		catch (const std::string& e) { std::cout << e << std::endl; inFile.close(); return false; }
		catch (std::exception e) { std::cout << e.what() << std::endl; inFile.close(); return false; }
		catch (...) { std::cout << "not expected error" << std::endl; inFile.close(); return false; }

		global = std::move(globalTemp);

		return true;
	}

	bool SaveWizDB(const UserType& global, const std::string& fileName, const std::string& option, const std::string& option2) { /// , int option
		std::ofstream outFile;
		if (fileName.empty()) { return false; }
		if (option2 == "") {
			outFile.open(fileName);
			if (outFile.fail()) { return false; }
		}
		else {
			outFile.open(fileName, std::ios::app);
			if (outFile.fail()) { return false; }

			outFile << "\n";
		}

		if (option == "1") {
			global.SaveWithJson(outFile);
		}


		outFile.close();

		return true;
	}

	bool LoadDataFromFile3(const std::string& fileName, UserType& global, int pivot_num, int lex_thr_num) /// global should be empty
	{
		bool success = true;
		std::ifstream inFile;
		inFile.open(fileName, std::ios::binary);


		if (true == inFile.fail())
		{
			inFile.close(); return false;
		}
		UserType globalTemp;
		static VECTOR<Token2> strVec;

		try {
			InFileReserver3 ifReserver(inFile);
			wiz::LoadDataOption option;
			option.Assignment.push_back('=');
			option.Left.push_back('{');
			option.Right.push_back('}');
			//option.MuitipleLineCommentStart.push_back("###");
			//option.MuitipleLineCommentEnd.push_back("###");
			//option.LineComment.push_back("#");

			ifReserver.Num = 1 << 19;
			//	strVec.reserve(ifReserver.Num);
			// cf) empty file..
			if (false == _LoadData3(strVec, ifReserver, globalTemp, option, pivot_num, lex_thr_num))
			{
				inFile.close();
				return false; // return true?
			}

			inFile.close();
		}
		catch (const char* err) { std::cout << err << std::endl; inFile.close(); return false; }
		catch (const std::string& e) { std::cout << e << std::endl; inFile.close(); return false; }
		catch (std::exception e) { std::cout << e.what() << std::endl; inFile.close(); return false; }
		catch (...) { std::cout << "not expected error" << std::endl; inFile.close(); return false; }

		global = std::move(globalTemp);
		return true;
	}
}

