/****************************************************************
 * file lightgen_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for machine code generation using GNU
 *      lightning.  See also https://www.gnu.org/software/lightning/
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2023 - 2025 The Reflective Persistent System Team
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
 * Notice: See also companion file cppgen_rps.cc for C++ code
 *    generation and gccjit_rps.cc for GCC libgccjit based one.
 *
 * See
 *    https://lists.gnu.org/archive/html/lightning/2024-09/msg00010.html
 ************************************************************************/

#include "refpersys.hh"

/// GNU lightning implementation header.
/// See https://www.gnu.org/software/lightning/

/// a GNU lightning library after 2.2.2 (or GNU lightning commit
/// 3b0fff9206a458d7e11db of August 21, 2023) is required.


/// GNU lightning
extern "C" {
#include "lightning.h"
};

extern "C" const char rps_lightgen_gitid[];
const char rps_lightgen_gitid[]= RPS_GITID;

extern "C" const char rps_lightgen_date[];
const char rps_lightgen_date[]= __DATE__;

extern "C" const char rps_lightgen_shortgitid[];
const char rps_lightgen_shortgitid[]= RPS_SHORTGITID;




/// According to www.gnu.org/software/lightning/manual/lightning.html
/// every GNU lightning macro uses the _jit identifier... The type of
/// that identifier is a pointer to the abstract jit_state_t ...
////////////////////////////////////////////////////////////////


extern "C" void rpsldpy_lightning_code_generator(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno);

