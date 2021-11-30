#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <time.h>
#include "Commands.h"

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  std::cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  std::cout << __PRETTY_FUNCTION__ << " <-- " << endl;
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

JobsList::JobsList(){}
JobsList::~JobsList(){}

SmallShell::SmallShell() {
// TODO: add your implementation
	smash_pid = getpid();
	jobs_list = JobsList();
	curr_cmd_prompt = "smash";
  lastPWD = (char*)malloc(COMMAND_ARGS_MAX_LENGTH);
}

SmallShell::~SmallShell() {
// TODO: add your implementation
  delete lastPWD;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  
  /*if(string(cmd_line).find("|") != string::npos){
		return new PipeCommand(cmd_line);
  }
  else if(string(cmd_line).find(">") != string::npos){
	  return new RedirectionCommand(cmd_line);
  }
  else*/ if (firstWord.compare("pwd") == 0) {
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
/*********************Command*****************************/
Command::Command(const char *cmd_line) : cmd_line(cmd_line) {
    char* arguments[COMMAND_ARGS_MAX_LENGTH];
    args_num = _parseCommandLine(cmd_line, arguments);
    for(int i = 0; i < args_num; i++){
        args[i] = arguments[i];
    }
}

Command::~Command(){
    for(int i = 0; i < args_num; i++){
        delete[] args[i];
    }
    SmallShell& smash = SmallShell::getInstance();
    smash.jobs_list.fg_job_id = NO_FG_JOB;
}
/*********************built-in command********************/

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line){}

/*********************change prompt command***************/

ChangePromptCommand::ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void ChangePromptCommand::execute(){
    SmallShell& smash = SmallShell::getInstance();
    if(this->args_num == 1){
      smash.curr_cmd_prompt = "smash";
    }
    else{
      smash.curr_cmd_prompt = args[1];
    }
}

/*********************show pid command********************/

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void ShowPidCommand::execute(){
    SmallShell& smash = SmallShell::getInstance();
    std::cout << "smash pid is " << smash.smash_pid << endl;
}

/*********************pwd command*************************/

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void GetCurrDirCommand::execute(){
  char* pwd = get_current_dir_name();
  if(pwd == nullptr){
    perror("smash error: get_current_dir_name() failed");
    return;
  }
  std::cout << pwd << endl;    
  delete pwd;
}


/*********************change dir command******************/

ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ChangeDirCommand::execute(){
    if(this->args_num > 2){
      cerr << "smash error: cd: too many arguments" << endl;
      return;
    }
    SmallShell& smash = SmallShell::getInstance();
    char* dir = args[1];
    char* pwd = get_current_dir_name();
    if(pwd == nullptr){
      perror("smash error: get_current_dir_name() failed");
      return;
    }
    if(strcmp(dir,"-") == 0){
        if(strcmp(smash.lastPWD,"")==0){
          cerr << "smash error: cd: OLDPWD not set" << endl;
          return;
        }
        dir = smash.lastPWD;
    }
    strcpy(smash.lastPWD,pwd);
    if(chdir(dir) == -1){
      perror("smash error: chdir() failed");
    }
    delete pwd;
}

/*********************jobs command************************/

JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void JobsCommand::execute(){
    SmallShell& smash = SmallShell::getInstance();
    map<int, JobsList::JobEntry>::iterator it;
    for(it = smash.jobs_list.jobsMap.begin(); it != smash.jobs_list.jobsMap.end(); it++){
      if((it->second.status != STOPPED) || (it->second.status != UNFINISHED)){
        continue;
      }
      std::cout << "[" << it->first << "] " << it->second.cmd << " : " << it->second.pid << " ";
      time_t diff_time = difftime(it->second.add_time, time(NULL));
      std::cout << diff_time << " secs";
      if(it->second.status == STOPPED){
        std::cout << " (stopped)";
      }
      std::cout << endl;
    }
}

void JobsList::addJob(Command& cmd, bool isStopped){
  removeFinishedJobs(); //First delete all finished jobs
  int job_id;
  SmallShell& smash = SmallShell::getInstance();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  bool is_empty = jobs_map.empty(); //check if the jobs list is empty
  if(is_empty) {job_id = 1;}
  else{job_id = jobs_map.rbegin() ->first +1;}//returns the maximal job id since the map is sorted.
  //initialization
  jobs_map[job_id] = JobEntry();
  jobs_map[job_id].pid = cmd.pid;
  jobs_map[job_id].cmd = cmd.cmd_line;
  jobs_map[job_id].add_time = time(nullptr);
  jobs_map[job_id].status = (isStopped ? STOPPED : UNFINISHED);
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
  return false;
}

KillCommand::KillCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

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

  std::cout << "signal number " << sig_num << " was sent to pid " << pid << endl;
  smash.jobs_list.removeFinishedJobs();
}

ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

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
    std::cout << job.cmd <<" : "<< job.pid <<endl;
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

