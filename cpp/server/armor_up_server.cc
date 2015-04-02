#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include "micro_http_server.h"
#include "supp/helpers.h"
#include "core/armor_up.h"

using namespace micro_http_server;

std::unique_ptr<ArmorUp> armor_up;

void ForkAndDetach() {
  pid_t pid = fork();
  CHECK(-1 != pid);
  if (0 != pid) {
    _exit(0);
  }
}

void StartDaemon(const std::string &working_directory) {
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
  CHECK(-1 != chdir(working_directory.c_str()));

  // Set the user file creation mask to zero.
  umask(0);
  
  // reopen stdin
  std::freopen("/dev/null", "r", stdin);

  // reopen stdout
  std::freopen("/dev/null", "w", stdout);


  // reopen stderr
  std::freopen("/home/breakds/tmp/process.txt", "w", stderr);
}


int main(int argc, char **argv) {
  if (argc < 2) {
    Log(FATAL, L"Please specifcy the dataset folder.");
  }

  StartDaemon(argv[1]);
  
  // Initialize the armor up engine.
  armor_up.reset(new ArmorUp(argv[1]));

  MicroHttpServer<SpecialPostHandler> server(8887);

  while (true) {
    sleep(1);
  }

  std::fclose(stderr);
  
  return 0;
}
