/****************************************************************
 * file output_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It holds the Qt5 code related to the Qt5 output widget
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

#include "refpersys.hh"
#include "qthead_qrps.hh"


extern "C" const char rps_output_gitid[];
const char rps_output_gitid[]= RPS_GITID;

extern "C" const char rps_output_date[];
const char rps_output_date[]= __DATE__;


//////////////////////////////////////////////////////////////// RpsQOutputTextEdit
RpsQOutputTextEdit::RpsQOutputTextEdit(QWidget*parent)
  : QTextEdit(parent),
    outptxt_objref()
{
  setDocumentTitle("output");
} // end RpsQOutputTextEdit::RpsQOutputTextEdit

RpsQOutputTextEdit::~RpsQOutputTextEdit()
{
} // end RpsQOutputTextEdit::~RpsQOutputTextEdit


void
RpsQOutputTextEdit::create_outpedit_object(Rps_CallFrame*callerframe)
{
  RPS_LOCALFRAME(nullptr /*no descr*/,
                 callerframe,
                 Rps_ObjectRef obtxed;
                );
  RPS_ASSERT(!outptxt_objref);
  _.obtxed =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_1NWEOIzo3WU03mE42Q) /*rps_output_textedit class*/);
  auto paylt = _.obtxed->put_new_plain_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  paylt->set_qtptr(this);
  outptxt_objref = _.obtxed;
} // end RpsQOutputTextEdit::create_outpedit_object


//////////////////////////////////////////////////////////////// RpsQOutputTextDocument
RpsQOutputTextDocument::RpsQOutputTextDocument(RpsQWindow*parent)
  : QTextDocument(parent)
{
#warning incomplete RpsQOutputTextDocument
} // end RpsQOutputTextDocument::RpsQOutputTextDocument

RpsQOutputTextDocument::~RpsQOutputTextDocument()
{
} // end RpsQOutputTextEdit::~RpsQOutputTextEdit




////////////////////////////////////////////////////////////////
// C++ closure for _0TwK4TkhEGZ03oTa5m
//!display Val0 in Ob1Win at depth Val2Depth
extern "C" rps_applyingfun_t rpsapply_0TwK4TkhEGZ03oTa5m;
Rps_TwoValues
rpsapply_0TwK4TkhEGZ03oTa5m (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0val, const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth, const Rps_Value arg3_ __attribute__((unused)), ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
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
  ////==== body of _0TwK4TkhEGZ03oTa5m ====
  if (!_.winob1)
    throw RPS_RUNTIME_ERROR_OUT("display value " << _.val0v << " without window object");
  std::lock_guard<std::recursive_mutex> gu(*(_.winob1->objmtxptr()));
  auto winpayl = _.winob1->get_dynamic_payload<Rps_PayloadQt<QObject>>();
  RpsQOutputTextEdit* qouted= nullptr;
  if (!winpayl || !winpayl->qtptr()
      || !(qouted=qobject_cast<RpsQOutputTextEdit*>(winpayl->qtptr())))
    throw  RPS_RUNTIME_ERROR_OUT("display value " << _.val0v
                                 << " has bad window object " << _.winob1);
  RPS_FATALOUT("unimplemented rpsapply_0TwK4TkhEGZ03oTa5m");
  /// we should display val0 in winob1 at depth2, but how....
#warning unimplemented rpsapply_0TwK4TkhEGZ03oTa5m
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _0TwK4TkhEGZ03oTa5m
} // end of rpsapply_0TwK4TkhEGZ03oTa5m


/// for the display_value_qt RefPerSys methods see
/// https://gitlab.com/bstarynk/refpersys/-/wikis/output-subwindow-of-RefPerSys


