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
 *      Basile Starynkevitch <basile@starynkevitch.net> (France)
 *      Abhishek Chakravarti <abhishek@taranjali.org> (India)
 *      Nimesh Neema <nimeshneema@gmail.com> (India)
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
 * Notice:
 *    See also companion file lightgen_rps.cc for GNU lightning
 *    code generation and the GCC MELT paper on
 *    https://arxiv.org/abs/1109.0779
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
public:
  struct cppgen_data_st   /// internal data for C++ code generation
  {
    // used in cppgen_datavect below
    Rps_ObjectRef cppg_object;
    Rps_Value cppg_data;
    intptr_t cppg_num;
  };
  static void mark_gc_cppgen_data(Rps_GarbageCollector&gc, struct cppgen_data_st*d, unsigned depth=0);
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend Rps_PayloadCplusplusGen*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadCplusplusGen,Rps_ObjectZone*>(Rps_ObjectZone*);
  Rps_PayloadCplusplusGen (Rps_ObjectZone*owner);
  Rps_PayloadCplusplusGen(Rps_ObjectRef obr) :
    Rps_PayloadCplusplusGen(obr?obr.optr():nullptr) {};
protected:
  std::ostringstream cppgen_outcod;
  int cppgen_indentation;
  std::string cppgen_path;
  std::set<Rps_ObjectRef> cppgen_includeset;
  std::map<Rps_ObjectRef,long> cppgen_includepriomap;
  std::vector<struct cppgen_data_st> cppgen_datavect;
  static constexpr int cppgen_maxdatalen = 1<<18;
  virtual ~Rps_PayloadCplusplusGen()
  {
    cppgen_outcod.clear();
    cppgen_includeset.clear();
    cppgen_datavect.clear();
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
  static constexpr size_t maximal_cpp_code_size = 512*1024; // in bytes of generated code
  void check_size(int lineno=0);
  std::string cplusplus_file_path(void) const
  {
    return cppgen_path;
  };
  const struct cppgen_data_st&checked_nth_const_data(int n) const;
  struct cppgen_data_st& checked_nth_data(int n);
  const struct cppgen_data_st*checked_nth_const_ptr(int n) const
  {
    return &checked_nth_const_data(n);
  };
  struct cppgen_data_st*checked_nth_ptr(int n)
  {
    return &checked_nth_data(n);
  };
  const struct cppgen_data_st*nth_const_ptr(int n) const
  {
    if (n<0)
      n += (int)cppgen_datavect.size();
    if (n<0 || n>=(int)cppgen_datavect.size())
      return nullptr;
    return &cppgen_datavect.at(n);
  };
  struct cppgen_data_st*nth_ptr(int n)
  {
    if (n<0)
      n += (int)cppgen_datavect.size();
    if (n<0 || n>=(int)cppgen_datavect.size())
      return nullptr;
    return &cppgen_datavect.at(n);
  };
  int push_new_data(const struct cppgen_data_st&d);
  int push_new_data(Rps_ObjectRef obj=nullptr,
                    Rps_Value val=nullptr,
                    intptr_t num=0)
  {
    struct cppgen_data_st d
    {
      .cppg_object= obj,
      .cppg_data= val,
      .cppg_num=num
    };
    return push_new_data(d);
  };
  unsigned data_size(void) const
  {
    return (unsigned)cppgen_datavect.size();
  };
  std::string eol_indent(void)
  {
    std::string s="\n";
    for (int i=0; i<cppgen_indentation; i++) s += ' ';
    check_size();
    return s;
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
  long compute_include_priority(Rps_CallFrame*callerframe,  Rps_ObjectRef argcurinclude);
  void add_cplusplus_include(Rps_CallFrame*callerframe,  Rps_ObjectRef argcurinclude);
  void emit_initial_cplusplus_comment(Rps_CallFrame*callerframe, Rps_ObjectRef argmodule);
  void emit_cplusplus_includes(Rps_CallFrame*callerframe, Rps_ObjectRef argmodule);
  void emit_cplusplus_declarations(Rps_CallFrame*callerframe, Rps_ObjectRef argmodule);
  void emit_cplusplus_definitions(Rps_CallFrame*callerframe, Rps_ObjectRef argmodule);
  virtual const std::string payload_type_name(void) const
  {
    return "cplusplusgen";
  };
  void output(std::function<void(std::ostringstream&out)> fun, bool raw=false);
  void raw_output(std::function<void(std::ostringstream&out)> fun)
  {
    output (fun, /*raw:*/true);
  };
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};        // end class  Rps_PayloadCplusplusGen


Rps_PayloadCplusplusGen::Rps_PayloadCplusplusGen(Rps_ObjectZone*ob)
  : Rps_Payload(Rps_Type::PaylCplusplusGen,ob), cppgen_outcod(),
    cppgen_indentation(0), cppgen_path()
{
} // end Rps_PayloadCplusplusGen::Rps_PayloadCplusplusGen


int
Rps_PayloadCplusplusGen::push_new_data(const struct cppgen_data_st&d)
{
  if (RPS_UNLIKELY(data_size() > cppgen_maxdatalen))
    {
      RPS_WARNOUT("too big data size "<< data_size()
                  << " in C++ generator " << RPS_OBJECT_DISPLAY(owner())
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,
                                             "Rps_PayloadCplusplusGen::push_new_data"));
      throw RPS_RUNTIME_ERROR_OUT("too big data size "<< data_size()
                                  << " in C++ generator " << owner());
    };
  int ix= (int)data_size();
  cppgen_datavect.push_back(d);
  return ix;
} // end Rps_PayloadCplusplusGen::push_new_data

