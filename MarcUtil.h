/** \file   MarcUtil.h
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
#ifndef MARC_UTIL_H
#define MARC_UTIL_H


#include <string>
#include <vector>
#include <cstdio>

#include "DirectoryEntry.h"
#include "Leader.h"

    
namespace MarcUtil {


bool ReadFields(const std::string &raw_fields, const std::vector<DirectoryEntry> &dir_entries,
		std::vector<std::string> * const fields, std::string * const err_msg);


// Returns false on error and EOF.  To distinguish between the two: on EOF "err_msg" is empty but not when an
// error has been detected.  For each entry in "dir_entries" there will be a corresponding entry in "field_data".
bool ReadNextRecord(FILE * const input, Leader ** const leader, std::vector<DirectoryEntry> * const dir_entries,
		    std::vector<std::string> * const field_data, std::string * const err_msg);


// Creates a binary, a.k.a. "raw" representation of a MARC21 record.
std::string ComposeRecord(const std::vector<DirectoryEntry> &dir_entries, const std::vector<std::string> &fields,
			  Leader * const leader);


// Performs a few sanity checks.
bool RecordSeemsCorrect(const std::string &record, std::string * const err_msg);


} // namespace MarcUtil


#endif // ifndef MARC_UTIL_H