////////////////////////////////////////////////////////////////
// C++ closure for _8KJHUldX8GJ03G5OWp
//!method int/display_value_qt
extern "C" rps_applyingfun_t rpsapply_8KJHUldX8GJ03G5OWp;
Rps_TwoValues
rpsapply_8KJHUldX8GJ03G5OWp (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0recv, ///
                             const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth,
                             const Rps_Value arg3_ __attribute__((unused)), ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
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
  QTextCursor qcursout;
  QTextCharFormat qcfmt;
  ////==== body of _8KJHUldX8GJ03G5OWp ====
  _.intv = arg0recv;
  _.obwin = arg1obwin.as_object();
  _.depthv = arg2depth;
  RPS_ASSERT(_.obwin);
  RPS_ASSERT(_.depthv.is_int());
  int depth = (int) (_.depthv.to_int());
  intptr_t curint = _.intv.to_int();
  std::lock_guard<std::recursive_mutex> guobwin (*(_.obwin->objmtxptr()));
  auto qoutwpayl =
    _.obwin->get_dynamic_payload<Rps_PayloadQt<RpsQOutputTextEdit>>();
  RPS_ASSERT(qoutwpayl);
  RpsQOutputTextEdit* qoutxed = qoutwpayl->qtptr();
  RPS_ASSERT(qoutxed);
  /// FIXME: we have not decided yet (in commit 4b2038cfb35dafe) what
  /// cursor should be really used. Temporarily pretend it is the
  /// default one ...
  qcursout = qoutxed->textCursor();
#warning method int/display_value_qt: missing initialization of QTextCharFormat qcfmt for int-s
  //// should compute somehow qcfmt; perhaps by using
  //// RPS_USERPREFERENCE_JSON at startup time...
  RPS_WARNOUT("rpsapply_8KJHUldX8GJ03G5OWp missing initialization of qcfmt "
	      << RPS_BACKTRACE_HERE(2, "rpsapply_8KJHUldX8GJ03G5OWp: qcfmt"));
  char intbuf[32];
  memset (intbuf, 0, sizeof(intbuf));
  snprintf(intbuf, sizeof(intbuf), "%lld", (long long) curint);
  qcursout.insertText(QString(intbuf), qcfmt);
  ///
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _8KJHUldX8GJ03G5OWp
} // end of rpsapply_8KJHUldX8GJ03G5OWp


////////////////////////////////////////////////////////////////

// C++ closure for _2KnFhlj8xW800kpgPt
//!method string/display_value_qt
extern "C" rps_applyingfun_t rpsapply_2KnFhlj8xW800kpgPt;
Rps_TwoValues
rpsapply_2KnFhlj8xW800kpgPt (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0, const Rps_Value arg1, ///
                             const Rps_Value arg2, const Rps_Value arg3, ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_2KnFhlj8xW800kpgPt,
                 callerframe, //
                 //Rps_Value arg0v;
                 //Rps_Value arg1v;
                 //Rps_Value arg2v;
                 //Rps_Value arg3v;
                 //Rps_ObjectRef obr;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  // _.arg0v = arg0;
  // _.arg1v = arg1;
  // _.arg2v = arg2;
  // _.arg3v = arg3;
  ////==== body of _2KnFhlj8xW800kpgPt ====
  ;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _2KnFhlj8xW800kpgPt
} // end of rpsapply_2KnFhlj8xW800kpgPt


////////////////////////////////////////////////////////////////
// C++ closure for _7oa7eIzzcxv03TmmZH
//!method double/display_value_qt
extern "C" rps_applyingfun_t rpsapply_7oa7eIzzcxv03TmmZH;
Rps_TwoValues
rpsapply_7oa7eIzzcxv03TmmZH (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0, const Rps_Value arg1, ///
                             const Rps_Value arg2, const Rps_Value arg3, ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_7oa7eIzzcxv03TmmZH,
                 callerframe, //
                 //Rps_Value arg0v;
                 //Rps_Value arg1v;
                 //Rps_Value arg2v;
                 //Rps_Value arg3v;
                 //Rps_ObjectRef obr;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  // _.arg0v = arg0;
  // _.arg1v = arg1;
  // _.arg2v = arg2;
  // _.arg3v = arg3;
  ////==== body of _7oa7eIzzcxv03TmmZH ====
  ;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _7oa7eIzzcxv03TmmZH
} // end of rpsapply_7oa7eIzzcxv03TmmZH




// C++ closure for _33DFyPOJxbF015ZYoi
//!method tuple/display_value_qt
extern "C" rps_applyingfun_t rpsapply_33DFyPOJxbF015ZYoi;
Rps_TwoValues
rpsapply_33DFyPOJxbF015ZYoi (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0, const Rps_Value arg1, ///
                             const Rps_Value arg2, const Rps_Value arg3, ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_33DFyPOJxbF015ZYoi,
                 callerframe, //
                 //Rps_Value arg0v;
                 //Rps_Value arg1v;
                 //Rps_Value arg2v;
                 //Rps_Value arg3v;
                 //Rps_ObjectRef obr;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  // _.arg0v = arg0;
  // _.arg1v = arg1;
  // _.arg2v = arg2;
  // _.arg3v = arg3;
  ////==== body of _33DFyPOJxbF015ZYoi ====
  ;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _33DFyPOJxbF015ZYoi
} // end of rpsapply_33DFyPOJxbF015ZYoi


