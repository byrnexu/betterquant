#pragma once

#include "SubAndUnSubSvc.hpp"

namespace bq::md::svc::binance {

class SubAndUnSubSvcOfBinance : public SubAndUnSubSvc {
 public:
  using SubAndUnSubSvc::SubAndUnSubSvc;

 private:
  std::tuple<WSReqGroup, WSReqGroup> convertTopicToWSReq(
      TopicGroupNeedMaintSPtr& topicGroupNeedMaint) final;

  WSReqGroup makeReqGroup(TopicGroupNeedMaintSPtr& topicGroupNeedMaint,
                          TopicOP topicOP);

  std::tuple<int, std::string, std::string> GetExchSymAndMDType(
      const std::string& topic) const;

 private:
  std::uint64_t id_{0};
};

}  // namespace bq::md::svc::binance
