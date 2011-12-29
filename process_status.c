#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ps3.h"
#include "process_status.h"

int process_count;

static struct process_status *new_process_status(void) {
  struct process_status empty = { 0 };
  struct process_status *r = malloc(sizeof(struct process_status));
  *r = empty;
  return r;
}

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

struct process_status *read_processes_status(void) {
  struct process_status *r = 0;
  struct process_status *p = 0;
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
      struct process_status *newp = new_process_status();

      process_count++;

      if (!r) {
	r = newp;
      } else {
	p->next_process = newp;
      }

      p = newp;

      if (fscanf(statfile, "%d %s "
		 "%c "
		 "%d %d %d %d %d "
		 "%lu "
		 "%lu %lu %lu %lu "
		 "%Lu %Lu %Lu %Lu "  /* utime stime cutime cstime */
		 "%ld %ld %ld %ld "
		 "%Lu "  /* start_time */
		 "%lu "
		 "%ld ",
		 &p->pid, p->comm,
		 &p->state,
		 &p->ppid, &p->pgrp, &p->session, &p->tty, &p->tpgid,
		 &p->flags,
		 &p->min_flt, &p->cmin_flt, &p->maj_flt, &p->cmaj_flt,
		 &p->utime, &p->stime, &p->cutime, &p->cstime,
		 &p->priority, &p->nice, &p->timeout, &p->it_real_value,
		 &p->start_time,
		 &p->vsize,
		 &p->rss) != 24) {
	fprintf(stderr, "can't read %s\n", statname);
	exit(1);
      }

      remove_parens(p->comm);

      fclose(statfile);
    }
  }

  closedir(proc);

  return r;
}

void free_processes_status(struct process_status *this) {
  while (this) {
    struct process_status *next = this->next_process;
    free(this);
    this = next;
  }
}

void diff(struct process_status *newp, struct process_status *oldp) {
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
