
// see http://refpersys.org/
// passed to commit fa7485505f0acadc of RefPerSys (master branch)
// GPLv3+ licensed
// © 2021 Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createoperators.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obrepldelim;
		 Rps_ObjectRef obdelimdict;
                 Rps_ObjectRef obclassoper;
		 Rps_ObjectRef obreplprecedence;
                 Rps_ObjectRef obclassbinaryop;
                 Rps_ObjectRef obclassunaryop;
		 Rps_ObjectRef obclassrepldelim;
		 Rps_ObjectRef obplusdelim;
		 Rps_ObjectRef obplusbinop;
		 Rps_ObjectRef obminusdelim;
		 Rps_ObjectRef obminusbinop;
		 Rps_ObjectRef obmultdelim;
		 Rps_ObjectRef obmultbinop;
		 Rps_ObjectRef obdivdelim;
		 Rps_ObjectRef obdivbinop;
		 Rps_ObjectRef obmoddelim;
		 Rps_ObjectRef obmodbinop;
                 Rps_Value strname;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obclassrepldelim = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
  _f.obdelimdict = RPS_ROOT_OB(_627ngdqrVfF020ugC5); //"repl_delim"∈string_dictionary
  auto paylsdicdelim =
    _f.obdelimdict->get_dynamic_payload<Rps_PayloadStringDict>();
  RPS_ASSERT(paylsdicdelim != nullptr);
  /// create the repl_operator superclass
  {
    _f.obclassoper = Rps_ObjectRef::make_named_class(&_,
						     Rps_ObjectRef::the_class_class(),
						     "repl_operator");
    _f.strname = Rps_String::make("repl_operator");
    _f.obclassoper->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			       _f.strname);
    rps_add_root_object(_f.obclassoper);
    _f.strname = nullptr;
  }
  /// create the repl_binary_operator superclass
  {
    _f.obclassbinaryop = Rps_ObjectRef::make_named_class(&_,
							   _f.obclassoper,
						     "repl_binary_operator");
    _f.strname = Rps_String::make("repl_binary_operator");
    _f.obclassbinaryop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			       _f.strname);
    rps_add_root_object(_f.obclassbinaryop);
    _f.strname = nullptr;
  }
  /// create the repl_unary_operator superclass
  {
    _f.obclassunaryop = Rps_ObjectRef::make_named_class(&_,
							   _f.obclassoper,
						     "repl_unary_operator");
    _f.strname = Rps_String::make("repl_unary_operator");
    _f.obclassunaryop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			       _f.strname);
    rps_add_root_object(_f.obclassunaryop);
    _f.strname = nullptr;
  }
  /// we are inspired by precedence in C++, see https://en.cppreference.com/w/cpp/language/operator_precedence
  /// create the repl_precedence attribute, conventionally values are small non-negative tagged integers
  {
    _f.obreplprecedence
      = Rps_ObjectRef::make_new_strong_symbol(&_,
					      "repl_precedence");
    _f.strname =  Rps_String::make("repl_precedence");
    _f.obreplprecedence->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			       _f.strname);
    rps_add_root_object(_f.obreplprecedence);
    _f.strname = nullptr;
  }
  //////////////// binary + and addition
  /// create the + delimiter
  {
    _f.obplusdelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("+");
    _f.obplusdelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("plus!delim");
    _f.obplusdelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("+"), _f.obplusdelim);
    /// create the plus binary operation with precedence 6
    _f.obplusbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obplusdelim->put_attr(_f.obclassbinaryop, _f.obplusbinop);
    _f.strname =  Rps_String::make("plus!binop");
    _f.obplusbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obplusbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(6));
    _f.obplusbinop->put_attr( _f.obclassrepldelim, _f.obplusdelim);
  }
  //////////////// binary - and substraction
  /// create the - delimiter
  {
    _f.obminusdelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("-");
    _f.obminusdelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("minus!delim");
    _f.obminusdelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("-"), _f.obminusdelim);
    /// create the minus binary operation with precedence 6
    _f.obminusbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obminusdelim->put_attr(_f.obclassbinaryop, _f.obminusbinop);
    _f.strname =  Rps_String::make("minus!binop");
    _f.obminusbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obminusbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(6));
    _f.obminusbinop->put_attr( _f.obclassrepldelim, _f.obminusdelim);
  }
  //////////////// binary * and multiplication
  /// create the * delimiter
  {
    _f.obmultdelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("*");
    _f.obmultdelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("mult!delim");
    _f.obmultdelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("*"), _f.obmultdelim);
    /// create the mult binary operation with precedence 5
    _f.obmultbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obmultdelim->put_attr(_f.obclassbinaryop, _f.obmultbinop);
    _f.strname =  Rps_String::make("mult!binop");
    _f.obmultbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obmultbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(5));
    _f.obmultbinop->put_attr( _f.obclassrepldelim, _f.obmultdelim);
  }
  //////////////// binary / and division
  /// create the / delimiter
  {
    _f.obdivdelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("/");
    _f.obdivdelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("div!delim");
    _f.obdivdelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("/"), _f.obdivdelim);
    /// create the div binary operation with precedence 5
    _f.obdivbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obdivdelim->put_attr(_f.obclassbinaryop, _f.obdivbinop);
    _f.strname =  Rps_String::make("div!binop");
    _f.obdivbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obdivbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(5));
    _f.obdivbinop->put_attr( _f.obclassrepldelim, _f.obdivdelim);
  }
  //////////////// binary % and modulus
  /// create the % delimiter
  {
    _f.obmoddelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("%");
    _f.obmoddelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("mod!delim");
    _f.obmoddelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("%"), _f.obmoddelim);
    /// create the mod binary operation with precedence 5
    _f.obmodbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obmoddelim->put_attr(_f.obclassbinaryop, _f.obmodbinop);
    _f.strname =  Rps_String::make("mod!binop");
    _f.obmodbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obmodbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(5));
    _f.obmodbinop->put_attr( _f.obclassrepldelim, _f.obmoddelim);
  }
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/local/include  -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createoperators.cc -o /tmp/rpsplug_createoperators.so" ;;
 ** End: ;;
 ****************/

