#pragma once

#include "db/DBE.hpp"
#include "def/Def.hpp"
#include "util/Json.hpp"
#include "util/Logger.hpp"
#include "util/Pch.hpp"
#include "util/Random.hpp"

namespace bq::db {

template <typename TableSchema>
class TBLRec {
 public:
  using RecWithKeyFieldsSPtr = std::shared_ptr<typename TableSchema::KeyFields>;
  using RecWithValFieldsSPtr = std::shared_ptr<typename TableSchema::ValFields>;
  using RecWithAllFieldsSPtr = std::shared_ptr<typename TableSchema::AllFields>;

 public:
  TBLRec() { initMember(); }

  TBLRec(RecWithAllFieldsSPtr& recWithAllFields) {
    const auto doc = ConvertStructToDoc(*recWithAllFields);
    initByDocOfAllFields(doc);
  }

  TBLRec(const Doc& doc) { initByDocOfAllFields(doc); }

 private:
  void initMember() {
    recWithKeyFields_ = std::make_shared<typename TableSchema::KeyFields>();
    recWithValFields_ = std::make_shared<typename TableSchema::ValFields>();
    recWithAllFields_ = std::make_shared<typename TableSchema::AllFields>();
    docOfKeyFields_ = std::make_shared<Doc>();
    docOfValFields_ = std::make_shared<Doc>();
    docOfAllFields_ = std::make_shared<Doc>();
    fieldNameGroupOfKey_ = GetMemberNameFromStruct(*recWithKeyFields_);
    fieldNameGroupOfVal_ = GetMemberNameFromStruct(*recWithValFields_);
    fieldNameGroupOfAll_ = GetMemberNameFromStruct(*recWithAllFields_);
  }

  void initByDocOfAllFields(const Doc& doc) {
    auto fieldNameGroupContains =
        [](const std::vector<std::string>& fieldNameGroup,
           const std::string& fieldName) {
          const auto iter = std::find(std::begin(fieldNameGroup),
                                      std::end(fieldNameGroup), fieldName);
          if (iter == std::end(fieldNameGroup)) {
            return false;
          } else {
            return true;
          }
        };

    initMember();

    auto docOfKeyFields = std::make_shared<Doc>();
    docOfKeyFields->SetObject();
    auto docOfValFields = std::make_shared<Doc>();
    docOfValFields->SetObject();
    auto docOfAllFields = std::make_shared<Doc>();
    docOfAllFields->SetObject();

    auto iter = doc.MemberBegin();
    for (; iter != doc.MemberEnd(); ++iter) {
      const auto fieldName = iter->name.GetString();
      if (fieldNameGroupContains(fieldNameGroupOfKey_, fieldName)) {
        Val key(fieldName, docOfKeyFields->GetAllocator());
        Val value(doc[fieldName], docOfKeyFields->GetAllocator());
        docOfKeyFields->AddMember(key, value, docOfKeyFields->GetAllocator());
      }

      if (fieldNameGroupContains(fieldNameGroupOfVal_, fieldName)) {
        Val key(fieldName, docOfValFields->GetAllocator());
        Val value(doc[fieldName], docOfValFields->GetAllocator());
        docOfValFields->AddMember(key, value, docOfValFields->GetAllocator());
      }

      if (fieldNameGroupContains(fieldNameGroupOfAll_, fieldName)) {
        Val key(fieldName, docOfAllFields->GetAllocator());
        Val value(doc[fieldName], docOfAllFields->GetAllocator());
        docOfAllFields->AddMember(key, value, docOfAllFields->GetAllocator());
      }
    }

    setDocOfKeyFields(docOfKeyFields);
    setDocOfValFields(docOfValFields);
    setDocOfAllFields(docOfAllFields);

    setJsonStrOfKeyFields(bq::ConvertDocToJsonStr(*docOfKeyFields));
    setJsonStrOfValFields(bq::ConvertDocToJsonStr(*docOfValFields));
    setJsonStrOfAllFields(bq::ConvertDocToJsonStr(*docOfAllFields));

    const auto recWithKeyFields =
        ConvertJsonStrToStruct<typename TableSchema::KeyFields>(
            getJsonStrOfKeyFields());
    const auto recWithValFields =
        ConvertJsonStrToStruct<typename TableSchema::ValFields>(
            getJsonStrOfValFields());
    const auto recWithAllFields =
        ConvertJsonStrToStruct<typename TableSchema::AllFields>(
            getJsonStrOfAllFields());

    setRecWithKeyFields(
        std::make_shared<typename TableSchema::KeyFields>(recWithKeyFields));
    setRecWithValFields(
        std::make_shared<typename TableSchema::ValFields>(recWithValFields));
    setRecWithAllFields(
        std::make_shared<typename TableSchema::AllFields>(recWithAllFields));
  }

