/**************************************************************
    LZARI.C -- A Data Compression Program
    (tab = 4 spaces)
***************************************************************
    4/7/1989 Haruhiko Okumura
    Use, distribute, and modify this program freely.
    Please send me your improved versions.
        PC-VAN		SCIENCE
        NIFTY-Serve	PAF01022
        CompuServe	74050,1022
**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memmgrcommon.h"
#include "filebuffer.h"
#include "md5.h"
#include "dk_io.h"
#include "dk_defines.h"
#include "decode.h"

// ----------------------------------------------------------------------------

const unsigned int FILE_SIGNATURE = 0x00000000L;
const unsigned short FILE_VERSION = 0;

typedef struct _compressionHeader
{
	unsigned int nFileSignature;
	unsigned short nVersion;
	short bCompressed;				// is the file compressed?
	unsigned int nOriginalFileSize;	// length of the original file in bytes
	char szOriginalFileName[16];
	unsigned char md5Checksum[16];	// checksum of the original file
} COMPRESSION_HEADER;

typedef struct _encryptionHeader
{
	unsigned int nEncryptionSignature;
	short nVersion;
	short bEncrypted;				// is the file encrypted?
	short nPhase;					// (offset) between 0 - 360 degree
	short nAmplitude;				// 1 - 100
	short nFrequency;				// y = phase + (amplitude * sin(frequency * x))
	unsigned char md5checksum[16];	// checksum of the original file
} ENCRYPTION_HEADER;

// ----------------------------------------------------------------------------

static int nErrorCode = DKIO_ERROR_NONE;

#define N		 4096	// size of ring buffer
#define F		   60	// upper limit for match_length
#define THRESHOLD	2   // encode string into position and length
						//   if match_length is greater than this
#define NIL			N	// index for root of binary search trees

unsigned char  text_buf[N + F - 1]; // ring buffer of size N,

/********** Bit I/O **********/

FILE  *inFile, *outFile;
static unsigned int  buffer, mask = 0;

static int GetBit(void)  /* Get one bit (0 or 1) */
{
    if ( (mask >>= 1) == 0 )
    {
        buffer = DKIO_FGetc(inFile);  
		mask = 128;
    }
    return((buffer & mask) != 0);
}

/********** Arithmetic Compression **********/

/*  If you are not familiar with arithmetic compression, you should read
        I. E. Witten, R. M. Neal, and J. G. Cleary,
            Communications of the ACM, Vol. 30, pp. 520-540 (1987),
    from which much have been borrowed.  */

#define M   15

/*	Q1 (= 2 to the M) must be sufficiently large, but not so
    large as the unsigned long 4 * Q1 * (Q1 - 1) overflows.  */

#define Q1  (1UL << M)
#define Q2  (2 * Q1)
#define Q3  (3 * Q1)
#define Q4  (4 * Q1)
#define MAX_CUM (Q1 - 1)

#define N_CHAR  (256 - THRESHOLD + F)
/* character code = 0, 1, ..., N_CHAR - 1 */

unsigned long int  low = 0, high = Q4, value = 0;
int  shifts = 0;  /* counts for magnifying low and high around Q2 */
int  char_to_sym[N_CHAR], sym_to_char[N_CHAR + 1];
unsigned int
sym_freq[N_CHAR + 1],  /* frequency for symbols */
sym_cum[N_CHAR + 1],   /* cumulative freq for symbols */
position_cum[N + 1];   /* cumulative freq for positions */

static void StartModel(void)  /* Initialize model */
{
    int ch, sym, i;

    sym_cum[N_CHAR] = 0;
    for ( sym = N_CHAR; sym >= 1; sym-- )
    {
        ch = sym - 1;
        char_to_sym[ch] = sym;  
		sym_to_char[sym] = ch;
        sym_freq[sym] = 1;
        sym_cum[sym - 1] = sym_cum[sym] + sym_freq[sym];
    }
    sym_freq[0] = 0;  /* sentinel (!= sym_freq[1]) */
    position_cum[N] = 0;
    for ( i = N; i >= 1; i-- )
    {
	    position_cum[i - 1] = position_cum[i] + 10000 / (i + 200);
    }
	/* empirical distribution function (quite tentative) */
    /* Please devise a better mechanism! */
}