// C++ closure for _1568ZHTl0Pa00461I2
//!method set/display_value_qt
extern "C" rps_applyingfun_t rpsapply_1568ZHTl0Pa00461I2;
Rps_TwoValues
rpsapply_1568ZHTl0Pa00461I2 (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0, const Rps_Value arg1, ///
                             const Rps_Value arg2, const Rps_Value arg3, ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_1568ZHTl0Pa00461I2,
                 callerframe, //
                 //Rps_Value arg0v;
                 //Rps_Value arg1v;
                 //Rps_Value arg2v;
                 //Rps_Value arg3v;
                 //Rps_ObjectRef obr;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  // _.arg0v = arg0;
  // _.arg1v = arg1;
  // _.arg2v = arg2;
  // _.arg3v = arg3;
  ////==== body of _1568ZHTl0Pa00461I2 ====
  ;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _1568ZHTl0Pa00461I2
} // end of rpsapply_1568ZHTl0Pa00461I2


// C++ closure for _18DO93843oX02UWzq6
//!method object/display_value_qt
extern "C" rps_applyingfun_t rpsapply_18DO93843oX02UWzq6;
Rps_TwoValues
rpsapply_18DO93843oX02UWzq6 (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0, const Rps_Value arg1, ///
                             const Rps_Value arg2, const Rps_Value arg3, ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_18DO93843oX02UWzq6,
                 callerframe, //
                 //Rps_Value arg0v;
                 //Rps_Value arg1v;
                 //Rps_Value arg2v;
                 //Rps_Value arg3v;
                 //Rps_ObjectRef obr;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  // _.arg0v = arg0;
  // _.arg1v = arg1;
  // _.arg2v = arg2;
  // _.arg3v = arg3;
  ////==== body of _18DO93843oX02UWzq6 ====
  ;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _18DO93843oX02UWzq6
} // end of rpsapply_18DO93843oX02UWzq6


// C++ closure for _52zVxP3mTue034OWsD
//!method qtptr/display_value_qt
extern "C" rps_applyingfun_t rpsapply_52zVxP3mTue034OWsD;
Rps_TwoValues
rpsapply_52zVxP3mTue034OWsD (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0, const Rps_Value arg1, ///
                             const Rps_Value arg2, const Rps_Value arg3, ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_52zVxP3mTue034OWsD,
                 callerframe, //
                 //Rps_Value arg0v;
                 //Rps_Value arg1v;
                 //Rps_Value arg2v;
                 //Rps_Value arg3v;
                 //Rps_ObjectRef obr;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  // _.arg0v = arg0;
  // _.arg1v = arg1;
  // _.arg2v = arg2;
  // _.arg3v = arg3;
  ////==== body of _52zVxP3mTue034OWsD ====
  ;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _52zVxP3mTue034OWsD
} // end of rpsapply_52zVxP3mTue034OWsD


// C++ closure for _42cCN1FRQSS03bzbTz
//!method json/display_value_qt
extern "C" rps_applyingfun_t rpsapply_42cCN1FRQSS03bzbTz;
Rps_TwoValues
rpsapply_42cCN1FRQSS03bzbTz (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0, const Rps_Value arg1, ///
                             const Rps_Value arg2, const Rps_Value arg3, ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_42cCN1FRQSS03bzbTz,
                 callerframe, //
                 //Rps_Value arg0v;
                 //Rps_Value arg1v;
                 //Rps_Value arg2v;
                 //Rps_Value arg3v;
                 //Rps_ObjectRef obr;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  // _.arg0v = arg0;
  // _.arg1v = arg1;
  // _.arg2v = arg2;
  // _.arg3v = arg3;
  ////==== body of _42cCN1FRQSS03bzbTz ====
  ;
  RPS_LOCALRETURNTWO(_.resmainv, _.resxtrav); // result of _42cCN1FRQSS03bzbTz
} // end of rpsapply_42cCN1FRQSS03bzbTz



/************************************************************* end of file output_qrps.cc ****/
