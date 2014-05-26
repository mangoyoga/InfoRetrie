#include <iostream>
#include <shlwapi.h>
#include <io.h>
#include <Windows.h>
#include <string.h>
#include "tinyxml2.h"
#include "stdlib.h"

#include "index.h"
#include "compress.h"
#include "util.h"
//using namespace tinyxml2;
using namespace std;
/*const string Index::datapath="/host/disk/19960820/";
const string Index::indexpath="../input/index_data/index_temp/";
const string Index::fileidpath="../input/index_data/fileid.txt";*/
Index::Index(string indexDir)
{
	block_size=0;

	CreateDirectory(indexDir.c_str(), 0);
	//string command = "mkdir " + indexDir;
	//system(command.c_str());
	this->indexDir = indexDir.append("\\");
	fileNames.clear();
	docname.clear();

	docid=0;
}
Index::~Index()
{
}
void Index::getAllFiles(string dir)
{
	_finddata_t filedata;
	string folderPath = dir;
	string strfind = folderPath + "\\*";
	string path;
	long hFile = _findfirst(strfind.c_str(), &filedata);
	if (hFile == -1L)
	{
		cout << "Find no file in path \"" << dir << "\"." << endl;
		system("pause");
		return;
	}
	while (true)
	{
		if (filedata.attrib & _A_SUBDIR)
		{
			if ( (strcmp(filedata.name,".") != 0 ) &&(strcmp(filedata.name,"..") != 0))
			{
				getAllFiles(dir + "\\" + filedata.name);
			}
		}
		else if ((filedata.attrib & _A_SUBDIR) == 0)
		{
			path = dir + "\\" + filedata.name;
			fileNames.push_back(path);
		}
		if (_findnext(hFile, &filedata) != 0)
			break;
	}
	inputpath = dir + "\\";
}
void Index::getStopwords(string file)
{
	fstream stopfile;
	string line,word;
	
	stopfile.open(file.c_str(),ios::in);
	if(!stopfile)
	{
		cout<<"Stopwords file not find!";
		return;
	}
	while(getline(stopfile,line))
	{
		istringstream stream(line);
		stream>>word;
		stopwords.insert(word);
	}

	stopfile.close();
}
void Index::build()
{
	char name[10];
	int docnum = fileNames.size();
	//int indexednum = 0;
	index_num = 0;
	//int blocknum=0;
	block_num = 0;
	int file_num = 0;
	for (int i = 0; i < docnum; i ++)
	{
		if (fileNames[i].substr(fileNames[i].length()-4, 4) != ".txt")
			continue;
		parseAndIndex(fileNames[i], file_num);
		//indexednum ++;
		
		file_map.insert(make_pair(file_num, fileNames[i]));
		file_num ++;
	}
	if (index_num > 0)
	//if ((index_num % MAX_INDEX_FILE) != 0)
	{
		writeTmpBlock(block_num);
		block_num ++;
	}
	mergeAllIndex(block_num);
}