const struct Rps_PayloadCplusplusGen::cppgen_data_st&
Rps_PayloadCplusplusGen::checked_nth_const_data(int n) const
{
  int orign = n;
  if (n<0)
    n += (int)cppgen_datavect.size();
  if (RPS_UNLIKELY(n<0 || n>=(int)cppgen_datavect.size()))
    {
      RPS_WARNOUT("out of index  access #" << orign << " of data size "<< data_size()
                  << " in C++ generator " << owner()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,
                                             "Rps_PayloadCplusplusGen::checked_nth_const_data"));
      throw RPS_RUNTIME_ERROR_OUT("out of index data access #" << orign << " of size "<< data_size()
                                  << " in C++ generator " << owner());;
    };
  return cppgen_datavect.at(n);
} // end Rps_PayloadCplusplusGen::checked_nth_const_data

struct Rps_PayloadCplusplusGen::cppgen_data_st&
Rps_PayloadCplusplusGen::checked_nth_data(int n)
{
  int orign = n;
  if (n<0)
    n += (int)cppgen_datavect.size();
  if (RPS_UNLIKELY(n<0 || n>=(int)cppgen_datavect.size()))
    {
      RPS_WARNOUT("out of index  access #" << orign << " of data size "<< data_size()
                  << " in C++ generator " << owner()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,
                                             "Rps_PayloadCplusplusGen::checked_nth_data"));
      throw RPS_RUNTIME_ERROR_OUT("out of data access #" << orign << " of size "<< data_size()
                                  << " in C++ generator " << owner());
    }
  return cppgen_datavect.at(n);
} // end Rps_PayloadCplusplusGen::checked_nth_data

void
Rps_PayloadCplusplusGen::output(std::function<void(std::ostringstream&out)> fun,
                                bool raw)
{
  std::string buf;
  std::ostringstream out(buf);
  fun(out);
  if (raw)
    {
      cppgen_outcod << buf;
    }
  else
    {
      for (char c: buf)
        {
          if (c=='\n')
            cppgen_outcod << eol_indent();
          else
            cppgen_outcod << c;
        };
    }
  cppgen_outcod.flush();
} // end Rps_PayloadCplusplusGen::output

void
Rps_PayloadCplusplusGen::mark_gc_cppgen_data(Rps_GarbageCollector&gc, struct cppgen_data_st*d, unsigned depth)
{
  if (!d)
    return;
  if (d->cppg_object)
    d->cppg_object.gc_mark(gc);
  if (d->cppg_data)
    d->cppg_data.gc_mark(gc, depth);
} // end Rps_PayloadCplusplusGen::mark_gc_cppgen_data

void
Rps_PayloadCplusplusGen::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  RPS_ASSERT(depth <= maxdepth);
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> guown(*(owner()->objmtxptr()));
  out << std::endl << BOLD_esc << "*C++ generator payload with"
      << NORM_esc << " " << data_size() << " data"
      << BOLD_esc << "*" << NORM_esc
      << std::endl;
#warning Rps_PayloadCplusplusGen::output_payload incomplete
  // TODO: when depth is 0 show the generated C++ code buffer
  // when depth is not too big show the internal data
} // end of Rps_PayloadCplusplusGen::output_payload

