/****************************************************************
 * file backtrace_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It provides support for backtraces, using Ian Taylor libbacktrace
 *      from https://github.com/ianlancetaylor/libbacktrace
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright  2020 - 2025 The Reflective Persistent System Team
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


extern "C" const char rps_backtrace_gitid[];
const char rps_backtrace_gitid[]= RPS_GITID;

extern "C" const char rps_backtrace_date[];
const char rps_backtrace_date[]= __DATE__;

extern "C" const char rps_backtrace_shortgitid[];
const char rps_backtrace_shortgitid[]= RPS_SHORTGITID;

#define RPS_FASTABORT(Msg) do {                                 \
    std::clog << " RefPerSys FAST ABORT:" << __FILE__ << ':'    \
              << __LINE__  << std::endl                         \
              << "..@" << __PRETTY_FUNCTION__                   \
              << " §¤: " << Msg << std::endl << std::flush;     \
    abort();                                                    \
  } while(0)

/// actually, in file main_rps.cc we have something like  asm volatile ("rps_end_of_main: nop");
extern "C" void rps_end_of_main(void);
extern "C" int main(int, char**);

std::recursive_mutex Rps_Backtracer:: _backtr_mtx_;

/// Notice that Rps_Backtracer should use assert, not RPS_ASSERT since
/// RPS_ASSERT is recursively using Rps_Backtracer!
void
Rps_Backtracer::bt_error_method(const char*msg, int errnum)
{
  assert (msg != nullptr);
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  fprintf(stderr, "BackTrace Error [%s:%d] %s (#%d)",
          __FILE__, __LINE__,
          msg?msg:"???",
          errnum);
  fflush(nullptr);
} // end Rps_Backtracer::bt_error_method


/// both backtrace_full and backtrace_simple callbacks are continuing with a 0 return code:
enum { RPS_CONTINUE_BACKTRACE=0, RPS_STOP_BACKTRACE=1 };


void
Rps_Backtracer::bt_error_cb (void *data, const char *msg,
                             int errnum)
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  assert (data != nullptr);
  Rps_Backtracer* btp = static_cast<Rps_Backtracer*>(data);
  assert (btp->magicnum() == _backtr_magicnum_);
  btp->bt_error_method(msg, errnum);
} // end of Rps_Backtracer::bt_error_cb



void
Rps_Backtracer::output(std::ostream&outs)
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (RPS_UNLIKELY(_backtr_magicnum_ != backtr_magic))
    RPS_FASTABORT("corrupted Rps_Backtracer");
  if (&outs == &std::cerr || &outs == &std::clog)
    backtr_ontty = rps_without_terminal_escape ? false : isatty(STDERR_FILENO);
  else if (&outs == &std::cout)
    backtr_ontty = rps_without_terminal_escape ? false : isatty(STDOUT_FILENO);
  backtr_todo = Todo::Do_Output;
  backtr_depth = 0;
  auto bk = bkind();
  RPS_DEBUG_LOG(MISC, "Rps_Backtracer::output start kind " << bkindname());
  switch (bk)
    {
    case Kind::None:
      RPS_FASTABORT("unexpected None kind in Rps_Backtracer::output");
    case Kind::FullOut_Kind:
      backtr_outs = &outs;
      backtrace_full(rps_backtrace_common_state, backtr_skip,
                     backtrace_full_cb, backtrace_error_cb,
                     (void*)this);
      if (backtr_outs)
        {
          std::ostringstream& fullout = std::get<FullOut_t>(this->backtr_variant);
          RPS_ASSERT(fullout);
          fullout << std::flush;
          *backtr_outs << fullout.str() << std::flush;
        }
      backtr_todo = Todo::Do_Nothing;
      return;
    case Kind::FullClos_Kind:
      backtrace_full(rps_backtrace_common_state, backtr_skip,
                     backtrace_full_cb, backtrace_error_cb,
                     (void*)this);
      backtr_todo = Todo::Do_Nothing;
      return;
    default:
      RPS_FASTABORT("unexpected kind Rps_Backtracer::output kind=" << bkindname());
    }; // end switch bkind()
  RPS_DEBUG_LOG(MISC, "Rps_Backtracer::output end kind " << bkindname() << std::endl);
} // end Rps_Backtracer::output




void
Rps_Backtracer::print(FILE*outf)
{
  if (!outf)
    return;
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (RPS_UNLIKELY(_backtr_magicnum_ != backtr_magic))
    RPS_FASTABORT("corrupted Rps_Backtracer");
  backtr_todo = Todo::Do_Print;
  if (isatty(fileno(outf)))
    backtr_ontty = !rps_without_terminal_escape;
  else
    backtr_ontty = false;
  backtr_depth = 0;
  RPS_DEBUG_LOG(MISC, "Rps_Backtracer::print start kind " << bkindname());
  switch (bkind())
    {
    case Kind::None:
      RPS_FASTABORT("unexpected None kind in Rps_Backtracer::print");
    case Kind::FullOut_Kind:
    {
      backtrace_full(rps_backtrace_common_state, backtr_skip,
                     backtrace_full_cb, backtrace_error_cb,
                     (void*)this);
      std::ostringstream& fullout = std::get<FullOut_t>(this->backtr_variant);
      RPS_ASSERT(fullout);
      fullout << std::flush;
      fputs(fullout.str().c_str(), outf);
      fflush(outf);
      backtr_todo = Todo::Do_Nothing;
    }
    return;
    case Kind::FullClos_Kind:
      backtrace_full(rps_backtrace_common_state, backtr_skip,
                     backtrace_full_cb, backtrace_error_cb,
                     (void*)this);
      backtr_todo = Todo::Do_Nothing;
      return;
    default:
      RPS_FASTABORT("unexpected Rps_Backtracer::print kind=" << bkindname());
    }; // end switch bkind();
  RPS_DEBUG_LOG(MISC, "Rps_Backtracer::print end kind " << bkindname() << std::endl);
} // end Rps_Backtracer::print


std::string
Rps_Backtracer::pc_to_string(uintptr_t pc, bool* gotmain)
{
  if (pc == 0)
    return "◎"; //U+25CE BULLSEYE
  else if (pc == (uintptr_t)-1)
    return "";
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (RPS_UNLIKELY(_backtr_magicnum_ != backtr_magic))
    RPS_FASTABORT("pc_to_string: corrupted Rps_Backtracer");
  const char* BOLD_esc = (backtr_ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* ITALICS_esc = (backtr_ontty?RPS_TERMINAL_ITALICS_ESCAPE:"");
  const char* UNDERLINE_esc = (backtr_ontty?RPS_TERMINAL_UNDERLINE_ESCAPE:"");
  const char* FAINT_esc = (backtr_ontty?RPS_TERMINAL_FAINT_ESCAPE:"");
  const char* NORMAL_esc = (backtr_ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  if (gotmain)
    *gotmain = false;
  if (RPS_UNLIKELY(pc < 0xffff))
    {
      char buf[32];
      memset (buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "%s%s[%03d] /??%s %p",
               ITALICS_esc,  BOLD_esc, backtr_depth,  NORMAL_esc,
               (void*)pc);
      return std::string(buf);
    }
  else // perhaps legitimate pc
    {
      std::ostringstream outs;
      {
        char beforebuf[32];
        memset (beforebuf, 0, sizeof(beforebuf));
        snprintf (beforebuf, sizeof(beforebuf), "[%03d]", backtr_depth);
        outs << ITALICS_esc <<  BOLD_esc
             << beforebuf << NORMAL_esc;
      }
      const char*demangled = nullptr;
      Dl_info dif = {};
      memset ((void*)&dif, 0, sizeof(dif));
      if (dladdr((void*)pc, &dif))
        {
          std::string filnamestr;
          std::string funamestr;
          int delta = pc - (uintptr_t) dif.dli_saddr;
          if (dif.dli_fname && strstr(dif.dli_fname, ".so"))
            filnamestr = std::string(basename(dif.dli_fname));
          else if (dif.dli_fname && strstr(dif.dli_fname, rps_progname))
            filnamestr = std::string(basename(rps_progname));
          if (dif.dli_sname != nullptr)
            {
              if (dif.dli_sname[0] == '_')
                {
                  int status = -1;
                  demangled  = abi::__cxa_demangle(dif.dli_sname, nullptr, 0, &status);
                  if (demangled && demangled[0])
                    funamestr = std::string (demangled);
                };
              if (!strncmp(dif.dli_sname, RPS_APPLYINGFUN_PREFIX,
                           sizeof(RPS_APPLYINGFUN_PREFIX)-1))
                {
                  char nambuf[80];
                  memset (nambuf, 0, sizeof(nambuf));
                  const char* pend=nullptr;
                  bool ok = false;
                  Rps_Id oid(dif.dli_sname
                             + sizeof(RPS_APPLYINGFUN_PREFIX),//
                             &pend, &ok);
                  if (ok && oid)
                    {
                      Rps_ObjectZone* obzf = Rps_ObjectZone::find(oid);
                      if (obzf)
                        {
                          Rps_Value namev =
                            obzf-> get_physical_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); //name∈named_attribute
                          if (namev && namev.is_string())
                            {
                              u8_strncpy((uint8_t*)nambuf,
                                         (const uint8_t*)(namev.as_cstring()),
                                         sizeof(nambuf)-1);
                            }
                        }
                      if (nambuf[0])
                        funamestr = std::string(dif.dli_sname) + ":"
                                    + std::string(nambuf);
                      else
                        funamestr = std::string(dif.dli_sname) + "?";
                    }
                }
              if (funamestr.empty())
                funamestr = std::string(dif.dli_sname);
            }
          else funamestr = "??";
          if (delta != 0)
            {
              if (funamestr.empty())
                {
                  outs <<  ' ' << (void*)pc
                       << "!: " << filnamestr
                       << "+" << std::showbase << std::hex << delta
                       << NORMAL_esc << std::flush;
                }
              else
                {
                  outs << ' ' << (void*)pc
                       << "!: " << filnamestr
                       << "+" << std::showbase << std::hex << delta
                       << ' ' << funamestr
                       << NORMAL_esc << std::flush;
                }
            } // end if delta != 0
          else // delta is 0
            {
              if (funamestr.empty())
                {
                  outs <<  ' ' << (void*)pc
                       << "!: " << filnamestr
                       << std::flush;
                }
              else
                {
                  outs << ' ' << (void*)pc
                       << "!: " << filnamestr
                       << ' ' << funamestr << std::flush;
                }
            } // end if delta is 0
          if (demangled)
            free((void*)demangled);
          if (gotmain && funamestr == std::string("main"))
            *gotmain = true;
        }
      else // dladdr failed
        {
          outs << " §:" << (void*)pc << '?' << std::flush;
        }
      return outs.str();
    } // endif perhaps legitimate pc
} // end Rps_Backtracer::pc_to_string





std::string
Rps_Backtracer::detailed_pc_to_string(uintptr_t pc, const char*pcfile, int pclineno,
                                      const char*pcfun)
{
  if (pc == 0)
    return "○"; //U+25CB WHITE CIRCLE
  else if (pc == (uintptr_t)-1)
    return "";
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (RPS_UNLIKELY(_backtr_magicnum_ != backtr_magic))
    RPS_FASTABORT("detailed_pc_to_string: corrupted Rps_Backtracer");
  const char* BOLD_esc = (backtr_ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* ITALICS_esc = (backtr_ontty?RPS_TERMINAL_ITALICS_ESCAPE:"");
  const char* UNDERLINE_esc = (backtr_ontty?RPS_TERMINAL_UNDERLINE_ESCAPE:"");
  const char* FAINT_esc = (backtr_ontty?RPS_TERMINAL_FAINT_ESCAPE:"");
  const char* NORMAL_esc = (backtr_ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  if (pcfile && pcfile[0] && pcfun && pcfun[0])
    {
      const char*basepcfile = pcfile;
      {
        const char*lastslash = strrchr(pcfile, '/');
        if (lastslash && lastslash[1])
          basepcfile = lastslash+1;
      }
      std::ostringstream outs;
      {
        char beforebuf[32];
        memset (beforebuf, 0, sizeof(beforebuf));
        snprintf (beforebuf, sizeof(beforebuf), "[%03d]", backtr_depth);
        outs << ITALICS_esc << beforebuf << ' ';
      }
      char *dempcfun = nullptr;
      if (pcfun[0] == '_')
        {
          int status = -1;
          const char*demangled =  abi::__cxa_demangle(pcfun, nullptr, 0, &status);
          if (demangled && status==0)
            dempcfun = (char*) demangled;
        }
      outs << ITALICS_esc << basepcfile << ':' << pclineno << "°:" << NORMAL_esc << " "
           << UNDERLINE_esc << (dempcfun?dempcfun:pcfun) << NORMAL_esc
           << " "
           << FAINT_esc << "@" << (void*)pc << NORMAL_esc
           << std::flush;
      if (dempcfun)
        free((void*)dempcfun);
      return outs.str();
    }
  else // some of pcfile or pcfun is missing
    return pc_to_string(pc);
} // end Rps_Backtracer::detailed_pc_to_string



Rps_Backtracer::Rps_Backtracer(struct FullOut_Tag,
                               const char*fromfil, const int fromlin, int skip,
                               const char*name, std::ostream* out)
  :
  backtr_magic(_backtr_magicnum_),
  backtr_todo(Todo::Do_Nothing),
  backtr_ontty(out?false:!rps_without_terminal_escape),
  backtr_mainthread(rps_is_main_thread()),
  backtr_gotlast(false),
  backtr_variant(std::ostringstream{}),
  backtr_outs(out),
  backtr_fromfile(fromfil),
  backtr_fromline(fromlin),
  backtr_skip(skip),
  backtr_depth(0),
  backtr_name(name)
{

  if (bkind() != Kind::FullOut_Kind)
    RPS_FASTABORT("corrupted Rps_Backtracer::Rps_Backtracer/FullOutTag kind=" << bkindname());
} // end Rps_Backtracer::Rps_Backtracer/FullOut_Tag


////////////////
Rps_Backtracer::Rps_Backtracer(struct FullClos_Tag,
                               const char*fromfil, const int fromlin,  int skip,
                               const char*name,
                               const std::function<bool(Rps_Backtracer&bt,  uintptr_t pc,
                                   const char*pcfile, int pclineno,
                                   const char*pcfun)>& fun)
  : backtr_magic(_backtr_magicnum_),
    backtr_todo(Todo::Do_Nothing),
    backtr_ontty(false),
    backtr_mainthread(rps_is_main_thread()),
    backtr_gotlast(false),
    backtr_variant(fun),
    backtr_outs(nullptr),
    backtr_fromfile(fromfil),
    backtr_fromline(fromlin),
    backtr_skip(skip),
    backtr_depth(0),
    backtr_name(name)
{
  if (bkind() != Kind::FullClos_Kind)
    RPS_FASTABORT("corrupted Rps_Backtracer::Rps_Backtracer/FullClos_Tag");
} // end Rps_Backtracer::Rps_Backtracer/FullClos_Tag



const std::string
Rps_Backtracer::bkindname(void) const
{
  auto k = bkind();
  switch (k)
    {
    case Kind::None:
      return "None";
    case Kind::FullOut_Kind:
      return "FullOut";
    case Kind::FullClos_Kind:
      return "FullClos";
    }
  char buf[32];
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "?kind#%d?", (int)k);
  return buf;
} // end Rps_Backtracer::bkindname


std::ostream*
Rps_Backtracer::boutput(void) const
{
  if (magicnum() != _backtr_magicnum_)
    RPS_FASTABORT("corrupted backtracer");
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  auto k = bkind();
  switch (k)
    {
    case Kind::None:
      RPS_FASTABORT("Rps_Backtracer::boutput with Kind::None");
    case Kind::FullOut_Kind:
      return backtr_outs;
    case Kind::FullClos_Kind:
      RPS_WARNOUT("unimplemented Rps_Backtracer::boutput for kind #" << (int)bkind()
                  << ":" << bkindname());
      return nullptr;
    }
  return nullptr;
} // end Rps_Backtracer::boutput



/// this static method is the backtrace_simple_callback passed to
/// backtrace_simple.  See comment inside backtrace.h header.
int
Rps_Backtracer::backtrace_simple_cb(void*data, uintptr_t pc)
{
  // this backtrace_simple_cb function is passed to backtrace_simple
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (!data)
    RPS_FASTABORT("corruption - no data");
  Rps_Backtracer* bt = reinterpret_cast<Rps_Backtracer*>(data);
  if (bt->magicnum() != _backtr_magicnum_)
    RPS_FASTABORT("corrupted backtracer");
  bt->backtr_depth++;
  if (!pc)
    return RPS_STOP_BACKTRACE;
  if (bt->backtr_gotlast)
    return RPS_STOP_BACKTRACE;
  switch (bt-> backtr_todo)
    {
    case Todo::Do_Nothing:
      RPS_FASTABORT("backtrace_simple_cb unexpected Todo::Do_Nothing");
    case Todo::Do_Output:
      switch (bt->bkind())
        {
        case Kind::None:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output unexpected Kind::None");
        case Kind::FullOut_Kind:
        {
          std::ostringstream& fullout = std::get<FullOut_t>(bt->backtr_variant);
          RPS_ASSERT(fullout);
          bool gotmain = false;
          fullout << bt->pc_to_string(pc, &gotmain) << std::endl;
          if (pc >= (uintptr_t)main && pc <= (uintptr_t)rps_end_of_main)
            return RPS_CONTINUE_BACKTRACE;
          if (gotmain && bt->bmainthread())
            {
              bt->backtr_gotlast = true;
            };
          return RPS_CONTINUE_BACKTRACE;
        }
        case Kind::FullClos_Kind:
        {
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output unexpected Kind::FullClos");
        }
        default:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output bad kind " << bt->bkindname());
        }
      break;
    case Todo::Do_Print:
      switch (bt->bkind())
        {
        case Kind::None:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print unexpected Kind::None");
        case Kind::FullOut_Kind:
        {
          std::ostringstream& fullout = std::get<FullOut_t>(bt->backtr_variant);
          RPS_ASSERT(fullout);
          bool gotmain = false;
          std::string str = bt->pc_to_string(pc, &gotmain);
          fullout << str;
          if (pc >= (uintptr_t)main && pc <= (uintptr_t)rps_end_of_main)
            bt->backtr_gotlast = true;
          if (gotmain && bt->bmainthread())
            bt->backtr_gotlast = true;
          return RPS_CONTINUE_BACKTRACE;
        }
        case Kind::FullClos_Kind:
        {
          FullClos_t fullclo = std::get<FullClos_t>(bt->backtr_variant);
          RPS_ASSERT(fullclo);
          bool ok = fullclo(*bt, pc,
                            /*pcfile:*/(const char*)nullptr,
                            /*pclineno:*/(int)0,
                            /*pcfun:*/(const char*)nullptr);
          if (pc >= (uintptr_t)main && pc <= (uintptr_t)rps_end_of_main)
            {
              bt->backtr_gotlast = true;
            };
          if (ok)
            return RPS_CONTINUE_BACKTRACE;
          else
            return RPS_STOP_BACKTRACE;
        }
        default:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print bad kind " << bt->bkindname());
        }
      break;
    default:
      RPS_FASTABORT("bad todo in Rps_Backtracer::backtrace_simple_cb");
    }
} // end Rps_Backtracer::backtrace_simple_cb



