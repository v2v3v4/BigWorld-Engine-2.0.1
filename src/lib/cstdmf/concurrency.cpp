/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "concurrency.hpp"

DECLARE_DEBUG_COMPONENT2( "CStdMF", 0 )

SimpleMutex MatrixMutexHolder::mutex_[MUTEX_MATRIX_SIZE];
volatile unsigned long MatrixMutexHolder::threadId_[MUTEX_MATRIX_SIZE];

static void NoOp() { }
void (*pMainThreadIdleStartFunc)() = &NoOp;
void (*pMainThreadIdleEndFunc)() = &NoOp;

#ifdef BWCLIENT_AS_PYTHON_MODULE

const  int c_ltsDataSize        = 1024;
const  int c_ltsOffsetOffset    = c_ltsDataSize;
const  int c_ltsTotalBufferSize = c_ltsOffsetOffset + sizeof(int);
static int s_refCounter = 0;

typedef std::vector<void*> VoidPtrVector;
static VoidPtrVector s_tlsData;


int tlsGetIndex()
{
	static int tlsIndex = TlsAlloc();
	return tlsIndex;
}

int tlsGetOffSet(int step)
{
	char * data      = tlsGetData();
	int * dataOffSet = (int*)&data[c_ltsOffsetOffset];
	MF_ASSERT(*dataOffSet+step<=c_ltsOffsetOffset);

	int oldOffSet    = *dataOffSet;

	*dataOffSet += step;
	return oldOffSet;
}

char * tlsGetData()
{
	int index = tlsGetIndex();
	char * tlsData = (char *) TlsGetValue(index);
	if (tlsData == NULL)
	{
		tlsData = new char[c_ltsTotalBufferSize];
		s_tlsData.push_back(tlsData);

		memset(tlsData, c_initFalse, c_ltsDataSize);

		// reset offet (c_initFalse may not be 0)
		int * offset = (int*)&tlsData[c_ltsOffsetOffset];
		*offset = 0;

		TlsSetValue(tlsGetIndex(), tlsData); 
	}
	return tlsData;
}

void tlsIncRef()
{
	++s_refCounter;
}

void tlsDecRef()
{
	--s_refCounter;
	if (s_refCounter == 0)
	{
		TlsFree(tlsGetIndex());

		VoidPtrVector::const_iterator it  = s_tlsData.begin();
		VoidPtrVector::const_iterator end = s_tlsData.end();
		while (it != end)
		{
			delete [] (*it);
		}
		s_tlsData.clear();
	}
}

#endif // BWCLIENT_AS_PYTHON_MODULE

// concurrency.cpp
