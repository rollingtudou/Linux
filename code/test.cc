// #include <iostream>
// #include <string>
// using namespace std;

// class StInfo {
// public:
//     int SID;
//     string name, cla, phone;

//     // 构造函数，初始化成员变量
//     StInfo() : SID(0), name(""), cla(""), phone("") {}

//     void SetInfo(int sid, string name, string cla, string phone);
//     void PrintInfo();
// };

// void StInfo::SetInfo(int sid, string name, string cla, string phone) {
//     SID = sid;
//     this->name = name;
//     this->cla = cla;
//     this->phone = phone;
// }

// void StInfo::PrintInfo() {
//     cout << "学号:" << SID << "\n姓名:" << name << "\n班级:" << cla << "\n手机号:" << phone << endl;
// }

// int main() {
//     int SID;
//     string name, cla, phone;

//     // 输入 SID
//     cout << "请输入学号: ";
//     cin >> SID;
//     cin.ignore(); // 清除换行符，避免影响 getline

//     // 使用 getline 获取整行输入，支持空格
//     cout << "请输入姓名: ";
//     getline(cin, name);
//     cout << "请输入班级: ";
//     getline(cin, cla);
//     cout << "请输入手机号: ";
//     getline(cin, phone);

//     StInfo A;
//     A.SetInfo(SID, name, cla, phone);
//     A.PrintInfo();

//     return 0;
// }


//念数字
//思路：1，先判断正负性，如果是负数，先输出“fu"，并将数字去绝对值。2，将整数准成所对应的字符串。3，便利字符串中的每一个字符，通过数字字符减去字符“0”得到对应的数值，再根据数值输出对应的拼音
#include<iostream>
#include<string>
using namespace std;
int main()
{
	string num;
	cin >> num;
	//用数组检查，数组的第一位是不是符号“-”，是，休闲输出fu，让后使用substr函数从数组的第二位开始截取，去掉“—”，，跟新num为去掉符号的字符串
	if (num[0] == '-') {
 		cout << "fu";
		num=num.substr(1);
	}
	//定义拼音数组,用来存储0到9
	string pinyin[]={"li","xi","nan","wu","ku","qin","bai","bo","qian","qi"};
		//遍历字符串
	for (size_t i = 0; i < num.length(); ++i)
	{
		char c = num[i];
		int digit = c - '0';
		if (i != 0) {
			cout << " ";
		}
		cout << pinyin[digit];
	}
	return 0;
}