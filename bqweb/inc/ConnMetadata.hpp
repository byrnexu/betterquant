/*!
 * \file ConnMetadata.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include "WebDef.hpp"
#include "util/StdExt.hpp"

namespace bq::web {

class ConnMetadata {
 public:
  ConnMetadata(int no, websocketpp::connection_hdl hdl, std::string uri);

  void updateActiveTime();
  void setStatus(const std::string& value);
  void setServer(const std::string& value);
  void setErrorReason(const std::string& value);

  int getNo() const;
  websocketpp::connection_hdl getHdl() const;
  std::string getStatus() const;
  std::string getUri() const;
  std::uint32_t getTimeDurOfNotRecvMsg() const;
  std::string toStr() const;

  friend std::ostream& operator<<(std::ostream& out, ConnMetadata const& data);

 private:
  std::uint32_t getTimeDurOfNotRecvMsgImpl() const;

 private:
  int no_;
  const boost::posix_time::ptime createTime_;
  boost::posix_time::ptime activeTime_;
  const websocketpp::connection_hdl hdl_;
  std::string status_;
  const std::string uri_;
  std::string server_;
  std::string m_error_reason_;
  mutable std::ext::spin_mutex mtxConnMetadata_;
};

inline std::ostream& operator<<(std::ostream& out, ConnMetadata const& data) {
  out << data.toStr();
  return out;
}

}  // namespace bq::web
