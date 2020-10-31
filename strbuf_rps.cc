/****************************************************************
 * file strbuf_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the string buffer code.
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


//// end of file strbuf_rps.cc
