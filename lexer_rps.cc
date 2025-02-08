/****************************************************************
 * file lexer_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the lexer support for the Read Eval Print Loop
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net> (France)
 *      Abhishek Chakravarti <abhishek@taranjali.org> (India)
 *      Nimesh Neema <nimeshneema@gmail.com> (India)
 *
 *      © Copyright 2019 - 2024 The Reflective Persistent System Team
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

#include "refpersys.hh"

/// libunistring www.gnu.org/software/libunistring/
#include "unictype.h"
#include "uniconv.h"

#include <termios.h>
#include <wordexp.h>

extern "C" const char rps_lexer_gitid[];
const char rps_lexer_gitid[]= RPS_GITID;

extern "C" const char rps_lexer_date[];
const char rps_lexer_date[]= __DATE__;

extern "C" const char rps_lexer_shortgitid[];
const char rps_lexer_shortgitid[]= RPS_SHORTGITID;


extern "C" Rps_StringValue rps_lexer_token_name_str_val;
Rps_StringValue rps_lexer_token_name_str_val(nullptr);

Rps_TokenSource* Rps_TokenSource::toksrc_current_;

Rps_TokenSource::Rps_TokenSource(std::string name)
  : toksrc_name(name), toksrc_line(0), toksrc_col(0), toksrc_counter(0),
    toksrc_linebuf{},
    toksrc_token_deq(),
    toksrc_ptrnameval(nullptr)
{
  RPS_ASSERT(toksrc_current_ == nullptr);
  toksrc_current_ = this;
} // end Rps_TokenSource::Rps_TokenSource

void
Rps_TokenSource::gc_mark(Rps_GarbageCollector&gc, unsigned depth)
{
  RPS_ASSERT(gc.is_valid_garbcoll());
  really_gc_mark(gc,depth);
} // end Rps_TokenSource::gc_mark

void
Rps_TokenSource::really_gc_mark(Rps_GarbageCollector&gc, unsigned depth)
{
  RPS_ASSERT(gc.is_valid_garbcoll());
  RPS_ASSERT(depth < max_gc_depth);
  if (rps_lexer_token_name_str_val)
    rps_lexer_token_name_str_val.gc_mark(gc,depth);
  if (toksrc_ptrnameval)
    toksrc_ptrnameval->gc_mark(gc, depth+1);
  toksrc_token_deq.gc_mark(gc, depth+1);
} // end Rps_TokenSource::really_gc_mark


/// the current token source name is needed as a string value for
/// constructing Rps_LexTokenValue-s. We want to avoid creating that
/// source name, as a RefPerSys string, at every lexical token. So we
/// try to memoize it in rps_lexer_token_name_str_val;
Rps_Value
Rps_TokenSource::source_name_val(Rps_CallFrame*callframe)
{
  RPS_ASSERT(callframe==nullptr || callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  if (rps_lexer_token_name_str_val && rps_lexer_token_name_str_val.is_string()
      && rps_lexer_token_name_str_val.to_cppstring() == toksrc_name)
    return rps_lexer_token_name_str_val;
  rps_lexer_token_name_str_val = Rps_String::make(toksrc_name);
  return rps_lexer_token_name_str_val;
} // end Rps_TokenSource::source_name_val


const Rps_LexTokenZone*
Rps_TokenSource::make_token(Rps_CallFrame*callframe,
                            Rps_ObjectRef lexkindarg, Rps_Value lexvalarg,
                            const Rps_String*sourcev)
{
  RPS_LOCALFRAME(RPS_ROOT_OB(_0S6DQvp3Gop015zXhL), //lexical_token∈class
                 /*callerframe:*/callframe,
                 Rps_ObjectRef lexkindob;
                 Rps_Value lexval;
                 Rps_LexTokenZone*tokenp;
                 Rps_Value namestrv;
                 const Rps_String* nstrv;
                 const Rps_String* srcv;
                );
  RPS_ASSERT(rps_is_main_thread());
  _f.lexkindob = lexkindarg;
  _f.lexval = lexvalarg;
  _f.namestrv = source_name_val(&_);
  _f.nstrv = _f.namestrv.as_string();
  _f.srcv = sourcev;
  RPS_ASSERT (!_f.srcv || _f.srcv->stored_type() == Rps_Type::String);
  _f.tokenp =
    Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,
    Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
    (this, _f.lexkindob, _f.lexval, _f.nstrv,
     toksrc_line, toksrc_col);
#warning Rps_TokenSource::make_token should probably use _f.srcv
  return _f.tokenp;
} // end Rps_TokenSource::make_token

const std::string
Rps_TokenSource::position_str(int col) const
{
  if (col<0) col = toksrc_col;
  std::ostringstream outs;
  outs << toksrc_name << ":L" << toksrc_line << ",C"  << col << std::flush;
  return outs.str();
} // end Rps_TokenSource::position_str


void
Rps_TokenSource::display_current_line_with_cursor(std::ostream&out) const
{
  out << toksrc_linebuf << std::endl;
  int curcol = col();
  const char*nextp = nullptr;
  int i = 0;
  for (const char* curp = toksrc_linebuf.c_str();
       curp && *curp && i < curcol;
       curp = nextp, i++)
    {
      ucs4_t curuc=0;
      int ulen=curp?u8_strmbtouc(&curuc, (const uint8_t*)curp):0; // length in bytes
      if (ulen<0 || curuc==0)
        break;
      out << ' ';
      nextp = curp+ulen;
    }
  out << "↑" << std::endl;
} // end Rps_TokenSource::display_current_line_with_cursor

Rps_TokenSource::~Rps_TokenSource()
{
  toksrc_name.clear();
  toksrc_line= -1;
  toksrc_col= -1;
  toksrc_linebuf.clear();
  toksrc_token_deq.clear();
  toksrc_current_ = nullptr;
} // end Rps_TokenSource::~Rps_TokenSource

