/****************************************************************
 * file repl_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the Read-Eval-Print-Loop code but don't use GNU readline
 *      anymore in Feb. 2024 (after commit 5b20c981b9e)
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


#include "wordexp.h"

extern "C" const char rps_repl_gitid[];
const char rps_repl_gitid[]= RPS_GITID;

extern "C" const char rps_repl_date[];
const char rps_repl_date[]= __DATE__;

extern "C" const char rps_repl_shortgitid[];
const char rps_repl_shortgitid[]= RPS_SHORTGITID;


std::vector<std::string> rps_completion_vect;

/// a C++ closure for getting the REPL lexical token.... with
/// lookahead=0, next token, with lookahead=1 the second-next token
std::function<Rps_LexTokenValue(Rps_CallFrame*,unsigned)> rps_repl_cmd_lexer_fun;

/// these REPL lexical tokens are looked ahead, so we need a function
/// to consume them... Returning true when the leftmost token is
/// forgotten
std::function<bool(Rps_CallFrame*)> rps_repl_consume_cmd_token_fun;

bool rps_repl_stopped;

std::string
rps_repl_version(void)
{

  std::string res = "REPL";
  {
    char gitstart[48];
    memset (gitstart, 0, sizeof(gitstart));
    strncpy(gitstart, rps_repl_gitid, sizeof(gitstart)-2);
    res += " git ";
    res += gitstart;
  }
  return res;
} // end rps_repl_version


extern "C" void
rps_do_builtin_repl_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd, Rps_TokenSource& intoksrc,
                            const char*title);

extern "C" rps_applyingfun_t rpsapply_repl_not_implemented;

Rps_TwoValues
rpsapply_repl_not_implemented(Rps_CallFrame*callerframe,
                              const Rps_Value arg0,
                              const Rps_Value arg1,
                              const Rps_Value arg2,
                              const Rps_Value arg3,
                              const std::vector<Rps_Value>* restargs)
{
  if (!restargs)
    RPS_WARNOUT("rpsapply_repl_not_implemented arg0:" << arg0
                << " arg1:" << arg1
                << " arg2:" << arg2
                << " arg3:" << arg3
                << std::endl
                << " from caller frame:"
                << Rps_ShowCallFrame(callerframe)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_repl_not_implemented"));
  else
    {
      auto nbargs = restargs->size();
      std::ostringstream outs;
      for (int aix = 0; aix < (int)nbargs; aix++)
        {
          outs << " rest#" << aix << ":" << restargs->at(aix);
        }
      outs.flush();
      RPS_WARNOUT("rpsapply_repl_not_implemented arg0:" << arg0
                  << " arg1:" << arg1
                  << " arg2:" << arg2
                  << " arg3:" << arg3
                  << std::endl
                  << outs.str()
                  << std::endl
                  << " from caller frame:"
                  << Rps_ShowCallFrame(callerframe)
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_repl_not_implemented/many"));
    }
  return {nullptr,nullptr};
} // end rpsapply_repl_not_implemented




/// Create a new REPL command, and output to stdout some draft C++
/// code to parse it.... To be called from the main thread.
void
rps_repl_create_command(Rps_CallFrame*callframe, const char*commandname)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_4CZZ2JlnkQT02YJ6sM), //repl_command∈symbol
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obsymb;
                           Rps_ObjectRef obcommand;
                           Rps_ObjectRef obfun;
                           Rps_ObjectRef obreplcmdclass;
                           Rps_Value closv;
                           Rps_Value strnamev;
                );
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame(callframe));
  RPS_ASSERT(commandname != nullptr);
  _f.obreplcmdclass = RPS_ROOT_OB(_8CncrUdoSL303T5lOK); //repl_command∈class
  bool goodname = isalpha(commandname[0]);
  for (const char*pc = commandname; *pc && goodname; pc++)
    goodname = isalnum(*pc)
               || (pc>commandname && *pc == '_' && pc[-1] != '_');
  if (!goodname)
    {
      RPS_WARNOUT("rps_repl_create_command invalid command name " << commandname << std::endl
                  << ".. called from " <<  Rps_ShowCallFrame(&_));
      std::string msg = "invalid REPL command name ";
      msg += commandname;
      throw std::runtime_error(msg);
    };
  _f.strnamev = Rps_StringValue(commandname);
  _f.obsymb = Rps_ObjectRef::make_new_strong_symbol(&_, std::string(commandname));
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> obsymb=" << _f.obsymb);
  RPS_ASSERT(_f.obsymb);
  Rps_PayloadSymbol* paylsymb =
    _f.obsymb->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT(paylsymb);
  /// the command name should be a fresh symbol...
  if (paylsymb->symbol_value())
    {
      RPS_WARNOUT("rps_repl_create_command command name "
                  << commandname << " already known as symbol " << _f.obsymb
                  << std::endl
                  << ".. called from " <<  Rps_ShowCallFrame(&_));
      return;
    }
  _f.obcommand
    = Rps_ObjectRef::make_object(&_,
                                 RPS_ROOT_OB(_8CncrUdoSL303T5lOK), //repl_command∈class
                                 Rps_ObjectRef::root_space());
  paylsymb->symbol_put_value(_f.obcommand);
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> obcommand=" << _f.obcommand);
  /* We need to create some object ObFun, of class 9Gz1oNPCnkB00I6VRS
     == core_function∈class and make a closure from it; that closure
     would be the repl_command_parser == _4I8GwXXfO3P01cdzyd of
     ObFun. We also need to output on stdout some C++ skeleton code
     for it. */
  _f.obfun
    = Rps_ObjectRef::make_object(&_,
                                 RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS), //core_function∈class
                                 Rps_ObjectRef::root_space());
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> obfun=" << _f.obfun);
  _f.strnamev = Rps_StringValue(std::string{commandname} + "°replcfun");
  _f.closv = Rps_ClosureValue(_f.obfun, {_f.obcommand,_f.obsymb});
  _f.obfun->put_attr(RPS_ROOT_OB(_8CncrUdoSL303T5lOK), //repl_command∈class
                     _f.obcommand);
  _f.obfun->put_attr(Rps_ObjectRef::the_name_object(),
                     _f.strnamev);
  _f.obfun->put_applying_function(rpsapply_repl_not_implemented);
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> closv=" << _f.closv);
  _f.obcommand->put_attr(RPS_ROOT_OB(_4I8GwXXfO3P01cdzyd), ///  repl_command_parser∈symbol
                         _f.closv);
  _f.strnamev = Rps_StringValue(commandname);
  _f.obcommand->put_attr(Rps_ObjectRef::the_name_object(),
                         _f.strnamev);
  // the symbol should be reachable at dump time and known to the command
  _f.obcommand->put_attr(Rps_ObjectRef::the_symbol_class(),
                         _f.obsymb);
  std::cout << std::endl << std::endl << std::endl
            << "/*# C++ function " << _f.obfun << " for REPL command " << commandname << "*/" << std::endl;
  std::cout << "extern \"C\" rps_applyingfun_t rpsapply" << _f.obfun->oid() << ";" << std::endl;
  std::cout << "Rps_TwoValues" << std::endl << "rpsapply"
            << _f.obfun->oid() << "(Rps_CallFrame*callerframe," << std::endl
            << "                           const Rps_Value arg0," << std::endl
            << "                           const Rps_Value arg1," << std::endl
            << "                           [[maybe_unused]] const Rps_Value arg2," << std::endl
            << "                           [[maybe_unused]] const Rps_Value arg3," << std::endl
            << "                           [[maybe_unused]] const std::vector<Rps_Value>* restargs)" << std::endl
            << "{" << std::endl
            << "   static Rps_Id descoid;\n"
            << "   if (!descoid)\n" "     descoid=Rps_Id(\"" <<  _f.obfun->oid() << "\");" << std::endl
            << "   RPS_" "LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid)," << std::endl
            << "                   callerframe," << std::endl
            << "   );" << std::endl
            << "   RPS_" "DEBUG_LOG(CMD, \"REPL command " << commandname << " start arg0=\" << arg0 << \"∈\" << arg0.compute_class(&_)"
            << std::endl
            << "                << \" arg1=\" << arg1 <<  << \"∈\" << arg1.compute_class(&_) << std::endl" << std::endl
            << "                << \" from \" << std::endl" << std::endl
            << "                << Rps_ShowCallFrame(&_));" << std::endl
            << "#warning incomplete rpsapply" << _f.obfun->oid() << " for REPL command " << commandname << std::endl
            << "  RPS_" "WARNOUT(\"incomplete rpsapply" << _f.obfun->oid() << " for REPL command " << commandname << " from \" << std::endl" << std::endl
            << "                << RPS_FULL_BACKTRACE_HERE(1, \"rpsapply" << _f.obfun->oid() << " for REPL command " << commandname << "\"));" << std::endl
            << "  return {nullptr,nullptr};" << std::endl
            << "} //end of rpsapply" << _f.obfun->oid() << " for REPL command " << commandname
            << std::endl << std::endl;
  _f.obreplcmdclass->append_comp1(Rps_ObjectValue(_f.obcommand));
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " added " << _f.obcommand << " to repl_command class " << _f.obreplcmdclass);
  /* see also OBSOLETErps_repl_interpret which would apply that closure */
