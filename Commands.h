#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define UNLISTED 2
#define STOPPED 1
#define UNFINISHED 0
#define NO_FG_JOB -1
const std::string WHITESPACE = " \n\r\t\f\v";



class Command {
// TODO: Add your data members
 public:
	char* args[COMMAND_MAX_ARGS];
	int args_num = 0;
	const char * cmd_line;
	pid_t pid;

  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};
/*
class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};*/

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members 
public:
  ChangeDirCommand(const char* cmd_line);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class ChangePromptCommand : public BuiltInCommand {
// TODO: Add your data members 
public:
  ChangePromptCommand(const char* cmd_line);
  virtual ~ChangePromptCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
public:
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() {}
  void execute() override;
};
/*
class HeadCommand : public BuiltInCommand {
 public:
  HeadCommand(const char* cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};*/

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members 
public:
  QuitCommand(const char* cmd_line);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsList {
 public:
  
  class JobEntry {
   // TODO: Add your data members
   public:
	std::string cmd;
	pid_t pid;
	time_t add_time;
	time_t stop_time;
	int status;
	
    JobEntry(){
      add_time = 0;
      pid = 0;
      status = UNFINISHED;
    }
  };
  int fg_job_id;
  JobEntry fgJob;
  std::map<int, JobEntry> jobsMap;//A map to manage the jobs list
  //std::map<int, JobsList::JobEntry> unlistedMap;//A map for unlisted jobs which still run.
 // TODO: Add your data members  public:
  JobsList();
  ~JobsList();
  void addJob(Command& cmd, bool isStopped = false);//completed
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();//completed
  JobEntry * getJobById(int jobId);//we dont need it
  void removeJobById(int jobId);//we dont need it
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);//completed
  // TODO: Add extra methods or modify exisitng ones as needed
};

class SmallShell {
 private:
    // TODO: Add your data members
    SmallShell();
 public:
    std::string cmd_line;
    int smash_pid;//this is the smash pid
    JobsList jobs_list;
    std::string curr_cmd_prompt;
    char* lastPWD;
    
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
      static SmallShell instance; // Guaranteed to be destroyed.
      // Instantiated on first use.
      return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
