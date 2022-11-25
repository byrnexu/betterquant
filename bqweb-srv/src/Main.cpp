/*!
 * \file Main.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/20
 *
 * \brief
 */

#include "WebSrv.hpp"
#include "config-proj.hpp"
#include "util/BQUtil.hpp"
#include "util/Pch.hpp"
#include "util/ProgOpt.hpp"

using namespace bq;

int main(int argc, char** argv) {
  bq::GFlagsHolder gflagsHolder(argc, argv, PROJ_VER, "--conf=filename");
  bq::PrintLogo();

  if (const auto ret = bq::WebSrv::get_mutable_instance().init(bq::FLAGS_conf);
      ret != 0) {
    return EXIT_FAILURE;
  }

  if (const auto ret = bq::WebSrv::get_mutable_instance().run(); ret != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
