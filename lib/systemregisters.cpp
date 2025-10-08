/*
 * Copyright 2016-2021 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is part of Tarmac Trace Utilities
 */

#include "libtarmac/registers.hh"
#include "libtarmac/misc.hh"

#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "libtarmac/third_party/rapidcsv.h"

using std::dec;
using std::ostream;
using std::ostringstream;
using std::string;

struct RegPrefixInfo {
    const char *name;
    size_t namelen;
    unsigned size, disp, n, offset;
};

#define MAKE_REGOFFSET_ENUM_ADVANCE(id, size, disp, n)                         \
    offset_##id, last_##id = offset_##id + disp * n - 1,
#define MAKE_REGOFFSET_ENUM_NOADVANCE(id, size, disp, n)                       \
    offset_##id, last_##id = offset_##id - 1,
enum {
    REGPREFIXLIST(MAKE_REGOFFSET_ENUM_ADVANCE, MAKE_REGOFFSET_ENUM_NOADVANCE)
};
#undef MAKE_REGOFFSET_ENUM

#define NO_OFFSET UINT_MAX

#define MAKE_REGPREFIX_INFO(id, size, disp, n)                                 \
    {#id, sizeof(#id) - 1, size, disp, n, offset_##id},
static const RegPrefixInfo reg_prefixes[] = {
    REGPREFIXLIST(MAKE_REGPREFIX_INFO, MAKE_REGPREFIX_INFO)};
#undef MAKE_REGPREFIX_INFO

static inline bool compare(const char *a, size_t alen, const char *b,
                           size_t blen)
{
    return alen == blen && !strncasecmp(a, b, alen);
}

#define STRINGWITHLEN(s) s, sizeof(s) - 1

std::vector <std::string> sysreg_names{};

bool read_sysreg_names_csv(string path)
{
    rapidcsv::Document doc(path, rapidcsv::LabelParams(0, -1));
    sysreg_names = doc.GetColumnNames();
    return true;
}

bool lookup_sysreg_name(const string &name)
{
    // is general-purpose reg
    if (name.size() < 4)
        return false;

    // no system register loaded in csv
    if (sysreg_names.size() == 0)
        return false;

    return std::find(sysreg_names.begin(), sysreg_names.end(), name) != sysreg_names.end();
}