/*//SPIMI: generate all index files
void Index::genAllIndexFiles(string dir)
{
	//DIR *pData=NULL;
	_finddata_t filedata, filedata2;
	//struct dirent *pDataEntry=NULL;
	string folderPath = dir;
	string strfind = folderPath + "\\*";
	long hFile = _findfirst(strfind.c_str(), &filedata);
	long hFile2;
	int indexednum = 0;
	if (hFile == -1L)
	{
		cout << "Find no file in path \"" << dir << "\"." << endl;
		system("pause");
		return;
	}

	// temp
	//int tempi=0;
	//blocknum=1;
	//docid=40000;
	// end temp

	while (true)
	{
		//int uppernum = atoi(filedata.name);
		//if (filedata.attrib & _A_SUBDIR && uppernum > 19970624)
		if (filedata.attrib & _A_SUBDIR)
		{
			if ( (strcmp(filedata.name,".") != 0 ) &&(strcmp(filedata.name,"..") != 0))
			{
				cout << "index dir " << filedata.name << endl;
				string newPath = folderPath + "\\" + filedata.name;
				string strfind2 = newPath + "\\*";
				hFile2 = _findfirst(strfind2.c_str(), &filedata2);
				if (hFile2 == -1L)
				{
					cout << "Find no file in path \"" << newPath << "\"." << endl;
					system("pause");
					return;
				}
				while (true)
				{
					//stringstream lowername(filedata2.name);
					//int lowernum;
					//lowername >> lowernum;
					//if ((filedata2.attrib & _A_SUBDIR) == 0 && lowernum > 684084) {
					if ((filedata2.attrib & _A_SUBDIR) == 0) {

						//// temp
						//tempi ++;
						//if (tempi <= 40000)
						//{
						//	if (_findnext(hFile2, &filedata2) != 0)
						//		break;
						//	continue;
						//}
						//// end temp

						string path = newPath + "\\" + filedata2.name;
						parseAndIndex(path, filedata2.name);
						indexednum ++;
						if (indexednum >= MAX_INDEX_FILE)
						{
							writeTmpBlock();
							indexednum = 0;
						}
					}
					if (_findnext(hFile2, &filedata2) != 0)
						break;
				}
			}
		}
		if (_findnext(hFile, &filedata) != 0)
			break;
	}
	writeTmpBlock();
	mergeAllIndex();
}*/
void Index::parseAndIndex(string path, int file_id)
{
	ifstream fin(path, ios::binary);
	if (!fin) {
		printf("open infile %s error!\n", path.c_str());
		exit(1);
	}

	Doc mydoc;
	mydoc.file_id = file_id;
	string line;
	char tmpstr[128];//string tmpstr;
	string body;
	string key, value;
	line = "";
	int i;
	long offset = 0;
	while (!fin.eof()) // while(getline(fin, tmpstr)) 读到217890行跳出循环
	{
		//getline(fin, tmpstr);
		fin.get(tmpstr, 128); // 当读入长度为0时，会设置ios_base::failbit
		fin.clear(); // 设置了failbit后无法peek
		if (fin.peek() == '\n')
			fin.get();
	/*	string tmpstr;
		getline(fin, tmpstr);*/
		line += tmpstr;
		//if (tmpstr.size() == 0 || (tmpstr.back()!=31 && tmpstr.back()!=30) )
		if (strlen(tmpstr) == 0 || (tmpstr[strlen(tmpstr)-1] != 31 && tmpstr[strlen(tmpstr)-1] != 30) )
			continue;
		
		// Get key and value
		for (i = 0; i < line.length(); i ++) 
		{
			if (line[i] == '=') 
				break;
		}
		if (i != line.length())
		{
			key = line.substr(0, i);
			value = line.substr(i+1, line.length() - i - 2);
			if (key == "url") 
			{
				mydoc.url = value;
			}
			else if (key == "body") 
			{
				offset += 5;
				mydoc.offset = offset;
				body = value;
			}
		}
		//if (line.at(line.length() - 1) == 31)
		if (line.back() == 31)
		{
			indexDocument(mydoc, body);
			index_num ++;
			if (docid == 600 || docid == 6415)
			{
				cout << "attention!";
			}
		}
		if (index_num >= MAX_INDEX_FILE)
		{
			writeTmpBlock(block_num);
			block_num ++;
			index_num = 0;
		}
		offset = fin.tellg();
		line = "";
	}
}
//void Index::parseAndIndex(string path)
//{
//	tinyxml2::XMLDocument doc;
//	doc.LoadFile(path.c_str());
//	tinyxml2::XMLElement *root = doc.FirstChildElement("newsitem");
//	tinyxml2::XMLElement *title = root?root->FirstChildElement("title"):NULL;
//	string content = title&&title->GetText()?title->GetText():"";
//	const char *s = doc.FirstChildElement("newsitem")->FirstChildElement("headline")->GetText();
//	if(s!=NULL)
//		content.append(s);
//	tinyxml2::XMLElement *text = doc.FirstChildElement("newsitem")->FirstChildElement("text")->FirstChildElement("p");	
//	while(text != NULL)
//	{
//		content.append("\n");
//		s = text->GetText();
//		if(s!=NULL)
//			content.append(s);
//		text = text->NextSiblingElement("p");
//	}
//	// index file
//	indexDocument(path, content);
//}

