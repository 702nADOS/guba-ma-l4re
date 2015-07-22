#pragma once

/*basic implementation of a generic single-linked list*/


#include "global.h"

template <typename T>
class list {

private:
	T _item;
	list* _next;

public:


	list(T item);
	~list() {
		if (_next != NULL) {
			delete _item;
			return;
		}
		else {
			delete _item;
			delete _next;
			return;
		}
	}

	void add(list<T>* next);
	void add(T next);
	void add(T next, unsigned size);
	T get();
	T get(unsigned id);
	unsigned size();
	bool valid(int id);
	list<T>* next();
	bool contains(T item);
	unsigned pos(T item);
	list<T>* remove(T item);

};


template <typename T>
list<T>::list(T item) : _item(item), _next(NULL) {};
template <typename T>
void list<T>::add(list<T>* next) {
	if (_next == NULL)
		_next = next;
	else
		_next->add(next);
}
template <typename T>
void list<T>::add(T next) {
	if (_next == NULL)
		_next = new list<T>(next);
	else
		_next->add(next);
}
template <typename T>
void list<T>::add(T next, unsigned size) {
	for (int i = 0; i < size; ++i)
	{
		add(new list<T>(next[i]));
	}
}
template <typename T>
T list<T>::get() {
	return _item;
}
template <typename T>
T list<T>::get(unsigned id) {
	if (id <= 0)
		return _item;
	else
		return _next->get(id - 1);
}
template <typename T>
unsigned list<T>::size() {
	if(this==NULL)
	{
		return 0;
	}
	if (_next == NULL)
		return 1;
	else
		return _next->size()+1;
}
template <typename T>
bool list<T>::valid(int id){
	if(id >= this->size() || id < 0)
		return false;
	else
		return true;
}
template <typename T>
list<T>* list<T>::next() {
	return _next;
}
template <typename T>
bool list<T>::contains(T item) {
	if(this==NULL)
		return false;
	else if (_item == item)
		return true;
	else if (_next == NULL)
		return false;
	else
		_next->contains(item);
}
template <typename T>
unsigned list<T>::pos(T item) {
	if (_item == item)
		return 0;
	else if (_next == NULL)
		return -1;
	else{
		unsigned pos = _next->pos(item);
		if(pos<0)
			return -1;
		else
			return pos+1;
	}
}
template <typename T>
list<T>* list<T>::remove(T item) {
	if (_item == item)
	{
		list* next = _next;
		delete this;
		return next;
	}
	else if (_next == NULL)
	{
		return &this;
	}
	else {
		_next = _next->remove(item);
		return this;
	}
}

