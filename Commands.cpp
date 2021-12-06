#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include "fcntl.h"
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

JobsList::JobEntry::JobEntry(){
  add_time = 0;
  pid = getpid();
  if(pid == -1){
    perror("smash error: getpid failed");
  }
  status = UNFINISHED;
}

SmallShell::SmallShell() {
// TODO: add your implementation
	smash_pid = getpid();
	jobs_list = JobsList();
  curr_cmd_prompt = "smash";
  lastPWD = (char*)malloc(COMMAND_ARGS_MAX_LENGTH);
  jobs_list.fgJob = jobs_list.fgJob.createDummy();
  jobs_list.fg_job_id = NO_FG_JOB;
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
  if(built_in_cmd.count(firstWord.substr(0,firstWord.length()-1))==1 && firstWord[firstWord.length()-1] == '&'){
    firstWord = firstWord.substr(0,firstWord.length()-1);
  }
  
  
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
  else if (firstWord.compare("head") == 0) {
    return new HeadCommand(cmd_line);
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
Command::Command(const char *cmd_line) : cmd_line(cmd_line){
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
    smash.jobs_list.fgJob = smash.jobs_list.fgJob.createDummy();
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
    perror("smash error: get_current_dir_name failed");
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
    if(this->args_num == 1){
      return;
    }
    SmallShell& smash = SmallShell::getInstance();
    char* dir = args[1];
    char* pwd = get_current_dir_name();
    if(pwd == nullptr){
      perror("smash error: get_current_dir_name failed");
      return;
    }
    if(strcmp(dir,"-") == 0){
      if(strcmp(smash.lastPWD,"")==0){
        cerr << "smash error: cd: OLDPWD not set" << endl;
        return;
      }
      if(chdir(smash.lastPWD) == -1){
        perror("smash error: chdir failed");
        return;
      }
    }
    else{
      if(chdir(dir) == -1){
        perror("smash error: chdir failed");
        return;
      }
    }
    strcpy(smash.lastPWD,pwd);
    delete pwd;
}

/*********************jobs command************************/

JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void JobsCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();
  map<int, JobsList::JobEntry>::iterator it;
  for(it = smash.jobs_list.jobsMap.begin(); it != smash.jobs_list.jobsMap.end(); it++){
    if((it->second.status == STOPPED) || (it->second.status == UNFINISHED)){
      std::cout << "[" << it->first << "] " << it->second.cmd << " : " << it->second.pid << " ";
      time_t diff_time = difftime(time(NULL), it->second.add_time);
      std::cout << diff_time << " secs";
      if(it->second.status == STOPPED){
        std::cout << " (stopped)";
      }
      std::cout << endl;
    }
  }
}

/*********************head command************************/

HeadCommand::HeadCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void HeadCommand::execute(){
    int rows_to_read;
    ifstream infile;
    if(args_num == 1){
      cerr << "smash error: head: not enough arguments" << endl;
      return;
    }
    string file_name;
    if(args_num == 2){
      rows_to_read = 10;
      file_name = string(args[1]);
    }else{
      if(args_num == 3){
        rows_to_read = stoi(string(args[1]).substr(1, strlen(args[1])));
        file_name = string(args[2]);
      }else{
        return;
      }
    }
    infile.open(file_name, ifstream::in);
    if(!infile.good()){
        perror("smash error: open failed");
        return;
    }
    string buffer;
    for(int i = 0; i < rows_to_read; i++){
      if(infile.eof()){
          i = rows_to_read;
          continue;
      }
      getline(infile, buffer);
      cout << buffer;
      if(!infile.eof()){
        cout << endl;
      }
    }
    infile.close();
}

/*********************redirection command*****************/

RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line) {}

