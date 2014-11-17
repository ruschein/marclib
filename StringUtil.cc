#include "StringUtil.h"


namespace StringUtil {


std::string PadLeading(const std::string &s, const std::string::size_type min_length, const char pad_char) {
    const std::string::size_type length(s.length());
    if (length >= min_length)
	return s;

    return std::string(min_length - length, pad_char) + s;
}


size_t Split(const std::string &s, const char delimiter, std::vector<std::string> * const pieces) {
    pieces->clear();

    size_t start_pos(0);
    size_t next_delim_pos = s.find(delimiter, start_pos);
    while (next_delim_pos != std::string::npos) {
	pieces->push_back(s.substr(start_pos, next_delim_pos - start_pos));
	start_pos = next_delim_pos + 1;
	next_delim_pos = s.find(delimiter, start_pos);
    }

    pieces->push_back(s.substr(start_pos));

    return pieces->size();
}


std::string RightTrim(std::string * const s, const std::string &trim_set)
{
    ssize_t pos(s->size());
    while (pos > 0 and trim_set.find((*s)[pos - 1]) != std::string::npos)
	--pos;

    s->resize(pos);
    return *s;
}
    

} // namespace StringUtil
