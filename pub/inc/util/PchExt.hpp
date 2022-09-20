/*!
 * \file PchExt.hpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#pragma once

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <xxhash.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <blockingconcurrentqueue.h>
#include <cpr/cpr.h>
#include <gflags/gflags.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <readerwriterqueue.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#define SPDLOG_FMT_EXTERNAL
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>
#include <yyjson.h>

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/assign.hpp>
#include <boost/bimap.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/config.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/convert.hpp>
#include <boost/convert/spirit.hpp>
#include <boost/crc.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/dll.hpp>
#include <boost/dll/alias.hpp>
#include <boost/dll/config.hpp>
#include <boost/dll/library_info.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/dll/shared_library_load_mode.hpp>
#include <boost/dll/smart_library.hpp>
#include <boost/filesystem.hpp>
#include <boost/generator_iterator.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/interprocess/errors.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/permissions.hpp>
#include <boost/interprocess/segment_manager.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/locale.hpp>
#include <boost/mpl/string.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/optional.hpp>
#include <boost/random.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/regex.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>
#include <boost/xpressive/xpressive_static.hpp>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/config/asio_client.hpp>
