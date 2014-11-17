/** \file   StringUtil.h
 *  \brief  Various utility functions related to the processing of MARC-21 records.
 *  \author Dr. Johannes Ruscheinski (johannes.ruscheinski@uni-tuebingen.de)
 *
 *  \copyright 2014 Universitätsbiblothek Tübingen.  All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef STRING_UTIL_H
#define STRING_UTIL_H


#include <algorithm>
#include <string>
#include <vector>


namespace StringUtil {


/** \return True iff "s" starts with "prefix", else false. */
inline bool StartsWith(const std::string &s, const std::string &prefix) {
  if (prefix.length() > s.length())
    return false;

  return prefix == s.substr(0, prefix.length());
}


/** \return True iff "s" ends with "suffix", else false. */
inline bool EndsWith(const std::string &s, const std::string &suffix) {
  if (suffix.length() > s.length())
    return false;

  return suffix == s.substr(s.length() - suffix.length());
}


/** Pads "s" with leading "pad_char"'s if s.length() < min_length. */
std::string PadLeading(const std::string &s, const std::string::size_type min_length, const char pad_char = ' ');


/** Splits "s" on "delimiter" and returns the number of "pieces".  N.B., a `piece' can be empty if a delimiter is the
 *  first or last character in "s" or if two or more delimiters follow each other with no intervening other
 *  character.  If "s" is empty, "pieces" will contain a single empty string.
 */
size_t Split(const std::string &s, const char delimiter, std::vector<std::string> * const pieces);


/** Trims any of the characters in "trim_set" off of the end of "*s". */
std::string RightTrim(std::string * const s, const std::string &trim_set);


/** Replaces all occurrences of "original" in "*s" with "replacement". */
inline std::string Replace(std::string * const s, const char original, const char replacement) {
    std::replace(s->begin(), s->end(), original, replacement);
    return *s;
}
    

} // namespace StringUtil


#endif // ifndef STRING_UTIL_H
