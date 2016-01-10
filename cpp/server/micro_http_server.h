#ifndef _MICRO_HTTP_SERVER_
#define _MICRO_HTTP_SERVER_

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <cstring>
#include <cwchar>
#include <cstdio>
#include <microhttpd.h>
#include <memory>
#include <type_traits>
#include "supp/helpers.h"

using namespace monster_avengers;

namespace micro_http_server {

  const int MAX_POST_DATA_SIZE = 2048;

  
  namespace {

    template <typename Handler> struct PostCycleInfo;
    
    template <typename Handler>
    int IteratePostData(void *coninfo, MHD_ValueKind kind, 
                        const char *key, const char *filename,
                        const char *content_type, 
                        const char *transfer_encoding,
                        const char *data, uint64_t off, size_t size) {
      PostCycleInfo<Handler> *info = 
        static_cast<PostCycleInfo<Handler>*>(coninfo);
      return info->handler->ProcessKeyValue(key, data);
    }

    int SendResponse(MHD_Connection *connection, char *content) {
      MHD_Response *response = 
        MHD_create_response_from_buffer(strlen(content),
        				static_cast<void*>(content),
        				MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      MHD_add_response_header(response, "Connection", "Keep-Alive");
      MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
      int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);
      return ret;
    }

    // -------------------- PostCycleInfo --------------------
    // PostCycleInfo holds the information that handles one full POST
    // request cycle. It will be destroyed after the request cycle.
    //
    // It consists of a handler, which handles the request and
    // generate the responses, and a MHD_PostProcessor, which is used
    // to iterate over the POST data key value pairs and store the
    // interesting ones.
    template <typename Handler>
    struct PostCycleInfo {
      std::unique_ptr<Handler> handler;
      MHD_PostProcessor *post_processor;

      PostCycleInfo(MHD_Connection *connection) :
	handler(new Handler()),
	post_processor(MHD_create_post_processor(connection,
						 MAX_POST_DATA_SIZE,
						 IteratePostData<Handler>,
						 static_cast<void*>(this)))
      {}


      ~PostCycleInfo() {
        MHD_destroy_post_processor(post_processor);
      }
    };

    constexpr char ERROR_GET_MESSAGE[] = 
      "GET is not supported by SimplePostServer.";
  }  // namespace


  class PostHandler {
  public:
    PostHandler() = default;
    
    virtual int ProcessKeyValue(const std::string &key,
 				const std::string &value) = 0;
    virtual std::string GenerateResponse() = 0;
    
    virtual int HandleRequest(MHD_Connection *connection) {
      std::string content = GenerateResponse();
      int size = content.size() + 4;
      buffer_.reset(new char[size]);
      snprintf(buffer_.get(), size, "%s", content.c_str());
      return SendResponse(connection, buffer_.get());
    }

  private:
    std::unique_ptr<char[]> buffer_;
  };
  
  template <typename Handler>
  class SimplePostServer {
  public:
    SimplePostServer(int port) {
      daemon_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port,
				 nullptr, nullptr, &EntryPoint, nullptr, 
				 MHD_OPTION_NOTIFY_COMPLETED, &RequestComplete, 
				 nullptr, MHD_OPTION_END);
    }

    ~SimplePostServer() {
      MHD_stop_daemon(daemon_);
    }
    
  private:
    static void RequestComplete(void *cls, MHD_Connection *connection, 
			 void **con_cls, 
			 MHD_RequestTerminationCode toe) {
      PostCycleInfo<Handler> *info = 
        reinterpret_cast<PostCycleInfo<Handler>*>(*con_cls);
      if (nullptr != info) {
        delete info;
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
          PostCycleInfo<Handler> *info = 
            new PostCycleInfo<Handler>(connection);
	  *con_cls = info;
          return MHD_YES;
	}
      }

      if (0 == strcmp(method, "GET")) {
	return SendResponse(connection, const_cast<char*>(ERROR_GET_MESSAGE));
      }

      if (0 == strcmp(method, "POST")) {
        PostCycleInfo<Handler> *info = 
          static_cast<PostCycleInfo<Handler>*>(*con_cls);
	if (*upload_data_size > 0) {
          MHD_post_process(info->post_processor, upload_data, 
                           *upload_data_size);
          *upload_data_size = 0;
	  return MHD_YES;
	} else {
	  return info->handler->HandleRequest(connection);
	}
      }
      return MHD_NO;
    }

    MHD_Daemon *daemon_;
  };
  
}  // namespace microhttpd

#endif  // _MICRO_HTTP_SERVER_