/// temporary payload for GNU lightning code generation:
class Rps_PayloadLightningCodeGen : public Rps_Payload
{
  friend void rpsldpy_lightning_code_generator(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno);
public:
  typedef long lightnodenum_t;
  friend Rps_PayloadLightningCodeGen*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadLightningCodeGen,Rps_ObjectZone*>(Rps_ObjectZone*);
  virtual ~Rps_PayloadLightningCodeGen();
  jit_state_t* lightg_jist;
  std::map<jit_node*,lightnodenum_t> lightg_nod2num_map;
  std::map<lightnodenum_t,jit_node*> lightg_num2nod_map;
#define _jit this->lightg_jist
#define RPSJITLIGHTPAYLOAD_LOCKGUARD_AT(Lin) \
  std::lock_guard<std::recursive_mutex> gu##Lin(*(this->owner()->objmtxptr()));
#define RPSJITLIGHTPAYLOAD_LOCKGUARD() RPSJITLIGHTPAYLOAD_LOCKGUARD_AT(__LINE__)
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  inline Rps_PayloadLightningCodeGen(Rps_ObjectZone*owner);
  Rps_PayloadLightningCodeGen(Rps_ObjectRef obr) :
    Rps_PayloadLightningCodeGen(obr?obr.optr():nullptr) {};
  virtual const std::string payload_type_name(void) const
  {
    return "lightning_code_generator";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
public:
  void rpsjit_prolog(void)
  {
    RPSJITLIGHTPAYLOAD_LOCKGUARD();
    jit_prolog();
  };
  void rpsjit_epilog(void)
  {
    RPSJITLIGHTPAYLOAD_LOCKGUARD();
    jit_epilog();
  };
  void rpsjit_realize(void)
  {
    RPSJITLIGHTPAYLOAD_LOCKGUARD();
    jit_realize();
  };
  bool rpsjit_is_frozen() const
  {
    RPSJITLIGHTPAYLOAD_LOCKGUARD();
    return lightg_jist == nullptr;
  };
  lightnodenum_t rpsjit_register_node(jit_node_t  *jn)
  {
    lightnodenum_t num=0;
    RPSJITLIGHTPAYLOAD_LOCKGUARD();
    RPS_ASSERT(jn != nullptr);
    size_t nbnod = lightg_nod2num_map.size();
    RPS_ASSERT(nbnod == lightg_num2nod_map.size());
    auto numit = lightg_nod2num_map.find(jn);
    if (numit != lightg_nod2num_map.end())
      {
        num = numit->second;
        RPS_ASSERT(num>0);
        RPS_ASSERT(lightg_num2nod_map.find(num) != lightg_num2nod_map.end());
        return num;
      };
    num = (lightnodenum_t)(nbnod+1);
    lightg_nod2num_map.insert({jn,num});
    lightg_num2nod_map.insert({num,jn});
    return num;
  };// end member function rpsjit_register_node
  jit_node_t* rpsjit_node_of_num(lightnodenum_t num) const
  {
    if (num==0)
      return nullptr;
    jit_node_t* nd=nullptr;
    RPSJITLIGHTPAYLOAD_LOCKGUARD();
    auto numit = lightg_num2nod_map.find(num);
    if (numit != lightg_num2nod_map.end())
      {
        nd = numit->second;
        RPS_ASSERT(lightg_nod2num_map.find(nd) != lightg_nod2num_map.end());
      }
    return nd;
  };            // end rpsjit_node_of_num
  lightnodenum_t rpsjit_num_of_node(jit_node_t* nd) const
  {
    if (nd == nullptr)
      return 0;
    RPSJITLIGHTPAYLOAD_LOCKGUARD();
    lightnodenum_t num=0;;
    auto nodit = lightg_nod2num_map.find(nd);
    if (nodit != lightg_nod2num_map.end())
      {
        num = nodit->second;
        RPS_ASSERT(lightg_num2nod_map.find(num) != lightg_num2nod_map.end());
      }
    return num;
  };
  Rps_ObjectRef make_lightgen_code_object(Rps_CallFrame*callframe, Rps_ObjectRef classarg, Rps_ObjectRef spacearg);
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};        // end class Rps_PayloadLightningCodeGen





////////////////////////////////////////////////////////////////
Rps_PayloadLightningCodeGen::Rps_PayloadLightningCodeGen(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylLightCodeGen,owner), lightg_jist(nullptr),
    lightg_nod2num_map(), lightg_num2nod_map()
{
  lightg_jist = jit_new_state();
  RPSJITLIGHTPAYLOAD_LOCKGUARD();
} // end of Rps_PayloadLightningCodeGen::Rps_PayloadLightningCodeGen

Rps_PayloadLightningCodeGen::~Rps_PayloadLightningCodeGen()
{
  RPSJITLIGHTPAYLOAD_LOCKGUARD();
  for (auto itnod: lightg_nod2num_map)
    {
      jit_node_t*curnod = itnod.first;
      lightnodenum_t curnum= itnod.second;
      RPS_ASSERT(curnod != nullptr);
      RPS_ASSERT(curnum > 0);
      RPS_ASSERT(lightg_num2nod_map.find(curnum) != lightg_num2nod_map.end());
    };
  lightg_nod2num_map.clear();
  lightg_num2nod_map.clear();
  _jit_destroy_state(lightg_jist); /// all nodes get destroyed!
  lightg_jist = nullptr;
} // end destructor Rps_PayloadLightningCodeGen::~Rps_PayloadLightningCodeGen

void
Rps_PayloadLightningCodeGen::gc_mark(Rps_GarbageCollector&gc) const
{
} // end of Rps_PayloadLightningCodeGen::gc_mark

void
Rps_PayloadLightningCodeGen::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du);
  RPS_POSSIBLE_BREAKPOINT();
} // end Rps_PayloadLightningCodeGen::dump_scan

void
Rps_PayloadLightningCodeGen::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  RPS_ASSERT(du);
  RPS_POSSIBLE_BREAKPOINT();
#warning incomplete Rps_PayloadLightningCodeGen::dump_json_content
  RPS_WARNOUT("incomplete Rps_PayloadLightningCodeGen::dump_json_content owner=" << RPS_OBJECT_DISPLAY(owner()));
} // end Rps_PayloadLightningCodeGen::dump_json_content

