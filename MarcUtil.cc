#include "MarcUtil.h"
#include <memory>
    
namespace MarcUtil {


bool ReadFields(const std::string &raw_fields, const std::vector<DirectoryEntry> &dir_entries,
		std::vector<std::string> * const fields, std::string * const err_msg)
{
    fields->clear();
    fields->reserve(dir_entries.size());

    if (raw_fields[raw_fields.size() - 1] != '\x1D') {
	*err_msg = "missing trailing record terminator!";
	return false;
    }

    size_t field_start(0);
    for (const auto &dir_entry : dir_entries) {
	const size_t next_field_start = field_start + dir_entry.getFieldLength();
	if (next_field_start >= raw_fields.length()) {
	    *err_msg = "misaligned field, extending past the record!";
	    return false;
	}

	const std::string field(raw_fields.substr(field_start, dir_entry.getFieldLength()));
	if (field[field.length() - 1] != '\x1E') {
	    *err_msg = "missing field terminator at end of field!";
	    return false;
	}

	fields->push_back(field.substr(0, field.length() - 1));
	field_start = next_field_start;
    }

    if (field_start + 1 != raw_fields.length()) {
	*err_msg = "field extents do not exhaust record!";
	return false;
    }

    return true;
}


// Returns false on error and EOF.  To distinguish between the two: on EOF "err_msg" is empty but not when an
// error has been detected.  For each entry in "dir_entries" there will be a corresponding entry in "field_data".
bool ReadNextRecord(FILE * const input, Leader ** const leader, std::vector<DirectoryEntry> * const dir_entries,
		    std::vector<std::string> * const field_data, std::string * const err_msg)
{
    dir_entries->clear();
    field_data->clear();
    err_msg->clear();

    //
    // Read leader.
    //

    char leader_buf[Leader::LEADER_LENGTH];
    ssize_t read_count;
    if ((read_count = std::fread(leader_buf, 1, sizeof leader_buf, input))
	!= static_cast<ssize_t>(Leader::LEADER_LENGTH))
    {
	if (read_count != 0)
	    *err_msg = "Short read for a leader or premature EOF!";
	return false;
    }

    if (not Leader::ParseLeader(std::string(leader_buf, Leader::LEADER_LENGTH), leader, err_msg)) {
	delete *leader;
	return false;
    }

    //
    // Parse directory entries.
    //

    const ssize_t directory_length((*leader)->getBaseAddressOfData() - Leader::LEADER_LENGTH);
    char directory_buf[directory_length];
    if ((read_count = std::fread(directory_buf, 1, directory_length, input)) != directory_length) {
	*err_msg = "Short read for a directory or premature EOF!";
	return false;
    }

    if (not DirectoryEntry::ParseDirEntries(std::string(directory_buf, directory_length), dir_entries, err_msg))
	return false;

    //
    // Parse variable fields.
    //

    const size_t field_data_size((*leader)->getRecordLength() - Leader::LEADER_LENGTH - directory_length);
    char raw_field_data[field_data_size];
    if ((read_count = std::fread(raw_field_data, 1, field_data_size, input))
	!= static_cast<ssize_t>(field_data_size))
	{
	    *err_msg = "Short read for field data or premature EOF! (Expected "
		+ std::to_string(field_data_size) + " bytes, got "+ std::to_string(read_count) +" bytes.)";
	    return false;
	}

    if (not ReadFields(std::string(raw_field_data, field_data_size), *dir_entries, field_data, err_msg))
	return false;

    return true;
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

    if (record.length() > 99999) {
	*err_msg = "record length (" + std::to_string(record.length())
                   + ") exceeds maxium legal record length (99999)!";
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


} // namespace MarcUtil
