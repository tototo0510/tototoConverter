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
// �E �����̒��g�������Ă���ƌ듮��

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
	int LOCexe;
	int LOCcom;
	string path;
	string name;
	string funcdef;
	DataType type;
};

struct UDB
{
	int LOC;
	int LOCexe;
	int LOCcom;
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
	tmp.LOC = stoi(line[4]);
	tmp.funcdef = line[2];
		
	///////////////////////////////////////////////
	// �֐���`�𕪉����āA�֐��������o���B
	///////////////////////////////////////////////
	// c#
	if (strstr(tmp.path.c_str(), ".cs") != NULL || strstr(tmp.path.c_str(), ".CS") != NULL)
	{
		istringstream iss(tmp.funcdef);
		vector<string> funcdefs;
		string token;
		while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

		// ��namespace, class, function��3��ނ���̂ŁA�����𕪉�
		// ���\���̂Ȃǂ�����悤�Ȃ�A���킹�ďC�����K�v
		if (strstr(tmp.funcdef.c_str(), "class ") != NULL)
		{
			// "class XX"�ƂȂ��Ă���ꍇ�ɁA����"���O���B�O��"��split�����Ƃ��ɂȂ��Ȃ�
			// "���Ȃ��ꍇ�͂��̂܂ܒl������
			istringstream issFunc2(funcdefs[funcdefs.size() - 1]);
			string funcname2;
			std::getline(issFunc2, funcname2, '"');
			tmp.name = funcname2;
			tmp.type = DataType::TYPE_CLASS;
		}
		else if (strstr(tmp.funcdef.c_str(), "namespace ") != NULL)
		{
			istringstream issFunc2(funcdefs[funcdefs.size() - 1]);
			string funcname2;
			std::getline(issFunc2, funcname2, '"');
			tmp.name = funcname2;
			tmp.type = DataType::TYPE_NAMESPACE;
		}
		else if (strstr(tmp.funcdef.c_str(), "struct ") != NULL)
		{
			istringstream issFunc2(funcdefs[funcdefs.size() - 1]);
			string funcname2;
			std::getline(issFunc2, funcname2, '"');
			tmp.name = funcname2;
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

			// "���O��
			istringstream issFunc2(funcname);
			string funcname2;
			while (std::getline(issFunc2, funcname2, '"'));						

			tmp.name = funcname2;
			tmp.type = TYPE_FUNC;
		}
	}
	// c++, c, h
	else if (strstr(tmp.path.c_str(), ".h") != NULL || strstr(tmp.path.c_str(), ".c") != NULL
		|| strstr(tmp.path.c_str(), ".H") != NULL || strstr(tmp.path.c_str(), ".C") != NULL)
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

			// "���O��
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
	
		for each(auto iteite in ite->second)
		{
			// �����֐����̂��̂��o�^�ς݁@ == overload �Ȃ̂�skip
			if (iteite.name == tmp.name && iteite.type == tmp.type)
			{
				return false;
			}
		}
		ite->second.push_back(tmp);
	}

	return true;
}