static void UpdateModel(int sym)
{
    int i, c, ch_i, ch_sym;

    if ( sym_cum[0] >= MAX_CUM )
    {
        c = 0;
        for ( i = N_CHAR; i > 0; i-- )
        {
            sym_cum[i] = c;
            c += (sym_freq[i] = (sym_freq[i] + 1) >> 1);
        }
        sym_cum[0] = c;
    }
    for ( i = sym; sym_freq[i] == sym_freq[i - 1]; i-- ) 
		;
    if ( i < sym )
    {
        ch_i = sym_to_char[i];    
		ch_sym = sym_to_char[sym];
        sym_to_char[i] = ch_sym;  
		sym_to_char[sym] = ch_i;
        char_to_sym[ch_i] = sym;  
		char_to_sym[ch_sym] = i;
    }
    sym_freq[i]++;
    while ( --i >= 0 ) 
	{
		sym_cum[i]++;
	}
}

static int BinarySearchSym(unsigned int x)
/* 1      if x >= sym_cum[1],
   N_CHAR if sym_cum[N_CHAR] > x,
   i such that sym_cum[i - 1] > x >= sym_cum[i] otherwise */
{
    int i, j, k;

    i = 1;  j = N_CHAR;
    while ( i < j )
    {
        k = (i + j) / 2;
        if ( sym_cum[k] > x ) 
		{
			i = k + 1;
        }
		else 
		{
			j = k;
		}
    }
    return i;
}

static int BinarySearchPos(unsigned int x)
/* 0 if x >= position_cum[1],
   N - 1 if position_cum[N] > x,
   i such that position_cum[i] > x >= position_cum[i + 1] otherwise */
{
    int i, j, k;

    i = 1;  j = N;
    while ( i < j )
    {
        k = (i + j) / 2;
        if ( position_cum[k] > x ) 
		{
			i = k + 1;
        }
		else 
		{
			j = k;
		}
    }
    return i - 1;
}

static void StartDecode(void)
{
    int i;

    for ( i = 0; i < M + 2; i++ )
    {
	    value = 2 * value + GetBit();
	}
}

static int DecodeChar(void)
{
    int  sym, ch;
    unsigned long int  range;

    range = high - low;
    sym = BinarySearchSym((unsigned int)(((value - low + 1) * sym_cum[0] - 1) / range));
    high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
    low +=       (range * sym_cum[sym    ]) / sym_cum[0];
    for ( ; ; )
    {
        if ( low >= Q2 )
        {
            value -= Q2;  
			low -= Q2;  
			high -= Q2;
        }
        else 
		if ( low >= Q1 && high <= Q3 )
        {
            value -= Q1;  
			low -= Q1;  
			high -= Q1;
        }
        else 
		if ( high > Q2 ) 
		{
			break;
        }
		low += low;  
		high += high;
        value = 2 * value + GetBit();
    }
    ch = sym_to_char[sym];
    UpdateModel(sym);
    return ch;
}

static int DecodePosition(void)
{
    int position;
    unsigned long int  range;

    range = high - low;
    position = BinarySearchPos((unsigned int)(((value - low + 1) * position_cum[0] - 1) / range));
    high = low + (range * position_cum[position    ]) / position_cum[0];
    low +=       (range * position_cum[position + 1]) / position_cum[0];
    for ( ; ; )
    {
        if ( low >= Q2 )
        {
            value -= Q2;  
			low -= Q2;  
			high -= Q2;
        }
        else 
		if ( low >= Q1 && high <= Q3 )
        {
            value -= Q1;  
			low -= Q1;  
			high -= Q1;
        }
        else 
		if ( high > Q2 ) 
		{
			break;
        }
		low += low;  
		high += high;
        value = 2 * value + GetBit();
    }
    return position;
}

// ----------------------------------------------------------------------------

// make sure to initialize all local variables
static void InitDecode()
{
	mask = 0;

	low = 0;
	high = Q4;
	value = 0;
	shifts = 0;  /* counts for magnifying low and high around Q2 */

//	nErrorCode = ERROR_NONE; //SCG:2-26-99
	nErrorCode = DKIO_ERROR_NONE;
}


