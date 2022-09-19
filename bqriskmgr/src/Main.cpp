#include "RiskMgr.hpp"
#include "config-proj.hpp"
#include "util/BQUtil.hpp"
#include "util/Pch.hpp"
#include "util/ProgOpt.hpp"

using namespace bq::riskmgr;

int main(int argc, char** argv) {
  bq::GFlagsHolder gflagsHolder(argc, argv, PROJ_VER, "--conf=filename");
  bq::PrintLogo();

  const auto svc = std::make_shared<bq::riskmgr::RiskMgr>(bq::FLAGS_conf);

  if (const auto ret = svc->init(); ret != 0) {
    return EXIT_FAILURE;
  }

  if (const auto ret = svc->run(); ret != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
