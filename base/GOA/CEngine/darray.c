/* 
 *
 * File: darray.c
 * ---------------
 *	David Wright
 *	10/8/98
 * 
 *  See darray.h for function descriptions
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "darray.h" 

#define DEF_GROWBY 8

// STRUCTURES
struct DArrayImplementation
{
	int count, capacity;
	int elemsize;
	int growby;
	ArrayElementFreeFn elemfreefn;
	void *list; //array of elements
};

// PROTOTYPES
static void *mylsearch(const void *key, void *base, int count, int size,
					 ArrayCompareFn comparator);

// FUNCTIONS

/* FreeElement
 * Frees the element at position N in the array
 */
static void FreeElement(DArray array, int n)
{
	if (array->elemfreefn != NULL)
		array->elemfreefn(ArrayNth(array,n));
}

/* ArrayGrow
 * Reallocates the array to a new size, incresed by growby
 */
static void ArrayGrow(DArray array)
{
	array->capacity +=  array->growby;
	array->list = realloc(array->list, array->capacity * array->elemsize);
	assert(array->list);
}

/* SetElement
 * Sets the element at pos to the contents of elem
 */
static void SetElement(DArray array, const void *elem, int pos)
{
	memcpy(ArrayNth(array,pos), elem, array->elemsize);
}

DArray ArrayNew(int elemSize, int numElemsToAllocate, 
                ArrayElementFreeFn elemFreeFn)
{
	DArray array;

	array = (DArray) malloc(sizeof(struct DArrayImplementation));
	assert(array);
	assert(elemSize);
	if (numElemsToAllocate == 0)
		numElemsToAllocate = DEF_GROWBY;
	array->count = 0;
	array->capacity = numElemsToAllocate;;
	array->elemsize = elemSize;
	array->growby = numElemsToAllocate;
	array->elemfreefn = elemFreeFn;
	array->list = malloc(array->capacity * array->elemsize);
	assert(array->list);

	return array;
}

void ArrayFree(DArray array)
{
	int i;
	
	assert(array);
	for (i = 0; i < array->count; i++)
	{
		FreeElement(array, i);
	}
	free(array->list);
	free(array);
}


int ArrayLength(const DArray array)
{
	return array->count;
}

void *ArrayNth(DArray array, int n)
{
	assert( (n >= 0) && (n < array->count));
	
	return (char *)array->list + array->elemsize*n;
}

/* ArrayAppend
 * Just do an Insert at the end of the array
 */
void ArrayAppend(DArray array, const void *newElem)
{
	ArrayInsertAt(array, newElem, array->count);
}

void ArrayInsertAt(DArray array, const void *newElem, int n)
{
	assert( (n >= 0) && (n <= array->count));

	if (array->count == array->capacity)
		ArrayGrow(array);
	array->count++;
	if (n < array->count - 1) //if we aren't appending
		memmove(ArrayNth(array, n+1), ArrayNth(array,n), 
				(array->count - 1 - n) * array->elemsize);
	SetElement(array, newElem, n);
}

void ArrayDeleteAt(DArray array, int n)
{
   	assert( (n >= 0) && (n < array->count));

	FreeElement(array,n);
	if (n < array->count - 1) //if not last element
		memmove(ArrayNth(array,n),ArrayNth(array,n+1),
			    (array->count - 1 - n) * array->elemsize);
	array->count--;
}


void ArrayReplaceAt(DArray array, const void *newElem, int n)
{
	assert( (n >= 0) && (n < array->count));

	FreeElement(array, n);
	SetElement(array, newElem,n);
}


void ArraySort(DArray array, ArrayCompareFn comparator)
{
	qsort(array->list, array->count, array->elemsize, comparator);
}

//assert will be raised by ArrayNth if fromindex out of range
int ArraySearch(DArray array, const void *key, ArrayCompareFn comparator, 
                  int fromIndex, int isSorted)
{
	void *res;

	if (array->count == 0)
		return NOT_FOUND;

   	if (isSorted)
		res=bsearch(key, ArrayNth(array,fromIndex),
					array->count - fromIndex, array->elemsize, comparator);
	else
		res=mylsearch(key, ArrayNth(array, fromIndex), 
					  array->count - fromIndex, array->elemsize, comparator);
	if (res != NULL)
		return (((char *)res - (char *)array->list) / array->elemsize);
	else
		return NOT_FOUND;
}


void ArrayMap(DArray array, ArrayMapFn fn, void *clientData)
{
	int i;

	assert(fn);

	for (i = 0; i < array->count; i++)
		fn(ArrayNth(array,i), clientData);
		
}

/* mylsearch
 * Implementation of a standard linear search on an array, since we
 * couldn't use lfind
 */
static void *mylsearch(const void *key, void *base, int count, int size,
					 ArrayCompareFn comparator)
{
	int i;
	for (i = 0; i < count; i++)
	{
		if (comparator(key, (char *)base + size*i) == 0)
			return (char *)base + size*i;
	}
	return NULL;
}
