/*!
 * \file ConnMetaData.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include "ConnMetadata.hpp"

#include "WebDef.hpp"
#include "util/StdExt.hpp"

namespace bq::web {

ConnMetadata::ConnMetadata(int no, websocketpp::connection_hdl hdl,
                           std::string uri)
    : no_(no),
      createTime_(boost::posix_time::microsec_clock::universal_time()),
      activeTime_(boost::posix_time::microsec_clock::universal_time()),
      hdl_(hdl),
      status_("Connecting"),
      uri_(uri),
      server_("N/A") {}

void ConnMetadata::updateActiveTime() {
  std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
  activeTime_ = boost::posix_time::microsec_clock::universal_time();
}

void ConnMetadata::setStatus(const std::string& value) {
  std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
  status_ = value;
}

void ConnMetadata::setServer(const std::string& value) {
  std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
  server_ = value;
}

void ConnMetadata::setErrorReason(const std::string& value) {
  std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
  m_error_reason_ = value;
}

int ConnMetadata::getNo() const {
  std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
  return no_;
}

websocketpp::connection_hdl ConnMetadata::getHdl() const { return hdl_; }

std::string ConnMetadata::getStatus() const {
  std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
  return status_;
}

std::string ConnMetadata::getUri() const { return uri_; }

std::uint32_t ConnMetadata::getTimeDurOfNotRecvMsg() const {
  std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
  return getTimeDurOfNotRecvMsgImpl();
}

std::string ConnMetadata::toStr() const {
  {
    std::lock_guard<std::ext::spin_mutex> guard(mtxConnMetadata_);
    const auto ret = fmt::format(
        "["
        "No: {}; "
        "CreateTime: {}; "
        "ActiveTime: {}; "
        "TimeDurOfNotRecvMsg: {}ms; "
        "URI: {}; "
        "Status: {}; "
        "Remote Server: {}; "
        "Error/Close reason: {}"
        "]",
        no_,                                            //
        boost::posix_time::to_iso_string(createTime_),  //
        boost::posix_time::to_iso_string(activeTime_),  //
        getTimeDurOfNotRecvMsgImpl(),                   //
        uri_,                                           //
        status_,                                        //
        (server_.empty() ? "None Specified" : server_),
        (m_error_reason_.empty() ? "N/A" : m_error_reason_));
    return ret;
  }
}

std::uint32_t ConnMetadata::getTimeDurOfNotRecvMsgImpl() const {
  const auto now = boost::posix_time::microsec_clock::universal_time();
  const auto td = now - activeTime_;
  return td.total_milliseconds();
}

}  // namespace bq::web