 public:
  std::string getSqlOfInsert() const {
    std::string sep = "";
    std::string fieldNamePart = "";
    std::string fieldValuePart = "";
    auto iter = docOfAllFields_->MemberBegin();
    for (; iter != docOfAllFields_->MemberEnd(); ++iter) {
      const auto fieldName = iter->name.GetString();
      const auto& fieldValue = (*docOfAllFields_)[fieldName];
      fieldNamePart = fieldNamePart + sep + "`" + fieldName + "`";
      fieldValuePart = fieldValuePart + sep + getValInStrFmt(fieldValue);
      sep = ", ";
    }
    const auto ret =
        fmt::format("INSERT INTO {}({}) VALUES({});", TableSchema::TableName,
                    fieldNamePart, fieldValuePart);
    return ret;
  }

  std::string getSqlOfInsertIgnore() const {
    std::string sep = "";
    std::string fieldNamePart = "";
    std::string fieldValuePart = "";
    auto iter = docOfAllFields_->MemberBegin();
    for (; iter != docOfAllFields_->MemberEnd(); ++iter) {
      const auto fieldName = iter->name.GetString();
      const auto& fieldValue = (*docOfAllFields_)[fieldName];
      fieldNamePart = fieldNamePart + sep + "`" + fieldName + "`";
      fieldValuePart = fieldValuePart + sep + getValInStrFmt(fieldValue);
      sep = ", ";
    }
    const auto ret =
        fmt::format("INSERT IGNORE INTO {}({}) VALUES({});",
                    TableSchema::TableName, fieldNamePart, fieldValuePart);
    return ret;
  }

  std::string getSqlOfReplace() const {
    std::string sep = "";
    std::string fieldNamePart = "";
    std::string fieldValuePart = "";
    auto iter = docOfAllFields_->MemberBegin();
    for (; iter != docOfAllFields_->MemberEnd(); ++iter) {
      const auto fieldName = iter->name.GetString();
      const auto& fieldValue = (*docOfAllFields_)[fieldName];
      fieldNamePart = fieldNamePart + sep + "`" + fieldName + "`";
      fieldValuePart = fieldValuePart + sep + getValInStrFmt(fieldValue);
      sep = ", ";
    }
    const auto ret =
        fmt::format("REPLACE INTO {}({}) VALUES({});", TableSchema::TableName,
                    fieldNamePart, fieldValuePart);
    return ret;
  }

  std::string getSqlOfDelete(
      OnlyModifyIsDel onlyModifyIsDel = OnlyModifyIsDel::True) {
    std::string sep = "";
    std::string condPart = "";
    auto iter = docOfKeyFields_->MemberBegin();
    for (; iter != docOfKeyFields_->MemberEnd(); ++iter) {
      const auto fieldName = iter->name.GetString();
      const auto& fieldValue = (*docOfKeyFields_)[fieldName];
      condPart = fmt::format("{}{}`{}` = {}", condPart, sep, fieldName,
                             getValInStrFmt(fieldValue));
      sep = " AND ";
    }
    std::string ret;
    if (onlyModifyIsDel == OnlyModifyIsDel::True) {
      condPart = fmt::format("{}{} `isDel` = 0", condPart, sep);
      ret = fmt::format("UPDATE {} SET `isDel` = 1 WHERE {};",
                        TableSchema::TableName, condPart);
    } else {
      ret = fmt::format("DELETE FROM {} WHERE {};", TableSchema::TableName,
                        condPart);
    }
    return ret;
  }