Rps_StreamTokenSource::Rps_StreamTokenSource(std::string path)
  : Rps_TokenSource(""), toksrc_input_stream()
{
  wordexp_t wx= {};
  int err = wordexp(path.c_str(), &wx, WRDE_SHOWERR);
  if (err)
    {
      RPS_WARNOUT("stream token source for '" << Rps_Cjson_String(path) << "' failed: error#" << err);
      throw std::runtime_error(std::string{"bad stream token source:"} + path);
    }
  if (wx.we_wordc == 0)
    {
      RPS_WARNOUT("no stream token source for '" << (Rps_Cjson_String(path)) << "'");
      throw std::runtime_error(std::string{"no stream token source:"} + path);
    }
  else if (wx.we_wordc > 1)
    {
      RPS_WARNOUT("ambiguous stream token source for '" << path << "' expanded to "
                  << wx.we_wordv[0] << " and " << wx.we_wordv[1]
                  << ((wx.we_wordc>2)?" etc...":" files"));
      throw std::runtime_error(std::string{"ambiguous stream token source:"} + path);
    };
  char* curword = wx.we_wordv[0];
  toksrc_input_stream.open(curword);
  set_name(std::string(curword));
  RPS_DEBUG_LOG(REPL, "constr °StreamTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(LOWREP, "constr °StreamTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(CMD, "constr °StreamTokenSource@ " <<(void*)this << " " << *this);
} // end Rps_StreamTokenSource::Rps_StreamTokenSource

void
Rps_StreamTokenSource::display(std::ostream&out) const
{
  output(out, 0, Rps_Value::debug_maxdepth);
  out << std::endl;
  display_current_line_with_cursor(out);
} // end Rps_StreamTokenSource::display

Rps_StreamTokenSource::~Rps_StreamTokenSource()
{
  toksrc_input_stream.close();
  RPS_DEBUG_LOG(REPL, "destr °StreamTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(LOWREP, "destr °StreamTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(CMD, "destr °StreamTokenSource@ " <<(void*)this << " " << *this);
} // end Rps_StreamTokenSource::~Rps_StreamTokenSource

bool
Rps_StreamTokenSource::get_line(void)
{
  std::getline( toksrc_input_stream, toksrc_linebuf);
  if (!toksrc_input_stream && toksrc_linebuf.empty())
    return false;
  starting_new_input_line();
  return true;
} // end Rps_StreamTokenSource::get_line



Rps_CinTokenSource::Rps_CinTokenSource()
  : Rps_TokenSource("-")
{
  RPS_DEBUG_LOG(REPL, "constr CinTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(LOWREP, "constr CinTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(CMD, "constr CinTokenSource@ " <<(void*)this << " " << *this);
};                              // end Rps_CinTokenSource::Rps_CinTokenSource

Rps_CinTokenSource::~Rps_CinTokenSource()
{
  RPS_DEBUG_LOG(REPL, "destr CinTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(LOWREP, "destr CinTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(CMD, "destr CinTokenSource@ " <<(void*)this << " " << *this);
};      // end Rps_CinTokenSource::~Rps_CinTokenSource

bool
Rps_CinTokenSource::get_line(void)
{
  std::getline(std::cin, toksrc_linebuf);
  if (!std::cin && toksrc_linebuf.empty()) return false;
  starting_new_input_line();
  return true;
} // end Rps_CinTokenSource::get_line


void
Rps_CinTokenSource::display(std::ostream&out) const
{
  output(out, 0, Rps_Value::debug_maxdepth);
  out << std::endl;
  display_current_line_with_cursor(out);
} // end Rps_CinTokenSource::display


////////////////
Rps_StringTokenSource::Rps_StringTokenSource(std::string inptstr, std::string name)
  : Rps_TokenSource(name), toksrcstr_inp(inptstr), toksrcstr_str(inptstr)
{
  RPS_DEBUG_LOG(REPL, "constr StringTokenSource@ " <<(void*)this << " " << (*this)
                << " from " << Rps_QuotedC_String(toksrcstr_str)
                << " of " << toksrcstr_str.size() << " bytes, named " << name
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "const StringTokenSource"));
  RPS_DEBUG_LOG(LOWREP, "constr StringTokenSource@ " <<(void*)this << " " << (*this)
                << " from " << Rps_QuotedC_String(toksrcstr_str));
  RPS_DEBUG_LOG(CMD, "constr StringTokenSource@ " <<(void*)this << " " << (*this)
                << " from " << Rps_QuotedC_String(toksrcstr_str));
} // end Rps_StringTokenSource::Rps_StringTokenSource


Rps_StringTokenSource::~Rps_StringTokenSource()
{
  RPS_DEBUG_LOG(REPL, "destr ~StringTokenSource " << *this
                << " with "  << Rps_QuotedC_String(toksrcstr_str)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "destr StringTokenSource"));
  RPS_DEBUG_LOG(LOWREP, "destr StringTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(CMD, "destr StringTokenSource@ " <<(void*)this << " " << *this);
} // end Rps_StringTokenSource::~Rps_StringTokenSource


bool
Rps_StringTokenSource::get_line()
{
  std::getline(toksrcstr_inp, toksrc_linebuf);
  if (!toksrcstr_inp && toksrc_linebuf.empty())
    return false;
  starting_new_input_line();
  RPS_DEBUG_LOG(REPL, "Rps_StringTokenSource::get_line at " << position_str());
  return true;
} // end Rps_StringTokenSource::get_line()


void
Rps_StringTokenSource::output (std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  if (depth > maxdepth)
    {
      out << "??";
      return;
    };
  std::string abbrev = toksrcstr_str;
  auto firstnl = abbrev.find('\n');
  if (firstnl>0 && firstnl<toksrcstr_str.length())
    abbrev.resize(firstnl-1);
  const size_t maxabbrevlen = 24;
  if (abbrev.length() > maxabbrevlen)
    {
      const uint8_t* curabc = (const uint8_t*)abbrev.c_str();
      const uint8_t* abstart = (const uint8_t*)abbrev.c_str();
      while ((int)(curabc - abstart) < (int)maxabbrevlen && *curabc)
        {
          int curclen = u8_strmblen(curabc);
          if (curclen<=0)
            break;
          auto prevabc = curabc;
          curabc += curclen;
          if ((int)(curabc - abstart) >= (int)maxabbrevlen && prevabc > abstart)
            {
              abbrev.resize(abstart - prevabc);
              break;
            }
        }
    }
  out << "StriTokSrc°" << name();
  if (abbrev.length() < toksrcstr_str.length())
    out << Rps_QuotedC_String(abbrev) << "⋯" // U+22EF MIDLINE HORIZONTAL ELLIPSIS;
        << "l" << toksrcstr_str.length();
  else
    out << Rps_QuotedC_String(abbrev);
  out << '@' << position_str() << " tok.cnt:" << token_count()
      << " str: " << Rps_QuotedC_String(toksrcstr_inp.str());
}       // end Rps_StringTokenSource::output


void
Rps_StringTokenSource::display(std::ostream&out) const
{
  output(out, 0, Rps_Value::debug_maxdepth);
  out << std::endl;
  display_current_line_with_cursor(out);
} // end Rps_StringTokenSource::display



////////////////

Rps_MemoryFileTokenSource::Rps_MemoryFileTokenSource(const std::string path)
  : Rps_TokenSource(path), toksrcmfil_path(path),
    toksrcmfil_start(nullptr),
    toksrcmfil_line(nullptr),
    toksrcmfil_end(nullptr), toksrcmfil_nextpage(nullptr)
{
  int fd= open(path.c_str(), O_RDONLY);
  if (fd<0)
    RPS_FATALOUT("cannot open memory file token source " << path
                 << ":" << strerror(errno));
  struct stat st= {};
  memset ((void*)&st, 0, sizeof(st));
  if (fstat(fd, &st))
    RPS_FATALOUT("cannot fstat fd#" << fd << " for memory file " << path
                 << ":" << strerror(errno));
  if ((st.st_mode & S_IFMT) != S_IFREG)
    RPS_FATALOUT("memory file source " << path << " is not a plain regular file ; fd#" << fd);
  size_t fsiz = st.st_size;
  long pgsiz = sysconf(_SC_PAGESIZE);
  RPS_ASSERT(pgsiz > 0 && (pgsiz & (pgsiz-1)) == 0); // page size should be a power of 2
  int logpgsiz = -1;
  for (int i=10; i<32 && logpgsiz<0; i++)
    {
      if (1L<<i == pgsiz)
        {
          logpgsiz=i;
          break;
        }
    };
  RPS_ASSERT(pgsiz == 1L<<logpgsiz);
  size_t mappedsize = fsiz;
  if (mappedsize & (( 1L<<logpgsiz)-1))
    mappedsize = (mappedsize | ((1L<<logpgsiz)-1)) + 1;
  RPS_ASSERT(mappedsize % pgsiz == 0);
  void* ad = mmap(nullptr, mappedsize, PROT_READ, MAP_PRIVATE, fd, mappedsize);
  if (ad == MAP_FAILED)
    RPS_FATALOUT("memory file source " << path << " mmap failure for fd#" << fd
                 << " " << (mappedsize>>10) << " kb " << strerror(errno));
  toksrcmfil_start = toksrcmfil_line = (char*)ad;
  toksrcmfil_nextpage = (char*)ad + mappedsize;
  toksrcmfil_end = (char*)ad + fsiz;
  close(fd);
  RPS_DEBUG_LOG(REPL, "constr MemoryFileTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(LOWREP, "constr MemoryFileTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(CMD, "constr MemoryFileTokenSource@ " <<(void*)this << " " << *this);
};                              // end Rps_MemoryFileTokenSource::Rps_MemoryFileTokenSource

Rps_MemoryFileTokenSource::~Rps_MemoryFileTokenSource()
{
  RPS_DEBUG_LOG(REPL, "destr MemoryFileTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(LOWREP, "destr MemoryFileTokenSource@ " <<(void*)this << " " << *this);
  RPS_DEBUG_LOG(CMD, "destr MemoryFileTokenSource@ " <<(void*)this << " " << *this);
  RPS_ASSERT(toksrcmfil_start != nullptr && toksrcmfil_end != nullptr);
  if (munmap((void*)toksrcmfil_start, toksrcmfil_nextpage-toksrcmfil_start))
    RPS_FATALOUT("failed to munmap MemoryFileTokenSource@ " <<(void*)this
                 << " path " << toksrcmfil_path
                 << " from " << (void*)toksrcmfil_start
                 << " to " << (void*)toksrcmfil_nextpage);
  toksrcmfil_start=nullptr;
  toksrcmfil_line=nullptr;
  toksrcmfil_end=nullptr;
  toksrcmfil_nextpage=nullptr;
};      // end Rps_MemoryFileTokenSource::~Rps_MemoryFileTokenSource

bool
Rps_MemoryFileTokenSource::get_line(void)
{
#warning unimplemented Rps_MemoryFileTokenSource::get_line
  RPS_FATALOUT("unimplemented Rps_MemoryFileTokenSource::get_line " << *this);
  return false;
} // end Rps_MemoryFileTokenSource::get_line


void
Rps_MemoryFileTokenSource::output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
#warning incomplete Rps_MemoryFileTokenSource::output
  out << "Rps_MemoryFileTokenSource@" << (void*)this
      << "(" << Rps_Cjson_String(name()) << ")"
      << " path:" << Rps_Cjson_String(toksrcmfil_path)
      // << " offset:" <<
      << std::endl;
} // end Rps_MemoryFileTokenSource::output

void
Rps_MemoryFileTokenSource::display(std::ostream&out) const
{
  output(out, 0, Rps_Value::debug_maxdepth);
  out << std::endl;
  display_current_line_with_cursor(out);
} // end Rps_MemoryFileTokenSource::display



////////////////////////////////
Rps_LexTokenValue
Rps_TokenSource::get_token(Rps_CallFrame*callframe)
{
  RPS_ASSERT(callframe==nullptr || callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_0S6DQvp3Gop015zXhL), //lexical_token∈class
                           /*callerframe:*/callframe,
                           Rps_Value res;
                           Rps_ObjectRef lexkindob;
                           Rps_Value lextokv;
                           Rps_ObjectRef oblex;
                           Rps_Value namev;
                           Rps_Value delimv;
                           Rps_ObjectRef obdelim;
                );
  const char* curp = curcptr();
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "+Rps_TokenSource::get_token#" << (toksrc_counter+1) << "? start curp="
                << Rps_QuotedC_String(curp) << " at " << startpos << std::endl
                << "... token_deq:" << toksrc_token_deq << " source:" << *this
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::get_token/start"));
  ucs4_t curuc=0;
  int ulen= -1;
  size_t linelen = toksrc_linebuf.size();
  // check that we have a proper UTF-8 character
  ulen=curp?u8_strmbtouc(&curuc, (const uint8_t*)curp):0; // length in bytes
  if (ulen<0)
    {
      std::ostringstream errout;
      errout << "bad UTF-8 encoding in " << toksrc_name << ":L" << toksrc_line << ",C" << toksrc_col << std::flush;
      RPS_WARNOUT("Rps_TokenSource::get_token#" << (toksrc_counter+1) << "?  fails: " << errout.str() << " in " << (*this));
      throw std::runtime_error(errout.str());
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1) << "?  beforespace curp=" << Rps_QuotedC_String(curp)
                << " startpos:" << startpos << " at:" << position_str());
  while (curp && isspace(*curp) && toksrc_col<(int)linelen)
    curp++, toksrc_col++;
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1) << "? afterspace curp=" << Rps_QuotedC_String(curp)
                << " startpos:" << startpos << " at:" << position_str());
  if (toksrc_col>=(int)linelen)
    {
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << (toksrc_counter+1) << "? EOL  :-◑> ∅null at " << position_str()
                    << " startpos:" << startpos << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      })
                   );
      return nullptr;
    }
  ulen=curp?u8_strmbtouc(&curuc, (const uint8_t*)curp):0; // length in bytes
  /// lex numbers?
  if (isdigit(*curp) ||
      ((curp[0] == '+' || curp[0] == '-') && isdigit(curp[1])))
    {
      int curlin = toksrc_line;
      int curcol = toksrc_col;
      char*endint=nullptr;
      char*endfloat=nullptr;
      const char*startnum = curp;
      bool isfloat = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1) << "?  startnum=" << Rps_QuotedC_String(startnum)
                    << " at " << position_str() << " startpos:" << startpos);
      long long l = strtoll(startnum, &endint, 0);
      double d = strtod(startnum, &endfloat);

      RPS_ASSERT(endint != nullptr && endfloat != nullptr);
      if (endfloat > endint)
        {
          isfloat = true;
          toksrc_col += endfloat - startnum;
          _f.lextokv = Rps_DoubleValue(d);
          _f.lexkindob = RPS_ROOT_OB(_98sc8kSOXV003i86w5); //double∈class
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#"
                        << (toksrc_counter+1) << "? doubleval " << _f.lextokv << " curpos:" << position_str()
                        << " curcptr:" << Rps_QuotedC_String(curcptr()));
        }
      else
        {
          isfloat = false;
          toksrc_col += (int)(endint - startnum);
          _f.lextokv = Rps_Value::make_tagged_int(l);
          _f.lexkindob = RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b); //int∈class
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1) << "? intval "
                        << _f.lextokv << " curpos:" << position_str()
                        << " curcptr:" << Rps_QuotedC_String(curcptr()) << std::endl
                        << "... token_deq:" << toksrc_token_deq);
        }
      _f.namev = source_name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1) << "? namev:" << _f.namev
                    << " curpos:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()) << std::endl
                    << "... in:" << (*this)
                    << " " << (isfloat?"floating-point":"integer") << " number");
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         curlin, curcol);
      _f.res = Rps_LexTokenValue(lextok);
      lextok->set_serial(++toksrc_counter);
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                    << " from¤ " << *this << std::endl
                    << "... curcptr:" <<  Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << " number :-◑> " << _f.res << " @! " << position_str()
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      })
                   );
      return _f.res;
    } //- end lexing numbers
  ///
  /// lex infinities (double) - but not NAN
  else if ((!strncmp(curp, "+INF", 4)
            || !strncmp(curp, "-INF", 4))
           && !isalnum(curp[4]))
    {
      int curlin = toksrc_line;
      int curcol = toksrc_col;
      RPS_DEBUG_LOG(REPL, "get_token#" << (toksrc_counter+1) << "?"
                    << " from¤ " << *this << " infinity " << position_str());
      bool pos = *curp == '+';
      double infd = (pos
                     ?std::numeric_limits<double>::infinity()
                     : -std::numeric_limits<double>::infinity());
      toksrc_col += 4;
      _f.lextokv = Rps_DoubleValue(infd);
      _f.lexkindob = RPS_ROOT_OB(_98sc8kSOXV003i86w5); //double∈class
      _f.namev= source_name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         curlin, curcol);
      _f.res = Rps_LexTokenValue(lextok);
      lextok->set_serial(++toksrc_counter);
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                    << " from¤ " << *this << std::endl
                    <<" infinity :-◑> " << _f.res << " @! " << position_str()
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return _f.res;
    } //- end lexing infinities

  /// lex names or objectids
  else if (isalpha(*curp) || *curp == '_')
    {
      const char*startname = curp;
      int curlin = toksrc_line;
      int curcol = toksrc_col;
      int startcol = curcol;
      while ((isalpha(*curp) || *curp == '_') && toksrc_col<(int)linelen)
        curp++, toksrc_col++;
      std::string namestr(startname, toksrc_col-startcol);
      RPS_DEBUG_LOG(REPL, "get_token#" << (toksrc_counter+1) << "? namestr: '"
                    << Rps_Cjson_String(namestr)
                    << "' tokensrc:" << *this << " startcol=" << startcol
                    << " toksrc_col:" << toksrc_col);
      _f.namev = source_name_val(&_);
      RPS_DEBUG_LOG(REPL, "get_token oid|name '" << namestr << "' namev=" << _f.namev << " at "
                    << position_str(startcol) << " ... " << position_str()
                    << " curcptr:" <<  Rps_QuotedC_String(curcptr()));
      _f.oblex = Rps_ObjectRef::find_object_or_null_by_string(&_, namestr);
      RPS_DEBUG_LOG(REPL, "get_token#" << (toksrc_counter+1) << "?  oid|name '" << namestr << "' oblex=" << _f.oblex);
      const Rps_String* str = _f.namev.to_string();
      RPS_DEBUG_LOG(REPL, "get_token#" << (toksrc_counter+1) << "?  namestr='" << Rps_Cjson_String(namestr) << "' oblex=" << _f.oblex
                    << " namev=" << _f.namev << ", str=" << Rps_Value(str)<< " at "
                    << position_str(startcol) << " ... " << position_str());
      if (_f.oblex)
        {
          _f.lexkindob = RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ); //object∈class
          _f.lextokv = _f.oblex;
          Rps_LexTokenZone* lextokz =
            Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (this, _f.lexkindob, _f.lextokv,
             str,
             curlin, curcol);
          _f.res = Rps_LexTokenValue(lextokz);
          lextokz->set_serial(++toksrc_counter);
          RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                        << " from¤ " << *this
                        << std::endl
                        << " object :-◑> " << _f.res << std::endl
                        << "... @! " << position_str()
                        << " curcptr:" <<  Rps_QuotedC_String(curcptr())
                        << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          return _f.res;
        }
      else if (isalpha(namestr[0]))  // new symbol
        {
          _f.lexkindob = RPS_ROOT_OB(_36I1BY2NetN03WjrOv); //symbol∈class
          _f.lextokv = Rps_StringValue(namestr);
          Rps_LexTokenZone* lextok =
            Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (this, _f.lexkindob, _f.lextokv,
             str,
             curlin, curcol);
          _f.res = Rps_LexTokenValue(lextok);
          lextok->set_serial(++toksrc_counter);
          RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                        << " from¤ " << *this << std::endl
                        << " symbol :-◑> " << _f.res << " @! " << position_str()
                        << " curcptr:" <<  Rps_QuotedC_String(curcptr())
                        << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          return _f.res;
        }
      else   // bad name
        {
          toksrc_col = startcol;
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << toksrc_counter << "? FAIL bad name " << _f.namev
                        << " @! " << position_str()
                        << " curcptr:" <<  Rps_QuotedC_String(curcptr())
                        << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::get_token/badname")
                        << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          return nullptr;
        }
    }
  //// literal single line strings are like in C++
  else if (*curp == '"')   /// plain literal string, on a single line
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      std::string litstr = lex_quoted_literal_string(&_);
      _f.namev= source_name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lexkindob = RPS_ROOT_OB(_62LTwxwKpQ802SsmjE); //string∈class
      _f.lextokv = Rps_String::make(litstr);
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      lextok->set_serial(++toksrc_counter);
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                    << " from¤ " << *this << std::endl
                    << " single-line string :-◑> " << _f.res << " @! " << position_str()
                    << " curcptr:" <<  Rps_QuotedC_String(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return _f.res;
    } // end single-line literal string token

  //// raw literal strings may span across several lines, like in C++
  //// see https://en.cppreference.com/w/cpp/language/string_literal
  else if (curp[0] == 'R' && curp[1] == '"' && isalpha(curp[2]))
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      std::string litstr = lex_raw_literal_string(&_);
      _f.namev= source_name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lexkindob = RPS_ROOT_OB(_62LTwxwKpQ802SsmjE); //string∈class
      _f.lextokv = Rps_String::make(litstr);
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      lextok->set_serial(++toksrc_counter);
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                    << " from¤ " << *this << std::endl
                    << " multi-line literal string :-◑> " << _f.res << " @! " << position_str()
                    << " curcptr:" <<  Rps_QuotedC_String(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return _f.res;
    } // end possibly multi-line raw literal strings

  //// a code chunk or macro string is mixing strings and
  //// objects.... Inspired by GCC MELT see
  //// starynkevitch.net/Basile/gcc-melt/MELT-Starynkevitch-DSL2011.pdf
  /* Improvement over GCC MELT: a macro string can start with "#{"
     ending with "}#" or "#a{" ending with "}a#" or "#ab{" ending with
     "}ab#" or "#abc{" ending with "}abc#" or "#abcd{" ending with
     "}abcd#" or "#abcde{" ending with "}abcde#" or "#abcdef{" ending
     with "}abcdef#" with the letters being arbitrary latin letters,
     upper or lowercase. but no more than 7 letters. */
  else if (curp[0] == '#'
           && (
             curp[1] == '{'
             || (isalpha(curp[1])
                 && (curp[2] == '{'
                     || (isalpha(curp[2])
                         && (curp[3] == '{'
                             || (isalpha(curp[3])
                                 && (curp[4] == '{'
                                     || (isalpha(curp[4])
                                         && (curp[5] == '{'
                                             || (isalpha(curp[6])
                                                 && (curp[6] == '{'
                                                     || (isalpha(curp[7])
                                                         && (curp[7] == '{'
                                                             || (isalpha(curp[7])
                                                                 && (curp[8] == '{'
                                                                     || (isalpha(curp[8])
                                                                         && curp[9] == '{')
                                                                    )
                                                                )
                                                            )
                                                        )
                                                    )
                                                )
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
           )
          )
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1) << "?  code_chunk starting at " << position_str() << curp);
      _f.namev= source_name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lextokv = lex_code_chunk(&_);
      _f.lexkindob = RPS_ROOT_OB(_3rXxMck40kz03RxRLM); //code_chunk∈class
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      lextok->set_serial(++toksrc_counter);
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                    << " from¤ " << *this << std::endl
                    << " code_chunk :-◑> " << _f.res << " @! " << position_str()  << std::endl
                    << "... curcptr:" <<  Rps_QuotedC_String(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return _f.res;
    } // end lexing code chunk

  //// sequence of at most four ASCII or UTF-8 punctuation
  else if (ispunct(*curp) || uc_is_punct(curuc))
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1)
                    <<"? start punctuation curp='" << Rps_QuotedC_String(curp) << "' at " << position_str());
      std::string delimpos = position_str();
      //int startcol = toksrc_col;
      _f.delimv = get_delimiter(&_);
      std::string delimstartstr {curp};
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token#" << (toksrc_counter+1) << "? after "
                    << " get_delimiter_object delimv="
                    << _f.delimv << " at " << position_str() << std::endl
                    << " curp:" << Rps_QuotedC_String(curp)  << " curcptr:"
                    << Rps_QuotedC_String(curcptr()));
      if (!_f.delimv)
        {
          RPS_WARNOUT("invalid delimiter " << Rps_QuotedC_String(delimstartstr) << " at " << delimpos
                      << " curp:" << Rps_QuotedC_String(curp)  << " curcptr:"
                      <<  Rps_QuotedC_String(curcptr())
                      << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::get_token")
                      << std::endl
                      << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          std::string warndelimstr{"invalid delimiter "};
          warndelimstr +=  Rps_Cjson_String(delimstartstr);
          warndelimstr += " at ";
          warndelimstr += delimpos;
          throw std::runtime_error(warndelimstr);
        }
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::get_token#" << toksrc_counter
                    << " from¤ " << *this << std::endl
                    << " delimiter :-◑> " << _f.delimv << " at " << position_str()
                    << " curp:" << Rps_QuotedC_String(curp)
                    << " curcptr:" <<  Rps_QuotedC_String(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return _f.delimv;
    }
#warning Rps_TokenSource::get_token incomplete
  RPS_FATALOUT("unimplemented / incomplete Rps_TokenSource::get_token#" << (toksrc_counter+1) << "? @ " << name()
               << std::endl << "... from " << *this << std::endl
               << "... pos: " << position_str() << " start:" << startpos
               << " curp:" << Rps_QuotedC_String(curp) << std::endl
               << "... curcptr:" <<  Rps_QuotedC_String(curcptr())
               << " token_deq:" << toksrc_token_deq << std::endl
               << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  // we should refactor properly the rps_repl_lexer & Rps_LexTokenZone constructor here
} // end Rps_TokenSource::get_token


Rps_Value
Rps_TokenSource::get_delimiter(Rps_CallFrame*callframe)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_Value res;
                 Rps_ObjectRef obdictdelim;
                 Rps_Value delimv;
                 Rps_Value namev;
                 Rps_ObjectRef lexkindob;
                 Rps_Value lextokv;
                );
  const char*curp=nullptr;
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  std::string startpos = position_str();
  _f.obdictdelim = RPS_ROOT_OB(_627ngdqrVfF020ugC5); //"repl_delim"∈string_dictionary
  auto paylstrdict = _f.obdictdelim->get_dynamic_payload<Rps_PayloadStringDict>();
  RPS_ASSERT (paylstrdict != nullptr);
  std::string delimstr;
  int nbdelimch=0; // number of delimiter UTF8 characters
  bool again=false;
  const char* startp = curcptr();
  unsigned startcol = toksrc_col;
  ucs4_t curuc=0;
  RPS_ASSERT(startp);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_delimiter start " << *this << Rps_QuotedC_String(startp) << " at startpos:" << startpos);
  curp = startp;
  do
    {
      curuc = 0;
      int ulen= -1;
      again=false;
      if (!curp)
        break;
      if (isspace(*curp))
        break;
      // FIXME: maybe we should remember the byte position of every
      // delimiter character (it can be UTF-8 like °)
      if (*curp < 127 && ispunct(*curp))
        {
          delimstr.push_back(*curp);
          curp ++;
          nbdelimch ++;
          again = true;
        }
      else if (*(const uint8_t*)curp > 128
               && (ulen=u8_strmbtouc(&curuc, (const uint8_t*)curp) // length in bytes
                        > 0)
               &&  uc_is_punct(curuc))
        {
          delimstr.append(curp, ulen);
          curp += ulen;
          nbdelimch ++;
          again = true;
        }
      else
        again = false;
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_delimiter delimstr='" << Rps_Cjson_String(delimstr) << "' nbdelimch="
                << nbdelimch << " startpos:" << startpos);
  /***
   * TODO: we need to find the longest substring in delimstr which is a known delimiter
   ***/
  int loopcnt = 0;
  static constexpr int maxloopdelim = 16;
  while (!delimstr.empty() && loopcnt < maxloopdelim)
    {
      loopcnt ++;
      _f.delimv = paylstrdict->find(delimstr);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_delimiter punctuation delimv=" << _f.delimv << " for delimstr='"
                    << Rps_Cjson_String(delimstr)
                    << "' loopcnt#" << loopcnt
                    << " dict@*" << *(paylstrdict->owner()));
      if (_f.delimv)
        {
          _f.lexkindob = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
          _f.lextokv = _f.delimv;
          toksrc_col += delimstr.size();
          const Rps_String* strv = _f.namev.to_string();
          Rps_LexTokenZone* lextok =
            Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (this,_f.lexkindob, _f.lextokv,
             strv,
             toksrc_line, startcol);
          lextok->set_serial(++toksrc_counter);
          _f.res = Rps_LexTokenValue(lextok);
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_delimiter delimiter :-◑> " << _f.res << std::endl
                        << "... at " << position_str() << std::endl
                        << "... from¤ " << *this
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          })
              << " startpos " << startpos << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::get_delimiter"));
          return _f.res;
        };
      /// truncate the delimstr by one ending UTF-8
      const uint8_t* prevu8 = u8_prev(&curuc,
                                      reinterpret_cast<const uint8_t*>
                                      (delimstr.c_str()+delimstr.size()),
                                      reinterpret_cast<const uint8_t*>(delimstr.c_str()));
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_delimiter prevu8='" << (const char*)prevu8
                    << "' for delimstr='" << delimstr << "' with curuc#" << (int)curuc
                    << " at " << startpos << " loopcnt#" << loopcnt);
      if (!prevu8)
        break;
      unsigned curlen = delimstr.size();
      unsigned prevlen = (delimstr.c_str()+delimstr.size() - (const char*)prevu8) +1;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_delimiter for delimstr='"
                    << delimstr <<"' curlen=" << curlen
                    << " prevlen=" << prevlen
                    << " prevu8='" << Rps_Cjson_String((const char*)prevu8)
                    << "'");
      if (prevlen==0 || prevlen > curlen)
        break;
      delimstr[prevlen] = (char)0; /// facilitates debugging, in principle useless
      delimstr.resize(prevlen);
      RPS_DEBUG_LOG(REPL, "get_delimiter now delimstr='" << delimstr << "' at " << startpos
                    << " prevlen=" << prevlen << " curlen=" << curlen
                    << " loopcnt#" << loopcnt << std::endl);
      usleep (250000);
    }
  RPS_WARNOUT("Rps_TokenSource::get_delimiter failing at " << startpos
              << " for " << startp << " in " << *this << std::endl
              << " git " << rps_gitid << " timestamp " << rps_timestamp
              << " delimstr='"  << Rps_Cjson_String(delimstr)
              << "'"
              << std::endl << " host:" << rps_hostname()
              << std::endl << " procversion:" << rps_get_proc_version()
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::get_delimiter-fail"));
  std::string failmsg {"Rps_TokenSource::get_delimiter failing at "};
  failmsg += startpos;
  throw std::runtime_error{failmsg};
} // end Rps_TokenSource::get_delimiter



