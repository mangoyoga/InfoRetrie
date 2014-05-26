#include"util.h"
//#include"../search/indexreader.h"
char* trim(char *src)
{
	char *fp=src;
	while(*src)
	{
		if(*src!=' '&&*src!='\t'&&*src!='\r')
		{
			fp=src;
			break;
		}
		src++;
	}
	while(*src)
	{
		if(*src==' '||*src=='\t'||*src=='\r')
		{
			*src='\0';
			break;
		}
		src++;
	}
	return fp;
}
string strtolow(string str)
{
	transform(str.begin(),str.end(),str.begin(),::tolower);
	return str;
}
//if have time, to be optimize!
void bool_not(list<int>& a,vector<int>& b, list<int>& c)
{
	vectolis(b,a);
    for(list<int>::iterator it = c.begin();it!=c.end();it++)
    {
    	a.remove(*it);
    }
}
void bool_and(list<int>& a,list<int>& b, list<int>& c)
{
	a.clear();
    list<int>::iterator it = b.begin();
    list<int>::iterator itc = c.begin();
    while(it!=b.end()&&itc!=c.end())
    {
    	if(*it==*itc)
    	{
    		a.push_back(*it);
    		it++;
    		itc++;
    	}
    	else if(*it>*itc)
    		itc++;
    	else
    		it++;
    }
}
void bool_or(list<int>& a,list<int>& b, list<int>& c)
{
	a.clear();
    list<int>::iterator it = b.begin();
    list<int>::iterator itc = c.begin();
    while(it!=b.end()&&itc!=c.end())
    {
    	if(*it==*itc)
    	{
    		a.push_back(*it);
    		it++;
    		itc++;
    	}
    	else if(*it>*itc)
    	{
    		a.push_back(*itc);
    		itc++;
    	}
    	else
    	{
    		a.push_back(*it);
    		it++;
    	}
    }
    if(it==b.end())
    {
    	while(itc!=c.end())
    	{
    		a.push_back(*itc);
    		itc++;
    	}
    }
    if(itc==c.end())
    {
    	while(it!=b.end())
    	{
    		a.push_back(*it);
    		it++;
    	}
    }
}
void tokenspace(string str,vector<string> &tokens)
{
	istringstream stream(str);
	string word;
	while(stream>>word)
		tokens.push_back(word);
}
void positionintersect(vector<pos> &p1,vector<pos> &p2,vector<pos> &hits,int k)
{
	vector<pos>::iterator it1=p1.begin();
	vector<pos>::iterator it2=p2.begin();
	while(it1!=p1.end()&&it2!=p2.end())
	{
		if(it1->docid==it2->docid)
		{
			pos hit;
			vector<int>::iterator p2=it2->position.begin();
			vector<int>::iterator p1=it1->position.begin();
			while(p1!=it1->position.end())
			{
				while(p2!=it2->position.end())
				{
					if(abs(*p1-*p2)<k)
					{
						hit.position.push_back(*p1);
						hit.position.push_back(*p2);
					}
					else if(*p2>*p1)
						break;
					p2++;
				}
				p1++;
			}
			if(hit.position.size()>0)
			{
				hit.docid=it1->docid;
				hits.push_back(hit);
			}
			it1++;
			it2++;
		}
		else if(it1->docid>it2->docid)
			it2++;
		else 
			it1++;
	}
}		
void postolis(vector<pos> &v,list<int> &l)
{
	for(vector<pos>::iterator it=v.begin();it!=v.end();it++)
		l.push_back(it->docid);
}			
void vectolis(vector<int> &v,list<int> &l)
{
	for(vector<int>::iterator it=v.begin();it!=v.end();it++)
		l.push_back(*it);
}
int minTri(int a,int b,int c)
{
	return a<b?(a<c?a:c):(b<c?b:c);
}
/*int levenshteinDistance(string a,string b)
{
	int m[a.length()+1][b.length()+1];
	for(int i=0;i<a.length()+1;i++)
		m[i][0]=i;
	for(int j=0;j<b.length()+1;j++)
		m[0][j]=j;
	for(int i=1;i<a.length()+1;i++)
	{
		for(int j=1;j<b.length()+1;j++)
		{
			if(a[i]==b[j])
				m[i][j]=minTri(m[i-1][j]+1,m[i][j-1]+1,m[i-1][j-1]);
			else
				m[i][j]=minTri(m[i-1][j]+1,m[i][j-1]+1,m[i-1][j-1]+1);
			//cout<<m[i][j]<<" ";
		}
		//cout<<endl;
	}
	return m[a.length()][b.length()];
}*/				
//test
/*
int main()
{
	string str="who are you";
	vector<string> tokens;
	tokenspace(str,tokens);
	for(vector<string>::iterator it=tokens.begin();it!=tokens.end();it++)
	{
		//cout<<*it<<"-"<<endl;
	}
	IndexReader in;
	in.genIndexFromFile();
	vector<pos> p1=in.index[tokens[0]];
	vector<pos> p2=in.index[tokens[1]];
	//cout<<"----"<<endl;
	for(vector<pos>::iterator its=p1.begin();its!=p1.end();its++)
	{
		
		//cout<<its->docid<<" "<<in.docname[its->docid]<<endl;
	}
	//cout<<"----"<<endl;
	for(vector<pos>::iterator its=p2.begin();its!=p2.end();its++)
	{

		//cout<<its->docid<<" "<<in.docname[its->docid]<<endl;
	}
	//cout<<"----"<<endl;
	vector<pos> p3;
	positionintersect(p1,p2,p3,10);
	cout<<"start"<<endl;
	cout<<p3.size()<<endl;
	for(vector<pos>::iterator its=p3.begin();its!=p3.end();its++)
	{
		//cout<<"loop"<<endl;
		cout<<its->docid<<" "<<in.docname[its->docid]<<endl;
	}
	cout<<"end"<<endl;
	cout<<levenshteinDistance("bruts","abuts")<<endl;
	return 0;
}	*/