// UDB�f�[�^��(�^�C�v)(�֐���`)(�t�@�C���p�X)(LOC)�̏�
bool analyzeUDB(vector<string> line, map<int, vector<UDB>> &UDBdata, map<int, string> &UDBfiles, const map<int, string> TECfiles, vector<UDB> &namespaceVec)
{
	// �ǂݍ��񂾃f�[�^���擾
	const string type = line[0];

	UDB tmp;
	tmp.name = line[1];

	// ���O��Ԃƍ\���̂͂͂���
	if (strstr(type.c_str(), "Namespace") != NULL)
	{
		tmp.type = DataType::TYPE_NAMESPACE;
	}
	else if (strstr(type.c_str(), "Struct") != NULL)
	{
		tmp.type = DataType::TYPE_STRUCT;
	}
	else if (strstr(type.c_str(), "Class") != NULL)
	{
		tmp.type = DataType::TYPE_CLASS;
	}
	else if (strstr(type.c_str(), "Func") != NULL || strstr(type.c_str(), "Construc") || strstr(type.c_str(), "Method"))
	{
		tmp.type = DataType::TYPE_FUNC;
	}
	// �t�@�C����CountLineCodeExe����Ȃ̂ł����ł͂���
	else if (strstr(type.c_str(), "File") != NULL)
	{
		return false;
	}
	
	for (int i = 2; i < line.size(); i++) {
		if (line[i] == "")
		{
			if (tmp.type != DataType::TYPE_NAMESPACE)
			{
				// cout << "��f�[�^����" << endl;
				return false;
			}
			else
			{
				continue;
			}
		}

		// �����񂪐�����̏ꍇ
		if (std::all_of(line[i].cbegin(), line[i].cend(), isdigit))
		{
			// 1��ڂ�LOC�Ȃ̂ŁA��x�Ń��[�v�𔲂���
			tmp.path = line[i-1];
			tmp.LOC = stoi(line[i]);
			tmp.LOCexe = stoi(line[i + 1]);
			tmp.LOCcom = stoi(line[i + 2]);
			break;
		}
	}

	////////////////////////////////////////////////
	// TEC���ɂȂ��t�@�C���̃f�[�^�ł����skip����
	////////////////////////////////////////////////
	int counter = 0;
	if (tmp.type != DataType::TYPE_NAMESPACE)
	{
		for each (auto ite in TECfiles)
		{
			if (strcmp(tmp.path.c_str(), ite.second.c_str()) == 0) break;
			else counter++;
		}

		// ��v����t�@�C�����Ȃ�����
		if (counter == TECfiles.size()) return false;
	}

	///////////////////////////////////////////////
	// �֐���`�𕪉����āA�֐��������o���B
	///////////////////////////////////////////////
	// c#
	// namespace�̓t�@�C���p�X���Ȃ����AC#���L�Ȃ̂ł�����ɗU��
	if (strstr(tmp.path.c_str(), ".cs") != NULL || strstr(tmp.path.c_str(), ".CS") != NULL || tmp.type == DataType::TYPE_NAMESPACE)
	{
		istringstream iss(tmp.name);
		vector<string> funcdefs;
		string token;
		while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

		// ��namespace, class, function��3��ނ���̂ŁA�����𕪉�
		// ���\���̂Ȃǂ�����悤�Ȃ�A���킹�ďC�����K�v
		if (tmp.type == DataType::TYPE_CLASS || tmp.type == DataType::TYPE_NAMESPACE || tmp.type == DataType::TYPE_STRUCT)
		{
			//"XX"��XX�̃p�^�[��������
			istringstream issFunc(funcdefs[funcdefs.size() - 1]);
			string funcname;
			std::getline(issFunc, funcname, '\"');
			std::getline(issFunc, funcname, '\"');
			tmp.name = funcname; // ���ŏI�v�f�����̂܂܃N���X���ɂȂ��Ă���͂�
		}
		// function
		else if(strstr(type.c_str(), "Func") != NULL || strstr(type.c_str(), "Construc") || strstr(type.c_str(), "Method"))
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

			// XXX.XX.XX��.���O��
			if (strstr(funcname2.c_str(), ".") != NULL)
			{
				istringstream issFunc3(funcname2);
				while (std::getline(issFunc3, funcname, '.'));
			}
			else
				funcname = funcname2;

			tmp.name = funcname;
		}
		else
		{
			return false;
		}
	}
	// c++, c, h
	else if (strstr(tmp.path.c_str(), ".h") != NULL || strstr(tmp.path.c_str(), ".c") != NULL
		|| strstr(tmp.path.c_str(), ".H") != NULL || strstr(tmp.path.c_str(), ".C") != NULL)
	{
		if (strstr(type.c_str(), "Func") != NULL || strstr(type.c_str(), "Construc") || strstr(type.c_str(), "Method"))
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
			// ����"��(��split�����Ƃ��ɊO���
			istringstream issFunc2(funcname);
			string funcname2;
			while (std::getline(issFunc2, funcname2, '"'));

			tmp.name = funcname2;
		}
		else
		{
			return false;
		}
	}
	else
	{
		cout << "����`�t�@�C���`��: " << tmp.path << endl;
		return false;
	}


	// �������
	// namespace�̓t�@�C���p�X���Ȃ��̂ŁA�ʂ̎����ŊǗ�����
	if (tmp.type == DataType::TYPE_NAMESPACE)
	{
		namespaceVec.push_back(tmp);
	}
	// �t�@�C�������ɖ��o�^
	else if (UDBfiles.empty() || (--UDBfiles.end())->second != tmp.path)
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

		for each(auto iteite in ite->second)
		{
			// �����֐����̂��̂��o�^�ς݁@ == overload �Ȃ̂�skip
			// �R���X�g���N�^�̏ꍇ�A�N���X�� == �֐����ɂȂ�̂Œ���
			if (iteite.name == tmp.name && iteite.type == tmp.type) return false;
		}

		ite->second.push_back(tmp);
	}

	return true;
}

void loadTEC(map<int, vector<TEC>> &TECdata, map<int, string> &TECfiles, vector<string> &skipTECLines)
{
	// CSV�t�@�C����ǂݍ���
	ifstream ifs("tec.txt");
	if (!ifs) return;

	string line;
	while (std::getline(ifs, line))
	{
		////////////////////////////////////////////////
		// TEC����̂悭�킩��Ȃ��f�[�^�́Askip����
		////////////////////////////////////////////////
		// BEGIN_MESSAGE_MAP�Ƃ�
		if(strstr(line.c_str(), "_MAP") != NULL) skipTECLines.push_back(line);
		// IMPLEMENT_SERIAL�Ƃ�
		else if(strstr(line.c_str(), "IMPLEMENT_") != NULL) skipTECLines.push_back(line);
		// �����������Ă������
		else if (strstr(line.c_str(), "{") != NULL || strstr(line.c_str(), "}") != NULL) skipTECLines.push_back(line);
		// ���W���[���O
		else if (strstr(line.c_str(), "���W���[���O") != NULL || strstr(line.c_str(), "}") != NULL) skipTECLines.push_back(line);
		else
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
	}

	return;
}