std::string
Rps_TokenSource::lex_quoted_literal_string(Rps_CallFrame*callframe)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  std::string rstr;
  const char* curp = curcptr();
  size_t linelen = toksrc_linebuf.size();
  const char*eol = curp + (linelen-toksrc_col);
  rstr.reserve(4+2*(eol-curp)/3);
  RPS_ASSERT(*curp == '"');
  toksrc_col++, curp++;
  char curch = 0;
  while ((curch=(*curp)) != (char)0)
    {
      if (curch == '"')
        {
          toksrc_col++;
          return rstr;
        }
      else if (curch == '\\')
        {
          char nextch = curp[1];
          char shortbuf[16];
          memset(shortbuf, 0, sizeof(shortbuf));
          switch (nextch)
            {
            case '\'':
            case '\"':
            case '\\':
              rstr.push_back (nextch);
              toksrc_col += 2;
              continue;
            case 'a':
              rstr.push_back ('\a');
              toksrc_col += 2;
              continue;
            case 'b':
              rstr.push_back ('\b');
              toksrc_col += 2;
              continue;
            case 'f':
              rstr.push_back ('\f');
              toksrc_col += 2;
              continue;
            case 'e':                   // ESCAPE
              rstr.push_back ('\033');
              toksrc_col += 2;
              continue;
            case 'n':
              rstr.push_back ('\n');
              toksrc_col += 2;
              continue;
            case 'r':
              rstr.push_back ('\r');
              toksrc_col += 2;
              continue;
            case 't':
              rstr.push_back ('\t');
              toksrc_col += 2;
              continue;
            case 'v':
              rstr.push_back ('\v');
              toksrc_col += 2;
              continue;
            case 'x':
            {
              int p = -1;
              int c = 0;
              if (sscanf (curp + 2, "%02x%n", &c, &p) > 0 && p > 0)
                {
                  rstr.push_back ((char)c);
                  toksrc_col += p + 2;
                  continue;
                }
              else
                goto lexical_error_backslash;
            }
            case 'u': // four hexdigit escape Unicode
            {
              int p = -1;
              int c = 0;
              if (sscanf (curp + 2, "%04x%n", &c, &p) > 0 && p > 0)
                {
                  int l =
                    u8_uctomb ((uint8_t *) shortbuf, (ucs4_t) c, sizeof(shortbuf));
                  if (l > 0)
                    {
                      rstr.append(shortbuf);
                      toksrc_col += p + l;
                      continue;
                    }
                  else
                    goto lexical_error_backslash;
                }
              else
                goto lexical_error_backslash;
            }
            case 'U': // eight hexdigit escape Unicode
            {
              int p = -1;
              int c = 0;
              if (sscanf (curp + 2, "%08x%n", &c, &p) > 0 && p > 0)
                {
                  int l =
                    u8_uctomb ((uint8_t *) shortbuf, (ucs4_t) c, sizeof(shortbuf));
                  if (l > 0)
                    {
                      rstr.append(shortbuf);
                      toksrc_col += p + l;
                      continue;
                    }
                  else
                    goto lexical_error_backslash;
                }
            }
            default:
              goto lexical_error_backslash;
            } // end switch nextch
        } // end if curch is '\\'
      else if (curch >= ' ' && curch < 0x7f)
        {
          rstr.push_back (curch);
          toksrc_col++;
          curp++;
          continue;
        }
      /// accepts any correctly encoded UTF-8
      else if (int l = u8_mblen ((uint8_t *)(curp), eol-curp); l>0)
        {
          rstr.append(curp, l);
          toksrc_col += l;
          curp += l;
          continue;
        }
      /// improbable lexical error....
      else
        {
          RPS_WARNOUT("Rps_TokenSource::lex_quoted_literal_string : lexical error at "
                      << position_str() << " in " << *this);
          throw std::runtime_error("lexical error");
        }
    } // end while