void RedirectionCommand::execute(){
  bool append = true;  
  unsigned int pos = string(cmd_line).find(">");
  if(cmd_line[pos+1] != '>') append = false;
  string s = _trim(string(cmd_line).substr(0,pos));
  const char* new_cmd_line = s.c_str();
  SmallShell& smash = SmallShell::getInstance();
  string s2 = _trim(string(cmd_line).substr(pos + (append ? 2 : 1), strlen(cmd_line)-1));
  const char* file_name = s2.c_str();
  Command* command = smash.CreateCommand(new_cmd_line);
  int std_out = dup(1);
  int filed;
  if(append){
    filed = open(file_name, O_CREAT | O_WRONLY | O_APPEND, 0777);
  }else{
    filed = creat(file_name, 0777);
  }
  if(filed == -1){
    perror("smash error: open failed");
    return;
  }
  if(dup2(filed, 1) == -1){
    perror("smash error: dup2 failed");
    return;
  }
  if(close(filed) == -1){
    perror("smash error: close failed");
    return;
  }
  command->execute();
  if(dup2(std_out, 1) == -1){
    perror("smash error: dup2 failed");
    return;
  }
  if(close(std_out) == -1){
    perror("smash error: close failed");
    return;
  }
}

/*********************pipe command************************/

PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line) {}

void PipeCommand::execute(){
  bool err_flag = false;  
  unsigned int pos = string(cmd_line).find("|");
  if(cmd_line[pos+1] == '&') err_flag = true;
  string s = _trim(string(cmd_line).substr(0,pos));
  char first_cmd_line[COMMAND_ARGS_MAX_LENGTH];
  strcpy(first_cmd_line, s.c_str());
  SmallShell& smash = SmallShell::getInstance();
  int out_pipe = (err_flag ? 2 : 1);
  string s2 = _trim(string(cmd_line).substr(pos + out_pipe, strlen(cmd_line)));  
  char second_cmd_line[COMMAND_ARGS_MAX_LENGTH];
  strcpy(second_cmd_line, s2.c_str());
  Command* first_command = smash.CreateCommand(first_cmd_line);
  Command* second_command = smash.CreateCommand(second_cmd_line);
  smash.jobs_list.removeFinishedJobs();
  int fdt[2];
  if(pipe(fdt) == -1){
    perror("smash error: pipe failed");
    return;
  }
  int first_child_pid = fork();
  if(first_child_pid == -1){
    perror("smash error: fork failed");
    return;
  }
  if(first_child_pid == 0){
    if(setpgrp() == -1)
    {
        perror("smash error: setpgrp failed");
        return;
    }
    if(dup2(fdt[1],out_pipe) == -1){
      perror("smash error: dup2 failed");
      return;
    }
    if((close(fdt[0]) == -1) || (close(fdt[1])) == -1){
      perror("smash error: close failed");
      return;
    }
    
    if(smash.built_in_cmd.count(first_command->args[0]) == 1){
      first_command->execute();
      exit(0);
    }else{
      // char bash[] = "/bin/bash";
      // char flag[] = "-c";
      char* const bash_cmd[] = {(char*) "/bin/bash", (char *)"-c", (char*) first_cmd_line, nullptr};
      if(execv("/bin/bash", bash_cmd) == -1){
        perror("smash error: execv failed");
        return;
      }
    }
  }
  int second_child_pid = fork();
  if(second_child_pid == -1){
    perror("smash error: fork failed");
    return;
  }
  if(second_child_pid == 0){
    if(setpgrp() == -1)
    {
        perror("smash error: setpgrp failed");
        return;
    }
    if(dup2(fdt[0],0) == -1){
      perror("smash error: dup2 failed");
      return;
    }
    if((close(fdt[0]) == -1) || (close(fdt[1])) == -1){
      perror("smash error: close failed");
      return;
    }
    if(smash.built_in_cmd.count(second_command->args[0]) == 1){
      second_command->execute();
      exit(0);
    }else{
      // char bash[] = "/bin/bash";
      // char flag[] = "-c";
      char* const bash_cmd[] = {(char*) "/bin/bash", (char *)"-c", (char*)second_cmd_line, nullptr};
      if(execv("/bin/bash", bash_cmd) == -1){
        perror("smash error: execv failed");
        return;
      }
    }
  }
  if((close(fdt[0]) == -1) || (close(fdt[1])) == -1){
    perror("smash error: close failed");
    return;
  }
  if(waitpid(first_child_pid, nullptr, 0) == -1){
      perror("smash error: waitpid failed");
      exit(0);
  }
  if(waitpid(second_child_pid, nullptr, 0) == -1){
      perror("smash error: waitpid failed");
      exit(0);
  }   
}