bool Index::isText(char& c)	// if c is a letter, number, apostrophe or hyphen
{
	//return isalnum(c) || c == '\'' || c == '-';
	//return isupper(c) || islower(c);
	return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
}

void Index::indexDocument(Doc mydoc, string content)
{
	//int docId = docid;
	int begin = 0, end = 0, position = 0, size = content.size(), wordnum = 0;
	while (begin < size)
	{
		while (begin < size && !isText(content[begin]))
			begin ++;
		if (begin >= size)
			break;
		end = begin + 1;
		while (end < size && isText(content[end]))
			end ++;
		string term = content.substr(begin, end - begin);
		position = begin;
		begin = end + 1;
		term = strtolow(term);
		wordnum ++;
		if (stopwords.find(term) == stopwords.end()) 
		{
			if (index.find(term) == index.end()) 
			{
				vector<pos> pl;
				pos t;
				t.docid = docid;
				t.position.push_back(position);
				pl.push_back(t);
				DictionaryItem di;
				di.postingList = pl;
				index.insert(make_pair(term, di));
			}
			else if (index[term].postingList.back().docid != docid)
			{
				pos t;
				t.docid = docid;
				t.position.push_back(position);
				index.at(term).postingList.push_back(t);
			}
			else
			{
				index.at(term).postingList.back().position.push_back(position);
			}
		}
	}
	mydoc.len = wordnum;
//	doc_len.insert(make_pair(docId, wordnum));
	url_map.insert(make_pair(docid, mydoc));
	docid ++;
}

void Index::indexDocument(string path, string content)
{
	//cout << "index " << name << endl;
	int docId = docid;
	docname.insert(make_pair(docId, path));
	//collection.push_back(name);

	/*const char *spilt="[]\t\r\n ,#()+-.|\"':?~!;";
	char *str=(char *)malloc(content.length()+10);
	int position=0;
	if(str==NULL)
	{
		cout<<"malloc error!"<<endl;
		return;
	}
	strcpy(str,content.c_str());
	char *p=strtok(str,spilt);
	while(p)
	{
		position ++;
		string temp=p;
		temp=strtolow(temp);
		if(stopwords.find(temp)==stopwords.end())
		{
			if(index.find(temp)==index.end())
			{
				vector<pos> pl;
				pos t;
				t.docid=docId;
				t.position.push_back(position);
				pl.push_back(t);
				DictionaryItem di;
				di.postingList = pl;
				index.insert(pair<string,DictionaryItem >(temp,di));
			}
			else if(index.at(temp).postingList.back().docid!=docId)
			{
				pos t;
				t.docid=docId;
				t.position.push_back(position);
				index.at(temp).postingList.push_back(t);
			}
			else
			{
				index.at(temp).postingList.back().position.push_back(position);
			}
		}
//		cout<<"--"<<p<<"--"<<endl;
		p=strtok(NULL,spilt);
	}*/
	// index document
	int begin = 0, end = 0;//, index = 0;
	int position=0;
	int size = content.size();
	while(begin < size)
	{
		if(begin < size && !isText(content[begin]))
		{
			begin++;
			continue;
		}
		if (begin >= size) 
			break;
		end = begin+1;
		while(end < size)
		{
			if(!isText(content[end]))
				break;
			end++;
		}
		string term = content.substr(begin, end-begin);
		/*char *termC = new char[term.length()+1];
		strcpy(termC, term.c_str());
		termC[porting::stem(termC, 0, term.length()-1)+1] = '\0';
		term = termC;
		delete termC;
		addToIndex(term, docId, index++);
		begin = end;
		block_size++;

		if(block_size >= MAX_BLOCK_SIZE)
		{
			writeTmpBlock();
		}*/
		begin = end+1;	// change
		position ++;
		term=strtolow(term);
		if(stopwords.find(term)==stopwords.end())
		{
			if(index.find(term)==index.end())
			{
				vector<pos> pl;
				pos t;
				t.docid=docId;
				t.position.push_back(position);
				pl.push_back(t);
				DictionaryItem di;
				di.postingList = pl;
				index.insert(pair<string,DictionaryItem >(term,di));
			}
			else if(index.at(term).postingList.back().docid!=docId)
			{
				pos t;
				t.docid=docId;
				t.position.push_back(position);
				index.at(term).postingList.push_back(t);
			}
			else
			{
				index.at(term).postingList.back().position.push_back(position);
			}
		}
	}
	doc_len.insert(make_pair(docId, position));
	docid ++;
}

