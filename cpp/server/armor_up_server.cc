#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

#include "supp/helpers.h"
#include "core/armor_up.h"

using namespace monster_avengers;

std::unique_ptr<ArmorUp> armor_up;

std::string query_cache;

void ForkAndDetach() {
  pid_t pid = fork();
  CHECK(-1 != pid);
  if (0 != pid) {
    _exit(0);
  }
}

struct PostHandler {
  MHD_PostProcessor *post_processor;
};

int IteratePostData(void *coninfo, MHD_ValueKind kind, 
		    const char *key, const char *filename,
		    const char *content_type, const char *transfer_encoding,
		    const char *data, uint64_t off, size_t size) {
  Log(INFO, L"called with %s = %s.", key, data); 
  if (0 == strcmp(key, "query")) {
    query_cache = data;
    return MHD_NO;
  }
  return MHD_YES;
}

void RequestComplete(void *cls, MHD_Connection *connection, 
		     void **con_cls, MHD_RequestTerminationCode toe) {
  Log(INFO, L"RequestComplete called.");
  PostHandler *handler = static_cast<PostHandler*>(*con_cls);
  if (nullptr != handler) {
    MHD_destroy_post_processor(handler->post_processor);
  }
  
  delete handler;
  *con_cls = nullptr;
}

int AnswerRequest(void *cls,
                  MHD_Connection *connection,
                  const char *url,
                  const char *method,
                  const char *version,
                  const char *upload_data,
                  size_t *upload_data_size,
                  void **con_cls) {

  if (nullptr == *con_cls) {
    Log(INFO, L"started processing request.");
    PostHandler *handler = new PostHandler();
    if (0 == strcmp(method, "POST")) {
      handler->post_processor = MHD_create_post_processor(connection,
							  512,
							  IteratePostData,
							  static_cast<void*>(handler));
    }
    *con_cls = handler;
    return MHD_YES;
  }

  int ret = 0;
  
  if (0 == strcmp(method, "POST")) {
    PostHandler *handler = static_cast<PostHandler*>(*con_cls);
    if (*upload_data_size > 0) {
      MHD_post_process(handler->post_processor, upload_data, *upload_data_size);
      *upload_data_size = 0;
      return MHD_YES;
    } else {
      const wchar_t *json_result = L"{result: 12, success: true}";
      MHD_Response *response = 
	MHD_create_response_from_buffer(wcslen(json_result) * sizeof(wchar_t),
					(void*) json_result,
					MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      MHD_add_response_header(response, "Connection", "Keep-Alive");
      ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);
      Log(INFO, L"Answered a query.");
      return ret;
    }
  }

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

  if (argc < 2) {
    Log(FATAL, L"Please specifcy the dataset folder.");
  }
  
  StartDaemon(argv[1]);

  // Initialize the armor up engine.
  armor_up.reset(new ArmorUp(argv[1]));
  
  
  MHD_Daemon *daemon;
  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8887, 
                            nullptr, nullptr,
                            &AnswerRequest, nullptr, 
			    MHD_OPTION_NOTIFY_COMPLETED, &RequestComplete, 
			    nullptr, MHD_OPTION_END);
  while (true) {
    sleep(1);
  }

  std::fclose(stderr);
  MHD_stop_daemon(daemon);
  
  return 0;
}
