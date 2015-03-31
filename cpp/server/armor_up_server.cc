#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include "supp/helpers.h"

using namespace monster_avengers;

void ForkAndDetach() {
  pid_t pid = fork();
  CHECK(-1 != pid);
  if (0 != pid) {
    _exit(0);
  }
}

int AnswerRequest(void *cls,
                  MHD_Connection *connection,
                  const char *url,
                  const char *method,
                  const char *version,
                  const char *upload_data,
                  size_t *upload_data_size,
                  void **con_cls) {
  const char *json_result = "{result: 12, success: true}";
  int ret = 0;
  MHD_Response *response = 
    MHD_create_response_from_buffer(strlen(json_result),
                                    (void*) json_result,
                                    MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, "Content-Type", "application/json");
  MHD_add_response_header(response, "Connection", "Keep-Alive");
  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  Log(INFO, L"Answered a query.");
  return ret;
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
  StartDaemon("/home/breakds");
  
  
  MHD_Daemon *daemon;
  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8887, 
                            nullptr, nullptr,
                            &AnswerRequest, nullptr, MHD_OPTION_END);
  while (true) {
    sleep(1);
  }

  std::fclose(stderr);
  MHD_stop_daemon(daemon);
  
  return 0;
}
