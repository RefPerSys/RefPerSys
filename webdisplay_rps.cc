/****************************************************************
 * file webdisplay_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the web value and object displaying code.
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
#include "headweb_rps.hh"



extern "C" const char rps_webdisplay_gitid[];
const char rps_webdisplay_gitid[]= RPS_GITID;

extern "C" const char rps_webdisplay_date[];
const char rps_webdisplay_date[]= __DATE__;

////////////////////////////////////////////////////////////////
// C++ closure for _0TwK4TkhEGZ03oTa5m
//!display Val0 in Ob1Win at depth Val2Depth
extern "C" rps_applyingfun_t rpsapply_0TwK4TkhEGZ03oTa5m;
Rps_TwoValues
rpsapply_0TwK4TkhEGZ03oTa5m(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0val,
                            const Rps_Value arg1obweb, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0TwK4TkhEGZ03oTa5m,
                 callerframe, //
                 Rps_Value val0v;
                 Rps_ObjectRef webob1;
                 Rps_Value depth2v;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  _f.val0v = arg0val;
  _f.webob1 = arg1obweb.to_object();
  _f.depth2v = arg2depth;
  int depth = _f.depth2v.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_0TwK4TkhEGZ03oTa5m start val0v=" << _f.val0v
                << ", webob=" << _f.webob1
                << ", depth=" << depth);
  ////==== body of _0TwK4TkhEGZ03oTa5m ====
  Rps_PayloadWebex*pwebex = Rps_PayloadWebex::webex_of_object(&_, _f.webob1);
  RPS_ASSERT(pwebex != nullptr);
  /// we now can emit HTML code into pwebex, using its webex_resp...
#warning unimplemented rpsapply_0TwK4TkhEGZ03oTa5m
  RPS_FATALOUT("unimplemented rpsapply_0TwK4TkhEGZ03oTa5m val0v=" << _f.val0v
               << ", webob=" << _f.webob1
               << ", depth=" << depth
               << ", webex:: reqnum#" << pwebex->web_request_num()
               << " method:" << pwebex->web_request_methname()
               << " path:" << pwebex->web_request_path());
} // end of rpsapply_0TwK4TkhEGZ03oTa5m !display Val0 in Ob1Win at depth Val2Depth

void
rps_web_display_html_for_value(Rps_CallFrame*callerframe,
                               const Rps_Value arg0val, //
                               const Rps_Value arg1obweb, ///
                               int depth)
{
  RPS_LOCALFRAME(RPS_ROOT_OB(_0deGTf5hQwu01xJkyi), // the display_value_web symbol
                 callerframe, //
                 Rps_Value val0v; //
                 Rps_ObjectRef webob1; //
                 Rps_Value depth2v; //
                );
  _f.val0v = arg0val;
  _f.webob1 = arg1obweb.to_object();
  _f.depth2v = Rps_Value(depth, Rps_Value::Rps_IntTag{});
  Rps_PayloadWebex*pwebex = Rps_PayloadWebex::webex_of_object(&_, _f.webob1);
  RPS_ASSERT(pwebex != nullptr);
  if (!_f.val0v)
    {
      *(pwebex->web_response()) << "<span class='nullval_cl'>_</span>";
      return;
    }
  else if (_f.val0v.is_empty())
    {
      *(pwebex->web_response()) << "<span class='emptyval_cl'>?_?</span>";
      return;
    }
  // _0TwK4TkhEGZ03oTa5m is for the rpsapply_0TwK4TkhEGZ03oTa5m above
  Rps_ClosureValue(rpskob_0TwK4TkhEGZ03oTa5m, {}).apply3(&_,_f.val0v,_f.webob1, _f.depth2v);
} // end rps_web_display_html_for_value

////////////////////////////////////////////////////////////////
// C++ closure for _8KJHUldX8GJ03G5OWp
//!method int/display_value_web
extern "C" rps_applyingfun_t rpsapply_8KJHUldX8GJ03G5OWp;
Rps_TwoValues
rpsapply_8KJHUldX8GJ03G5OWp(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0recv, ///
                            const Rps_Value arg1obweb, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_8KJHUldX8GJ03G5OWp,
                 callerframe, //
                 Rps_Value intv;
                 Rps_ObjectRef obweb;
                 Rps_Value depthv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _8KJHUldX8GJ03G5OWp ====
  _f.intv = arg0recv;
  _f.obweb = arg1obweb.as_object();
  _f.depthv = arg2depth;
  RPS_DEBUG_LOG(WEB, "rpsapply_8KJHUldX8GJ03G5OWp start intv=" << _f.intv
                << ", obweb=" << _f.obweb
                << ", depth=" << _f.depthv);
#warning unimplemented rpsapply_8KJHUldX8GJ03G5OWp
  RPS_FATAL("unimplemented rpsapply_8KJHUldX8GJ03G5OWp");
} // end of  rpsapply_8KJHUldX8GJ03G5OWp !method int/display_value_web


////////////////////////////////////////////////////////////////

// C++ closure for _2KnFhlj8xW800kpgPt
//!method string/display_value_web
extern "C" rps_applyingfun_t rpsapply_2KnFhlj8xW800kpgPt;
Rps_TwoValues
rpsapply_2KnFhlj8xW800kpgPt(Rps_CallFrame*callerframe,
                            const Rps_Value arg0_receiver,
                            const Rps_Value arg1_object_window,
                            const Rps_Value arg2_recursive_depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_2KnFhlj8xW800kpgPt,
                 callerframe, //
                 Rps_Value string_value;
                 Rps_ObjectRef object_window;
                 Rps_Value recursive_depth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  _f.string_value = arg0_receiver;
  RPS_ASSERT(_f.string_value.is_string());
  _f.object_window = arg1_object_window.as_object();
  RPS_ASSERT(_f.object_window);
  _f.recursive_depth = arg2_recursive_depth;
  RPS_ASSERT(_f.recursive_depth.is_int());
  RPS_DEBUG_LOG(WEB, "rpsapply_2KnFhlj8xW800kpgPt start string_value=" << _f.string_value
                << ", object_window, =" << _f.object_window
                << ", recursive_depth=" <<  _f.recursive_depth);
  ////==== body of _2KnFhlj8xW800kpgPt ====
#warning unimplemented rpsapply_2KnFhlj8xW800kpgPt
  RPS_FATAL("unimplemented rpsapply_2KnFhlj8xW800kpgPt");
} // end of  rpsapply_2KnFhlj8xW800kpgPt !method string/display_value_web

////////////////////////////////////////////////////////////////
// C++ closure for _7oa7eIzzcxv03TmmZH
//!method double/display_value_web
extern "C" rps_applyingfun_t rpsapply_7oa7eIzzcxv03TmmZH;
Rps_TwoValues
rpsapply_7oa7eIzzcxv03TmmZH(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_7oa7eIzzcxv03TmmZH,
                 callerframe, //
                 Rps_Value doubleval;
                 Rps_ObjectRef object_window;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  ////==== body of _7oa7eIzzcxv03TmmZH !method double/display_value_web ====
  _f.doubleval = arg0_recv;
  RPS_ASSERT (_f.doubleval.is_double());
  _f.object_window = arg1_objwnd.as_object();
  RPS_ASSERT(_f.object_window);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT(_f.recdepth.is_int());
  RPS_DEBUG_LOG(WEB, "rpsapply_7oa7eIzzcxv03TmmZH start doubleval=" << _f.doubleval
                << "object_window, =" << _f.object_window
                << ", recdepth=" <<  _f.recdepth);
#warning unimplemented rpsapply_7oa7eIzzcxv03TmmZH
  RPS_FATAL("unimplemented rpsapply_7oa7eIzzcxv03TmmZH");
}
// end of rpsapply_7oa7eIzzcxv03TmmZH !method double/display_value_web




// C++ closure for _33DFyPOJxbF015ZYoi
//!method tuple/display_value_web
extern "C" rps_applyingfun_t rpsapply_33DFyPOJxbF015ZYoi;
Rps_TwoValues
rpsapply_33DFyPOJxbF015ZYoi(Rps_CallFrame*callerframe, //
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_33DFyPOJxbF015ZYoi,
                 callerframe, //
                 Rps_Value tupleval;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value arg3v;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );
  ////==== body of _33DFyPOJxbF015ZYoi !method tuple/display_value_web ====
  _f.tupleval = arg0_recv;
  RPS_ASSERT (_f.tupleval.is_tuple());
  _f.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_f.objwnd);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_33DFyPOJxbF015ZYoi start tupleval=" << _f.tupleval
                << "objwnd =" << _f.objwnd
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
#warning unimplemented rpsapply_33DFyPOJxbF015ZYoi
  RPS_FATAL("unimplemented rpsapply_33DFyPOJxbF015ZYoi");
} // end of rpsapply_33DFyPOJxbF015ZYoi !method tuple/display_value_web


// C++ closure for _1568ZHTl0Pa00461I2
//!method set/display_value_web
extern "C" rps_applyingfun_t rpsapply_1568ZHTl0Pa00461I2;
Rps_TwoValues
rpsapply_1568ZHTl0Pa00461I2(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_1568ZHTl0Pa00461I2,
                 callerframe, //
                 Rps_Value setval;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  ////==== body of _1568ZHTl0Pa00461I2 !method set/display_value_web ====
  _f.setval = arg0_recv;
  RPS_ASSERT (_f.setval.is_set());
  _f.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_f.objwnd);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_web start setval=" << _f.setval
                << "objwnd =" << _f.objwnd
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
#warning unimplemented rpsapply_1568ZHTl0Pa00461I2
  RPS_FATAL("unimplemented rpsapply_1568ZHTl0Pa00461I2");
} // end of rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_web


// C++ closure for _18DO93843oX02UWzq6
//!method object/display_value_web
extern "C" rps_applyingfun_t rpsapply_18DO93843oX02UWzq6;
Rps_TwoValues
rpsapply_18DO93843oX02UWzq6(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_objrecv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_18DO93843oX02UWzq6,
                 callerframe, //
                 Rps_ObjectRef obrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  _f.obrecv = arg0_objrecv.as_object();
  RPS_DEBUG_LOG(WEB, "rpsapply_18DO93843oX02UWzq6 start arg0_objrecv="
                << arg0_objrecv << ", arg1_objwnd=" << arg1_objwnd
                << ", arg2_recdepth=" << arg2_recdepth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_18DO93843oX02UWzq6") << std::endl);
#warning unimplemented rpsapply_18DO93843oX02UWzq6
  RPS_FATAL("unimplemented rpsapply_18DO93843oX02UWzq6");

} // end of rpsapply_18DO93843oX02UWzq6 !method object/display_value_web


// C++ closure for _0rgijx7CCnq041IZEd
//!method immutable_instance/display_value_web
extern "C" rps_applyingfun_t rpsapply_0rgijx7CCnq041IZEd;
Rps_TwoValues
rpsapply_0rgijx7CCnq041IZEd (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_inst, ///
                             const Rps_Value arg1_objwnd, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0rgijx7CCnq041IZEd,
                 callerframe, //
                 Rps_InstanceValue instrecv;
                 Rps_ObjectRef objwnd;
                 Rps_ObjectRef obconn;
                 Rps_ObjectRef obattr;
                 Rps_Value recdepth;
                 Rps_Value compv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_web====
  _f.instrecv = Rps_InstanceValue(arg0_inst.as_instance());
  RPS_ASSERT (_f.instrecv);
  _f.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_f.objwnd);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_0rgijx7CCnq041IZEd start instrecv=" << _f.instrecv
                << "objwnd =" << _f.objwnd
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
#warning unimplemented rpsapply_0rgijx7CCnq041IZEd
  RPS_FATAL("unimplemented rpsapply_0rgijx7CCnq041IZEd");
} // end of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_web




// C++ closure for _6Wi00FwXYID00gl9Ma
//!method closure/display_value_web
extern "C" rps_applyingfun_t rpsapply_6Wi00FwXYID00gl9Ma;
Rps_TwoValues
rpsapply_6Wi00FwXYID00gl9Ma (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_clos, ///
                             const Rps_Value arg1_objwnd, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_6Wi00FwXYID00gl9Ma,
                 callerframe, //
                 Rps_ClosureValue closrecv;
                 Rps_ObjectRef objwnd;
                 Rps_ObjectRef obconn;
                 Rps_ObjectRef obmeta;
                 Rps_Value recdepth;
                 Rps_Value compv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _6Wi00FwXYID00gl9Ma !method closure/display_value_web ====
  _f.closrecv = Rps_ClosureValue(arg0_clos.as_closure());
  RPS_ASSERT (_f.closrecv);
  _f.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_f.objwnd);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_6Wi00FwXYID00gl9Ma start closrecv=" << _f.closrecv
                << "objwnd =" << _f.objwnd
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
  _f.obconn = _f.closrecv->conn();
  unsigned width = _f.closrecv->cnt();
#warning unimplemented rpsapply_6Wi00FwXYID00gl9Ma
  RPS_FATALOUT("unimplemented rpsapply_6Wi00FwXYID00gl9Ma width=" << width);
} // end of rpsapply_6Wi00FwXYID00gl9Ma !method closure/display_value_web



// C++ closure for _42cCN1FRQSS03bzbTz
//!method json/display_value_web
extern "C" rps_applyingfun_t rpsapply_42cCN1FRQSS03bzbTz;
Rps_TwoValues
rpsapply_42cCN1FRQSS03bzbTz(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_json,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_42cCN1FRQSS03bzbTz,
                 callerframe, //
                 Rps_Value jsrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );
  ////==== body of _42cCN1FRQSS03bzbTz !method json/display_value_web ====
  ;
  _f.jsrecv = arg0_json;
  RPS_ASSERT (_f.jsrecv.is_json());
  _f.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_f.objwnd);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_42cCN1FRQSS03bzbTz start jsrecv=" << _f.jsrecv
                << "objwnd =" << _f.objwnd
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
#warning unimplemented rpsapply_42cCN1FRQSS03bzbTz
  RPS_FATAL("unimplemented rpsapply_42cCN1FRQSS03bzbTz");
} // end of rpsapply_42cCN1FRQSS03bzbTz !method json/display_value_web




////////////////////////////////////////////////////////////////
// C++ closure for _4x9jd2yAe8A02SqKAx
//!method object/display_object_occurrence_web
extern "C" rps_applyingfun_t rpsapply_4x9jd2yAe8A02SqKAx;
Rps_TwoValues
rpsapply_4x9jd2yAe8A02SqKAx (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obweb, ///
                             const Rps_Value arg2depth, //
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_ )
{
  RPS_LOCALFRAME(rpskob_4x9jd2yAe8A02SqKAx,
                 callerframe, //
                 Rps_ObjectRef recvob;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _4x9jd2yAe8A02SqKAx  !method object/display_object_occurrence_web ====
  RPS_DEBUG_LOG(WEB, "rpsapply_4x9jd2yAe8A02SqKAx start arg0obj=" << arg0obj
                << ", arg1obweb=" << arg1obweb
                << ", arg2depth=" << arg2depth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(2, "!method object/display_object_occurrence_web"));
  _f.recvob = arg0obj.as_object();
  RPS_ASSERT(_f.recvob);
  _f.objwnd = arg1obweb.as_object();
  RPS_ASSERT (_f.objwnd);
  _f.recdepth = arg2depth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_4x9jd2yAe8A02SqKAx recvob=" << _f.recvob
                << " of class:" <<  _f.recvob->compute_class(&_) << std::endl
                << "... objwnd=" << _f.objwnd
                << " of class:" <<  _f.objwnd->compute_class(&_)
                << "... depthi=" << depthi <<std::endl
                << "!method object/display_object_occurrence_web" << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_4x9jd2yAe8A02SqKAx")
                <<std::endl);
#warning unimplemented rpsapply_4x9jd2yAe8A02SqKAx
  RPS_FATAL("unimplemented rpsapply_4x9jd2yAe8A02SqKAx");
} // end of rpsapply_4x9jd2yAe8A02SqKAx !method object/display_object_occurrence_web


////////////////////////////////////////////////////////////////
// C++ closure for _5nSiRIxoYQp00MSnYA
//!method object!display_object_content_web
extern "C" rps_applyingfun_t rpsapply_5nSiRIxoYQp00MSnYA;
Rps_TwoValues
rpsapply_5nSiRIxoYQp00MSnYA (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obweb, ///
                             const Rps_Value arg2depth, //
                             const Rps_Value arg3optqtposition, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_ )
{
  /* In the usual case, this RefPerSys method is called with 3
     arguments.  But in special cases, the 4th argument is a position
     in the document of the text cursor .... */
  RPS_LOCALFRAME(rpskob_5nSiRIxoYQp00MSnYA,
                 callerframe, //
                 Rps_ObjectRef recvob;
                 Rps_ObjectRef classob;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value optqtposition;
                 Rps_ObjectRef spacob;
                 Rps_Value setattrs;
                 Rps_ObjectRef attrob;
                 Rps_Value attrval;
                 Rps_Value curcomp;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _5nSiRIxoYQp00MSnYA !method object!display_object_content_web ====
  _f.recvob = arg0obj.as_object();
  RPS_ASSERT(_f.recvob);
  _f.objwnd = arg1obweb.as_object();
  RPS_ASSERT (_f.objwnd);
  _f.recdepth = arg2depth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  _f.optqtposition = arg3optqtposition;
  RPS_ASSERT (!_f.optqtposition || _f.optqtposition.is_int());
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_f.objwnd->objmtxptr()));
  RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA start object!display_object_content_web recvob=" << _f.recvob
                << ", objwnd =" << _f.objwnd
                << " of class:" <<  _f.objwnd->compute_class(&_) << std::endl
                << "... depthi=" <<  depthi
                << std::endl << "+++ object!display_object_content_web +++");
