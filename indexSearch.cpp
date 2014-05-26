#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <iomanip>
#include "indexSearch.h"
using namespace std;

void strSplit(vector<string> & words, string & line, string seperate)
{
	int len = line.length(), i, j;
	for (i = 0; i < len; i++) {
		while (i < len && seperate.find(line.at(i)) != seperate.npos)
			i ++;
		if (i >= len)
			break;
		for (j = i+1; j < len; j++) {
			if (seperate.find(line.at(j)) != seperate.npos)
				break;
		}
		words.push_back(line.substr(i, j-i));
		i = j;
	}
}

indexSearch::indexSearch(string dir)
{
	indexdir = dir.append("\\");

	// load dictionary
	string fname = "dictionary.dat";
	ifstream fin((dir + fname).c_str(), ios::binary);
	if (!fin)
	{
		printf("open infile %s error!\n", (indexdir+fname).c_str());
		exit(1);
	}

	long start, end;
	start = fin.tellg();
	fin.seekg(0, ios::end);
	end = fin.tellg();
	int size = sizeof(unit); //int size = 3*sizeof(int) + sizeof(long);
	int num = (end - start) / size;
	wordNum = num;
	dict = new unit [num];
	fin.seekg(0, ios::beg);
	for (int i=0; i<num; i++)
	{
		fin.read((char*)&(dict[i].df), sizeof(int));
		fin.read((char*)&(dict[i].block_id), sizeof(int));
		fin.read((char*)&(dict[i].offset_blk), sizeof(long));
		fin.read((char*)&(dict[i].offset_str), sizeof(int));
	}
	fin.close();

	fname = "string.txt";
	fin.open((dir + fname).c_str());
	if (!fin)
	{
		printf("open infile %s error!\n", (indexdir+fname).c_str());
		exit(1);
	}
	start = fin.tellg();
	fin.seekg(0, ios::end);
	end = fin.tellg();
	fin.seekg(0, ios::beg);
	num = end - start;
	allstr = new char [num + 10];
	for (int i=0; i<num; i++)
	{
		fin >> allstr[i];
	}
	allstr[num] = 0;
	fin.close();

	// load doc id
	fname = "docmap.txt";
	fin.open((dir + fname).c_str());
	if (!fin)
	{
		printf("open infile %s error!\n", (indexdir+fname).c_str());
		exit(1);
	}
	string tmpname;
	int tmpid, tmplen;
	while (fin >> tmpid >> tmpname >> tmplen) {
		doc_name.insert(pair<int, string>(tmpid, tmpname));
		doc_len[tmpid] = tmplen;
	}
	fin.close();
}

indexSearch::~indexSearch()
{
	delete [] allstr;
	delete [] dict;
}

int indexSearch::compare(int p, string & term)
{
	int start = dict[p].offset_str, end, len, judge;
	if (p + 1 < this->wordNum)
		end = dict[p+1].offset_str;
	else
	{
		for (end = start; allstr[end] != 0; end ++);
	}
	len = end - start;
	char * str = new char[len+1];
	strncpy(str, allstr + start, len);
	str[len] = 0;
	judge = strcmp(term.c_str(), str);
	delete [] str;
	return judge;
}

int indexSearch::binarySearch(string & term)
{
	int l = 0, r = wordNum-1, m, judge;
	bool find = false;
	while (l <= r)
	{
		m = (l + r) / 2;
		judge = compare(m, term);
		if (judge < 0)
		{
			r = m-1;
		}
		else if (judge > 0)
		{
			l = m+1;
		}
		else
		{
			find = true;
			break;
		}
	}
	if (!find)
	{
		return -1;
	}
	else
	{
		return m;
	}
}
string indexSearch::plFileName(int i)
{
	stringstream ss;
	ss << "postings" << i << ".dat";
	return ss.str();
}

