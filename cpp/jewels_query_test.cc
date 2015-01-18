#include "jewels_query.h"


using namespace monster_avengers;

void Print(const std::unordered_set<Signature> &result,
           const std::vector<Effect> &required) {
  wprintf(L"--------------------\n");
  for (const Signature &key : result) {
    sig::ExplainSignature(key, required);
  }
}

int main() {
  std::setlocale(LC_ALL, "en_US.UTF-8");

  DataSet data("/home/breakds/pf/projects/monster-avengers/dataset/TESTING");
  std::vector<Effect> required = {Effect(0, 2), Effect(1, 4)};
  HoleClient hole_client(data, required);

  wprintf(L"size: %lld\n", hole_client.DFS(0, 5, 0).size());
  wprintf(L"size: %lld\n", hole_client.Query(0, 5, 0).size());
  for (const Signature &key : hole_client.DFS(0, 5, 0)) {
    if (0 == hole_client.Query(0, 5, 0).count(key)) {
      sig::ExplainSignature(key, required);
    }
  }
  // Print(hole_client.DFS(0, 5, 0), required);
  // Print(hole_client.Query(0, 5, 0), required);
  
  CHECK(hole_client.DFS(1, 0, 0) == hole_client.Query(1, 0, 0));
  CHECK(hole_client.DFS(2, 0, 0) == hole_client.Query(2, 0, 0));
  CHECK(hole_client.DFS(3, 0, 0) == hole_client.Query(3, 0, 0));
  CHECK(hole_client.DFS(4, 0, 0) == hole_client.Query(4, 0, 0));
  CHECK(hole_client.DFS(5, 0, 0) == hole_client.Query(5, 0, 0));
  CHECK(hole_client.DFS(6, 0, 0) == hole_client.Query(6, 0, 0));
  CHECK(hole_client.DFS(7, 0, 0) == hole_client.Query(7, 0, 0));
  CHECK(hole_client.DFS(0, 1, 0) == hole_client.Query(0, 1, 0));
  CHECK(hole_client.DFS(0, 5, 0) == hole_client.Query(0, 5, 0));
  
  return 0;
}