#warning unimplemented rpsapply_5nSiRIxoYQp00MSnYA
  RPS_FATAL("unimplemented rpsapply_5nSiRIxoYQp00MSnYA");
} // end of rpsapply_5nSiRIxoYQp00MSnYA !method object!display_object_content_web



////////////////////////////////////////////////////////////////
// C++ closure for _8lKdW7lgcHV00WUOiT
//!method class/display_object_payload_web
extern "C" rps_applyingfun_t rpsapply_8lKdW7lgcHV00WUOiT;
Rps_TwoValues
rpsapply_8lKdW7lgcHV00WUOiT (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0class, const Rps_Value arg1obweb, ///
                             const Rps_Value arg2depth, const Rps_Value _arg3 __attribute__((unused)), ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_8lKdW7lgcHV00WUOiT,
                 callerframe, //
                 Rps_ObjectRef obclass; //
                 Rps_ObjectRef obweb; //
                 Rps_ObjectRef obsuper; //
                 Rps_Value depthv; //
                 Rps_Value resmainv; //
                 Rps_Value resxtrav; //
                 Rps_SetValue setselv; //
                 Rps_ObjectRef obcursel; //
                 Rps_ClosureValue curmethclos;
                );
  ////==== body of _8lKdW7lgcHV00WUOiT ====
  _f.obclass = arg0class.as_object();
  RPS_ASSERT(_f.obclass);
  _f.obweb = arg1obweb.as_object();
  RPS_ASSERT(_f.obweb);
  _f.depthv = arg2depth;
  RPS_ASSERT(_f.depthv.is_int());
  auto depthi = _f.depthv.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_8lKdW7lgcHV00WUOiT start !method class/display_object_payload_web @!@° obclass="
                << _f.obclass << ", obweb=" << _f.obweb
                << " of class:" << Rps_Value(_f.obweb).compute_class(&_)
                << ", depthi=" << depthi << std::endl
                << RPS_FULL_BACKTRACE_HERE(2,
                    "?£!? rpsapply_8lKdW7lgcHV00WUOiT !method class/display_object_payload_web")
                << std::endl
               );
#warning unimplemented rpsapply_8lKdW7lgcHV00WUOiT
  RPS_FATAL("unimplemented rpsapply_8lKdW7lgcHV00WUOiT");
} // end of rpsapply_8lKdW7lgcHV00WUOiT


///////////////////////////////////////////////////////// end of file webdisplay_rps.cc