lexical_error_backslash:
  RPS_WARNOUT("Rps_TokenSource::lex_quoted_literal_string  : bad backslash escape at "
              << position_str() << " in " << *this);
  throw std::runtime_error("lexical bad backslash escape");
} // end Rps_TokenSource::lex_quoted_literal_string

std::string
Rps_TokenSource::lex_raw_literal_string(Rps_CallFrame*callframe)
{
  std::string result;
#warning some code from rps_lex_raw_literal_string file repl_rps.cc lines 1050-1115 should go here
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  const char* curp = curcptr();
  //size_t linelen = toksrc_linebuf.size();
  /// For C++, raw literal strings are multi-line, and explained in
  /// en.cppreference.com/w/cpp/language/string_literal ... For
  /// example R"delim(raw characters \)delim" In RefPerSys, we
  /// restrict the <delim> to contain only letters, up to 15 of
  /// them...
  char delim[16];
  memset (delim, 0, sizeof(delim));
  int pos= -1;
  int startlineno= toksrc_line;
  int startcolno= toksrc_col;
  std::string locname = toksrc_name;
  if (sscanf(curp,  "R\"%15[A-Za-z](%n", delim, &pos) < 1
      || !isalpha(delim[0])
      || pos<=1)
    /// should never happen
    RPS_FATALOUT("corrupted Rps_TokenSource::lex_raw_literal_string "
                 << Rps_QuotedC_String(curp) << " in " << *this
                 << std::endl
                 << Rps_ShowCallFrame(callframe));
  toksrc_col += pos;
  RPS_ASSERT(strlen(delim)>0 && strlen(delim)<15);
  char endstr[24];
  memset(endstr, 0, sizeof(endstr));
  snprintf(endstr, sizeof(endstr), ")%s\"", delim);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_raw_literal_string start L" << startlineno
                << ",C" << startcolno
                << "@" << toksrc_name
                << " endstr " << endstr << " in " << *this);
  const char*endp = nullptr;
  while ((curp = curcptr()) != nullptr
         && (endp=strstr(curp, endstr)) == nullptr)
    {
      std::string reststr{curp};
      if (!get_line())
        {
          RPS_WARNOUT("Rps_TokenSource::lex_raw_literal_string without end of string "
                      << endstr
                      << " starting L" << startlineno
                      << ",C" << startcolno
                      << "@" << toksrc_name
                      << std::endl
                      << Rps_ShowCallFrame(callframe)
                      << " in " << *this);
          throw std::runtime_error(std::string{"lex_raw_literal_string failed to find "}
                                   + endstr);
        }
      result += reststr;
    };                          // end while curp....
  if (endp)
    toksrc_col += endp - curp;
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_raw_literal_string gives "
                << Rps_QuotedC_String(result)
                << " at " << position_str() << " in " << *this);
  return result;
} // end Rps_TokenSource::lex_raw_literal_string