DictionaryItem * indexSearch::find(string term)
{
	int point = binarySearch(term);
	if (point < 0)
	{
		cout << "word not found!" << endl << endl;
		return NULL;
	}
	int block_id = dict[point].block_id;
	long offset_blk = dict[point].offset_blk;
	int df = dict[point].df;
	//stringstream ss;
	//ss << "postings" << block_id << ".dat";
	string name = plFileName(block_id);
	ifstream fin((indexdir+name).c_str(), ios::binary);
	if (!fin)
	{
		printf("open infile %s error!\n", (indexdir+name).c_str());
		exit(1);
	}
	//fin.seekg(offset_blk, ios::beg);
	fin.seekg(offset_blk, ios::beg);
	DictionaryItem * di = new DictionaryItem;
	int dfr, predoc = 0, curdoc, tf, prepos, curpos;
	dfr = VBRead(fin);
	if (dfr != df)
	{
		printf("get df wrong! %d != %d\n", dfr, df);
		return NULL;
	}
	for (int i = 0; i < df; i ++) {
		curdoc = VBRead(fin) + predoc;
		predoc = curdoc;
		pos mypos;
		mypos.docid = curdoc;
		tf = VBRead(fin);
		prepos = 0;
		for (int j = 0; j < tf; j ++) {
			curpos = VBRead(fin) + prepos;
			prepos = curpos;
			mypos.position.push_back(curpos);
		}
		di->postingList.push_back(mypos);
	}
	//DictionaryItem * di = Compress::GetDI(fin, offset_blk, df);
	fin.close();
	return di;
}

int heap_cmp(const pair<int, float> p1, const pair<int, float> p2)
{
	if (p1.second == p2.second)
		return p1.first > p2.first;
	return p1.second < p2.second;
}

void indexSearch::search(string line)
{
	vector<string> words;
	strSplit(words, line, " ");

	map<string, float> w_tq;
	map<int, float> doclen;
	map<int, float> scores;
	map<int, map<string, int> > postings;
	int qsize = words.size();
	int docnum = doc_name.size();

	for (int i = 0; i < qsize; i ++) {
		if (w_tq.find(words[i]) == w_tq.end()) {
			w_tq[words[i]] = 1;
		}
		else {
			w_tq[words[i]] ++;
		}
	}
	qsize = w_tq.size();

	map<string, float>::iterator it;
	DictionaryItem * di;
	int docid, df, tf;

	for (it = w_tq.begin(); it != w_tq.end(); it ++) {
		di = find(it->first);
		if (di == NULL)
			continue;
		df = di->postingList.size();
		w_tq[it->first] *= log(double(docnum / df));
		vector<pos>::iterator pit;
		for (pit = di->postingList.begin(); pit != di->postingList.end(); pit ++) {
			docid = pit->docid;
			tf = pit->position.size();

			//if (postings.find(docid) == postings.end())
			postings[docid][it->first] = tf;

			float s = it->second * tf;
			if (scores.find(docid) == scores.end()) {
				scores[docid] = s;
			}
			else {
				scores[docid] += s;
			}

			int len = tf * tf;
			if (doclen.find(docid) == doclen.end()) {
				doclen[docid] = len;
			}
			else {
				doclen[docid] += len;
			}
		}
		delete di;
	}
	map<int, float>::iterator dit;
	vector<pair<int, float> > heap;
	for (dit = scores.begin(); dit != scores.end(); dit ++) {
		doclen[dit->first] = sqrt(doclen[dit->first]);
		//dit->second = dit->second / doclen[dit->first];
		//dit->second = dit->second / doc_len[dit->first];
		heap.push_back(*dit);
	}
	make_heap(heap.begin(), heap.end(), heap_cmp);
	int total = heap.size();

	vector<int> result;
	int i;
	for (i = 0; i < TOP_K; i ++) {
		if (heap.size() == 0)
			break;
		pop_heap(heap.begin(), heap.end(), heap_cmp);
		result.push_back((heap.back()).first);
		heap.pop_back();
	}
	int topx = i;

	cout << "There are " << total << " document(s) fit the query. Top " << topx << " results are displayed below." << endl;
	for (i = 0; i < topx; i ++) {
		docid = result[i];
		cout << i+1 << " " << doc_name[docid] << " ";
		cout << "score = " << scores[result[i]] << ", doc len = " << doc_len[docid] << endl;
		cout << setiosflags(ios::left);
		for (it = w_tq.begin(); it != w_tq.end(); it ++) {
			cout << "\t" << it->first << ": w_tq=" << setw(5) << it->second << ", hit num=" << setw(3) << postings[docid][it->first] << endl;
		}
		cout << endl;
	}
	cout << endl;
}

