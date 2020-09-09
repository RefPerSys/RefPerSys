/****************************************************************
 * file main_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It implements the agenda mechanism.
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


std::recursive_mutex Rps_Agenda::agenda_mtx_;
std::deque<Rps_ObjectRef> Rps_Agenda::agenda_fifo_[Rps_Agenda::AgPrio__Last];

void
Rps_Agenda::gc_mark(Rps_GarbageCollector&gc)
{
  std::lock_guard<std::recursive_mutex> gu(agenda_mtx_);
  for (int ix=AgPrio_Low; ix<AgPrio__Last; ix++)
    {
      auto& curfifo = agenda_fifo_[ix];
      for (auto it: curfifo)
        {
          Rps_ObjectRef ob = *it;
          if (ob)
            ob->gc_mark(gc);
        }
    }
} // end Rps_Agenda::gc_mark


//// loading of agenda related payload
void
rpsldpy_agenda(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
} // end of rpsldpy_agenda

Rps_PayloadAgenda::~Rps_PayloadAgenda()
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
} // end Rps_PayloadAgenda::~Rps_PayloadAgenda

void
Rps_PayloadAgenda::gc_mark(Rps_GarbageCollector&gc) const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  Rps_Agenda::gc_mark(gc);
} // end Rps_PayloadAgenda::gc_mark

void
Rps_PayloadAgenda::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  RPS_ASSERT (du != nullptr);
} // end Rps_PayloadAgenda::dump_scan

void
Rps_PayloadAgenda::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  RPS_ASSERT (du != nullptr);
} // end Rps_PayloadAgenda::dump_json_content


bool
Rps_PayloadAgenda::is_erasable() const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  return false;
} // end Rps_PayloadAgenda::is_erasable

//// end of file agenda_rps.cc
