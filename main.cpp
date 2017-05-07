#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cctype>
#include <algorithm>

#define CSV_TOKEN ','
#define TSV_TOKEN '\t'

using namespace std;

// XXX (int a, int b)
// XXX
// short XXX
// XXX(int a, int b)

// �O��
// �@ ���O��" (" �� "("�Œu������B
// �A �I�[�o�[���[�h�͖��Ή�
// �B ���W���[���O������
// �C excel�ȂǗp���āA�t�@�C�������Ƀ\�[�g�ς݂ł���
// �D �p���������Ă���ƌ듮�삷��

enum DataType
{
	TYPE_NAMESPACE,
	TYPE_CLASS,
	TYPE_FUNC,
	TYPE_STRUCT,
	TYPE_NUM
};

struct TEC
{
	int LOC;
	string path;
	string name;
	string funcdef;
	DataType type;
};

struct UDB
{
	int LOC;
	string path;
	string name;
	DataType type;
};

// tec�f�[�^��(��)(�t�@�C���p�X)(�֐���`)(LOC)�̏�
bool analyzeTEC(vector<string> line, map<int, vector<TEC>> &TECdata, map<int, string> &TECfiles)
{
	// �ǂݍ��񂾃f�[�^���擾
	TEC tmp;
	tmp.path = line[1];
	tmp.LOC = stoi(line[3]);
	tmp.funcdef = line[2];

	///////////////////////////////////////////////
	// �֐���`�𕪉����āA�֐��������o���B
	///////////////////////////////////////////////
	// c#
	if (strstr(tmp.path.c_str(), ".cs") != NULL)
	{
		istringstream iss(tmp.funcdef);
		vector<string> funcdefs;
		string token;
		while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

		// ��namespace, class, function��3��ނ���̂ŁA�����𕪉�
		// ���\���̂Ȃǂ�����悤�Ȃ�A���킹�ďC�����K�v
		if (strstr(tmp.funcdef.c_str(), "class ") != NULL)
		{
			tmp.name = funcdefs[funcdefs.size() - 1]; // ���ŏI�v�f�����̂܂܃N���X���ɂȂ��Ă���͂�
			tmp.type = DataType::TYPE_CLASS;
		}
		else if (strstr(tmp.funcdef.c_str(), "namespace ") != NULL)
		{
			tmp.name = funcdefs[funcdefs.size() - 1]; // ���ŏI�v�f�����̂܂�namespace�̖��O�ɂȂ��Ă���͂�
			tmp.type = DataType::TYPE_NAMESPACE;
		}
		else if (strstr(tmp.funcdef.c_str(), "struct ") != NULL)
		{
			tmp.name = funcdefs[funcdefs.size() - 1]; // ���ŏI�v�f�����̂܂�namespace�̖��O�ɂȂ��Ă���͂�
			tmp.type = DataType::TYPE_STRUCT;
		}
		// ��strstr���S�p�����ɑΉ����Ă��邩���s��
		else if (strstr(tmp.funcdef.c_str(), "���W���[���O"))
		{
			cout << "���W���[���O��skip" << endl;
			return false;

		}
		// function
		else
		{
			// ��funcdefs[funcdefs.size() - 1]�́AXXX::xxx(param 1, param2, ...) �̌`���̂͂�(�N���X�����Ă��Ȃ����̂�����)
			// �֐����ƈ����̌�ɉ��������邱�Ƃ͂Ȃ��͂�
			int i;
			for (i = 0; i < funcdefs.size(); i++)
				if (strstr(funcdefs[i].c_str(), "(")) break;


			// ��̃��[�v��break���Ȃ�����(�֐���`�Ɉ������Ȃ������ꍇ)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// �֐����������擾
			std::getline(issFunc, funcname, '(');
			tmp.name = funcname;
			tmp.type = TYPE_FUNC;
		}
	}
	// c++, c, h
	else if (strstr(tmp.path.c_str(), ".h") != NULL || strstr(tmp.path.c_str(), ".c") != NULL)
	{
		if (strstr(tmp.funcdef.c_str(), "���W���[���O"))
		{
			cout << "���W���[���O��skip" << endl;
			return false;

		}
		// function
		else
		{
			istringstream iss(tmp.funcdef);
			vector<string> funcdefs;
			string token;

			while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

			int i;
			for (i = 0; i < funcdefs.size(); i++)
				if (strstr(funcdefs[i].c_str(), "(")) break;


			// ��̃��[�v��break���Ȃ�����(�֐���`�Ɉ������Ȃ������ꍇ)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// �֐����������擾
			std::getline(issFunc, funcname, '(');
			tmp.name = funcname;
			tmp.type = TYPE_FUNC;
		}
	}
	else
	{
		cout << "����`�t�@�C���`��: " << tmp.path << endl;
		return false;
	}

	// �������
	// �t�@�C�������ɖ��o�^
	if (TECfiles.empty() || (--TECfiles.end())->second != tmp.path)
	{
		// TECfiles.size(), tmp.path
		TECfiles.insert(pair<int, string>(TECfiles.size(), tmp.path));

		vector<TEC> input;
		input.push_back(tmp);
		TECdata.insert(pair<int, vector<TEC>>(TECdata.size(), input));
	}
	// ���Ƀt�@�C���p�X��o�^�ς݂̃f�[�^
	else
	{
		// �t�@�C���p�X�����͍X�V�����A�֐������̂ݍX�V
		map<int, string>::iterator cur = TECfiles.end(); 
		cur--;
		int curfileID = cur->first;
		map<int, vector<TEC>>::iterator ite = TECdata.find(curfileID);
		ite->second.push_back(tmp);
	}

	return true;
}

