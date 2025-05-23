/****************************************************************
 * file strbufdict_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the string buffer code, and code for dictionaries
 *      associating strings to values.
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



extern "C" const char rps_strbufdict_gitid[];
const char rps_strbufdict_gitid[]= RPS_GITID;

extern "C" const char rps_strbufdict_date[];
const char rps_strbufdict_date[]= __DATE__;

extern "C" const char rps_strbufdict_shortgitid[];
const char rps_strbufdict_shortgitid[]= RPS_SHORTGITID;


Rps_PayloadStrBuf::Rps_PayloadStrBuf(Rps_ObjectZone*obz)
  : Rps_Payload(Rps_Type::PaylStrBuf, obz),
    strbuf_buffer(),
    strbuf_indent(0),
    strbuf_transient(false)
{
} // end PayloadStrBuf::Rps_PayloadStrBuf


Rps_PayloadStrBuf::~Rps_PayloadStrBuf()
{
} // end Rps_PayloadStrBuf::~Rps_PayloadStrBuf

void
Rps_PayloadStrBuf::gc_mark([[maybe_unused]] Rps_GarbageCollector& gc) const
{
} // end Rps_PayloadStrBuf::gc_mark


void
Rps_PayloadStrBuf::append_string(const std::string&str)
{
  if (str.empty())
    return;
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  strbuf_buffer.sputn(str.c_str(), str.size());
} // end Rps_PayloadStrBuf::append_string

void
Rps_PayloadStrBuf::prepend_string(const std::string&str)
{
  if (str.empty())
    return;
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
#warning Rps_PayloadStrBuf::prepend_string implementation is inefficient
  if (strbuf_buffer.str().empty())
    {
      strbuf_buffer.sputn(str.c_str(), str.size());
      return;
    }
  else
    {
      /// inefficient; TODO: improve
      std::string oldcont = strbuf_buffer.str();
      std::string newcont = str + oldcont;
      strbuf_buffer.sputn(newcont.c_str(), newcont.size());
      return;
    }
} // end Rps_PayloadStrBuf::prepend_string

void
Rps_PayloadStrBuf::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  return;
} // end Rps_PayloadStrBuf::dump_scan


Rps_ObjectRef
Rps_PayloadStrBuf::make_string_buffer_object(Rps_CallFrame*callframe, Rps_ObjectRef obclassarg, Rps_ObjectRef obspacearg)
{
  RPS_LOCALFRAME(/*descr:*/ RPS_ROOT_OB(_1rfASGBBbFz02VUsMw), //"rps_serve_onion_expanded_stream"∈rps_routine
                            /*prev:*/callframe,
                            Rps_ObjectRef obsbuf;
                            Rps_ObjectRef obclass;
                            Rps_ObjectRef obspace;
                );
  _f.obclass = obclassarg;
  _f.obspace = obspacearg;
  if (!_f.obclass)
    _f.obclass = Rps_PayloadStrBuf::the_string_buffer_class();
  if (_f.obclass != Rps_PayloadStrBuf::the_string_buffer_class()
      && !_f.obclass->is_subclass_of(Rps_PayloadStrBuf::the_string_buffer_class()))
    {
      std::ostringstream outmsg;
      outmsg << "make_string_buffer_object:" << _f.obclass->oid() << " not a subclass of string_buffer" << std::endl;
      throw std::runtime_error(outmsg.str());
    }
  _f.obsbuf = Rps_ObjectRef::make_object(&_, _f.obclass, _f.obspace);
  [[maybe_unused]] auto paylsbuf = _f.obsbuf->put_new_plain_payload<Rps_PayloadStrBuf>();
  return _f.obsbuf;
} // end of Rps_PayloadStrBuf::make_string_buffer_object


void
Rps_PayloadStrBuf::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_string_buffer
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  if (strbuf_transient)
    return;
  const std::string&str = strbuf_buffer.str();
  auto eol = str.find('\n');
  if (eol > 0)
    {
      auto begit = str.begin();
      Json::Value jarr(Json::arrayValue);
      while (eol > 0)
        {
          auto eolit = begit+eol;
          std::string linstr = str.substr(begit-str.begin(),eolit-str.begin());
          jarr.append(Json::Value(linstr));
          begit = eolit;
          eol = str.find(begit+1-str.begin(), '\n');
        }
      jv["strbuf_lines"] = jarr;
    }
  else
    {
      jv["strbuf_string"] = Json::Value(str);
    };
  jv["strbuf_indent"] = Json::Value(strbuf_indent);
  return;
} // end Rps_PayloadStrBuf::dump_json_content

