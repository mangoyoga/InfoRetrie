#include <iostream>
#include "index.h"
using namespace std;

int main(int argc, char ** argv)
{
	if (argc < 2)
	{
		cout << argv[0] << " input_directory\n";
	}
	string path = argv[1];
	Index index("index");
	index.getAllFiles(path);
	index.build();
	return 0;
}