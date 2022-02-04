/// file ~/tmp/rpsplug_createoperator4.cc
// see http://refpersys.org/
// passed to commit 3d5f59723886b0 of RefPerSys (master branch)
// GPLv3+ licensed
// © 2021 Copyright the RefPerSys team
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createoperator4.so --batch --dump=.

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
		 Rps_ObjectRef obequaldelim;
		 Rps_ObjectRef obequalbinop;
		 Rps_ObjectRef obsamedelim;
		 Rps_ObjectRef obsamebinop;
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
  //////////////// binary = and 'equal' operator
  /// create the = delimiter
  {
    _f.obequaldelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("=");
    _f.obequaldelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("equal!delim");
    _f.obequaldelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("="), _f.obequaldelim);
    /// create the equal binary operation with precedence 10
    _f.obequalbinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obequaldelim->put_attr(_f.obclassbinaryop, _f.obequalbinop);
    _f.strname =  Rps_String::make("equal!binop");
    _f.obequalbinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obequalbinop->put_attr(_f.obreplprecedence,
			    Rps_Value::make_tagged_int(10));
    _f.obequalbinop->put_attr( _f.obclassrepldelim, _f.obequaldelim);
    RPS_DEBUG_LOG(REPL, "obequaldelim=" << _f.obequaldelim
		  << " obequalbinop=" << _f.obequalbinop);
  }
  //////////////// binary == and 'same' operator
  /// create the == delimiter
  {
    _f.obsamedelim = Rps_ObjectRef::make_object
      (&_,
       _f.obclassrepldelim,
       Rps_ObjectRef::root_space());
    _f.strname =  Rps_String::make("==");
    _f.obsamedelim->put_attr(_f.obclassrepldelim, _f.strname);
    _f.strname =  Rps_String::make("same!delim");
    _f.obsamedelim->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    paylsdicdelim->add(std::string("=="), _f.obsamedelim);
    /// create the same binary operation with precedence 10
    _f.obsamebinop = Rps_ObjectRef::make_object
      (&_,
       _f.obclassbinaryop,
       Rps_ObjectRef::root_space());
    _f.obsamedelim->put_attr(_f.obclassbinaryop, _f.obsamebinop);
    _f.strname =  Rps_String::make("same!binop");
    _f.obsamebinop->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
    _f.obsamebinop->put_attr(_f.obreplprecedence,
			    Rps_Value::make_tagged_int(10));
    _f.obsamebinop->put_attr( _f.obclassrepldelim, _f.obsamedelim);
    RPS_DEBUG_LOG(REPL, "obsamedelim=" << _f.obsamedelim
		  << " obsamebinop=" << _f.obsamebinop);
  }
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/local/include  -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createoperator4.cc -o /tmp/rpsplug_createoperator4.so" ;;
 ** End: ;;
 ****************/