/********** Decode **********/

#ifdef _DEBUG
void TEST_WriteFile( CFileBuffer *pBuffer, const char *szFileName )
{
    int nNumChars = pBuffer->GetNumChars();
	unsigned char *pChars = (unsigned char*)pBuffer->GetBuffer();

	char filePath[64];
	strcpy( filePath, "d:\\test\\" );
	strcat( filePath, szFileName );

	FILE *out = fopen( filePath, "wb" );
	if( out == NULL )
	{
		return;
	}

	for ( int i = 0; i < nNumChars; i++ )
	{
		fputc( pChars[i], out );
	}

	fclose( out );
}
#endif _DEBUG

// ----------------------------------------------------------------------------

int ENCRYPT_GetErrorCode()
{
	return nErrorCode;
}

CFileBuffer* ENCRYPT_DecodeToBuffer( const char *szInFileName )
{
    int  i, j, k, r, c;
    unsigned long int count = 0;
	COMPRESSION_HEADER compressionHeader;


	// LAB - trivial encryption

	if (!strcmp(szInFileName + strlen(szInFileName) - 4, ".vsc")) {

		int  length;

		DKIO_FOpenFile(szInFileName, &inFile);
		if (!inFile) {
		    nErrorCode = DKIO_ERROR_EMPTY;
			return NULL;
		}

		length = DKIO_FFileLength(inFile);

		CFileBuffer *pBuffer = new CFileBuffer(length - 4);

		// skip the header

		DKIO_FGetc(inFile);
		DKIO_FGetc(inFile);
		DKIO_FGetc(inFile);
		DKIO_FGetc(inFile);

		for (i = 0; i < (length - 4); i++) {

			c = DKIO_FGetc(inFile);
			c ^= 0x96;
			c &= 0xff;
			pBuffer->PutC(c);		
		}
		DKIO_FCloseFile(inFile);

#ifdef _DEBUG
		TEST_WriteFile(pBuffer, szInFileName);
#endif _DEBUG

		return pBuffer;
	}


	InitDecode();

	DKIO_FOpenFile( szInFileName, &inFile );
	if ( !inFile )
	{
//		nErrorCode = ERROR_OPENINGFILE;			//SCG: 2-26-99
		nErrorCode = DKIO_ERROR_OPENINGFILE;
		return NULL;
	}

    DKIO_FRead( &compressionHeader, sizeof(compressionHeader), inFile );
    if ( compressionHeader.nOriginalFileSize == 0 )
    {
//	    nErrorCode = ERROR_EMPTY;			//SCG: 2-26-99
	    nErrorCode = DKIO_ERROR_EMPTY;
		return NULL;             //  nothing to decode, empty file
	}
	if ( compressionHeader.nFileSignature != FILE_SIGNATURE )
	{
//		nErrorCode = ERROR_CORRUPT;			//SCG: 2-26-99
		nErrorCode = DKIO_ERROR_CORRUPT;
		return NULL;
	}
	if ( !compressionHeader.bCompressed )
	{
//		nErrorCode = ERROR_CORRUPT;			//SCG: 2-26-99
		nErrorCode = DKIO_ERROR_CORRUPT;
		return NULL;
	}

	CFileBuffer *pBuffer = new CFileBuffer( compressionHeader.nOriginalFileSize );
	if ( !pBuffer )
	{
//		nErrorCode = ERROR_MEMFAILED;			//SCG: 2-26-99
		nErrorCode = DKIO_ERROR_MEMFAILED;
		return NULL;
	}

	StartDecode();  
	StartModel();
    for ( i = 0; i < N - F; i++ ) 
	{
		text_buf[i] = ' ';
    }
	r = N - F;
    for ( count = 0; count < compressionHeader.nOriginalFileSize; )
    {
        c = DecodeChar();
        if ( c < 256 )
        {
            pBuffer->PutC(c);  
			text_buf[r++] = c;
            r &= (N - 1);  
			count++;
        }
        else
        {
            i = (r - DecodePosition() - 1) & (N - 1);
            j = c - 255 + THRESHOLD;
            for ( k = 0; k < j; k++ )
            {
                c = text_buf[(i + k) & (N - 1)];
                pBuffer->PutC(c);  
				text_buf[r++] = c;
                r &= (N - 1);  
				count++;
            }
        }
    }
	DKIO_FCloseFile( inFile );
	
	unsigned char md5Checksum[16];
	MD5_ComputeChecksum( pBuffer, md5Checksum );
	for ( i = 0; i < 16; i++ )
	{
		if ( (unsigned char)compressionHeader.md5Checksum[i] != md5Checksum[i] )
		{
#ifdef _DEBUG
			TEST_WriteFile( pBuffer, compressionHeader.szOriginalFileName );
#endif _DEBUG
			delete pBuffer;
			
//			nErrorCode = ERROR_CORRUPT; //SCG:2-26-99
			nErrorCode = DKIO_ERROR_CORRUPT;
			return NULL;
		}
	}

	return pBuffer;
}


