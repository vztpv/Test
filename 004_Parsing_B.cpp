static bool isState0(const long long state_reserve)
{
	return 1 == state_reserve;
}

template <class Reserver>
static bool _LoadData(ARRAY_QUEUE<Token>& strVec, Reserver& reserver, UserType& global, const wiz::LoadDataOption& option) // first, strVec.empty() must be true!!
{

	int state = 0;
	int braceNum = 0;
	long long state_reserve = 0;
	std::vector< UserType* > nestedUT(1);
	std::string var1, var2, val;

	bool varOn = false;

	nestedUT[0] = &global;
	{
		reserver(strVec, option);

		while (strVec.empty())
		{
			reserver(strVec, option);
			if (
				strVec.empty() &&
				reserver.end()
				) {
				return false; // throw "Err nextToken does not exist"; // cf) or empty file or empty std::string!
			}
		}
	}

	//	for (int i = 0; i < strVec.size(); ++i) {
	//		cout << strVec[i].str << endl;
	//	}

	int sum = 0;
	int a = clock();
	int b;

	while (false == strVec.empty()) {

		//	cout << state << " " << Utility::Top(strVec, nestedUT[braceNum], reserver, option) << endl;

		switch (state)
		{
		case 0:
		{
			const std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Left, top[0])) {
				state = 2;
			}
			else {
				std::pair<bool, std::string> bsPair = Utility::LookUp(strVec, nestedUT[braceNum], reserver, option);
				if (bsPair.first) {
					if (bsPair.second.size() == 1 && -1 != Equal(option.Assignment, bsPair.second[0])) {
						Utility::Pop(strVec, &var2, nestedUT[braceNum], reserver, option);
						Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);
						state = 2;
					}
					else {
						if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver, option)) {
							nestedUT[braceNum]->AddItem("", move(var1));
							state = 0;
						}
					}
				}
				else {
					if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver, option)) {
						nestedUT[braceNum]->AddItem("", move(var1));
						state = 0;
					}
				}
			}
		}
		break;
		case 1:
		{
			const std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);
				state = 0;
			}
			else {
				// syntax error.
				throw "syntax error 1 ";
			}
		}
		break;
		case 2:
		{
			const std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Left, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);

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
				state = 3;
			}
			else {
				if (Utility::Pop(strVec, &val, nestedUT[braceNum], reserver, option)) {

					nestedUT[braceNum]->AddItem(move(var2), move(val));
					var2 = "";
					val = "";

					state = 0;
				}
			}
		}
		break;
		case 3:
		{
			const std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);

				nestedUT[braceNum] = nullptr;
				braceNum--;

				state = 0;
			}
			else {
				{
					/// uisng struct
					state_reserve++;
					state = 4;
				}
				//else
				{
					//	throw  "syntax error 2 ";
				}
			}
		}
		break;
		case 4:
		{
			const std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Left, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);

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

				state = 5;
			}
			else if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);
				state = isState0(state_reserve) ? 0 : 4;
				state_reserve--;

				{
					nestedUT[braceNum] = nullptr;
					braceNum--;
				}
			}
			else {
				std::pair<bool, std::string> bsPair = Utility::LookUp(strVec, nestedUT[braceNum], reserver, option);
				if (bsPair.first) {
					if (bsPair.second.size() == 1 && -1 != Equal(option.Assignment, bsPair.second[0])) {
						// var2
						Utility::Pop(strVec, &var2, nestedUT[braceNum], reserver, option);
						Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option); // pass EQ_STR
						state = 6;
					}
					else {
						// var1
						if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver, option)) {
							nestedUT[braceNum]->AddItem("", move(var1));
							var1 = "";

							state = 4;
						}
					}
				}
				else
				{
					// var1
					if (Utility::Pop(strVec, &var1, nestedUT[braceNum], reserver, option))
					{
						nestedUT[braceNum]->AddItem("", move(var1));
						var1 = "";

						state = 4;
					}
				}
			}
		}
		break;
		case 5:
		{
			const std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);

				//if (flag1 == 0) {
				nestedUT[braceNum] = nullptr;
				braceNum--;
				// }
				//
				state = 4;
			}
			else {
				int idx = -1;
				int num = -1;


				{
					/// uisng struct
					state_reserve++;
					state = 4;
				}
				//else
				{
					//	throw "syntax error 4  ";
				}
			}
		}
		break;
		case 6:
		{
			std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Left, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);

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
				state = 7;
			}
			else {
				if (Utility::Pop(strVec, &val, nestedUT[braceNum], reserver, option)) {

					nestedUT[braceNum]->AddItem(move(var2), move(val));
					var2 = ""; val = "";

					top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);

					if (strVec.empty())
					{
						//
					}
					else if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
						Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);

						{
							state = isState0(state_reserve) ? 0 : 4;
							state_reserve--;

							{
								nestedUT[braceNum] = nullptr;
								braceNum--;
							}
						}
						{
							//state = 4;
						}
					}
					else {
						state = 4;
					}
				}
			}
		}
		break;
		case 7:
		{
			const std::string top = Utility::Top(strVec, nestedUT[braceNum], reserver, option);
			if (top.size() == 1 && -1 != Equal(option.Right, top[0])) {
				Utility::Pop(strVec, nullptr, nestedUT[braceNum], reserver, option);
				//

				nestedUT[braceNum] = nullptr;
				braceNum--;
				//
				state = 4;
			}
			else {
				int idx = -1;
				int num = -1;

				{
					/// uisng struct
					state_reserve++;

					state = 4;
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

		if (strVec.size() < 10) {
			b = clock();
			sum += b - a;
			//std::cout << b - a << "ms" << std::endl;

			reserver(strVec, option);

			while (strVec.empty()) // (strVec.empty())
			{
				reserver(strVec, option);
				if (
					strVec.empty() &&
					reserver.end()
					) {
					// throw "Err nextToken does not exist2";
					break;
				}
			}
			a = clock();
		}
	}
	if (state != 0) {
		throw std::string("error final state is not 0!  : ") + toStr(state);
	}
	if (braceNum != 0) {
		throw std::string("chk braceNum is ") + toStr(braceNum);
	}
	 b = clock();

	//std::cout << sum << "ms" << std::endl;
	return true;
}
