/****************************************************************
 * file command_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It holds the Qt5 code related to the Qt5 command widget
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


extern "C" const char rps_command_gitid[];
const char rps_command_gitid[]= RPS_GITID;

extern "C" const char rps_command_date[];
const char rps_command_date[]= __DATE__;


////////////////////////////////////////////////////////////////
RpsQCommandTextEdit::RpsQCommandTextEdit(QWidget*parent)
  : QTextEdit(parent),
    cmdtxt_objref(),
    cmdtxt_valmap()
{
  setDocumentTitle("command");
} // end RpsQCommandTextEdit::RpsQCommandTextEdit

RpsQCommandTextEdit::~RpsQCommandTextEdit()
{
} // end RpsQCommandTextEdit::~RpsQCommandTextEdit

void
RpsQCommandTextEdit::create_cmdedit_object(Rps_CallFrame*callerframe)
{
  RPS_LOCALFRAME(nullptr /*no descr*/,
                 callerframe,
                 Rps_ObjectRef obcmed;
                );
  RPS_ASSERT(!cmdtxt_objref);
  _.obcmed =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_54CP9eaTmxT00lzbEW) /*rps_command_textedit class*/);
  auto paylt = _.obcmed->put_new_plain_payload<Rps_PayloadQt<RpsQCommandTextEdit>>();
  paylt->set_qtptr(this);
  RPS_DEBUG_LOG(GUI, "RpsQCommandTextEdit::create_cmdedit_object obcmed=" << _.obcmed << " with paylt@" << (void*)paylt);
  cmdtxt_objref = _.obcmed;
} // end RpsQCommandTextEdit::create_cmdedit_object


/************************************ end of file command_qrps.cc ****/