void Index::writeTmpBlock(int blocknum)
{
	stringstream ss;
	ss << "tmp" << blocknum << ".txt";
	string filename = ss.str();
	cout << "Writing block " << filename << "..." << endl;
	
	// write file
	ofstream out(indexDir + filename, ios::out);
	if (!out)
	{
		printf("open outfile %s error!\n", filename.c_str());
		exit(1);
	}
	map<string,DictionaryItem>::iterator map_iter;
	for (map_iter=index.begin(); map_iter!=index.end(); map_iter++)
	{
		DictionaryItem & di = map_iter->second;
		vector<pos> & vp = di.postingList;
		out << map_iter->first << " " << vp.size() << " ";
		vector<pos>::iterator vec_iter;
		for (vec_iter=vp.begin(); vec_iter!=vp.end(); vec_iter++)
		{
			vector<int> & vi = vec_iter->position;
			out << vec_iter->docid << " " << vec_iter->position.size() << " ";
			vector<int>::iterator it;
			for (it=vi.begin(); it!=vi.end(); it++)
			{
				out << *it << " ";
			}
		}
		out << endl;
	}
	out.close();
	index.clear();

	// write doc id
	filename = "docmap.txt";
	out.open(indexDir + filename, ios::app);
	if (!out)
	{
		printf("open outfile %s error!\n", filename.c_str());
		exit(1);
	}
	map<long, Doc>::iterator mapit;
	for (mapit = url_map.begin(); mapit != url_map.end(); mapit ++)
	{
		out << mapit->first << " " << mapit->second.url << " " << mapit->second.len << " "
			<< mapit->second.file_id << " " << mapit->second.offset << endl;
	}
	url_map.clear();
	out.close();
	//map<long, string>::iterator mapit;
	//for (mapit=docname.begin(); mapit!=docname.end(); mapit++)
	//{
	//	out << mapit->first << " " << mapit->second << " " << doc_len[mapit->first] << endl;
	//}
	//out.close();
	//docname.clear();
	//doc_len.clear();
	
	// write file id
	filename = "filemap.txt";
	out.open(indexDir + filename, ios::app);
	if (!out)
	{
		printf("open outfile %s error!\n", filename.c_str());
		exit(1);
	}
	//map<int, string>::iterator fileit;
	//for (fileit = filename.begin(); fileit != filename.end(); fileit ++)
	map<int, string>::iterator fileit;
	for (fileit = file_map.begin(); fileit != file_map.end(); fileit ++)
	{
		out << fileit->first << " " << fileit->second << endl;
	}
	filename.clear();
	out.close();
}

/*int heap_cmp(const HEAP_PAIR p1, const HEAP_PAIR p2)
{
	if (p1.second.first == p2.second.first)
	{
		return p1.first > p2.first;
	}
	return p1.second.first > p2.second.first;
}*/