/*********************external command************************/

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
    char r = '\r';
    char null_term = '\0';
    if(target_cmd[strlen(target_cmd) - 1] == r){
      target_cmd[strlen(target_cmd) - 1] = null_term;
    }

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
    ExternalCommand* ex_cmd = new ExternalCommand(cmd_line); 
    ex_cmd->pid = pid;
    if(_isBackgroundComamnd(cmd_line)){
      smash.jobs_list.addJob(ex_cmd ,false);
    }

    else{
      smash.jobs_list.fg_job_id = 0;
      smash.jobs_list.fgJob = JobsList::JobEntry();
      smash.jobs_list.fgJob.pid = pid;
      smash.jobs_list.fgJob.cmd = string(cmd_line);
      smash.jobs_list.fgJob.status = UNLISTED;
      if(waitpid(pid, NULL, WUNTRACED) == -1){
        perror("smash error: waitpid failed");
        return;
      }
      if(smash.jobs_list.fgJob.pid == pid){
        smash.jobs_list.fg_job_id = NO_FG_JOB;
        smash.jobs_list.fgJob = smash.jobs_list.fgJob.createDummy();
      }
    }
  }
}

JobsList::JobEntry JobsList::JobEntry::createDummy(){
  JobEntry job;
  job.add_time = -1;
  job.cmd = "";
  job.pid = -1;
  job.status = NO_FG_JOB;
  job.stop_time = -1;
  return job;
}

void JobsList::addJob(Command* cmd, bool isStopped){
  removeFinishedJobs(); //First delete all finished jobs
  int job_id;
  SmallShell& smash = SmallShell::getInstance();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  bool is_empty = jobs_map.empty(); //check if the jobs list is empty
  if(is_empty) {job_id = 1;}
  else{job_id = jobs_map.rbegin()->first +1;}//returns the maximal job id since the map is sorted.
  //initialization
  jobs_map[job_id] = JobEntry();
  jobs_map[job_id].pid = cmd->pid;
  jobs_map[job_id].cmd = cmd->cmd_line;
  jobs_map[job_id].add_time = time(nullptr);
  jobs_map[job_id].status = (isStopped ? STOPPED : UNFINISHED);
}

void JobsList::removeFinishedJobs(){
  SmallShell& smash = SmallShell::getInstance();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  //map<int, JobsList::JobEntry>::iterator = jobs_map.begin();
  map<int, JobsList::JobEntry>::iterator it = jobs_map.begin();
  JobsList::JobEntry job_entry = it->second;

  while(it != jobs_map.end()){
    if(waitpid(it->second.pid, nullptr, WNOHANG) == it->second.pid){
      it = jobs_map.erase(it);
      //it = jobs_map.begin();
    }
    else{++it;}
  }
}

bool isNumber(const string& str){
  size_t i = 0;
  if(str[0] == '-'){
    i++;
  }
  for(; i < str.length();i++){
    if (std::isdigit(str[i]) == false) return false;
    return true;
  }
  return false;
}

/*********************kill command************************/

