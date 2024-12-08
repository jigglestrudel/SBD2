#include <iostream>
#include <cstdlib>

#include "RecordFile.h"
#include "BTreeFile.h"

int main(int argc, char** argv)
{
	std::cout << "Hello world!!\n";
	RecordFile plik("test.bin", 10);
	Record rekord(2, { 1,1,1 });

	//plik.findEmptyPlaces();
	//
	//std::cout << plik.getRecordFromFile(2) << "\n";
	//std::cout << plik.getRecordFromFile(1) << "\n";
	//std::cout << plik.getRecordFromFile(3) << "\n";
	//for (int i = 0; i < 20; i++)
	//	std::cout << plik.putRecordInFile(rekord) << "\n";
	////plik.deleteRecordFromFile(10);
	////plik.deleteRecordFromFile(12);
	////plik.deleteRecordFromFile(15);
	//std::cout << plik.putRecordInFile(rekord) << "\n";
	////plik.deleteRecordFromFile(10);

	srand(time(NULL));
	BTreeFile bdrzewo("tree.bin", 2);
	bdrzewo.findEmptyPages();
	bdrzewo.findRoot();
	bdrzewo.showTree();
	bdrzewo.showFile();
	for (uint64_t i = 0; i < 100; i++)
	{
		
		bdrzewo.insert({ (uint64_t)(rand() % 100), 0 });
			//<< "\n";
		bdrzewo.showTree();
	}
	//for (uint64_t i = 0; i < 100; i++)
	{
		//bdrzewo.remove((uint64_t)(rand() % 100));
		//<< "\n";
		//bdrzewo.showTree();
	}
	bdrzewo.offloadAll();
	bdrzewo.showFile();
	
	return 0;
}