Rps_Value
Rps_TokenSource::lex_code_chunk(Rps_CallFrame*callframe)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obchunk;
                           Rps_Value namev;
                           Rps_Value res;
                           Rps_Value chunkelemv;
                );
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  struct Rps_ChunkData_st chkdata = {};
  chkdata.chunkdata_magic = rps_chunkdata_magicnum;
  chkdata.chunkdata_lineno = toksrc_line;
  chkdata.chunkdata_colno = toksrc_col;
  chkdata.chunkdata_name = toksrc_name;
  const char* curp = curcptr();
  _f.namev= source_name_val(&_);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_code_chunk start " << *this << " curp:" << Rps_QuotedC_String(curp));
  RPS_ASSERT(curp != nullptr && *curp != (char)0);
  char startchunk[12];
  memset(startchunk, 0, sizeof(startchunk));
  int pos= -1;
  if (!strncmp(curp, "#{", 2))
    {
      pos = 2;
      startchunk[0] = (char)0;
      strcpy(chkdata.chunkdata_endstr, "}#");
    }
  else if (sscanf(curp,  "#%6[a-zA-Z]{%n", startchunk, &pos)>0 && pos>0 && isalpha(startchunk[0]))
    {
      snprintf(chkdata.chunkdata_endstr, sizeof(chkdata.chunkdata_endstr), "}%s#", startchunk);
    }
  else // should never happen
    RPS_FATALOUT("corrupted Rps_TokenSource::lex_code_chunk @ " << position_str()
                 << " " << curp
                 << std::endl);
  _f.obchunk =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                               nullptr);
  _f.obchunk->put_attr2(RPS_ROOT_OB(_1B7ITSHTZWp00ektj1), //input∈symbol
                        _f.namev,
                        RPS_ROOT_OB(_5FMX3lrhiw601iqPy5), //line∈symbol
                        Rps_Value((intptr_t)chkdata.chunkdata_lineno, Rps_Value::Rps_IntTag{})
                       );
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_code_chunk @ " << position_str()
                << " start obchunk:" << _f.obchunk
                << " endstr:'" << chkdata.chunkdata_endstr << "'");
  auto paylvec = _f.obchunk->put_new_plain_payload<Rps_PayloadVectVal>();
  RPS_ASSERT(paylvec);
  int oldline = -1;
  int oldcol = -1;
  toksrc_col += strlen(chkdata.chunkdata_endstr);
  chkdata.chunkdata_colno += strlen(chkdata.chunkdata_endstr);
  do
    {
      oldline = toksrc_line;
      oldcol = toksrc_col;
      _f.chunkelemv = lex_chunk_element(&_, _f.obchunk, &chkdata);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_code_chunk @ " << name()
                    << ":L" << toksrc_line << ",C" << toksrc_col
                    << std::endl
                    << " obchunk=" << _f.obchunk
                    << ", chunkelemv=" << _f.chunkelemv);
      if (_f.chunkelemv)
        paylvec->push_back(_f.chunkelemv);
      /// for $. chunkelemv is null, but position changed...
      ///
      /// possibly related:
      /// https://framalistes.org/sympa/arc/refpersys-forum/2020-12/msg00036.html
    }
  while (_f.chunkelemv || toksrc_col>oldcol || toksrc_line>oldline);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_code_chunk " << " in " << *this
                << " :-◑> obchunk=" << _f.obchunk << " @!" << position_str()
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  return _f.obchunk;
} // end of Rps_TokenSource::lex_code_chunk



