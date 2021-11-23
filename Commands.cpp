#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
	pid = getpid();
	jobslist = JobsList();
	curr_cmd_prompt = "smash";
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  
  if(string(cmd_line).find("|") != string::npos){
		return new PipeCommand(cmd_line);
  }
  else if(string(cmd_line).find(">") != string::npos){
	  return new RedirectionCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
    return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
    return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0) {
    return new JobsCommand(cmd_line);
  }
  else if (firstWord.compare("kill") == 0) {
    return new KillCommand(cmd_line);
  }
  else if (firstWord.compare("fg") == 0) {
    return new ForegroundCommand(cmd_line);
  }
  else if (firstWord.compare("bg") == 0) {
    return new BackgroundCommand(cmd_line);
  }
  else if (firstWord.compare("quit") == 0) {
    return new QuitCommand(cmd_line);
  }
  else {
    return new ExternalCommand(cmd_line);
  }
  
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {

  // TODO: Add your implementation here
  // for example:
	 Command* cmd = CreateCommand(cmd_line);
     cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void JobsList::addJob(Command* cmd, bool isStopped){
  removeFinishedJobs(); //First delete all finished jobs
  int job_id;
  SmallShell& smash = SmallShell::getInstance();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  bool is_empty = jobs_map.empty(); //check if the jobs list is empty
  if(is_empty) {job_id = 1;}
  else{job_id = jobs_map.rbegin() ->first +1;//returns the maximal job id since the map is sorted.
  //initialization
  jobs_map[job_id] = JobEntry();
  jobs_map[job_id].pid = cmd->pid;
  jobs_map[job_id].cmd = cmd->cmd_line;
  jobs_map[job_id].add_time = time(nullptr);

  if(isStopped){jobs_map[job_id].status = STOPPED;}
  else{jobs_map[job_id].status = UNFINISHED;}
  }
}

void JobsList::removeFinishedJobs(){
  SmallShell& smash = SmallShell::getInstance();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  //map<int, JobsList::JobEntry>::iterator = jobs_map.begin();
  auto it = jobs_map.begin();
  JobsList::JobEntry job_entry = it->second;
  while(it != jobs_map.end()){
    if(waitpid(job_entry.pid, nullptr, WNOHANG) == job_entry.pid){
      jobs_map.erase(it);
      it++;
    }
    else{it++;}
  }
}


void KillCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();
  
}
