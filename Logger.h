#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>
#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <thread>
#include <memory> //unique_ptr

#include "ThreadSafeQueue.h"
#include "CrossPlatformReadWriteLocker.h"

#define LOG_DIR "Logs"

#ifdef WIN32
    #define LOG_EXPORT __declspec(dllexport)
#else
    #define LOG_EXPORT
#endif


//use exmple: log(Level::LOG_ERROR,"My Module","my log !");
//use exmple: log(Level::LOG_INFORMATION,"His Module","his comment");
#define Log(...) Logger::GetInstance().Write(__FUNCTION__,__FILE__,__VA_ARGS__)

//use exmple: Report("My Module","my report !");
//use exmple: Report("His Module","his comment");
#define Report(...) Logger::GetInstance().InnerReport(__VA_ARGS__)




enum class Level {
	LOG_INFORMATION,
	LOG_DEBUG,
	LOG_ERROR,
	LOG_FATAL
};

class Logger
{
public:
	typedef std::pair<std::string,Level>  FileAndLevel;
	 //key: module name ,val: set of files and the minimum Level
	typedef std::map<std::string,std::set<FileAndLevel>> LogConfig;

	//get the singelton Logger instance
	static LOG_EXPORT  Logger& GetInstance();

	//add the logger configuration of files and moduels to be added to the logger DB
	LOG_EXPORT void Init(const LogConfig& config);


	//utility function to convert from std::string to Level
	static LOG_EXPORT  Level ToLevel(const std::string& levelStr);

	//utility function to convert from Level to std::string
	static LOG_EXPORT  const std::string& ToStrLevel(Level level);

	//write log function,should not be use directly instead use the macro "Log(...)"
	LOG_EXPORT void Write(const char* function,const char* file, Level level, const std::string& module, const std::string& commment);


	//write log without function, file,  and level to the given module ,should not be use directly instead use the macro "Report(...)"
	LOG_EXPORT void InnerReport(const std::string& module, const std::string& commment);

	LOG_EXPORT ~Logger();

private:

	//insert to log all the string format to be write in the logs files
	void SetLogStr(std::string& log,const char* function,const char* file, Level level, const std::string& module, const std::string& commment);

	//insert to the vector files all the files handlers in accordance to the level and the module
	void SetNeededFiles(std::vector<std::shared_ptr<std::ofstream>>& currentFiles, Level level, const std::string& module);

	std::string Time()const;
	void CreateFiles();
	Logger();


	Logger(const Logger&);  //deleted
	void operator=(const Logger&);//deleted

	//DB of module and his associate Files name and the min level for wirting
	std::map<std::string,std::set<FileAndLevel>> db;

	//DB that handle the files handlers
	std::map<std::string,std::shared_ptr<std::ofstream>> files;

	//to manage exlusive write to the db and inclusive reads from it
	ReadWriteLocker dbLocker;
	
	typedef struct {
		std::string log;
		std::vector<std::shared_ptr<std::ofstream>> files;
	}Package;
	//queue to deliver masseges to the thread that in charge of writing the logs to the files
	ThreadSafeQueue<Package> queue;
	//indicator to the executor
    bool runFlag;

	//Thread that in charge of writing the logs to the files
	std::thread executor;

	//Get the log and write it to the needed files
	void WriteToFiles();

	


	
};



#endif //__LOGGER_H__
