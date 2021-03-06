#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process_status.h"

#include "processes.h"

using namespace std;

processes::processes() {
  init("/proc");
}

processes::processes(const char* proc_dir) {
  init(proc_dir);
}

void processes::init(const char* proc_dir) {
  auto proc = opendir(proc_dir);
  dirent* psdir;

  if (!proc) {
    return;
  }

  while (psdir = readdir(proc)) {
    FILE* statfile = 0;
    char statname[32] = "";

    if (isdigit(psdir->d_name[0])) {
      strcat(statname, proc_dir);
      strcat(statname, "/");
      strcat(statname, psdir->d_name);
      strcat(statname, "/stat");
      statfile = fopen(statname, "r");
    }

    if (statfile) {
      ps.push_back(new process_status(statfile));
      fclose(statfile);
    }
  }

  closedir(proc);
}

void processes::free_processes_status() {
  for (auto it: ps) {
    delete(it);
  }
}

int operator>(ProcessStatus& p1, ProcessStatus& p2) {
  return p1.getPid() > p2.getPid();
}

int operator<(ProcessStatus& p1, ProcessStatus& p2) {
  return p1.getPid() < p2.getPid();
}

void processes::diff(processes oldps, int ups) {
  auto newIt = begin();
  auto oldIt = oldps.begin();
  while (newIt != end()) {
    auto newp = *newIt;
    ProcessStatus* oldp;
    if (oldIt == oldps.end() || *newp < *(oldp = *oldIt)) {
      newp->createCpuHistory();
      newIt++;
    } else if (*newp > *oldp) {
      oldIt++;
    } else {
      newp->updateCpuHistory(oldp, ups);
      newIt++;
      oldIt++;
    }
  }
}

vector<ProcessStatus*>::iterator processes::begin() {
  return ps.begin();
}

vector<ProcessStatus*>::iterator processes::end() {
  return ps.end();
}

void processes::push_back(ProcessStatus* p) {
  ps.push_back(p);
}

long processes::size() {
  return ps.size();
}