void loadUDB(map<int, vector<UDB>> &UDBdata, map<int, string> &UDBfiles, vector<string> &skipUDBLines, const map<int, string> TECfiles, vector<UDB> &namespaceVec)
{
	// CSV�t�@�C����ǂݍ���
	ifstream ifs("udb.csv");
	if (!ifs) return;

	string line;

	// �^�C�g���sskip
	std::getline(ifs, line);

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
		if (!analyzeUDB(tokens, UDBdata, UDBfiles, TECfiles, namespaceVec))
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

	ofs << "path,funcdef,LOC,LOCexe,LOCcom" << endl;

	for each (auto target in input)
	{
		for each(auto elem in target.second)
		{
			ofs << elem.path << "," << elem.funcdef << "," << elem.LOC << "," << elem.LOCexe << "," << elem.LOCcom <<  endl;
		}
	}
}

void outputFile(const string filename, const map<int, vector<UDB>> input)
{
	ofstream ofs(filename); //�t�@�C���o�̓X�g���[��

	ofs << "path,funcdef,LOC,LOCexe,LOCcom" << endl;


	for each (auto target in input)
	{
		for each(auto elem in target.second)
		{
			ofs << elem.path << "," << elem.name << "," << elem.LOC << "," << elem.LOCexe << "," << elem.LOCcom << endl;
		}
	}
}

int main(void)
{
	map<int, vector<TEC>> TECdata, updatedTECdata;
	map<int, vector<UDB>> UDBdata;
	map<int, string> TECfiles;
	map<int, string> UDBfiles;
	vector<UDB> namespaceVec;
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

	cout << "TECdata.size(): " << TECdata.size() << endl;
	cout << "skipTEClines.size(): " << skipTEClines.size() << endl;

	outputFile("output/skip_tec.txt", skipTEClines);

	loadUDB(UDBdata, UDBfiles, skipUDBlines, TECfiles, namespaceVec);
	cout << "end load UDB" << endl;

	cout << "UDBdata.size(): " << UDBdata.size() << endl;
	cout << "skipUDBlines.size(): " << skipUDBlines.size() << endl;
	cout << "namspaceVec.size(): " << namespaceVec.size() << endl;

	outputFile("output/skip_udb.txt", skipUDBlines);
	
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
			// LOC���X�V�������ǂ���
			bool isUpdated = false;

			if (iteite->type == DataType::TYPE_NAMESPACE) 
			{
				for (vector<UDB>::iterator target = namespaceVec.begin(); target != namespaceVec.end(); target++)
				{
					if (target->name == iteite->name && target->type == iteite->type)
					{
						TEC tmp;
						tmp.path = iteite->path;
						tmp.funcdef = iteite->funcdef;
						tmp.LOC = target->LOC;
						tmp.LOCexe = target->LOCexe;
						tmp.LOCcom = target->LOCcom;
						updatedTEC.push_back(tmp);
						target = namespaceVec.erase(target);
						iteite = ite->second.erase(iteite);
						isUpdated = true;
						break;
					}
				}

				// namespace��������Ȃ�����
				// UDB��1�ɂ܂Ƃ߂Ă���̂ŁA2��ڈȍ~�͌�����Ȃ��B
				// ���̏ꍇ�ALOC��0�ɂ��Ēǉ�����
				if (!isUpdated)
				{
					TEC tmp;
					tmp.path = iteite->path;
					tmp.funcdef = iteite->funcdef;
					tmp.LOC = 0;
					tmp.LOCexe = 0;
					tmp.LOCcom = 0;
					updatedTEC.push_back(tmp);
					iteite = ite->second.erase(iteite);
					isUpdated = true;
				}
			}
			else
			{
				for (vector<UDB>::iterator target = targetVec.begin(); target != targetVec.end(); target++)
				{
					if (target->name == iteite->name && target->type == iteite->type)
					{
						TEC tmp;
						tmp.path = iteite->path;
						tmp.funcdef = iteite->funcdef;
						tmp.LOC = target->LOC;
						tmp.LOCexe = target->LOCexe;
						tmp.LOCcom = target->LOCcom;
						updatedTEC.push_back(tmp);
						target = targetVec.erase(target);
						iteite = ite->second.erase(iteite);
						isUpdated = true;
						break;
					}
				}
			}
			
			// �X�V�����ꍇ�Aerase�ɂ���Ď��̗v�f�Ɉڂ��Ă��邽�߁A++����K�v���Ȃ�
			if (!isUpdated) iteite++;
		}

		updatedTECdata.insert(pair<int, vector<TEC>>(ite->first, updatedTEC));

	}

	cout << "end convert" << endl;

	outputFile("output/updated_tec.csv",updatedTECdata);
	outputFile("output/noused_tec.csv", TECdata);
	outputFile("output/noused_udb.csv", UDBdata);

	return 0;
}