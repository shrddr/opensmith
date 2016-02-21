#pragma once
template <class T>
class CircularBuffer
{
public:
	CircularBuffer(size_t size) : _size(size), _pos(0)
	{
		_data = new T[size];
		for (size_t i = 0; i < size; i++)
			_data[i] = 0;
	}
	~CircularBuffer()
	{
		delete[] _data;
	}
	void push_back(const T& value)
	{
		_data[_pos] = value;
		_pos = (_pos + 1) % _size;
	}
	T& operator[](int i)
	{
		return _data[(_pos + i) % _size];
	}
	size_t size() { return _size; }
private:
	T* _data;
	size_t _size;
	size_t _pos;
};