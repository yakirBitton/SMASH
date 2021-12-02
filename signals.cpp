#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
  cout << "smash: got ctrl-Z" << endl;
  SmallShell& smash = SmallShell::getInstance();
  JobsList::JobEntry job = smash.jobs_list.fgJob;
  if(job.pid == -1){
    return;
  }
  if(smash.jobs_list.fg_job_id == 0){
    if(smash.jobs_list.jobsMap.empty()){
      smash.jobs_list.fg_job_id = 1;
    }else{
      smash.jobs_list.fg_job_id = (smash.jobs_list.jobsMap.rbegin()->first) + 1;
    }
  }
  job.status = STOPPED;
  job.add_time = time(NULL);
  job.stop_time = time(NULL);
  smash.jobs_list.jobsMap[smash.jobs_list.fg_job_id] = job;
  if(killpg(smash.jobs_list.fgJob.pid, SIGSTOP) == -1){
    perror("smash error: kill failed");
    return;
  }
  cout << "smash: proccess " << job.pid << " was stopped" << endl;
  smash.jobs_list.fgJob = job.createDummy();
  smash.jobs_list.fg_job_id = NO_FG_JOB;
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  cout << "smash: got ctrl-C" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if(smash.jobs_list.fgJob.pid == -1){
    return;
  }
  if(killpg(smash.jobs_list.fgJob.pid, SIGKILL) == -1){
    perror("smash error: kill failed");
    return;
  }
  cout << "smash: proccess " << smash.jobs_list.fgJob.pid << " was killed" << endl;
  smash.jobs_list.fgJob = smash.jobs_list.fgJob.createDummy();
  smash.jobs_list.fg_job_id = NO_FG_JOB;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
  cout << "smash got an alarm" << endl;
}

