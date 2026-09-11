
// see http://refpersys.org/
// passed to commit d042ff07fce622dc of RefPerSys (master branch)
// GPLv3+ licensed
// © 2021 Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createoperator3.so --batch --dump=.

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
		 Rps_ObjectRef obanddelim;
		 Rps_ObjectRef obandbinop;
		 Rps_ObjectRef obordelim;
		 Rps_ObjectRef oborbinop;
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
  RPS_DEBUG_LOG(REPL, "obclassoper=" << _f.obclassoper
		<< " obclassbinaryop=" << _f.obclassbinaryop
		<< " obclassunaryop=" << _f.obclassunaryop);
  //see https://en.cppreference.com/w/cpp/language/operator_precedence
  /// get repl_precedence attribute, conventionally values are small non-negative tagged integers
  _f.obreplprecedence = RPS_ROOT_OB(_7iVRsTR8u3D00Cy0hp); //repl_precedence∈symbol
  RPS_DEBUG_LOG(REPL, "obreplprecedence=" << _f.obreplprecedence);
  //////////////// binary && and 'and' operator
  /// create the && delimiter
  {
    _f.obanddelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("&&");
    _f.obanddelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("and!delim");
    _f.obanddelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("&&"), _f.obanddelim);
    /// create the and binary operation with precedence 14
    _f.obandbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obanddelim->put_attr(_f.obclassbinaryop, _f.obandbinop);
    _f.strname =  Rps_String::make("and!binop");
    _f.obandbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obandbinop->put_attr(_f.obreplprecedence,
			    Rps_Value::make_tagged_int(14));
    _f.obandbinop->put_attr( _f.obclassrepldelim, _f.obanddelim);
    RPS_DEBUG_LOG(REPL, "obanddelim=" << _f.obanddelim
		  << " obandbinop=" << _f.obandbinop);
  }
  //////////////// binary || and 'or' operator
  /// create the || delimiter
  {
    _f.obordelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("||");
    _f.obordelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("or!delim");
    _f.obordelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("||"), _f.obordelim);
    /// create the or binary operation with precedence 15
    _f.oborbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obordelim->put_attr(_f.obclassbinaryop, _f.oborbinop);
    _f.strname =  Rps_String::make("or!binop");
    _f.oborbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.oborbinop->put_attr(_f.obreplprecedence,
			   Rps_Value::make_tagged_int(15));
    _f.oborbinop->put_attr( _f.obclassrepldelim, _f.obordelim);
    RPS_DEBUG_LOG(REPL, "obordelim=" << _f.obordelim
		  << " oborbinop=" << _f.oborbinop);
  }
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/local/include  -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createoperator3.cc -o /tmp/rpsplug_createoperator3.so" ;;
 ** End: ;;
 ****************/