void
Rps_PayloadCplusplusGen::check_size(int lineno)
{
  char linbuf[16];
  memset(linbuf, 0, sizeof(linbuf));
  if (lineno > 0)
    snprintf(linbuf, sizeof(linbuf), ":%d", lineno);
  if ((size_t)cppgen_outcod.tellp() > (size_t)maximal_cpp_code_size)
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

long
Rps_PayloadCplusplusGen::compute_include_priority(Rps_CallFrame*callerframe,
    Rps_ObjectRef obincl)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obcurinclude;
                 Rps_ObjectRef obincludedep;
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obmodule;
                 Rps_Value priov;
                 Rps_Value dependv;
                );
  intptr_t prionum =0;
  _f.obcurinclude = obincl;
  RPS_ASSERT(_f.obcurinclude);
  std::lock_guard<std::recursive_mutex> gucurinclude(*_f.obcurinclude->objmtxptr());
  _f.obgenerator = owner();
  std::lock_guard<std::recursive_mutex> gugen(*_f.obgenerator->objmtxptr());
  {
    auto it = cppgen_includepriomap.find(_f.obcurinclude);
    if (it != cppgen_includepriomap.end())
      return it->second;
  }
  _f.priov = //
    _f.obcurinclude->get_attr1(&_,
                               RPS_ROOT_OB(_6yJysqz6dYF007I4Y5) //"include_priority"∈named_attribute
                              );
  if (_f.priov.is_int())
    {
      prionum = _f.priov.as_int();
      if (prionum>0)
        {
          cppgen_includepriomap.insert({_f.obcurinclude, prionum});
        }
      else
        {
          cppgen_includepriomap.insert({_f.obcurinclude, 1});
        }
    }
  _f.dependv = //
    _f.obcurinclude->get_attr1(&_,
                               RPS_ROOT_OB(_658gwjgB3oq02ZBhYJ) //cxx_dependencies∈symbol
                              );
  if (_f.dependv.is_set())
    {
      int nbdep = _f.dependv.as_set()->cardinal();
      for (int dix=0; dix<nbdep; dix++)
        {
          _f.obincludedep = _f.dependv.as_set()->at(dix);
          long priodep = compute_include_priority(&_, _f.obincludedep);
          if (priodep>0)
            {
              prionum += priodep;
              cppgen_includepriomap.insert({_f.obcurinclude, prionum});
            }
        }
    }
  else if (_f.dependv.is_tuple())
    {
      int nbdep = _f.dependv.as_tuple()->size();
      for (int dix=0; dix<nbdep; dix++)
        {
          _f.obincludedep = _f.dependv.as_tuple()->at(dix);
          if (!_f.obincludedep)
            continue;
          long priodep = compute_include_priority(&_, _f.obincludedep);
          if (priodep>0)
            {
              prionum += priodep;
              cppgen_includepriomap.insert({_f.obcurinclude, prionum});
            }
        }
    }
  return prionum;
} // end Rps_PayloadCplusplusGen::compute_include_priority

void
Rps_PayloadCplusplusGen::gc_mark(Rps_GarbageCollector&gc) const
{
  for (Rps_ObjectRef obinc: cppgen_includeset)
    gc.mark_obj(obinc);
  if (!cppgen_datavect.empty())
    for (auto it : cppgen_datavect)
      mark_gc_cppgen_data(gc, &it);
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
  _f.vincldep = _f.obgenerator->get_attr1(&_,
                                          RPS_ROOT_OB(_658gwjgB3oq02ZBhYJ)); //cxx_dependencies∈symbol
  if (!_f.vincldep)
    return;
  if (_f.vincldep.is_object())
    {
      _f.obincludedep = _f.vincldep.to_object();
      if (cppgen_includeset.find(_f.obincludedep) != cppgen_includeset.end())
        add_cplusplus_include(&_, _f.obincludedep);
    }
  else if (_f.vincldep.is_set())
    {
      int nbdep = _f.vincldep.as_set()->cardinal();
      for (int nix = 0; nix < nbdep; nix++)
        {
          _f.obincludedep = _f.vincldep.as_set()->at(nix);
          RPS_ASSERT(_f.obincludedep);
          if (cppgen_includeset.find(_f.obincludedep) != cppgen_includeset.end())
            add_cplusplus_include(&_, _f.obincludedep);
        }
    }
  else if (_f.vincldep.is_tuple())
    {
      int nbdep = _f.vincldep.as_tuple()->size();
      for (int nix = 0; nix < nbdep; nix++)
        {
          _f.obincludedep = _f.vincldep.as_tuple()->at(nix);
          if (!_f.obincludedep)
            continue;
          if (cppgen_includeset.find(_f.obincludedep) != cppgen_includeset.end())
            add_cplusplus_include(&_, _f.obincludedep);
        }
    }
  else
    {
      RPS_WARNOUT("in C++ generated module " << _f.obmodule
                  << " with generator " << _f.obgenerator
                  << " include " << _f.obcurinclude
                  << " has unexpected cxx_dependencies " << _f.vincldep);
      throw RPS_RUNTIME_ERROR_OUT("rps_generate_cplusplus_code bad cxx_dependencies "
                                  <<  _f.vincldep
                                  << "  obmodule=" << _f.obmodule
                                  << " obgenerator=" << _f.obgenerator);
    }
} // end Rps_PayloadCplusplusGen::add_cplusplus_include