void Index::mergeAllIndex(int blocknum)
{
	cout << "Merging..." << endl;
	vector<ifstream> in;
	//vector<HEAP_PAIR> vec;
	pair<string, DictionaryItem>** pairs = new pair<string, DictionaryItem>* [blocknum];
	bool* isover = new bool[blocknum];
	ofstream fout, strfile, dicfile;
	strfile.open((indexDir + "string.txt").c_str());
	dicfile.open((indexDir + "dictionary.dat").c_str(), ios::binary);
	stringstream ss;
	string filename;
	ss.str("");
	index_num = 0;
	ss << "postings" << index_num << ".dat"; 
	filename = ss.str();
	fout.open(indexDir + filename, ios::binary);
	pair<string, DictionaryItem> *pl;

	for (int i = 0; i < blocknum; i ++)
	{
		ss.str("");
		ss << "tmp" << i << ".txt";
		string path = indexDir + ss.str();//filename;
		in.push_back(ifstream(path, ios::in));
		if (!in.at(i))
		{
			printf("open infile %s error!\n", path.c_str());
			exit(1);
		}
		pl = loadPostingList(in.at(i));
		if (!pl)
		{
			in.at(i).close();
			isover[i] = true;
		}
		else
		{
			pairs[i] = pl;
			isover[i] = false;
		}
	}

	while (true) {
		bool allover = true;
		int mini = 0;
		string minstr;
		for (int i = 0; i < blocknum; i ++) {
			if (isover[i])
				continue;
			allover = false;
			mini = i;
			minstr = pairs[i]->first;
			break;
		}
		if (allover)
			break;
		for (int i = mini + 1; i < blocknum; i ++) {
			if (isover[i])
				continue;
			if (pairs[i]->first < minstr) {
				mini = i;
				minstr = pairs[i]->first;
			}
		}

		pair<string, DictionaryItem> tmppair;
		tmppair.first = minstr;
		vector<pos> & lastPos = tmppair.second.postingList;
		for (int i = mini; i < blocknum; i ++) {
			if (isover[i])
				continue;
			if (pairs[i]->first == minstr) {
				vector<pos> & currentPos = pairs[i]->second.postingList;
				lastPos.insert(lastPos.end(), currentPos.begin(), currentPos.end());
				//delete &(pairs[i]->second);
				delete pairs[i];
				pl = loadPostingList(in.at(i));
				if (!pl)
				{
					in.at(i).close();
					isover[i] = true;
				}
				else
				{
					pairs[i] = pl;
				}
			}
		}
		writeCompressPostingList(fout, strfile, dicfile, tmppair);
	}
	fout.close();
	strfile.close();
	dicfile.close();
}
/*
void Index::mergeAllIndex(int blocknum)
{
	cout << "Merging..." << endl;
	//vector<HEAP_PAIR> heap;
	vector<HEAP_PAIR> vec;
	int vecsize;
	vector<ifstream> in;
	ofstream fout, strfile, dicfile;
	bool* isover = new bool[blocknum];
	
	// temp blocknum
	//blocknum = 21;
	// end
	
	cout << "blocknum=" << blocknum << endl;
	int i;
	stringstream ss;
	string filename;
	for (i = 0; i < blocknum; i ++)
	{
		ss.str("");
		ss << "tmp" << i << ".txt";
		filename = ss.str();
		string path = indexDir + filename;
		in.push_back(ifstream(path, ios::in));
		if (!in.at(i))
		{
			printf("open infile %s error!\n", path.c_str());
			exit(1);
		}
		isover[i] = false;

	}
	strfile.open(indexDir + "string.txt", ios::out);
	if (!strfile)
	{
		printf("open strfile %s error!\n", indexDir + "string.txt");
		exit(1);
	}
	dicfile.open(indexDir + "dictionary.dat", ios::out);
	if (!dicfile)
	{
		printf("open dicfile %s error!\n", indexDir + "dictionary.dat");
		exit(1);
	}

	ss.str("");
	ss << "postings" << index_num << ".txt"; 
	filename = ss.str();
	fout.open(indexDir + filename, ios::out);
	if (!fout)
	{
		printf("open outfile %s error!\n", filename.c_str());
		exit(1);
	}
	cout << "Writing " << filename << endl;

	// init heap
	for (i=0; i<blocknum; i++)
	{
		pair<string, DictionaryItem> *pl = loadPostingList(in.at(i));
		if (!pl)
		{
			in.at(i).close();
			isover[i] = true;
			continue;
		}
		vec.push_back(make_pair(i, pl));
		//heap.push_back(make_pair(i, *pl));
	}
	//make_heap(heap.begin(), heap.end(), heap_cmp);

	// merge
	int readFrom;
	//pop_heap(heap.begin(), heap.end(), heap_cmp);
	//HEAP_PAIR last_pair = heap.at(heap.size()-1);
	//heap.pop_back();
	//readFrom = last_pair.first;
	//pair<string, DictionaryItem> *pl = loadPostingList(in.at(readFrom));
	//if (!pl)
	//{
	//	in.at(readFrom).close();
	//}
	//else
	//{
	//	heap.push_back(make_pair(readFrom, *pl));
	//	push_heap(heap.begin(), heap.end(), heap_cmp);
	//}
	vecsize = vec.size();
	vector<HEAP_PAIR>::iterator tmp;
	vector<HEAP_PAIR>::iterator it;
	pair<string, DictionaryItem> * last_pair, * current_pair;
	
	for (tmp = vec.begin(); tmp != vec.end(); tmp ++)
	{
		if (tmp->second != NULL)
			break;
		else
			continue;
	}
	for (it=vec.begin(); it!=vec.end(); it++)
	{
		if (NULL == it->second)
			continue;
		if (it->second->first < tmp->second->first)
			tmp = it;
	}
	cout << tmp->first << " " << tmp->second->first << endl;
	last_pair = tmp->second;
	readFrom = tmp->first;
	pair<string, DictionaryItem> * pl = loadPostingList(in.at(readFrom));
	if (!pl)
	{
		in.at(readFrom).close();
		vecsize --;
	}
	vec[readFrom].second = pl;
	while (vecsize > 0)
	{
		for (tmp = vec.begin(); tmp != vec.end(); tmp ++)
		{
			if (tmp->second != NULL)
				break;
			else
				continue;
		}
		for (it=vec.begin(); it!=vec.end(); it++)
		{
			if (NULL == it->second)
				continue;
			if (it->second->first < tmp->second->first)
				tmp = it;
		}
		//pop_heap(heap.begin(), heap.end(), heap_cmp);
		//HEAP_PAIR current_pair = heap.at(heap.size()-1);
		//heap.pop_back();
		readFrom = tmp->first;
		current_pair = tmp->second;
		pl = loadPostingList(in.at(readFrom));
		if (!pl)
		{
			in.at(readFrom).close();
			vecsize --;
		}
		vec[readFrom].second = pl;
		
		if (last_pair->first == current_pair->first)
		{
			vector<pos> & lastPos = last_pair->second.postingList;
			vector<pos> & currentPos = current_pair->second.postingList;
			lastPos.insert(lastPos.end(), currentPos.begin(), currentPos.end());
			delete current_pair;
		}
		else
		{
			writePostingList(fout, *last_pair);
			delete last_pair;
			last_pair = current_pair;
		}
	}
	writePostingList(fout, *last_pair);
	delete last_pair;
	fout.close();

	// write dictionary
	filename = "dictionary.txt";
	//fout.open(indexDir + filename, ios::out);
	fout.open(indexDir + filename, ios::out);
	if (!fout)
	{
		printf("open outfile %s error!\n", filename.c_str());
		exit(1);
	}
	HashDict::iterator dictit;
	for (dictit=dict.begin(); dictit!=dict.end(); dictit++)
	{
		fout << dictit->first << " " << dictit->second.first << " " << dictit->second.second << endl;
	}
	fout.close();
	
	// write blocks dir
	filename = "blocksDir.txt";
	//fout.open(indexDir + filename, ios::out);
	fout.open(indexDir + filename, ios::out);
	if (!fout)
	{
		printf("open outfile %s error!\n", filename.c_str());
		exit(1);
	}
	for (i=0; i<=index_num; i++)
	{
		fout << "postings" << i << ".txt" << endl;
	}
	fout.close();
	
}*/
pair<string, DictionaryItem> *Index::loadPostingList(ifstream & input)
{
	pair<string, DictionaryItem> * postingList;
	string term;
	int df, docId, tf, position, i, j;
	if (input >> term >> df)
	{
		DictionaryItem di;//DictionaryItem *di = new DictionaryItem;
		for (i=0; i < df; i++)
		{
			input >> docId >> tf;
			pos tmpPos;
			tmpPos.docid = docId;
			for (j=0; j < tf; j++)
			{
				input >> position;
				tmpPos.position.push_back(position);
			}
			di.postingList.push_back(tmpPos);//di->postingList.push_back(tmpPos);
		}
		postingList = new pair<string, DictionaryItem>(term, di);//postingList = new pair<string, DictionaryItem>;
		//postingList->first = term;
		//postingList->second = di;//postingList->second = *di;
	}
	else
	{
		postingList = NULL;
	}
	return postingList;
}
 
