#pragma once

#include "def/Def.hpp"
#include "util/Pch.hpp"

namespace bq {

template <typename Struct>
std::vector<std::string> GetMemberNameFromStruct(Struct& stru) {
  nlohmann::ordered_json j = stru;
  std::vector<std::string> ret;
  for (auto& item : j.items()) {
    ret.emplace_back(item.key());
  }
  return ret;
}

inline rapidjson::Document CloneDoc(const rapidjson::Document& doc) {
  rapidjson::Document ret;
  ret.CopyFrom(doc, ret.GetAllocator());
  return ret;
}

inline void MergeVal(Val& target, Val& source, Val::AllocatorType& allocator) {
  assert(target.IsObject());
  assert(source.IsObject());
  for (Val::MemberIterator itr = source.MemberBegin();
       itr != source.MemberEnd(); ++itr)
    target.AddMember(itr->name, itr->value, allocator);
}

inline rapidjson::Document ConvertValToDoc(const rapidjson::Value& val) {
  rapidjson::Document ret;
  ret.CopyFrom(val, ret.GetAllocator());
  return ret;
}

inline std::string ConvertDocToJsonStr(const rapidjson::Document& doc) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  return std::string(buffer.GetString(), buffer.GetSize());
}

inline std::string ConvertValToJsonStr(const rapidjson::Value& val) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  val.Accept(writer);
  return std::string(buffer.GetString(), buffer.GetSize());
}

template <typename Struct>
std::string ConvertStructToJsonStr(const Struct& stru) {
  nlohmann::ordered_json j = stru;
  const auto ret = j.dump();
  return ret;
}

template <typename Struct>
Struct ConvertJsonStrToStruct(const std::string& jStr) {
  nlohmann::ordered_json j = nlohmann::ordered_json::parse(jStr);
  const auto ret = j.get<Struct>();
  return ret;
}

template <typename Struct>
Doc ConvertStructToDoc(const Struct& stru) {
  const auto str = ConvertStructToJsonStr(stru);
  Doc ret;
  ret.Parse(str.data());
  return ret;
}

inline std::string DiffOfJson(const std::string& lhs, const std::string& rhs) {
  const auto lhsJson = nlohmann::json::parse(lhs);
  const auto rhsJson = nlohmann::json::parse(rhs);
  const auto diff = nlohmann::json::diff(lhsJson, rhsJson);
  return diff.dump();
}

#define GET_STR(varName, doc, fieldName, retIfInvalid)                     \
  {                                                                        \
    if (!(doc).HasMember((fieldName)) || !(doc)[(fieldName)].IsString()) { \
      LOG_W("Invalid field value of {}.", (fieldName));                    \
      return retIfInvalid;                                                 \
    }                                                                      \
    varName = (doc)[(fieldName)].GetString();                              \
  }

#define GET_STR_WITH_DFT_VAL(varName, doc, fieldName, dftVal)              \
  {                                                                        \
    if (!(doc).HasMember((fieldName)) || !(doc)[(fieldName)].IsString()) { \
      varName = dftVal;                                                    \
    } else {                                                               \
      varName = (doc)[(fieldName)].GetString();                            \
    }                                                                      \
  }

}  // namespace bq
