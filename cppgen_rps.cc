/****************************************************************
 * file cppgen_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for C++ code generation.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2023 - 2024 The Reflective Persistent System Team
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
 *
 * Notice:
 *    See also companion file lightgen_rps.cc for GNU lightning
 *    code generation.
 ******************************************************************************/

#include "refpersys.hh"



extern "C" const char rps_cppgen_gitid[];
const char rps_cppgen_gitid[]= RPS_GITID;

extern "C" const char rps_cppgen_date[];
const char rps_cppgen_date[]= __DATE__;


extern "C" const char rps_cppgen_shortgitid[];
const char rps_cppgen_shortgitid[]= RPS_SHORTGITID;


class Rps_PayloadCplusplusGen : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend Rps_PayloadCplusplusGen*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadCplusplusGen,Rps_ObjectZone*>(Rps_ObjectZone*);
  Rps_PayloadCplusplusGen (Rps_ObjectZone*owner);
  Rps_PayloadCplusplusGen(Rps_ObjectRef obr) :
    Rps_PayloadCplusplusGen(obr?obr.optr():nullptr) {};
  std::ostringstream cppgen_outcod;
  int cppgen_indentation;
  std::string cppgen_path;
  std::set<Rps_ObjectRef> cppgen_includeset;
  virtual ~Rps_PayloadCplusplusGen()
  {
    cppgen_outcod.clear();
  };
protected:
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1) / sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  static constexpr size_t maximal_size = 512*1024;
  void check_size(int lineno=0);
  std::ostringstream& endl_indent(std::ostringstream& out)
  {
    out << std::endl;
    for (int i=0; i<cppgen_indentation; i++) out << ' ';
    check_size();
    return out;
  };
  std::ostringstream& checked_endl(std::ostringstream& out)
  {
    out << std::endl;
    check_size();
    return out;
  };
  void indent_more(void)
  {
    cppgen_indentation++;
  };
  void indent_less(void)
  {
    cppgen_indentation--;
  };
  void clear_indentation(void)
  {
    cppgen_indentation=0;
  };
  void set_indentation(int i)
  {
    cppgen_indentation=(i>0)?i:0;
  };
  void set_file_path(std::string p)
  {
    cppgen_path = p;
  };
  void add_cplusplus_include(Rps_CallFrame*callerframe,  Rps_ObjectRef argcurinclude);
  virtual const std::string payload_type_name(void) const
  {
    return "cplusplusgen";
  };
};

Rps_PayloadCplusplusGen::Rps_PayloadCplusplusGen(Rps_ObjectZone*ob)
  : Rps_Payload(Rps_Type::PaylCplusplusGen,ob), cppgen_outcod(),
    cppgen_indentation(0), cppgen_path()
{
} // end Rps_PayloadCplusplusGen::Rps_PayloadCplusplusGen

void
Rps_PayloadCplusplusGen::check_size(int lineno)
{
  char linbuf[16];
  memset(linbuf, 0, sizeof(linbuf));
  if (lineno > 0) snprintf(linbuf, sizeof(linbuf), ":%d", lineno);
  if (cppgen_outcod.tellp() > maximal_size)
    {
      RPS_WARNOUT("in C++ generator " << owner()
                  << (cppgen_path.empty()?"":" for path ")
                  << cppgen_path
                  << ((lineno>0)?" from " __FILE__ : "")
                  << ((lineno>0)?linbuf:"")
                  << " too big C++ generated code");
      throw std::runtime_error("too big C++ generated code");
    }
} // end check_size

void
Rps_PayloadCplusplusGen::gc_mark(Rps_GarbageCollector&gc) const
{
  for (Rps_ObjectRef obinc: cppgen_includeset)
    gc.mark_obj(obinc);
} // end Rps_PayloadCplusplusGen::gc_mark

void
Rps_PayloadCplusplusGen::dump_scan(Rps_Dumper*) const
{
  return;
};

void
Rps_PayloadCplusplusGen::dump_json_content(Rps_Dumper*, Json::Value&) const
{
  return;
};

void
Rps_PayloadCplusplusGen::add_cplusplus_include(Rps_CallFrame*callerframe,
    Rps_ObjectRef argcurinclude)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obcurinclude;
                 Rps_ObjectRef obincludedep;
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obmodule;
                 Rps_Value vincldep;
                );
  _f.obgenerator = owner();
  _f.obcurinclude = argcurinclude;
  _f.obmodule = _f.obgenerator->get_attr1(&_,
                                          RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC)).as_object(); //"code_module"∈named_attribute;
  RPS_ASSERT(_f.obcurinclude);
  std::lock_guard<std::recursive_mutex> gucurinclude(*_f.obcurinclude->objmtxptr());
  if (!_f.obcurinclude->is_instance_of(RPS_ROOT_OB(_0CQWWIMNvTH01h1bE0))) //cpp_include_file∈class
    {
      RPS_WARNOUT("in C++ generated module " << _f.obmodule
                  << " with generator " << _f.obgenerator
                  << " include " << _f.obcurinclude
                  << " is not a cpp_include_file");
      throw RPS_RUNTIME_ERROR_OUT("rps_generate_cplusplus_code bad include "
                                  << _f.obcurinclude
                                  << "  obmodule=" << _f.obmodule
                                  << " obgenerator=" << _f.obgenerator);
    };
  cppgen_includeset.insert(_f.obcurinclude);
  vincldep = _f.obgenerator->get_attr1(&_,
                                       RPS_ROOT_OB(_658gwjgB3oq02ZBhYJ)); //cxx_dependencies∈symbol
  if (vincldep.is_object())
    {
      _f.obincludedep = vincldep.to_object();
      if (!cppgen_includeset.contains(_f.obincludedep))
        add_cplusplus_include(&_, _f.obincludedep);
    }
