#ifndef _DECODE_H
#define _DECODE_H

int ENCRYPT_GetErrorCode();
CFileBuffer* ENCRYPT_DecodeToBuffer( const char *szInFileName );
int ENCRYPT_IsFileEncrypted( const char *szInFileName );

#endif _DECODE_H