  std::string getSqlOfUpdate() {
    std::string sep = "";
    std::string condPart = "";
    auto iter = docOfKeyFields_->MemberBegin();
    for (; iter != docOfKeyFields_->MemberEnd(); ++iter) {
      const auto fieldName = iter->name.GetString();
      const auto& fieldValue = (*docOfKeyFields_)[fieldName];
      condPart = fmt::format("{}{}`{}` = {}", condPart, sep, fieldName,
                             getValInStrFmt(fieldValue));
      sep = " AND ";
    }

    sep = "";
    std::string updatePart = "";
    iter = docOfValFields_->MemberBegin();
    for (; iter != docOfValFields_->MemberEnd(); ++iter) {
      const auto fieldName = iter->name.GetString();
      const auto& fieldValue = (*docOfValFields_)[fieldName];
      updatePart = fmt::format("{}{}`{}` = {}", updatePart, sep, fieldName,
                               getValInStrFmt(fieldValue));
      sep = ", ";
    }

    const auto ret = fmt::format("UPDATE {} SET {} WHERE {};",
                                 TableSchema::TableName, updatePart, condPart);
    return ret;
  }

 private:
  std::string getValInStrFmt(const Val& v) const {
    std::string ret;
    if (v.IsString()) {
      ret = fmt::format("\"{}\"", v.GetString());
    } else if (v.IsInt()) {
      ret = fmt::format("{}", v.GetInt());
    } else if (v.IsUint()) {
      ret = fmt::format("{}", v.GetUint());
    } else if (v.IsInt64()) {
      ret = fmt::format("{}", v.GetInt64());
    } else if (v.IsUint64()) {
      ret = fmt::format("{}", v.GetUint64());
    } else if (v.IsDouble()) {
      ret = fmt::format("{}", v.GetDouble());
    } else {
      LOG_W(
          "Get value in str fmt failed because the current type is not "
          "supported.");
    }
    return ret;
  }

 public:
  auto getRecWithKeyFields() const { return recWithKeyFields_; }
  auto getRecWithValFields() const { return recWithValFields_; }
  auto getRecWithAllFields() const { return recWithAllFields_; }

  auto getJsonStrOfKeyFields() const { return jsonStrOfKeyFields_; }
  auto getJsonStrOfValFields() const { return jsonStrOfValFields_; }
  auto getJsonStrOfAllFields() const { return jsonStrOfAllFields_; }

  auto getDocOfKeyFields() const { return docOfKeyFields_; }
  auto getDocOfValFields() const { return docOfValFields_; }
  auto getDocOfAllFields() const { return docOfAllFields_; }

  auto getFieldNameGroupOfKey() const { return fieldNameGroupOfKey_; }
  auto getFieldNameGroupOfVal() const { return fieldNameGroupOfVal_; };
  auto getFieldNameGroupOfAll() const { return fieldNameGroupOfAll_; };

  void setRecWithKeyFields(const RecWithKeyFieldsSPtr& value) {
    recWithKeyFields_ = value;
  }
  void setRecWithValFields(const RecWithValFieldsSPtr& value) {
    recWithValFields_ = value;
  }
  void setRecWithAllFields(const RecWithAllFieldsSPtr& value) {
    recWithAllFields_ = value;
  }

  void setJsonStrOfKeyFields(const std::string& value) {
    jsonStrOfKeyFields_ = value;
  }
  void setJsonStrOfValFields(const std::string& value) {
    jsonStrOfValFields_ = value;
  }
  void setJsonStrOfAllFields(const std::string& value) {
    jsonStrOfAllFields_ = value;
  }

  void setDocOfKeyFields(const DocSPtr& value) { docOfKeyFields_ = value; }
  void setDocOfValFields(const DocSPtr& value) { docOfValFields_ = value; }
  void setDocOfAllFields(const DocSPtr& value) { docOfAllFields_ = value; }

 private:
  RecWithKeyFieldsSPtr recWithKeyFields_{nullptr};
  RecWithValFieldsSPtr recWithValFields_{nullptr};
  RecWithAllFieldsSPtr recWithAllFields_{nullptr};

  std::string jsonStrOfKeyFields_{""};
  std::string jsonStrOfValFields_{""};
  std::string jsonStrOfAllFields_{""};

  DocSPtr docOfKeyFields_{nullptr};
  DocSPtr docOfValFields_{nullptr};
  DocSPtr docOfAllFields_{nullptr};

