#include "util.h"
#include <KrisLibrary/utils/stringutils.h>
#include <limits.h>
#include <unistd.h>

namespace util{
  void SetSimulatedRobot( Robot *robot, WorldSimulation &sim, const Config &q)
  {
    Config dq(q); dq.setZero();
    SetSimulatedRobot( robot, sim, q, dq);
  }
  void SetSimulatedRobot( Robot *robot, WorldSimulation &sim, const Config &q, const Config &dq)
  {
    robot->UpdateConfig(q);
    int robotidx = 0;
    ODERobot *simrobot = sim.odesim.robot(robotidx);
    simrobot->SetConfig(q);
    simrobot->SetVelocities(dq);
  }


// struct tm;
//Member  Type  Meaning Range
//tm_sec  int seconds after the minute  0-61*
//tm_min  int minutes after the hour  0-59
//tm_hour int hours since midnight  0-23
//tm_mday int day of the month  1-31
//tm_mon  int months since January  0-11
//tm_year int years since 1900  
//tm_wday int days since Sunday 0-6
//tm_yday int days since January 1  0-365
//tm_isdst  int Daylight Saving Time flag

  std::string GetCurrentTimeString(const char* delimiter)
  {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    ostringstream os;

    int hour = now->tm_hour;
    int min = now->tm_min;
    int sec = now->tm_sec;

    os << setfill('0') << setw(2) << hour << delimiter << setfill('0') << setw(2) << min << delimiter << setfill('0') << setw(2) << sec;
    return os.str();
  }

  std::string GetCurrentDateString()
  {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    ostringstream os;

    int yyyy = now->tm_year + 1900;
    int mm = now->tm_mon + 1;
    int dd = now->tm_mday;

    os << yyyy << "_" << setfill('0') << setw(2) << mm << "_" << setfill('0') << setw(2) << dd;
    return os.str();
  }

  std::string GetCurrentDateTimeString()
  {
    return GetCurrentDateString()+"_"+GetCurrentTimeString(":");
  }


  void PrintCurrentTime()
  {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    printf ( "The current date/time is: %s\n", asctime (now) );

  }
  std::string GetApplicationFolder()
  {
    return GetExecFilePath()+"/../";
  }

  std::string GetDataFolder()
  {
    return GetExecFilePath()+"/../data";
  }
//https://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from
  std::string GetExecPath()
  {
    char result[ PATH_MAX ];
    ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    return std::string( result, (count > 0) ? count : 0 );
  }
  std::string GetExecFilePath()
  {
    boost::filesystem::path p(GetExecPath());
    return p.parent_path().string();
  }
  std::string GetFileBasename(const char *file)
  {
    boost::filesystem::path p(file);
    return p.stem().string();
  }
  std::string GetFileBasename(const std::string& file)
  {
    return GetFileBasename(file.c_str());
  }
  std::string GetFileExtension(const char *file)
  {
    boost::filesystem::path p(file);
    return p.extension().string();
  }
  std::string GetFileExtension(const std::string& file)
  {
    return GetFileExtension(file.c_str());
  }

  bool StartsWith(const std::string &s, const char* prefix){
    return ::StartsWith(s.c_str(),prefix);
  }
  bool StartsWith(const std::string &s, const std::string &prefix){
    return ::StartsWith(s.c_str(),prefix.c_str());
  }
  bool EndsWith(const std::string &s, const char* suffix){
    return ::EndsWith(s.c_str(),suffix);
  }
  bool EndsWith(const std::string &s, const std::string &suffix){
    return ::EndsWith(s.c_str(),suffix.c_str());
  }

  std::string RemoveStringBeginning(const std::string &s, const std::string &prefix){
    std::string out = s.substr(prefix.size()+1,s.size()-(prefix.size()+1));
    return out;
  }
}