BackgroundCommand::BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

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
    JobsList::JobEntry* target_job_ptr = smash.jobs_list.getLastStoppedJob(&target_job_id);
    if(target_job_ptr == nullptr){
      cerr<<"smash error: bg: there are no stopped jobs to resume"<<endl;
      return;
    }
    target_job = *target_job_ptr;
  }
      
  if(target_job.status == UNFINISHED){
      cerr<<"smash error: bg: job-id" << target_job_id <<" is already running in the background"<<endl;
    }
  
  std::cout<<target_job.cmd<<" : "<<target_job.pid<<endl;
  target_job.status = UNFINISHED;

  if(kill(target_job.pid, SIGCONT) == -1){
      perror("smash error: kill failed");
      return;
    }
}

QuitCommand::QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void QuitCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();

  for( int i=1; i<args_num;i++){
    string arg = string(args[i]);
    if(arg == "kill"){
      std::cout << "smash: sending SIGKILL signal to " << smash.jobs_list.jobsMap.size() << " jobs"<<endl;
      smash.jobs_list.killAllJobs();
      break;
    }
  }
  exit(0);
}

void JobsList::killAllJobs(){
  SmallShell& smash = SmallShell::getInstance();
  map<int, JobsList::JobEntry> jobs_map = smash.jobs_list.jobsMap;
  map<int, JobsList::JobEntry>::iterator it;
  JobsList::JobEntry job;
  for(it = jobs_map.begin(); it != jobs_map.end(); it++){
    job = it->second;
    std::cout<< job.pid << ": "<< job.cmd << endl;
  }
}

ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line){}

void ExternalCommand::execute() {
  SmallShell& smash = SmallShell::getInstance();
  pid_t pid = fork();
  if(pid == -1){
    perror("smash error: fork failed");
    return;
  }
  if(pid == 0){
    if(setpgrp() == -1){
      perror("smash error: setpgrp failed");
      return;
    }

    char target_cmd[COMMAND_ARGS_MAX_LENGTH];
    strcpy(target_cmd,cmd_line);

    if(_isBackgroundComamnd(cmd_line)){_removeBackgroundSign(target_cmd);}

    char  bash[]= "/bin/bash";
    char  flag[] = "-c";

    char* const bash_cmd[] = {bash, flag, (char*)target_cmd, nullptr};

    if(execv("/bin/bash", bash_cmd) == -1){
      perror("smash error: execv failed");
      return;
    }
  }

  else{
    if(_isBackgroundComamnd(cmd_line)){
      ExternalCommand ex_cmd = ExternalCommand(cmd_line);
      smash.jobs_list.addJob( ex_cmd , false);
    }

    else{
      //smash.jobs_list.fg_job_id = pid;
      //smash.jobs_list.fgJob = JobsList::JobEntry(cmd_line);
      if(waitpid(pid,NULL,WUNTRACED) == -1){
        perror("smash error: waitpid failed");
        return;
      }
    }
  }
}
