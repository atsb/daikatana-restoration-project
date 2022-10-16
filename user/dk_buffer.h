//#pragma once

#ifndef _DK_BUFFER_H
#define _DK_BUFFER_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

//a template class for a fixed length buffer class.
template <int buffer_len>
class CBuffer {
  public:
    CBuffer();
    //pass parametes like printf
    CBuffer(const char *format_string, ...);

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    //sets the string.  pass parameters like printf.
    void Set(const char *format_string, ...);

    void StripInvalidChars(int max_len = -1);

    //returns the name as a null terminated string.
    operator const char *() const;

    //returns true if the string is non-empty.
    operator bool() const;

    //copies the given null-terminated string.  NULL or empty strings set the buffer to ""
    const char *operator=(const char *str);

    char operator[](int i) const;

    void Save(FILE *file) const;
    void Load(FILE *file, int version);

    void NewEnding(int position, const char *new_ending);
    const char *Right(int num_chars_skipped) const;
    void InsertChar(char c, int pos);
    void RemoveChar(int pos);

  protected:
    char buffer[buffer_len];
};

template <int buffer_len>
CBuffer<buffer_len>::CBuffer() {
    buffer[0] = '\0';
}

template <int buffer_len>
CBuffer<buffer_len>::CBuffer(const char *format_string, ...) {
    //get our parameter list.
    va_list args;
    va_start(args, format_string);

//unix - _vsnprintf / vsprintf
#if _MSC_VER
    //call _vsnprintf
    _vsnprintf(buffer, buffer_len, format_string, args);
#else
	int len = vsprintf(buffer, format_string, args);
	assert(len < buffer_len);
#endif

    //put a null at the end.
    buffer[buffer_len - 1] = '\0';
}

template <int buffer_len>
void CBuffer<buffer_len>::Set(const char *format_string, ...) {
    //get our parameter list.
    va_list args;
    va_start(args, format_string);

//unix - _vsnprintf / vsprintf
#if _MSC_VER
    //call _vsnprintf
    _vsnprintf(buffer, buffer_len, format_string, args);
#else
	int len = vsprintf(buffer, format_string, args);
	assert(len < buffer_len);
#endif

    //put a null at the end.
    buffer[buffer_len - 1] = '\0';
}


template <int buffer_len>
void CBuffer<buffer_len>::StripInvalidChars(int max_len)
{
  char bogus_chars[] = { '%', '/', '\\' };
  char *n;
  int loop, i, len, slen;

  n = buffer;
  len = sizeof(bogus_chars);
  slen = strlen(buffer);

  for (i = 0;i < slen;i++)
  {
    for (loop = 0;loop < len;loop++)
    {
      if (*n == bogus_chars[loop])
      {
        *n = '_'; // replace with a '_'
        break;
      }
    }

    n++;
  }

  if (max_len > -1) 
    buffer[max_len] = '\0'; // truncate
}




template <int buffer_len>
CBuffer<buffer_len>::operator const char *() const {
    return buffer;
}

template <int buffer_len>
CBuffer<buffer_len>::operator bool() const {
    return buffer[0] != '\0';
}

template <int buffer_len>
const char *CBuffer<buffer_len>::operator=(const char *str) {
    if (str == NULL || str[0] == '\0') {
        buffer[0] = '\0';
        return buffer;
    }

    //copy the string, converting to lower case.
    strncpy(buffer, str, buffer_len - 1);

    //make the last character null.
    buffer[buffer_len - 1] = '\0';

    return buffer;
}

template <int buffer_len>
void CBuffer<buffer_len>::Save(FILE *file) const {
    fwrite(&buffer[0], buffer_len, 1, file);
}

template <int buffer_len>
void CBuffer<buffer_len>::Load(FILE *file, int version) {
    fread(&buffer[0], buffer_len, 1, file);
}

template <int buffer_len>
void CBuffer<buffer_len>::NewEnding(int position, const char *new_ending) {
    if (new_ending == NULL) return;
    if (position < 0 || position >= buffer_len - 1) return;

    //copy the string to the position in the buffer.
    strncpy(&buffer[position], new_ending, buffer_len - 1 - position);

    //make sure there is a 0 at the end.
    buffer[buffer_len - 1] = '\0';
}

template <int buffer_len>
const char *CBuffer<buffer_len>::Right(int num_chars_skipped) const {
    //step through the buffer the specified number of characters.  we must do
    //this to check for the end of the string.
    for (int pos = 0; pos < num_chars_skipped; pos++) {
        //check for the end of the string.
        if (buffer[pos] == '\0') return "";
    }

    //return the portion of the string starting at pos.
    return &buffer[pos];
}

template <int buffer_len>
void CBuffer<buffer_len>::InsertChar(char c, int pos) {
    //check the position.
    if (pos < 0 || pos >= buffer_len) return;

    //move all the characters after the position right one.
    for (int i = buffer_len - 2; i >= pos; i--) {
        buffer[i + 1] = buffer[i];
    }

    //insert the character.
    buffer[pos] = c;

    //make sure we have a 0 at the end.
    buffer[buffer_len - 1] = '\0';
}

template <int buffer_len>
void CBuffer<buffer_len>::RemoveChar(int pos) {
    //check the position.
    if (pos < 0 || pos >= buffer_len - 1) return;

    //move the characters after the pos left one.
    for (int i = pos; i < buffer_len - 2; i++) {
        buffer[i] = buffer[i + 1];
    }
}

template <int buffer_len>
char CBuffer<buffer_len>::operator[](int i) const {
    if (i < 0 || i >= buffer_len) return buffer[0];

    return buffer[i];
}


//some buffer types.
typedef CBuffer<16> buffer16;
typedef CBuffer<24> buffer24;
typedef CBuffer<32> buffer32;
typedef CBuffer<64> buffer64;
typedef CBuffer<128> buffer128;
typedef CBuffer<256> buffer256;

#endif // _DK_BUFFER_H
