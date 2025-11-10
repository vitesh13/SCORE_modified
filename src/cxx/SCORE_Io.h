// SPDX-License-Identifier: GNU General Public License v3.0 or later

#ifndef __SCORE_IO_H_INCLUDED__
#define __SCORE_IO_H_INCLUDED__


//STL mainly I/O related includes
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <string.h>

typedef long long __int64;

#include <cassert>
//macros relevant to Volkers C IO code only
#define BUFSIZE 1024
#define _STR(x) _VAL(x)
#define _VAL(x) #x
#define ASSERT(cond) if(!(cond)) std::cerr << "failed assertion:" << __FILE__ << " line " << _STR(__LINE__) << ":" << #cond << "\n";
#define TOLFP(x) ((1.0)-(0.99999/((double) x))) //tolerance
//#define ERRTXT(text) (text" : file: "__FILE__" line:"_STR(__LINE__)"\n")
#define stringsNotEqual(a,b) strcmp(a,b)
#define stringsEqual(a,b) !stringsNotEqual(a,b)


#define DEBUG
/*#ifdef DEBUG
	#include <cassert>
	#define QUICKASSERT(condition) if(!(condition)) { std::cerr << "Assertion failed at " << __FILE__ << ":" << __LINE__ << "\n inside " << __FUNCTION__ << "\nCondition: " << condition << "\n"; abort(); }
#else
	#define QUICKASSERT(condition) ((void)0)
#endif*/

//should debug tests should be compiled in
//#define DEBUG

#ifndef DEBUG
		#define QUICKASSERT(cond)       ((void)0)
#else
		#define QUICKASSERT(cond)       ASSERT(cond)
#endif


//compile in the desired output
//#define REPORTSTYLE_DEVELOPER
//detailed function intermediate results
//#define REPORTSTYLE_USER
//only least physical relevant output
//#define REPORTSTYLE_CELLCYCLES


#define READ			0
#define WRITE			1
#define APPEND			2
#define FAILURE			0
#define SUCCESS			1


class dataLine;
class dataBlock;
typedef dataLine * dataLineP;
typedef dataBlock * dataBlockP;

using namespace std;

void reportError( char * message); //int rank = 0);
void exitus (const char * s);


typedef struct
{
	union
	{
		float f;
		long i;
		char *s;
	} d;
	char type;

} univData;
typedef univData * univDataP;


class dataLine
{
public:
	dataLine();
	dataLine( int size );
	~dataLine();

	univDataP dat;
	long dataCount;

	dataLineP next;
	dataLineP prev;
};


class dataBlock
{
public:
	dataBlock();
	~dataBlock();
	long columnCount;
	long lineCount;

	dataLineP first;
	dataLineP last;

	char head[BUFSIZE];
	char name[128];
};

class io
{

public:
	io( void  );
	~io();

	short open( short rw, const char * filename, FILE ** file );
	short open( const char * filename, ofstream * file );
	short open( const char * filename, ifstream file );
	short write( char * message, FILE * file );
	short write( string message, ofstream * file );

	dataBlockP readDataBlock( const char *name, const char *fileName );
	dataBlockP readDataBlock( const char *name, FILE * file );

	char *newString(const char *s);
	double getReal( dataLineP line, long column );
	double getReal2( dataLineP line, long column );
	long getInt( dataLineP line, long column );
	char * getString( dataLineP line, long column );

	double geTReal( const char *s, dataBlockP db );
	long geTInt( const char *s, dataBlockP db );
	char * geTString( const char *s, dataBlockP db );

};


#endif
