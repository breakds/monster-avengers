#ifndef _MICRO_HTTP_SERVER_DAEMON_
#define _MICRO_HTTP_SERVER_DAEMON_

#include <cerrno>
#include <csignal>
#include <cstring>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

#include "supp/helpers.h"

using namespace monster_avengers;

namespace micro_http_server {

  class Daemon {
  private:
    void ForkAndDetach() {
      pid_t pid = fork();
      CHECK(-1 != pid);
      if (0 != pid) {
	_exit(0);
      }
    }
  
  public:
    Daemon(const std::string &err_file, 
	   const std::string &out_file, 
	   const std::string &working_directory) {
      err_file_ = err_file_.empty() ? "/dev/null" : err_file_;
      out_file_ = out_file_.empty() ? "/dev/null" : out_file_;
      working_directory_ = working_directory.empty() ?
	"/tmp" : working_directory;
      started_ = false;
    }

    ~Daemon() {
      if (started_) {
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
      }
    }

    void Start() {
      // This is a modified version of the sample daemon starting code at
      // http://www.microhowto.info/howto/cause_a_process_to_become_a_daemon_in_c.html.

      // First fork
      ForkAndDetach();

      // Set session id
      CHECK(-1 != setsid());

      signal(SIGHUP, SIG_IGN);
  
      // Second fork
      ForkAndDetach();

      // Set working directory
      CHECK(-1 != chdir(working_directory_.c_str()));

      // Set the user file creation mask to zero.
      umask(0);

      // reopen stdin
      std::freopen("/dev/null", "r", stdin);

      // reopen stdout
      std::freopen(out_file_.c_str(), "w", stdout);

      // reopen stderr
      std::freopen(err_file_.c_str(), "w", stderr);
    
      started_ = true;
    }

  private:
    bool started_;
    std::string err_file_;
    std::string out_file_;
    std::string working_directory_;
  };

}  // namespace micro_http_server

#endif  // _MICRO_HTTP_SERVER_DAEMON_