// UDB�f�[�^��(�^�C�v)(�֐���`)(�t�@�C���p�X)(LOC)�̏�
bool analyzeUDB(vector<string> line, map<int, vector<UDB>> &UDBdata, map<int, string> &UDBfiles, const map<int, string> TECfiles)
{
	// �ǂݍ��񂾃f�[�^���擾
	const string type = line[0];

	UDB tmp;
	tmp.name = line[1];

	for (int i = 2; i < line.size(); i++) {
		// �����񂪐�����̏ꍇ
		if (std::all_of(line[i].cbegin(), line[i].cend(), isdigit))
		{
			// 1��ڂ�LOC�Ȃ̂ŁA��x�Ń��[�v�𔲂���
			tmp.path = line[i-1];
			tmp.LOC = stoi(line[i]);
			break;
		}
	}


	////////////////////////////////////////////////
	// TEC���ɂȂ��t�@�C���̃f�[�^�ł����skip����
	////////////////////////////////////////////////
	int counter = 0;
	for each (auto ite in TECfiles)
	{
		if (strcmp(tmp.path.c_str(), ite.second.c_str()) == 0) break;
		else counter++;
	}

	// ��v����t�@�C�����Ȃ�����
	if (counter == TECfiles.size()) return false;

	///////////////////////////////////////////////
	// �֐���`�𕪉����āA�֐��������o���B
	///////////////////////////////////////////////
	// c#
	if (strstr(tmp.path.c_str(), ".cs") != NULL)
	{
		istringstream iss(tmp.name);
		vector<string> funcdefs;
		string token;
		while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

		// ��namespace, class, function��3��ނ���̂ŁA�����𕪉�
		// ���\���̂Ȃǂ�����悤�Ȃ�A���킹�ďC�����K�v
		if (strstr(type.c_str(), "class ") != NULL)
		{
			tmp.name = funcdefs[funcdefs.size() - 1]; // ���ŏI�v�f�����̂܂܃N���X���ɂȂ��Ă���͂�
			tmp.type = DataType::TYPE_CLASS;
		}
		else if (strstr(type.c_str(), "namespace ") != NULL)
		{
			tmp.name = funcdefs[funcdefs.size() - 1]; // ���ŏI�v�f�����̂܂�namespace�̖��O�ɂȂ��Ă���͂�
			tmp.type = DataType::TYPE_NAMESPACE;
		}
		else if (strstr(type.c_str(), "struct ") != NULL)
		{
			tmp.name = funcdefs[funcdefs.size() - 1]; // ���ŏI�v�f�����̂܂�namespace�̖��O�ɂȂ��Ă���͂�
			tmp.type = DataType::TYPE_STRUCT;
		}
		// ��strstr���S�p�����ɑΉ����Ă��邩���s��
		else if (strstr(type.c_str(), "���W���[���O"))
		{
			cout << "���W���[���O��skip" << endl;
			return false;

		}
		// function
		else
		{
			// ��funcdefs[funcdefs.size() - 1]�́AXXX::xxx(param 1, param2, ...) �̌`���̂͂�(�N���X�����Ă��Ȃ����̂�����)
			// �֐����ƈ����̌�ɉ��������邱�Ƃ͂Ȃ��͂�
			int i;
			for (i = 0; i < funcdefs.size(); i++)
				if (strstr(funcdefs[i].c_str(), "(")) break;


			// ��̃��[�v��break���Ȃ�����(�֐���`�Ɉ������Ȃ������ꍇ)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// �֐����������擾
			std::getline(issFunc, funcname, '(');

			// "XXX()"�Ƃ����p�^�[��������̂ŁA����"���O������
			istringstream issFunc2(funcname);
			string funcname2;
			while (std::getline(issFunc2, funcname2, '"'));

			// C#�̊֐�����XXXXX.XXXX.XXX�̂悤�ɂȂ��Ă���̂ŁA.�ł���ɋ�؂�
			istringstream issFunc3(funcname2);
			while (std::getline(issFunc3, funcname, '.'));

			tmp.name = funcname;
			tmp.type = TYPE_FUNC;
		}
	}
	// c++, c, h
	else if (strstr(tmp.path.c_str(), ".h") != NULL || strstr(tmp.path.c_str(), ".c") != NULL)
	{
		if (strstr(tmp.name.c_str(), "���W���[���O"))
		{
			cout << "���W���[���O��skip" << endl;
			return false;

		}
		// function
		else
		{
			istringstream iss(tmp.name);
			vector<string> funcdefs;
			string token;

			while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

			int i;
			for (i = 0; i < funcdefs.size(); i++)
				if (strstr(funcdefs[i].c_str(), "(")) break;


			// ��̃��[�v��break���Ȃ�����(�֐���`�Ɉ������Ȃ������ꍇ)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// �֐����������擾
			std::getline(issFunc, funcname, '(');

			// "XXX()"�Ƃ����p�^�[��������̂ŁA����"���O������
			istringstream issFunc2(funcname);
			string funcname2;
			while (std::getline(issFunc2, funcname2, '"'));

			tmp.name = funcname2;
			tmp.type = TYPE_FUNC;
		}
	}
	else
	{
		cout << "����`�t�@�C���`��: " << tmp.path << endl;
		return false;
	}


	// �������
	// �t�@�C�������ɖ��o�^
	if (UDBfiles.empty() || (--UDBfiles.end())->second != tmp.path)
	{
		// TECfiles.size(), tmp.path
		UDBfiles.insert(pair<int, string>(counter, tmp.path));

		vector<UDB> input;
		input.push_back(tmp);
		UDBdata.insert(pair<int, vector<UDB>>(counter, input));

	}
	// ���Ƀt�@�C���p�X��o�^�ς݂̃f�[�^
	else
	{
		// �t�@�C���p�X�����͍X�V�����A�֐������̂ݍX�V
		map<int, string>::iterator cur = UDBfiles.end();
		cur--;
		int curfileID = cur->first;
		map<int, vector<UDB>>::iterator ite = UDBdata.find(curfileID);
		ite->second.push_back(tmp);
	}

	return true;
}