  std::vector<std::string> fieldNameGroupOfKey_;
  std::vector<std::string> fieldNameGroupOfVal_;
  std::vector<std::string> fieldNameGroupOfAll_;
};

template <typename TableSchema>
using TBLRecSPtr = std::shared_ptr<TBLRec<TableSchema>>;

template <typename TableSchema>
using TBLRecSet = std::map<std::string, TBLRecSPtr<TableSchema>>;

template <typename TableSchema>
using TBLRecSetSPtr = std::shared_ptr<TBLRecSet<TableSchema>>;

template <typename TableSchema>
class TBLRecSetMaker {
 public:
  static std::tuple<int, TBLRecSetSPtr<TableSchema>> ExecSql(
      const DBEngSPtr& dbEng, const std::string& sql) {
    const auto identity = GET_RAND_STR();
    auto [ret, execRetOfSql] = dbEng->syncExec(identity, sql);
    if (ret != 0) {
      LOG_W("Exec sql failed. {}", sql);
      return {-1, nullptr};
    }
    return Parse(execRetOfSql);
  }

 private:
  static std::tuple<int, TBLRecSetSPtr<TableSchema>> Parse(
      const std::string& execRetOfSql) {
    auto ret = std::make_shared<TBLRecSet<TableSchema>>();

    auto doc = std::make_shared<Doc>();
    doc->Parse(execRetOfSql.data());
    if ((*doc)["recordSetGroup"].Size() == 0) {
      return {0, ret};
    }

    const auto statusCode = (*doc)["statusCode"].GetInt();
    if (statusCode != 0) {
      const std::string statusMsg = (*doc)["statusMsg"].GetString();
      LOG_W("Make tbl rec set failed because of exec sql failed. [{} - {}]",
            statusCode, statusMsg);
      return {-1, nullptr};
    }

    for (std::size_t i = 0; i < (*doc)["recordSetGroup"][0].Size(); ++i) {
      const auto record = ConvertValToDoc((*doc)["recordSetGroup"][0][i]);
      const auto tblRec = std::make_shared<TBLRec<TableSchema>>(record);
      ret->emplace(tblRec->getJsonStrOfKeyFields(), tblRec);
    }

    return {0, ret};
  }
};

template <typename TableSchema>
inline std::tuple<TBLRecSetSPtr<TableSchema>, TBLRecSetSPtr<TableSchema>,
                  TBLRecSetSPtr<TableSchema>>
TBLRecSetCompare(const TBLRecSetSPtr<TableSchema>& newTBLRecSet,
                 const TBLRecSetSPtr<TableSchema>& oldTBLRecSet) {
  auto tblRecSetAdd = std::make_shared<TBLRecSet<TableSchema>>();
  auto tblRecSetDel = std::make_shared<TBLRecSet<TableSchema>>();
  auto tblRecSetChg = std::make_shared<TBLRecSet<TableSchema>>();

  for (const auto& newTBLRec : *newTBLRecSet) {
    if (const auto iter = oldTBLRecSet->find(newTBLRec.first);
        iter == std::end(*oldTBLRecSet)) {
      const auto tblRec = std::make_shared<TBLRec<TableSchema>>(
          *newTBLRec.second->getDocOfAllFields());
      tblRecSetAdd->emplace(newTBLRec.first, tblRec);
      if (!oldTBLRecSet->empty()) {
        LOG_D("Find new tbl rec. new rec = {}",
              newTBLRec.second->getJsonStrOfAllFields());
      }
    } else {
      const auto newValue = newTBLRec.second->getJsonStrOfValFields();
      const auto oldValue = iter->second->getJsonStrOfValFields();
      if (newValue != oldValue) {
        const auto tblRec = std::make_shared<TBLRec<TableSchema>>(
            *newTBLRec.second->getDocOfAllFields());
        tblRecSetChg->emplace(newTBLRec.first, tblRec);
        const auto diff = DiffOfJson(oldValue, newValue);
        LOG_D("Find tbl rec changed. key = {} diff = {}",
              newTBLRec.second->getJsonStrOfKeyFields(), diff);
      }
    }
  }

  for (const auto& oldTBLRec : *oldTBLRecSet) {
    if (newTBLRecSet->find(oldTBLRec.first) == std::end(*newTBLRecSet)) {
      const auto tblRec = std::make_shared<TBLRec<TableSchema>>(
          *oldTBLRec.second->getDocOfAllFields());
      tblRecSetDel->emplace(oldTBLRec.first, tblRec);
      LOG_D("Find tbl rec deleted. old rec = {}",
            oldTBLRec.second->getJsonStrOfAllFields());
    }
  }

  return {tblRecSetAdd, tblRecSetDel, tblRecSetChg};
}

template <typename TableSchema>
int ExecRecInsert(const DBEngSPtr& dbEng,
                  const TBLRecSPtr<TableSchema>& tblRec) {
  const auto identity = GET_RAND_STR();
  const auto sql = tblRec->getSqlOfInsert();
  auto [ret, execRet] = dbEng->syncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Insert rec to table failed. [identity = {}] [sql = {}]", identity,
          sql);
    return ret;
  }
  return 0;
}

