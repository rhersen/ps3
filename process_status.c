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