void
Rps_PayloadCplusplusGen::emit_initial_cplusplus_comment(Rps_ProtoCallFrame*callerframe, Rps_ObjectRef argobmodule)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obmodule;
                 Rps_Value initcppcomv;
                 Rps_ClosureValue closv;
                 Rps_Value mainresv;
                 Rps_Value xtraresv;
                );
  _f.obmodule = argobmodule;
  _f.obgenerator = owner();
  _f.initcppcomv = _f.obmodule->get_attr1(&_,
                                          RPS_ROOT_OB(_6QhoB1m97HC03kkKTa)  //"initial_cpp_comment"∈named_attribute
                                         );
  if (_f.initcppcomv.is_closure())
    {
      _f.closv = Rps_ClosureValue(_f.initcppcomv);
      check_size(__LINE__);
      RPS_DEBUG_LOG(CODEGEN,
                    "Rps_PayloadCplusplusGen::emit_initial_cplusplus_comment closv=" << _f.closv << " obmodule=" << _f.obmodule
                    << " obgenerator=" << _f.obgenerator);
      Rps_TwoValues tv = //
        _f.closv.apply2(&_, _f.obmodule, _f.obgenerator);
      _f.mainresv = tv.main();
      _f.xtraresv = tv.xtra();
      RPS_DEBUG_LOG(CODEGEN,
                    "Rps_PayloadCplusplusGen::emit_initial_cplusplus_comment applied closv=" << _f.closv
                    << std::endl
                    << "to module=" << _f.obmodule
                    << " obgenerator=" << _f.obgenerator
                    << " mainresv=" << _f.mainresv
                    << " xtraresv=" << _f.xtraresv);
      check_size(__LINE__);
    }
  else if (_f.initcppcomv.is_string())
    {
      check_size(__LINE__);
      std::string comstr = _f.initcppcomv.to_cppstring();
      cppgen_outcod << "//@" ;
      for (char c: comstr)
        {
          if (c=='\n'||c=='\r'||c=='\f'||c=='\v'||c==(char)0)
            cppgen_outcod << eol_indent() << "//@";
          else
            cppgen_outcod << c;
        };
      cppgen_outcod << eol_indent() << std::flush;
    }
  else
    {
      if (!cppgen_path.empty())
        cppgen_outcod << "//!! generated " << cppgen_path
                      << " from " <<  _f.obmodule
                      << " by " << _f.obgenerator
                      << eol_indent() << std::flush;
      else
        cppgen_outcod << "//-- generated from " <<  _f.obmodule
                      << " by " << _f.obgenerator
                      << eol_indent() << std::flush;
    }
} // end Rps_PayloadCplusplusGen::emit_initial_cplusplus_comment