//// loading of Rps_PayloadStrBuf; see above Rps_PayloadStrBuf::dump_json_content
void
rpsldpy_string_buffer(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_DEBUG_LOG(LOAD,"start rpsldpy_string_buffer obz=" << obz
                << " jv=" << jv
                << " spacid=" << spacid
                << " lineno=" << lineno);
  auto paylsbuf = obz->put_new_plain_payload<Rps_PayloadStrBuf>();
  RPS_ASSERT(paylsbuf);
  if (jv.isMember("strbuf_lines"))
    {
      auto jarr = jv["strbuf_lines"];
      if (jarr.isArray())
        {
          int arrlen = jarr.size();
          for (int ix=0; ix<arrlen; ix++)
            {
              auto& jcomp = jarr[ix];
              if (jcomp.isString())
                paylsbuf->append_string(jcomp.asString());
              paylsbuf->append_string("\n");
            }
        }
    }
  else if (jv.isMember("strbuf_string"))
    {
      std::string str = jv["strbuf_string"].asString();
      paylsbuf->append_string(str);
    }
  else
    RPS_WARNOUT("rpsldpy_string_buffer: incorrect jv=" << jv
                << "in spacid=" << spacid
                << " lineno=" << lineno);
  paylsbuf->strbuf_indent = jv["strbuf_indent"].asInt();
} // end of rpsldpy_string_buffer


void
Rps_PayloadStrBuf::clear_buffer()
{
/// clear the buffer
  strbuf_buffer = std::stringbuf("");
} // end Rps_PayloadStrBuf::clear_buffer

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


Rps_PayloadStringDict::Rps_PayloadStringDict(Rps_ObjectZone*obz)
  : Rps_Payload(Rps_Type::PaylStringDict, obz),
    dict_map(),
    dict_is_transient(false)
{
} // end PayloadStringDict::Rps_PayloadStringDict


Rps_PayloadStringDict::~Rps_PayloadStringDict()
{
  dict_map.clear();
  dict_is_transient = false;
} // end Rps_PayloadStringDict::~Rps_PayloadStringDict

void
Rps_PayloadStringDict::gc_mark(Rps_GarbageCollector&gc) const
{
  for (auto it : dict_map)
    {
      it.second.gc_mark(gc);
    }
} // end Rps_PayloadStringDict::gc_mark

void
Rps_PayloadStringDict::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  if (dict_is_transient)
    return;
  for (auto it : dict_map)
    {
      rps_dump_scan_value(du,it.second,0);
    }
} // end Rps_PayloadStringDict::dump_scan

void
Rps_PayloadStringDict::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_string_dictionary below
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  if (dict_is_transient)
    return;
  Json::Value jarr(Json::arrayValue);
  for (auto it : dict_map)
    {
      if (!rps_is_dumpable_value(du,it.second))
        continue;
      Json::Value jcur = rps_dump_json_value(du, it.second);
      Json::Value jent(Json::objectValue);
      jent["str"] = it.first;
      jent["val"] = jcur;
      jarr.append(jent);
    }
  jv["payload"] = "string_dictionary";
  jv["dictionary"] = jarr;
} // end Rps_PayloadStringDict::dump_json_content


//// loading of Rps_PayloadStringDict; see above Rps_PayloadStringDict::dump_json_content
void
rpsldpy_string_dictionary(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_DEBUG_LOG(LOAD, "rpsldpy_string_dictionary object " << obz->oid()
                << " in space " << spacid << " lineno#" << lineno << " jv="
                << jv);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  if (!jv.isMember("dictionary"))
    {
      RPS_FATALOUT("rpsldpy_string_dictionary: object " << obz->oid()
                   << " in space " << spacid << " lineno#" << lineno
                   << " has incomplete payload"
                   << std::endl
                   << " jv " << (jv));
    }
  auto payldict = obz->put_new_plain_payload<Rps_PayloadStringDict>();
  Json::Value jarr = jv["dictionary"];
  if (!jarr.isArray())
    RPS_FATALOUT("rpsldpy_string_dictionary: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad dictionary "
                 << std::endl
                 << jarr);
  unsigned nbent = jarr.size();
  for (int entix=0; entix<(int)nbent; entix++)
    {
      Json::Value jcurent = jarr[entix];
      if (!jcurent.isObject()
          || !jcurent.isMember("str")
          || !jcurent.isMember("val"))
        continue;
      std::string curstr = jcurent["str"].asString();
      if (curstr.empty())
        continue;
      payldict->add(curstr, Rps_Value(jcurent["val"],ld));
    }
} // end rpsldpy_string_dictionary

void
Rps_PayloadStringDict::add(const std::string&str, Rps_Value val)
{
  if (!str.empty() && !val.is_empty())
    dict_map.insert({str,val});
  else if (!str.empty() && !val)
    dict_map.erase(str);
} // end Rps_PayloadStringDict::add

