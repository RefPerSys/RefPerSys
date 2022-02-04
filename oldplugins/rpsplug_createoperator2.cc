
// see http://refpersys.org/
// passed to commit a47947a0929a4f of RefPerSys (master branch)
// GPLv3+ licensed
// © 2021 Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createoperator2.so --batch --dump=.

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
		 Rps_ObjectRef oblessdelim;
		 Rps_ObjectRef oblessbinop;
		 Rps_ObjectRef obgreaterdelim;
		 Rps_ObjectRef obgreaterbinop;
		 Rps_ObjectRef oblessequaldelim;
		 Rps_ObjectRef oblessequalbinop;
		 Rps_ObjectRef obgreaterequaldelim;
		 Rps_ObjectRef obgreaterequalbinop;
		 Rps_ObjectRef obnotequaldelim;
		 Rps_ObjectRef obnotequalbinop;
                 Rps_Value strname;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obclassrepldelim = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
  _f.obdelimdict = RPS_ROOT_OB(_627ngdqrVfF020ugC5); //"repl_delim"∈string_dictionary
  auto paylsdicdelim =
    _f.obdelimdict->get_dynamic_payload<Rps_PayloadStringDict>();
  RPS_ASSERT(paylsdicdelim != nullptr);
  /// get the repl_operator superclass
  _f.obclassoper =  RPS_ROOT_OB(_9j12Nhm4itk00YYUW7); //repl_operator∈class
  /// get the repl_binary_operator superclass
  _f.obclassbinaryop = RPS_ROOT_OB(_55Z5Wgzuprq01MU6Br); // repl_binary_operator∈class
  /// get the repl_unary_operator superclass
  _f.obclassunaryop = RPS_ROOT_OB(_6vcJz35mTam01zYLjL); //repl_unary_operator∈class
  //see https://en.cppreference.com/w/cpp/language/operator_precedence
  /// get repl_precedence attribute, conventionally values are small non-negative tagged integers
  _f.obreplprecedence = RPS_ROOT_OB(_7iVRsTR8u3D00Cy0hp); //repl_precedence∈symbol
  //////////////// binary < and less
  /// create the < delimiter
  {
    _f.oblessdelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("<");
    _f.oblessdelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("less!delim");
    _f.oblessdelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("<"), _f.oblessdelim);
    /// create the less binary operation with precedence 9
    _f.oblessbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.oblessdelim->put_attr(_f.obclassbinaryop, _f.oblessbinop);
    _f.strname =  Rps_String::make("less!binop");
    _f.oblessbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.oblessbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(9));
    _f.oblessbinop->put_attr( _f.obclassrepldelim, _f.oblessdelim);
  }
  //////////////// binary > and greater
  /// create the > delimiter
  {
    _f.obgreaterdelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make(">");
    _f.obgreaterdelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("greater!delim");
    _f.obgreaterdelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string(">"), _f.obgreaterdelim);
    /// create the greater binary operation with precedence 9
    _f.obgreaterbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obgreaterdelim->put_attr(_f.obclassbinaryop, _f.obgreaterbinop);
    _f.strname =  Rps_String::make("greater!binop");
    _f.obgreaterbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obgreaterbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(9));
    _f.obgreaterbinop->put_attr( _f.obclassrepldelim, _f.obgreaterdelim);
  }
  //////////////// binary <= and lessequal
  /// create the <= delimiter
  {
    _f.oblessequaldelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("<=");
    _f.oblessequaldelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("lessequal!delim");
    _f.oblessequaldelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("<="), _f.oblessequaldelim);
    /// create the lessequal binary operation with precedence 9
    _f.oblessequalbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.oblessequaldelim->put_attr(_f.obclassbinaryop, _f.oblessequalbinop);
    _f.strname =  Rps_String::make("lessequal!binop");
    _f.oblessequalbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.oblessequalbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(9));
    _f.oblessequalbinop->put_attr( _f.obclassrepldelim, _f.oblessequaldelim);
  }
  //////////////// binary > and greater
  /// create the > delimiter
  {
    _f.obgreaterdelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make(">");
    _f.obgreaterdelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("greater!delim");
    _f.obgreaterdelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string(">"), _f.obgreaterdelim);
    /// create the greater binary operation with precedence 9
    _f.obgreaterbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obgreaterdelim->put_attr(_f.obclassbinaryop, _f.obgreaterbinop);
    _f.strname =  Rps_String::make("greater!binop");
    _f.obgreaterbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obgreaterbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(9));
    _f.obgreaterbinop->put_attr( _f.obclassrepldelim, _f.obgreaterdelim);
  }
  //////////////// binary >= and greaterequal
  /// create the >= delimiter
  {
    _f.obgreaterequaldelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make(">=");
    _f.obgreaterequaldelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("greaterequal!delim");
    _f.obgreaterequaldelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string(">="), _f.obgreaterequaldelim);
    /// create the greaterequal binary operation with precedence 9
    _f.obgreaterequalbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obgreaterequaldelim->put_attr(_f.obclassbinaryop, _f.obgreaterequalbinop);
    _f.strname =  Rps_String::make("greaterequal!binop");
    _f.obgreaterequalbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obgreaterequalbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(9));
    _f.obgreaterequalbinop->put_attr( _f.obclassrepldelim, _f.obgreaterequaldelim);
  }
  //////////////// binary != and notequal
  /// create the != delimiter
  {
    _f.obnotequaldelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("!=");
    _f.obnotequaldelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("notequal!delim");
    _f.obnotequaldelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("!="), _f.obnotequaldelim);
    /// create the notequal binary operation with precedence 9
    _f.obnotequalbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obnotequaldelim->put_attr(_f.obclassbinaryop, _f.obnotequalbinop);
    _f.strname =  Rps_String::make("notequal!binop");
    _f.obnotequalbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obnotequalbinop->put_attr(_f.obreplprecedence, Rps_Value::make_tagged_int(9));
    _f.obnotequalbinop->put_attr( _f.obclassrepldelim, _f.obnotequaldelim);
  }
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/local/include  -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createoperator2.cc -o /tmp/rpsplug_createoperator2.so" ;;
 ** End: ;;
 ****************/