void
Rps_PayloadCplusplusGen::emit_cplusplus_includes(Rps_ProtoCallFrame*callerframe, Rps_ObjectRef argobmodule)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obmodule;
                 Rps_ObjectRef obcurinclude;
                 Rps_Value vinclude;
                 Rps_Value vincludeset;
                 Rps_Value voldval;
                 Rps_StringValue vstrpath;
                 Rps_ClosureValue vclos;
                 Rps_Value vxtrares;
                 Rps_Value vmain;
                );
  std::vector<cppgen_data_st> cppgen_data;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    for (cppgen_data_st curdat : cppgen_data)
      {
        gc->mark_obj(curdat.cppg_object);
        gc->mark_value(curdat.cppg_data);
      }
  });
  _f.obmodule = argobmodule;
  _f.obgenerator = owner();
  /**
     The "include" attribute of the module describes how would the
     #include-s in the generated C++ file be obtained and generated.

     We need to document more.... **/
  _f.vinclude =
    _f.obmodule->get_attr1(&_,
                           RPS_ROOT_OB(_0XF2N1YQ87p02GXXir)); //"include"∈named_attribute
  RPS_DEBUG_LOG(CODEGEN,"Rps_PayloadCplusplusGen::emit_cplusplus_includes start obmodule=" << _f.obmodule
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
                    "Rps_PayloadCplusplusGen::emit_cplusplus_includes include "
                    << _f.vinclude << " with closure=" << _f.voldval
                    << " obmodule=" << _f.obmodule
                    << " obgenerator=" << _f.obgenerator);
    }
  else
    RPS_DEBUG_LOG(CODEGEN,
                  "Rps_PayloadCplusplusGen::emit_cplusplus_includes plain include "
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
          std::lock_guard<std::recursive_mutex> guobcurincl(*_f.obcurinclude->objmtxptr());
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
          std::lock_guard<std::recursive_mutex> guobcurincl(*_f.obcurinclude->objmtxptr());
          _f.obgenerator->get_dynamic_payload<Rps_PayloadCplusplusGen>()->add_cplusplus_include(&_,_f.obcurinclude);
        }
    }
  else
    {
      RPS_WARNOUT("in Rps_PayloadCplusplusGen::emit_cplusplus_includes C++ generated module "
                  << _f.obmodule
                  << " with generator " << _f.obgenerator
                  << " unexpected include " << _f.vinclude);
      throw RPS_RUNTIME_ERROR_OUT("in Rps_PayloadCplusplusGen::emit_cplusplus_includes obmodule="
                                  << _f.obmodule << " obgenerator=" << _f.obgenerator
                                  << "unexpected include=" << _f.vinclude);
    }
  ///TODO: sort cleverly the cppgen_includeset and emit the #include
  for (Rps_ObjectRef thecurinclob : cppgen_includeset)
    {
      _f.obcurinclude = thecurinclob;
      RPS_ASSERT(_f.obcurinclude);
      std::lock_guard<std::recursive_mutex> guobcurincl(*_f.obcurinclude->objmtxptr());
      long inclprio = 0;
      auto it = cppgen_includepriomap.find(_f.obcurinclude);
      if (it != cppgen_includepriomap.end())
        inclprio = it->second;
      else
        inclprio = compute_include_priority(&_, _f.obcurinclude);
      cppgen_datavect.push_back(cppgen_data_st
      {
        .cppg_object=_f.obcurinclude,
        .cppg_data=nullptr,
        .cppg_num=inclprio});
      continue;
    };
  _f.obcurinclude = nullptr;
  /// now sort the cppgen_datavect according to its cppg_num
  std::sort(cppgen_datavect.begin(), cppgen_datavect.end(),
            [&](const struct cppgen_data_st&x, const struct cppgen_data_st&y)
  {
    if (x.cppg_num < y.cppg_num)
      return true;
    else
      return false;
  });
  int nbinc = cppgen_datavect.size();
  cppgen_outcod << "///++ " << nbinc << " includes in obmodule "
                << _f.obmodule
                << std::endl;
  for (int ix=0; ix<nbinc; ix++)
    {
      _f.obcurinclude = cppgen_datavect[ix].cppg_object;
      RPS_ASSERT(_f.obcurinclude);
      std::lock_guard<std::recursive_mutex>
      guobcurincl(*_f.obcurinclude->objmtxptr());
      _f.vstrpath =
        _f.obcurinclude->get_attr1(&_,
                                   RPS_ROOT_OB(_2774WA6lAq504yCPWa) //"file_path"∈named_attribute
                                  ).as_string();
      if (!_f.vstrpath)
        {
          RPS_WARNOUT("in Rps_PayloadCplusplusGen::emit_cplusplus_includes C++ generated module "
                      << _f.obmodule
                      << " with generator " << _f.obgenerator
                      << " include " << _f.obcurinclude << " without file_path=_2774WA6lAq504yCPWa");
          throw RPS_RUNTIME_ERROR_OUT("in Rps_PayloadCplusplusGen::emit_cplusplus_includes obmodule="
                                      << _f.obmodule << " obgenerator=" << _f.obgenerator
                                      << " include=" << _f.obcurinclude
                                      << " without file_path=_2774WA6lAq504yCPWa");
        }
      cppgen_outcod << "#include \""
                    << Rps_Cjson_String(_f.vstrpath.to_cppstring())
                    << "\"" << std::endl;
    }
