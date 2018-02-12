//
// Created by hjzh on 18-2-11.
//
/// The file consists of a lot of string processing functions

#ifndef VIDEODETECTDEMO_STRINGUTILITY_H
#define VIDEODETECTDEMO_STRINGUTILITY_H

#include <string>
#include <vector>
namespace StringUtility {
  typedef std::string::size_type (std::string::*FuncFind)(const std::string& delim,
                                                          std::string::size_type offset) const;

  std::vector<std::string> split(const std::string& srcstr, const std::string& delimeterStr,
                                 bool removeEmpty=false, bool fullMatch=false) {
    std::vector<std::string> result;
    std::string::size_type t_startInd = 0;
    std::string::size_type t_skip = 1;

    FuncFind pFuncFind = &std::string::find_first_of;

    if (fullMatch)
    {
      t_skip = delimeterStr.length();
      pFuncFind = &std::string::find;
    }

    while (t_startInd != srcstr.npos)
    {
      std::string::size_type t_endInd = (srcstr.*pFuncFind)(delimeterStr, t_startInd);
      if (t_skip == 0) t_endInd = srcstr.npos;

      std::string t_substr = srcstr.substr(t_startInd, t_endInd - t_startInd);

      if (!(removeEmpty && t_substr.empty()))
      {
        result.push_back(std::move(t_substr));
      }

      if ((t_startInd = t_endInd) != srcstr.npos)
      {
        t_startInd += t_skip;
      }
    }

    return result;
  }

  std::string remove_the_delimiter(const std::string& srcstr, const char delimiter) {
    std::vector<std::string> t_vecSubStrs;
    std::size_t t_begin = 0;
    std::size_t t_nLenSubStr = 0;
    for (std::size_t i = 0; i < srcstr.size(); i++) {
      if (srcstr[i] == delimiter) {
        if ((t_nLenSubStr = i - t_begin) > 0) {
          t_vecSubStrs.push_back(srcstr.substr(t_begin, i - t_begin));
        }
        t_begin = i + 1;
      } else if (i == srcstr.size() - 1) {
        t_vecSubStrs.push_back(srcstr.substr(t_begin, i - t_begin + 1));
      }
    }

    // combine all substrs into one whole string without delimiter
    std::string strRes;
    for (const auto& val : t_vecSubStrs) {
      strRes += val;
    }
    return strRes;
  }
}
#endif //VIDEODETECTDEMO_STRINGUTILITY_H
