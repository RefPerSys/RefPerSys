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


/************************************************************* end of file output_qrps.cc ****/
