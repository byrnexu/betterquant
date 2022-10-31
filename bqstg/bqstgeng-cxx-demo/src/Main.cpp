/*!
 * \file Main.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "StgEng.hpp"
#include "StgInstTaskHandlerOfCPerpTest.hpp"
#include "StgInstTaskHandlerOfPerpTest.hpp"
#include "StgInstTaskHandlerOfSpotTest.hpp"
#include "config-proj.hpp"
#include "util/ProgOpt.hpp"

using namespace bq::stg;

int main(int argc, char** argv) {
  bq::GFlagsHolder gflagsHolder(argc, argv, PROJ_VER, "--conf=filename");

  const auto stgEng = std::make_shared<StgEng>(bq::FLAGS_conf);
  const auto stgInstTaskHandler =
      std::make_shared<StgInstTaskHandlerOfSpotTest>(stgEng.get());
  if (const auto ret = stgEng->init(stgInstTaskHandler); ret != 0) {
    return EXIT_FAILURE;
  }

  if (const auto ret = stgEng->run(); ret != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
