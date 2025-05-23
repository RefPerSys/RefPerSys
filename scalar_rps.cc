/****************************************************************
 * file scalar_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to scalar values (and strings)
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2019 - 2025 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "refpersys.hh"

extern "C" const char rps_scalar_gitid[];
const char rps_scalar_gitid[]= RPS_GITID;

extern "C" const char rps_scalar_date[];
const char rps_scalar_date[]= __DATE__;

extern "C" const char rps_scalar_shortgitid[];
const char rps_scalar_shortgitid[]= RPS_SHORTGITID;

/** important NOTICE
 *
 * Don't change the code of the below function
 * rps_compute_cstr_two_64bits_hash after mid-september 2023. This
 * code is shared (copied) into the guifltk-refpersys program file
 * jsonrpsfltk.cc
 *
 * This rps_compute_cstr_two_64bits_hash function uses the u8_mbtouc
 * function from GNU libunistring library to handle Unicode UTF8
 * characters.
 *
 * Function: int u8_mbtouc (ucs4_t *puc, const uint8_t *s, size_t n)
 *   Returns the length (number of units) of the first character in s,
 *   putting its ucs4_t representation in *puc. Upon failure, *puc is
 *   set to 0xfffd, and an appropriate number of units is returned.
 *
 * This u8_mbtouc function fails if an invalid sequence of units is
 * encountered at the beginning of s, or if additional units (after
 * the n provided units) would be needed to form a character.
 **/
int
rps_compute_cstr_two_64bits_hash(int64_t ht[2], const char*cstr, int len)
{
  if (!ht || !cstr)
    return 0;
  if (len < 0)
    len = strlen(cstr);
  ht[0] = 0;
  ht[1] = 0;
  if (len == 0)
    return 0;
  int64_t h0=len, h1=60899;
  const char*end = cstr + len;
  int utf8cnt = 0;
  for (const char*pc = cstr; pc < end; )
    {
      ucs4_t uc1=0, uc2=0, uc3=0, uc4=0;
      int l1 = u8_mbtouc(&uc1, (const uint8_t*)pc, end - pc);
      if (l1<0)
        return 0;
      utf8cnt ++;
      pc += l1;
      if (pc >= end)
        break;
      h0 = (h0 * 60869) ^ (uc1 * 5059 + (h1 & 0xff));
      int l2 = u8_mbtouc(&uc2, (const uint8_t*)pc, end - pc);
      if (l2<0)
        return 0;
      h1 = (h1 * 53087) ^ (uc2 * 43063 + utf8cnt + (h0 & 0xff));
      utf8cnt ++;
      pc += l2;
      if (pc >= end)
        break;
      int l3 = u8_mbtouc(&uc3, (const uint8_t*)pc, end - pc);
      if (l3<0)
        return 0;
      h1 = (h1 * 73063) ^ (uc3 * 53089 + (h0 & 0xff));
      utf8cnt ++;
      pc += l3;
      if (pc >= end)
        break;
      int l4 = u8_mbtouc(&uc4, (const uint8_t*)pc, end - pc);
      if (l4<0)
        return 0;
      h0 = (h0 * 73019) ^ (uc4 * 23057 + 11 * (h1 & 0x1ff));
      utf8cnt ++;
      pc += l4;
      if (pc >= end)
        break;
    }
  ht[0] = h0;
  ht[1] = h1;
  return utf8cnt;
} // end of rps_compute_cstr_two_64bits_hash


const Rps_String*
Rps_String::make(const char*cstr, int len)
{
  cstr = normalize_cstr(cstr);
  len = normalize_len(cstr, len);
  if (u8_check(reinterpret_cast<const uint8_t*>(cstr), len))
    throw std::domain_error("invalid UTF-8 string");
  Rps_String* str
    = rps_allocate_with_wordgap<Rps_String> (len/sizeof(void*)+1, cstr, len);
  return str;
} // end of Rps_String::make


Json::Value
Rps_String::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  if (cstr()[0] == '_')
    {
      Json::Value vmap(Json::objectValue);
      vmap["str"] = cstr();
      return vmap;
    }
  else
    return Json::Value(cstr());
} // end Rps_String::dump_json