KillCommand::KillCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void KillCommand::execute(){
  int sig_num, job_id;
  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();
  //check if the syntax is good
  if(args_num != 3 || !isNumber(string(args[1]).substr(1)) || !isNumber(args[2]) || args[1][0] != '-'){
      cerr << "smash error: kill: invalid arguments" << endl;
      return;
    }

  sig_num = stoi(string(args[1]).substr(1));
  job_id = stoi(args[2]);
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  map<int, JobsList::JobEntry>::iterator it;
  it = jobs_map.find(job_id);
  if(it == jobs_map.end()){
      cerr<<"smash error: kill: job-id " << job_id << " does not exist"<<endl;
      return;
  }
  int pid = it->second.pid;
  if(kill(pid, sig_num) == -1){
    perror("smash error: kill failed");
    return;
  }

  std::cout << "signal number " << sig_num << " was sent to pid " << pid << endl;
  smash.jobs_list.removeFinishedJobs();
}

/*********************foreground command******************/

ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ForegroundCommand::execute(){
  if(args_num >= 3 ){
    cerr <<"smash error: fg: invalid arguments"<<endl;
    return;
  }
  else if(args_num == 2){
    if(!isNumber(args[1])){
      cerr <<"smash error: fg: invalid arguments"<<endl;
      return;
    }
  }

  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();

  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  int job_id;
  JobsList::JobEntry job;

  if(args_num == 1){//no specific job_id was specified.
  
   if(smash.jobs_list.jobsMap.empty()){
     cerr<<"smash error: fg: jobs list is empty"<<endl;
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
      cerr<<"smash error: fg: job-id " << job_id << " does not exist"<<endl;
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
    smash.jobs_list.fg_job_id = job_id;
    smash.jobs_list.fgJob.status = UNLISTED;
   

    smash.jobs_list.jobsMap.erase(job_id);
    int wstatus = 0;
    if(waitpid(job.pid, &wstatus, WUNTRACED) != job.pid){
      perror("smash error: waitpid failed");
    }
}

/*JobsList::JobEntry* JobsList::getLastStoppedJob(int* jobId){
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
}*/

/*********************background command******************/

BackgroundCommand::BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void BackgroundCommand::execute(){
  int target_job_id;

  if(args_num >= 3){
    cerr<<"smash error: bg: invalid arguments" << endl;
    return;
  }

  SmallShell& smash = SmallShell::getInstance();
  smash.jobs_list.removeFinishedJobs();
  map<int, JobsList::JobEntry>& jobs_map = smash.jobs_list.jobsMap;
  std::map<int, JobsList::JobEntry>::reverse_iterator it;

  if(args_num == 2){
    if(!isNumber(args[1])){
      cerr<<"smash error: bg: invalid arguments" << endl;
      return;
      }

      target_job_id = stoi(args[1]);
      for(it = jobs_map.rbegin();it != jobs_map.rend();it++){
        if(it->first == target_job_id){
          break;
        }
      }
      if(it == jobs_map.rend()){
      cerr << "smash error: bg: job-id " << target_job_id << " does not exist" << endl;
      return;
      }
  }
  else{
    for(it = smash.jobs_list.jobsMap.rbegin(); it != smash.jobs_list.jobsMap.rend(); it++){
      if(it->second.status == STOPPED){
        target_job_id = it->first;
        break;
      }
    }
    if(it == jobs_map.rend()){
     cerr<<"smash error: bg: there is no stopped jobs to resume"<<endl;
     return;
    }
  }
      
  if(it->second.status == UNFINISHED){
      cerr<<"smash error: bg: job-id " << target_job_id <<" is already running in the background"<<endl;
      return;
    }
  
  std::cout<< it->second.cmd << " : " << it->second.pid <<endl;
  it->second.status = UNFINISHED;

  if(kill(it->second.pid, SIGCONT) == -1){
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
      std::cout << "smash: sending SIGKILL signal to " << smash.jobs_list.jobsMap.size() << " jobs:"<<endl;
      smash.jobs_list.killAllJobs();
      exit(0);
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
    if(kill(job.pid, SIGKILL) == -1){
      perror("smash error: kill failed");
      return;
    }
  }
}
