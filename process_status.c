#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ps3.h"
#include "process_status.h"

int process_count;

static void remove_parens(char *s) {
  if (s[0] == '(') {
    strcpy(s, s + 1);
  }

  {
    size_t last = strlen(s) - 1;

    if (s[last] == ')') {
      s[last] = 0;
    }
  }
}

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

    remove_parens(comm);
}

process_status *read_processes_status(void) {
  process_status *r = 0;
  process_status *p = 0;
  DIR *proc = opendir("/proc");
  struct dirent *psdir;

  process_count = 0;

  while (psdir = readdir(proc)) {
    FILE *statfile = 0;
    char statname[32] = "/proc/";

    if (isdigit(psdir->d_name[0])) {
      strcat(statname, psdir->d_name);
      strcat(statname, "/stat");
      statfile = fopen(statname, "r");
    }

    if (statfile) {
      process_status *newp = new process_status(statfile);

      process_count++;

      if (!r) {
	r = newp;
      } else {
	p->next_process = newp;
      }

      p = newp;

      fclose(statfile);
    }
  }

  closedir(proc);

  return r;
}

void free_processes_status(process_status *thiz) {
  while (thiz) {
    process_status *next = thiz->next_process;
    free(thiz);
    thiz = next;
  }
}

void diff(process_status *newp, process_status *oldp) {
  while (newp) {
    if (!oldp || newp->pid < oldp->pid) {
/*       printf("%d %s was started\n", newp->pid, newp->comm); */
      newp->cpuhist[newp->histidx] = newp->utime + newp->stime;
      newp->cpu += newp->cpuhist[newp->histidx];
      newp = newp->next_process;
    } else if (newp->pid > oldp->pid) {
/*       printf("%d %s died\n", oldp->pid, oldp->comm); */
      oldp = oldp->next_process;
    } else {
      int i;

      for (i = 0; i < ups; i++) {
	newp->cpuhist[i] = oldp->cpuhist[i];
	newp->cpu += oldp->cpuhist[i];
      }

      newp->cpuhist[oldp->histidx] = newp->utime + newp->stime - oldp->utime - oldp->stime;
      newp->histidx = (oldp->histidx + 1) % ups;   

/*       if (strcmp(newp->comm, "gltop") == 0 && newp->cpuhist[oldp->histidx]) { */
/* 	printf("%d\n", newp->cpuhist[oldp->histidx]); */
/*       } */

      newp = newp->next_process;
      oldp = oldp->next_process;
    }
  }

/*   if (oldp) { */
/*     printf("%d %s died\n", oldp->pid, oldp->comm); */
/*   } */
}
