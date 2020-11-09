/****************************************************************
 * file strbufdict_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the string buffer code, and code for dictionnaries
 *      associating strings to values.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2020 The Reflective Persistent System Team
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


Rps_PayloadStrBuf::Rps_PayloadStrBuf(Rps_ObjectZone*obz)
  : Rps_Payload(Rps_Type::PaylStrBuf, obz),
    strbuf_out(),
    strbuf_indent(0),
    strbuf_transient(false)
{
} // end PayloadStrBuf::Rps_PayloadStrBuf


Rps_PayloadStrBuf::~Rps_PayloadStrBuf()
{
} // end Rps_PayloadStrBuf::~Rps_PayloadStrBuf

void
Rps_PayloadStrBuf::gc_mark(Rps_GarbageCollector&gc) const
{
} // end Rps_PayloadStrBuf::gc_mark

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
  auto paylsbuf = _f.obsbuf->put_new_plain_payload<Rps_PayloadStrBuf>();
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
  const std::string&str = strbuf_out.str();
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
                paylsbuf->strbuf_out << jcomp.asString();
            }
        }
    }
  else if (jv.isMember("strbuf_string"))
    {
      std::string str = jv["strbuf_string"].asString();
      paylsbuf->strbuf_out << str;
    }
  else
    RPS_WARNOUT("rpsldpy_string_buffer: incorrect jv=" << jv
                << "in spacid=" << spacid
                << " lineno=" << lineno);
  paylsbuf->strbuf_indent = jv["strbuf_indent"].asInt();
} // end of rpsldpy_string_buffer

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
  /// see function rpsldpy_string_dictionnary below
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
  jv["payload"] = "string_dictionnary";
  jv["dictionnary"] = jarr;
} // end Rps_PayloadStringDict::dump_json_content


//// loading of Rps_PayloadStringDict; see above Rps_PayloadStringDict::dump_json_content
void
rpsldpy_string_dictionnary(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  if (!jv.isMember("dictionnary"))
    {
      RPS_FATALOUT("rpsldpy_string_dictionnary: object " << obz->oid()
                   << " in space " << spacid << " lineno#" << lineno
                   << " has incomplete payload"
                   << std::endl
                   << " jv " << (jv));
    }
  auto payldict = obz->put_new_plain_payload<Rps_PayloadStringDict>();
  Json::Value jarr = jv["dictionnary"];
  if (!jarr.isArray())
    RPS_FATALOUT("rpsldpy_string_dictionnary: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad dictionnary "
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
} // end rpsldpy_string_dictionnary

void
Rps_PayloadStringDict::add(const std::string&str, Rps_Value val)
{
  if (!str.empty() && !val.is_empty())
    dict_map.insert({str,val});
  else if (!str.empty() && !val)
    dict_map.erase(str);
} // end Rps_PayloadStringDict::add

void
Rps_PayloadStringDict::remove(const std::string&str)
{
  dict_map.erase(str);
} // end Rps_PayloadStringDict::remove

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
  RPS_LOCALFRAME(/*descr:*/nullptr,
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


//// end of file strbufdict_rps.cc
