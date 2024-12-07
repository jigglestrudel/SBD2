#pragma once
#include <iostream>
#include <cstdint>

struct Trapezoid
{
public:
	double a, b, h;
	Trapezoid() : a(0), b(0), h(0) {};
	Trapezoid(double a, double b, double h) : a(a), b(b), h(h) {};
	friend std::ostream& operator<< (std::ostream& stream, const Trapezoid& trapezoid)
	{
		return stream << "(" << trapezoid.a << ", " << trapezoid.b << ", " << trapezoid.h << ")";
	};
};

typedef std::uint64_t Key;

class Record
{
public:
	Record(Key k, Trapezoid v);
	~Record();

	Key getKey();
	Trapezoid getValue();
	void setValue(Trapezoid value);

	friend std::ostream& operator<< (std::ostream& stream, const Record& record)
	{
		return stream << record._key << ": " << record._value;
	};

private:
	Key _key;
	Trapezoid _value;
};