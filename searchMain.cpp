#include "search.h"
#include "indexSearch.h"

int main(int argc, char ** argv)
{
	//Search mysearch(argv[1]);
	string line;
	indexSearch index("index");
	cout << "input your word to search. input '/q' to exit.\n";
	while (true)
	{
		cout << "input query(input '/q' to exit): ";
		getline(cin, line);
		if (line == "/q")
			break;
		//mysearch.display(word, mysearch.search(word));
		index.search(line);
	}
	return 0;
}