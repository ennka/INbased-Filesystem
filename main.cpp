#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
/*
#include "INode.h"
#include "Superblock.h"
#include "File.h"
*/
#include "Filesystem.h"
//#include "Address.h"

using namespace std;

int strToNum(string str) {
	
	int length = (int)str.size();
	int result = 0;
	for (int i = 0; i < length; ++i) {
		if (str[i] >= '0' && str[i] <= '9')
		{
			int num= str[i] - '0';
			result = result * 10 + num;
		}
		else return -1;
	}
	return result;
}

int main() {
	System system;
	system.initialize();
	system.copyright();
	if (!system.flag)return 0;
	while (1) {
		system.tip();
		string str;
		
		getline(cin, str);
		int len = str.size();

		vector<string> cmd;
		string temp = "";
		for (int i = 0; i < len; ++i) {
			if (str[i] == ' ') {
				if (temp != "") {
					cmd.push_back(temp);
					temp = "";
				}
				continue;
			}
			temp += str[i];
		}
		if (temp != "") cmd.push_back(temp);
		int s = cmd.size();
		if (s == 0) continue;

		if (cmd[0] == "createFile") {
			if (s !=3) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else {
				int res = strToNum(cmd[2]);
				if (res >= 0) system.result(cmd[0], system.createFile(cmd[1], res));
				else cout << "Command \'" << cmd[0] << "\': please enter a number to represent file size" << endl;
			}
		}
		else if (cmd[0] == "deleteFile") {
			if (s != 2) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.result(cmd[0], system.deleteFile(cmd[1]));
		}
		else if (cmd[0] == "createDir") {
			if (s != 2) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.result(cmd[0], system.createDir(cmd[1]));
		}
		else if (cmd[0] == "deleteDir") {
			if (s != 2) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.result(cmd[0], system.deleteDir(cmd[1]));
		}
		else if (cmd[0] == "changeDir") {
			if (s != 2) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.result(cmd[0], system.changeDir(cmd[1]));
		}
		else if (cmd[0] == "dir") {
			if (s > 1) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.dir();
		}
		else if (cmd[0] == "cp") {
			if (s != 3) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.result(cmd[0], system.cp(cmd[1], cmd[2]));
		}
		else if (cmd[0] == "sum") {
			if (s > 1) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.sum();
		}
		else if (cmd[0] == "cat") {
			if (s != 2) cout << "Command \'" << cmd[0] << "\': wrong parameterss" << endl;
			else system.result(cmd[0], system.cat(cmd[1]));
		}
		else if (cmd[0] == "help") {
			if (s > 1) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else system.help();
		}
		else if (cmd[0] == "exit") {
			if (s > 1) cout << "Command \'" << cmd[0] << "\': wrong parameters" << endl;
			else break;
		}
		else {
			cout << cmd[0] << ": command not exist" << endl;
		}
	}
	return 0;
}