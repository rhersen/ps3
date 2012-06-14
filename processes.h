#include <vector>

class processes {
  std::vector<process_status*> ps;
 public:
  processes();
  void free_processes_status();
  void diff(processes, int);
  long size();
  std::vector<process_status*>::iterator begin();
  std::vector<process_status*>::iterator end();
};