Rps_Value
Rps_TokenSource::lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg, Rps_ChunkData_st*chkdata)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(chkdata && chkdata->chunkdata_magic == rps_chunkdata_magicnum);
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_Value res;
                           Rps_ObjectRef obchunk;
                           Rps_ObjectRef namedob;
                );
  _f.obchunk = obchkarg;
  RPS_DEBUG_LOG(LOW_REPL, "Rps_TokenSource::lex_chunk_element chunkdata_colno=" << chkdata->chunkdata_colno
                << " curpos:" << position_str()
                << " linebuf:'" << toksrc_linebuf << "' of size:" << toksrc_linebuf.size());
  RPS_ASSERT(chkdata->chunkdata_colno>=0
             && chkdata->chunkdata_colno <= (int)toksrc_linebuf.size());
  const char*pc = toksrc_linebuf.c_str() + chkdata->chunkdata_colno;
  const char*eol =  toksrc_linebuf.c_str() +  toksrc_linebuf.size();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element pc='" << Rps_Cjson_String(pc) << "'"
                << " @" << position_str(chkdata->chunkdata_colno));
  if (!pc || pc[0] == (char)0 || pc == eol)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element end-of-line");
      toksrc_col = toksrc_linebuf.size();
      return nullptr;
    }
  // name-like chunk element
  if (isalpha(*pc))
    {
      /// For C name-like things, we return the object naming them or else a string
      int startnamecol =  chkdata->chunkdata_colno;
      const char*startname = pc;
      const char*endname = pc;
      const char*eol =  toksrc_linebuf.c_str() +  toksrc_linebuf.size();
      while ((isalnum(*endname) || *endname=='_') && endname<eol)
        endname++;
      std::string curname(startname, endname - startname);
      _f.namedob = Rps_ObjectRef::find_object_or_null_by_string(&_, curname);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element curname='" << curname
                    << "' in " << position_str(startnamecol)
                    << " namedob=" << _f.namedob);
      chkdata->chunkdata_colno += endname - startname;
      if (_f.namedob)
        _f.res = Rps_ObjectValue(_f.namedob);
      else
        _f.res = Rps_StringValue(curname);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element curname='" << curname
                    << "' res=" << _f.res << " at " << position_str(startnamecol));

      return _f.res;
    }
  // integer (base 10) chunk element
  else if (isdigit(*pc) || (pc[0] == '-' && isdigit(pc[1])))
    {
      char* endnum = nullptr;
      long long ll = strtoll(pc, &endnum, 10);
      int startcol =  chkdata->chunkdata_colno ;
      chkdata->chunkdata_colno += endnum - pc;
      _f.res = Rps_Value((intptr_t)ll,
                         Rps_Value::Rps_IntTag{});
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element number=" << ll
                    << " res=" << _f.res << " at " << position_str(startcol));
      return _f.res;
    }
  /// For sequence of spaces, we return an instance of class space and
  /// value the number of space characters
  else if (isspace(*pc))
    {
      int startspacecol = chkdata->chunkdata_colno;
      int endspacecol = startspacecol;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element start space obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      while (pc<eol && isspace(*pc))
        endspacecol++, pc++;
      _f.res = Rps_InstanceValue(RPS_ROOT_OB(_2i66FFjmS7n03HNNBx), //space∈class
                                 std::initializer_list<Rps_Value>
      {
        Rps_Value((intptr_t)(endspacecol-startspacecol),
        Rps_Value::Rps_IntTag{})
      });
      chkdata->chunkdata_colno += endspacecol-startspacecol;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element obchunk=" << _f.obchunk
                    << " -> number res=" << _f.res
                    << " @" << position_str(startspacecol)
                    << " now chunking @ " << position_str(chkdata->chunkdata_colno));
      return _f.res;
    }
  /// code chunk meta-variable or meta-notation....
  else if (*pc == '$' && pc < eol)
    {
      int startcol = chkdata->chunkdata_colno;
      // a dollar followed by a name is a meta-variable; that name should be known
      if (isalpha(pc[1]))
        {
          const char*startname = pc+1;
          const char*endname = startname;
          while (endname < eol && (isalnum(*endname) || *endname == '_'))
            endname++;;
          std::string metaname(startname, endname-startname);
          _f.namedob = Rps_ObjectRef::find_object_or_null_by_string(&_, metaname);
          if (!_f.namedob)
            {
              RPS_WARNOUT("lex_chunk_element: unknown metavariable name " << metaname
                          << " in " << position_str(startcol));
              throw std::runtime_error(std::string{"lexical error - bad metaname "} + metaname + " in code chunk");
            }
          chkdata->chunkdata_colno += (endname-startname) + 1;
          _f.res = Rps_InstanceValue(RPS_ROOT_OB(_1oPsaaqITVi03OYZb9), //meta_variable∈symbol
                                     std::initializer_list<Rps_Value> {_f.namedob});
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element space obchunk=" << _f.obchunk
                        << " -> metavariable res=" << _f.res
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.res;
        }
      /// two dollars are parsed as one
      else if (pc[1]=='$')
        {
          chkdata->chunkdata_colno += 2;
          _f.res = Rps_StringValue("$");
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element dollar obchunk=" << _f.obchunk
                        << " -> dollar-string res=" << _f.res
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.res;
        }
    }
  //// end of chunk is }letters# or }#
  else if (*pc == '}' && !strcmp(pc, chkdata->chunkdata_endstr))
    {
      chkdata->chunkdata_colno += strlen(chkdata->chunkdata_endstr);
      _f.res = nullptr;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element end-of-chunk obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      toksrc_col += strlen(chkdata->chunkdata_endstr);
      return nullptr;
    }
  //// any other sequence of UTF-8 excluding right brace } or dollar sign $; we stop lexing when letter, digit, space
  else
    {
      RPS_ASSERT(eol != nullptr && eol >= pc);
      //size_t restsiz = eol - pc;
      const char *startpc = pc;
      const uint8_t* curu8p = (const uint8_t*)pc;
      const uint8_t* eolu8p = (const uint8_t*)eol;
      while (curu8p < eolu8p)
        {
          if (*(const char*)curu8p == '}' || *(const char*)curu8p == '$')
            break;
          if (isspace(*pc))
            break;
          if (isalnum(*pc))
            break;
          int u8len = u8_mblen(curu8p, eolu8p - curu8p);
          if (u8len <= 0)
            break;
          curu8p += u8len;
          pc += u8len;
        };
      std::string str{startpc, (unsigned) (curu8p-(const uint8_t*)startpc)};
      _f.res = Rps_StringValue(str);
      chkdata->chunkdata_colno += str.size();
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element strseq obchunk=" << _f.obchunk
                    << " -> plain-string res=" << _f.res
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      return _f.res;
    }
