/*!
 * \file PosSnapshotImpl.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/11/09
 *
 * \brief
 */

#include <boost/python.hpp>

#include "StgEngUtil.hpp"

namespace bq {

std::string handlePYErr() {
  using namespace boost::python;

  PyObject* ptype = nullptr;
  PyObject* pvalue = nullptr;
  PyObject* ptraceback = nullptr;

  // Fetch the exception information. If there was no error ptype will be set
  // to null. The other two values might set to null anyway.
  PyErr_Fetch(&ptype, &pvalue, &ptraceback);
  if (ptype == nullptr) {
    throw std::runtime_error(
        "A Python error was detected but when we called "
        "PyErr_Fetch() it returned null indicating that "
        "there was no error.");
  }

  // Sometimes pvalue is not an instance of ptype. This converts it. It's
  // done lazily for performance reasons.
  PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
  if (ptraceback != nullptr) {
    PyException_SetTraceback(pvalue, ptraceback);
  }

  // Get Boost handles to the Python objects so we get an easier API.
  handle<> htype(ptype);
  handle<> hvalue(allow_null(pvalue));
  handle<> htraceback(allow_null(ptraceback));

  // Import the `traceback` module and use it to format the exception.
  object traceback = import("traceback");
  object format_exception = traceback.attr("format_exception");
  object formatted_list = format_exception(htype, hvalue, htraceback);
  object formatted = str("\n").join(formatted_list);
  return extract<std::string>(formatted);
}

}  // namespace bq