void Index::writeCompressPostingList(ofstream & fout, ofstream & fstr, ofstream & fdic, pair<string, DictionaryItem> & pair)
{
	vector<pos> & pl = pair.second.postingList;
	int df = pl.size(), docId, tf;
	
	// write dictionary
	long offset_blk = fout.tellp();
	//dict.insert(HashDict::value_type(pair.first, make_pair(index_num, offset)));
	int offset_str = fstr.tellp();
	fstr << pair.first;
	fdic.write((char*)(&df), sizeof(int));
	fdic.write((char*)(&index_num), sizeof(int));
	fdic.write((char*)(&offset_blk), sizeof(long));
	fdic.write((char*)(&offset_str), sizeof(int));

	// write postings
	VBWrite(fout, df);
	int predoc, curdoc, prepos, curpos;
	predoc = 0;
	for (int i = 0; i < df; i++) {
		curdoc = pl.at(i).docid;
		VBWrite(fout, curdoc - predoc); // docid
		predoc = curdoc;

		tf = pl.at(i).position.size();
		VBWrite(fout, tf);	// tf
		prepos = 0;
		for (int j = 0; j < tf; j++) {
			curpos = pl.at(i).position.at(j);
			VBWrite(fout, curpos - prepos);
			prepos = curpos;
		}
		block_size += tf;
	}

	if (block_size >= MAX_BLOCK_SIZE)
	{
		fout.close();
		block_size = 0;
		index_num ++;
		stringstream ss;
		ss << "postings" << index_num << ".txt";
		string filename = ss.str();
		fout.open(indexDir + filename, ios::binary);
	}
}
void Index::writePostingList(ofstream & fout, pair<string, DictionaryItem> & pair)
{
	vector<pos> & pl = pair.second.postingList;
	int df = pl.size(), docId, tf, i, j;
	long offset = fout.tellp();
	dict.insert(HashDict::value_type(pair.first, make_pair(index_num, offset)));
	fout << pair.first << " " << df << " ";
	for (i=0; i<df; i++)
	{
		docId = pl.at(i).docid;
		vector<int> & position = pl.at(i).position;
		tf = pl.at(i).position.size();
		fout << docId << " " << tf << " ";
		for (j=0; j<tf; j++)
		{
			fout << position.at(j) << " ";
		}
		block_size += tf;
	}
	fout << endl;

	if (block_size >= MAX_BLOCK_SIZE)
	{
		fout.close();
		block_size = 0;
		index_num ++;
		stringstream ss;
		ss << "postings" << index_num << ".txt";
		string filename = ss.str();
		//fout.open(indexDir + filename, ios::out);
		fout.open(indexDir + filename, ios::out);
		if (!fout)
		{
			printf("open outfile %s error!\n", filename.c_str());
			exit(1);
		}
		cout << "Writing " << filename << endl;
	}
}