#warning Rps_TokenSource::lex_chunk_element should parse delimiters...
  RPS_FATALOUT("unimplemented Rps_TokenSource::lex_chunk_element obchunk=" << _f.obchunk << " @ " << name()
               << ":L" << toksrc_line << ",C" << toksrc_col);
#warning unimplemented Rps_TokenSource::lex_chunk_element, see rps_lex_chunk_element in repl_rps.cc:1229-1415
} // end Rps_TokenSource::lex_chunk_element





Rps_Value
Rps_TokenSource::lookahead_token(Rps_CallFrame*callframe, unsigned rank)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_Value lextokv;
                );
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token start rank#" << rank << " in " << (*this) << std::endl
                << "… pos:" << position_str()
                << " token_deq:" << toksrc_token_deq << std::endl
                << "… curcptr:" << Rps_QuotedC_String(curcptr())
                //<< " called from:" << std::endl << Rps_ShowCallFrame(&_)
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::lookahead_token start")
                << std::endl);
  RPS_ASSERT(_.call_frame_depth() < 32);
  while (rank >= toksrc_token_deq.size())
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token loop rank#"
                    << rank << " in " << (*this) << " pos:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()));
      _f.lextokv = get_token(&_);
      if (_f.lextokv)
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token got-token-pushing lextokv:" << _f.lextokv
                        << " pos:" << position_str()
                        << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::lookahead_token pushing"));
          toksrc_token_deq.push_back(_f.lextokv);
        }
      else
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token rank#" << rank
                        << " (get_token/fail) missing from:"
                        << std::endl
                        << "… pos:" << position_str()
                        << " token_deq:" << toksrc_token_deq << std::endl << Rps_ShowCallFrame(&_));
          return nullptr;
        }
    };                          // end while rank >= toksrc_token_deq.size()
  //
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token rank#" << rank
                << " pos:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << "… in " << *this << " token_deq:" << toksrc_token_deq);
  if (rank<toksrc_token_deq.size())
    {
      _f.lextokv = toksrc_token_deq[rank];
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token rank#" << rank << " => " << _f.lextokv);
      return _f.lextokv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token§FAIL rank#" << rank << " missing,"
                << " pos:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl << RPS_DEBUG_BACKTRACE_HERE(1, "Rps_TokenSource::lookahead_token FAIL")
                << std::endl
                << "… in " << *this
                << " token_deq:" << toksrc_token_deq);
  return nullptr;
} // end Rps_TokenSource::lookahead_token




