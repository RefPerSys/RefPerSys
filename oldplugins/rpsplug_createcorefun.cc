 // see http://refpersys.org/
 // passed to commit 4429f1f565aa7636a of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createcorefun.so --batch --dump=.
 
 void rps_do_plugin(const Rps_Plugin* plugin)
 {
   RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                  Rps_ObjectRef obcorefun;
                  Rps_ObjectRef obclasscorefun;
                  Rps_Value strname;
                  );
   RPS_INFORMOUT("running plugin " << plugin->plugin_name);
 
   /// the existing class for core function objects
   _f.obclasscorefun = RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS);
   _f.obcorefun = Rps_ObjectRef::make_object
     (&_,
      _f.obclasscorefun,
      Rps_ObjectRef::root_space());
   _f.strname = Rps_String::make("rps_repl_interpret");
   _f.obcorefun->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
                             _f.strname);
   rps_add_root_object(_f.obcorefun);
 
 } // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/refpersys -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createcorefun.cc -o /tmp/rpsplug_createcorefun.so" ;;
  ** End: ;;
  ****************/
