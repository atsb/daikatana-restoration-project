#pragma once

#ifndef _DK_HASHTABLE_H_
#define _DK_HASHTABLE_H_

//class that is used to store a linked list of hash table entries.
template <class C>
class hashtable_listentry {
  public:
    //pointer to next item in the list.
    hashtable_listentry<C> *next;

    //the key that was used to insert the item into the table.
    int key;

    //pointer to the item itself.
    C *item;
};

template <class C>
class hashtable {
  public:
    hashtable(int size);
    ~hashtable();

    //resets the table.
    void Reset();

  protected:
    //the size of the hash table array.
    int size;

    //the array of hash lists.
    hashtable_listentry<C> **hasharray;

    //hash function.
    int Hash(unsigned int key);

  public:
    //adds an item to the hash table.
    void Add(C *item, unsigned int key);

    //removes an item from the table.
    void Delete(C *item, unsigned int key);

    //removes the first item that is found that was inserted with the given key.
    C *Delete(unsigned int key);
};


template <class C>
hashtable<C>::hashtable(int size) {
    bound_min(size, 1);

    //set our size.
    this->size = size;

    //allocate our array of hash lists.
    hasharray = new hashtable_listentry<C> *[size];

    //initialize all the list pointers to NULL.
    for (int i = 0; i < size; i++) {
        hasharray[i] = NULL;
    }
}

template <class C>
void hashtable<C>::Reset() {
    //go through the table and delete all the lists.
    for (int i = 0; i < size; i++) {
        //walk the list of hash entries.
        hashtable_listentry<C> *list = hasharray[i];

        while (list != NULL) {
            //get a pointer to the next entry.
            hashtable_listentry<C> *next = list->next;

            //delete the entry.
            delete list;

            //go to the next entry.
            list = next;
        }

        //set the pointer to the list to NULL.
        hasharray[i] = NULL;
    }
}

//magic number to use in the hash function 1/golden ratio.
#define MAGIC_NUMBER  0.6180339887499

//hash function.
template <class C>
int hashtable<C>::Hash(unsigned int key) {
    //multiply the key by our magic number and get the fractional part.
    double product = MAGIC_NUMBER * (key & 0x000fffff);
    double fraction = product - floor(product);

    //get the index we hash to.
    int index = int(fraction * size);
    if (index < 0 || index >= size) {
        index = 0;
    }

    return index;
}

//adds an item to the hash table.
template <class C>
void hashtable<C>::Add(C *item, unsigned int key) {
    if (item == NULL) return;

    //get the index we will put this item given its key.
    int index = Hash(key);

    //make a new hash entry.
    hashtable_listentry<C> *entry = new hashtable_listentry<C>;

    //fill in the data.
    entry->next = hasharray[index];
    entry->item = item;
    entry->key = key;

    //put the entry into it's list.
    hasharray[index] = entry;
}

//removes an item from the table.
template <class C>
void hashtable<C>::Delete(C *item, unsigned int key) {
    if (item == NULL) return;

    //get the index that this item hashes to.
    int index = Hash(key);

    //go through the list at that index and find the item.
    hashtable_listentry<C> **list = &hasharray[index];
    
    while (*list != NULL) {
        //check the item at this entry.
        if ((*list)->item == item) {
            //found it.  get the next entry.
            hashtable_listentry<C> *next = (*list)->next;

            //delete the entry.
            delete (*list);

            //remove the deleted entry from the list.
            *list = next;

            return;
        }

        //go to the next item.
        list = &(*list)->next;
    }
}

//removes the first item that is found that was inserted with the given key.
template <class C>
C *hashtable<C>::Delete(unsigned int key) {
    //get the index that this key hashes to.
    int index = Hash(key);

    //go down the list at that index and find an item that was inserted with the given key.
    hashtable_listentry<C> **list = &hasharray[index];
    
    while (*list != NULL) {
        //check the item at this entry.
        if ((*list)->key == key) {
            //found it.  get the next entry.
            hashtable_listentry<C> *next = (*list)->next;

            //get the item stored in the entry.
            C *item = (*list)->item;

            //delete the entry.
            delete (*list);

            //remove the deleted entry from the list.
            *list = next;

            //return the item.
            return item;
        }

        //go to the next item.
        list = &(*list)->next;
    }

    //couldnt find one.
    return NULL;
}

#endif // _DK_HASHTABLE_H_




