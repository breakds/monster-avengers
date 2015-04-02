#ifndef _MICROHTTPD_WRAPPER_
#define _MICROHTTPD_WRAPPER_

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <memory>
#include "supp/helpers.h"

using namespace monster_avengers;

namespace micro_http_server {

  const int MAX_POST_DATA_SIZE = 512;


  class SpecialPostHandler {
  public:
    static int IteratePostData(void *coninfo, MHD_ValueKind kind, 
		    const char *key, const char *filename,
		    const char *content_type, const char *transfer_encoding,
		    const char *data, uint64_t off, size_t size) {
      Log(INFO, L"called with %s = %s.", key, data); 
      if (0 == strcmp(key, "query")) {
	return MHD_NO;
      }
      return MHD_YES;
    }

    void Initialize(MHD_Connection *connection) {
      post_processor_ = MHD_create_post_processor(connection, 
						  MAX_POST_DATA_SIZE,
						  IteratePostData,
						  static_cast<void*>(this));
    }

    void ProcessPostData(const char *data, size_t *size) {
      MHD_post_process(post_processor_, data, *size);
      *size = 0;
    }

    int HandleRequest(MHD_Connection *connection) {
      std::wstring json_result = L"{result: 12, success: true}";
      wchar_t buffer[512];
      wcscpy(buffer, json_result.c_str());
      MHD_Response *response = 
	MHD_create_response_from_buffer(json_result.size() * sizeof(wchar_t),
					static_cast<void*>(buffer),
					MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json; char-set=utf-8");
      MHD_add_response_header(response, "Connection", "Keep-Alive");
      int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);
      return ret;
    }

  private:
    MHD_PostProcessor *post_processor_;
  };
  
  template <typename PostHandler>
  class MicroHttpServer {
  public:
    MicroHttpServer(int port) {
      daemon_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port,
				 nullptr, nullptr, &EntryPoint, nullptr, 
				 MHD_OPTION_NOTIFY_COMPLETED, &RequestComplete, 
				 nullptr, MHD_OPTION_END);
    }

    ~MicroHttpServer() {
      MHD_stop_daemon(daemon_);
    }
    
  private:
    static void RequestComplete(void *cls, MHD_Connection *connection, 
			 void **con_cls, 
			 MHD_RequestTerminationCode toe) {
      Log(INFO, L"RequestComplete called.");
      PostHandler *handler = static_cast<PostHandler*>(*con_cls);
      if (nullptr != handler) {
	delete handler;	
	*con_cls = nullptr;
      }
    }

    static int EntryPoint(void *cls,
			  MHD_Connection *connection,
			  const char *url,
			  const char *method,
			  const char *version,
			  const char *upload_data,
			  size_t *upload_data_size,
			  void **con_cls) {
      if (nullptr == *con_cls) {
	if (0 == strcmp(method, "POST")) {
	  PostHandler *handler = new PostHandler();
	  *con_cls = handler;
	}
	return MHD_YES;
      }

      if (0 == strcmp(method, "POST")) {
	PostHandler *handler = static_cast<PostHandler*>(*con_cls);
	if (*upload_data_size > 0) {
	  handler->ProcessPostData(upload_data, upload_data_size);
	  return MHD_YES;
	} else {
	  return handler->HandleRequest(connection);
	}
      }
      return MHD_NO;
    }

    MHD_Daemon *daemon_;
  };
  
}  // namespace microhttpd

#endif  // _MICROHTTPD_WRAPPER_