/// This static method is the backtrace_full_callback passed to
/// backtrace_full.  See comment inside backtrace.h header.
int
Rps_Backtracer::backtrace_full_cb(void *data, uintptr_t pc,
                                  const char *filename, int lineno,
                                  const char *function)
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (!data)
    RPS_FASTABORT("corruption - no data");
  Rps_Backtracer* bt = reinterpret_cast<Rps_Backtracer*>(data);
  if (bt->magicnum() != _backtr_magicnum_)
    RPS_FASTABORT("corrupted backtracer");
  if (!pc)
    return RPS_STOP_BACKTRACE;
  if (bt->backtr_gotlast)
    return RPS_STOP_BACKTRACE;
  bt->backtr_depth++;
  switch (bt-> backtr_todo)
    {
    case Todo::Do_Nothing:
      RPS_FASTABORT("backtrace_full_cb unimplemented Todo::Do_Nothing");
    case Todo::Do_Output:
      switch (bt->bkind())
        {
        case Kind::None:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unexpected Kind::None");
        case Kind::FullOut_Kind:
        {
          std::ostringstream& fullout = std::get<FullOut_t>(bt->backtr_variant);
          RPS_ASSERT(fullout);
          fullout << bt->detailed_pc_to_string(pc,filename,lineno,function) << std::endl;
          if (pc >= (uintptr_t)main && pc <= (uintptr_t)rps_end_of_main)
            bt->backtr_gotlast = true;
          if (filename && function && !strcmp(function, "main")
              && strstr(filename, "main_rps"))
            bt->backtr_gotlast = true;
          return RPS_CONTINUE_BACKTRACE;
        }
        case Kind::FullClos_Kind:
        {
          auto fullclo = std::get<FullClos_t>(bt->backtr_variant);
          RPS_ASSERT(fullclo);
          bool ok = fullclo(*bt, pc, filename, lineno,  function);
          if (pc >= (uintptr_t)main && pc <= (uintptr_t)rps_end_of_main)
            bt->backtr_gotlast = true;
          if (ok && filename && function && !strcmp(function, "main")
              && strstr(filename, "main_rps"))
            bt->backtr_gotlast = true;
          return RPS_CONTINUE_BACKTRACE;
        }
        default:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print bad kind");
        }
      break;
    case Todo::Do_Print:
      switch (bt->bkind())
        {
        case Kind::None:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unexpected Kind::None");
        case Kind::FullOut_Kind:
        {
          std::ostringstream& fullout = std::get<FullOut_t>(bt->backtr_variant);
          RPS_ASSERT(fullout);
        }
        RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unexpected Kind::FullOut_Kind");
        case Kind::FullClos_Kind:
        {
          auto fullclo = std::get<FullClos_t>(bt->backtr_variant);
          RPS_ASSERT(fullclo);
          fullclo(*bt, pc, filename, lineno,  function);
          if (pc >= (uintptr_t)main && pc <= (uintptr_t)rps_end_of_main)
            bt->backtr_gotlast = true;
          if (filename && function && !strcmp(function, "main")
              && strstr(filename, "main_rps"))
            bt->backtr_gotlast = true;
          return RPS_CONTINUE_BACKTRACE;
        }
        default:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print bad kind");
        }
      break;
    }
  RPS_FASTABORT("unimplemented Rps_Backtracer::backtrace_full_cb");
} // end Rps_Backtracer::backtrace_full_cb

void
Rps_Backtracer::backtrace_error_cb(void* data, const char*msg, int errnum)
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  RPS_FASTABORT("Rps_Backtracer::backtrace_error_cb data=" << data << ", msg=" << msg << ", errnum=" << errnum);
} // end Rps_Backtracer::backtrace_error_cb



Rps_Backtracer::~Rps_Backtracer()
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (this->magicnum() != _backtr_magicnum_)
    RPS_FASTABORT("corrupted backtracer");
  if (backtr_outs)
    {
      *backtr_outs << "-----++-----";
      if (!backtr_fromfile.empty())
        *backtr_outs << "/ " << backtr_fromfile << ":" << backtr_fromline;
      *backtr_outs << " -- " << backtr_name;
      *backtr_outs << std::endl;
    }
} // end Rps_Backtracer::~Rps_Backtracer



//////////////////////////////////////////////////////////////// eof backtrace_rps.cc
