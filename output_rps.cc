/****************************************************************
 * file output_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to pretty output of values and objects
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2024 - 2025 The Reflective Persistent System Team
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

extern "C" const char rps_output_gitid[];
const char rps_output_gitid[]= RPS_GITID;

extern "C" const char rps_output_date[];
const char rps_output_date[]= __DATE__;

extern "C" const char rps_output_shortgitid[];
const char rps_output_shortgitid[]= RPS_SHORTGITID;

extern "C" const char rps_output_timestamp[];
const char rps_output_timestamp[]= __TIMESTAMP__;

////////////////


void
Rps_OutputValue::do_output(std::ostream& out) const
{
  /// output value _out_val at _out_depth to out, using Rps_ZoneValue::val_output....
  if (_out_depth > _out_maxdepth)
    {
      out << "?";
      return;
    };
  if (_out_val.is_empty())
    {
      out << "*nil*";
      return;
    }
  else if (_out_val.is_int())
    {
      out << _out_val.as_int();
      return;
    };
  const Rps_ZoneValue* outzv = _out_val.as_ptr();
  RPS_ASSERT(outzv);
  outzv->val_output(out, _out_depth, _out_maxdepth);
} // end Rps_OutputValue::do_output


void
Rps_Object_Display::output_routine_addr(std::ostream&out, void*funaddr) const
{
  if (funaddr == nullptr)
    out << "⏚"; //U+23DA EARTH GROUND
  else if (funaddr == RPS_EMPTYSLOT) // should not happen....
    out << "⦱"; //U+29B1 EMPTY SET WITH OVERBAR
  else
    {
      Dl_info adinf;
      memset ((void*)&adinf, 0, sizeof(adinf));
      if (dladdr(funaddr, &adinf))
        {
          if (funaddr==adinf.dli_saddr)
            {
              out << "&" << adinf.dli_sname;
              const char*demangled = nullptr;
              if (adinf.dli_sname[0]=='_' && adinf.dli_sname[1])   // mangled C++ name
                {
                  int status = -1;
                  demangled  = abi::__cxa_demangle(adinf.dli_sname, nullptr, 0, &status);
                  if (demangled && status == 0 && demangled[0])
                    out << "≡" // U+2261 IDENTICAL TO
                        << demangled;
                  free((void*)demangled);
                }
              out << "=" << funaddr;
            }
          else
            {
              size_t delta=(const char*)funaddr - (const char*)adinf.dli_saddr;
              out << "&" << adinf.dli_sname << "+" << delta << "=" << funaddr;
            }
        }
      else
        out << "?" << funaddr;
      if (adinf.dli_fname) /// shared object name
        out << " in " << adinf.dli_fname;
    };
} // end Rps_Object_Display::output_routine_addr


void
rps_sort_object_vector_for_display(std::vector<Rps_ObjectRef>&vectobr)
{
  std::sort(vectobr.begin(), vectobr.end(),
            [](Rps_ObjectRef leftob, Rps_ObjectRef rightob)
  {
    return Rps_ObjectRef::compare_for_display
           (leftob,rightob)<0;
  });
} // end rps_sort_object_vector_for_display


//// called in practice by RPS_OBJECT_DISPLAY macro
void
Rps_Object_Display::output_display(std::ostream&out) const
{
  char obidbuf[32];
  memset (obidbuf, 0, sizeof(obidbuf));
#warning incomplete Rps_Object_Display::output_display should be moved to objects_rps.cc
  if (!_dispfile)
    return;
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  /*** FIXME:
   *
   * in practice we may need to define a special debugging output
   * stream, since in commit 31e7ab2efcce973 (may 2025) the ontty
   * above is always false because RPS_DEBUGNL_LOG_AT and RPS_DEBUG_AT
   * macros are declaring a local std::ostringstream...
   ***/
  RPS_POSSIBLE_BREAKPOINT();
  if (!_dispobref)
    {
      out << BOLD_esc << "__" << NORM_esc
          << " (*" << _dispfile << ":" << _displine << "*)" << std::endl;
      return;
    };
  /// We lock the displayed object to avoid other threads modifying it
  /// during the display.
  std::lock_guard<std::recursive_mutex> gudispob(*_dispobref->objmtxptr());
  _dispobref->oid().to_cbuf24(obidbuf);
  out  << std::endl
       << BOLD_esc
       <<  _dispobref << "::{¤¤ object "
       << NORM_esc
       << std::endl << "  of class "
       << _dispobref->get_class()
       << std::endl;
  {
    Rps_ObjectRef obspace =  _dispobref->get_space();
    if (!obspace.is_empty())
      out <<  "¤ in space " << _dispobref->get_space() << std::endl;
    else
      out << BOLD_esc << "¤ temporary" << NORM_esc << " space"
          << std::endl;
  };
  double obmtim = _dispobref->get_mtime();
  {
    char mtimbuf[64];
    memset (mtimbuf, 0, sizeof(mtimbuf));
    rps_strftime_centiseconds(mtimbuf, sizeof(mtimbuf),
                              "%Y, %b, %d %H:%M:%S.__ %Z", obmtim);
    out   << BOLD_esc << "** mtime: " << mtimbuf
          << "   *hash:" << _dispobref->val_hash()
          << NORM_esc
          << std::endl;
  };
  //// °°°°°°°°°°° display function pointers .....
  rps_magicgetterfun_t* getfun = _dispobref->magic_getter_function();
  if (getfun)
    {
      out << BOLD_esc << "⊚ magic attribute getter function "
          << NORM_esc;
      output_routine_addr(out, reinterpret_cast<void*>(getfun));
    }
  rps_applyingfun_t*applfun = _dispobref->applying_function();
  unsigned nbphysattr = 0;
  if (applfun)
    {
      out << BOLD_esc << "⊚ applying function "
          << NORM_esc;
      output_routine_addr(out, reinterpret_cast<void*>(applfun));
    }
  //// °°°°°°°°°°° display physical attributes
  Rps_Value setphysattr = _dispobref->set_of_physical_attributes();
  if (setphysattr.is_empty()) {
    out << BOLD_esc
        << "** no physical attributes **"
        << NORM_esc << std::endl;
    nbphysattr = 0;
  }
  else
    {
      RPS_ASSERT(setphysattr.is_set());
      const Rps_SetOb*physattrset = setphysattr.as_set();
      nbphysattr = physattrset->cardinal();
      if (nbphysattr == 1)
        {
          const Rps_ObjectRef thesingleattr = physattrset->at(0);
          RPS_ASSERT(thesingleattr);
          const Rps_Value thesingleval = _dispobref->get_physical_attr(thesingleattr);
          out<< BOLD_esc << "** one physical attribute **"
             << NORM_esc << std::endl;
          out << BOLD_esc << "*"
              << NORM_esc << thesingleattr << ": "
              << Rps_OutputValue(thesingleval, _dispdepth, disp_max_depth)
              << std::endl;
        }
      else if (nbphysattr > 1)
        {
          /// TODO: we need to sort physattrset in displayable order
          /// (alphabetically by name, else by objid), using
          /// Rps_ObjectRef::compare_for_display
          out<< BOLD_esc << "** "
             << nbphysattr << " physical attributes **[<"
             << NORM_esc << std::endl;
          std::vector<Rps_ObjectRef> attrvect(nbphysattr);
          for (int ix=0; ix<(int)nbphysattr; ix++)
            attrvect[ix] = physattrset->at(ix);
          rps_sort_object_vector_for_display(attrvect);
          for (int ix=0; ix<(int)nbphysattr; ix++)
            {
              const Rps_ObjectRef curattr = attrvect[ix];
              const Rps_Value curval =  _dispobref->get_physical_attr(curattr);
              out << " " << BOLD_esc << "*"
                  << NORM_esc << curattr << ": "
                  << Rps_OutputValue(curval, _dispdepth, disp_max_depth)
                  << std::endl;
            }
          out << BOLD_esc << ">]" << NORM_esc << std::endl;
        };
    };
  //// °°°°°°°°°°° display physical components
  unsigned nbphyscomp = _dispobref->nb_physical_components();
  if (nbphyscomp == 0)
    {
      out << BOLD_esc << "* no physical components *" << NORM_esc << std::endl;
    }
  else if (nbphyscomp == 1)
    {
      out << BOLD_esc << "* one physical component *" << NORM_esc << std::endl;
    }
  else
    {
      out << BOLD_esc << "* " << nbphyscomp << " physical components *"
          << NORM_esc << std::endl;
    }
  const std::vector<Rps_Value> vectcomp =
    _dispobref->vector_physical_components();
  for (unsigned ix=0; ix<nbphyscomp; ix++)
    {
      out << BOLD_esc << " [" << ix << "]" << NORM_esc << " "
          << Rps_OutputValue(vectcomp[ix], _dispdepth, disp_max_depth)
          << std::endl;
    };
  Rps_Payload*payl = _dispobref->get_payload();
  if (!payl)
    {
      out << BOLD_esc << "* no payload *" << NORM_esc << std::endl;
    }
  else
    {
      out << BOLD_esc << "* " << payl->payload_type_name() << " payload *"
          << NORM_esc << std::endl;
      payl->output_payload(out, _dispdepth, disp_max_depth);
    };
  char oidpref[16];
  memset (oidpref, 0, sizeof(oidpref));
  memcpy (oidpref, obidbuf, sizeof(oidpref)/2);
  out << " " << BOLD_esc << "|-" << oidpref << "¤¤}" << NORM_esc << std::endl;
} // end Rps_Object_Display::output_display



#warning TODO: add or move code related to pretty output here

//// end of file output_rps.cc
