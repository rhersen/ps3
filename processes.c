#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process_status.h"

#include "processes.h"

using namespace std;

processes::processes() {
  DIR *proc = opendir("/proc");
  struct dirent *psdir;

  while (psdir = readdir(proc)) {
    FILE *statfile = 0;
    char statname[32] = "/proc/";

    if (isdigit(psdir->d_name[0])) {
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

void processes::diff(processes oldps, int ups) {
  auto newIt = begin();
  auto oldIt = oldps.begin();
  while (newIt != end()) {
    auto newp = *newIt;
    auto oldp = *oldIt;
    if (oldIt == oldps.end() || newp->pid < oldp->pid) {
      newp->cpuhist[newp->histidx] = newp->utime + newp->stime;
      newp->cpu += newp->cpuhist[newp->histidx];
      newIt++;
    } else if (newp->pid > oldp->pid) {
      oldIt++;
    } else {
      int i;

      for (i = 0; i < ups; i++) {
        newp->cpuhist[i] = oldp->cpuhist[i];
        newp->cpu += oldp->cpuhist[i];
      }

      newp->cpuhist[oldp->histidx] = newp->utime + newp->stime - oldp->utime - oldp->stime;
      newp->histidx = (oldp->histidx + 1) % ups;   

      newIt++;
      oldIt++;
    }
  }
}

vector<process_status*>::iterator processes::begin() {
  return ps.begin();
}

vector<process_status*>::iterator processes::end() {
  return ps.end();
}

long processes::size() {
  return ps.size();
}
