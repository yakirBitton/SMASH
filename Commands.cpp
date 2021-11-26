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

bool isNumber(const string& str){
  for(char const &ch : str){
    if (std::isdigit(ch) == false) return false;
    return true;
  }
}

void KillCommand::execute(){
  int sig_num, job_id;
  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();
  //check if the syntax is good
  if(args_num != 3 || !isNumber(args[1]) || !isNumber(args[2]) || args[1][0] != '-'){
    cerr << "smash error: kill: invalid arguments" << endl;
    return;
  }
  sig_num = stoi(string(args[1]).substr(1));
  job_id = stoi(args[2]);

  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  map<int, JobsList::JobEntry>::iterator it;
  it = jobs_map.find(job_id);
  if(it == jobs_map.end()){
      cerr<<"smash error: kill: job_id " + to_string(job_id) + " does not exist"<<endl;
      return;
  }

  if(kill(job_id, sig_num) == -1){
    perror("smash error: kill failed");
    return;
  }

  int pid = it->second.pid;

  cout<<"signal number "<<sig_num<<" was sent to pid "<<pid<<endl;
  smash.jobs_list.removeFinishedJobs();
}

void ForegroundCommand::execute(){
  if(args_num >= 3 || !isNumber(args[1])){
    cerr <<"smash error: fg: invalid arguments"<<endl;
  }

  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();

  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  int job_id;
  JobsList::JobEntry job;

  if(args_num == 1){//no specific job_id was specified.
   if(smash.jobs_list.jobsMap.empty()){
     cerr<<"smash error: fg: jobslist is empty"<<endl;
     return;
   }

   job = smash.jobs_list.jobsMap.rbegin()->second;
   job_id = smash.jobs_list.jobsMap.rbegin()->first;
  }

  else{
    job_id = stoi(args[1]);
    std::map<int, JobsList::JobEntry>::iterator it;
    it = jobs_map.find(job_id);
    if(it == jobs_map.end()){
      cerr<<"smash error: fg: job-id " << job_id << "does not exist"<<endl;
      return;
    }

    job = it->second;
  }
    cout << job.cmd <<" : "<< job.pid <<endl;
    if(kill(job.pid, SIGCONT) == -1){
      perror("smash error: kill failed");
      return;
    }
    smash.jobs_list.fgJob = job;
    smash.jobs_list.fg_job_id = job.pid;
    //now the job has moved to fg so it moves to unlistedMap
    /*map<int, JobsList::JobEntry>& unlisted_map = smash.jobs_list.unlistedMap;
    std::map<int, JobsList::JobEntry>::iterator unlisted_it;
    unlisted_it = unlisted_map.begin();
    unlisted_map.insert(unlisted_it, std::pair<int, JobsList::JobEntry>(job_id,job));
    *///remove th fg job from the list.

    smash.jobs_list.jobsMap.erase(job_id);
    int wstatus = 0;
    if(waitpid(job.pid, &wstatus, WUNTRACED) != job.pid){
      perror("smash error: waitpid failed");
    }
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int* jobId){
  SmallShell& smash = SmallShell::getInstance();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;

  //map<int, JobsList::JobEntry>::iterator it;
  for(auto it = jobs_map.rbegin();it != jobs_map.rend(); it++){
    if(it->second.status == STOPPED){
      *jobId = it->first;
      return &it->second;
    }
  }
  return nullptr;
}

void BackgroundCommand::execute(){
  int target_job_id;
  JobsList::JobEntry target_job;
  

  if(args_num >= 3){
    cerr<<"smash error:bg: invalid arguments";
    return;
  }

  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;



  if(args_num == 2){
    if(!isNumber(args[1])){
      cerr<<"smash error:bg: invalid arguments";
      return;
      }

      target_job_id = stoi(args[1]);
      std::map<int, JobsList::JobEntry>::iterator it;
      it = jobs_map.find(target_job_id);
      if(it == jobs_map.end()){
      cerr<<"smash error: bg: job-id " << target_job_id << "does not exist"<<endl;
      return;
      }
      target_job = it->second;
  }
  else{
    target_job = *smash.jobs_list.getLastStoppedJob(&target_job_id);
    if(&target_job == nullptr){
      cerr<<"smash error: bg: there are no stopped jobs to resume"<<endl;
      return;
    }
  }
      
  if(target_job.status == UNFINISHED){
      cerr<<"smash error: bg: job-id" << target_job_id <<" is already running in the background"<<endl;
    }
  
  cout<<target_job.cmd<<" : "<<target_job.pid<<endl;
  target_job.status = UNFINISHED;

  if(kill(target_job.pid, SIGCONT) == -1){
      perror("smash error: kill failed");
      return;
    }
}

