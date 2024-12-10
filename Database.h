#pragma once
#include <string>

#include "RecordFile.h"
#include "BTreeFile.h"

#define COLLECTIONS_DIR "collections"
#define DEFAULT_D 2
#define DEFAULT_B 10
#define EMPTY_LIMIT 0.5

class Database
{
public:
	Database();
	~Database();

	void start();

	void openCollection(std::string name);
	bool createCollection(std::string name, int d, int b);
	void deleteCollection(std::string name);
	void close();

	void readInstructionsFile();
	void tui();

	Key insertWithoutKey(Trapezoid trapezoid, bool count_operations);
	bool insert(Record record_with_key);

	Record get(Key key);
	void remove(Key key);
	void modify(Key key, Trapezoid trapezoid);

	void printFiles();
	void printAllInOrder();

private:
	void resetOperationsCount();
	void printOperationsCount();
	int getDriveReads();
	int getDriveWrites();

	bool is_open;
	std::string name;
	RecordFile* main_file;
	BTreeFile* index_file;
};