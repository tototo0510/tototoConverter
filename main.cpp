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

// 前提
// ① 事前に" (" を "("で置換する。
// ② オーバーロードは未対応
// ③ モジュール外も無視
// ④ excelなど用いて、ファイル名順にソート済みである
// ⑤ 継承を書いていると誤動作する
// ⑥ 処理の中身を書いていると誤動作

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

// tecデータは(空)(ファイルパス)(関数定義)(LOC)の順
bool analyzeTEC(vector<string> line, map<int, vector<TEC>> &TECdata, map<int, string> &TECfiles)
{
	// 読み込んだデータを取得
	TEC tmp;
	tmp.path = line[1];
	tmp.LOC = stoi(line[4]);
	tmp.funcdef = line[2];
		
	///////////////////////////////////////////////
	// 関数定義を分解して、関数名を取り出す。
	///////////////////////////////////////////////
	// c#
	if (strstr(tmp.path.c_str(), ".cs") != NULL || strstr(tmp.path.c_str(), ".CS") != NULL)
	{
		istringstream iss(tmp.funcdef);
		vector<string> funcdefs;
		string token;
		while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

		// ★namespace, class, functionの3種類あるので、そこを分解
		// ★構造体などがあるようなら、合わせて修正が必要
		if (strstr(tmp.funcdef.c_str(), "class ") != NULL)
		{
			// "class XX"となっている場合に、後ろの"を外す。前の"はsplitしたときになくなる
			// "がない場合はそのまま値が入る
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
		// ★strstrが全角文字に対応しているかが不安
		else if (strstr(tmp.funcdef.c_str(), "モジュール外"))
		{
			cout << "モジュール外はskip" << endl;
			return false;
		}
		// function
		else
		{
			// ★funcdefs[funcdefs.size() - 1]は、XXX::xxx(param 1, param2, ...) の形式のはず(クラス化していないものもある)
			// 関数名と引数の後に何かが来ることはないはず
			int i;
			for (i = 0; i < funcdefs.size(); i++)
				if (strstr(funcdefs[i].c_str(), "(")) break;


			// 上のループでbreakしなかった(関数定義に引数がなかった場合)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// 関数名部分を取得
			std::getline(issFunc, funcname, '(');

			// "を外す
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
		if (strstr(tmp.funcdef.c_str(), "モジュール外"))
		{
			cout << "モジュール外はskip" << endl;
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


			// 上のループでbreakしなかった(関数定義に引数がなかった場合)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// 関数名部分を取得
			std::getline(issFunc, funcname, '(');

			// "を外す
			istringstream issFunc2(funcname);
			string funcname2;
			while (std::getline(issFunc2, funcname2, '"'));

			tmp.name = funcname2;
			tmp.type = TYPE_FUNC;
		}
	}
	else
	{
		cout << "未定義ファイル形式: " << tmp.path << endl;
		return false;
	}


	// 辞書作り
	// ファイル辞書に未登録
	if (TECfiles.empty() || (--TECfiles.end())->second != tmp.path)
	{
		// TECfiles.size(), tmp.path
		TECfiles.insert(pair<int, string>(TECfiles.size(), tmp.path));

		vector<TEC> input;
		input.push_back(tmp);
		TECdata.insert(pair<int, vector<TEC>>(TECdata.size(), input));
	}
	// 既にファイルパスを登録済みのデータ
	else
	{
		// ファイルパス辞書は更新せず、関数辞書のみ更新
		map<int, string>::iterator cur = TECfiles.end(); 
		cur--;
		int curfileID = cur->first;
		map<int, vector<TEC>>::iterator ite = TECdata.find(curfileID);
	
		for each(auto iteite in ite->second)
		{
			// 同じ関数名のものが登録済み　 == overload なのでskip
			if (iteite.name == tmp.name && iteite.type == tmp.type)
			{
				return false;
			}
		}
		ite->second.push_back(tmp);
	}

	return true;
}

// UDBデータは(タイプ)(関数定義)(ファイルパス)(LOC)の順
bool analyzeUDB(vector<string> line, map<int, vector<UDB>> &UDBdata, map<int, string> &UDBfiles, const map<int, string> TECfiles, vector<UDB> &namespaceVec)
{
	// 読み込んだデータを取得
	const string type = line[0];

	UDB tmp;
	tmp.name = line[1];

	// 名前空間と構造体ははじく
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
	// ファイルはCountLineCodeExeが空なのでここではじく
	else if (strstr(type.c_str(), "File") != NULL)
	{
		return false;
	}
	
	for (int i = 2; i < line.size(); i++) {
		if (line[i] == "")
		{
			if (tmp.type != DataType::TYPE_NAMESPACE)
			{
				// cout << "空データあり" << endl;
				return false;
			}
			else
			{
				continue;
			}
		}

		// 文字列が数字列の場合
		if (std::all_of(line[i].cbegin(), line[i].cend(), isdigit))
		{
			// 1回目がLOCなので、一度でループを抜ける
			tmp.path = line[i-1];
			tmp.LOC = stoi(line[i]);
			tmp.LOCexe = stoi(line[i + 1]);
			tmp.LOCcom = stoi(line[i + 2]);
			break;
		}
	}

	////////////////////////////////////////////////
	// TEC側にないファイルのデータであればskipする
	////////////////////////////////////////////////
	int counter = 0;
	if (tmp.type != DataType::TYPE_NAMESPACE)
	{
		for each (auto ite in TECfiles)
		{
			if (strcmp(tmp.path.c_str(), ite.second.c_str()) == 0) break;
			else counter++;
		}

		// 一致するファイルがなかった
		if (counter == TECfiles.size()) return false;
	}

	///////////////////////////////////////////////
	// 関数定義を分解して、関数名を取り出す。
	///////////////////////////////////////////////
	// c#
	// namespaceはファイルパスがないが、C#特有なのでこちらに誘導
	if (strstr(tmp.path.c_str(), ".cs") != NULL || strstr(tmp.path.c_str(), ".CS") != NULL || tmp.type == DataType::TYPE_NAMESPACE)
	{
		istringstream iss(tmp.name);
		vector<string> funcdefs;
		string token;
		while (std::getline(iss, token, ' ')) funcdefs.push_back(token);

		// ★namespace, class, functionの3種類あるので、そこを分解
		// ★構造体などがあるようなら、合わせて修正が必要
		if (tmp.type == DataType::TYPE_CLASS || tmp.type == DataType::TYPE_NAMESPACE || tmp.type == DataType::TYPE_STRUCT)
		{
			//"XX"とXXのパターンがある
			istringstream issFunc(funcdefs[funcdefs.size() - 1]);
			string funcname;
			std::getline(issFunc, funcname, '\"');
			std::getline(issFunc, funcname, '\"');
			tmp.name = funcname; // ★最終要素がそのままクラス名になっているはず
		}
		// function
		else if(strstr(type.c_str(), "Func") != NULL || strstr(type.c_str(), "Construc") || strstr(type.c_str(), "Method"))
		{
			// ★funcdefs[funcdefs.size() - 1]は、XXX::xxx(param 1, param2, ...) の形式のはず(クラス化していないものもある)
			// 関数名と引数の後に何かが来ることはないはず
			int i;
			for (i = 0; i < funcdefs.size(); i++)
				if (strstr(funcdefs[i].c_str(), "(")) break;
			
			// 上のループでbreakしなかった(関数定義に引数がなかった場合)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// 関数名部分を取得
			std::getline(issFunc, funcname, '(');

			// "XXX()"というパターンがあるので、頭の"を外したい
			istringstream issFunc2(funcname);
			string funcname2;
			while (std::getline(issFunc2, funcname2, '"'));

			// XXX.XX.XXの.を外す
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


			// 上のループでbreakしなかった(関数定義に引数がなかった場合)
			if (i == funcdefs.size()) i--;

			istringstream issFunc(funcdefs[i]);
			string funcname;
			// 関数名部分を取得
			std::getline(issFunc, funcname, '(');

			// "XXX()"というパターンがあるので、頭の"を外したい
			// 後ろの"は(でsplitしたときに外れる
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
		cout << "未定義ファイル形式: " << tmp.path << endl;
		return false;
	}


	// 辞書作り
	// namespaceはファイルパスがないので、別の辞書で管理する
	if (tmp.type == DataType::TYPE_NAMESPACE)
	{
		namespaceVec.push_back(tmp);
	}
	// ファイル辞書に未登録
	else if (UDBfiles.empty() || (--UDBfiles.end())->second != tmp.path)
	{
		// TECfiles.size(), tmp.path
		UDBfiles.insert(pair<int, string>(counter, tmp.path));

		vector<UDB> input;
		input.push_back(tmp);
		UDBdata.insert(pair<int, vector<UDB>>(counter, input));
	}
	// 既にファイルパスを登録済みのデータ
	else
	{
		// ファイルパス辞書は更新せず、関数辞書のみ更新
		map<int, string>::iterator cur = UDBfiles.end();
		cur--;
		int curfileID = cur->first;
		map<int, vector<UDB>>::iterator ite = UDBdata.find(curfileID);

		for each(auto iteite in ite->second)
		{
			// 同じ関数名のものが登録済み　 == overload なのでskip
			// コンストラクタの場合、クラス名 == 関数名になるので注意
			if (iteite.name == tmp.name && iteite.type == tmp.type) return false;
		}

		ite->second.push_back(tmp);
	}

	return true;
}

void loadTEC(map<int, vector<TEC>> &TECdata, map<int, string> &TECfiles, vector<string> &skipTECLines)
{
	// CSVファイルを読み込む
	ifstream ifs("tec.txt");
	if (!ifs) return;

	string line;
	while (std::getline(ifs, line))
	{
		////////////////////////////////////////////////
		// TECさんのよくわからないデータは、skipする
		////////////////////////////////////////////////
		// BEGIN_MESSAGE_MAPとか
		if(strstr(line.c_str(), "_MAP") != NULL) skipTECLines.push_back(line);
		// IMPLEMENT_SERIALとか
		else if(strstr(line.c_str(), "IMPLEMENT_") != NULL) skipTECLines.push_back(line);
		// 処理が書いてあるもの
		else if (strstr(line.c_str(), "{") != NULL || strstr(line.c_str(), "}") != NULL) skipTECLines.push_back(line);
		// モジュール外
		else if (strstr(line.c_str(), "モジュール外") != NULL || strstr(line.c_str(), "}") != NULL) skipTECLines.push_back(line);
		else
		{
			// 行をタブ区切りで分解
			string token;
			istringstream iss(line);
			vector<string> tokens;
			while (std::getline(iss, token, TSV_TOKEN))
			{
				tokens.push_back(token);
			}

			// 解析してデータを挿入できなかった場合、skipした行として保持(最後に出力する)
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
	// CSVファイルを読み込む
	ifstream ifs("udb.csv");
	if (!ifs) return;

	string line;

	// タイトル行skip
	std::getline(ifs, line);

	while (std::getline(ifs, line))
	{
		// 行をタブ区切りで分解
		string token;
		istringstream iss(line);
		vector<string> tokens;
		while (std::getline(iss, token, CSV_TOKEN))
		{
			tokens.push_back(token);
		}

		// 解析してデータを挿入できなかった場合、skipした行として保持(最後に出力する)
		if (!analyzeUDB(tokens, UDBdata, UDBfiles, TECfiles, namespaceVec))
		{
			skipUDBLines.push_back(line);
		}
	}

	return;
}

void outputFile(const string filename, const vector<string> input)
{
	ofstream ofs(filename); //ファイル出力ストリーム
	for each (auto target in input)
	{
		ofs << target << endl;
	}
}

void outputFile(const string filename, const map<int, vector<TEC>> input)
{
	ofstream ofs(filename); //ファイル出力ストリーム

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
	ofstream ofs(filename); //ファイル出力ストリーム

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
	// LOC更新
	////////////////////////////////////////////////////////////////////////////
	for (auto ite = TECdata.begin(); ite != TECdata.end(); ite++)
	{
		// 対象ファイルのデータを取得(
		map<int, vector<UDB>>::iterator targetIte = UDBdata.find(ite->first);
		if (targetIte == UDBdata.end()) continue;

		vector<UDB> targetVec = targetIte->second;

		vector<TEC> updatedTEC;

		for (vector<TEC>::iterator iteite = ite->second.begin(); iteite != ite->second.end();)
		{
			// LOCを更新したかどうか
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

				// namespaceが見つからなかった
				// UDBは1個にまとめているので、2回目以降は見つからない。
				// その場合、LOCを0にして追加する
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
			
			// 更新した場合、eraseによって次の要素に移っているため、++する必要がない
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