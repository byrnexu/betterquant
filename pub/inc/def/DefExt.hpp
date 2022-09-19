#pragma once

#include "util/Pch.hpp"
#include "util/StdExt.hpp"

namespace bq {

#define API extern "C" BOOST_SYMBOL_EXPORT

#define CONFIG Config::get_mutable_instance().get()
#define CONV_OPT(type, val) boost::convert<type>((val))
#define CONV(type, val) boost::convert<type>((val)).value()

#define GET_RAND_STR() RandomStr::get_mutable_instance().get()
#define GET_RAND_INT() RandomInt::get_mutable_instance().get()

#define LOCK(mtx)                                               \
  std::unique_ptr<std::lock_guard<std::mutex>> guard;           \
  if (lockFunc == LockFunc::True) {                             \
    guard = std::make_unique<std::lock_guard<std::mutex>>(mtx); \
  }

#define SPIN_LOCK(mtx)                                                    \
  std::unique_ptr<std::lock_guard<std::ext::spin_mutex>> guard;           \
  if (lockFunc == LockFunc::True) {                                       \
    guard = std::make_unique<std::lock_guard<std::ext::spin_mutex>>(mtx); \
  }

#define JSER(Type, ...)                                                        \
  friend void to_json(nlohmann::ordered_json& nlohmann_json_j,                 \
                      const Type& nlohmann_json_t) {                           \
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))   \
  }                                                                            \
  friend void from_json(const nlohmann::ordered_json& nlohmann_json_j,         \
                        Type& nlohmann_json_t) {                               \
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM, __VA_ARGS__)) \
  }

#define MIDX_MEMER BOOST_MULTI_INDEX_MEMBER

#define LOWER_ENUM(enum_value) \
  boost::to_lower_copy(std::string(magic_enum::enum_name((enum_value))))

#define UPPER_ENUM(enum_value) \
  boost::to_upper_copy(std::string(magic_enum::enum_name((enum_value))))

#define ENUM_VALUE_TO_STR(enum_value) \
  std::string(magic_enum::enum_name((enum_value)))

using Doc = rapidjson::Document;
using DocSPtr = std::shared_ptr<Doc>;
using Val = rapidjson::Value;
using ValSPtr = std::shared_ptr<Val>;

struct JsonData {
  JsonData(const std::string& str) {
    doc_ = yyjson_read(str.data(), str.size(), 0);
    root_ = yyjson_doc_get_root(doc_);
  }
  JsonData(yyjson_doc* doc, yyjson_val* root) : doc_(doc), root_(root) {}
  yyjson_doc* doc_{nullptr};
  yyjson_val* root_{nullptr};
  ~JsonData() {
    if (doc_) {
      yyjson_doc_free(doc_);
      doc_ = nullptr;
    }
  }
};
using JsonDataSPtr = std::shared_ptr<JsonData>;

}  // namespace bq

struct boost::cnv::by_default : boost::cnv::spirit {};