void loadTEC(map<int, vector<TEC>> &TECdata, map<int, string> &TECfiles, vector<string> &skipTECLines)
{
	// CSV�t�@�C����ǂݍ���
	ifstream ifs("tec.tsv");
	if (!ifs) return;

	string line;
	while (std::getline(ifs, line))
	{
		// �s���^�u��؂�ŕ���
		string token;
		istringstream iss(line);
		vector<string> tokens;
		while (std::getline(iss, token, TSV_TOKEN))
		{
			tokens.push_back(token);
		}

		// ��͂��ăf�[�^��}���ł��Ȃ������ꍇ�Askip�����s�Ƃ��ĕێ�(�Ō�ɏo�͂���)
		if (!analyzeTEC(tokens, TECdata, TECfiles))
		{
			skipTECLines.push_back(line);
		}
	}

	return;
}

void loadUDB(map<int, vector<UDB>> &UDBdata, map<int, string> &UDBfiles, vector<string> &skipUDBLines, const map<int, string> TECfiles)
{
	// CSV�t�@�C����ǂݍ���
	ifstream ifs("udb.csv");
	if (!ifs) return;

	string line;
	while (std::getline(ifs, line))
	{
		// �s���^�u��؂�ŕ���
		string token;
		istringstream iss(line);
		vector<string> tokens;
		while (std::getline(iss, token, CSV_TOKEN))
		{
			tokens.push_back(token);
		}

		// ��͂��ăf�[�^��}���ł��Ȃ������ꍇ�Askip�����s�Ƃ��ĕێ�(�Ō�ɏo�͂���)
		if (!analyzeUDB(tokens, UDBdata, UDBfiles, TECfiles))
		{
			skipUDBLines.push_back(line);
		}
	}

	return;
}

void outputFile(const string filename, const vector<string> input)
{
	ofstream ofs(filename); //�t�@�C���o�̓X�g���[��
	for each (auto target in input)
	{
		ofs << target << endl;
	}
}

