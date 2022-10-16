//#pragma once

#ifndef _DK_POINTER_H_
#define _DK_POINTER_H_

//nice macro to define pClass types.
#define pointer_type(classname) typedef pointer<classname> p##classname

//encapsulates a pointer to an object of type C.  class C must have standard reference tracking defined.
template <class C>
class pointer {
  public:
    __inline pointer();
    __inline ~pointer();

    //for statements such as: pClass var = a_ptr_to_Class
    __inline pointer(C *item);

    __inline pointer(pointer<C> &ptr);

    //to assign a pointer to the item.
    __inline pointer<C> &operator=(C *item);

    __inline pointer<C> &operator=(pointer<C> &ptr);

    //to get a pointer to the item.
    __inline operator C *();
    __inline operator const C *() const;
    __inline operator C &();

    __inline C *operator->();

  protected:
    //the object we point to.
    C *item;
};

template <class C>
pointer<C>::pointer() {
    item = NULL;
}

template <class C>
pointer<C>::pointer(C *ptr) {
    item = NULL;

    *this = ptr;
}

template <class C>
pointer<C>::~pointer() {
    //decrement the count of our item.
    delcdec(item);
}

template <class C>
pointer<C>::pointer(pointer<C> &ptr) {
    //set our pointer.
    setinc(item, ptr.item);
}

template <class C>
pointer<C> &pointer<C>::operator=(C *ptr) {
    //increment the count on the new item.
    inc(ptr);

    //decrement the count of our old item.
    delcdec(item);

    //set our pointer.
    item = ptr;

    return *this;
}

template <class C>
pointer<C> &pointer<C>::operator=(pointer<C> &ptr) {
    //increment the count of our new item.
    inc(ptr.item);

    //decrement our old item.
    delcdec(item);

    //set our new pointer.
    item = ptr.item;

    return *this;
}

template <class C>
pointer<C>::operator C *() {
    return item;
}

template <class C>
pointer<C>::operator const C *() const {
    return item;
}

template <class C>
pointer<C>::operator C &() {
    return *item;
}

template <class C>
C *pointer<C>::operator->() {
    return item;
}

#endif // _DK_POINTER_H_