void
Rps_String::val_output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  if (depth > maxdepth)
    {
      out << "??";
      return;
    };
  Json::Value jstr(cstr());
  out << jstr;
} // end Rps_String::val_output

Rps_ObjectRef
Rps_String::compute_class(Rps_CallFrame*) const
{
  return RPS_ROOT_OB(_62LTwxwKpQ802SsmjE); // the `string` class
} // end Rps_String::compute_class

void
Rps_Double::val_output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  if (depth > maxdepth)
    {
      out << "??";
      return;
    }
  out << dval();
} // end Rps_Double::val_output

Rps_ObjectRef
Rps_Double::compute_class(Rps_CallFrame*) const
{
  return RPS_ROOT_OB(_98sc8kSOXV003i86w5); // the `double` class
} // end Rps_String::compute_class

////////////////////////////////////////////////////////////////
///// UTF8 encoded string output
void
rps_output_utf8_html(std::ostream&out, const char*str, int bytlen, bool nl2br)
{
  if (!str)
    return;
  if (bytlen<0)
    bytlen = strlen(str);
  const char*end = str + bytlen;
  const uint8_t *next;
  const uint8_t* uend =(const uint8_t*)end;
  for (const uint8_t* pc = (const uint8_t*)str;
       pc < uend && *pc;
       pc = next)
    {
      ucs4_t uc=0;
      next = u8_next(&uc, pc);
      switch (uc)
        {
        case '&':
          out << "&amp;" ;
          break;
        case '\'':
          out << "&apos;";
          break;
        case '"':
          out << "&quot;";
          break;
        case '<':
          out << "&lt;";
          break;
        case '>':
          out << "&gt;";
          break;
        case '\r':
          out << '\r';
          break;
        case '\n':
          if (nl2br)
            out << "<br/>";
          out << '\n';
          break;
        case '\t':
          out << '\t';
          break;
        case '\v':
          out << '\v';
          break;
        case '\f':
          out << '\f';
          break;
        case ' ':
          out << ' ';
          break;
        /// optimizing frequent cases
        case '.':
        case ',':
        case':':
        case ';':
        case '?':
        case'_':
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
        case '[':
        case ']':
        case'{':
        case '}':
        case '!':
        case '=':
        case '0' ... '9':
        case 'a' ... 'z':
        case 'A' ... 'Z':
          out<<(char)uc;
          break;
        default:
          if (uc>' ' && uc<127) out<< (char)uc;
          else
            {
              char ubuf[16];
              memset(ubuf, 0, sizeof(ubuf));
              snprintf(ubuf, sizeof(ubuf), "&#%u;", (unsigned)uc);
              out <<ubuf;
            }
          break;
        }
    }
} // end rps_output_utf8_html


void
rps_output_utf8_cjson(std::ostream&out, const char*str, int bytlen)
{
  if (!str)
    return;
  if (bytlen<0)
    bytlen = strlen(str);
  const char*end = str + bytlen;
  const uint8_t *next;
  const uint8_t* uend =(const uint8_t*)end;
  for (const uint8_t* pc = (const uint8_t*)str;
       pc < uend && *pc;
       pc = next)
    {
      ucs4_t uc=0;
      next = u8_next(&uc, pc);
      switch (uc)
        {
        case '\'':
          out << "\\'";
          break;
        case '\\':
          out << "\\\\";
          break;
        case '"':
          out << "\\\"";
          break;
        case '\r':
          out << "\\r";
          break;
        case '\n':
          out << "\\n";
          break;
        case '\t':
          out << "\\t";
          break;
        case '\v':
          out << "\\v";
          break;
        case '\f':
          out << "\\f";
          break;
        case ' ':
          out << ' ';
          break;
        /// frequent cases
        case '.':
        case '&':
        case ',':
        case':':
        case ';':
        case '!':
        case '?':
        case'_':
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
        case '[':
        case ']':
        case'{':
        case '}':
        case '=':
        case '<':
        case '>':
        case '0' ... '9':
        case 'a' ... 'z':
        case 'A' ... 'Z':
          out<<(char)uc;
          break;
        default:
          if (uc>' ' && uc<127)
            out<< (char)uc;
          else
            {
              char ubuf[16];
              memset(ubuf, 0, sizeof(ubuf));
              if (uc<255)
                snprintf(ubuf, sizeof(ubuf), "\\x%02x", (unsigned)uc);
              else if (uc<65535)
                snprintf(ubuf, sizeof(ubuf), "\\u%04x", (unsigned)uc);
              else
                snprintf(ubuf, sizeof(ubuf), "\\U%08x", (unsigned)uc);
              out <<ubuf;
            }
          break;
        }
    }
} // end rps_output_utf8_cjson