void
rpsldpy_lightning_code_generator(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  auto payl = obz->put_new_plain_payload<Rps_PayloadLightningCodeGen>();
  RPS_ASSERT(payl);
#warning unimplemented rpsldpy_lightning_code_generator
  RPS_WARNOUT("unimplemented rpsldpy_lightning_code_generator jv=" << jv << std::endl
              << " spacid=" << spacid
              << " lineno=" << lineno << std::endl
	      << " obz=" << RPS_OBJECT_DISPLAY(obz) << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsldpy_lightning_code_generator"));
} // end rpsldpy_lightning_code_generator

void
Rps_PayloadLightningCodeGen::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> guown(*(owner()->objmtxptr()));
  out << BOLD_esc << "* GNU lightning code generator for "
      << lightg_num2nod_map.size() << " nodes*"
      << NORM_esc << std::endl;
} // end Rps_PayloadLightningCodeGen::output_payload

////////////////////////////////////////////////////////////////
//// Returns true on successful code generation
bool
rps_generate_lightning_code(Rps_CallFrame*callerframe,
                            Rps_ObjectRef argobmodule,
                            Rps_Value arggenparam)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(argobmodule);
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obmodule;
                 Rps_ObjectRef obgenerator;
                 Rps_Value genparamv;
                 Rps_Value elemv;
                 Rps_Value mainv;
                 Rps_Value xtrav;

                );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
#warning probably needs more GC-ed data in rps_generate_lightning_code
  });
  _f.obmodule = argobmodule;
  _f.genparamv = arggenparam;
  std::lock_guard<std::recursive_mutex> gumodule(*_f.obmodule->objmtxptr());
  _f.obgenerator =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_6SM7PykipQW01HVClH) //midend_lightning_code_generator∈class
                              );
  std::lock_guard<std::recursive_mutex> gugenerator(*_f.obgenerator->objmtxptr());
  Rps_PayloadLightningCodeGen*paylgen =
    _f.obgenerator->put_new_plain_payload<Rps_PayloadLightningCodeGen>();
  RPS_ASSERT(paylgen != nullptr);
  _f.obgenerator->put_attr(RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC), //"code_module"∈named_attribute
                           _f.obmodule);
  ///TODO: perhaps a better attribute is needed
  _f.obgenerator->put_attr(RPS_ROOT_OB(_5VC4IuJ0dyr01b8lA0), //generate_code∈named_selector
                           _f.genparamv);
  RPS_DEBUG_LOG (CODEGEN, "GNU lightning generator "
                 << RPS_OBJECT_DISPLAY(_f.obgenerator)
                 << std::endl
                 << " for module " << RPS_OBJECT_DISPLAY(_f.obmodule)
                 << std::endl
                 << " generation params " << _f.genparamv << std::endl
                 << " thread=" << rps_current_pthread_name()
                 << " jit_r_num=" << JIT_R_NUM << " jit_v_num=" << JIT_V_NUM);
  /// iterate on every component of the module
  int mix = -1;
  for (mix = 0; (unsigned)mix < _f.obmodule->nb_components(&_); mix++)
    {
      _f.mainv = nullptr;
      _f.xtrav = nullptr;
      _f.elemv = _f.obmodule->component_at(&_, mix);
      if (!_f.elemv)
        continue;
      RPS_DEBUG_LOG (CODEGEN, "GNU lightning generator " << _f.obgenerator
                     << " elemv=" << _f.elemv
                     << " mix#" << mix
                     << " for module " << _f.obmodule
                     << " generation params " << _f.genparamv << std::endl
                     << " thread=" << rps_current_pthread_name());
      if (_f.elemv.is_closure())
        {
          // if the element is a closure, we apply it, and that could
          // add more elements to the module...
          Rps_TwoValues apres =
            Rps_ClosureValue(_f.elemv).apply4(&_,
                                              _f.obgenerator,
                                              _f.genparamv,
                                              _f.obmodule,
                                              Rps_Value::make_tagged_int(mix));
          /// make the GC happy
          _f.mainv = apres.main();
          _f.xtrav = apres.xtra();
          if (!_f.mainv && !_f.xtrav)
            RPS_WARNOUT("rps_generate_lightning_code failed to apply element#" << mix
                        << "=" << _f.elemv
                        << " to generator " << _f.obgenerator
                        << " with parameters " << _f.genparamv
                        << " of module " << _f.obmodule);
          continue; /// the for loop in the module
        }
      else
        {
          Rps_TwoValues snres =
            _f.elemv.send4(&_,
                           //lightning_generate_code∈named_selector:
                           RPS_ROOT_OB(_6GiKCsHJDCi04m74XV),
                           _f.obgenerator,
                           _f.genparamv,
                           _f.obmodule,
                           Rps_Value::make_tagged_int(mix));
          _f.mainv = snres.main();
          _f.xtrav = snres.xtra();
          if (!_f.mainv && !_f.xtrav)
            RPS_WARNOUT("rps_generate_lightning_code failed to send"
                        " lightning_generate_code to element#" << mix
                        << "=" << _f.elemv
                        << " and generator " << _f.obgenerator
                        << " with parameters " << _f.genparamv
                        << " of module " << _f.obmodule);

        };
    };
  RPS_WARNOUT("unimplemented rps_generate_lightning_code obmodule="
              << RPS_OBJECT_DISPLAY(_f.obmodule) << std::endl
              << " obgenerator=" << RPS_OBJECT_DISPLAY(_f.obgenerator) << std::endl
              << " genparamv=" << _f.genparamv << std::endl
              << " thread=" << rps_current_pthread_name()
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_generate_lightning_code/end-incomplete"));
#warning unimplemented rps_generate_lightning_code
  return false;
} // end rps_generate_lightning_code