template <typename TableSchema>
void ExecTBLInsert(const DBEngSPtr& dbEng,
                   const TBLRecSetSPtr<TableSchema>& tblRecSet) {
  for (const auto& rec : *tblRecSet) {
    ExecRecInsert(dbEng, rec.second);
  }
}

template <typename TableSchema>
int ExecRecInsertIgnore(const DBEngSPtr& dbEng,
                        const TBLRecSPtr<TableSchema>& tblRec) {
  const auto identity = GET_RAND_STR();
  const auto sql = tblRec->getSqlOfInsertIgnore();
  auto [ret, execRet] = dbEng->syncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Insert ignore rec to table failed. [identity = {}] [sql = {}]",
          identity, sql);
    return ret;
  }
  return 0;
}

template <typename TableSchema>
void ExecTBLInsertIgnore(const DBEngSPtr& dbEng,
                         const TBLRecSetSPtr<TableSchema>& tblRecSet) {
  for (const auto& rec : *tblRecSet) {
    ExecRecInsertIgnore(dbEng, rec.second);
  }
}

template <typename TableSchema>
int ExecRecReplace(const DBEngSPtr& dbEng,
                   const TBLRecSPtr<TableSchema>& tblRec) {
  const auto identity = GET_RAND_STR();
  const auto sql = tblRec->getSqlOfReplace();
  auto [ret, execRet] = dbEng->syncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Replace rec to table failed. [identity = {}] [sql = {}]", identity,
          sql);
    return ret;
  }
  return 0;
}

template <typename TableSchema>
void ExecTBLReplace(const DBEngSPtr& dbEng,
                    const TBLRecSetSPtr<TableSchema>& tblRecSet) {
  for (const auto& rec : *tblRecSet) {
    ExecRecReplace(dbEng, rec.second);
  }
}

template <typename TableSchema>
int ExecRecDelete(const DBEngSPtr& dbEng, const TBLRecSPtr<TableSchema>& tblRec,
                  OnlyModifyIsDel onlyModifyIsDel = OnlyModifyIsDel::True) {
  const auto identity = GET_RAND_STR();
  const auto sql = tblRec->getSqlOfDelete(onlyModifyIsDel);
  auto [ret, execRet] = dbEng->syncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Delete rec from table failed. [identity = {}] [sql = {}]", identity,
          sql);
    return ret;
  }
  return 0;
}

template <typename TableSchema>
void ExecTBLDelete(const DBEngSPtr& dbEng,
                   const TBLRecSetSPtr<TableSchema>& tblRecSet,
                   OnlyModifyIsDel onlyModifyIsDel = OnlyModifyIsDel::True) {
  for (const auto& rec : *tblRecSet) {
    ExecRecDelete(dbEng, rec.second, onlyModifyIsDel);
  }
}

template <typename TableSchema>
int ExecRecUpdate(const DBEngSPtr& dbEng,
                  const TBLRecSPtr<TableSchema>& tblRec) {
  const auto identity = GET_RAND_STR();
  const auto sql = tblRec->getSqlOfUpdate();
  auto [ret, execRet] = dbEng->syncExec(identity, sql);
  if (ret != 0) {
    LOG_W("Update rec of table failed. [identity = {}] [sql = {}]", identity,
          sql);
    return ret;
  }
  return 0;
}

template <typename TableSchema>
void ExecTBLUpdate(const DBEngSPtr& dbEng,
                   const TBLRecSetSPtr<TableSchema>& tblRecSet) {
  for (const auto& rec : *tblRecSet) {
    ExecRecUpdate(dbEng, rec.second);
  }
}

}  // namespace bq::db
