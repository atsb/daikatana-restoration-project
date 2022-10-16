//#pragma once

#ifndef _DK_ARRAY_H
#define _DK_ARRAY_H

#include "dk_misc.h"

template <class C>
class array {
  public:
    array();
    ~array();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Reset();

    void operator=(const array<C> &other);

    void Save(FILE *file) const;
    void Load(FILE *file, int version);

  protected:
    //the number of elements in our array.
    int num;

    //our array of pointers to the objects.
    C **items;

    void Insert(C *item, int position);

  public:
    //
    //member access functions
    //
    int Num() const;
    const C *Item(int num) const;
    //finds the item, assuming the array is sorted.
    const C *Find(const void *identifier, int (*find_func)(const C *item, const void *id));

    //
    //functions to modify the array in some way
    //
    void Add(C *item);
    void Remove(C *item);
    void Remove(int num);
    C *Item(int num);
    C *Item(const void *identifier, int (*find_func)(const C *item, const void *id));
    void Add(C *item, int (*compare_func)(const C **item1, const C **item2));
    void Add(C *item, int index);

    void Sort(int (*compare_func)(const C **item1, const C **item2));
};

//no-nonsense wrapper for very simple embedded object array.  
template <class C>
class array_obj {
  public:
    array_obj();
    ~array_obj();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void operator=(const array_obj<C> &other);

    void Save(FILE *file) const;
    void Load(FILE *file, int version);

  protected:
    //the length of the array.
    int num;

    //the array of items.
    C *items;

  public:
    //
    // member access
    //
    int Num() const;
    const C &Item(int index) const;
    const C &operator[](int index) const;

    //sets the length of the array.  this call will erase all existing data.
    void Length(int len);

    //adds an element to the end of the array, increasing the length by 1.
    //the operator=(const C &other) function or equivalent must be implemented for this call to compile.
    void Add(const C &item);
    
    //increases length by 1, moves elements [position, num-1] up an index, and adds the element at index=position
    //the operator=(const C &other) function or equivalent must be implemented for this call to compile.
    void Insert(const C &item, int position);

    //removes an element from the array, decreasing the length by 1.
    //the operator=(C &other) function must be implemented for this call to compile.
    void Remove(int index);

    //gets a pointer to an item that can be modified.
    C &Item(int index);
};


template <class C>
array<C>::array() {
    num = 0;
    items = NULL;
}

template <class C>
array<C>::~array() {
    Reset();
}

template <class C>
void array<C>::Reset() {
    //delete the items in our array.
    for (int i = 0; i < num; i++) {
        delete items[i];
    }

    //delete the array.
    delca(items);

    num = 0;
}

template <class C>
void array<C>::operator=(const array<C> &other) {
    //delete all of our shit first.
    Reset();

    num = other.num;

    //check if we need to copy any items over from the other array.
    if (num < 1) return;

    //make a new array.
    items = new C *[num];

    //copy the items over.
    for (int i = 0; i < num; i++) {
        //allocate a new item.
        items[i] = new C;

        //copy the item from the other array.
        *items[i] = *other.items[i];
    }
}

template <class C>
void array<C>::Save(FILE *file) const {
    //save the number of items we have.
    fwrite(&num, sizeof(num), 1, file);

    //save each item
    for (int i = 0; i < num; i++) {
        items[i]->Save(file);
    }
}

//can be run from the exe only
template <class C>
void array<C>::Load(FILE *file, int version) {
    Reset();

    //load the number of items
    fread(&num, sizeof(num), 1, file);

    //allocate our array and load the items.
    if (num > 0) {
        items = new C *[num];

        for (int i = 0; i < num; i++) {
            //allocate the object.
            items[i] = new C;

            items[i]->Load(file, version);
        }
    }
}

//
//member access functions
//
template <class C>
int array<C>::Num() const {
    return num;
}

template <class C>
const C *array<C>::Item(int index) const {
    if (index < 0 || index >= num) return NULL;

    return items[index];
}

//
//functions to modify the array in some way
//
template <class C>
void array<C>::Add(C *item) {
    if (item == NULL) return;

    //insert the item at the end of the array.
    Insert(item, num);
}

template <class C>
void array<C>::Add(C *item, int index) {
    if (item == NULL) return;
    if (index < 0) return;

    //keep the at or below num.
    bound_max(index, num);

    //insert the item at the given index.
    Insert(item, index);
}

