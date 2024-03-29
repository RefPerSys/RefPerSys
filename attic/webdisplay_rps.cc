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
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.webob1, RPS_CHECK_OSTREAM_PTR);
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
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.webob1, RPS_CHECK_OSTREAM_PTR);
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
                                [[maybe_unused]] int depth)
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
  if (depth >= 2)
    RPS_DEBUG_LOG(WEB, "rps_web_display_html_for_objref obdisp0=" << _f.obdisp0
                  << " webob1=" << _f.webob1 << " depth:" << depth);
  else
    RPS_DEBUG_LOG(WEB, "rps_web_display_html_for_objref obdisp0=" << _f.obdisp0
                  << " webob1=" << _f.webob1 << " depth:" << depth << std::endl
                  << RPS_DEBUG_BACKTRACE_HERE(1, "rps_web_display_html_for_objref"));
  //
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.webob1, RPS_CHECK_OSTREAM_PTR);
  if (!_f.obdisp0)
    {
      *pout << "<span class='nullobref_rpscl'>__</span>";
      return;
    }
  std::lock_guard<std::recursive_mutex> guobdisp(*(_f.obdisp0->objmtxptr()));
  _f.namev = _f.obdisp0->get_attr1(&_,
                                   RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); //name∈named_attribute
  if (_f.namev.is_string())
    {
      *pout << "<span class='namedob_rpscl' rps_obid='" << _f.obdisp0->oid() << "'>"
            << Rps_Html_Nl2br_String(_f.namev.to_string()->cppstring())
            << "</span>";
      if (depth==0)
        {
          *pout << "⁖" // U+2056 THREE DOT PUNCTUATION
                <<  "<span class='namedoid_rpscl' rps_obid='" << _f.obdisp0->oid() << "'>"
                << _f.obdisp0->oid()
                << "</span>";
        }
    }
  else   // no name
    {
      *pout << "<span class='anonob_rpscl' rps_obid='" << _f.obdisp0->oid() << "'>"
            << _f.obdisp0->oid() << "</span>";
    };
  RPS_DEBUG_LOG(WEB, "end rps_web_display_html_for_objref obdisp0=" << _f.obdisp0 << " depth:" << depth << " filetell:" << pout->tellp());
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
  RPS_ASSERT(depth>=0);
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
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
  RPS_ASSERT(depth>=0);
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
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
  RPS_ASSERT(depth>=0);
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  RPS_ASSERT(pout);
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
                            const Rps_Value arg1_objweb, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_33DFyPOJxbF015ZYoi,
                 callerframe, //
                 Rps_Value tuplev;
                 Rps_ObjectRef obweb;
                 Rps_ObjectRef compob;
                 Rps_Value recdepth;
                );
  ////==== body of _33DFyPOJxbF015ZYoi !method tuple/display_value_web ====
  constexpr int max_depth = 5; // FIXME, should be improved
  _f.tuplev = arg0_recv;
  RPS_ASSERT (_f.tuplev.is_tuple());
  _f.obweb = arg1_objweb.as_object();
  RPS_ASSERT (_f.obweb);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_33DFyPOJxbF015ZYoi start tuplev=" << _f.tuplev
                << "obweb =" << _f.obweb
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  RPS_ASSERT(pout);
  constexpr unsigned period_nl = 5;
  unsigned nbcomp = _f.tuplev.as_tuple()->cnt();
  if (nbcomp < period_nl)
    *pout << "<span class='smalltupleval_rpscl'>";
  else
    *pout << "<div class='bigtupleval_rpscl'>" << std::endl;
  *pout << "<span class='decorval_rpscl'>[</span>";
  if (depthi <  max_depth)
    {
      for (unsigned ix=0; ix < nbcomp; ix++)
        {
          _f.compob = _f.tuplev.as_tuple()->at(ix);
          if (ix>0 && ix%period_nl == 0)
            *pout << "<br class='decor_rpscl'/>" << std::endl;
          else if (ix>0)
            *pout << ' ';
          rps_web_display_html_for_objref(&_, _f.compob,
                                          _f.obweb,
                                          depthi+1);
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
  return Rps_TwoValues{ _f.obweb};
} // end of rpsapply_33DFyPOJxbF015ZYoi !method tuple/display_value_web


// C++ closure for _1568ZHTl0Pa00461I2
//!method set/display_value_web
extern "C" rps_applyingfun_t rpsapply_1568ZHTl0Pa00461I2;
Rps_TwoValues
rpsapply_1568ZHTl0Pa00461I2(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_obweb, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_1568ZHTl0Pa00461I2,
                 callerframe, //
                 Rps_Value setval;
                 Rps_ObjectRef obweb;
                 Rps_Value recdepth;
                 Rps_ObjectRef compob;
                );

  ////==== body of _1568ZHTl0Pa00461I2 !method set/display_value_web ====
  _f.setval = arg0_recv;
  RPS_ASSERT (_f.setval.is_set());
  _f.obweb = arg1_obweb.as_object();
  RPS_ASSERT (_f.obweb);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_web start setval=" << _f.setval
                << "obweb =" << _f.obweb
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  constexpr int max_depth = 5; // FIXME, should be improved
  RPS_ASSERT(pout);
  constexpr unsigned period_nl = 5;
  unsigned nbelem = _f.setval.as_set()->cardinal();
  if (nbelem < period_nl)
    *pout << "<span class='smallsetval_rpscl'>";
  else
    *pout << "<div class='bigsetval_rpscl'>" << std::endl;
  *pout << "<span class='decorval_rpscl'>{</span>";
  if (depthi <  max_depth)
    {
      for (unsigned ix=0; ix < nbelem; ix++)
        {
          _f.compob = _f.setval.as_set()->at(ix);
          if (ix>0 && ix%period_nl == 0)
            *pout << "<br class='decor_rpscl'/>" << std::endl;
          else if (ix>0)
            *pout << ' ';
          rps_web_display_html_for_objref(&_, _f.compob,
                                          _f.obweb,
                                          depthi+1);
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
  return Rps_TwoValues{ _f.obweb};
} // end of rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_web


// C++ closure for _18DO93843oX02UWzq6
//!method object/display_value_web
extern "C" rps_applyingfun_t rpsapply_18DO93843oX02UWzq6;
Rps_TwoValues
rpsapply_18DO93843oX02UWzq6(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_objrecv,
                            const Rps_Value arg1_obweb, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_18DO93843oX02UWzq6,
                 callerframe, //
                 Rps_ObjectRef obrecv;
                 Rps_ObjectRef obweb;
                 Rps_Value recdepth;
                 //....etc....
                );
  _f.obrecv = arg0_objrecv.as_object();
  _f.obweb = arg1_obweb.as_object();
  _f.recdepth = arg2_recdepth;
  RPS_DEBUG_LOG(WEB, "rpsapply_18DO93843oX02UWzq6 start arg0_objrecv="
                << arg0_objrecv << ", arg1_obweb=" << arg1_obweb
                << ", arg2_recdepth=" << arg2_recdepth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_18DO93843oX02UWzq6") << std::endl);
  RPS_ASSERT(_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  RPS_ASSERT(pout);
  rps_web_display_html_for_objref(&_,
                                  _f.obrecv,
                                  _f.obweb,
                                  depthi);
  return Rps_TwoValues{ _f.obweb};
} // end of rpsapply_18DO93843oX02UWzq6 !method object/display_value_web


// C++ closure for _0rgijx7CCnq041IZEd
//!method immutable_instance/display_value_web
extern "C" rps_applyingfun_t rpsapply_0rgijx7CCnq041IZEd;
Rps_TwoValues
rpsapply_0rgijx7CCnq041IZEd (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_inst, ///
                             const Rps_Value arg1_obweb, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0rgijx7CCnq041IZEd,
                 callerframe, //
                 Rps_InstanceValue instrecv;
                 Rps_ObjectRef obweb;
                 Rps_ObjectRef connob;
                 Rps_ObjectRef obattr;
                 Rps_Value recdepth;
                 Rps_Value sonv;
                 //....etc....
                );
  ////==== body of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_web====
  _f.instrecv = Rps_InstanceValue(arg0_inst.as_instance());
  RPS_ASSERT (_f.instrecv);
  _f.obweb = arg1_obweb.as_object();
  RPS_ASSERT (_f.obweb);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  constexpr int max_depth = 5; // FIXME, should be improved
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  RPS_ASSERT(pout);
  RPS_DEBUG_LOG(WEB, "rpsapply_0rgijx7CCnq041IZEd start instrecv=" << _f.instrecv
                << "obweb =" << _f.obweb
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
  constexpr unsigned period_nl = 5;
  _f.connob = _f.instrecv->conn();
  unsigned nbsons =  _f.instrecv->cnt();
  if (nbsons > period_nl)
    *pout << "<div class='biginstanceval_rpscl'>" << std::endl;
  else
    *pout << "<span class='instanceval_rpscl'>";
  *pout << "⊠"; // U+22A0 SQUARED TIMES
  rps_web_display_html_for_objref(&_,
                                  _f.connob,
                                  _f.obweb,
                                  (depthi+3<max_depth)?(depthi+3):max_depth);
  if (depthi <  max_depth)
    {
      *pout << " ⟨";	// U+27E8 MATHEMATICAL LEFT ANGLE BRACKET
      for (unsigned ix=0; ix<nbsons; ix++)
        {
          if (ix>0 && ix%period_nl == 0)
            *pout << "<br class='decor_rpscl'/>" << std::endl;
          else if (ix>0)
            *pout << ' ';
          _f.sonv = _f.instrecv->at(ix);
          rps_web_display_html_for_value(&_,
                                         _f.sonv,
                                         _f.obweb,
                                         depthi+1);
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
  return Rps_TwoValues{ _f.obweb};
} // end of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_web




// C++ closure for _6Wi00FwXYID00gl9Ma
//!method closure/display_value_web
extern "C" rps_applyingfun_t rpsapply_6Wi00FwXYID00gl9Ma;
Rps_TwoValues
rpsapply_6Wi00FwXYID00gl9Ma (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_clos, ///
                             const Rps_Value arg1_obweb, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_6Wi00FwXYID00gl9Ma,
                 callerframe, //
                 Rps_ClosureValue closrecv;
                 Rps_ObjectRef obweb;
                 Rps_ObjectRef obconn;
                 Rps_ObjectRef obmeta;
                 Rps_Value recdepth;
                 Rps_Value compv;
                 //....etc....
                );
  ////==== body of _6Wi00FwXYID00gl9Ma !method closure/display_value_web ====
  _f.closrecv = Rps_ClosureValue(arg0_clos.as_closure());
  RPS_ASSERT (_f.closrecv);
  _f.obweb = arg1_obweb.as_object();
  RPS_ASSERT (_f.obweb);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  constexpr unsigned period_nl = 5; // FIXME, should be improved
  constexpr int max_depth = 5; // FIXME, should be improved
  RPS_DEBUG_LOG(WEB, "rpsapply_6Wi00FwXYID00gl9Ma start closrecv=" << _f.closrecv
                << "obweb =" << _f.obweb
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  RPS_ASSERT(pout);
  _f.obconn = _f.closrecv->conn();
  unsigned arity = _f.closrecv->cnt();
  if (arity > period_nl)
    *pout << "<div class='bigclosureval_rpscl'>" << std::endl;
  else
    *pout << "<span class='closureval_rpscl'>";
  *pout << "<span class='decorval_rpscl'>𝛌</span>"; //U+1D6CC MATHEMATICAL BOLD SMALL LAMDA
  RPS_ASSERT(_f.obconn);
  if (depthi <  max_depth)
    {
      rps_web_display_html_for_objref(&_,
                                      _f.obconn,
                                      _f.obweb,
                                      1+(depthi+max_depth)/2);
      *pout << "<span class='decorval_rpscl'>⁅</span>";
      //U+U+2045 LEFT SQUARE BRACKET WITH QUILL2045 LEFT SQUARE BRACKET WITH QUILL
      for (unsigned ix=0; ix<arity; ix++)
        {
          if (ix>0 && ix%period_nl == 0)
            *pout << "<br class='decor_rpscl'/>" << std::endl;
          else if (ix>0)
            *pout << ' ';
          _f.compv = _f.closrecv->at(ix);
          rps_web_display_html_for_value(&_,
                                         _f.compv,
                                         _f.obweb,
                                         depthi+1);
          _f.compv = nullptr;
        }
      *pout << "<span class='decorval_rpscl'>⁆</span>";
      //U+2046 RIGHT SQUARE BRACKET WITH QUILL
    }
  else   // too deep
    {
      rps_web_display_html_for_objref(&_,
                                      _f.obconn,
                                      _f.obweb,
                                      max_depth);
      *pout << "<sup class='arity_rpscl'>" << arity << "</sup>";
    }
  if (arity > period_nl)
    *pout << "</div>" << std::endl; // for bigclosureval_rpscl
  else
    *pout << "</span>" << std::endl; // for closureval_rpscl
  return Rps_TwoValues{ _f.obweb};
} // end of rpsapply_6Wi00FwXYID00gl9Ma !method closure/display_value_web



// C++ closure for _42cCN1FRQSS03bzbTz
//!method json/display_value_web
extern "C" rps_applyingfun_t rpsapply_42cCN1FRQSS03bzbTz;
Rps_TwoValues
rpsapply_42cCN1FRQSS03bzbTz(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_json,
                            const Rps_Value arg1_obweb, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_42cCN1FRQSS03bzbTz,
                 callerframe, //
                 Rps_Value jsrecv;
                 Rps_ObjectRef obweb;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );
  ////==== body of _42cCN1FRQSS03bzbTz !method json/display_value_web ====
  ;
  _f.jsrecv = arg0_json;
  RPS_ASSERT (_f.jsrecv.is_json());
  _f.obweb = arg1_obweb.as_object();
  RPS_ASSERT (_f.obweb);
  _f.recdepth = arg2_recdepth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_42cCN1FRQSS03bzbTz start jsrecv=" << _f.jsrecv
                << "obweb =" << _f.obweb
                << ", recdepth=" <<  _f.recdepth
                << ", depthi=" << depthi);
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  constexpr int max_depth = 5; // FIXME, should be improved
  RPS_ASSERT(pout);
  if (depthi <  max_depth)
    {
      std::ostringstream outstr;
      outstr <<  _f.jsrecv.as_json() << std::flush;
      std::string jstr = outstr.str();
      *pout << "<span class='json_rpscl'>JSON:" << Rps_Html_Nl2br_String(jstr)
            << "</span>" << std::endl;
    }
  else   // too deep
    {
      *pout << "<span class='json_rpscl'>JSON_" << _f.jsrecv.as_json()->json().size() << "</span>";
    }
  return Rps_TwoValues{ _f.obweb};
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
                 Rps_ObjectRef obweb;
                 Rps_Value recdepth;
                 //....etc....
                );
  ////==== body of _4x9jd2yAe8A02SqKAx  !method object/display_object_occurrence_web ====
  RPS_DEBUG_LOG(WEB, "rpsapply_4x9jd2yAe8A02SqKAx start arg0obj=" << arg0obj
                << ", arg1obweb=" << arg1obweb
                << ", arg2depth=" << arg2depth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(2, "!method object/display_object_occurrence_web"));
  _f.recvob = arg0obj.as_object();
  RPS_ASSERT(_f.recvob);
  _f.obweb = arg1obweb.as_object();
  RPS_ASSERT (_f.obweb);
  _f.recdepth = arg2depth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  RPS_DEBUG_LOG(WEB, "rpsapply_4x9jd2yAe8A02SqKAx recvob=" << _f.recvob
                << " of class:" <<  _f.recvob->compute_class(&_) << std::endl
                << "... obweb=" << _f.obweb
                << " of class:" <<  _f.obweb->compute_class(&_)
                << "... depthi=" << depthi <<std::endl
                << "!method object/display_object_occurrence_web" << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_4x9jd2yAe8A02SqKAx")
                <<std::endl);
  rps_web_display_html_for_objref(&_,
                                  _f.recvob,
                                  _f.obweb,
                                  depthi);
  return Rps_TwoValues{ _f.obweb};
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
                             const Rps_Value arg3optdocposv, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_ )
{
  /* In the usual case, this RefPerSys method is called with 3
     arguments.  But in special cases, the 4th argument is a position
     in the document of the text cursor .... */
  RPS_LOCALFRAME(rpskob_5nSiRIxoYQp00MSnYA,
                 callerframe, //
                 Rps_ObjectRef recvob;
                 Rps_ObjectRef classob;
                 Rps_ObjectRef obweb;
                 Rps_ObjectRef obsel_display_object_payload_web;
                 Rps_Value recdepth;
                 Rps_Value optdocposv;
                 Rps_ObjectRef spacob;
                 Rps_SetValue setattrs;
                 Rps_ObjectRef attrob;
                 Rps_Value attrval;
                 Rps_Value curcomp;
                 //....etc....
                );
  ////==== body of _5nSiRIxoYQp00MSnYA !method object!display_object_content_web ====
  _f.recvob = arg0obj.as_object();
  RPS_ASSERT(_f.recvob);
  _f.obweb = arg1obweb.as_object();
  RPS_ASSERT (_f.obweb);
  _f.recdepth = arg2depth;
  RPS_ASSERT (_f.recdepth.is_int());
  auto depthi = _f.recdepth.to_int();
  _f.optdocposv = arg3optdocposv;
  // The below is a temporary and ugly workaround, since
  // _0Z23UbC0G9b01WZZPN is not a root object in commit
  // 0edbb672f5c63034
  _f.obsel_display_object_payload_web =
    Rps_PayloadSymbol::find_named_object(std::string("display_object_payload_web"));
  RPS_ASSERT (!_f.optdocposv || _f.optdocposv.is_int());
  std::lock_guard<std::recursive_mutex> obwebmtx(*(_f.obweb->objmtxptr()));
  RPS_DEBUGNL_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA start object!display_object_content_web recvob=" << _f.recvob
                  << " of class:" <<  _f.recvob->compute_class(&_)
                  << ", obweb =" << _f.obweb
                  << " of class:" <<  _f.obweb->compute_class(&_) << std::endl
                  << "... depthi=" <<  depthi
                  << std::endl
                  << "+++ object!display_object_content_web +++");
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  RPS_ASSERT(pout);
  _f.setattrs = _f.recvob->set_of_attributes(&_);
  _f.classob = _f.recvob->compute_class(&_);
  _f.spacob = _f.recvob->get_space();
  RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA object!display_object_content_web recvob=" << _f.recvob
                << " setattrs=" << _f.setattrs
                << ", classob=" << _f.classob
                << ", spacob=" << _f.spacob);
  /*** TODO: we probably should output some <div> with all attributes,
   * all components, and a payload specific display using selector
   * display_object_content_web ...
   ***/
  *pout << "<div class='objcontent_rpscl' id='rpsobc_"
        <<_f.recvob->oid()
        << "'>"
        << std::endl;
  /// display the object title
  *pout << "<span class='objtitle_rpsl' id='rpsobtit_"
        <<_f.recvob->oid()
        << "'>"
        << "⁑" // U+2051 TWO ASTERISKS ALIGNED VERTICALLY
        << std::flush;
  RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA object!display_object_content_web displaying recv="
                << _f.recvob << " classob=" << _f.classob << " obweb=" << _f.obweb);
  rps_web_display_html_for_objref(&_, _f.recvob, _f.obweb, 0);
  *pout << "</span>" << std::endl;
  /// should display the class and space
  *pout << "<span class='objclass_rpscl' id='rpsobcla_"
        <<_f.recvob->oid()
        << "'> ∈&nbsp;";//U+2208 ELEMENT OF
  RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA object!display_object_content_web displaying classob=" << _f.classob << " obweb="
                << _f.obweb << " for recvob=" << _f.recvob);
  rps_web_display_html_for_objref(&_, _f.classob, _f.obweb, 0);
  *pout << "</span>" << std::endl;
  if (!_f.spacob)
    {
      // temporary object without space
      *pout << "<span class='objspace_rpscl' id='rpsobspa_"
            <<_f.recvob->oid()
            << "'>temporary</span>" << std::endl;
    }
  else if (_f.spacob == RPS_ROOT_OB(_8J6vNYtP5E800eCr5q))
    {
      // object inside the initial space
      *pout << "<span class='objspace_rpscl' id='rpsobspa_"
            <<_f.recvob->oid()
            << "'>initial space</span>" << std::endl;
    }
  else
    {
      *pout << "<span class='objspace_rpscl' id='rpsobspa_"
            <<_f.recvob->oid()
            << "'> ＊space&nbsp;"; //U+FF0A FULLWIDTH ASTERISK
      rps_web_display_html_for_objref(&_, _f.spacob, _f.obweb, 0);
      *pout << "</span>" << std::endl;
      *pout << "<br class='objbreak_rpscl' id='rpsobbrtit_"
            <<_f.recvob->oid()
            << "'/>" << std::endl;
      *pout  << "<span class='objspace_rpscl' id='rpsobspa_"
             <<_f.recvob->oid()
             << "'/>" << std::endl;
    }
  //// output attributes:
  {
    unsigned nbattr = _f.setattrs.as_set()->cardinal();
    RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA recvob=" << _f.recvob
                  << " has " << nbattr << " attributes");
    if (nbattr > 0)
      {
        *pout  << "<ul class='objattr_rpscl' id='rpsobattrs_"
               <<_f.recvob->oid()
               << "'/>" << std::endl;
        for (unsigned atix = 0; atix < nbattr; atix++)
          {
            _f.attrob = _f.setattrs.as_set()->at(atix);
            _f.attrval = _f.recvob->get_attr1(&_,  _f.attrob);
            *pout << "<li class='objatentry_rpscl'>";
            rps_web_display_html_for_objref(&_, _f.attrob, _f.obweb, 0);
            *pout << " ↦ "; // U+21A6 RIGHTWARDS ARROW FROM BAR
            rps_web_display_html_for_value(&_, _f.attrval, _f.obweb, 1);
            *pout << "</li>" << std::endl;
            _f.attrob = nullptr;
            _f.attrval = nullptr;
          }
        *pout  << "</ul>" << std::endl;
      }
  }
  //// output components:
  {
    unsigned nbcomp = _f.recvob->nb_components(&_);
    RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA recvob=" << _f.recvob
                  << " has " << nbcomp << " components");
    if (nbcomp > 0)
      {
        *pout  << "<ol class='objcomp_rpscl' start='0' id='rpsobcomp_"
               <<_f.recvob->oid()
               << "'/>" << std::endl;
        for (unsigned compix = 0; compix < nbcomp; compix++)
          {
            _f.curcomp =  _f.recvob->component_at(&_, compix);
            RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA recvob=" << _f.recvob
                          << " comp#" << compix << "= " << _f.curcomp);
            *pout << "<li class='objcomp_rpscl'>";
            rps_web_display_html_for_value(&_, _f.curcomp, _f.obweb, 1);
            *pout << "</li>" << std::endl;
            _f.curcomp = nullptr;
          }
        *pout  << "</ol>" << std::endl;
      }
  }
  auto payl = _f.obweb->get_payload();
  if (payl)
    {
      /// should display the payload....
      RPS_DEBUG_LOG(WEB, "rpsapply_5nSiRIxoYQp00MSnYA recvob=" << _f.recvob
                    << " obweb=" << _f.obweb
                    << " before displaying payload");
      (void) Rps_ObjectValue(_f.obweb).send2(&_,
                                             _f.obweb,
                                             _f.obsel_display_object_payload_web,
                                             Rps_Value((intptr_t)1));
    };
  *pout << "</div>" << std::endl;
  RPS_DEBUG_LOG(WEB, "end rpsapply_5nSiRIxoYQp00MSnYA recvob=" << _f.recvob
                << " obweb=" << _f.obweb
                << "  !method object!display_object_content_web"
                << std::endl);
  return Rps_TwoValues{_f.obweb};
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
                 Rps_ObjectRef obelem; //
                 Rps_Value depthv; //
                 Rps_Value resmainv; //
                 Rps_Value resxtrav; //
                 Rps_SetValue setselv; //
                 Rps_SetValue setattrv; //
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
  constexpr int max_depth = 5; // FIXME, should be improved
  RPS_DEBUG_LOG(WEB, "rpsapply_8lKdW7lgcHV00WUOiT start !method class/display_object_payload_web @!@° obclass="
                << _f.obclass << ", obweb=" << _f.obweb
                << " of class:" << Rps_Value(_f.obweb).compute_class(&_)
                << ", depthi=" << depthi << std::endl
                << RPS_FULL_BACKTRACE_HERE(2,
                    "?£!? rpsapply_8lKdW7lgcHV00WUOiT !method class/display_object_payload_web")
                << std::endl
               );
  std::ostream* pout = rps_web_ostream_ptr(&_, _f.obweb, RPS_CHECK_OSTREAM_PTR);
  RPS_ASSERT(pout);
  auto paylcla = _f.obclass->get_classinfo_payload();
  RPS_ASSERT(paylcla);
  _f.obsuper = paylcla->superclass();
  _f.setattrv = Rps_SetValue(paylcla->attributes_set());
  _f.setselv = paylcla->compute_set_of_own_method_selectors(&_);
  RPS_DEBUG_LOG(WEB, "rpsapply_8lKdW7lgcHV00WUOiT obclass=" << _f.obclass
                << " setattrv=" << _f.setattrv
                << " setselv=" << _f.setselv);
  *pout << "<div class='classinfo_rpscl' rps_obid='" <<  _f.obweb->oid()
        << "'>" << std::endl;
  *pout << "<span class='decorval_rpscl'>super:</span>&nbsp;";
  rps_web_display_html_for_objref(&_, _f.obsuper,
                                  _f.obweb,
                                  depthi+1);
  *pout << "<br class='decor_rpscl'/>" << std::endl;
  if (depthi<max_depth)
    {
      /// display set of attributes
      unsigned cardat = _f.setattrv.as_set()->cardinal();
      RPS_DEBUG_LOG(WEB, "rpsapply_8lKdW7lgcHV00WUOiT obclass=" << _f.obclass << " cardat="<< cardat);
      if (cardat>0)
        {
          *pout << "<div class='classattr_rpscl' rps_obid='" <<  _f.obweb->oid()
                << "'>" << std::endl;
          *pout << "<span class='classattrcardinal_rpscl' rps_obid='" <<  _f.obweb->oid()
                << "'>" << cardat << " attributes"
                << "</span>" << std::endl;
          *pout << "<ul class='classattrelem_rpscl' rps_obid='" <<  _f.obweb->oid()
                << "'>" << std::endl;
          for (unsigned atix=0; atix<cardat; atix++)
            {
              _f.obelem = _f.setattrv.as_set()->at(atix);
              RPS_DEBUG_LOG(WEB, "rpsapply_8lKdW7lgcHV00WUOiT obclass=" << _f.obclass
                            << " attribute atix#" << atix
                            << " obelem=" << _f.obelem);
              RPS_ASSERT(_f.obelem);
              *pout << "<li class='classattr_rpscl' rps_obid='" <<  _f.obweb->oid()
                    << "'>" << std::endl;
              rps_web_display_html_for_objref(&_, _f.obelem, _f.obweb, depthi+1);
              *pout << "</li>" << std::endl;
              _f.obelem = nullptr;
            }
          *pout << "</ul>" << std::endl; // end classattrelem_rpscl
          *pout << "</div>" << std::endl; // end classattr_rpscl
        };
      /// display method selectors
      unsigned nbsel =  _f.setselv.as_set()->cardinal();
      RPS_DEBUG_LOG(WEB, "rpsapply_8lKdW7lgcHV00WUOiT obclass=" << _f.obclass << " nbsel="<< nbsel);
      if (nbsel>0)
        {
          *pout << "<div class='classmethsel_rpscl' rps_obid='" <<  _f.obweb->oid()
                << "'>" << std::endl;
          *pout << "<span class='classmethnb_rpscl' rps_obid='" <<  _f.obweb->oid()
                << "'>" << nbsel << " method selectors"
                << "</span>" << std::endl;
          *pout << "<dl class='classmethods_rpscl'  rps_obid='" <<  _f.obweb->oid()
                << "'>" << std::endl;
          for (unsigned selix=0; selix < nbsel; selix++)
            {
              _f.obcursel =  _f.setselv.as_set()->at(selix);
              _f.curmethclos = paylcla->get_own_method(_f.obcursel);
              RPS_DEBUG_LOG(WEB, "rpsapply_8lKdW7lgcHV00WUOiT obclass=" << _f.obclass
                            << " selix#" << selix << " obcursel=" << _f.obcursel
                            << " curmethclos=" << _f.curmethclos);
              *pout << "<dt class='methodselob_rpscl'  rps_obid='" <<  _f.obweb->oid()
                    << "'>" << std::endl;
              rps_web_display_html_for_objref(&_, _f.obcursel, _f.obweb, depthi+2);
              *pout << "</dt>" << std::endl;
              *pout << "<dd class='methodselclo_rpscl'> rps_obid='" <<  _f.obweb->oid()
                    << "'>" << std::endl;
              rps_web_display_html_for_value(&_,
                                             _f.curmethclos,
                                             _f.obweb,
                                             depthi+1);
              *pout << "</dd>" << std::endl;
              _f.obcursel = nullptr;
              _f.curmethclos = nullptr;
            }
          *pout << "</dl>" << std::endl;  // end classmethods_rpscl
          *pout << "</div>" << std::endl; // end classmethsel_rpscl
        };
    }
  *pout << "</div>" << std::endl; // end classinfo_rpscl
  RPS_DEBUG_LOG(WEB, "end rpsapply_8lKdW7lgcHV00WUOiT !method class/display_object_payload_web obclass="
                << _f.obclass << ", obweb=" << _f.obweb
                << " depthi=" << depthi
               );
  return Rps_TwoValues{_f.obweb};
} // end of rpsapply_8lKdW7lgcHV00WUOiT


///////////////////////////////////////////////////////// end of file webdisplay_rps.cc
