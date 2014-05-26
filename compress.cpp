#include <string>
#include "compress.h"
#include "index.h"
using namespace std;

unsigned char VB10000000 = 1 << 7;
unsigned char VB01111111 = VB10000000 - 1;

void VBWrite(ofstream & fout, int value)
{
	unsigned char tmp;
	while (value >= 128) {
		tmp = value % 128;
		fout.write((char*)&tmp, sizeof(BYTE));
		value /= 128;
	}
	tmp = value + 128;
	fout.write((char*)&tmp, sizeof(BYTE));
}

int VBRead(ifstream & fin)
{
	unsigned char tmp;
	int mul = 1;
	int value = 0;
	while (true) {
		fin.read((char *)&tmp, sizeof (unsigned char));
		value += mul * (tmp & VB01111111);
		mul *= 128;
		if (tmp >= 128)
			break;
	}
	return value;
}

// dictionary as a string
void Compress::indexCompress(string dir, string infile, string outstr, string outpnt)
{
	dir.append("\\");
	string inpath = dir + infile, strpath = dir + outstr, pntpath = dir + outpnt;
	ifstream fin(inpath);
	if (!fin)
	{
		printf("open infile %s error!\n", inpath.c_str());
		exit(1);
	}
	ofstream fstrout(strpath);
	if (!fstrout)
	{
		printf("open outfile %s error!\n", strpath.c_str());
		exit(1);
	}
	ofstream fpntout(pntpath, ios::binary);
	if (!fpntout)
	{
		printf("open outfile %s error!\n", pntpath.c_str());
		exit(1);
	}
	
	int block_id, offset;
	long point;
	string term;
	while (fin >> term >> block_id >> offset)
	{
	/*	num++;
		num%=k;
	//	length=term.length();*/
		fpntout.write((char*)(&block_id), sizeof(int));
		fpntout.write((char*)(&offset), sizeof(int));
		point = fstrout.tellp();
		fpntout.write((char*)(&point), sizeof(long));
		fstrout << term;
	}
	fstrout.close();
	fpntout.close();
	fin.close();
}

void Compress::postingCompress(string inpath, string outpstpath, string outstrpath, string outdicpath, int block_id)
{
	// open files
	ifstream fin(inpath);
	if (!fin)
	{
		printf("open infile %s error!\n", inpath.c_str());
		exit(1);
	}
	ofstream fpst(outpstpath, ios::binary);
	if (!fpst)
	{
		printf("open outfile %s error!\n", outpstpath.c_str());
		exit(1);
	}
	ofstream fstr(outstrpath, ios::app);
	if (!fstr)
	{
		printf("open outfile %s error!\n", outstrpath.c_str());
		exit(1);
	}
	ofstream fdic(outdicpath, ios::app | ios::binary);
	if (!fdic)
	{
		printf("open outfile %s error!\n", outdicpath.c_str());
		exit(1);
	}
	
	cout << "Compressing postings" << block_id << ".txt" << endl;
	string term;
	int df, tf, position;
	long offset_blk;
	int offset_str;
	int lastdocid, curdocid, diff;
	while (fin >> term)
	{
		DictionaryItem di;
		fin >> df;
		for (int i=0; i<df; i++)
		{
			pos posting;
			fin >> posting.docid;
			fin >> tf;
			for (int j=0; j<tf; j++)
			{
				fin >> position;
				posting.position.push_back(position);
			}
			di.postingList.push_back(posting);
		}

		// write dictionary
		offset_blk = fpst.tellp();
		offset_str = fstr.tellp();
		fstr << term;
		fdic.write((char*)(&df), sizeof(int));
		fdic.write((char*)(&block_id), sizeof(int));
		fdic.write((char*)(&offset_blk), sizeof(long));
		fdic.write((char*)(&offset_str), sizeof(int));

		// write postings
		vector<pos> & pl = di.postingList;
		vector<BYTE> vecall;
		vector<BYTE> vecid = VBEncodeNumber(pl.at(0).docid);	// docid
		vecall.insert(vecall.end(), vecid.begin(), vecid.end());

		lastdocid = pl.at(0).docid;
		vector<int>& tmppos = pl.at(0).position;
		tf = tmppos.size();
		vector<BYTE> vectf = VBEncodeNumber(tf);	// tf
		vecall.insert(vecall.end(), vectf.begin(), vectf.end());

		for (int i=0; i<tf; i++)
		{
			vector<BYTE> vecpos = VBEncodeNumber(tmppos.at(i));		// position
			vecall.insert(vecall.end(), vecpos.begin(), vecpos.end());
		}

		for (int i=1; i<df; i++)
		{
			curdocid = pl.at(i).docid;
			diff = curdocid - lastdocid;
			lastdocid = curdocid;
			vector<int> & tmppos = pl.at(i).position;
			tf = tmppos.size();
			vector<BYTE> vecid = VBEncodeNumber(diff);	// docid
			vecall.insert(vecall.end(), vecid.begin(), vecid.end());
			vector<BYTE> vectf = VBEncodeNumber(tf);	// tf
			vecall.insert(vecall.end(), vectf.begin(), vectf.end());
			for (int j=0; j<tf; j++)
			{
				vector<BYTE> vecpos = VBEncodeNumber(tmppos.at(j));	// position
				vecall.insert(vecall.end(), vecpos.begin(), vecpos.end());
			}
		}
		fpst.write((char*)(&vecall[0]), sizeof(BYTE) * vecall.size());
	}

	fpst.close();
	fstr.close();
	fdic.close();
	fin.close();
}

vector<BYTE> Compress::VBEncodeNumber(int n)
{
	vector<BYTE> vec;
	vec.clear();
	while (true)
	{
		vec.insert(vec.begin(), n%128);
		if (n<128)
			break;
		n /= 128;
	}
	vec[vec.size()-1] += 128;
	return vec;
}

int Compress::VBDecode(ifstream & fin)
{
	BYTE byte;
	int n = 0;
	fin.read((char*)&byte, sizeof(BYTE));
	while (byte < 128)
	{
		n *= 128;
		n += (int) byte;
		fin.read((char*)&byte, sizeof(BYTE));
	}
	n *= 128;
	n += ((int)byte - 128);
	return n;
}

DictionaryItem * Compress::GetDI(ifstream & fin, long offset, int df)
{
	fin.seekg(offset, ios::beg);
	DictionaryItem * di = new DictionaryItem;
	vector<pos> & pl = di->postingList;
	int preid = 0, n, diff, tf, tmppos;
	for (int i = 0; i < df; i ++)
	{
		pos tmpp;
		diff = VBDecode(fin);
		preid += diff;
		tmpp.docid = preid;
		tf = VBDecode(fin);
		for (int j = 0; j < tf; j ++)
		{
			tmppos = VBDecode(fin);
			tmpp.position.push_back(tmppos);
		}
		pl.push_back(tmpp);
	}
	return di;
}