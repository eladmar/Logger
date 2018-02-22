#include "Logger.h"
#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>		//assert
#include <stdexcept>	//std::runtime_error
#include <time.h>		//time

#ifdef WIN32
#include <direct.h>		//mkdir
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>	//mkdir
#define MKDIR(dir) mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO )
#endif


using namespace std;


LOG_EXPORT Logger& Logger::GetInstance()
{
	static Logger x;
	return x;
}


LOG_EXPORT void Logger::Init(const LogConfig& config)
{
	WriterGuard g(dbLocker);
	
	if(	config.begin() != config.end())
	{
		db.insert(config.begin(),config.end());
		MKDIR(LOG_DIR);
		CreateFiles();
	}
}


Logger::Logger() :queue(),
				  runFlag(true),
				  executor(&Logger::WriteToFiles,this) //starts the thread that write the logs to files
{}


void Logger::WriteToFiles()
{
	while(runFlag)
	{
		Package pack;
		queue.Pop(pack);
		for( auto file = pack.files.begin() ; file != pack.files.end() ; ++file)
		{
			(**file) << pack.log.c_str();
			(**file).flush();
		}
	}
}


void Logger::CreateFiles()
{
	auto iter = db.begin();

	for(;iter != db.end(); ++iter)
	{
		auto& fileALevelSet = iter->second;
		auto setIter = fileALevelSet.begin();

		for(;setIter != fileALevelSet.end(); ++setIter)
		{
			const string& fileName = setIter->first;
			auto filesIter = files.find(fileName);

			if(filesIter == files.end())
			{
				string pathAndName(LOG_DIR);
				pathAndName+= '/';
                pathAndName+= fileName;
				files[fileName]= shared_ptr<ofstream>(new std::ofstream(pathAndName ,ios::app|ios_base::out));	
			}
		}
	}
}


#ifdef WIN32
#pragma warning(disable : 4996) //localtime() invoke warning in windows
#endif
//get time if string format
std::string Logger::Time()const
{
	char timeStr[64];
	time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime (&rawtime);
    sprintf(timeStr, "%d/%d/%d %d:%d:%d",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return timeStr;
}


LOG_EXPORT const std::string& Logger::ToStrLevel(Level level)
{
	static const string LOG_INFORMATION("LOG_INFORMATION");
	static const string LOG_DEBUG("LOG_DEBUG");
	static const string LOG_ERROR("LOG_ERROR");
	static const string LOG_FATAL("LOG_FATAL");
	switch(Level)
	{
		case Level::LOG_INFORMATION:
			return LOG_INFORMATION;
		case Level::LOG_DEBUG:
			return LOG_DEBUG;
		case Level::LOG_ERROR:
			return LOG_ERROR;
		case Level::LOG_FATAL:
			return LOG_FATAL;
		default:
			throw runtime_error("no suitable match ");
			return LOG_FATAL;
	}
}


LOG_EXPORT Level Logger::ToLevel(const std::string& levelStr)
{
	if( levelStr == "LOG_INFORMATION")
	{
		return Level::LOG_INFORMATION;
	}
	if( levelStr == "LOG_DEBUG")
	{
		return Level::LOG_DEBUG;
	}
	else if(levelStr == "LOG_ERROR")
	{
		return Level::LOG_ERROR;
	}
	else if(levelStr == "LOG_FATAL")
	{
		return Level::LOG_FATAL;
	}
	else
	{
		throw runtime_error("no suitable match ");
		return Level::LOG_FATAL;
	}
}


LOG_EXPORT Logger::~Logger()
{
    //tell the thread to stop exequte writes
    runFlag = false;

    //push empty package so if the exequter thread is wait on Pop he will release and exit the loop..
    queue.Push(Package());
    executor.join();
}


void Logger::SetLogStr(string& log,const char* function,const char* file, Level level, const std::string& module, const std::string& commment)
{
	try
	{
		log =  Time() + " ; " + module + " ; " + function +   " ; " + ToStrLevel(level) + " ; " + commment + "\n";
	}
	catch(std::runtime_error& err)
	{
		log =  Time() + " ; " + module + " ; " + function +   " ;Logger warning: Invalid Level ; " + commment + "\n";
	}
		
}


//insert to currentFiles attribute all the files that meets the requirements of module and minimun level
void Logger::SetNeededFiles(std::vector<std::shared_ptr<std::ofstream>>& currentFiles, Level level, const std::string& module)
{
	auto iter = db.begin();
	for(; iter != db.end(); ++iter)
	{
		if(iter->first == module )
		{
			set<FileAndLevel>& fileAndLevel = iter->second;
			auto fAlIter =fileAndLevel.begin();
			for(;fAlIter!= fileAndLevel.end(); ++fAlIter)
			{
				if(fAlIter->second <= level)
				{
					auto file = files.find(fAlIter->first);
					assert(file !=files.end());
					currentFiles.push_back(file->second);
                }
			}
			break;
		}
	}

}

LOG_EXPORT void Logger::InnerReport(const std::string& module, const std::string& commment)
{
	ReaderGuard g(dbLocker);
	Package pack;
	pack.log =  Time()  + ": " + commment + "\n";
	SetNeededFiles(pack.files, Level::LOG_INFORMATION, module);
    //send the packege to the writer thread
	queue.Push(pack);
}

LOG_EXPORT void  Logger::Write(const char* function,const char* file, Level level, const std::string& module, const std::string& commment)
{
	ReaderGuard g(dbLocker);
	Package pack;

    //set the packege
	SetLogStr(pack.log, function, file, level, module, commment);
	SetNeededFiles(pack.files, level, module);

    //send the packege to the writer thread
	queue.Push(pack);
}
