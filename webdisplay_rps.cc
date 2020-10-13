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
                 Rps_ObjectRef compob;
                 Rps_ObjectRef connob;
                 Rps_Value sonv;
                 //....etc....
                );
  _f.val0v = arg0val;
  _f.webob1 = arg1obweb.to_object();
  _f.depth2v = arg2depth;
  int depth = _f.depth2v.to_int();
  constexpr int max_depth = 5; // FIXME, should be improved
  RPS_DEBUG_LOG(WEB, "rpsapply_0TwK4TkhEGZ03oTa5m start val0v=" << _f.val0v
                << ", webob=" << _f.webob1
                << ", depth=" << depth);
  ////==== body of _0TwK4TkhEGZ03oTa5m ====
  std::ostream* pout = rps_web_output(&_, _f.webob1, RPS_CHECK_OUTPUT);
  /////////
  /// we now can emit HTML code into pwebex, using its webex_resp...
  switch (_f.val0v.type())
    {
    case Rps_Type::Int:
    {
      intptr_t i = _f.val0v.as_int();
      *pout << "<span class='intval_rpscl'>" << i << "</span>";
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::None:
    {
      *pout << "<span class='nullval_rpscl'>_</span>";
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::String:
    {
      const std::string str = _f.val0v.as_cppstring();
      *pout << "<q class='decor_rpscl'>"
            << "<span class='stringval_rpscl'>"
            << Rps_Html_Nl2br_String(str)
            << "</span>"
            << "</q>";
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::Double:
    {
      double x = _f.val0v.as_double();
      *pout << "<span class='doubleval_rpscl'>"
            << x
            << "</span>";
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::Set:
    {
      constexpr unsigned period_nl = 5;
      unsigned nbelem = _f.val0v.as_set()->cardinal();
      if (nbelem < period_nl)
        *pout << "<span class='smallsetval_rpscl'>";
      else
        *pout << "<div class='bigsetval_rpscl'>" << std::endl;
      *pout << "<span class='decorval_rpscl'>{</span>";
      if (depth <  max_depth)
        {
          for (unsigned ix=0; ix < nbelem; ix++)
            {
              _f.compob = _f.val0v.as_set()->at(ix);
              if (ix>0 && ix%period_nl == 0)
                *pout << "<br class='decor_rpscl'/>" << std::endl;
              else if (ix>0)
                *pout << ' ';
              rps_web_display_html_for_objref(&_, _f.compob,
                                              _f.webob1,
                                              depth+1);
              _f.compob = nullptr;
            }
        }
      else
        {
          *pout << "<span class='decorval_rpscl'>…</span>";
          //U+2026 HORIZONTAL ELLIPSIS
        }
      *pout << "<span class='decorval_rpscl'>}</span>";
      if (nbelem < period_nl)
        *pout << "</span>"; // for smallsetval_rpscl
      else
        *pout << "</div>" << std::endl; // for bigsetval_rpscl
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::Tuple:
    {
      constexpr unsigned period_nl = 5;
      unsigned nbcomp = _f.val0v.as_tuple()->cnt();
      if (nbcomp < period_nl)
        *pout << "<span class='smalltupleval_rpscl'>";
      else
        *pout << "<div class='bigtupleval_rpscl'>" << std::endl;
      *pout << "<span class='decorval_rpscl'>[</span>";
      if (depth <  max_depth)
        {
          for (unsigned ix=0; ix < nbcomp; ix++)
            {
              _f.compob = _f.val0v.as_tuple()->at(ix);
              if (ix>0 && ix%period_nl == 0)
                *pout << "<br class='decor_rpscl'/>" << std::endl;
              else if (ix>0)
                *pout << ' ';
              rps_web_display_html_for_objref(&_, _f.compob,
                                              _f.webob1,
                                              depth+1);
              _f.compob = nullptr;
            }
        }
      else
        {
          *pout << "<span class='decorval_rpscl'>…</span>";
          //U+2026 HORIZONTAL ELLIPSIS
        }
      *pout << "<span class='decorval_rpscl'>]</span>";
      if (nbcomp < period_nl)
        *pout << "</span>"; // for smalltupleval_rpscl
      else
        *pout << "</div>" << std::endl; // for bigtupleval_rpscl
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::Object:
    {
      rps_web_display_html_for_objref(&_,
                                      _f.val0v.as_object(),
                                      _f.webob1,
                                      depth+1);
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::Closure:
    {
      constexpr unsigned period_nl = 5;
      _f.connob = _f.val0v.as_closure()->conn();
      unsigned arity = _f.val0v.as_closure()->cnt();
      if (arity > period_nl)
        *pout << "<div class='bigclosureval_rpscl'>" << std::endl;
      else
        *pout << "<span class='closureval_rpscl'>";
      *pout << "<span class='decorval_rpscl'>𝛌</span>"; //U+1D6CC MATHEMATICAL BOLD SMALL LAMDA
      RPS_ASSERT(_f.connob);
      if (depth <  max_depth)
        {
          rps_web_display_html_for_objref(&_,
                                          _f.connob,
                                          _f.webob1,
                                          1+(depth+max_depth)/2);
          *pout << "<span class='decorval_rpscl'>⁅</span>";
          //U+U+2045 LEFT SQUARE BRACKET WITH QUILL2045 LEFT SQUARE BRACKET WITH QUILL
          for (unsigned ix=0; ix<arity; ix++)
            {
              if (ix>0 && ix%period_nl == 0)
                *pout << "<br class='decor_rpscl'/>" << std::endl;
              else if (ix>0)
                *pout << ' ';
              _f.sonv = _f.val0v.as_closure()->at(ix);
              rps_web_display_html_for_value(&_,
                                             _f.sonv,
                                             _f.webob1,
                                             depth+1);
              _f.sonv = nullptr;
            }
          *pout << "<span class='decorval_rpscl'>⁆</span>";
          //U+2046 RIGHT SQUARE BRACKET WITH QUILL
        }
      else   // too deep
        {
          rps_web_display_html_for_objref(&_,
                                          _f.connob,
                                          _f.webob1,
                                          max_depth);
          *pout << "<sup class='arity_rpscl'>" << arity << "</sup>";
        }
      if (arity > period_nl)
        *pout << "</div>" << std::endl; // for bigclosureval_rpscl
      else
        *pout << "</span>" << std::endl; // for closureval_rpscl
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::Instance:
    {
      constexpr unsigned period_nl = 5;
      _f.connob = _f.val0v.as_instance()->conn();
      unsigned nbsons =  _f.val0v.as_instance()->cnt();
      if (nbsons > period_nl)
        *pout << "<div class='biginstanceval_rpscl'>" << std::endl;
      else
        *pout << "<span class='instanceval_rpscl'>";
      *pout << "⊠"; // U+22A0 SQUARED TIMES
      rps_web_display_html_for_objref(&_,
                                      _f.connob,
                                      _f.webob1,
                                      (depth+3<max_depth)?(depth+3):max_depth);
      if (depth <  max_depth)
        {
          *pout << " ⟨";	// U+27E8 MATHEMATICAL LEFT ANGLE BRACKET
          for (unsigned ix=0; ix<nbsons; ix++)
            {
              if (ix>0 && ix%period_nl == 0)
                *pout << "<br class='decor_rpscl'/>" << std::endl;
              else if (ix>0)
                *pout << ' ';
              _f.sonv = _f.val0v.as_instance()->at(ix);
              rps_web_display_html_for_value(&_,
                                             _f.sonv,
                                             _f.webob1,
                                             depth+1);
              _f.sonv = nullptr;
            }
          *pout << "⟩"; //U+27E9 MATHEMATICAL RIGHT ANGLE BRACKET
        }
      else   // too deep
        {
          *pout << "┄<sup class='arity_rpscl'>" << nbsons << "</sup>";
        }
      if (nbsons > period_nl)
        *pout << "</div>" <<   std::endl;// for biginstanceval_rpscl
      else
        *pout << "</span>" << std::endl; // for instanceval_rpscl
      return Rps_TwoValues{ _f.webob1};
    }
    case Rps_Type::Json:
    {
      if (depth <  max_depth)
        {
          std::ostringstream outstr;
          outstr <<  _f.val0v.as_json() << std::flush;
          std::string jstr = outstr.str();
          *pout << "<span class='json_rpscl'>JSON:" << Rps_Html_Nl2br_String(jstr)
                << "</span>" << std::endl;
        }
      else   // too deep
        {
          *pout << "<span class='json_rpscl'>JSON_" << _f.val0v.as_json()->json().size() << "</span>";
        }
      return Rps_TwoValues{ _f.webob1};
    }
    //// TODO: for composite values we need to use the depth. If a
    //// threshold has been reached, we don't display contents.
    default:
      RPS_FATALOUT("rpsapply_0TwK4TkhEGZ03oTa5m val0v=" << _f.val0v
                   << " has unexpected type"
                   << ", webob=" << _f.webob1
                   << ", depth=" << depth
                  );
    };				// end switch _f.val0v.type()
} // end of rpsapply_0TwK4TkhEGZ03oTa5m !display Val0 in Ob1Win at depth Val2Depth



void
rps_web_display_html_for_value(Rps_CallFrame*callerframe,
                               const Rps_Value arg0val, //
                               const Rps_ObjectRef arg1obweb, ///
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
  std::ostream* pout = rps_web_output(&_, _f.webob1, RPS_CHECK_OUTPUT);
  if (!_f.val0v)
    {
      *pout << "<span class='nullval_rpscl'>_</span>";
      return;
    }
  else if (_f.val0v.is_empty())
    {
      *pout << "<span class='emptyval_rpscl'>?_?</span>";
      return;
    }
  // _0TwK4TkhEGZ03oTa5m is for the rpsapply_0TwK4TkhEGZ03oTa5m above
  Rps_ClosureValue(rpskob_0TwK4TkhEGZ03oTa5m, {}).apply3(&_,_f.val0v,_f.webob1, _f.depth2v);
} // end rps_web_display_html_for_value


void
rps_web_display_html_for_objref(Rps_CallFrame*callerframe,
                                Rps_ObjectRef arg0ob, //
                                Rps_ObjectRef arg1obweb, ///
                                int depth)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe, //
                 Rps_ObjectRef obdisp0; //
                 Rps_ObjectRef webob1; //
                 Rps_Value depth2v; //
                 Rps_Value namev;
                );
  _f.obdisp0 = arg0ob;
  _f.webob1 = arg1obweb;
  RPS_ASSERT(_f.obdisp0);
  std::ostream* pout = rps_web_output(&_, _f.webob1, RPS_CHECK_OUTPUT);
  std::lock_guard<std::recursive_mutex> guobdisp(*(_f.obdisp0->objmtxptr()));
  _f.namev = _f.obdisp0->get_attr1(&_,
                                   RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); //name∈named_attribute
  if (_f.namev.is_string())
    {
      *pout << "<span class='namedob_rpscl' rps_obid='" << _f.obdisp0->oid() << "'>"
            << Rps_Html_Nl2br_String(_f.namev.to_string()->cppstring())
            << "</span>";
    }
  else   // no name
    {
      *pout << "<span class='anonob_rpscl' rps_obid='" << _f.obdisp0->oid() << "'>"
            << _f.obdisp0->oid() << "</span>";
    };
} // end rps_web_display_html_for_objref

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
                << ", depthv=" << _f.depthv);
  int depth =  _f.depthv.to_int();
  std::ostream* pout = rps_web_output(&_, _f.obweb, RPS_CHECK_OUTPUT);
  RPS_ASSERT(pout);
  intptr_t i = _f.intv.as_int();
  *pout << "<span class='intval_rpscl'>" << i << "</span>";
  return Rps_TwoValues{ _f.obweb};
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
                 Rps_Value stringv;
                 Rps_ObjectRef obweb;
                 Rps_Value depthv;
                );

  _f.stringv = arg0_receiver;
  RPS_ASSERT(_f.stringv.is_string());
  _f.obweb = arg1_object_window.as_object();
  RPS_ASSERT(_f.obweb);
  _f.depthv = arg2_recursive_depth;
  RPS_ASSERT(_f.depthv.is_int());
  ////==== body of _2KnFhlj8xW800kpgPt ====
  RPS_DEBUG_LOG(WEB, "rpsapply_2KnFhlj8xW800kpgPt °string/display_value_web start stringv=" << _f.stringv
                << ", obweb=" << _f.obweb
                << ", depthv=" <<  _f.depthv);
  int depth =  _f.depthv.to_int();
  std::ostream* pout = rps_web_output(&_, _f.obweb, RPS_CHECK_OUTPUT);
  RPS_ASSERT(pout);
  const std::string str = _f.stringv.as_cppstring();
  *pout << "<q class='decor_rpscl'>"
        << "<span class='stringval_rpscl'>"
        << Rps_Html_Nl2br_String(str)
        << "</span>"
        << "</q>";
  return Rps_TwoValues{ _f.obweb};
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
                 Rps_Value doublev;
                 Rps_ObjectRef obweb;
                 Rps_Value depthv;
                );

  ////==== body of _7oa7eIzzcxv03TmmZH !method double/display_value_web ====
  _f.doublev = arg0_recv;
  RPS_ASSERT (_f.doublev.is_double());
  _f.obweb = arg1_objwnd.as_object();
  RPS_ASSERT(_f.obweb);
  _f.depthv = arg2_recdepth;
  RPS_ASSERT(_f.depthv.is_int());
  RPS_DEBUG_LOG(WEB, "rpsapply_7oa7eIzzcxv03TmmZH start doublev=" << _f.doublev
                << "obweb=" << _f.obweb
                << ", depthv=" <<  _f.depthv);
  int depth =  _f.depthv.to_int();
  std::ostream* pout = rps_web_output(&_, _f.obweb, RPS_CHECK_OUTPUT);
  double x = _f.doublev.as_double();
  *pout << "<span class='doubleval_rpscl'>"
        << x
        << "</span>";
  return Rps_TwoValues{ _f.obweb};
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
