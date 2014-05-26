#ifndef UTIL_H
#define UTIL_H
#include <iostream>
#include <stdio.h>
#include <map>
#include <string>
#include <dirent.h>
#include <list>
#include <cstring>
#include <vector>
#include <sys/types.h>
#include <stdlib.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cmath>
#include <set>

using namespace std;
#define BUF 1024*1024
#define K 10
typedef struct document{
	string fname;
	string fcontent;
}doc;
typedef struct docposition{
	int docid;
	vector<int> position;
}pos;
typedef struct term_doclist{
	string term;
	vector<pos> list;
}term_doclist;

template <class T>
class Myheap
{
private:
	typedef bool(*COMP_FUNC)(const T&,const T&);
	COMP_FUNC _comp;
	vector<T> _data;

public:
	Myheap(COMP_FUNC compare_function=NULL)
		:_comp(compare_function)
	{
		if(NULL == _comp)
			make_heap(_data.begin(),_data.end());
		else
			make_heap(_data.begin(),_data.end(),_comp);

	}
	void add(const T& a)
	{
		_data.push_back(a);
		if(NULL==_comp)
			push_heap(_data.begin(),_data.end());
		else
			push_heap(_data.begin(),_data.end(),_comp);
	}
	T remove()
	{
		if(NULL==_comp)
			pop_heap(_data.begin(),_data.end());
		else
			pop_heap(_data.begin(),_data.end(),_comp);
		T rm=_data.back();
		_data.pop_back();
		return rm;

	}
	bool isEmpty() const
	{
		return _data.empty();
	}
	void clear()
	{
		_data.clear();
	}

};




char* trim(char *src);

string strtolow(string str);
void bool_not(list<int>&,vector<int>&,list<int>&);
void bool_and(list<int>&,list<int>&,list<int>&);
void bool_or(list<int>&,list<int>&,list<int>&);
void tokenspace(string str,vector<string> &tokens);
void positionintersect(vector<pos> &p1,vector<pos> &p2,vector<pos> &hits,int k);
void postolis(vector<pos> &v,list<int> &l);
void vectolis(vector<int> &v,list<int> &l);
//int levenshteinDistance(string a,string b);
int minTri(int a,int b,int c);
#endif