#warning incomplete PayloadCplusplusGen::emit_cplusplus_includes
} // end Rps_PayloadCplusplusGen::emit_cplusplus_includes

void
Rps_PayloadCplusplusGen::emit_cplusplus_declarations(Rps_CallFrame*callerframe, Rps_ObjectRef argmodule)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obmodule;
                 Rps_Value vcomp;
                 Rps_ObjectRef obcomp;
                );
  _f.obgenerator = owner();
  _f.obmodule = argmodule;
  // TODO: we probably need a selector to send some message related to C++ declaration emission
  //
  // it could happen that the components number of the module is
  // changing during the loop, or that the components are dynamically
  // inserted (e.g. in the generator object)...
  // TODO: document more.
  for (int cix=0; cix<(int)_f.obmodule->nb_components(&_); cix++)
    {
      _f.obcomp = nullptr;
      std::lock_guard<std::recursive_mutex> gugenerator(*_f.obgenerator->objmtxptr());
      _f.vcomp = _f.obmodule->component_at(&_, cix, /*dontfail=*/true);
      if (!_f.vcomp)
        continue;
      if (_f.vcomp.is_object())
        {
          _f.obcomp = _f.vcomp.as_object();
          std::lock_guard<std::recursive_mutex> guobcomp(*_f.obcomp->objmtxptr());

        }
      else
        {
          RPS_WARNOUT("in module " << _f.obmodule
                      << " component#" << cix
                      << " = " << _f.vcomp
                      << " is not an object," << std::endl
                      << "...so cannot be translated to C++ declaration.");
          throw RPS_RUNTIME_ERROR_OUT("rps_generate_cplusplus_code bad component#" << cix
                                      << " = " << _f.vcomp
                                      << " cannot be declared"
                                      << " in obmodule=" << _f.obmodule
                                      << " obgenerator=" << _f.obgenerator);
        }
    };
#warning incomplete PayloadCplusplusGen::emit_cplusplus_declarations
} // end Rps_PayloadCplusplusGen::emit_cplusplus_declarations

void
Rps_PayloadCplusplusGen::emit_cplusplus_definitions(Rps_CallFrame*callerframe, Rps_ObjectRef argmodule)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obmodule;
                 Rps_Value vcomp;
                 Rps_ObjectRef obcomp;
                );
  _f.obgenerator = owner();
  _f.obmodule = argmodule;
  // TODO: we probably need a selector to send some message related to C++ definition emission
  //
  // It could happen that the number of components in the module is
  // changing during the loop
  for (int cix=0; cix<(int)_f.obmodule->nb_components(&_); cix++)
    {
      _f.obcomp = nullptr;
      _f.vcomp = _f.obmodule->component_at(&_, cix, /*dontfail=*/true);
      if (!_f.vcomp)
        continue;
      if (_f.vcomp.is_object())
        {
          _f.obcomp = _f.vcomp.as_object();
          std::lock_guard<std::recursive_mutex> guobcomp(*_f.obcomp->objmtxptr());
        }
      else
        {
          RPS_WARNOUT("in module " << _f.obmodule
                      << " component#" << cix
                      << " = " << _f.vcomp
                      << " is not an object so cannot be translated to C++ definition.");
          throw RPS_RUNTIME_ERROR_OUT("rps_generate_cplusplus_code bad component#" << cix
                                      << " = " << _f.vcomp
                                      << " cannot be defined"
                                      << " in obmodule=" << _f.obmodule
                                      << " obgenerator=" << _f.obgenerator);
        }
    };
#warning incomplete PayloadCplusplusGen::emit_cplusplus_definitions
} // end Rps_PayloadCplusplusGen::emit_cplusplus_definitions

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
                 Rps_Value vcomp;
                 Rps_Value vmain;
                 Rps_Value vxtra;
                );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    RPS_ASSERT(gc != nullptr);
