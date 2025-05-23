/****************************************************************
 * file userpref_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for user preferences.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net> (France)
 *      Abhishek Chakravarti <abhishek@taranjali.org> (India)
 *      Nimesh Neema <nimeshneema@gmail.com> (India)
 *
 *      © Copyright (C) 2024 - 2025 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * License:
 *    This program is free software: you can redistribute it
 *    and/or modify it under the terms of the GNU General Public
 *    License as published by the Free Software Foundation, either
 *    version 3 of the License, or (at your option) any later
 *    version. Alternatively, at your choice you can also use the GNU
 *    Lesser General Public License v3 or any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details (or LGPLv3).
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG INIReader
//@@PKGCONFIG inih

#include "refpersys.hh"

/// from /github.com/benhoyt/inih or /github.com/OSSystems/inih
/// packaged as libinih-dev & libinih1
#include "INIReader.h"

extern "C" const char rps_userpref_gitid[];
const char rps_userpref_gitid[]= RPS_GITID;

extern "C" const char rps_userpref_date[];
const char rps_userpref_date[]= __DATE__;

extern "C" const char rps_userpref_shortgitid[];
const char rps_userpref_shortgitid[]= RPS_SHORTGITID;

static INIReader* rps_userpref_ird;
static std::atomic_bool rps_userpref_is_parsed;

extern "C" void rps_set_user_preferences(char*);

extern "C" void rps_parse_user_preferences(Rps_MemoryFileTokenSource*mts);

#define RPS_USER_PREFERENCE_MAGIC "*REFPERSYS_USER_PREFERENCES"
static Rps_MemoryFileTokenSource* rps_userpref_mts;

static void     // passed to atexit
rps_delete_user_preferences(void)
{
  if (rps_userpref_mts)
    delete rps_userpref_mts;
  rps_userpref_mts = nullptr;
  if (rps_userpref_ird)
    delete rps_userpref_ird;
  rps_userpref_ird = nullptr;
} // end rps_delete_user_preferences

void
rps_set_user_preferences(char*path)
{
  RPS_ASSERT(!access(path, R_OK));
  RPS_ASSERT(rps_is_main_thread());
  if (rps_userpref_mts)
    RPS_FATALOUT("cannot set user preferences more than once, here to "
                 << path << " but previously to "
                 << rps_userpref_mts->path());
  rps_userpref_mts = new  Rps_MemoryFileTokenSource(path);
  while (rps_userpref_mts->get_line())
    {
      const char*clp = rps_userpref_mts->curcptr();
      if (!clp)
        break;
      else if (!strncmp(clp, RPS_USER_PREFERENCE_MAGIC,
                        strlen(RPS_USER_PREFERENCE_MAGIC)))
        break;
    };
  rps_parse_user_preferences(rps_userpref_mts);
  atexit(rps_delete_user_preferences);
} // end  rps_set_user_preferences


void
rps_parse_user_preferences(Rps_MemoryFileTokenSource*mts)
{
  RPS_ASSERT(mts);
  RPS_POSSIBLE_BREAKPOINT();
  RPS_ASSERT(mts->toksrcmfil_line > mts->toksrcmfil_start
             && mts->toksrcmfil_line <  mts->toksrcmfil_end);
  int curlineno = mts->line();
  rps_userpref_ird = new INIReader(mts->toksrcmfil_line,
                                   mts->toksrcmfil_end - mts->toksrcmfil_line);
  RPS_ASSERT(rps_userpref_ird != nullptr);
  if (int pe = rps_userpref_ird->ParseError())
    {
      RPS_FATALOUT("failed to parse user preference "
                   << mts->path() << ":" << pe+curlineno);
    };
  bool parsedonce = !rps_userpref_is_parsed.exchange(true);
  if (!parsedonce)
    RPS_FATALOUT("rps_parse_user_preferences called more than once for "
                 << mts->path());
  RPS_INFORMOUT("rps_parse_user_preferences path " << mts->path());
  /// see also file etc/user-preferences-refpersys.txt as example
} // end rps_parse_user_preferences

bool
rps_has_parsed_user_preferences(void)
{
  return rps_userpref_is_parsed.load();
} // end rps_has_parsed_user_preferences

std::string
rps_userpref_get_string(const std::string& section, const std::string& name,
                        const std::string& default_value)
{
  if (!rps_userpref_ird)
    return default_value;
  return rps_userpref_ird->GetString(section,name,default_value);
} // end rps_userpref_get_string

long
rps_userpref_get_long(const std::string& section, const std::string& name, long default_value)
{
  if (!rps_userpref_ird)
    return default_value;
  return rps_userpref_ird->GetInteger(section,name,default_value);
} // end rps_userpref_get_long

double
rps_userpref_get_double(const std::string& section, const std::string& name, double default_value)
{
  if (!rps_userpref_ird)
    return default_value;
  return rps_userpref_ird->GetReal(section,name,default_value);
} // end rps_userpref_get_double


bool
rps_userpref_get_bool(const std::string& section, const std::string& name, bool default_value)
{
  if (!rps_userpref_ird)
    return default_value;
  return rps_userpref_ird->GetBoolean(section,name,default_value);
} // end rps_userpref_get_bool

bool
rps_userpref_has_section(const std::string& section)
{
  if (!rps_userpref_ird)
    return false;
  return rps_userpref_ird->HasSection(section);
} // end rps_userpref_has_section

bool
rps_userpref_has_value(const std::string& section, const std::string& name)
{
  if (!rps_userpref_ird)
    return false;
  return rps_userpref_ird->HasValue(section,name);
} // end rps_userpref_has_value



const char*
rps_userpref_find_dup_cstring(bool *pfound,
                              const char*csection, const char* cname)
{
  RPS_ASSERT(pfound);
  RPS_ASSERT(csection);
  RPS_ASSERT(cname);
  std::string section(csection);
  std::string name(cname);
  if (!rps_userpref_ird || !rps_userpref_ird->HasValue(section,name))
    {
      *pfound = false;
      return nullptr;
    };
  std::string val = rps_userpref_ird->GetString(section,name,"");
  const char*res = strdup(val.c_str());
  if (!res)
    {
      *pfound = false;
      return nullptr;
    };
  *pfound = true;
  return res;
}         // end rps_userpref_find_dup_cstring

/// TODO: maybe this function cannot work, we need to check.
const char*
rps_userpref_raw_cstring(const char*csection, const char*cname)
{
  RPS_ASSERT(csection);
  RPS_ASSERT(cname);
  std::string section(csection);
  std::string name(cname);
  if (!rps_userpref_ird || !rps_userpref_ird->HasValue(section,name))
    return nullptr;
  const std::string &val = rps_userpref_ird->GetString(section,name,"");
  return val.c_str();
} // end rps_userpref_raw_cstring

long
rps_userpref_find_clong(bool *pfound,
                        const char*csection, const char* cname)
{
  RPS_ASSERT(pfound);
  RPS_ASSERT(csection);
  RPS_ASSERT(cname);
  std::string section(csection);
  std::string name(cname);
  if (!rps_userpref_ird || !rps_userpref_ird->HasValue(section,name))
    {
      *pfound = false;
      return 0L;
    };
  long res = rps_userpref_ird->GetInteger(section,name,0L);
  *pfound = true;
  return res;
} // end rps_userpref_find_clong

double
rps_userpref_find_cdouble(bool *pfound,
                          const char*csection, const char* cname)
{
  RPS_ASSERT(pfound);
  RPS_ASSERT(csection);
  RPS_ASSERT(cname);
  std::string section(csection);
  std::string name(cname);
  if (!rps_userpref_ird || !rps_userpref_ird->HasValue(section,name))
    {
      *pfound = false;
      return 0.0;
    };
  double res = rps_userpref_ird->GetReal(section,name,0.0);
  *pfound = true;
  return res;
}         // end rps_userpref_find_cdouble

bool
rps_userpref_find_cbool(bool *pfound,
                        const char*csection, const char* cname)
{
  RPS_ASSERT(pfound);
  RPS_ASSERT(csection);
  RPS_ASSERT(cname);
  std::string section(csection);
  std::string name(cname);
  if (!rps_userpref_ird || !rps_userpref_ird->HasValue(section,name))
    {
      *pfound = false;
      return false;
    };
  bool res = rps_userpref_ird->GetBoolean(section,name,false);
  *pfound = true;
  return res;
}          // end rps_userpref_find_cbool

bool
rps_userpref_with_csection(const char*csection)
{
  if (!csection || !rps_userpref_ird)
    return false;
  std::string section(csection);
  return rps_userpref_ird->HasSection(section);
}         // end rps_userpref_with_csection

bool
rps_userpref_with_cvalue(const char*csection, const char*cname)
{
  if (!csection || !cname || !rps_userpref_ird)
    return false;
  std::string section(csection);
  std::string name(cname);
  return rps_userpref_ird->HasValue(section, name);
}         // end rps_userpref_with_cvalue

#warning using https://github.com/benhoyt/inih for user preferences
/// TODO: look into https://github.com/OSSystems/inih

////// end of file userpref_rps.cc
