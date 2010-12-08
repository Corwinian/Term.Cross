/*-----------------------------------------------------------------------------
    c_slst.hpp
-----------------------------------------------------------------------------*/
#ifndef _C_SLST_HPP_
#define _C_SLST_HPP_

#include <c_lib/c_types.hpp>

/*-----------------------------------------------------------------------------
    Простая реализация двунаправленного списка. Отсутствует поддержка
    одновременно нескольких курсоров.
    Элементы списка хранят указатели на объекты, которые ДОЛЖНЫ БЫТЬ созданы
    оператором new. Уничтожение этих объектов производится САМИМ СПИСКОМ, т.е.
    программа НЕ ДОЛЖНА вызывать для них delete.
-----------------------------------------------------------------------------*/
template <class T>
class TElement {
public:
	TElement( T * o, TElement * p, TElement * n ) : object( o ), prev( p ), next( n ) {}

	~TElement() {
		delete object;
	}

	T * object;         // хранимый объект

	TElement * prev;
	TElement * next;
};

template <class T> class TSimpleListCursor;

template <class T>
class TSimpleList {
public:
	TSimpleList(): first( 0 ), last( 0 ), current( 0 ), nelements( 0 ) {};

	~TSimpleList(){
		TElement<T> * p = first;
		while( p ) {
			TElement<T> * next = p->next;
			delete p;
			p = next;
		}
	}

	/*
	    Работа с курсором.
	bool setToFirst();
	bool setToLast();
	bool setToNext();
	bool setToPrev();
	*/
	/*
	    Возвращаемое значение:
	    0   курсор неопределен
	    1   ok
	bool isValid() const;
	*/

	/*
	    Перевести курсор в неопределенное состояние.
	void invalidate();
	*/


	/*
	    Возвращаемое значение:
	    0       курсор не определен
	    <>0     указатель, хранимый в элементе списка, на который указывает курсор
	T * elementAt();
	*/
	T * elementAt(TSimpleListCursor<T>* cursor){
		return cursor->current ? cursor->current->object : 0;
	}

	/*
	    Добавляет элемент следом за курсором. Вызов функции не меняет положения курсора.
	    Возвращаемое значение:
	    0   курсор был неопределен - функция ничего не сделала
	    1   ok
	*/
	bool addAt(TSimpleListCursor<T>* cursor, T* o){
		if( !cursor->isValid() )
			return false;

		TElement<T> * next = cursor->current->next;

		cursor->current->next = new TElement<T>( o, cursor->current, next );

		if( next ) next->prev = cursor->current->next;

		nelements++;

		return 1;
	}


	/*
	    Вызов функции не меняет положения курсора.
	*/
	void addAsFirst( T* o ){
		TElement<T> * second = first;
	
		first = new TElement<T>( o, 0, second );
		if( second ) {       // на момент вызова список уже не был пустым
			second->prev = first;
		} else {               // на момент вызова список был пуст
			last = first;   // last был равен 0
		}

		nelements++;
	}

	/*
	    Вызов функции не меняет положения курсора.
	*/
	void addAsLast( T* o ){
		TElement<T> * beforelast = last;

		last = new TElement<T>( o, beforelast, 0 );
		if( beforelast ) {   // на момент вызова список уже не был пустым
			beforelast->next = last;
		} else {               // на момент вызова список был пуст
			first = last;   // first был равен 0
		}

		nelements++;
	}


	/*
	    Удаляет элемент, на который указывает курсор. После вызова курсор указывает на элемент, следующий за удаленным.
	    Возвращаемое значение:
	    0   курсор был неопределен - функция ничего не сделала
	    1   ok
	*/
	bool removeAt(TSimpleListCursor<T>* cursor){
		if( cursor->current == 0 ) return false;

		TElement<T> * prev = cursor->current->prev;
		TElement<T> * next = cursor->current->next;

		delete cursor->current;

		if( prev ) {
			prev->next = next;
		} else {           // если удаляемый элемент был в списке первым
			first = next;
		}

		if( next ) {
			next->prev = prev;
		} else {           // если удаляемый элемент был в списке последним
			last = prev;
		}

		cursor->current = next;

		nelements--;

		return true;
	}

	void removeAll(){
		//if(first == 0) return;
		for(TElement<T>* current = first; current != 0; ){
			TElement<T>* t = current;
			current = current->next;
			delete t;
		}
		first = 0;
		last = 0;
	}

	/*
	    Возвращаемое значение:
	    0   список не пуст
	    1   список пустой
	*/
	bool isEmpty() const{
		return nelements == 0;
	}

	/*
	    Текущее число элементов в списке.
	*/
	uint32_t numberOfElements() const{
		return nelements;
	}

	/*
	    Применить функцию ко всем элементам списка.
	*/
	void applyToAll( void (* applyFunction)( T *, void * ), void * param = 0 ){
		TElement<T> * p = first;
		while( p ) {
			applyFunction( p->object, param );
			p = p->next;
		}
	}



	TSimpleListCursor<T> createCursor(){
		return TSimpleListCursor<T>(this);
	}

	TSimpleListCursor<T>* createCursorPtr(){
		return new TSimpleListCursor<T>(this);
	}

protected:
	void * cursorPosition() const {
		return current;
	}
	void setCursorPosition( void * c ) {
		current = (TElement<T> *)c;
	}



	// элемент списка

	TElement<T> * first;       // первый и последний элементы списка
	TElement<T> * last;

	TElement<T> * current;     // курсор

	uint32_t nelements;        // размер списка

	friend class TSimpleListCursor<T>;
};

template <class T> class TSimpleListCursor{
public:

	~TSimpleListCursor(){};

	bool setToFirst(){
		return (current = lst->first) != 0;
	}

	bool setToLast(){
		return (current = lst->last) != 0;
	}

	bool setToNext(){
		return current ? (current = current->next) != 0 : 0;
	}

	bool setToPrev(){
		return current ? (current = current->prev) != 0 : 0;
	}

	bool isValid() const {
		return current != 0;
	}

	void invalidate() {
		current = 0;
	}

	T * elementAt(){
		return current ? current->object : 0;
	}
	
protected:
	friend class TSimpleList<T>;

	TSimpleListCursor( TSimpleList<T>* pl ) : lst(pl), current( 0 ) {
		current = lst->first;
	}

	TSimpleList<T>* lst;
	TElement<T> * current;     // курсор

};
#endif
