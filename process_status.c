#include <stdio.h>

#include "process_status.h"

process_status::process_status(FILE* statfile) :
next_process(), cpu(), cpuhist(), histidx()
{
  int assigned = fscanf(statfile, "%d %s "
                        "%c "
                        "%d %d %d %d %d "
                        "%lu "
                        "%lu %lu %lu %lu "
                        "%Lu %Lu %Lu %Lu "  /* utime stime cutime cstime */
                        "%ld %ld %ld %ld "
                        "%Lu "  /* start_time */
                        "%lu "
                        "%ld ",
                        &pid, comm,
                        &state,
                        &ppid, &pgrp, &session, &tty, &tpgid,
                        &flags,
                        &min_flt, &cmin_flt, &maj_flt, &cmaj_flt,
                        &utime, &stime, &cutime, &cstime,
                        &priority, &nice, &timeout, &it_real_value,
                        &start_time,
                        &vsize,
                        &rss);

  if (assigned != 24) {
    throw "could not read statfile";
  }
}

void process_status::createCpuHistory() {
  cpuhist[histidx] = utime + stime;
  cpu += cpuhist[histidx];
}

void process_status::updateCpuHistory(ProcessStatus* oldp, int ups) {
  for (auto i = 0; i < ups; i++) {
    cpuhist[i] = oldp->getCpuhist(i);
    cpu += oldp->getCpuhist(i);
  }

  cpuhist[oldp->getHistidx()] = utime + stime - oldp->getUtime() - oldp->getStime();
  histidx = (oldp->getHistidx() + 1) % ups;   
}

int process_status::getPid() {
  return pid;
}

unsigned process_status::getCpuhist(int i) {
  return cpuhist[i];
}

int process_status::getHistidx() {
  return histidx;
}

unsigned long long process_status::getUtime() {
  return utime;
}

unsigned long long process_status::getStime() {
  return stime;
}

char process_status::getState() {
  return state;
}

long process_status::getNice() {
  return nice;
}
char* process_status::getComm() {
  return comm;
}
unsigned process_status::getCpu() {
  return cpu;
}
long process_status::getRss() {
  return rss;
}