///  for (auto incob: includeset)
///    gc->mark_obj(incob);
///  for (auto incob: includevect)
///    gc->mark_obj(incob);
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
  std::lock_guard<std::recursive_mutex> gugenerator(*_f.obgenerator->objmtxptr());
  _f.obgenerator->put_attr(RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC), //"code_module"∈named_attribute
                           _f.obmodule);
  auto cppgenpayl = _f.obgenerator->put_new_plain_payload<Rps_PayloadCplusplusGen>();
  RPS_DEBUG_LOG(CODEGEN,
                "rps_generate_cplusplus_code starting obmodule=" << _f.obmodule
                << " obgenerator=" << _f.obgenerator
                << " vgenparam=" << _f.vgenparam
                << RPS_FULL_BACKTRACE_HERE(1, "rps_generate_cplusplus_code/start"));
  try
    {
      Rps_TwoValues two
        = Rps_ObjectValue(_f.obgenerator).send2(&_,
            rpskob_29rlRCUyHHs04aWezh,
            //prepare_cplusplus_generation∈named_selector
            _f.obmodule,
            _f.vgenparam);
      _f.vmain = two.main();
      _f.vxtra = two.xtra();
    }
  catch  (std::exception&exc)
    {
      RPS_WARNOUT("rps_generate_cplusplus_code failed to prepare "
                  << _f.obgenerator << " for module " << _f.obmodule
                  << " got exception " << exc.what()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,"rps_generate_cplusplus_code"));
      return false;
    };
  /// detailed debug display after preparation
  RPS_DEBUG_LOG(CODEGEN,
                "rps_generate_cplusplus_code after preparation genparam="
                << _f.vgenparam << std::endl
                << " obmodule:" << RPS_OBJECT_DISPLAY(_f.obmodule) << std::endl
                << " obgenerator:" << RPS_OBJECT_DISPLAY(_f.obgenerator) << std::endl
                << RPS_FULL_BACKTRACE_HERE(1,"rps_generate_cplusplus_code/after preparation"));
  /// emit the C++ comment with copyright notice
  cppgenpayl->emit_initial_cplusplus_comment(&_, _f.obmodule);
  cppgenpayl->clear_indentation();
  /// emit C++ #include-s
  cppgenpayl->output([&](std::ostringstream&out)
  {
    out << std::endl << std::endl;
    out << "//// include files from " << _f.obmodule << std::endl;
  });
  cppgenpayl->emit_cplusplus_includes(&_,  _f.obmodule);
  cppgenpayl->clear_indentation();
  RPS_DEBUG_LOG(CODEGEN,
                "rps_generate_cplusplus_code emitted C++ includes obmodule=" << _f.obmodule
                << " obgenerator=" << _f.obgenerator
                << RPS_FULL_BACKTRACE_HERE(1, "rps_generate_cplusplus_code/emitted-includes"));
  /// emit C++ declarations
  cppgenpayl->output([&](std::ostringstream&out)
  {
    out << std::endl << std::endl;
    out << "//// C++ declarations from " << _f.obmodule << std::endl;
  });
  cppgenpayl->emit_cplusplus_declarations(&_,  _f.obmodule);
  cppgenpayl->clear_indentation();
  RPS_DEBUG_LOG(CODEGEN,
                "rps_generate_cplusplus_code emitted C++ declarations obmodule=" << _f.obmodule
                << " obgenerator=" << _f.obgenerator
                << RPS_FULL_BACKTRACE_HERE(1, "rps_generate_cplusplus_code/emitted-C++-declarations") << std::endl
                << RPS_OBJECT_DISPLAY(_f.obgenerator));
  /// emit C++ definitions
  cppgenpayl->output([&](std::ostringstream&out)
  {
    out << std::endl << std::endl;
    out << "//// C++ definitions from " << _f.obmodule << std::endl;
  });
  cppgenpayl->emit_cplusplus_definitions(&_,  _f.obmodule);
  cppgenpayl->clear_indentation();
  cppgenpayl->output([&](std::ostringstream&out)
  {
    out << std::endl << std::endl;
    out << "//// end of generated C++ from module " << _f.obmodule << " using generator "
        << _f.obgenerator << " in refpersys git " <<  rps_cppgen_shortgitid
        << " {<" __FILE__ ":" << __LINE__ << ">}" << std::endl;
    out << std::flush;
  });
  sync();
  RPS_WARNOUT("incomplete rps_generate_cplusplus_code obmodule="
              << _f.obmodule << " generator=" << _f.obgenerator
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_generate_cplusplus_code"));
  return false;
} // end rps_generate_cplusplus_code


#warning incomplete cppgen_rps.cc file
