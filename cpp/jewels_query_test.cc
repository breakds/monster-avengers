#include "jewels_query.h"


using namespace monster_avengers;

void Print(const std::unordered_set<Signature> &result,
           const std::vector<Effect> &required) {
  wprintf(L"--------------------\n");
  for (const Signature &key : result) {
    sig::ExplainSignature(key, required);
  }
}

void Compare(HoleClient &hole_client, int i, int j, int k,
             const std::vector<Effect> &required) {

  std::unordered_set<Signature> dfs_result = 
    hole_client.DFS(i, j, k);
  std::unordered_set<Signature> alg_result = 
    hole_client.Query(i, j, k);
  Print(dfs_result, required);
  Print(alg_result, required);
  wprintf(L"dfs size: %lld\n", dfs_result.size());
  wprintf(L"alg size: %lld\n", alg_result.size());
  for (const Signature &key : dfs_result) {
    if (0 == alg_result.count(key)) {
      sig::ExplainSignature(key, required);
    }
  }
}

int main() {
  std::setlocale(LC_ALL, "en_US.UTF-8");

  DataSet data("/home/breakds/pf/projects/monster-avengers/dataset/TESTING");
  std::vector<Effect> required = {Effect(0, 2), Effect(1, 4)};
  HoleClient hole_client(data, required);

  CHECK(hole_client.DFS(1, 0, 0) == hole_client.Query(1, 0, 0));
  CHECK(hole_client.DFS(2, 0, 0) == hole_client.Query(2, 0, 0));
  CHECK(hole_client.DFS(3, 0, 0) == hole_client.Query(3, 0, 0));
  CHECK(hole_client.DFS(4, 0, 0) == hole_client.Query(4, 0, 0));
  CHECK(hole_client.DFS(5, 0, 0) == hole_client.Query(5, 0, 0));
  CHECK(hole_client.DFS(6, 0, 0) == hole_client.Query(6, 0, 0));
  CHECK(hole_client.DFS(7, 0, 0) == hole_client.Query(7, 0, 0));
  CHECK(hole_client.DFS(0, 1, 0) == hole_client.Query(0, 1, 0));
  CHECK(hole_client.DFS(0, 5, 0) == hole_client.Query(0, 5, 0));
  CHECK(hole_client.DFS(1, 1, 0) == hole_client.Query(1, 1, 0));
  CHECK(hole_client.DFS(1, 2, 0) == hole_client.Query(1, 2, 0));
  CHECK(hole_client.DFS(3, 4, 0) == hole_client.Query(3, 4, 0));
  CHECK(hole_client.DFS(0, 0, 1) == hole_client.Query(0, 0, 1));
  CHECK(hole_client.DFS(2, 2, 1) == hole_client.Query(2, 2, 1));
  CHECK(hole_client.DFS(1, 0, 6) == hole_client.Query(1, 0, 6));
  
  return 0;
}
