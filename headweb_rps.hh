/****************************************************************
 * file headweb_rps.hh
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is the public C++ header file of the web interface
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2020 The Reflective Persistent System Team
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



#ifndef HEADWEB_RPS_INCLUDED
#define HEADWEB_RPS_INCLUDED

#include "refpersys.hh"
// for libonion Web service library, from https://www.coralbits.com/libonion/
#include "onion/onion.hpp"
#include "onion/http.hpp"
#include "onion/url.hpp"
#include "onion/request.hpp"
#include "onion/response.hpp"
#include "onion/shortcuts.h"
#include "onion/mime.h"
#include "onion/version.h"

extern "C" Onion::Onion rps_onion_server;

extern "C" onion_connection_status
rps_serve_onion_web(Rps_Value val, Onion::Url*purl, Onion::Request*preq, Onion::Response*pres);

extern "C" onion_connection_status
rps_serve_onion_file(Rps_CallFrame*callframe, Rps_Value val, Onion::Url*purl, Onion::Request*preq, Onion::Response*pres, uint64_t reqnum, const std::string& filepath);


////////////////////////////////////////////////////////////////
/// a web exchange object is created for most dynamic HTTP requests
/// it obviously is a transient object which is not persisted to disk.
class Rps_PayloadWebex : // the payload for a web exchange; see
// RefPerSys class web_exchange
  public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend class Rps_Agenda;
  friend void rps_run_web_service(void);
  friend onion_connection_status
  rps_serve_onion_web(Rps_Value val, Onion::Url*purl, Onion::Request*prequ, Onion::Response*presp);
  friend Rps_PayloadWebex*
  Rps_QuasiZone::rps_allocate3<Rps_PayloadWebex,Rps_ObjectZone*,Onion::Request&,Onion::Response&>(Rps_ObjectZone*,Onion::Request&,Onion::Response&);
protected:
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1) / sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "webex";
  };
  Rps_PayloadWebex(Rps_ObjectZone*,uint64_t,Onion::Request&,Onion::Response&);
  virtual ~Rps_PayloadWebex();
private:
  uint64_t webex_reqnum;	// unique request number
  double webex_startim;		// start monotonic time
  Onion::Request* webex_requ;	// pointer to request
  Onion::Response* webex_resp;	// pointer to response
  Rps_Value webex_state; 	// some mutable state
  uint64_t webex_numstate;	// some numerical state
};				// end class Rps_PayloadWebex



////////////////////////////////////////////////////////////////

extern "C" rpsldpysig_t rpsldpy_web_handler;
/// a web handler object describes the handling of web requests
class Rps_PayloadWebHandler : // the payload for a web handler; see
// RefPerSys class web_handler
  public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend class Rps_Agenda;
  friend rpsldpysig_t rpsldpy_web_handler;
  friend Rps_PayloadWebHandler*
  Rps_QuasiZone::rps_allocate2<Rps_PayloadWebHandler,Rps_ObjectZone*,const std::string&>(Rps_ObjectZone*,const std::string&);
protected:
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1) / sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  static constexpr int webh_max_path_elem_size_ = 256;
  virtual const std::string payload_type_name(void) const
  {
    return "web_handler";
  };
  Rps_PayloadWebHandler(Rps_ObjectZone*obz,const std::string&pathelem);
  virtual ~Rps_PayloadWebHandler();
private:
  std::string webh_pathelem;
  Rps_ClosureValue webh_gethandler;
  Rps_ClosureValue webh_posthandler;
};				// end class Rps_PayloadWebHandler


#endif /* HEADWEB_RPS_INCLUDED */
/******* end of file headweb_rps.hh *******/
