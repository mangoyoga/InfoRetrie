#ifndef INDEX_SEARCH_H
#define INDEX_SEARCH_H

#include <string>
#include <map>
#include "index.h"
#include "compress.h"
using namespace std;

struct unit
{
	int df;
	int block_id;
	long offset_blk;
	int offset_str;
};

class indexSearch
{
private:
	string indexdir;
	char * allstr;
	unit * dict;
	int wordNum;
	map<int, string> doc_name;
	map<int, int> doc_len;
	
	string plFileName(int i);

public:
	indexSearch(string dir);
	~indexSearch();
	int compare(int p, string & term);
	int binarySearch(string & term);
	DictionaryItem * find(string term);
	void search(string line);
};

#endif
