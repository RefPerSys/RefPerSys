/****************************************************************
 * file morevalues_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to more values (immutable instances, etc...)
 *      See also values_rps.cc file
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

extern "C" const char rps_morevalues_gitid[];
const char rps_morevalues_gitid[]= RPS_GITID;

extern "C" const char rps_morevalues_date[];
const char rps_morevalues_date[]= __DATE__;



const Rps_SetOb*
Rps_InstanceZone::class_attrset(Rps_ObjectRef obclass)
{
  if (!obclass || obclass.is_empty())
    return nullptr;
  std::lock_guard<std::recursive_mutex> gucla(*(obclass->objmtxptr()));
  auto paylcl = obclass->get_classinfo_payload();
  if (!paylcl)
    return nullptr;
  return paylcl->class_attrset();
} // end Rps_InstanceZone::class_attrset

void
Rps_InstanceZone::val_output(std::ostream& outs, unsigned depth) const
{
#warning unimplemented Rps_InstanceZone::val_output
  RPS_WARN("unimplemented Rps_InstanceZone::val_output");
} // end Rps_InstanceZone::val_output


Rps_ObjectRef
Rps_InstanceZone::compute_class(Rps_CallFrame*callerframe) const
{
  auto k_immutable_instance =
    RPS_ROOT_OB(_6ulDdOP2ZNr001cqVZ) /*=immutable_instance∈class*/;
  RPS_LOCALFRAME(k_immutable_instance,
                 callerframe,
                 Rps_ObjectRef obclass;
                );
  RPS_ASSERT(stored_type() == Rps_Type::Instance);
  _.obclass = get_class();
  RPS_ASSERT(_.obclass->get_classinfo_payload());
  return _.obclass;
} // end Rps_InstanceZone::compute_class



const Rps_SetOb*
Rps_InstanceZone::set_attributes(void) const
{
  Rps_ObjectRef obclass = get_class();
  auto setat = class_attrset(obclass);
  return setat;
} // end  Rps_InstanceZone::set_attributes


Rps_InstanceZone*
Rps_InstanceZone::make_from_attributes_components(Rps_ObjectRef classob,
    const std::vector<Rps_Value>& valvec,
    const std::map<Rps_ObjectRef,Rps_Value>& attrmap)
{
  Rps_InstanceZone*res = nullptr;
  RPS_FATALOUT("unimplemented Rps_InstanceZone::make_from_attributes_components classob=" << classob);
#warning unimplemented Rps_InstanceZone::make_from_attributes_components
} // end Rps_InstanceZone::make_from_attributes_components


/********************************************** end of file morevalues_rps.cc */