void outputFile(const string filename, const map<int, vector<TEC>> input)
{
	ofstream ofs(filename); //�t�@�C���o�̓X�g���[��
	for each (auto target in input)
	{
		for each(auto elem in target.second)
		{
			ofs << elem.path << "," << elem.funcdef << "," << elem.LOC << endl;
		}
	}

}


int main(void)
{
	map<int, vector<TEC>> TECdata, updatedTECdata;
	map<int, vector<UDB>> UDBdata;
	map<int, string> TECfiles;
	map<int, string> UDBfiles;
	vector<string> skipTEClines;
	vector<string> skipUDBlines;

	TECdata.clear();
	UDBdata.clear();
	TECfiles.clear();
	UDBfiles.clear();
	skipTEClines.clear();
	skipUDBlines.clear();

	loadTEC(TECdata, TECfiles, skipTEClines);
	cout << "end load TEC" << endl;

	//for each(auto ite in TECdata) 
	//{
	//	for each (auto iteite in ite.second)
	//	{
	//		cout << "LOC: " << iteite.LOC << ", NAME: " << iteite.name << ", PATH: " << iteite.path << std::endl;
	//	}
	//}

	//for each (auto ite in TECfiles)
	//{
	//	cout << ite.first << ": " << ite.second << std::endl;
	//}

	//cout << "skip" << endl;
	//for each (auto ite in skipTEClines)
	//{
	//	cout << ite << endl;
	//}

	loadUDB(UDBdata, UDBfiles, skipUDBlines, TECfiles);
	cout << "end load UDB" << endl;

	//for each(auto ite in UDBdata)
	//{
	//	for each (auto iteite in ite.second)
	//	{
	//		cout << "LOC: " << iteite.LOC << ", NAME: " << iteite.name << ", PATH: " << iteite.path << std::endl;
	//	}
	//}

	//cout << "skip" << endl;
	//for each (auto ite in skipUDBlines)
	//{
	//	cout << ite << endl;
	//}

	//for each (auto ite in UDBfiles)
	//{
	//	cout << ite.first << ": " << ite.second << std::endl;
	//}


	////////////////////////////////////////////////////////////////////////////
	// LOC�X�V
	////////////////////////////////////////////////////////////////////////////
	for (auto ite = TECdata.begin(); ite != TECdata.end(); ite++)
	{
		// �Ώۃt�@�C���̃f�[�^���擾(
		map<int, vector<UDB>>::iterator targetIte = UDBdata.find(ite->first);
		if (targetIte == UDBdata.end()) continue;

		vector<UDB> targetVec = targetIte->second;

		vector<TEC> updatedTEC;

		for (vector<TEC>::iterator iteite = ite->second.begin(); iteite != ite->second.end();)
		{
			bool isUpdated = false;

			for (vector<UDB>::iterator target = targetVec.begin(); target != targetVec.end(); target++)
			{
				if (target->name == iteite->name)
				{
					TEC tmp;
					tmp.path = iteite->path;
					tmp.funcdef = iteite->funcdef;
					tmp.LOC = target->LOC;
					updatedTEC.push_back(tmp);
					target = targetVec.erase(target);
					iteite = ite->second.erase(iteite);
					isUpdated = true;
					break;
				}
			}

			// �X�V�����ꍇ�Aerase�ɂ���Ď��̗v�f�Ɉڂ��Ă��邽�߁A++����K�v���Ȃ�
			if (!isUpdated) iteite++;
		}

		updatedTECdata.insert(pair<int, vector<TEC>>(ite->first, updatedTEC));

	}

	cout << "end convert" << endl;

	//for each(auto ite in updatedTECdata)
	//{
	//	for each (auto iteite in ite.second)
	//	{
	//		cout << "LOC: " << iteite.LOC << ", NAME: " << iteite.funcdef << ", PATH: " << iteite.path << std::endl;
	//	}
	//}

	//cout << "-- UDB���ɂ͂Ȃ����ATEC���ɂ͂���f�[�^" << endl;
	//for each(auto ite in TECdata)
	//{
	//	for each (auto iteite in ite.second)
	//	{
	//		cout << "LOC: " << iteite.LOC << ", NAME: " << iteite.funcdef << ", PATH: " << iteite.path << std::endl;
	//	}
	//}

	outputFile("updated_tec.csv",updatedTECdata);
	outputFile("noused_tec.csv", TECdata);
	outputFile("skip_tec.csv", skipTEClines);
	outputFile("skip_udb.csv", skipUDBlines);

	return 0;
}