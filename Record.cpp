#include "Record.h"


Record::Record(int k, Trapezoid v)
{
	this->_key = k;
	this->_value = v;
}

Record::~Record()
{
	
}

int Record::getKey()
{
	return this->_key;
}

Trapezoid Record::getValue()
{
	return this->_value;
}

void Record::setValue(Trapezoid value)
{
	this->_value = value;
}
