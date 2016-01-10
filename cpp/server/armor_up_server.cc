#include <memory>

#include "core/armor_up.h"
#include "daemon.h"
#include "dataset/dataset.h"
#include "micro_http_server.h"
#include "supp/helpers.h"
#include "utils/query.h"
#include "utils/formatter.h"


using micro_http_server::Daemon;
using micro_http_server::PostHandler;
using micro_http_server::SimplePostServer;


class SpecialPostHandler : public PostHandler{
public:

  SpecialPostHandler() : PostHandler(), query_cache_(""),
                         armor_up_(), formatter_(&armor_up_.GetArsenal()) {}
    
  int ProcessKeyValue(const std::string &key, 
		      const std::string &value) override {
    if (key == "query") {
      query_cache_ = value;
      return MHD_NO;
    }
    // Ignore anything when the first key value pair is not for "query".
    return MHD_NO;
  }

  std::string GenerateResponse() override {
    if (query_cache_.size() == 0) {
      return "{ \"errorMessage\": \"no query received.\" }";
    }

    // Convert the query to wstring.
    std::wstring query_text;
    query_text.assign(query_cache_.begin(), query_cache_.end());

    // Parse the query.
    Query query;
    if (!Query::Parse(query_text, &query).Success()) {
      return "{ \"errorMessage\": \"Corrupted Query\" }";
    } 

    // ArmorUp Search.
    std::vector<ArmorSet> result = std::move(armor_up_.Search(query));
    return formatter_.StringBatchFormat(result);
  }

  std::string query_cache_;
  ArmorUp armor_up_;
  DexJsonFormatter formatter_;
};


int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  
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

  Log(INFO, L"Loading database ...");
  Data::LoadSQLite(argv[1]);
  Log(INFO, L"Armor up!");
  
  Log(INFO, L"Starting server ...");
  SimplePostServer<SpecialPostHandler> server(port);
  Log(INFO, L"Server Started.");
  
  while (true) {
    sleep(100);
  }

  return 0;
}