#warning rps_repl_create_command incomplete
  RPS_WARNOUT("rps_repl_create_command incomplete for command "
              << commandname << " obfun=" << _f.obfun
              << " obcommand=" << _f.obcommand);
} // end rps_repl_create_command






Rps_Value
rps_lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg,  Rps_ChunkData_st*chkdata);




/// Inside a code chunk represented by object obchkarg, parse some
/// chunk element...
Rps_Value
rps_lex_chunk_element(Rps_CallFrame *callframe,
                      [[maybe_unused]] Rps_ObjectRef obchkarg, Rps_ChunkData_st *chkdata)
{
  RPS_ASSERT(chkdata != nullptr
             && chkdata->chunkdata_magic ==  rps_chunkdata_magicnum);
  RPS_ASSERT(callframe != nullptr
             && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obchunk;
                 Rps_Value chkelemv;
                 Rps_ObjectRef namedobv;
                );
#warning rps_lex_chunk_element is obsolete code
#if 0 && oldcode
  RPS_ASSERT(chkdata->chunkdata_plinebuf != nullptr);
  _f.obchunk = obchkarg;
  RPS_ASSERT(_f.obchunk);
  auto paylvect = _f.obchunk->get_dynamic_payload<Rps_PayloadVectVal>();
  RPS_ASSERT(paylvect != nullptr);
  const char*linestart = nullptr; //*chkdata->chunkdata_plinebuf;
#warning obsolete code in rps_lex_chunk_element which has no more sense
  int linelen = strlen(linestart);
  RPS_ASSERT(chkdata->chunkdata_colno >= 0 && chkdata->chunkdata_colno<linelen);
  RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start obchunk=" << _f.obchunk
                << ", linestart='" << linestart << "', linelen=" << linelen
                << " @L" << chkdata->chunkdata_lineno << ",C"
                <<  chkdata->chunkdata_colno
                << " endstr='" << chkdata->chunkdata_endstr
                <<  "' current:"
                << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                << linestart+chkdata->chunkdata_colno
                << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-start"));
  /// For C name-like things, we return the object naming them or else a string
  const char*curstr = linestart+chkdata->chunkdata_colno;
  if (isalpha(*curstr))
    {
      int startnamcol = chkdata->chunkdata_colno;
      int endnamcol = startnamcol;
      while (endnamcol<linelen && (isalnum(linestart[endnamcol]) || linestart[endnamcol]=='_'))
        endnamcol++;
      int namlen = endnamcol-startnamcol;
      std::string namstr = std::string(linestart+startnamcol, namlen);
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element obchunk=" << _f.obchunk
                    << " namstr='" << namstr << "' starting L"
                    << chkdata->chunkdata_lineno << ",C" << startnamcol
                    << " endnamcol=" << endnamcol);
      _f.namedobv = Rps_ObjectRef::find_object_by_string(&_, namstr,
                    Rps_ObjectRef::Null_When_Missing);
      chkdata->chunkdata_colno = endnamcol;
      curstr = nullptr;
      if (_f.namedobv)
        {
          RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element name obchunk=" << _f.obchunk
                        << " -> namedobv=" << _f.namedobv
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.namedobv;
        }
      else
        {
          _f.chkelemv = Rps_StringValue(namstr);
          RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element string obchunk=" << _f.obchunk
                        << " -> chkelemv=" << _f.chkelemv
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.chkelemv;
        }
    }
  /// For sequence of spaces, we return an instance of class space and value the number of space characters
  else if (isspace(*curstr))
    {
      int startspacecol = chkdata->chunkdata_colno;
      int endspacecol = startspacecol;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start space obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      while (endspacecol<linelen && isspace(linestart[endspacecol]))
        endspacecol++;
      _f.chkelemv = Rps_InstanceValue(RPS_ROOT_OB(_2i66FFjmS7n03HNNBx), //space∈class
                                      std::initializer_list<Rps_Value>
      {
        Rps_Value((intptr_t)(endspacecol-startspacecol),
        Rps_Value::Rps_IntTag{})
      });
      chkdata->chunkdata_colno += endspacecol-startspacecol+1;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element space obchunk=" << _f.obchunk
                    << " -> chkelemv=" << _f.chkelemv
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      curstr = nullptr;
      return _f.chkelemv;
    }
  /// code chunk meta-variable or meta-notation....
  else if (*curstr == '$'
           && chkdata->chunkdata_colno < linelen)
    {
      // a dollar followed by a name is a meta-variable; that name should be known
      const char*metastr = curstr;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start meta obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " endstr='" << chkdata->chunkdata_endstr
                    << "' metastr:" << metastr);
      if (isalpha(metastr[1]))
        {
          int startnameix=1, endnameix=1;
          while (isalnum(metastr[endnameix])||metastr[endnameix]=='_')
            endnameix++;
          std::string metaname(metastr+1, endnameix-startnameix);
          _f.namedobv = Rps_ObjectRef::find_object_by_string(&_, metaname,
                        Rps_ObjectRef::Null_When_Missing);
          if (!_f.namedobv)
            {
              RPS_WARNOUT("rps_lex_chunk_element: bad metavariable name " << metaname
                          << " input " <<  chkdata->chunkdata_input_name
                          << " line " <<  chkdata->chunkdata_lineno << ", column " << chkdata->chunkdata_colno);
              throw std::runtime_error("lexical error - metaname in code chunk");
            }
          chkdata->chunkdata_colno += endnameix-startnameix+1;
          _f.chkelemv = Rps_InstanceValue(RPS_ROOT_OB(_1oPsaaqITVi03OYZb9), //meta_variable∈symbol
                                          std::initializer_list<Rps_Value> {_f.namedobv});
          return _f.chkelemv;
        }
      /// two dollars are parsed as one
      else if (metastr[1] == '$')
        {
          // two dollars should be parsed as a single one, and we make that a string with following letters...
          int startnameix=1, endnameix=2;
          while (isalnum(metastr[endnameix])||metastr[endnameix]=='_')
            endnameix++;
          std::string dollname(metastr+1, endnameix-startnameix);
          chkdata->chunkdata_colno +=  endnameix-startnameix+1;
          _f.chkelemv = Rps_StringValue(dollname);
          return _f.chkelemv;
        }
      /// a dollar followed by a dot is ignored....
      else if (metastr[1] == '.')
        {
          chkdata->chunkdata_colno += 2;
          return nullptr;
        }
      /// probably other dollar things should be parsed as delimiters....
    }
  else if (*curstr=='}' && !strncmp(curstr, chkdata->chunkdata_endstr, strlen(chkdata->chunkdata_endstr)))
    {
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element ending  callframe=" << Rps_ShowCallFrame(callframe)
                    << std::endl << "… obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " curstr:"
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                    << curstr
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                    << " endstr='" << chkdata->chunkdata_endstr << "'"
                    << std::endl
                    <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-ending")
                   );
      chkdata->chunkdata_colno += strlen(chkdata->chunkdata_endstr);
      return nullptr;
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element INCOMPLETE  callframe=" << Rps_ShowCallFrame(callframe)
                    << std::endl << "… obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " curstr:"
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                    << curstr
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                    << " endstr='" << chkdata->chunkdata_endstr << "'"
                    << std::endl
                    <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-incomplete")
                   );
    }
  RPS_FATALOUT("unimplemented rps_lex_chunk_element callframe=" << Rps_ShowCallFrame(callframe)
               << " obchkarg=" << obchkarg
               << " chkdata=" << chkdata
               << " @L" << chkdata->chunkdata_lineno << ",C"
               <<  chkdata->chunkdata_colno
               << " linestart='" << linestart << "'");
