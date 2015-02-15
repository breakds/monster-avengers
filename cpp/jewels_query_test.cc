#include <utility>
#include <unordered_map>
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

void TestResidual(int one, int two, int three,
                  int stuffed_one, int stuffed_two, int stuffed_three,
                  int i, int j, int k) {
  Signature original = sig::HolesToKey(one, two, three);
  Signature stuffed = sig::HolesToKey(stuffed_one, stuffed_two, stuffed_three);
  int res_i(0), res_j(0), res_k(0);
  HoleClient::GetResidual(original, stuffed,
                          &res_i, &res_j, &res_k);
  // Uncomment for Debugging
  // wprintf(L"Expected: %d, %d, %d      Actual: %d, %d, %d\n", 
  //         i, j, k, res_i, res_j, res_k);
  CHECK(std::make_tuple(i, j, k) == std::make_tuple(res_i, res_j, res_k));
}

void TestJewelSolver(const DataSet &data,
                     const JewelSolver& solver, 
                     int i, int j, int k,
                     int p0, int p1, int p2) {
  std::unordered_map<int, int> result =
    std::move(solver.Solve(sig::ConstructKey(i, j, k, {p0, p1, p2})));
  wprintf(L"----------\n");
  for (auto &item : result) {
    wprintf(L"%3d x %ls\n", item.second, 
            data.jewel(item.first).name.c_str());
  }
}


int main() {
  std::setlocale(LC_ALL, "en_US.UTF-8");

  DataSet data("/home/breakds/pf/projects/monster-avengers/dataset/TESTING");
  std::vector<Effect> required = {Effect(0, 2), Effect(1, 4)};
  HoleClient hole_client(data, required);

  CHECK(hole_client.DFS(0, 0, 0) == hole_client.Query(0, 0, 0));
  CHECK(hole_client.Query(0, 0, 0).size() == 1); // has all zero signature.
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


  // Residual tests
  TestResidual(1, 2, 2, 0, 2, 0, 1, 0, 2);
  TestResidual(0, 0, 2, 0, 2, 0, 2, 0, 0);
  TestResidual(0, 0, 2, 1, 1, 1, 0, 0, 0);
  TestResidual(2, 2, 1, 4, 2, 0, 1, 0, 0);
  TestResidual(2, 2, 1, 4, 1, 1, 0, 0, 0);
  TestResidual(1, 2, 2, 8, 0, 0, 0, 0, 1);

  // JewelSolver tests
  JewelSolver solver(data, {{0, 10}, {1, 10}, {2, 10}});
  TestJewelSolver(data, solver, 2, 0, 0, 0, 0, 0);
  TestJewelSolver(data, solver, 0, 2, 0, 0, 0, 6);
  TestJewelSolver(data, solver, 0, 2, 1, 11, -3, 0);
  TestJewelSolver(data, solver, 0, 4, 1, 10, 0, 3);
  TestJewelSolver(data, solver, 4, 4, 1, 6, 4, 3);

  return 0;
}
