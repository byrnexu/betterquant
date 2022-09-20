/*!
 * \file MDWSCliAsyncTaskArg.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "def/BQConst.hpp"
#include "def/Def.hpp"

namespace bq::md {

struct WSCliAsyncTaskArg;
using WSCliAsyncTaskArgSPtr = std::shared_ptr<WSCliAsyncTaskArg>;

struct WSCliAsyncTaskArg {
  WSMsgType wsMsgType_;
  yyjson_doc *doc_{nullptr};
  yyjson_val *root_{nullptr};
  ~WSCliAsyncTaskArg() {
    if (doc_) {
      yyjson_doc_free(doc_);
      doc_ = nullptr;
    }
  }
};

}  // namespace bq::md
