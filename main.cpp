#include <iostream>
#include <cstdlib>

#include "Database.h"

int main(int argc, char** argv)
{
	Database db = Database();
	db.start();
	return 0;
}