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
 *      © Copyright 2020 - 2021 The Reflective Persistent System Team
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
rps_serve_onion_web(Rps_Value val, Onion::Url*purl, Onion::Request*preq, Onion::Response*pres, uint64_t reqnum);

extern "C" onion_connection_status
rps_serve_onion_file(Rps_CallFrame*callframe, Rps_Value val, Onion::Url*purl, Onion::Request*preq, Onion::Response*pres, uint64_t reqnum, const std::string& filepath);


/// given an object which is either a web_exchange or string_buffer, returns its ostream pointer. If check is true, raise an exception when none.
extern "C" std::ostream* rps_web_ostream_ptr(Rps_CallFrame*, Rps_ObjectRef ob, bool check=false);
constexpr bool RPS_CHECK_OSTREAM_PTR = true;
constexpr bool RPS_DONT_CHECK_OSTREAM_PTR = false;

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
  rps_serve_onion_web(Rps_Value val, Onion::Url*purl, Onion::Request*prequ, Onion::Response*presp, uint64_t reqcnt);
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
  Rps_PayloadWebex(Rps_ObjectZone*);
  Rps_PayloadWebex(Rps_ObjectZone*,uint64_t,Onion::Request*,Onion::Response*);
  virtual ~Rps_PayloadWebex();
  /// if ob is of class web_exchange, gives its payload. Otherwise
  /// return null:
  static Rps_PayloadWebex*webex_of_object(Rps_CallFrame*callerframe, Rps_ObjectRef ob);
private:
  std::atomic<uint64_t> webex_reqnum;	// unique request number
  double webex_startim;		// start monotonic time
  std::string webex_content_type;	// the Content-type
  Onion::Request* webex_requ;	// pointer to request
  Onion::Response* webex_resp;	// pointer to response
  Rps_Value webex_state; 	// some mutable state
  uint64_t webex_numstate;	// some numerical state
  int webex_indent;		// indentation in response, since we
  // might want to emit indented HTML, etc...
  //// the buffer containing the response data
  mutable std::ostringstream webex_outbuf;
public:
  void put_web_data(uint64_t reqnum, Onion::Request*requ, Onion::Response*resp);
  Onion::Request*  web_request() const
  {
    return webex_requ;
  };
  Onion::Response* web_response() const
  {
    return webex_resp;
  };
  void set_content_type(std::string);
  void set_http_response_code(int code);
  std::ostream* web_ostream_ptr() const
  {
    return &webex_outbuf;
  };
  bool web_output_contains(const std::string&substr) const
  {
    webex_outbuf.flush();
    return webex_outbuf.str().find(substr) != std::string::npos;
  }
  Rps_Value state_value() const
  {
    return webex_state;
  };
  void write_buffered_response(void);
  uint64_t numstate() const
  {
    return webex_numstate;
  };
  int web_indent() const
  {
    return webex_indent;
  };
  int web_increase_indent(int n=1)
  {
    webex_indent += n;
    return webex_indent;
  };
  int web_decrease_indent(int n=1)
  {
    webex_indent -= n;
    return webex_indent;
  };
  uint64_t web_request_num() const
  {
    return  webex_reqnum.load();
  };
  double web_request_start_time() const
  {
    return webex_startim;
  };
  unsigned web_request_methnum() const
  {
    if (!webex_requ) return 0;
    return (webex_requ->flags()) & OR_METHODS;
  };
  const char*web_request_methname() const
  {
    unsigned methnum = web_request_methnum();
    return onion_request_methods[methnum];
  };
  const std::string web_request_path() const
  {
    if (!webex_requ)
      return "";
    return webex_requ->path();
  };
  /* make an object of class web_exchange */
  static Rps_ObjectRef make_obwebex(Rps_CallFrame*callerframe, Onion::Request*req, Onion::Response*resp,
                                    uint64_t reqnum);
};				// end class Rps_PayloadWebex


extern "C" void
rps_web_display_html_for_value(Rps_CallFrame*callerframe,
                               const Rps_Value arg0val, //
                               Rps_ObjectRef arg1obweb, ///
                               int depth);


extern "C" void
rps_web_display_html_for_objref(Rps_CallFrame*callerframe,
                                Rps_ObjectRef arg0ob, //
                                Rps_ObjectRef arg1obweb, ///
                                int depth);

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
  static bool valid_path_element(const std::string&pathelem);
  Rps_PayloadWebHandler(Rps_ObjectZone*obz);
  virtual ~Rps_PayloadWebHandler();
  void put_path_element(const std::string&pathelem);
  void put_get_handler(Rps_Value val);
  void put_post_handler(Rps_Value val);
  void add_dict_handler(const std::string& path, Rps_Value val);
private:
  std::string webh_pathelem;
  Rps_ClosureValue webh_gethandler;
  Rps_ClosureValue webh_posthandler;
  std::map<std::string,Rps_Value> webh_dicthandler;
};				// end class Rps_PayloadWebHandler


#endif /* HEADWEB_RPS_INCLUDED */
/******* end of file headweb_rps.hh *******/
