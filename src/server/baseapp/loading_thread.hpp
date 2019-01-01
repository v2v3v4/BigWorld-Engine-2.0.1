/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASEAPP_LOADING_THREAD_HPP
#define BASEAPP_LOADING_THREAD_HPP

#include "worker_thread.hpp"

#include <deque>
#include <list>
#include <string>

/**
 *	This abstract class extends WorkerJob to add jobs that will be ticked
 *	periodically.
 */
class TickedWorkerJob : public WorkerJob
{
public:
	static void tickJobs();

protected:
	TickedWorkerJob();
	virtual ~TickedWorkerJob() {}

	virtual bool tick() = 0;

private:
	typedef std::list< TickedWorkerJob * > Jobs;
	static Jobs jobs_;
};


class BinaryOStream;

/**
 *  A class used for gradually reading a file from disk into a fixed-sized
 *  memory buffer.  This is used to implement BigWorld.addProxyFileData().
 */
class FileStreamingJob : public WorkerJob
{
public:
	FileStreamingJob( const std::string &path, int bufsize = 65536 );
	~FileStreamingJob();

	int size();
	int freeSpace();
	bool done();
	bool good();
	void read( BinaryOStream &os, int nBytes );
	void write( const char *src, int nBytes );

private:
	virtual float operator()();

protected:
	std::string path_;
	FILE *file_;
	char *buf_;

	// Chunks of the file that have been read from disk but haven't been
	// completely written out to packets.
	typedef std::deque< std::string* > Data;
	Data data_;

	// How much of the first string in data_ has already been read
	int offset_;

	// The number of bytes we actually have buffered at the moment
	int size_;

	// The number of bytes we would like to have buffered at any time
	int maxsize_;

	// Has all the data been read off disk?
	bool doneReading_;

	// Was there some kind of IO error?
	bool error_;

	SimpleMutex lock_;
};

#endif // BASEAPP_LOADING_THREAD_HPP
