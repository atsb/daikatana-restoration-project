/******
gutil.h
GameSpy C Engine SDK
  
Copyright 1999 GameSpy Industries, Inc

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757

******

 Please see the GameSpy C Engine SDK documentation for more 
 information

******/
typedef unsigned char uchar;


void swap_byte ( uchar *a, uchar *b );
uchar encode_ct ( uchar c );
void gs_encode ( uchar *ins, int size, uchar *result );
void gs_encrypt ( uchar *key, int key_len, uchar *buffer_ptr, int buffer_len );