#endif /*0 && obsolete*/
#warning we need to document and implement other chunk element conventions in rps_lex_chunk_element, in particular delimiters...
#warning unimplemented rps_lex_chunk_element
  return nullptr;
} // end rps_lex_chunk_element



////////////////////////////////////////////////////////////////
//// the Rps_LexTokenZone values are transient, but do need some GC support

Rps_LexTokenZone::Rps_LexTokenZone(Rps_TokenSource* tsrc, Rps_ObjectRef kindob, Rps_Value val, const Rps_String*filestringp, int line, int col)
  : Rps_LazyHashedZoneValue(Rps_Type::LexToken),
    lex_kind(kindob),
    lex_val(val),
    lex_file(filestringp),
    lex_src(tsrc),
    lex_lineno(line),
    lex_colno(col),
    lex_serial(0)
{
  RPS_ASSERT (!kindob || kindob->stored_type() == Rps_Type::Object);
  RPS_ASSERT (!filestringp || filestringp->stored_type() == Rps_Type::String);
} // end Rps_LexTokenZone::Rps_LexTokenZone

void
Rps_LexTokenZone::set_serial(unsigned serial)
{
  if (serial==0) return;
  RPS_ASSERT (lex_serial==0);
  lex_serial = serial;
} // end Rps_LexTokenZone::set_serial

Rps_LexTokenZone::~Rps_LexTokenZone()
{
  lex_kind = nullptr;
  lex_val = nullptr;
  lex_src = nullptr;
} // end Rps_LexTokenZone::~Rps_LexTokenZone

Rps_HashInt
Rps_LexTokenZone::compute_hash(void) const
{
  auto hkind = lex_kind?(lex_kind->val_hash()):0;
  auto hval = lex_val?(lex_val.valhash()):0;
  auto hfil = lex_file?(lex_file->val_hash()):0;
  // all the constants below are primes
  uint64_t h1 = (hkind * 45887) ^ (hval * 75937);
  uint64_t h2 = (hfil * 85817) + (lex_lineno * 85931 - lex_colno * 8573);
  Rps_HashInt h = (Rps_HashInt)(h1 + h2);
  if (!h)
    h = (h1&0xffff) + (h2&0xfffff) + 17;
  RPS_ASSERT(h != 0);
  return h;
} // end Rps_LexTokenZone

Rps_ObjectRef
Rps_LexTokenZone::the_lexical_token_class(void)
{
  return RPS_ROOT_OB(_0S6DQvp3Gop015zXhL);
} // end Rps_LexTokenZone::the_lexical_token_class

Rps_ObjectRef
Rps_LexTokenZone::compute_class(Rps_CallFrame*callframe) const
{
  // we need to create some lexical_token class object...
  RPS_ASSERT(callframe == nullptr || callframe->is_good_call_frame());
  return the_lexical_token_class();
} // end Rps_LexTokenZone::compute_class

void
Rps_LexTokenZone::gc_mark(Rps_GarbageCollector&gc, unsigned depth) const
{
  if (is_gcmarked(gc))
    return;
  if (RPS_UNLIKELY(depth > Rps_Value::max_gc_mark_depth))
    throw std::runtime_error("too deep Rps_LexTokenZone::gc_mark");
  if (lex_kind)
    lex_kind->gc_mark(gc,depth+1);
  if (lex_val)
    lex_val.gc_mark(gc,depth+1);
  if (lex_file)
    lex_file->gc_mark(gc,depth+1);
} // end Rps_LexTokenZone::gc_mark

void
Rps_LexTokenZone::dump_scan(Rps_Dumper*du, unsigned int) const
{
  RPS_ASSERT(du != nullptr);
} // end Rps_LexTokenZone::dump_scan

Json::Value
Rps_LexTokenZone::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  return Json::Value (Json::nullValue);
} // end Rps_LexTokenZone::dump_json