Rps_ObjectRef
Rps_PayloadLightningCodeGen::make_lightgen_code_object(Rps_CallFrame*callframe, Rps_ObjectRef obclassarg, Rps_ObjectRef obspacearg)
{
  RPS_ASSERT(!callframe || callframe->is_good_call_frame());
  RPS_LOCALFRAME(rpskob_4pI1uwdcVBJ01qlUth, //$lightning_code_object∈class,
                 callframe,
                 Rps_ObjectRef oblightgen;
                 Rps_ObjectRef obclass;
                 Rps_ObjectRef obspace;
                );
  _f.obclass = obclassarg;
  _f.obspace = obspacearg;
  if (!_f.obclass)
    _f.obclass = rpskob_4pI1uwdcVBJ01qlUth; //$lightning_code_object∈class
  else if (!Rps_Value(_f.obclass).is_subclass_of(&_,
           rpskob_4pI1uwdcVBJ01qlUth //$lightning_code_object∈class
                                                ))
    {
      RPS_WARNOUT("make_lightgen_code_object with bad class " << RPS_OBJECT_DISPLAY(_f.obclass)
                  << " from " << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadLightningCodeGen::make_lightgen_code_object"));
      throw std::runtime_error("invalid class for make_lightgen_code_object");
    };
  _f.oblightgen =  Rps_ObjectRef::make_object(&_, _f.obclass, _f.obspace);
  auto paylgen =  _f.oblightgen->put_new_plain_payload<Rps_PayloadLightningCodeGen>();
  RPS_ASSERT(paylgen);
  RPS_DEBUG_LOG (CODEGEN, "make_lightgen_code_object made " << RPS_OBJECT_DISPLAY(_f.oblightgen)
                 << std::endl
                 <<  " from " << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadLightningCodeGen::make_lightgen_code_object"));
#warning probably incomplete Rps_PayloadLightningCodeGen::make_lightgen_code_object
  return _f.oblightgen;
} // end Rps_PayloadLightningCodeGen::make_lightgen_code_object


#warning incomplete lightgen_rps.cc file
