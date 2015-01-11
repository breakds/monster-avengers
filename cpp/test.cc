#include "monster_hunter_data.h"

using namespace monster_avengers;

int main(int argc, char **argv) {
  std::setlocale(LC_ALL, "en_US.UTF-8");
  DataSet dataset("/home/breakds/pf/projects/monster-avengers/dataset/MH4");
  dataset.Summarize();
  return 0;
}