void
Rps_LexTokenZone::val_output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  if (depth > maxdepth)
    {
      out << "??";
      return;
    }
  bool showchunk = false;
  if (lex_serial>0)
    out << "LexToken#" << lex_serial << "{";
  else
    out << "LexToken{";
  if (lex_kind == RPS_ROOT_OB(_36I1BY2NetN03WjrOv)) // symbol∈class
    out<<"°symbol";
  else if (lex_kind == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b)) // int∈class
    out<<"°int";
  else if (lex_kind == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE)) //string∈class
    out<<"°string";
  else if (lex_kind == RPS_ROOT_OB(_98sc8kSOXV003i86w5)) // double∈class
    out<<"°double";
  else if (lex_kind == RPS_ROOT_OB(_3rXxMck40kz03RxRLM))   // code_chunk∈class
    {
      out<<"°code_chunk";
      if (depth==0)
        showchunk = true;
    }
  else if (lex_kind == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)) // object∈class
    out<<"°object";
  else if (lex_kind == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK)) //repl_delimiter∈class
    out<<"°delim";
  else if (!lex_kind)
    out<<"°°nul°°";
  else
    out<<"kind:" << lex_kind;
  if (depth > Rps_Value::max_output_depth || depth > maxdepth)
    {
      out << "…}";
      return;
    };
  out << ", val=";
  lex_val.output(out, depth+1,maxdepth);
  if (lex_val.is_object() && depth<=1)
    {
      Rps_ObjectRef obr = lex_val.as_object();
      if (obr)
        {
          std::unique_lock<std::recursive_mutex> guobr (*obr->objmtxptr());
          Rps_Value vname = obr->get_physical_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); //name∈named_attribute
          if (auto paylvect = obr->get_dynamic_payload<Rps_PayloadVectVal>())
            {
              unsigned vsiz = paylvect->size();
              out << "⟪";
              for (unsigned ix=0; ix<vsiz; ix++)
                {
                  if (ix>0 && ix % 4==0)
                    {
                      out << ",";
                      out << std::endl;
                      for (unsigned k=depth; k>0; k--) out << " ";
                    }
                  else if (ix>0) out << ", ";
                  out << "["<< ix << "]:";
                  paylvect->at(ix).output(out, depth+1, maxdepth);
                }
              out << "⟫";
            }
          else if (auto paylsymb = obr->get_dynamic_payload<Rps_PayloadSymbol>())
            {
              out << "symb:" << paylsymb->symbol_name();
            }
          else if (auto paylclass = obr->get_dynamic_payload<Rps_PayloadClassInfo>())
            {
              out << "class:" << paylclass->class_name_str();
            }
          else if (vname.is_string())
            {
              out << "°named" << Rps_QuotedC_String(vname.to_string()->cppstring());
            }
        }
    }
  if (showchunk)
    {
      /***
       * a code chunk is a transient object of class
       *   RPS_ROOT_OB(_3rXxMck40kz03RxRLM), code_chunk∈class and of
       *   payload a vector
       ***/
      Rps_ObjectRef obr = lex_val.as_object();
      if (obr)
        {
          std::unique_lock<std::recursive_mutex> guobr (*obr->objmtxptr());
          if (auto paylvect = obr->get_dynamic_payload<Rps_PayloadVectVal>())
            {
              unsigned vsiz = paylvect->size();
              out << "#{";
              for (unsigned ix=0; ix<vsiz; ix++)
                {
                  if (ix>0 && ix % 4==0)
                    {
                      out << ",";
                      out << std::endl;
                      for (unsigned k=depth; k>0; k--) out << " ";
                    }
                  else if (ix>0) out << ", ";
                  out << "["<< ix << "]:";
                  paylvect->at(ix).output(out, depth+1, maxdepth);
                }
              out << "}#";
            }
        }
      else
        RPS_WARNOUT("Rps_LexTokenZone::val_output code chunk without object"
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_LexTokenZone::val_output/codechunk?"));
    }
  if (lex_file)
    {
      out << ", @" << lex_file->cppstring()
          << ":" << lex_lineno << ":" << lex_colno;
    };
  out << "}";
} // end Rps_LexTokenZone::val_output


bool
Rps_LexTokenZone::equal(Rps_ZoneValue const&zv) const
{
  if (zv.stored_type() == Rps_Type::LexToken)
    {
      auto othlt = reinterpret_cast<const Rps_LexTokenZone*>(&zv);
      auto lh = lazy_hash();
      auto othlh = othlt->lazy_hash();
      if (lh != 0 && othlh != 0 && lh != othlh)
        return false;
      if (lex_file && othlt->lex_file)
        {
          if (lex_lineno != othlt->lex_lineno)
            return false;
          if (lex_colno != othlt->lex_colno)
            return false;
        };
      if (lex_kind != othlt->lex_kind)
        return false;
      if (!lex_file && !othlt->lex_file)
        return lex_val == (othlt->lex_val);
      return lex_val == othlt->lex_val && lex_file == othlt->lex_file;
    }
  return false;
} // end Rps_LexTokenZone::equal

bool
Rps_LexTokenZone::less(Rps_ZoneValue const&zv) const
{
  if (zv.stored_type() == Rps_Type::LexToken)
    {
      if (equal(zv))
        return false;
      auto othlt = reinterpret_cast<const Rps_LexTokenZone*>(&zv);
      if (!lex_file && othlt->lex_file)
        return true;
      if (!othlt->lex_file)
        return false;
      if (*lex_file < *othlt->lex_file)
        return true;
      else if (*lex_file > *othlt->lex_file)
        return false;
      if (lex_lineno < othlt->lex_lineno)
        return true;
      else if (lex_lineno > othlt->lex_lineno)
        return false;
      if  (lex_colno < othlt->lex_colno)
        return true;
      else if (lex_colno > othlt->lex_colno)
        return false;
      if (lex_kind < othlt->lex_kind)
        return true;
      else if (lex_kind > othlt->lex_kind)
        return false;
      if (lex_val < othlt->lex_val)
        return true;
      else if (lex_val > othlt->lex_val)
        return false;
      RPS_FATALOUT("Rps_LexTokenZone::less corruption: this=" << *this
                   << ", other=" << *othlt);
    }
  else
    return  Rps_Type::LexToken < zv.stored_type();
} // end Rps_LexTokenZone::less

////////////////////////////////////////////////////////////////


/// TODO: in commit 5a785d82c41da of May 16, 2024 all the builtin
/// commands are similarily implemented, the hope is this handwritten
/// C++ code to be soon and easily generated from new RefPerSys
/// objects....

void
rps_repl_builtin_shell_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                               Rps_TokenSource& intoksrc,
                               const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  RPS_INFORMOUT(std::endl << "running shell command " <<  Rps_SingleQuotedC_String(intoksrc.curcptr()));
  fflush(nullptr);
  int ret = system(intoksrc.curcptr());
  if (ret == 0)
    RPS_INFORMOUT("successful shell command " <<  Rps_SingleQuotedC_String(intoksrc.curcptr()));
  else
    RPS_WARNOUT("failed shell command " << Rps_SingleQuotedC_String(intoksrc.curcptr()) << " exited " << ret);
} // end rps_repl_builtin_shell_command

void
rps_repl_builtin_sh_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                            Rps_TokenSource& intoksrc,
                            const char*title)
{
  rps_repl_builtin_shell_command(callframe, obenvarg, builtincmd, intoksrc, title);
} // end rps_repl_builtin_sh_command

void
rps_repl_builtin_pwd_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                             Rps_TokenSource& intoksrc,
                             const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  char cwdbuf[rps_path_byte_size+4];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  char*cwd = getcwd(cwdbuf, rps_path_byte_size);
  if (cwd)
    RPS_INFORMOUT(std::endl << "working directory is " <<  cwdbuf);
  else
    RPS_WARNOUT("failed getcwd " << strerror(errno));
  fflush(nullptr);
} // end rps_repl_builtin_pwd_command


