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
bool read_sysreg_names_csv(string path)
{
    rapidcsv::Document doc(path, rapidcsv::LabelParams(0, -1));
    std::vector<std::string> names = doc.GetRow<std::string>(0);
    return true;
}

bool lookup_sysreg_name(RegisterId &out, const string &name)
{
    if (name.size() < 4)
        // is general-purpose reg
        return false;

    const char *prefix = name.c_str();
    const char *suffix = prefix + strcspn(prefix, "0123456789_");

    for (size_t i = 0; i < lenof(reg_prefixes); i++) {
        if (i == (size_t)RegPrefix::internal_flags)
            continue; // this isn't a real register name

        const RegPrefixInfo &pfx = reg_prefixes[i];
        if (compare(prefix, suffix - prefix, pfx.name, pfx.namelen)) {
            unsigned long index = 0;
            if (!*suffix) {
                /*
                 * Accept a register name without a numeric suffix
                 * only if the register class is a singleton.
                 */
                if (pfx.n != 1)
                    continue;
                index = 0;
            } else {
                /*
                 * Accept a register name _with_ a numeric suffix only
                 * if the register class is _not_ a singleton.
                 * Moreover, the suffix should be in range.
                 */
                if (pfx.n == 1)
                    continue;
                index = strtoul(suffix, NULL, 10);
                if (index >= pfx.n)
                    continue;
            }
            out.prefix = RegPrefix(i);
            out.index = index;
            return true;
        }
    }

    return false;
}
