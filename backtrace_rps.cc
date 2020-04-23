/****************************************************************
 * file backtrace_rps.cc
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
 *      © Copyright  2020 The Reflective Persistent System Team
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
#include "qthead_qrps.hh"


extern "C" const char rps_backtrace_gitid[];
const char rps_backtrace_gitid[]= RPS_GITID;

extern "C" const char rps_backtrace_date[];
const char rps_backtrace_date[]= __DATE__;

#define RPS_FASTABORT(Msg) do {					\
  fprintf(stderr, "%s:%d <%s> RefPerSys FAST ABORT: %s (%m)\n",	\
	__FILE__, __LINE__, __FUNCTION__, (Msg));		\
  fflush(stderr);						\
  abort();							\
} while(0)

std::recursive_mutex Rps_Backtracer:: _backtr_mtx_;

/// notice that Rps_Backtracer should use assert, not RPS_ASSERT!
void
Rps_Backtracer::bt_error_method(const char*msg, int errnum)
{
  assert (msg != nullptr);
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  fprintf(stderr, "BackTrace Error <%s:%d> %s (#%d)",
          __FILE__, __LINE__,
          msg?msg:"???",
          errnum);
  fflush(nullptr);
} // end Rps_Backtracer::bt_error_method



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
  backtr_todo = Todo::Do_Output;
  backtr_depth = 0;
  switch (bkind())
    {
    case Kind::None:
      RPS_FASTABORT("unexpected None kind in Rps_Backtracer::output");
    case Kind::SimpleOut:
    case Kind::SimpleClosure:
      backtrace_simple(rps_backtrace_common_state, backtr_skip,
                       backtrace_simple_cb, backtrace_error_cb,
                       (void*)this);
      backtr_todo = Todo::Do_Nothing;
      return;
    case Kind::StringOut:
    case Kind::FullClosure:
      backtrace_full(rps_backtrace_common_state, backtr_skip,
                     backtrace_full_cb, backtrace_error_cb,
                     (void*)this);
      backtr_todo = Todo::Do_Nothing;
      return;
    }; // end switch bkind()
  RPS_FASTABORT("unexpected kind Rps_Backtracer::output");
} // end Rps_Backtracer::output




void
Rps_Backtracer::print(FILE*outf)
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (RPS_UNLIKELY(_backtr_magicnum_ != backtr_magic))
    RPS_FASTABORT("corrupted Rps_Backtracer");
  backtr_todo = Todo::Do_Print;
  backtr_depth = 0;
  switch (bkind())
    {
    case Kind::None:
      RPS_FASTABORT("unexpected None kind in Rps_Backtracer::print");
    case Kind::SimpleOut:
    case Kind::SimpleClosure:
      backtrace_simple(rps_backtrace_common_state, backtr_skip,
                       backtrace_simple_cb, backtrace_error_cb,
                       (void*)this);
      backtr_todo = Todo::Do_Nothing;
      return;
    case Kind::StringOut:
    case Kind::FullClosure:
      backtrace_full(rps_backtrace_common_state, backtr_skip,
                     backtrace_full_cb, backtrace_error_cb,
                     (void*)this);
      backtr_todo = Todo::Do_Nothing;
      return;
    }; // end switch bkind()
  RPS_FASTABORT("unexpected kind Rps_Backtracer::print");
} // end Rps_Backtracer::print


std::string
Rps_Backtracer::pc_to_string(uintptr_t pc)
{
  if (pc == 0)
    return "◎"; //U+25CE BULLSEYE
  else if (pc == (uintptr_t)-1)
    return "";
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (RPS_UNLIKELY(_backtr_magicnum_ != backtr_magic))
    RPS_FASTABORT("pc_to_string: corrupted Rps_Backtracer");
  if (RPS_UNLIKELY(pc < 0xffff))
    {
      char buf[32];
      memset (buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf), "[%03d] /?? %p", backtr_depth, (void*)pc);
      return std::string(buf);
    }
  else // perhaps legitimate pc
    {
      std::ostringstream outs;
      char beforebuf[32];
      memset (beforebuf, 0, sizeof(beforebuf));
      snprintf (beforebuf, sizeof(beforebuf), "[%03d]", backtr_depth);
      {
        outs << beforebuf;
      };
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
                  demangled  = abi::__cxa_demangle(dif.dli_sname, NULL, 0, &status);
                  if (demangled && demangled[0])
                    funamestr = std::string (demangled);
                };
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
                       << std::flush;
                }
              else
                {
                  outs << ' ' << (void*)pc
                       << "!: " << filnamestr
                       << "+" << std::showbase << std::hex << delta
                       << ' ' << funamestr << std::flush;
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
        }
      else // dladdr failed
        {
          outs << beforebuf << ' ' << (void*)pc << '?' << std::flush;
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
  if (pcfile && pcfile[0] && pcfun && pcfun[0])
    {
      char linbuf[256];
      memset(linbuf, 0, sizeof(linbuf));
      int endpos= -1;
      // probably the string fits into linbuf
      if (RPS_LIKELY(snprintf(linbuf, sizeof(linbuf), "* %s:%d <%s> @%p%n",
                              pcfile, pclineno, pcfun, (void*)pc, &endpos) >0
                     && endpos > 0 && endpos<(int)sizeof(linbuf)-1))
        return std::string(linbuf);
      else
        {
          char numbuf[32];
          memset(numbuf, 0, sizeof(numbuf));
          std::string s;
          s.reserve(strlen(pcfile)+strlen(pcfun)+32);
          s += "* ";
          s += pcfile;
          s += "°:";
          s += std::to_string(pclineno);
          s += " <";
          s += pcfun;
          s += ">";
          snprintf(numbuf, sizeof(numbuf), " @%p", (void*)pc);
          s += numbuf;
          return s;
        }
    }
  else
    return pc_to_string(pc);
} // end Rps_Backtracer::detailed_pc_to_string



Rps_Backtracer::Rps_Backtracer(struct SimpleOutTag,
                               const char*fromfil, const int fromlin, int skip,
                               const char*name, std::ostream* out)
  :
  backtr_magic(_backtr_magicnum_),
  backtr_todo(Todo::Do_Nothing),
  backtr_variant(out?out:&std::clog),
  backtr_fromfile(fromfil),
  backtr_fromline(fromlin),
  backtr_skip(skip),
  backtr_depth(0),
  backtr_name(name)
{
} // end Rps_Backtracer::Rps_Backtracer/SimpleOutTag



Rps_Backtracer::Rps_Backtracer(struct SimpleClosureTag,
                               const char*fromfil, const int fromlin,  int skip,
                               const char*name,
                               const std::function<void(Rps_Backtracer&,  uintptr_t pc)>& fun)
  :
  backtr_todo(Todo::Do_Nothing),
  backtr_magic(_backtr_magicnum_),
  backtr_variant(fun),
  backtr_fromfile(fromfil),
  backtr_fromline(fromlin),
  backtr_skip(skip),
  backtr_depth(0),
  backtr_name(name)
{
} // end Rps_Backtracer::Rps_Backtracer/SimpleClosureTag

Rps_Backtracer::Rps_Backtracer(struct FullOutTag,
                               const char*fromfil, const int fromlin, int skip,
                               const char*name,  std::ostream* out)
  :
  backtr_todo(Todo::Do_Nothing),
  backtr_magic(_backtr_magicnum_),
  backtr_variant(out),
  backtr_fromfile(fromfil),
  backtr_fromline(fromlin),
  backtr_skip(skip),
  backtr_depth(0),
  backtr_name(name)
{
} // end Rps_Backtracer::Rps_Backtracer/FullOutTag

Rps_Backtracer::Rps_Backtracer(struct FullClosureTag,
                               const char*fromfil, const int fromlin,  int skip,
                               const char*name,
                               const std::function<void(Rps_Backtracer&bt,  uintptr_t pc,
                                   const char*pcfile, int pclineno,
                                   const char*pcfun)>& fun)
  : backtr_todo(Todo::Do_Nothing),
    backtr_magic(_backtr_magicnum_),
    backtr_variant(fun),
    backtr_fromfile(fromfil),
    backtr_fromline(fromlin),
    backtr_skip(skip),
    backtr_depth(0),
    backtr_name(name)
{
} // end Rps_Backtracer::Rps_Backtracer/FullClosureTag


const std::string
Rps_Backtracer::bkindname(void) const
{
  auto k = bkind();
  switch (k)
    {
    case Kind::None:
      return "None";
    case Kind::SimpleOut:
      return "SimpleOut";
    case Kind::StringOut:
      return "StringOut";
    case Kind::SimpleClosure:
      return "SimpleClosure";
    case Kind::FullClosure:
      return "FullClosure";
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
  auto k = bkind();
  switch (k)
    {
    case Kind::None:
      RPS_FASTABORT("Rps_Backtracer::boutput with Kind::None");
    case Kind::SimpleOut:
      return std::get<std::ostream*>(backtr_variant);
    case Kind::StringOut:
      return &std::get<std::ostringstream>(backtr_variant);
    case Kind::SimpleClosure:
    case Kind::FullClosure:
      RPS_WARNOUT("unimplemented Rps_Backtracer::boutput for kind #" << (int)bkind()
                  << ":" << bkindname());
      return nullptr;
    }
} // end Rps_Backtracer::boutput



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
  switch (bt-> backtr_todo)
    {
    case Todo::Do_Nothing:
      RPS_FASTABORT("backtrace_simple_cb unexpected Todo::Do_Nothing");
    case Todo::Do_Output:
      switch (bt->bkind())
        {
        case Kind::None:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output unexpected Kind::None");
        case Kind::SimpleOut:
        case Kind::StringOut:
        {
          std::ostream* outs = bt->boutput();
          if (outs)
            {
              *outs << bt->pc_to_string(pc) << std::endl;
              return 0;
            }
          else
            RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output without output");
        }
        break;
        case Kind::SimpleClosure:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output unimplemented Kind::SimpleClosure");
        case Kind::FullClosure:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output unexpected Kind::FullClosure");
        default:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Output bad kind");
        }
      break;
    case Todo::Do_Print:
      switch (bt->bkind())
        {
        case Kind::None:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print unexpected Kind::None");
        case Kind::SimpleOut:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print unimplemented Kind::SimpleOut");
        case Kind::SimpleClosure:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print unimplemented Kind::SimpleClosure");
        case Kind::StringOut:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print unexpected Kind::StringOut");
        case Kind::FullClosure:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print unexpected Kind::FullClosure");
        default:
          RPS_FASTABORT("backtrace_simple_cb Todo::Do_Print bad kind");
        }
      break;
    default:
      RPS_FASTABORT("bad todo in Rps_Backtracer::backtrace_simple_cb");
    }
} // end Rps_Backtracer::backtrace_simple_cb


int
Rps_Backtracer::backtrace_full_cb(void *data, uintptr_t pc,
                                  const char *filename, int lineno,
                                  const char *function)
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
#warning unimplemented Rps_Backtracer::backtrace_full_cb
  if (!data)
    RPS_FASTABORT("corruption - no data");
  Rps_Backtracer* bt = reinterpret_cast<Rps_Backtracer*>(data);
  if (bt->magicnum() != _backtr_magicnum_)
    RPS_FASTABORT("corrupted backtracer");
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
        case Kind::SimpleOut:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unimplemented Kind::SimpleOut");
        case Kind::SimpleClosure:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unimplemented Kind::SimpleClosure");
        case Kind::StringOut:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unexpected Kind::StringOut");
        case Kind::FullClosure:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unexpected Kind::FullClosure");
        default:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print bad kind");
        }
      break;
    case Todo::Do_Print:
      switch (bt->bkind())
        {
        case Kind::None:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unexpected Kind::None");
        case Kind::SimpleOut:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unimplemented Kind::SimpleOut");
        case Kind::SimpleClosure:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unimplemented Kind::SimpleClosure");
        case Kind::StringOut:
          RPS_FASTABORT("backtrace_full_cb Todo::Do_Print unexpected Kind::StringOut");
        case Kind::FullClosure:
        {
          auto fullclo = std::get<fullclos_t>(bt->backtr_variant);
          RPS_ASSERT(fullclo);
          fullclo(*bt, pc, filename, lineno,  function);
          return 0;
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
#warning unimplemented Rps_Backtracer::backtrace_error_cb
  RPS_FASTABORT("unimplemented Rps_Backtracer::backtrace_error_cb");
} // end Rps_Backtracer::backtrace_error_cb



Rps_Backtracer::~Rps_Backtracer()
{
  std::lock_guard<std::recursive_mutex> gu(_backtr_mtx_);
  if (this->magicnum() != _backtr_magicnum_)
    RPS_FASTABORT("corrupted backtracer");
#warning please carefully review Rps_Backtracer::~Rps_Backtracer
#if 0
  switch (bkind())
    {
    case Kind::None:
      RPS_FASTABORT("backtrace_simple_cb unexpected Kind::None");
    case Kind::SimpleOut:
      delete backtr_out;
      return;
    case Kind::StringOut:
      (&backtr_outstr)->~std::ostringstream();
      return;
    case Kind::SimpleClosure:
      backtr_simpleclos.~std::function<void(Rps_Backtracer&,  uintptr_t pc)> ();
      return;
    case Kind::FullClosure:
      backtr_fullclos.~std::function<void(Rps_Backtracer&,  uintptr_t pc,
                                          const char*pcfile, int pclineno,
                                          const char*pcfun)> ();
      return;
    default:
      RPS_FASTABORT("backtrace_simple_cb bad kind");
    }
#endif
} // end Rps_Backtracer::~Rps_Backtracer










#if 0 //////////////////////////// ************************** old
void
oldrps_print_simple_backtrace_level(OldRps_BackTrace* btp, FILE*outf, const char*beforebuf, uintptr_t pc)
{
  if (btp == nullptr || btp->magicnum() != OldRps_BackTrace::_bt_magicnum_)
    RPS_FATAL("bad btp@%p", (void*)btp);
  if (!outf)
    RPS_FATAL("missing outf");
  if (!beforebuf)
    beforebuf = "*";
  if (pc==0 || pc==(uintptr_t)-1)
    {
      fprintf(outf, "%s *********\n", beforebuf);
      return;
    }
  const char*demangled = nullptr;
  Dl_info dif = {};
  memset ((void*)&dif, 0, sizeof(dif));
  std::string filnamestr;
  std::string funamestr;
  if (dladdr((void*)pc, &dif))
    {
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
              demangled  = abi::__cxa_demangle(dif.dli_sname, NULL, 0, &status);
              if (demangled && demangled[0])
                funamestr = std::string (demangled);
            };
          if (funamestr.empty())
            funamestr = std::string(dif.dli_sname);
        }
      else funamestr = "??";
      if (delta != 0)
        {
          if (funamestr.empty())
            fprintf (outf, "%s %p: %s+%#x\n",
                     beforebuf, (void*)pc, filnamestr.c_str(), delta);
          else
            fprintf (outf, "%s %p: %40s+%#x %s\n",
                     beforebuf, (void*)pc, filnamestr.c_str(), delta,
                     funamestr.c_str());
        }
      else
        {
          if (funamestr.empty())
            fprintf (outf, "%s %p: %s+%#x\n",
                     beforebuf, (void*)pc, filnamestr.c_str(), delta);
          else
            fprintf (outf, "%s %p: %40s+%#x %s\n",
                     beforebuf, (void*)pc, filnamestr.c_str(),
                     delta, funamestr.c_str());
        }
    }
  else
    fprintf(outf, "%s %p.\n", beforebuf, (void*)pc);
  if (demangled)
    free((void*)demangled), demangled = nullptr;
  fflush(outf);
} // end of oldrps_print_simple_backtrace_level




void oldrps_print_full_backtrace_level
(OldRps_BackTrace* btp,
 FILE*outf, const char*beforebuf,
 uintptr_t pc, const char *filename, int lineno,
 const char *function)
{
  if (btp == nullptr || btp->magicnum() != OldRps_BackTrace::_bt_magicnum_)
    RPS_FATAL("bad btp@%p", (void*)btp);
  if (!outf)
    RPS_FATAL("missing outf");
  if (!beforebuf)
    beforebuf = "*";
  if (pc==0 || pc==(uintptr_t)-1)
    {
      fprintf(outf, "%s *********\n", beforebuf);
      return;
    }
  std::string locstr;
  std::string funamestr;
  char locbuf[80];
  Dl_info dif = {};
  const char* demangled = nullptr;
  bool wantdladdr = false;
  memset ((void*)&dif, 0, sizeof(dif));
  if (function)
    {
      if (function[0] == '_')
        {
          int status = -1;
          demangled  = abi::__cxa_demangle(function, NULL, 0, &status);
          if (demangled && demangled[0])
            funamestr = std::string (demangled);
        }
      else funamestr = std::string (function);
    }
  else
    wantdladdr = true;
  memset(locbuf, 0, sizeof(locbuf));
  if (filename)
    {
      snprintf(locbuf, sizeof(locbuf), "%s:%d",
               basename(filename), lineno);
      locstr = std::string(locbuf);
    }
  else
    wantdladdr = true;
  if (wantdladdr && dladdr((void*)pc, &dif))
    {
      int delta = pc - (uintptr_t) dif.dli_saddr;
      if (locstr.empty() && dif.dli_fname && strstr(dif.dli_fname, ".so"))
        {
          if (delta != 0)
            snprintf(locbuf, sizeof(locbuf),
                     "!%s+%#x", basename(dif.dli_fname), delta);
          else snprintf(locbuf, sizeof(locbuf),
                          "!%s", basename(dif.dli_fname));
          locstr = std::string (locbuf);
        }
      else if (locstr.empty() && dif.dli_fname && strstr(dif.dli_fname, rps_progname))
        {
          if (delta != 0)
            snprintf(locbuf, sizeof(locbuf),
                     "!!%s+%#x", basename(dif.dli_fname), delta);
          else snprintf(locbuf, sizeof(locbuf),
                          "!!%s", basename(dif.dli_fname));
          locstr = std::string (locbuf);
        }
      if (funamestr.empty())
        {
          if (dif.dli_sname && dif.dli_sname[0] == '_')
            {
              int status = -1;
              demangled  = abi::__cxa_demangle(dif.dli_sname, NULL, 0, &status);
              if (demangled && demangled[0])
                funamestr = std::string (demangled);
            }
          else if (dif.dli_sname)
            funamestr = std::string(dif.dli_sname);
          if (funamestr.empty())
            {
              funamestr = std::string("?_?");
            }
        }
    };
  fprintf(outf, "%s %p %s %s\n",
          beforebuf, (void*)pc, funamestr.c_str(), locstr.c_str());
  fflush(outf);
  if (demangled)
    free((void*)demangled), demangled=nullptr;
} // end of oldrps_print_full_backtrace_level



int
OldRps_BackTrace::bt_simple_method(uintptr_t ad)
{
  if (ad == 0 || ad == (uintptr_t)-1) return 0;
  if (_bt_simplecb)
    return _bt_simplecb(this,ad);
  oldrps_print_simple_backtrace_level(this,stderr,"*",ad);
  return 1;
} // end of  OldRps_BackTrace::bt_simple_method

int
OldRps_BackTrace::bt_simple_cb(void*data, uintptr_t pc)
{
  assert (data != nullptr);
  if (pc == 0 || pc == (uintptr_t)-1) return 1;
  OldRps_BackTrace* btp = static_cast<OldRps_BackTrace*>(data);
  assert (btp->magicnum() == _backtr_magicnum_);
  return btp->bt_simple_method(pc);
} // end  OldRps_BackTrace::bt_simple_cb


int
OldRps_BackTrace::bt_full_method(uintptr_t pc,
                                 const char *filename, int lineno,
                                 const char *function)
{
  if (pc == 0 || pc == (uintptr_t)-1)
    return 1;
  if (_bt_fullcb)
    return _bt_fullcb(this,pc,filename,lineno,function);
  oldrps_print_full_backtrace_level(this,stderr,"*",
                                    pc,filename,lineno,function);
  return 0;
} // end OldRps_BackTrace::bt_full_method

int
OldRps_BackTrace::bt_full_cb(void *data, uintptr_t pc,
                             const char *filename, int lineno,
                             const char *function)
{
  assert (data != nullptr);
  if (pc == 0 || pc == (uintptr_t)-1) return 1;
  OldRps_BackTrace* btp = static_cast<OldRps_BackTrace*>(data);
  assert (btp->magicnum() == _bt_magicnum_);
  return btp->bt_full_method(pc, filename, lineno, function);
} // end OldRps_BackTrace::bt_full_cb

OldRps_BackTrace::OldRps_BackTrace(const char*name, const void*data)
  : _bt_magic(_bt_magicnum_),
    _bt_name(name?name:"??"),
    _bt_simplecb(),
    _bt_fullcb(),
    _bt_data(data)
{
} // end of OldRps_BackTrace::OldRps_BackTrace

OldRps_BackTrace::~OldRps_BackTrace()
{
  assert (magicnum() == _bt_magicnum_);
  _bt_data = nullptr;
} // end OldRps_BackTrace::~OldRps_BackTrace


void
OldRps_BackTrace::run_simple_backtrace(int skip, const char*name)
{
  OldRps_BackTrace bt(name?name:"plain");
  int depthcount = 0;
  bt.set_simple_cb([&,stderr](OldRps_BackTrace*btp, uintptr_t pc)
  {
    assert (btp != nullptr && btp-> magicnum() ==  _bt_magicnum_);
    depthcount++;
    if (depthcount > (int)_bt_maxdepth_)
      {
        fprintf(stderr, "\n.... etc .....\n");
        return depthcount;
      }
    if (depthcount %5 == 0)
      fputc('\n', stderr);
    char countbuf[16];
    memset (countbuf, 0, sizeof(countbuf));
    snprintf(countbuf, sizeof(countbuf), "[%d] ", depthcount);
    oldrps_print_simple_backtrace_level(btp, stderr, countbuf, pc);
    fflush(stderr);
    return 0;
  });
  fprintf(stderr, "SIMPLE BACKTRACE (%s)\n", bt.name().c_str());
  bt.simple_backtrace(skip);
  fprintf(stderr, "***** end of %d simple backtrace levels (%s) *****\n",
          depthcount, bt.name().c_str());
  fflush(stderr);
} // end OldRps_BackTrace::run_simple_backtrace


void
OldRps_BackTrace::run_full_backtrace(int skip, const char*name, std::ostream&out)
{
  OldRps_BackTrace bt(name?name:"full");
  int depthcount = 0;
  bt.set_full_cb([&,stderr](OldRps_BackTrace*btp, uintptr_t pc,
                            const char*filnam, int lineno, const char*funam)
  {
    assert (btp != nullptr && btp-> magicnum() ==  _bt_magicnum_);
    if (pc == 0 || pc == (uintptr_t)-1)
      return -1;
    depthcount++;
    if (depthcount > (int)_bt_maxdepth_)
      {
        fprintf(stderr, "\n.... etc .....\n");
        return depthcount;
      }
    if (depthcount %5 == 0)
      fputc('\n', stderr);
    char countbuf[16];
    memset (countbuf, 0, sizeof(countbuf));
    snprintf(countbuf, sizeof(countbuf), "[%d] ", depthcount);
    oldrps_print_full_backtrace_level(btp, stderr, countbuf,
                                      pc, filnam, lineno, funam);
    return 0;
  });
  char thnam[40];
  memset(thnam, 0, sizeof(thnam));
  pthread_getname_np(pthread_self(), thnam, sizeof(thnam)-1);
  fprintf(stderr, "FULL BACKTRACE (%s) <%s>\n", bt.name().c_str(), thnam);
  bt.full_backtrace(skip);
  fprintf(stderr, "***** end of %d full backtrace levels (%s) *****\n",
          depthcount, bt.name().c_str());
  fflush(stderr);
} // end OldRps_BackTrace::run_full_backtrace



OldRps_BackTrace_Helper::OldRps_BackTrace_Helper(const char*fil, int line, int skip, const char*name)
  : _bth_magic(_bth_magicnum_), _bth_count(0),
    _bth_lineno(line), _bth_skip(skip),
    _bth_outs(),
    _bth_filename(fil),
    _bth_outptr(nullptr),
    _bth_backtrace(name,(void*)this)
{
  _bth_backtrace.set_full_cb
  ([=](OldRps_BackTrace*btp, uintptr_t pc, const char*filnam, int lineno, const char*funam)
  {
    if (pc == 0 || pc == (uintptr_t)-1)
      return -1;
    if (!_bth_outs)
      return -1;
    OldRps_BackTrace_Helper*bth =
      reinterpret_cast<OldRps_BackTrace_Helper*>(const_cast<void*>(btp->data()));
    assert (bth != nullptr && bth->has_good_magic());
    assert (btp == &bth->_bth_backtrace);
    bth->_bth_count++;
    char countbuf[16];
    memset (countbuf, 0, sizeof(countbuf));
    snprintf(countbuf, sizeof(countbuf), "[%d] ", bth->_bth_count);
    oldrps_print_full_backtrace_level(&bth->_bth_backtrace, foutlin, countbuf, pc, filnam, lineno, funam);
    fputc((char)0, foutlin);
    fflush(foutlin);
    *_bth_out << _bth_bufptr << std::endl;
    fclose(foutlin);
    return 0;
  });
} // end OldRps_BackTrace_Helper::OldRps_BackTrace_Helper

void
OldRps_BackTrace_Helper::do_out(std::ostream& out)
{
  _bth_backtrace.run_full_backtrace(_bth_skip, out);
} // end OldRps_BackTrace_Helper::do_out
#endif /************ old code ************/