void
rps_repl_builtin_cd_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                            Rps_TokenSource& intoksrc,
                            const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  char cwdbuf[rps_path_byte_size+4];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  wordexp_t wx;
  memset(&wx, 0, sizeof(wx));
  char*cwd = getcwd(cwdbuf, rps_path_byte_size);
  if (cwd)
    RPS_INFORMOUT(std::endl << "old working directory is " <<  cwdbuf);
  else
    RPS_WARNOUT("failed getcwd " << strerror(errno));
  const char*cp = intoksrc.curcptr();
  while (cp && isspace(*cp))
    cp++;
  if (!cp)
    RPS_WARNOUT("no path given to !cd builtin");
  else
    {
      int failexp = wordexp(cp, &wx, WRDE_SHOWERR|WRDE_UNDEF);
      if (failexp)
        RPS_WARNOUT("!cd builtin failed to expand " << cp
                    << ((failexp==WRDE_BADCHAR)?": bad char"
                        : (failexp==WRDE_BADVAL)?": bad shell var"
                        : (failexp==WRDE_NOSPACE)?": out of memory"
                        : (failexp==WRDE_SYNTAX)?": shell syntax error"
                        : ": other wordexp failure"));
      if (wx.we_wordc == 0)
        RPS_WARNOUT("!cd builtin cannot expand " << cp);
      else if (wx.we_wordc > 1)
        RPS_WARNOUT("!cd builtin ambiguous expand " << cp << " to "
                    << wx.we_wordv[0] << " and " << wx.we_wordv[1]
                    << ((wx.we_wordc > 2)?" etc.":""));
      else
        {
          if (chdir(wx.we_wordv[0]))
            {
              const char*err = strerror(errno);
              RPS_WARNOUT("!cd " << cp << " failed to chdir to " << wx.we_wordv[0] << " :" << err);
            }
          else
            {
              char*cwd = getcwd(cwdbuf, rps_path_byte_size);
              if (cwd)
                RPS_INFORMOUT(std::endl << "!cd " << cp << " changed to new working directory " <<  cwdbuf);
              else
                RPS_WARNOUT("failed getcwd " << strerror(errno));
            }
        };
      wordfree(&wx);
    }
} // end rps_repl_builtin_cd_command

void
rps_repl_builtin_pid_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                             Rps_TokenSource& intoksrc,
                             const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  RPS_INFORMOUT(std::endl << "process id is " <<  (int)getpid() << " on " << rps_hostname());
  fflush(nullptr);
} // end rps_repl_builtin_pid_command

void
rps_repl_builtin_getpid_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                Rps_TokenSource& intoksrc,
                                const char*title)
{
  rps_repl_builtin_pid_command(callframe, obenvarg, builtincmd, intoksrc, title);
} // end rps_repl_builtin_getpid_command

void
rps_repl_builtin_git_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                             Rps_TokenSource& intoksrc,
                             const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  RPS_INFORMOUT(std::endl << "gitid:" << rps_gitid
                << std::endl << "topdir:" << rps_topdirectory);
  fflush(nullptr);
} // end rps_repl_builtin_git_command


void
rps_repl_builtin_pmap_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                              Rps_TokenSource& intoksrc,
                              const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  char pmapbuf[64];
  memset (pmapbuf, 0, sizeof(pmapbuf));
  fflush(nullptr);
  snprintf(pmapbuf, sizeof(pmapbuf)-2, "/usr/bin/pmap %d", (int)getpid());
  if (system(pmapbuf))
    RPS_WARNOUT("failed running " << pmapbuf << ":" << strerror(errno));
} // end rps_repl_builtin_pmap_command



void
rps_repl_builtin_pfd_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                             Rps_TokenSource& intoksrc,
                             const char*title)
{
  char pfdbuf[64];
  char pathbuf[rps_path_byte_size];
  char entbuf[rps_path_byte_size];
  std::map<int, std::string> fdmap;
  memset (pfdbuf, 0, sizeof(pfdbuf));
  memset (entbuf, 0, sizeof(entbuf));
  memset (pathbuf, 0, sizeof(pathbuf));
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  snprintf (pfdbuf, sizeof(pfdbuf), "/proc/%d/fd/", (int)getpid());
  DIR* pfdir = opendir(pfdbuf);
  struct dirent* de= nullptr;
  if (!pfdir)
    {
      RPS_WARNOUT("failed opendir " << pfdbuf << ":" << strerror(errno));
      return;
    };
  RPS_POSSIBLE_BREAKPOINT();
  for (de=readdir(pfdir); de!=nullptr; de=readdir(pfdir))
    {
      if (!isdigit(de->d_name[0]))
        continue;
      int fdnum = atoi(de->d_name);
      if (de->d_type == DT_LNK)
        {
          memset (pathbuf, 0, sizeof(pathbuf));
          snprintf(pathbuf, sizeof(pathbuf), "/proc/%d/fd/%d", (int)getpid(), fdnum);
          if (strlen(pathbuf) >= sizeof(pathbuf)-2)
            continue;
          if (readlink(pathbuf, entbuf, sizeof(entbuf))<0)
            continue;
          std::string entstr;
          entstr.assign(entbuf);
          fdmap.insert({fdnum,entstr});
        };
    }
  closedir(pfdir), pfdir = nullptr;
  RPS_POSSIBLE_BREAKPOINT();
  RPS_INFORMOUT(std::endl << fdmap.size() << " file descriptors:");
  for (auto it: fdmap)
    {
      RPS_POSSIBLE_BREAKPOINT();
      std::cout << it.first << ":" << it.second << std::endl;
    }
  RPS_POSSIBLE_BREAKPOINT();
} // end rps_repl_builtin_pfd_command



void
rps_repl_builtin_time_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                              Rps_TokenSource& intoksrc,
                              const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  RPS_INFORMOUT(std::endl << "elapsed time (seconds): " <<  rps_elapsed_real_time()
                << ", cpu time: " << rps_process_cpu_time());
} // end rps_repl_builtin_time_command


void
rps_repl_builtin_gc_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                            Rps_TokenSource& intoksrc,
                            const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  std::function<void(Rps_GarbageCollector*)> markall = [&](Rps_GarbageCollector*gc)
  {
    for (Rps_CallFrame* cf = &_; cf != nullptr; cf = cf->previous_call_frame())
      cf->gc_mark_frame(gc);
  };
  rps_garbage_collect(&markall);
} // end rps_repl_builtin_gc_command


void
rps_repl_builtin_typeinfo_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                  Rps_TokenSource& intoksrc,
                                  const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  rps_print_types_info();
} // end rps_repl_builtin_typeinfo_command



void
rps_repl_builtin_version_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                 Rps_TokenSource& intoksrc,
                                 const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                );
  _f.obenv = obenvarg;
  rps_show_version();
} // end rps_repl_builtin_version_command



