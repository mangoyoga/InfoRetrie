#ifndef COMPRESS_H
#define COMPRESS_H

#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include "index.h"
using namespace std;

typedef unsigned char BYTE;

void VBWrite(ofstream & fout, int value);
int VBRead(ifstream & fin);

class Compress
{
public:
	vector<BYTE> VBEncodeNumber(int n);
	void indexCompress(string dir, string infile, string outstr, string outpnt);
	void postingCompress(string inpath, string outpstpath, string outstrpath, string outdicpath, int block_id);
	static int VBDecode(ifstream & fin);
	static DictionaryItem * GetDI(ifstream & fin, long offset, int df);
};
#endif