int ENCRYPT_IsFileEncrypted( const char *szInFileName )
{
	COMPRESSION_HEADER compressionHeader;
	
	
	if (!strcmp(szInFileName + strlen(szInFileName) - 4, ".vsc")) {

		char  idstring[8];

		DKIO_FOpenFile( szInFileName, &inFile );
		if (!inFile) {
			return FALSE;
		}

	    DKIO_FRead(idstring, 4, inFile);
		DKIO_FCloseFile(inFile);

		if ((idstring[0] == 'C') &&
		    (idstring[1] == 'V') &&
		    (idstring[2] == 'S') &&
		    (idstring[3] == 'C')) {

			return TRUE;
		}
		return FALSE;
		
	} else {

		DKIO_FOpenFile( szInFileName, &inFile );
		if ( !inFile )
		{
			return FALSE;
		}

	    DKIO_FRead( &compressionHeader, sizeof(compressionHeader), inFile );
		DKIO_FCloseFile( inFile );

	    if ( compressionHeader.nOriginalFileSize == 0 )
	    {
		    return FALSE;             /*  nothing to decode, empty file   */
		}
		if ( compressionHeader.nFileSignature != FILE_SIGNATURE )
		{
			return FALSE;
		}
		if ( !compressionHeader.bCompressed )
		{
			return FALSE;
		}
		return TRUE;
	}
}

// ----------------------------------------------------------------------------

#if 0