void
rps_repl_builtin_env_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                             Rps_TokenSource& intoksrc,
                             const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                );
  _f.obenv = obenvarg;
  auto paylenv = _f.obenv->get_dynamic_payload< Rps_PayloadEnvironment>();
  RPS_INFORMOUT(std::endl << "environment object is " << _f.obenv
                << Rps_Do_Output([&](std::ostream&outs)
  {
    if (paylenv)
      {
        outs << " with parent environment:" << paylenv->get_parent_environment() << std::endl;
        _f.descrv = paylenv->get_descr();
        if (_f.descrv)
          outs << "descriptor:" << _f.descrv << std::endl;
        std::function<bool(Rps_CallFrame*,Rps_ObjectRef,Rps_Value,void*)> outfun
          = [&](Rps_CallFrame*cf,Rps_ObjectRef obvar,Rps_Value value,void*d)
        {
          outs << "*" << obvar << "::" << value << std::endl;
          RPS_ASSERT(d == nullptr);
          return false;
        };
        paylenv->do_each_entry(&_, outfun);
      }
    else
      outs << " [without environment payload]";
  })
      << std::endl);
  rps_show_version();
} // end rps_repl_builtin_env_command




void
rps_repl_builtin_parse_expression_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
    Rps_TokenSource& intoksrc,
    const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_expression_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_expression_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_expression(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_expression " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_expression " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_expression_command

void
rps_repl_builtin_parse_expr_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                    Rps_TokenSource& intoksrc,
                                    const char*title)
{
  rps_repl_builtin_parse_expression_command(callframe, obenvarg, builtincmd, intoksrc, title);
} // end rps_repl_builtin_parse_expr_command

void
rps_repl_builtin_parse_disjunction_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
    Rps_TokenSource& intoksrc,
    const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_disjunction_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_disjunction_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_disjunction(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_disjunction " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_disjunction " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_disjunction_command


void
rps_repl_builtin_parse_disj_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                    Rps_TokenSource& intoksrc,
                                    const char*title)
{
  rps_repl_builtin_parse_disjunction_command(callframe, obenvarg, builtincmd, intoksrc, title);
} // end rps_repl_builtin_parse_disj_command


void
rps_repl_builtin_parse_conjunction_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
    Rps_TokenSource& intoksrc,
    const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_conjunction_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_conjunction_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_conjunction(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_conj " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_conjunction " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_conjunction_command


void
rps_repl_builtin_parse_conj_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                    Rps_TokenSource& intoksrc,
                                    const char*title)
{
  rps_repl_builtin_parse_conjunction_command(callframe, obenvarg, builtincmd, intoksrc, title);
} // end rps_repl_builtin_parse_conj_command