#warning incomplete code Rps_PayloadCplusplusGen::add_cplusplus_include
  else if (vincldep.is_set())
    {
    }
  else if (vincldep.is_tuple())
    {
    }
} // end Rps_PayloadCplusplusGen::add_cplusplus_include

//// return true on successful C++ code generation
bool
rps_generate_cplusplus_code(Rps_CallFrame*callerframe,
                            Rps_ObjectRef argobmodule,
                            Rps_Value arggenparam)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obmodule;
                 Rps_Value vgenparam;
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obincludeset;
                 Rps_ObjectRef obcurinclude;
                 Rps_ClosureValue vclos;
                 Rps_Value vinclude;
                 Rps_Value vincludeset;
                 Rps_Value voldval;
                 Rps_Value vmainres;
                 Rps_Value vxtrares;
                 Rps_Value vtype;
                );
  std::set<Rps_ObjectRef> includeset;
  std::vector<Rps_ObjectRef> includevect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    RPS_ASSERT(gc != nullptr);
    for (auto incob: includeset)
      gc->mark_obj(incob);
    for (auto incob: includevect)
      gc->mark_obj(incob);
  });
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(argobmodule);
  _f.obmodule = argobmodule;
  _f.vgenparam = arggenparam;
  std::lock_guard<std::recursive_mutex> gumodule(*_f.obmodule->objmtxptr());
  _f.obgenerator =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_2yzD3HZ6VQc038ekBU)//midend_cplusplus_code_generator∈class
                              );
  _f.obgenerator->put_attr(RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC), //"code_module"∈named_attribute
                           _f.obmodule);
  _f.obgenerator->put_new_plain_payload<Rps_PayloadCplusplusGen>();
  /**
     The "include" attribute of the module describes how would the
     #include-s in the generated C++ file be obtained and generated.

     We need to document more.... **/
  _f.vinclude =
    _f.obmodule->get_attr1(&_,
                           RPS_ROOT_OB(_0XF2N1YQ87p02GXXir)); //"include"∈named_attribute
  RPS_DEBUG_LOG(CODEGEN,"rps_generate_cplusplus_code start obmodule=" << _f.obmodule
                << " obgenerator=" << _f.obgenerator << " vinclude=" << _f.vinclude);
  /**
     TODO complete here:

     if vinclude is a set or sequence, use it;
     if it is a closure, apply it to the module and the generator

     WHAT ELSE

     How to deal with the (varying) set of types, of C++ routines, of
     C++ struct-s or class-es

  **/
  if (_f.vinclude.is_closure())
    {
      _f.voldval = _f.vinclude;
      _f.vclos = Rps_ClosureValue(_f.vinclude);
      Rps_TwoValues tv = //
        _f.vclos.apply2(&_, _f.obmodule, _f.obgenerator);
      _f.vinclude = tv.main();
      _f.vxtrares = tv.xtra();
      RPS_DEBUG_LOG(CODEGEN,
                    "rps_generate_cplusplus_code computed include "
                    << _f.vinclude << " with closure=" << _f.voldval
                    << " obmodule=" << _f.obmodule
                    << " obgenerator=" << _f.obgenerator);
    }
  else
    RPS_DEBUG_LOG(CODEGEN,
                  "rps_generate_cplusplus_code plain include "
                  << _f.vinclude << " obmodule=" << _f.obmodule
                  << " obgenerator=" << _f.obgenerator);
  if (_f.vinclude.is_set())
    {
      _f.vincludeset = _f.vinclude;
      unsigned cardinclset = _f.vinclude.as_set()->cardinal();
      /// TODO: we need to sort the set of includes.  Perhaps using
      /// some new constant attributes, maybe include_priority and
      /// cxx_include_dependencies
      for (int nix=0; nix<(int)cardinclset; nix++)
        {
          _f.obcurinclude = _f.vincludeset.as_set()->at(nix);
          RPS_ASSERT(_f.obcurinclude);
          _f.obgenerator->get_dynamic_payload<Rps_PayloadCplusplusGen>()->add_cplusplus_include(&_,_f.obcurinclude);
        }
    }
  else if (_f.vinclude.is_tuple())
    {
      unsigned lenincltup = _f.vinclude.as_tuple()->size();
      for (int nix=0; nix<(int)lenincltup; nix++)
        {
          _f.obcurinclude = _f.vinclude.as_tuple()->at(nix);
          RPS_ASSERT(_f.obcurinclude);
          _f.obgenerator->get_dynamic_payload<Rps_PayloadCplusplusGen>()->add_cplusplus_include(&_,_f.obcurinclude);
        }
    }
  else
    {
      RPS_WARNOUT("in C++ generated module " << _f.obmodule
                  << " with generator " << _f.obgenerator
                  << " unexpected include " << _f.vinclude);
      throw RPS_RUNTIME_ERROR_OUT("in rps_generate_cplusplus_code obmodule="
                                  << _f.obmodule << " obgenerator=" << _f.obgenerator
                                  << "unexpected include=" << _f.vinclude);
    }
#warning missing code in rps_generate_cplusplus_code
} // end rps_generate_cplusplus_code


#warning incomplete cppgen_rps.cc file
