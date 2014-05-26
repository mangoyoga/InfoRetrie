#ifndef INDEX_H
#define INDEX_H

#include <hash_map>
#include "util.h"

using namespace std;

#define TOP_K 10

class DictionaryItem
{
public:
	vector<pos> postingList;
};

typedef pair<int, pair<string, DictionaryItem>* > HEAP_PAIR;
typedef hash_map<string, pair<int, int>> HashDict;

#define MAX_INDEX_FILE 40000
//#define MAX_INDEX_FILE 1000
#define MAX_BLOCK_SIZE 1000000
class Doc {
	public:
		string url;
//		int id;
		int len;
		int file_id;
		long offset;
};

class Index {
	private:
		string indexDir;
		string inputpath;
		vector<string> fileNames;
		
		map<int, string> file_map;
		map<long, string> docname; //doc id and name
		map<long, int> doc_len;
		map<string,DictionaryItem> index; //index :term - postings list
		map<long, Doc> url_map;
		//string dataDir;
		long docid;
		int block_size, index_num, block_num; //file_num, 
		HashDict dict; // term, block-id, offset
		//string getTempBlockName(int i);
		pair<string, DictionaryItem> * loadPostingList(ifstream & input); // 6
		void writeCompressPostingList(ofstream & fout, ofstream & strfile, ofstream & dicfile, pair<string, DictionaryItem> & pl);
		void writePostingList(ofstream & fout, pair<string, DictionaryItem> & pl); // 7
	public:
	//	static const string datapath;
	//	static const string indexpath;
	//	static const string fileidpath;
		//int blocknum;
		map<int,string> termname; //term id and name
		set<string> stopwords;
		//vector<string> collection;

		Index(string indexDir);
		~Index();
		void getAllFiles(string dir);
		void build();
	//	void readDocsXmlFile(string dir);
	//	void writeIndexFile();
	//	void genIndex(vector<doc> &docs);
		void genAllIndexFiles(string dir); // 1
	//	void writeTempIndex(string dir,string filename);
		void writeTmpBlock(int blocknum); // 4
	//	string parserXml(string dir);
		void getStopwords(string file); // 0
	//	void writeDocId(string file);
		//void genIndexFromFile();
	//	void mergeAllIndex(string allindex,string indextemp,int num);
		void mergeAllIndex(int blocknum); // 5
		void parseAndIndex(string path, int file_id); // 2
		void indexDocument(Doc mydoc, string content);
		void indexDocument(string name, string content); // 3
		
		bool isText(char & c);
};
#endif