void
rps_repl_builtin_parse_comparison_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
    Rps_TokenSource& intoksrc,
    const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_comparison_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_comparison_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_comparison(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_comparison " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_comparison " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_comparison_command

void
rps_repl_builtin_parse_comparand_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
    Rps_TokenSource& intoksrc,
    const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_comparand_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_comparand_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_comparand(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_comparand " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_comparand " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_comparand_command



void
rps_repl_builtin_parse_sum_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                   Rps_TokenSource& intoksrc,
                                   const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_sum_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_sum_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_sum(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_sum " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_sum " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_sum_command



void
rps_repl_builtin_parse_product_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                       Rps_TokenSource& intoksrc,
                                       const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_product_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_product_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_product(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_product " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_product " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_product_command


void
rps_repl_builtin_parse_factor_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                      Rps_TokenSource& intoksrc,
                                      const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_factor_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_factor_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_factor(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_factor " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_factor " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_factor_command


void
rps_repl_builtin_parse_term_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                    Rps_TokenSource& intoksrc,
                                    const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_term_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_term_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_term(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_term " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_term " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_term_command


void
rps_repl_builtin_parse_primary_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                                       Rps_TokenSource& intoksrc,
                                       const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  const char*cp = intoksrc.curcptr();
  _f.obenv = obenvarg;
  bool ok= false;
  RPS_DEBUG_LOG(REPL, "rps_repl_builtin_parse_primary_command " << title
                << "… intoksrc:" << intoksrc << " obenv=" << _f.obenv
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_builtin_parse_primary_command")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    intoksrc.display_current_line_with_cursor(out);
  })
      << std::endl);
  _f.parvalv = intoksrc.parse_primary(&_, &ok);
  if (ok)
    RPS_INFORMOUT(std::endl << " parse_primary " << cp << " as " << _f.parvalv);
  else
    RPS_WARNOUT("failed parse_primary " << cp << " in " << intoksrc);
} // end rps_repl_builtin_parse_primary_command

////////////////////////////////////////////////////////////////

void
rps_do_builtin_repl_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const char*builtincmd,
                            Rps_TokenSource& intoksrc,
                            const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value lextokv;
                 Rps_Value lexval;
                 Rps_Value descrv;
                 Rps_Value parvalv;
                );
  _f.obenv = obenvarg;
  RPS_DEBUG_LOG(REPL, "rps_do_builtin_repl_command " << title
                << "… intoksrc:" << intoksrc << " BUILTIN " << builtincmd
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
  if (!strcmp(builtincmd, "sh"))
    {
      rps_repl_builtin_sh_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "shell"))
    {
      rps_repl_builtin_shell_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "pwd"))
    {
      rps_repl_builtin_pwd_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "cd"))
    {
      rps_repl_builtin_cd_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "pid"))
    {
      rps_repl_builtin_pid_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "getpid"))
    {
      rps_repl_builtin_getpid_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "git"))
    {
      rps_repl_builtin_git_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "time"))
    {
      rps_repl_builtin_time_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "gc"))
    {
      rps_repl_builtin_gc_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "typeinfo"))
    {
      rps_repl_builtin_typeinfo_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "version"))
    {
      rps_repl_builtin_version_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "env"))
    {
      rps_repl_builtin_env_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_expression"))
    {
      rps_repl_builtin_parse_expression_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_expr"))
    {
      rps_repl_builtin_parse_expr_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_disjunction"))
    {
      rps_repl_builtin_parse_disjunction_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_disj"))
    {
      rps_repl_builtin_parse_disj_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_conj"))
    {
      rps_repl_builtin_parse_conj_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_conjunction"))
    {
      rps_repl_builtin_parse_conjunction_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_comparison"))
    {
      rps_repl_builtin_parse_comparison_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_comparand"))
    {
      rps_repl_builtin_parse_comparand_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_sum"))
    {
      rps_repl_builtin_parse_sum_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_product"))
    {
      rps_repl_builtin_parse_product_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_factor"))
    {
      rps_repl_builtin_parse_factor_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_term"))
    {
      rps_repl_builtin_parse_term_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "parse_primary"))
    {
      rps_repl_builtin_parse_primary_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "pmap"))
    {
      rps_repl_builtin_pmap_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else if (!strcmp(builtincmd, "pfd"))
    {
      rps_repl_builtin_pfd_command(&_, _f.obenv, builtincmd, intoksrc, title);
    }
  else
    RPS_WARNOUT("invalid builtin " << builtincmd << " in "
                << intoksrc << " / " << title)    ;
} // end rps_do_builtin_repl_command



void
rps_do_one_repl_command(Rps_CallFrame*callframe, Rps_ObjectRef obenvarg, const std::string&cmd,
                        const char*title)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef obenv;
                 Rps_Value lextokv;
                 Rps_Value lexval;
                 Rps_ObjectRef cmdob;
                 Rps_Value parsmainv;
                 Rps_Value parsextrav;
                 Rps_Value cmdparserv;
                 Rps_Value tok0, tok1, tok2;
                );
  _f.obenv = obenvarg;
  RPS_ASSERT(title != nullptr && title[0]);
  RPS_ASSERT(!_f.obenv || (Rps_Value(_f.obenv)).is_object());
#warning rps_do_one_repl_command unimplemented
  RPS_DEBUG_LOG(REPL,"rps_do_one_repl_command starting obenv="
                << RPS_OBJECT_DISPLAY(_f.obenv)
                << std::endl << title << " cmd='" << Rps_Cjson_String(cmd) << "'"
		<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "rps_do_one_repl_command")
		<< std::endl << " in thread "
		<< rps_current_pthread_name());
  RPS_POSSIBLE_BREAKPOINT();
  Rps_StringTokenSource intoksrc(cmd, std::string(title) + "°repl");
  if (!intoksrc.get_line())
    {
      RPS_WARNOUT("rps_do_one_repl_command " << title << " no line from "
		  << Rps_Cjson_String(cmd) << " in thread "
		  << rps_current_pthread_name());
      return;
    }
  RPS_DEBUG_LOG(REPL,"rps_do_one_repl_command intoksrc="<< intoksrc
		<< std::endl
		<< "… p." << intoksrc.position_str()
		<< " obenv=" << _f.obenv);
  RPS_POSSIBLE_BREAKPOINT();
  _f.tok0 = intoksrc.lookahead_token(&_, 0);
  RPS_DEBUG_LOG(REPL,"rps_do_one_repl_command intoksrc="<< intoksrc
		<< std::endl
		<< "… tok0=" << _f.tok0);
  RPS_POSSIBLE_BREAKPOINT();
  _f.tok1 = intoksrc.lookahead_token(&_, 1);
  RPS_DEBUG_LOG(REPL,"rps_do_one_repl_command intoksrc="<< intoksrc
		<< std::endl
		<< "… tok1=" << _f.tok1);
  _f.tok2 = intoksrc.lookahead_token(&_, 2);
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL,"rps_do_one_repl_command intoksrc="<< intoksrc
		<< std::endl
		<< "… tok0=" << _f.tok0
		<< " tok1=" << _f.tok1
		<< " tok2=" << _f.tok2
		<< std::endl << RPS_FULL_BACKTRACE_HERE(1, "rps_do_one_repl_command"));
  std::string commandpos = intoksrc.position_str();
  RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command "
//<< title << "'"
//<< Rps_Cjson_String(cmd) << "'" << std::endl << "… "
                << "intoksrc:" << intoksrc
		<< std::endl
                << "… curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << " tok0=" << _f.tok0 << " tok1=" << _f.tok1
                << " tok2=" << _f.tok2);
  /*** TODO:
       For debugging purposes, we want builtin commands like !parse_term etc...
   ***/
  if (cmd[0] == '!' && isalpha(cmd[1]))
    {
      char builtincmd[64];
      memset (builtincmd, 0, sizeof(builtincmd));
      for (int i=0; i<(int)sizeof(builtincmd)-2; i++)
        {
          if (!isalnum(cmd[i+1]) && cmd[i+1] != '_')
            break;
          builtincmd[i] = cmd[i+1];
        };
      if (strlen(builtincmd) >= sizeof(builtincmd)-4)
        RPS_FATALOUT("rps_do_do_one_repl_command " << title
                     << Rps_Cjson_String(cmd)
                     << " too long builtin command " << builtincmd);
      intoksrc.advance_cursor_bytes(strlen(builtincmd)+1);
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command " << title
                    << Rps_Cjson_String(cmd) << std::endl
                    << "… intoksrc:" << intoksrc << " BUILTIN " << builtincmd
                    << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
      rps_do_builtin_repl_command(&_, _f.obenv, builtincmd, intoksrc, title);
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command " << title
                    << Rps_Cjson_String(cmd)
                    << "… intoksrc:" << intoksrc << " DONEBUILTIN " << builtincmd
                    << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
      return;
    }
  else if (cmd[0] == '@')
    {
      /*** A command starting with @ is parsed using carburetta
	   generated code from file carbrepl_rps.cbrt  */
      //intoksrc.advance_cursor_bytes(1);
      RPS_INFORMOUT("rps_do_one_repl_command carburetta '" << Rps_Cjson_String(cmd)
		    << "'"
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_do_one_repl_command carburetta")
                    << Rps_Do_Output([&](std::ostream& out)
      {
        intoksrc.display_current_line_with_cursor(out);
      })
          << std::endl);
      RPS_POSSIBLE_BREAKPOINT();
      /* TODO: actually this API for rps_do_carburetta_command is
         suboptimal, since the token source is built twice.  Perhaps
         rps_do_carburetta_command should be redesigned to get the
         command from a Rps_TokenSource.... */
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command " << title << " before calling rps_do_carburetta_command intoksrc=" << intoksrc);
      RPS_POSSIBLE_BREAKPOINT();
      rps_do_carburetta_command(&_,  /*obenv:*/_f.obenv, &intoksrc);
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command " << title << " after calling rps_do_carburetta_command");
      RPS_POSSIBLE_BREAKPOINT();
    } // end of carburetta handled commands
  _f.lextokv = intoksrc.get_token(&_);
  _f.lexval = nullptr;
  _f.cmdob = nullptr;
  RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command got lextokv=" << _f.lextokv
                << " pos=" << intoksrc.position_str() << std::endl
                << "… intoksrc:" << intoksrc
                << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                << " token_deq:" << intoksrc.token_dequeue());
  if (!_f.lextokv)
    {
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command no lexical token"
                    << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                    << " in " << intoksrc);
      return;
    }
  const Rps_LexTokenZone* lextokz = _f.lextokv.as_lextoken();
  RPS_ASSERT(lextokz);
  RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command lextokv="
                << _f.lextokv << " of kind:" << lextokz->lxkind());
  if (lextokz->lxkind()
      == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ))  //object∈class
    {
      _f.lexval = lextokz->lxval();
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command lextokv=" << _f.lextokv << " lexval=" << _f.lexval);
      RPS_ASSERT(_f.lexval.is_object());
      _f.cmdob = _f.lexval.as_object();
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command cmdob=" << _f.cmdob
                    << " at " << commandpos<< std::endl
                    << "… intoksrc:" << intoksrc
                    << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
    }
  else if (lextokz->lxkind()
           ==RPS_ROOT_OB(_36I1BY2NetN03WjrOv) //symbol∈class
          )
    {
      _f.lexval = lextokz->lxval();
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command symbol token "
                    << _f.lextokv << " of value " << _f.lexval
                    << " at " << commandpos << std::endl
                    << "… intoksrc:" << intoksrc
                    << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                    << " token_deq:" << intoksrc.token_dequeue());
      if (_f.lexval.is_string())
        _f.cmdob = Rps_PayloadSymbol::find_named_object(_f.lexval.as_string()->cppstring());
#warning unimplemented symbol token rps_do_one_repl_command
      RPS_WARNOUT("unimplemented symbol token rps_do_one_repl_command lextok=" << _f.lextokv
                  << " lexval:" << _f.lexval << " cmdob=" << _f.cmdob<< std::endl
                  << "… intoksrc:" << intoksrc
                  << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_do_one_repl_command/symbol"));
    }
  else
    {
      RPS_WARNOUT("rps_do_one_repl_command command at "
                  << commandpos << std::endl
                  << "Should start with an object or symbol but got "
                  << _f.lextokv << " of kind " << lextokz->lxkind()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_do_one_repl_command/non-obj-cmd"));
      return;
    }
  RPS_ASSERT(_f.lexval.is_object());
  _f.cmdob = _f.lexval.as_object();
  RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command cmdob=" << _f.cmdob
                << " at " << commandpos);
  if (_f.lexval.is_instance_of(&_,RPS_ROOT_OB(_8CncrUdoSL303T5lOK)))   //repl_command∈class
    {
      _f.cmdparserv = //
        _f.cmdob->get_attr1(&_,RPS_ROOT_OB(_4I8GwXXfO3P01cdzyd)); //repl_command_parser∈symbol
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command cmdob=" << _f.cmdob
                    << " is repl_command" << std::endl
                    << "of repl_command_parser: " << _f.cmdparserv
                    << " lextokv=" << _f.lextokv);
      if (_f.cmdparserv.is_closure())
        {
          RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command cmdob=" << _f.cmdob
                        << " before applying cmdparserv="
                        << _f.cmdparserv  << std::endl
                        << " with lextokv=" << _f.lextokv
                        << " command at " << commandpos
                        << " ... at position " <<  intoksrc.position_str()
                        << " curptr:" << Rps_QuotedC_String(intoksrc.curcptr()) << std::endl
                        <<  RPS_FULL_BACKTRACE_HERE(1, "rps_do_one_repl_command/before-apply"));
          Rps_TwoValues parspair = Rps_ClosureValue(_f.cmdparserv.to_closure()).apply2 (&_, _f.cmdob, _f.lextokv);
          _f.parsmainv = parspair.main();
          _f.parsextrav = parspair.xtra();
          RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command applied "
                        << _f.cmdparserv << " to cmd "
                        << title << " "<< std::endl
                        << "… intoksrc:" << intoksrc
                        << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                        << _f.cmdob
                        << " and got parsmainv:" << _f.parsmainv << ", parsextrav=" << _f.parsextrav
                        << std::endl
                        << "… now position is " <<  intoksrc.position_str() << " curptr:" <<
                        Rps_QuotedC_String(intoksrc.curcptr())  << std::endl
                        <<  RPS_FULL_BACKTRACE_HERE(1, "rps_do_repl_commands_vec/applied"));
          if (!_f.parsmainv && !_f.parsextrav)
            {
              RPS_WARNOUT("rps_do_one_repl_command: REPL command " << title
                          << " " << _f.cmdob << " at " << commandpos << " failed using "
                          << _f.cmdparserv << std::endl
                          << " ... at position " <<  intoksrc.position_str()
                          << " curptr:" << Rps_QuotedC_String(intoksrc.curcptr()) << std::endl
                          <<  RPS_FULL_BACKTRACE_HERE(1, "rps_do_one_repl_command/apply failed"));
              return;
            }
        }
      else
        {
          RPS_WARNOUT("rps_do_one_repl_command: REPL command " << _f.cmdob << " has a bad command parser " << _f.cmdparserv
                      << " after " << _f.lexval);
          return;
        }
      RPS_DEBUG_LOG(REPL, "rps_do_one_repl_command at  " << commandpos
                    << " " << title <<  " pos=" << intoksrc.position_str());
    }
  else
    {
      RPS_WARNOUT("rps_do_one_repl_command: REPL command unexpected token "
                  <<  _f.lextokv << " at " << commandpos << " " << title
                  << " now at " << intoksrc.position_str());
      return;
    }
} // end rps_do_one_repl_command



