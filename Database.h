#pragma once

#include "RecordFile.h"
#include "BTreeFile.h"

class Database
{
public:
	Database();
	~Database();

	void gui();

	bool insertWithoutKey(Trapezoid trapezoid);
	bool insert(Record record_with_key);

	Record get(Key key);
	void remove(Key key);
	void modify(Key key, Trapezoid trapezoid);


private:
	RecordFile main_file;
	BTreeFile index_file;
};