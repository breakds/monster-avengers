#include <memory>

#include "micro_http_server.h"
#include "daemon.h"
#include "supp/helpers.h"
#include "core/armor_up.h"

using micro_http_server::Daemon;
using micro_http_server::PostHandler;
using micro_http_server::SimplePostServer;

std::unique_ptr<ArmorUp> armor_up;


class SpecialPostHandler : public PostHandler{
public:
    
  int ProcessKeyValue(const std::string &key, 
		      const std::string &value) override {
    if (key == "query") {
      query_cache_ = value;
      return MHD_NO;
    }
    return MHD_YES;
  }

  std::string GenerateResponse() override {
    std::string content;
    try {
      std::wstring query_text;
      query_text.assign(query_cache_.begin(), query_cache_.end());
      Query query;
      if (!Query::Parse(query_text, &query).Success()) {
        throw 0;
      }
      std::wstring answer = std::move(armor_up->SearchSerialized(query));
      content.assign(answer.begin(), answer.end());
    } catch (int e) {
      content = "\"Query Format Error!\"";
    }
    return content;
  }

  std::string query_cache_;
};


int main(int argc, char **argv) {
  if (argc < 2) {
    Log(FATAL, L"Please call the command as: armor_up_server [dataset folder] [port]");
  }

  int port = 8887;
  
  if (argc >= 3) {
    try {
      port = std::stoi(argv[2]);
    } catch (std::invalid_argument&) {
      Log(FATAL, L"Invalid port %s.", argv[2]);
      _exit(-1);
    }
  }
  
  Log(INFO, L"Starting Server.");

  Daemon daemon("/tmp/armor_up_log.txt", "/tmp/armor_up_log.txt", argv[1]);
  daemon.Start();

  Log(INFO, L"Server Started.");

  // Initialize the armor up engine.
  armor_up.reset(new ArmorUp(argv[1]));

  Log(INFO, L"armor up!");

  SimplePostServer<SpecialPostHandler> server(port);

  while (true) {
    sleep(10);
  }

  return 0;
}
