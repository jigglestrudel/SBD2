#pragma once
#include <iostream>

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


class Record
{
public:
	Record(int k, Trapezoid v);
	~Record();

	int getKey();
	Trapezoid getValue();
	void setValue(Trapezoid value);

private:
	int _key;
	Trapezoid _value;
};