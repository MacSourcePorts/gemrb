/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "2DAImporter.h"

#include "Interface.h"
#include "Streams/FileStream.h"

using namespace GemRB;

#define SIGNLENGTH 256      //if a 2da has longer default value, change this

static bool StringCompKey(const std::string& str, TableMgr::key_t key)
{
	return stricmp(str.c_str(), key.c_str()) == 0;
}

p2DAImporter::p2DAImporter() noexcept
{
	colNames.reserve(10);
	rowNames.reserve(10);
	rows.reserve(10);
}

bool p2DAImporter::Open(DataStream* str)
{
	if (str == NULL) {
		return false;
	}
	char Signature[SIGNLENGTH];
	str->CheckEncrypted();

	str->ReadLine( Signature, sizeof(Signature) );
	const char* strp = Signature;
	while (*strp == ' ')
		strp++;
	if (strncmp( strp, "2DA V1.0", 8 ) != 0) {
		Log(WARNING, "2DAImporter", "Bad signature ({})! Complaining, but not ignoring...", str->filename);
		// we don't care about this, so exptable.2da of iwd2 won't cause a bigger problem
		// also, certain creatures are described by 2da's without signature.
		// return false;
	}
	str->ReadLine( Signature, sizeof(Signature) );
	const char* token = strtok(Signature, " ");
	if (token) {
		defVal = token;
	} else { // no whitespace
		defVal = Signature;
	}
	bool colHead = true;
	int row = 0;
	
	constexpr int MAXLENGTH = 8192;
	char buffer[MAXLENGTH]; // we can increase this if needed, but beware since it is a stack buffer
	while (true) {
		strret_t len = str->ReadLine(buffer, MAXLENGTH);
		if (len <= 0) {
			break;
		}
		if (buffer[0] == '#') { // allow comments
			continue;
		}

		if (colHead) {
			colHead = false;
			const char* cell = strtok(buffer, " ");
			while (cell != nullptr) {
				colNames.emplace_back(cell);
				cell = strtok(nullptr, " ");
			}
		} else {
			char* line = buffer;
			const char* cell = strtok(line, " ");
			if (cell == nullptr) continue;

			rowNames.emplace_back(cell);
			rows.emplace_back();
			rows[row].reserve(10);
			cell = strtok(nullptr, " ");
			while (cell != nullptr) {
				rows[row].emplace_back(cell);
				cell = strtok(nullptr, " ");
			}
			row++;
		}
	}

	delete str;
	assert(rows.size() < std::numeric_limits<index_t>::max());
	return true;
}

/** Returns the actual number of Rows in the Table */
p2DAImporter::index_t p2DAImporter::GetRowCount() const
{
	return static_cast<index_t>(rows.size());
}

p2DAImporter::index_t p2DAImporter::GetColNamesCount() const
{
	return static_cast<index_t>(colNames.size());
}

/** Returns the actual number of Columns in the Table */
p2DAImporter::index_t p2DAImporter::GetColumnCount(index_t row) const
{
	if (rows.size() <= row) {
		return 0;
	}
	return static_cast<index_t>(rows[row].size());
}
/** Returns a pointer to a zero terminated 2da element,
	if it cannot return a value, it returns the default */
const std::string& p2DAImporter::QueryField(index_t row, index_t column) const
{
	if (rows.size() <= row) {
		return defVal;
	}
	if (rows[row].size() <= column) {
		return defVal;
	}
	if (rows[row][column][0]=='*' && !rows[row][column][1]) {
		return defVal;
	}
	return rows[row][column];
}

const std::string& p2DAImporter::QueryDefault() const
{
	return defVal;
}

p2DAImporter::index_t p2DAImporter::GetRowIndex(const key_t& key) const
{
	for (index_t index = 0; index < rowNames.size(); index++) {
		if (StringCompKey(rowNames[index], key)) {
			return index;
		}
	}
	return npos;
}

p2DAImporter::index_t p2DAImporter::GetColumnIndex(const key_t& key) const
{
	for (index_t index = 0; index < colNames.size(); index++) {
		if (StringCompKey(colNames[index], key)) {
			return index;
		}
	}
	return npos;
}

const static std::string blank;
const std::string& p2DAImporter::GetColumnName(index_t index) const
{
	if (index < colNames.size()) {
		return colNames[index];
	}
	return blank;
}

const std::string& p2DAImporter::GetRowName(index_t index) const
{
	if (index < rowNames.size()) {
		return rowNames[index];
	}
	return blank;
}

p2DAImporter::index_t p2DAImporter::FindTableValue(index_t col, long val, index_t start) const
{
	index_t max = GetRowCount();
	for (index_t row = start; row < max; row++) {
		const std::string& ret = QueryField( row, col );
		long Value;
		if (valid_signednumber(ret.c_str(), Value) && (Value == val))
			return row;
	}
	return npos;
}

p2DAImporter::index_t p2DAImporter::FindTableValue(index_t col, const key_t& val, index_t start) const
{
	index_t max = GetRowCount();
	for (index_t row = start; row < max; row++) {
		const std::string& ret = QueryField( row, col );
		if (StringCompKey(ret, val))
			return row;
	}
	return npos;
}

p2DAImporter::index_t p2DAImporter::FindTableValue(const key_t& col, long val, index_t start) const
{
	index_t coli = GetColumnIndex(col);
	return FindTableValue(coli, val, start);
}

p2DAImporter::index_t p2DAImporter::FindTableValue(const key_t& col, const key_t& val, index_t start) const
{
	index_t coli = GetColumnIndex(col);
	return FindTableValue(coli, val, start);
}

#include "plugindef.h"

GEMRB_PLUGIN(0xB22F938, "2DA File Importer")
PLUGIN_CLASS(IE_2DA_CLASS_ID, p2DAImporter)
END_PLUGIN()