void
Rps_TokenSource::consume_front_token(Rps_CallFrame*callframe, bool*psuccess)
{
  RPS_ASSERT(rps_is_main_thread());
  static unsigned callcnt;
  callcnt++;
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::consume_front_token#" << callcnt <<" START from:"
                << std::endl << Rps_ShowCallFrame(callframe)
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::consume_front_token start")
                << " this-token:" << (*this) << " token_deq:" << toksrc_token_deq
                << " pos:" << position_str()
                << std::endl
                << "… curcptr:" << Rps_QuotedC_String(curcptr()) << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  ////
  if (toksrc_token_deq.empty())
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::consume_front_token#" << callcnt
                    <<" FAIL" << std::endl
                    << "… in " << *this
                    << std::endl << Rps_ShowCallFrame(callframe)
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      })
          << std::endl
          << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::consume_front_token FAIL"));
      if (psuccess==nullptr)
        throw std::runtime_error("Rps_TokenSource::consume_front_token without any queued token");
      else
        *psuccess=false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::consume_front_token#" << callcnt <<" fail€"
                    << " pos:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()) << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return;
    };
  ////
  RPS_ASSERT(!toksrc_token_deq.empty());
  toksrc_token_deq.pop_front();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::consume_front_token#" << callcnt
                << " done€, now token_deq:" << toksrc_token_deq
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::consume_front_token/done€")
                << " pos:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr()) << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (psuccess!=nullptr)
    *psuccess = true;
} // end Rps_TokenSource::consume_front_token


void
Rps_TokenSource::append_back_new_token(Rps_CallFrame*callframe, Rps_Value tokenv)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_Value lextokv;
                );
  _f.lextokv = tokenv;
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::append_back_new_token called from:" << std::endl << Rps_ShowCallFrame(&_)
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::append_back_new_token start")
                << std::endl
                << " this-token:" << (*this) << " token_deq:" << toksrc_token_deq
                << " tokenv:" << _f.lextokv);
  RPS_ASSERT (_f.lextokv && _f.lextokv.is_lextoken());
  toksrc_token_deq.push_back(tokenv);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::append_back_new_token done€ token_deq=" << toksrc_token_deq
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::append_back_new_token/done€"));
} // end Rps_TokenSource::append_back_new_token

extern "C" void rps_run_test_repl_lexer(const std::string&);

void
rps_run_test_repl_lexer(const std::string& teststr)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_0S6DQvp3Gop015zXhL),  //lexical_token∈class
                           /*callerframe:*/RPS_NULL_CALL_FRAME,
                           Rps_Value curlextokenv;
                );
  RPS_ASSERT(rps_is_main_thread());

  RPS_TIMER_START();
  Rps_StringTokenSource toktestsrc(teststr, "*test-repl-lexer*");
  bool gotl = toktestsrc.get_line();
  RPS_DEBUG_LOG(REPL, "start rps_run_test_repl_lexer gitid " << rps_gitid
                << " teststr: " << Rps_QuotedC_String(teststr)
                << " callframe:" << Rps_ShowCallFrame(&_)
                << " toktestsrc:" << toktestsrc
                << (gotl?" got line": " no line"));
  int tokcnt=0;
  int lincnt = 0;
  int loopcnt=0;
  while (!rps_repl_stopped)
    {
      loopcnt++;
      RPS_DEBUG_LOG(REPL, "rps_run_test_repl_lexer toktestsrc:" << toktestsrc
                    << " at " << toktestsrc.position_str()
                    << " loopcnt#" << loopcnt
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        toktestsrc.display_current_line_with_cursor(out);
      }));
      do
        {
          _f.curlextokenv = toktestsrc.get_token(&_);
          if (_f.curlextokenv)
            {
              tokcnt++;
              RPS_INFORMOUT("token#" << tokcnt << ":" << _f.curlextokenv
                            << " from " << toktestsrc.position_str()
                            << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                toktestsrc.display_current_line_with_cursor(out);
              }));
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "rps_run_test_repl_lexer will stop since no token in " << toktestsrc
                            << " at:"
                            << toktestsrc.position_str());
              rps_repl_stopped= true;
              break;
            }
        }
      while (_f.curlextokenv);
      if (!toktestsrc.get_line())
        break;
      lincnt++;
      RPS_DEBUG_LOG(REPL, "rps_run_test_repl_lexer got fresh line#" << lincnt
                    << " '"
                    << Rps_Cjson_String(toktestsrc.current_line()) << "' "
                    << toktestsrc.position_str());
    }
  RPS_DEBUG_LOG(REPL, "end rps_run_test_repl_lexer lincnt=" << lincnt
                << " tokcnt=" << tokcnt
                << " at " << toktestsrc.position_str()
                << std::endl);

  RPS_TIMER_STOP(REPL);
} // end rps_test_repl_string


//// end of file lexer_rps.cc