/*

void PutBit(int bit)  // Output one bit (bit = 0,1)
{
    static unsigned int  buffer = 0, mask = 128;

    if ( bit ) 
	{
		buffer |= mask;
	}
    if ( (mask >>= 1) == 0 )
    {
        if ( putc(buffer, outFile) == EOF ) 
		{

		}
        buffer = 0;  
		mask = 128;  
		codesize++;
    }
}

void FlushBitBuffer(void)  // Send remaining bits
{
    for ( int i = 0; i < 7; i++ ) 
	{
		PutBit(0);
	}
}

//********** LZSS with multiple binary trees **********

						        //    with extra F-1 bytes to facilitate string comparison
int match_position, match_length,  // of longest match.  These are
									// set by the InsertNode() procedure.
lson[N + 1], rson[N + 257], dad[N + 1];  // left & right children &
										 //	parents -- These constitute binary search trees.

void InitTree(void)  // Initialize trees
{
    int  i;

    // For i = 0 to N - 1, rson[i] and lson[i] will be the right and
    // left children of node i.  These nodes need not be initialized.
    // Also, dad[i] is the parent of node i.  These are initialized to
    // NIL (= N), which stands for 'not used.'
    // For i = 0 to 255, rson[N + i + 1] is the root of the tree
    // for strings that begin with character i.  These are initialized
    // to NIL.  Note there are 256 trees.

    for ( i = N + 1; i <= N + 256; i++ ) 
	{
		rson[i] = NIL;   // root
	}
    for ( i = 0; i < N; i++ ) 
	{
		dad[i] = NIL;   // node
	}
}

void InsertNode(int r)
// Inserts string of length F, text_buf[r..r+F-1], into one of the
// trees (text_buf[r]'th tree) and returns the longest-match position
// and length via the global variables match_position and match_length.
// If match_length = F, then removes the old node in favor of the new
// one, because the old one will be deleted sooner.
// Note r plays double role, as tree node and position in buffer.
{
    int  i, p, cmp, temp;
    unsigned char  *key;

    cmp = 1;  
	key = &text_buf[r];  
	p = N + 1 + key[0];
    rson[r] = lson[r] = NIL;  
	match_length = 0;
    for ( ; ; )
    {
        if ( cmp >= 0 )
        {
            if ( rson[p] != NIL ) 
			{
				p = rson[p];
			}
            else
            {
                rson[p] = r;  
				dad[r] = p;  
				return;
            }
        }
        else
        {
            if ( lson[p] != NIL ) 
			{
				p = lson[p];
			}
            else
            {
                lson[p] = r;  
				dad[r] = p;  
				return;
            }
        }
        for ( i = 1; i < F; i++ )
        {
		    if ( (cmp = key[i] - text_buf[p + i]) != 0 )  
				break;
        }
		if ( i > THRESHOLD )
        {
            if ( i > match_length )
            {
                match_position = (r - p) & (N - 1);
                if ( (match_length = i) >= F ) 
					break;
            }
            else 
			if ( i == match_length )
            {
                if ( (temp = (r - p) & (N - 1)) < match_position )
                    match_position = temp;
            }
        }
    }
    dad[r] = dad[p];  
	lson[r] = lson[p];  
	rson[r] = rson[p];
    dad[lson[p]] = r;  
	dad[rson[p]] = r;
    if ( rson[dad[p]] == p ) 
	{
		rson[dad[p]] = r;
	}
    else
	{
		lson[dad[p]] = r;
    }
	dad[p] = NIL;  // remove p
}

void DeleteNode(int p)  // Delete node p from tree
{
    int  q;

    if ( dad[p] == NIL ) 
	{
		return;  // not in tree
    }
	if ( rson[p] == NIL ) 
	{
		q = lson[p];
	}
    else 
	if ( lson[p] == NIL ) 
	{
		q = rson[p];
    }
	else
    {
        q = lson[p];
        if ( rson[q] != NIL )
        {
            do
            {
                q = rson[q];
            } while ( rson[q] != NIL );
            rson[dad[q]] = lson[q];  
			dad[lson[q]] = dad[q];
            lson[q] = lson[p];  
			dad[lson[p]] = q;
        }
        rson[q] = rson[p];  
		dad[rson[p]] = q;
    }
    dad[q] = dad[p];
    if ( rson[dad[p]] == p ) 
	{
		rson[dad[p]] = q;
	}
    else
	{
		lson[dad[p]] = q;
    }
	dad[p] = NIL;
}

void EncodeChar(int ch)
{
    int  sym;
    unsigned long int  range;

    sym = char_to_sym[ch];
    range = high - low;
    high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
    low +=       (range * sym_cum[sym    ]) / sym_cum[0];
    for ( ; ; )
    {
        if ( high <= Q2 ) 
		{
			Output(0);
        }
		else 
		if ( low >= Q2 )
        {
            Output(1);  
			low -= Q2;  
			high -= Q2;
        }
        else 
		if ( low >= Q1 && high <= Q3 )
        {
            shifts++;  
			low -= Q1;  
			high -= Q1;
        }
        else 
		{
			break;
        }
		low += low;  
		high += high;
    }
    UpdateModel(sym);
}

void EncodePosition(int position)
{
    unsigned long int  range;

    range = high - low;
    high = low + (range * position_cum[position    ]) / position_cum[0];
    low +=       (range * position_cum[position + 1]) / position_cum[0];
    for ( ; ; )
    {
        if ( high <= Q2 ) 
		{
			Output(0);
        }
		else 
		if ( low >= Q2 )
        {
            Output(1);  
			low -= Q2;  
			high -= Q2;
        }
        else 
		if ( low >= Q1 && high <= Q3 )
        {
            shifts++;  
			low -= Q1;  
			high -= Q1;
        }
        else 
		{
			break;
        }
		low += low;  
		high += high;
    }
}

void EncodeEnd(void)
{
    shifts++;
    if ( low < Q1 ) 
	{
		Output(0);
    }
	else 
	{
		Output(1);
    }
	FlushBitBuffer();  // flush bits remaining in buffer
}

static void Output(int bit)  // Output 1 bit, followed by its complements
{
    PutBit(bit);
    for ( ; shifts > 0; shifts-- ) 
	{
		PutBit( !bit );
	}
}

int GetFileLength( FILE *f )
{
    rewind( f );
	int nStartFile = ftell( f );
    fseek( f, 0L, SEEK_END );
    int nFileLength = ftell( f ) - nStartFile;
    
	rewind( f );

	return nFileLength;
}

int Encode(const char *szInFileName, const char *szOutFileName )
{
    int  i, c, len, r, s, last_match_length;
	unsigned long int printcount = 0;

	COMPRESSION_HEADER compressionHeader;

	inFile = fopen( szInFileName, "rb" );
	if ( !inFile )
	{
		return FALSE;
	}

	outFile = fopen( szOutFileName, "wb" );
	if ( !outFile )
	{
		return FALSE;
	}

    compressionHeader.nFileSignature = FILE_SIGNATURE;
	compressionHeader.nVersion = FILE_VERSION;
	compressionHeader.bCompressed = 1;
	compressionHeader.nOriginalFileSize = GetFileLength( inFile );
	strcpy( compressionHeader.szOriginalFileName, szInFileName );
	MD5_ComputeChecksum( inFile, compressionHeader.md5Checksum );

    if ( compressionHeader.nOriginalFileSize == 0 )
    {
	    return FALSE;
	}
    if ( fwrite( &compressionHeader, sizeof(compressionHeader), 1, outFile ) < 1 )
    {
		return FALSE;
	}

    codesize += sizeof(compressionHeader);

	rewind(inFile);  
	unsigned long int textsize = 0;
    StartModel();  
	InitTree();
    s = 0;  
	r = N - F;
    for ( i = s; i < r; i++ ) 
	{
		text_buf[i] = ' ';
    }
	for ( len = 0; len < F && (c = getc(inFile)) != EOF; len++ )
    {
	    text_buf[r + len] = c;
    }
	textsize = len;
    for ( i = 1; i <= F; i++ ) 
	{
		InsertNode(r - i);
    }
	
	InsertNode(r);
    do
    {
        if ( match_length > len ) 
		{
			match_length = len;
        }
		if ( match_length <= THRESHOLD )
        {
            match_length = 1;  
			EncodeChar(text_buf[r]);
        }
        else
        {
            EncodeChar(255 - THRESHOLD + match_length);
            EncodePosition(match_position - 1);
        }
        last_match_length = match_length;
        for ( i = 0; i < last_match_length && (c = getc(inFile)) != EOF; i++ )
        {
            DeleteNode(s);  
			text_buf[s] = c;
            if ( s < F - 1 ) 
			{
				text_buf[s + N] = c;
            }
			s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            InsertNode(r);
        }
        if ( (textsize += i) > printcount )
        {
            printf("%12ld\r", textsize);  printcount += 1024;
        }
        while ( i++ < last_match_length )
        {
            DeleteNode(s);
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            if ( --len ) 
			{
				InsertNode(r);
			}
        }
    } while ( len > 0 );

    EncodeEnd();

	fclose( inFile );
	fclose( outFile );

    printf("In : %lu bytes\n", textsize);
    printf("Out: %lu bytes\n", codesize);
    printf("Out/In: %.3f\n", (double)codesize / textsize);
}

int Decode( const char *szInFileName )
{
    int  i, j, k, r, c;
    unsigned long int count = 0;
	unsigned long int printcount = 0;
	COMPRESSION_HEADER compressionHeader;

	DKIO_FOpenFile( szInFileName, &inFile );
	if ( !inFile )
	{
		return FALSE;
	}

    if ( DKIO_FRead( &compressionHeader, sizeof(compressionHeader), inFile ) < 1 )
    {
		return FALSE;
	}

    if ( compressionHeader.nOriginalFileSize == 0 )
    {
	    return FALSE;
	}

	DKIO_FOpenFile( compressionHeader.szOriginalFileName, &outFile );
	if ( !outFile )
	{
		return FALSE;
	}
	
	StartDecode();  
	StartModel();
    for ( i = 0; i < N - F; i++ ) 
	{
		text_buf[i] = ' ';
    }
	r = N - F;
    for ( count = 0; count < compressionHeader.nOriginalFileSize; )
    {
        c = DecodeChar();
        if ( c < 256 )
        {
            DKIO_putc(c, outFile);  
			text_buf[r++] = c;
            r &= (N - 1);  
			count++;
        }
        else
        {
            i = (r - DecodePosition() - 1) & (N - 1);
            j = c - 255 + THRESHOLD;
            for ( k = 0; k < j; k++ )
            {
                c = text_buf[(i + k) & (N - 1)];
                putc(c, outFile);  
				text_buf[r++] = c;
                r &= (N - 1);  
				count++;
            }
        }
        if ( count > printcount )
        {
            //printf("%12lu\r", count);  
			printcount += 1024;
        }
    }
    //printf("%12lu\n", count);


	fseek( outFile, 0, SEEK_END );	// reset to end of file

	fclose( inFile );
	fclose( outFile );

	outFile = fopen( compressionHeader.szOriginalFileName, "rb" );

	unsigned char md5Checksum[16];
	MD5_ComputeChecksum( outFile, md5Checksum );
	for ( i = 0; i < 16; i++ )
	{
		if ( (unsigned char)compressionHeader.md5Checksum[i] != md5Checksum[i] )
		{
			//printf( "Bad checksum!\n" );
			break;
			return FALSE;
		}
	}

	fclose( outFile );

	return TRUE;
}

int Decode( const char *szInFileName, const char *szOutFileName )
{
    int  i, j, k, r, c;
    unsigned long int count = 0;
	unsigned long int printcount = 0;
	COMPRESSION_HEADER compressionHeader;

	inFile = fopen( szInFileName, "rb" );
	if ( !inFile )
	{
		return FALSE;
	}

    if ( fread( &compressionHeader, sizeof(compressionHeader), 1, inFile ) < 1 )
    {
		return FALSE;
	}

    if ( compressionHeader.nOriginalFileSize == 0 )
    {
	    return FALSE;				//  nothing to decode, empty file
	}

	if ( strlen( szOutFileName ) > 0 )
	{
		outFile = fopen( szOutFileName, "wb" );
	}
	else
	{
		outFile = fopen( compressionHeader.szOriginalFileName, "wb" );
	}
	if ( !outFile )
	{
		return FALSE;
	}
	
	StartDecode();  
	StartModel();
    for ( i = 0; i < N - F; i++ ) 
	{
		text_buf[i] = ' ';
    }
	r = N - F;
    for ( count = 0; count < compressionHeader.nOriginalFileSize; )
    {
        c = DecodeChar();
        if ( c < 256 )
        {
            putc(c, outFile);  
			text_buf[r++] = c;
            r &= (N - 1);  
			count++;
        }
        else
        {
            i = (r - DecodePosition() - 1) & (N - 1);
            j = c - 255 + THRESHOLD;
            for ( k = 0; k < j; k++ )
            {
                c = text_buf[(i + k) & (N - 1)];
                putc(c, outFile);  
				text_buf[r++] = c;
                r &= (N - 1);  
				count++;
            }
        }
        if ( count > printcount )
        {
            //printf("%12lu\r", count);  
			printcount += 1024;
        }
    }
    //printf("%12lu\n", count);


	fseek( outFile, 0, SEEK_END );	// reset to end of file

	fclose( inFile );
	fclose( outFile );

	outFile = fopen( compressionHeader.szOriginalFileName, "rb" );

	unsigned char md5Checksum[16];
	MD5_ComputeChecksum( outFile, md5Checksum );
	for ( i = 0; i < 16; i++ )
	{
		if ( (unsigned char)compressionHeader.md5Checksum[i] != md5Checksum[i] )
		{
			//printf( "Bad checksum!\n" );
			return FALSE;
		}
	}

	fclose( outFile );

	return TRUE;
}

*/

#endif 0