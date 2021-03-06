#include <vector>

class processes {
  std::vector<ProcessStatus*> ps;
  void init(const char*);
 public:
  processes(const char*);
  processes();
  void free_processes_status();
  void diff(processes, int);
  long size();
  void push_back(ProcessStatus* p);
  std::vector<ProcessStatus*>::iterator begin();
  std::vector<ProcessStatus*>::iterator end();
};