void
rps_do_repl_commands_vec(const std::vector<std::string>&cmdvec)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/RPS_NULL_CALL_FRAME,
                 Rps_ObjectRef envob;
                );
  RPS_ASSERT(rps_is_main_thread());
  int nbcmd = (int) (cmdvec.size());
  _f.envob = Rps_ObjectRef::make_object(&_,
                                        RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
                                       );
  auto paylenv = _f.envob->put_new_plain_payload<Rps_PayloadEnvironment>();
  RPS_ASSERT(paylenv);
  RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec start nbcmd:" << nbcmd
                << RPS_OBJECT_DISPLAY(_f.envob));
  for (int cix=0; cix<nbcmd; cix++)
    {
      RPS_POSSIBLE_BREAKPOINT();
      RPS_DEBUG_LOG(REPL, "REPL command [" << cix << "]: "
                    << cmdvec[cix]);
      if (cix % 4 == 0)
        usleep(128*1024);
      RPS_POSSIBLE_BREAKPOINT();
      char bufpath[72];
      int n = -1;
      memset (bufpath, 0, sizeof(bufpath));
      n = snprintf(bufpath, sizeof(bufpath), "ReplCmd[%d]'%s'", cix,
                   Rps_Cjson_String(cmdvec[cix]).c_str());
      RPS_ASSERT(n>0);
      if (n>(int)sizeof(bufpath)-5)
        {
          n = snprintf(bufpath, sizeof(bufpath), "ReplCmd{%d}", cix);
          RPS_DEBUG_LOG(REPL, "REPL " << bufpath
                        << " is " << cmdvec[cix]);
        }
      /// do the command
      try
        {
	  RPS_POSSIBLE_BREAKPOINT();
          RPS_DEBUG_LOG(REPL, "REPL before doing command " << Rps_Cjson_String(cmdvec[cix])
			<< " in env=" << _f.envob
			<< " " << bufpath);
	  RPS_POSSIBLE_BREAKPOINT();
          rps_do_one_repl_command(&_, _f.envob, cmdvec[cix],
                                  /*title:*/bufpath);
          RPS_DEBUG_LOG(REPL, "REPL command " << Rps_Cjson_String(cmdvec[cix])
                        << " done");
	  RPS_POSSIBLE_BREAKPOINT();
        }
      catch (std::exception&ex)
        {
          const std::type_info& extypinf= typeid(ex);
          std::string extyprawname = extypinf.name();
          int status = 0;
          char*extyprealname = abi::__cxa_demangle(extyprawname.c_str(),
                               nullptr, nullptr, &status);
          RPS_WARNOUT("REPL command#" << cix << " " << cmdvec[cix]
                      << " in env:" << _f.envob
                      << std::endl
                      << " failed with exception: " << ex.what() << (!status?" ":"")
                      << ((!status && extyprealname)?extyprealname:extyprawname.c_str()));
          free (extyprealname);
        }
    } // end for cix ...
} // end of rps_do_repl_commands_vec




// end of file repl_rps.cc