template <class C>
void array<C>::Remove(C *item) {
    //find the index of the item.
    for (int i = 0; i < num; i++) {
        if (item == items[i]) {
            //we found the item.
            Remove(i);
            return;
        }
    }
}

template <class C>
void array<C>::Remove(int index) {
    //check the number.
    if (index < 0 || index >= num) return;

    //check if this is the only item.
    if (num < 2) {
        //clear out the whole array.
        Reset();
        return;
    }

    //make a new array.
    C **newarray = new C *[num - 1];

    //copy the items before the index
    for (int i = 0; i < index; i++) {
        newarray[i] = items[i];
    }

    //copy the items after the index.
    for (i++; i < num; i++) {
        newarray[i - 1] = items[i];
    }

    //delete the old array.
    delca(items);
    
    //swap in the new array.
    items = newarray;

    //decrement our count.
    num--;
}

template <class C>
C *array<C>::Item(int index) {
    if (index < 0 || index >= num) return NULL;

    return items[index];
}

template <class C>
C *array<C>::Item(const void *identifier, int (*find_func)(const C *item, const void *id)) {
    return (C *)Find(identifier, find_func);
}

template <class C>
void array<C>::Sort(int (*compare_func)(const C **item1, const C **item2)) {
    if (compare_func == NULL) return;

    //check if we have more than 1 item.
    if (num < 2) return;

    //do the sort.
    qsort(items, num, sizeof(C *), (int (*)(const void *, const void *))compare_func);
}

template <class C>
const C *array<C>::Find(const void *identifier, int (*find_func)(const C *item, const void *id)) {
    if (identifier == NULL || find_func == NULL) return NULL;
    if (num < 1) return NULL;

    //the range of the array that the item is in.
    int top = 0;
    int bottom = num - 1;

    //search until we have gotten rid of most elements.
    for (;bottom - top > 1;) {
        //get the middle index.
        int middle = (top + bottom) / 2;

        //get the relationship of the middle item to the one we are looking for.
        int relation = find_func(items[middle], identifier);

        //check the relation that was returned.
        if (relation == 0) {
            //we found the item, how lucky.
            return items[middle];
        }
        else if (relation == -1) {
            //the item is above the middle.
            bottom = middle - 1;
        }
        else if (relation == 1) {
            //the item is below the middle.
            top = middle + 1;
        }
        else {
            //bad value from the find fuction.
            return NULL;
        }
    }

    //look through the remaining items.
    for (; top <= bottom; top++) {
        if (find_func(items[top], identifier) == 0) {
            //found it
            return items[top];
        }
    }

    //couldn't find it.
    return NULL;
}

template <class C>
void array<C>::Add(C *item, int (*compare_func)(const C **item1, const C **item2)) {
    if (item == NULL) return;
    if (compare_func == NULL) return;

    //check if there are any elements yet.
    if (num < 1) {
        Insert(item, 0);
        return;
    }

    //check if this item goes before the first item.
    if (compare_func((const C **)&item, 
        (const C **)&items[0]) < 0) {
        //this item goes at the beginning of the array.
        Insert(item, 0);
        return;
    }

    //the range of the array that contains the item that will preceed the new item.
    int top = 0;
    int bottom = num - 1;

    //search until we have gotten rid of most elements.
    for (;bottom - top > 1;) {
        //get the middle index.
        int middle = (top + bottom) / 2;

        //get the relationship of the middle item to the one we are looking for.
        int relation = compare_func((const C **)&item, (const C **)&items[middle]);

        //check the relation that was returned.
        if (relation == 0) {
            //the item is identical to the middle item, put this item directly after it.
            Insert(item, middle + 1);
            return;
        }
        else if (relation == -1) {
            //the item goes above the middle.
            bottom = middle - 1;
        }
        else if (relation == 1) {
            //the item goes below the middle.
            top = middle;
        }
        else {
            //bad value from the find fuction.
            return;
        }
    }

    //ok, now look through the remaining items.
    for (; top <= bottom; top++) {
        //check the relation.
        int relation = compare_func((const C **)&item, (const C **)&items[top]);

        if (relation == 0) {
            //put the item directly after this one.
            Insert(item, top + 1);
            return;
        }
        else if (relation < 0) {
            //the item goes above this item.
            Insert(item, top);
            return;
        }
    }
    
    //insert the item at the end.
    Insert(item, num);
}

