/** \file marc_grep.cc
 *  \brief marc_grep is a command-line utility for the extration of field and subfield values from MARC-21 records.
 *
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
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include "DirectoryEntry.h"
#include "Leader.h"
#include "MarcUtil.h"
#include "RegexMatcher.h"
#include "StringUtil.h"
#include "Subfields.h"
#include "util.h"


void Usage() {
    std::cerr << "Usage: " << progname << " input_filename field_reference\n";
    std::cerr << "\tField references are a mixed colon-separated list of either field codes like \"712\" or\n";
    std::cerr << "\tfield codes followed by one or more subfield codes like \"859aw\".\n";
    std::exit(EXIT_FAILURE);
}


void FieldGrep(const std::string &input_filename, std::string pattern) {
    FILE *input = std::fopen(input_filename.c_str(), "rb");
    if (input == NULL)
	Error("can't open \"" + input_filename + "\" for reading!");

    // Do we have a leader filter?
    unsigned offset;
    char leader_match('\0');
    if (pattern[0] == 'L') {
	if (std::sscanf(pattern.c_str(), "L[%u]=%c;", &offset, &leader_match) != 2 or leader_match == ';')
	    Error("Bad leader match specification!");
	if (offset >= Leader::LEADER_LENGTH)
	    Error("Leader match offset exceeds leader length (" + std::to_string(Leader::LEADER_LENGTH) + ")!");
	const std::string::size_type closing_brace_pos = pattern.find(']');
	if (closing_brace_pos + 4 > pattern.length() or pattern[closing_brace_pos + 3] != ';')
	    Error("Missing ';' after leader match specification!");
	pattern = pattern.substr(closing_brace_pos + 4);
    }

    std::string field_tag;
    std::string subfield_codes;
    if (not pattern.empty()) {
	if (pattern.length() < 3)
	    Error("Bad field pattern \"" + pattern + "\", must be at least 3 characters in length!");
	field_tag = pattern.substr(0, 3);
	subfield_codes = pattern.substr(3);
    }

    Leader *raw_leader;
    std::vector<DirectoryEntry> dir_entries;
    std::vector<std::string> field_data;
    std::string err_msg;
    unsigned count(0), matched_count(0);

    while (MarcUtil::ReadNextRecord(input, &raw_leader, &dir_entries, &field_data, &err_msg)) {
	++count;
	std::unique_ptr<Leader> leader(raw_leader);
	if (leader_match != '\0') {
	    if ((*leader)[offset] != leader_match)
		continue;
	    else if (field_tag.empty()) {
		++matched_count;
		continue;
	    }
	}

	std::string control_number;
	for (unsigned i(0); i < dir_entries.size(); ++i) {
	    if (dir_entries[i].getTag() == "001")
		control_number = field_data[i];

	    if (dir_entries[i].getTag() == field_tag) {
		bool matched(false);
		if (subfield_codes.empty()) {
		    std::cout << field_data[i] << "\n";
		    matched = true;
		} else {
		    const Subfields subfields(field_data[i]);
		    for (const char subfield_code : subfield_codes) {
			auto begin_end = subfields.getIterators(subfield_code);
			for (auto code_and_value(begin_end.first); code_and_value != begin_end.second;
			     ++code_and_value)
			{
			    matched = true;
			    std::cout << control_number << ':' << subfield_code << ':'
				      << code_and_value->second << '\n';
			}
		    }
		}

		if (matched)
		    ++matched_count;
		break;
	    }
	}
    }

    if (not err_msg.empty())
	Error(err_msg);
    std::cerr << "Matched " << matched_count << " records of " << count << " overall records.\n";

    std::fclose(input);
}

// Creates a binary, a.k.a. "raw" representation of a MARC21 record.
std::string ComposeRecord(const std::vector<DirectoryEntry> &dir_entries, const std::vector<std::string> &fields,
			  Leader * const leader)
{
    size_t record_size(Leader::LEADER_LENGTH);
    const size_t directory_size(dir_entries.size() * DirectoryEntry::DIRECTORY_ENTRY_LENGTH);
    record_size += directory_size;
    ++record_size; // field terminator
    for (const auto &dir_entry : dir_entries)
	record_size += dir_entry.getFieldLength();
    ++record_size; // record terminator

    leader->setRecordLength(record_size);
    leader->setBaseAddressOfData(Leader::LEADER_LENGTH + directory_size + 1);

    std::string record;
    record.reserve(record_size);
    record += leader->toString();
    for (const auto &dir_entry : dir_entries)
        record +=  dir_entry.toString();
    record += '\x1E';
    for (const auto &field : fields) {
	record += field;
	record += '\x1E';
    }
    record += '\x1D';

    return record;
}


// Performs a few sanity checks.
bool RecordSeemsCorrect(const std::string &record, std::string * const err_msg) {
    if (record.size() < Leader::LEADER_LENGTH) {
	*err_msg = "record too small to contain leader!";
	return false;
    }

    Leader *raw_leader;
    if (not Leader::ParseLeader(record.substr(0, Leader::LEADER_LENGTH), &raw_leader, err_msg))
	return false;
    const std::unique_ptr<Leader> leader(raw_leader);

    if (leader->getRecordLength() != record.length()) {
	*err_msg = "leader's record length (" + std::to_string(leader->getRecordLength())
	           + ") does not equal actual record length (" + std::to_string(record.length()) + ")!";
	return false;
    }

    if (leader->getBaseAddressOfData() <= Leader::LEADER_LENGTH) {
	*err_msg = "impossible base address of data!";
	return false;
    }

    const size_t directory_length(leader->getBaseAddressOfData() - Leader::LEADER_LENGTH - 1);
    if ((directory_length % DirectoryEntry::DIRECTORY_ENTRY_LENGTH) != 0) {
	*err_msg = "directory length is not a multiple of "
	           + std::to_string(DirectoryEntry::DIRECTORY_ENTRY_LENGTH) + "!";
	return false;
    }

    if (record[leader->getBaseAddressOfData() - 1] != '\x1E') {
	*err_msg = "directory is not terminated with a field terminator!";
	return false;
    }

    if (record[record.size() - 1] != '\x1D') {
	*err_msg = "record is not terminated with a record terminator!";
        return false;
    }
    
    return true;
}


int main(int argc, char **argv) {
    progname = argv[0];

    if (argc != 3)
	Usage();

    FieldGrep(argv[1], argv[2]);
}
