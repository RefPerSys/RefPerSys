/****************************************************************
 * file fltk_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the FLTK graphical user interface related code. See http://fltk.org/
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


extern "C" const char rps_fltk_gitid[];
const char rps_fltk_gitid[]= RPS_GITID;

extern "C" const char rps_fltk_date[];
const char rps_fltk_date[]= __DATE__;

void
rps_garbcoll_application(Rps_GarbageCollector&gc)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(gc.is_valid_garbcoll());
#warning incomplete rps_garbcoll_application
  RPS_WARNOUT("incomplete rps_garbcoll_application " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_garbcoll_application"));
} // end rps_garbcoll_application

bool
rps_is_main_gui_thread(void)
{
#warning unimplemented rps_is_main_gui_thread
  RPS_FATAL("unimplemented rps_is_main_gui_thread");
} // end rps_is_main_gui_thread



void
rps_run_fltk_gui(int &argc, char**argv)
{
  for (int ix=0; ix<argc; ix++)
    RPS_DEBUG_LOG(REPL, "FLTK GUI arg [" << ix << "]: " << argv[ix]);
  RPS_FATAL("unimplemented rps_run_fltk_gui");
} // rps_run_fltk_gui


////////////////////////////////////////////////////////////////
// C++ closure for _0TwK4TkhEGZ03oTa5m
//!display Val0 in Ob1Win at depth Val2Depth
extern "C" rps_applyingfun_t rpsapply_0TwK4TkhEGZ03oTa5m;
Rps_TwoValues
rpsapply_0TwK4TkhEGZ03oTa5m(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0val,
                            const Rps_Value arg1obwin, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0TwK4TkhEGZ03oTa5m,
                 callerframe, //
                 Rps_Value val0v;
                 Rps_ObjectRef winob1;
                 Rps_Value depth2v;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  _.val0v = arg0val;
  _.winob1 = arg1obwin.to_object();
  _.depth2v = arg2depth;
  int depth = _.depth2v.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_0TwK4TkhEGZ03oTa5m start val0v=" << _.val0v
                << ", winob=" << _.winob1
                << ", depth=" << depth);
  ////==== body of _0TwK4TkhEGZ03oTa5m ====
#warning unimplemented rpsapply_0TwK4TkhEGZ03oTa5m
  RPS_FATAL("unimplemented rpsapply_0TwK4TkhEGZ03oTa5m");
} // end of rpsapply_0TwK4TkhEGZ03oTa5m !display Val0 in Ob1Win at depth Val2Depth


////////////////////////////////////////////////////////////////
// C++ closure for _8KJHUldX8GJ03G5OWp
//!method int/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_8KJHUldX8GJ03G5OWp;
Rps_TwoValues
rpsapply_8KJHUldX8GJ03G5OWp(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0recv, ///
                            const Rps_Value arg1obwin, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_8KJHUldX8GJ03G5OWp,
                 callerframe, //
                 Rps_Value intv;
                 Rps_ObjectRef obwin;
                 Rps_Value depthv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _8KJHUldX8GJ03G5OWp ====
  _.intv = arg0recv;
  _.obwin = arg1obwin.as_object();
  _.depthv = arg2depth;
  RPS_DEBUG_LOG(GUI, "rpsapply_8KJHUldX8GJ03G5OWp start intv=" << _.intv
                << ", obwin=" << _.obwin
                << ", depth=" << _.depthv);
#warning unimplemented rpsapply_8KJHUldX8GJ03G5OWp
  RPS_FATAL("unimplemented rpsapply_8KJHUldX8GJ03G5OWp");
} // end of  rpsapply_8KJHUldX8GJ03G5OWp !method int/display_value_fltk


////////////////////////////////////////////////////////////////

// C++ closure for _2KnFhlj8xW800kpgPt
//!method string/display_value_fltk
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

  _.string_value = arg0_receiver;
  RPS_ASSERT(_.string_value.is_string());
  _.object_window = arg1_object_window.as_object();
  RPS_ASSERT(_.object_window);
  _.recursive_depth = arg2_recursive_depth;
  RPS_ASSERT(_.recursive_depth.is_int());
  RPS_DEBUG_LOG(GUI, "rpsapply_2KnFhlj8xW800kpgPt start string_value=" << _.string_value
                << ", object_window, =" << _.object_window
                << ", recursive_depth=" <<  _.recursive_depth);
  ////==== body of _2KnFhlj8xW800kpgPt ====
#warning unimplemented rpsapply_2KnFhlj8xW800kpgPt
  RPS_FATAL("unimplemented rpsapply_2KnFhlj8xW800kpgPt");
} // end of  rpsapply_2KnFhlj8xW800kpgPt !method string/display_value_fltk

////////////////////////////////////////////////////////////////
// C++ closure for _7oa7eIzzcxv03TmmZH
//!method double/display_value_fltk
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

  ////==== body of _7oa7eIzzcxv03TmmZH !method double/display_value_fltk ====
  _.doubleval = arg0_recv;
  RPS_ASSERT (_.doubleval.is_double());
  _.object_window = arg1_objwnd.as_object();
  RPS_ASSERT(_.object_window);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT(_.recdepth.is_int());
  RPS_DEBUG_LOG(GUI, "rpsapply_7oa7eIzzcxv03TmmZH start doubleval=" << _.doubleval
                << "object_window, =" << _.object_window
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_7oa7eIzzcxv03TmmZH
  RPS_FATAL("unimplemented rpsapply_7oa7eIzzcxv03TmmZH");
}
// end of rpsapply_7oa7eIzzcxv03TmmZH !method double/display_value_fltk




// C++ closure for _33DFyPOJxbF015ZYoi
//!method tuple/display_value_fltk
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
  ////==== body of _33DFyPOJxbF015ZYoi !method tuple/display_value_fltk ====
  _.tupleval = arg0_recv;
  RPS_ASSERT (_.tupleval.is_tuple());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_33DFyPOJxbF015ZYoi start tupleval=" << _.tupleval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_33DFyPOJxbF015ZYoi
  RPS_FATAL("unimplemented rpsapply_33DFyPOJxbF015ZYoi");
} // end of rpsapply_33DFyPOJxbF015ZYoi !method tuple/display_value_fltk


// C++ closure for _1568ZHTl0Pa00461I2
//!method set/display_value_fltk
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

  ////==== body of _1568ZHTl0Pa00461I2 !method set/display_value_fltk ====
  _.setval = arg0_recv;
  RPS_ASSERT (_.setval.is_set());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_fltk start setval=" << _.setval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_1568ZHTl0Pa00461I2
  RPS_FATAL("unimplemented rpsapply_1568ZHTl0Pa00461I2");
} // end of rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_fltk


// C++ closure for _18DO93843oX02UWzq6
//!method object/display_value_fltk
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
  RPS_DEBUG_LOG(GUI, "rpsapply_18DO93843oX02UWzq6 start arg0_objrecv="
                << arg0_objrecv << ", arg1_objwnd=" << arg1_objwnd
                << ", arg2_recdepth=" << arg2_recdepth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_18DO93843oX02UWzq6") << std::endl);
#warning unimplemented rpsapply_18DO93843oX02UWzq6
  RPS_FATAL("unimplemented rpsapply_18DO93843oX02UWzq6");

} // end of rpsapply_18DO93843oX02UWzq6 !method object/display_value_fltk


// C++ closure for _0rgijx7CCnq041IZEd
//!method immutable_instance/display_value_fltk
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
  ////==== body of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_fltk====
  _.instrecv = Rps_InstanceValue(arg0_inst.as_instance());
  RPS_ASSERT (_.instrecv);
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_0rgijx7CCnq041IZEd start instrecv=" << _.instrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_0rgijx7CCnq041IZEd
  RPS_FATAL("unimplemented rpsapply_0rgijx7CCnq041IZEd");
} // end of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_fltk




// C++ closure for _6Wi00FwXYID00gl9Ma
//!method closure/display_value_fltk
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
  ////==== body of _6Wi00FwXYID00gl9Ma !method closure/display_value_fltk ====
  _.closrecv = Rps_ClosureValue(arg0_clos.as_closure());
  RPS_ASSERT (_.closrecv);
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_6Wi00FwXYID00gl9Ma start closrecv=" << _.closrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  _.obconn = _.closrecv->conn();
  unsigned width = _.closrecv->cnt();
#warning unimplemented rpsapply_6Wi00FwXYID00gl9Ma
  RPS_FATAL("unimplemented rpsapply_6Wi00FwXYID00gl9Ma");
} // end of rpsapply_6Wi00FwXYID00gl9Ma !method closure/display_value_fltk



// C++ closure for _52zVxP3mTue034OWsD
//!method qtptr/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_52zVxP3mTue034OWsD;
Rps_TwoValues
rpsapply_52zVxP3mTue034OWsD(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_fltkptr,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth, ///
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_52zVxP3mTue034OWsD,
                 callerframe, //
                 Rps_Value qtrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _52zVxP3mTue034OWsD !method qtptr/display_value_fltk ====
  _.qtrecv = arg0_fltkptr;
  //RPS_ASSERT (_.qtrecv.is_fltkptr());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_52zVxP3mTue034OWsD start qtrecv=" << _.qtrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_52zVxP3mTue034OWsD
  RPS_FATAL("unimplemented rpsapply_52zVxP3mTue034OWsD");
}  // end of rpsapply_52zVxP3mTue034OWsD !method qtptr/display_value_fltk


// C++ closure for _42cCN1FRQSS03bzbTz
//!method json/display_value_fltk
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
  ////==== body of _42cCN1FRQSS03bzbTz !method json/display_value_fltk ====
  ;
  _.jsrecv = arg0_json;
  RPS_ASSERT (_.jsrecv.is_json());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_42cCN1FRQSS03bzbTz start jsrecv=" << _.jsrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_42cCN1FRQSS03bzbTz
  RPS_FATAL("unimplemented rpsapply_42cCN1FRQSS03bzbTz");
} // end of rpsapply_42cCN1FRQSS03bzbTz !method json/display_value_fltk




////////////////////////////////////////////////////////////////
// C++ closure for _4x9jd2yAe8A02SqKAx
//!method object/display_object_occurrence_fltk
extern "C" rps_applyingfun_t rpsapply_4x9jd2yAe8A02SqKAx;
Rps_TwoValues
rpsapply_4x9jd2yAe8A02SqKAx (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obwin, ///
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
  ////==== body of _4x9jd2yAe8A02SqKAx  !method object/display_object_occurrence_fltk ====
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx start arg0obj=" << arg0obj
                << ", arg1obwin=" << arg1obwin
                << ", arg2depth=" << arg2depth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(2, "!method object/display_object_occurrence_fltk"));
  _.recvob = arg0obj.as_object();
  RPS_ASSERT(_.recvob);
  _.objwnd = arg1obwin.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2depth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx recvob=" << _.recvob
                << " of class:" <<  _.recvob->compute_class(&_) << std::endl
                << "... objwnd=" << _.objwnd
                << " of class:" <<  _.objwnd->compute_class(&_)
                << "... depthi=" << depthi <<std::endl
                << "!method object/display_object_occurrence_fltk" << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_4x9jd2yAe8A02SqKAx")
                <<std::endl);
#warning unimplemented rpsapply_4x9jd2yAe8A02SqKAx
  RPS_FATAL("unimplemented rpsapply_4x9jd2yAe8A02SqKAx");
} // end of rpsapply_4x9jd2yAe8A02SqKAx !method object/display_object_occurrence_fltk


////////////////////////////////////////////////////////////////
// C++ closure for _5nSiRIxoYQp00MSnYA
//!method object!display_object_content_fltk
extern "C" rps_applyingfun_t rpsapply_5nSiRIxoYQp00MSnYA;
Rps_TwoValues
rpsapply_5nSiRIxoYQp00MSnYA (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obwin, ///
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
  ////==== body of _5nSiRIxoYQp00MSnYA !method object!display_object_content_fltk ====
  _.recvob = arg0obj.as_object();
  RPS_ASSERT(_.recvob);
  _.objwnd = arg1obwin.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2depth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  _.optqtposition = arg3optqtposition;
  RPS_ASSERT (!_.optqtposition || _.optqtposition.is_int());
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  RPS_DEBUG_LOG(GUI, "rpsapply_5nSiRIxoYQp00MSnYA start object!display_object_content_fltk recvob=" << _.recvob
                << ", objwnd =" << _.objwnd
                << " of class:" <<  _.objwnd->compute_class(&_) << std::endl
                << "... depthi=" <<  depthi
                << std::endl << "+++ object!display_object_content_fltk +++");
#warning unimplemented rpsapply_5nSiRIxoYQp00MSnYA
  RPS_FATAL("unimplemented rpsapply_5nSiRIxoYQp00MSnYA");
} // end of rpsapply_5nSiRIxoYQp00MSnYA !method object!display_object_content_fltk



////////////////////////////////////////////////////////////////
// C++ closure for _8lKdW7lgcHV00WUOiT
//!method class/display_object_payload_fltk
extern "C" rps_applyingfun_t rpsapply_8lKdW7lgcHV00WUOiT;
Rps_TwoValues
rpsapply_8lKdW7lgcHV00WUOiT (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0class, const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth, const Rps_Value _arg3 __attribute__((unused)), ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_8lKdW7lgcHV00WUOiT,
                 callerframe, //
                 Rps_ObjectRef obclass; //
                 Rps_ObjectRef obwin; //
                 Rps_ObjectRef obsuper; //
                 Rps_Value depthv; //
                 Rps_Value resmainv; //
                 Rps_Value resxtrav; //
                 Rps_SetValue setselv; //
                 Rps_ObjectRef obcursel; //
                 Rps_ClosureValue curmethclos;
                );
  ////==== body of _8lKdW7lgcHV00WUOiT ====
  _.obclass = arg0class.as_object();
  RPS_ASSERT(_.obclass);
  _.obwin = arg1obwin.as_object();
  RPS_ASSERT(_.obwin);
  _.depthv = arg2depth;
  RPS_ASSERT(_.depthv.is_int());
  auto depthi = _.depthv.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_8lKdW7lgcHV00WUOiT start !method class/display_object_payload_fltk @!@° obclass="
                << _.obclass << ", obwin=" << _.obwin
                << " of class:" << Rps_Value(_.obwin).compute_class(&_)
                << ", depthi=" << depthi << std::endl
                << RPS_FULL_BACKTRACE_HERE(2,
                    "?£!? rpsapply_8lKdW7lgcHV00WUOiT !method class/display_object_payload_fltk")
                << std::endl
               );
#warning unimplemented rpsapply_8lKdW7lgcHV00WUOiT
  RPS_FATAL("unimplemented rpsapply_8lKdW7lgcHV00WUOiT");
} // end of rpsapply_8lKdW7lgcHV00WUOiT


/***********
 * Temporarily kept constant objects
 *
 * For a few weeks in may or june 2020, we mention here as comments
 * several constant objects.
 *
 *    rpskob_0rgijx7CCnq041IZEd, rpskob_0TwK4TkhEGZ03oTa5m,
 *    rpskob_1568ZHTl0Pa00461I2, rpskob_18DO93843oX02UWzq6,
 *    rpskob_1wihX3eWD9o00QnxUX, rpskob_1DUx3zfUzIb04lqNVt,
 *    rpskob_1Win5yzaf1L02cBUlV, rpskob_2KnFhlj8xW800kpgPt,
 *    rpskob_33DFyPOJxbF015ZYoi, rpskob_41OFI3r0S1t03qdB2E,
 *    rpskob_42cCN1FRQSS03bzbTz, rpskob_4ojpzRzyRWz02DNWMe,
 *    rpskob_4x9jd2yAe8A02SqKAx, rpskob_52zVxP3mTue034OWsD,
 *    rpskob_5nSiRIxoYQp00MSnYA, rpskob_6Wi00FwXYID00gl9Ma,
 *    rpskob_7oa7eIzzcxv03TmmZH, rpskob_8lKdW7lgcHV00WUOiT,
 *    rpskob_8KJHUldX8GJ03G5OWp, rpskob_9uwZtDshW4401x6MsY, etc..
 *
 ************/


//////////////////////////////////////// end of file fltk_rps.cc
