#pragma once

#include "TDSrvRiskPluginDef.hpp"
#include "util/Pch.hpp"

namespace bq {
struct OrderInfo;
using OrderInfoSPtr = std::shared_ptr<OrderInfo>;
}  // namespace bq

namespace bq::td::srv {

class TDSrv;

class BOOST_SYMBOL_VISIBLE TDSrvRiskPlugin {
 public:
  TDSrvRiskPlugin(const TDSrvRiskPlugin&) = delete;
  TDSrvRiskPlugin& operator=(const TDSrvRiskPlugin&) = delete;
  TDSrvRiskPlugin(const TDSrvRiskPlugin&&) = delete;
  TDSrvRiskPlugin& operator=(const TDSrvRiskPlugin&&) = delete;

  explicit TDSrvRiskPlugin(TDSrv* tdSrv);

 public:
  int init(const std::string& configFilename);

 public:
  int load();

 private:
  virtual int doLoad() { return 0; }

 public:
  void unload();

 private:
  virtual void doUnload() {}

 public:
  std::string name() const { return name_; }
  bool enable() const { return enable_; }

 public:
  boost::dll::fs::path location() { return getLocation(); }

 private:
  virtual boost::dll::fs::path getLocation() const = 0;

 public:
  TDSrv* getTDSrv() const;

 public:
  int onOrder(const OrderInfoSPtr& order) { return doOnOrder(order); }

  int onCancelOrder(const OrderInfoSPtr& order) {
    return doOnCancelOrder(order);
  }

 private:
  virtual int doOnOrder(const OrderInfoSPtr& order) { return 0; }
  virtual int doOnCancelOrder(const OrderInfoSPtr& order) { return 0; }

 public:
  int onOrderRet(const OrderInfoSPtr& order) { return doOnOrderRet(order); }

  int onCancelOrderRet(const OrderInfoSPtr& order) {
    return doOnCancelOrderRet(order);
  }

 private:
  virtual int doOnOrderRet(const OrderInfoSPtr& order) { return 0; }
  virtual int doOnCancelOrderRet(const OrderInfoSPtr& order) { return 0; }

 public:
  std::string getMD5SumOfConf() const { return md5SumOfConf_; }

 private:
  TDSrv* tdSrv_;

  std::string name_;
  bool enable_{false};

  std::string md5SumOfConf_;

 protected:
  std::shared_ptr<spdlog::async_logger> logger_;
};

struct LibraryHoldingDeleter {
  std::shared_ptr<boost::dll::shared_library> lib_;
  void operator()(TDSrvRiskPlugin* p) const { delete p; }
};

inline std::shared_ptr<TDSrvRiskPlugin> bind(TDSrvRiskPlugin* plugin) {
  boost::dll::fs::path location = plugin->location();
  std::shared_ptr<boost::dll::shared_library> lib =
      std::make_shared<boost::dll::shared_library>(location);

  LibraryHoldingDeleter deleter;
  deleter.lib_ = lib;

  return std::shared_ptr<TDSrvRiskPlugin>(plugin, deleter);
}

inline std::shared_ptr<TDSrvRiskPlugin> GetPlugin(boost::dll::fs::path path,
                                                  const char* funcName,
                                                  TDSrv* tdSrv) {
  typedef TDSrvRiskPlugin*(Func)(TDSrv*);
  boost::function<Func> creator = boost::dll::import_alias<Func>(
      path, funcName, boost::dll::load_mode::append_decorations);

  TDSrvRiskPlugin* plugin = creator(tdSrv);
  return bind(plugin);
}

}  // namespace bq::td::srv
