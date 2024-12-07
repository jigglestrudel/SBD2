#include "Record.h"

Record::Record(Key k, Trapezoid v)
{
	this->_key = k;
	this->_value = v;
}

Record::~Record()
{
}

Key Record::getKey()
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