Rps_Value
Rps_PayloadStringDict::find(const std::string&str) const
{
  if (!str.empty())
    {
      auto it = dict_map.find(str);
      if (it != dict_map.end())
        return it->second;
    }
  return nullptr;
} // end Rps_PayloadStringDict::find

void
Rps_PayloadStringDict::remove(const std::string&str)
{
  dict_map.erase(str);
} // end Rps_PayloadStringDict::remove

void
Rps_PayloadStringDict::set_transient(bool transient)
{
  dict_is_transient = transient;
} // end PayloadStringDict::set_transient

void
Rps_PayloadStringDict::iterate_with_callframe(Rps_CallFrame*callerframe, const std::function <bool(Rps_CallFrame*,const std::string&,const Rps_Value)>& stopfun)
{
  RPS_ASSERT(callerframe == nullptr || callerframe->is_good_call_frame());
  RPS_ASSERT(stopfun);
  for (auto it : dict_map)
    {
      if (stopfun(callerframe, it.first, it.second))
        return;
    }
} // end Rps_PayloadStringDict::iterate_with_callframe

void
Rps_PayloadStringDict::iterate_with_data(void*data, const std::function <bool(void*,const std::string&,const Rps_Value)>& stopfun)
{
  RPS_ASSERT(stopfun);
  for (auto it : dict_map)
    {
      if (stopfun(data, it.first, it.second))
        return;
    }
} // end Rps_PayloadStringDict::iterate_with_callframe

void
Rps_PayloadStringDict::iterate_apply(Rps_CallFrame*callerframe, Rps_Value closarg)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*prev:*/callerframe,
                 Rps_ObjectRef obown;
                 Rps_Value closv;
                 Rps_Value curstrv;
                 Rps_Value curval;
                );

  RPS_ASSERT(callerframe == nullptr || callerframe->is_good_call_frame());
  if (!closarg.is_closure())
    return;
  _f.obown = owner();
  if (!_f.obown)
    return;
  _f.closv = closarg;
  for (auto it : dict_map)
    {
      _f.curstrv = Rps_StringValue(it.first);
      _f.curval = it.second;
      Rps_TwoValues pair = Rps_ClosureValue(_f.closv).apply3(&_, _f.obown, _f.curstrv, _f.curval);
      if (!pair)
        return;
      _f.curstrv = nullptr;
      _f.curval = nullptr;
    }
} // end Rps_PayloadStringDict::iterate_apply

Rps_ObjectRef
Rps_PayloadStringDict::the_string_dictionary_class(void)
{
  return RPS_ROOT_OB(_3FztYBKABxZ02DUPRm);
} // end Rps_PayloadStringDict::the_string_dictionary_class

Rps_ObjectRef
Rps_PayloadStringDict::make_string_dictionary_object(Rps_CallFrame*callframe, Rps_ObjectRef obclassarg, Rps_ObjectRef obspacearg)
{
  RPS_ASSERT(!callframe || callframe->is_good_call_frame());
  RPS_LOCALFRAME(the_string_dictionary_class(),
                 callframe,
                 Rps_ObjectRef obstrdict;
                 Rps_ObjectRef obclass;
                 Rps_ObjectRef obspace;
                );
  _f.obclass = obclassarg;
  _f.obspace = obspacearg;
  if (!_f.obclass)
    _f.obclass = the_string_dictionary_class();
  if (_f.obclass != the_string_dictionary_class()
      && !Rps_Value(_f.obclass).is_subclass_of(&_,
          the_string_dictionary_class()))
    throw std::runtime_error("invalid class for make_string_dictionary_object");
  _f.obstrdict = Rps_ObjectRef::make_object(&_, _f.obclass, _f.obspace);
  auto payldict = _f.obstrdict->put_new_plain_payload<Rps_PayloadStringDict>();
  RPS_ASSERT(payldict);
  return _f.obstrdict;
} // end of Rps_PayloadStringDict::make_string_dictionary_object



void
Rps_PayloadStringDict::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  RPS_ASSERT(depth <= maxdepth);
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> guown(*(owner()->objmtxptr()));
  out << std::endl << BOLD_esc << "**" << (dict_is_transient?" transient":"")
      << " string dictionary payload of " << dict_map.size() << " entries **" << NORM_esc;
  for (auto it: dict_map)
    {
      const std::string &nam = it.first;
      Rps_Value v = it.second;
      RPS_ASSERT(!nam.empty());
      RPS_ASSERT(v);
      out << "*:" << Rps_QuotedC_String(nam) << ":";
      v.output(out, depth+1, maxdepth);
      out << std::endl;
    }
} // end Rps_PayloadStringDict::output_payload

//// end of file strbufdict_rps.cc
