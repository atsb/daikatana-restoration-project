#pragma once

//
//  This is the most basic of all possible linked list template classes.
//

template <class C>
class list_element {
  protected:
    //the item.
    C item;

    //the next item in the list.
    list_element<C> *next;

  public:
    //
    //  member access
    //
    __inline C &Item();
    __inline const C &Item() const;
    __inline list_element<C> *&Next();
    __inline const list_element<C> *Next() const;
    __inline operator const C &() const;
    __inline operator C &();
};

template <class C>
C &list_element<C>::Item() {
    return item;
}

template <class C>
const C &list_element<C>::Item() const {
    return item;
}

template <class C>
list_element<C> *&list_element<C>::Next() {
    return next;
}

template <class C>
const list_element<C> *list_element<C>::Next() const {
    return next;
}

template <class C>
list_element<C>::operator const C &() const {
    return item;
}

template <class C>
list_element<C>::operator C &() {
    return item;
}


//
//  Next we have 3 template classes for wrapping the pointer to the first element of
//  the list.  The first is list<>, which should be used for integral types, such
//  as 'int', 'float', or 'class *'.  Next is list_object<>, which stores objects in the 
//  list elements.  Last is list_pointer<>, which is made for storing pointer<> types.
//

//
//  This template class is for storing itegral types such as 'int', float', and 'class *'.
//

template <class C>
class list {
  public:
    list();
    ~list();

  protected:
    list_element<C> *first;

  public:
    //
    //  member access
    //
    __inline list_element<C> *First();
    __inline const list_element<C> *First() const;

    //
    //for modifying the list
    //
    __inline void Add(C item);
};

template <class C>
list<C>::list() {
    first = NULL;
}

template <class C>
list<C>::~list() {
    //walk down the list and delete all the elements.
    while (first != NULL) {
        //get the next element.
        list_element<C> *next = first->Next();

        //delete the front list element.
        delete first;

        //the next item is now first.
        first = next;
    }
}

template <class C>
list_element<C> *list<C>::First() {
    return first;
}

template <class C>
const list_element<C> *list<C>::First() const {
    return first;
}

template <class C>
void list<C>::Add(C item) {
    //make a new element.
    list_element<C> *element = new list_element<C>;

    //initialize the element.
    element->Item() = item;

    //make it point at the first item in the list.
    element->Next() = first;

    //make this our first element.
    first = element;
}

//
//  For storing actual objects.  Calling the Add member will require a operator= function defined
//  for the class.
//

template <class C>
class list_object {
  public:
    list_object();
    ~list_object();

  protected:
    list_element<C> *first;

  public:
    //
    //  member access
    //
    __inline list_element<C> *First();
    __inline const list_element<C> *First() const;

    //
    //for modifying the list
    //
    __inline void Add(C &item);
    __inline C *New();
};

template <class C>
list_object<C>::list_object() {
    first = NULL;
}

template <class C>
list_object<C>::~list_object() {
    //walk down the list and delete all the elements.
    while (first != NULL) {
        //get the next element.
        list_element<C> *next = first->Next();

        //delete the front list element.
        delete first;

        //the next item is now first.
        first = next;
    }
}

template <class C>
list_element<C> *list_object<C>::First() {
    return first;
}

template <class C>
const list_element<C> *list_object<C>::First() const {
    return first;
}

template <class C>
void list_object<C>::Add(C &item) {
    //make a new element.
    list_element<C> *element = new list_element<C>;

    //initialize the element.
    element->Item() = item;

    //make it point at the first item in the list.
    element->Next() = first;

    //make this our first element.
    first = element;
}

template <class C>
C *list_object<C>::New() {
    //make a new element.
    list_element<C> *element = new list_element<C>;

    //put it at the beginning of the list.
    element->Next() = first;
    first = element;

    //return the address of the item in the element.
    return &element->Item();
}


//
//  This template class is specificly for storing objects of pointer<> type.
//

template <class C>
class list_pointer {
  public:
    list_pointer();
    ~list_pointer();

  protected:
    list_element<pointer<C> > *first;

  public:
    //
    //  member access
    //
    __inline list_element<pointer<C> > *First();

    //
    //for modifying the list
    //
    __inline void Add(C *item);
    __inline void Add(pointer<C> &item);
};

template <class C>
list_pointer<C>::list_pointer() {
    first = NULL;
}

template <class C>
list_pointer<C>::~list_pointer() {
    //walk down the list and delete all the elements.
    while (first != NULL) {
        //get the next element.
        list_element<pointer<C> > *next = first->Next();

        //delete the front list element.
        delete first;

        //the next item is now first.
        first = next;
    }
}

template <class C>
list_element<pointer<C> > *list_pointer<C>::First() {
    return first;
}

template <class C>
void list_pointer<C>::Add(C *item) {
    //make a new element.
    list_element<pointer<C> > *element = new list_element<pointer<C> >;

    //initialize the element.
    element->Item() = item;

    //make it point at the first item in the list.
    element->Next() = first;

    //make this our first element.
    first = element;
}

template <class C>
void list_pointer<C>::Add(pointer<C> &item) {
    //make a new element.
    list_element<pointer<C> > *element = new list_element<pointer<C> >;

    //initialize the element.
    element->Item() = item;

    //make it point at the first item in the list.
    element->Next() = first;

    //make this our first element.
    first = element;
}