template <class C>
void array<C>::Insert(C *item, int position) {
    if (item == NULL) return;
    if (position < 0 || position > num) return;

    //make a new array.
    C **newarray = new C *[num + 1];

    //copy the items before the new item.
    for (int i = 0; i < position; i++) {
        newarray[i] = items[i];
    }

    //put the item in.
    newarray[position] = item;

    //copy over the items that are after the item.
    for (i = position; i < num; i++) {
        newarray[i + 1] = items[i];
    }

    //delete the old array.
    delca(items);

    //swap in the new arra
    items = newarray;

    //increment our count.
    num++;
}






template <class C>
array_obj<C>::array_obj() {
    items = NULL;
    num = 0;
}

template <class C>
array_obj<C>::~array_obj() {
    delca(items);
}

template <class C>
void array_obj<C>::operator=(const array_obj<C> &other) {
    //delete our old data.
    delca(items);

    //get the number of items we have.
    num = other.num;

    //allocate an array.
    if (num < 1) return;

    items = new C[num];

    //copy the items.
    for (int i = 0; i < num; i++) {
        items[i] = other.items[i];
    }
}

template <class C>
int array_obj<C>::Num() const {
    return num;
}

template <class C>
const C &array_obj<C>::Item(int index) const {
    if (index < 0 || index >= num) {
        static C badobj;
        return badobj;
    }

    return items[index];
}

template <class C>
const C &array_obj<C>::operator[](int index) const {
    if (index < 0 || index >= num) {
        static C badobj;
        return badobj;
    }

    return items[index];
}

template <class C>
void array_obj<C>::Length(int len) {
    //delete our old data.
    delca(items);

    if (len < 1) {
        //our length is 0.
        num = 0;
        return;
    }

    num = len;

    //make a new array.
    items = new C[len];
}

template <class C>
C &array_obj<C>::Item(int index) {
    if (index < 0 || index >= num) {
        static C badobj;
        return badobj;
    }

    return items[index];
}

template <class C>
void array_obj<C>::Add(const C &item) {
    //make a new array of objects.
    C *newitems = new C[num + 1];

    //copy over the items.
    for (int i = 0; i < num; i++) {
        newitems[i] = items[i];
    }

    //put the new item in.
    newitems[num] = item;

    //delete the old array.
    delca(items);

    //swap in the new array.
    items = newitems;

    //increment the count.
    num++;
}

template <class C>
void array_obj<C>::Insert(const C &item, int position) {

    // check for valid position
    if (position<0||position>num)
        return;
    
    //make a new array of objects.
    C *newitems = new C[num + 1];

    //copy over the items.
    for (int i = 0; i < position; i++) {
        newitems[i] = items[i];
    }

    for (i = position+1; i < num+1; i++) {
        newitems[i] = items[i-1];
    }

    //put the new item in.
    newitems[position] = item;

    //delete the old array.
    delca(items);

    //swap in the new array.
    items = newitems;

    //increment the count.
    num++;
}

template <class C>
void array_obj<C>::Remove(int index) {
    //make sure we were given a valid index.
    if (index < 0 || index >= num) return;

    //make a new array of items.
    C *newitems = new C[num - 1];

    //copy the items before the index we are removing.
    for (int i = 0; i < index; i++) {
        newitems[i] = items[i];
    }

    //copy the items after the index we are removing.
    for (i++; i < num; i++) {
        newitems[i - 1] = items[i];
    }

    //delete the old array.
    delca(items);

    //swap in the new array.
    items = newitems;

    //decrement the count.
    num--;
}

template <class C>
void array_obj<C>::Save(FILE *file) const {
    //save the number of items.
    fwrite(&num, sizeof(num), 1, file);

    //save each item.
    for (int i = 0; i < num; i++) {
        items[i].Save(file);
    }
}

template <class C>
void array_obj<C>::Load(FILE *file, int version) {
    //delete the old data.
    delca(items);

    //load the number of items.
    fread(&num, sizeof(num), 1, file);

    if (num < 1) return;

    //allocate the array of items.
    items = new C[num];

    //read each item.
    for (int i = 0; i < num; i++) {
        items[i].Load(file, version);
    }
}

#endif // _DK_ARRAY_H