/** Given a shell pattern like foo/x*.h and a directory path like
   /usr/include:/usr/local/include find a readable plain file path;
   tilde patterns ~joe are expanded and $XX are expanded but not command
   line substitution like $(ls -lt *foo|head -1); for example
   rps_glob_plain_path("sys/stat.h",
   "/usr/include/:/usr/include/x86-64-linux/gnu/") would return
   "/usr/include/sys/stat.h" on my Linux desktop. If no file is found,
   the empty string is returned. */
std::string
rps_glob_plain_file_path(const char*shellpatt, const char*dirpath)
{
#warning unimplemented rps_glob_plain_file_path
  if (!shellpatt || !shellpatt[0])
    return std::string(nullptr);
  if (shellpatt[0] == '~')
    {
      wordexp_t wx = {};
      int err = wordexp(shellpatt, &wx, WRDE_NOCMD|WRDE_UNDEF);
      if (err)
        {
          wordfree(&wx);
          return std::string(nullptr);
        };
      if (wx.we_wordc==0)
        {
          wordfree(&wx);
          return std::string(nullptr);
        };
      if (wx.we_wordc==1)
        {
          char*rp = realpath(wx.we_wordv[0], nullptr);
          struct stat rs= {};
          wordfree(&wx);
          if (stat(rp, &rs) || (rs.st_mode & S_IFMT)!=S_IFREG
              || access(rp, R_OK))
            {
              free (rp);
              return std::string(nullptr);
            };
          return std::string(rp);
        }
      // several files but starting with ~
      {
        wordfree(&wx);
        return std::string(nullptr);
      };
    }
  else if (shellpatt[0] == '/')
    {
      /// absolute file path
      glob_t g = {};
      if (glob(shellpatt, GLOB_ERR|GLOB_MARK|GLOB_TILDE_CHECK, nullptr, &g))
        {
          globfree(&g);
          return std::string(nullptr);
        };
      if (g.gl_pathc != 1)
        {
          globfree(&g);
          return std::string(nullptr);
        }
      char*rp = realpath(g.gl_pathv[0], nullptr);
      struct stat rs= {};
      globfree(&g);
      if (stat(rp, &rs) || (rs.st_mode & S_IFMT)!=S_IFREG
          || access(rp, R_OK))
        {
          free (rp);
          return std::string(nullptr);
        };
      return std::string(rp);
    };
  if (!dirpath || !dirpath[0])
    return std::string(nullptr);
  {
    std::string dirstr(dirpath);
    const char* pc=nullptr;
    const char* colon=nullptr;
    for (pc = dirstr.c_str(); pc && *pc; pc = (colon?(colon+1):nullptr))
      {
        colon = strchr(pc, ':');
        std::string curdir;
        if (colon) curdir=std::string(pc, colon-pc-1);
        else curdir=std::string(pc);
        /// absolute file path
        glob_t g = {};
        std::string curpatt = curdir + "/" + shellpatt;
        if (glob(curpatt.c_str(), GLOB_ERR|GLOB_MARK, nullptr, &g))
          {
            globfree(&g);
            return std::string(nullptr);
          };
        if (g.gl_pathc != 1)
          {
            globfree(&g);
            return std::string(nullptr);
          }
        char*rp = realpath(g.gl_pathv[0], nullptr);
        struct stat rs= {};
        globfree(&g);
        if (stat(rp, &rs) || (rs.st_mode & S_IFMT)!=S_IFREG
            || access(rp, R_OK))
          {
            free (rp);
            return std::string(nullptr);
          };
        return std::string(rp);
      }
  }
  return std::string(nullptr);
} // end rps_glob_plain_file_path

//////////////////////////////////////////////// end of file scalar_rps.cc
