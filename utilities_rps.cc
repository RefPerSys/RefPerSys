
    }
    return 0;
    case RPSPROGOPT_DUMP:
    {
      if (side_effect)
        rps_dumpdir_str = std::string(arg);
    }
    return 0;
    case RPSPROGOPT_CHDIR_BEFORE_LOAD:
    {
      char cwdbuf[rps_path_byte_size+4];
      memset(cwdbuf, 0, sizeof(cwdbuf));
      if (side_effect)
        {
          if (chdir(arg))
            {
              RPS_FATALOUT("failed to chdir before loading to " << arg
                           << ":" << strerror(errno));
              char*cwd = getcwd(cwdbuf, rps_path_byte_size);
              if (!cwd)
                RPS_FATALOUT("failed to getcwd after chdir to " << arg);
              RPS_INFORMOUT("changed current directory before loading to "
                            << cwd);
            };
        }
      return 0;
    }
    case RPSPROGOPT_CHDIR_AFTER_LOAD:
    {
      if (side_effect)
        {
          rps_chdir_path_after_load = arg;
        }
      return 0;
    }
    case RPSPROGOPT_HOMEDIR:
    {
      struct stat rhomstat;
      memset (&rhomstat, 0, sizeof(rhomstat));
      if (stat(arg, &rhomstat))
        RPS_FATAL("failed to stat --refpersys-home %s: %m",
                  arg);
      if (!S_ISDIR(rhomstat.st_mode))
        RPS_FATAL("given --refpersys-home %s is not a directory",
                  arg);
      if ((rhomstat.st_mode & (S_IRUSR|S_IXUSR)) !=  (S_IRUSR|S_IXUSR))
        RPS_FATAL("given --refpersys-home %s is not user readable and executable",
                  arg);
      if (side_effect)
        {
          char*rhomrp = realpath(arg, nullptr);
          if (!rhomrp)
            RPS_FATAL("realpath failed on given --refpersys-home %s - %m",
                      arg);
          if (strlen(rhomrp) >= rps_path_byte_size -1)
            RPS_FATAL("too long realpath %s on given --refpersys-home %s - %m",
                      rhomrp, arg);
          strncpy(rps_bufpath_homedir, rhomrp, rps_path_byte_size -1);
          free (rhomrp), rhomrp = nullptr;
          RPS_INFORMOUT("set RefPerSys home directory to " << rps_bufpath_homedir);
        };
    }
    return 0;
    case RPSPROGOPT_RUN_NAME:
    {

      if (!rps_run_name.empty())
        RPS_FATALOUT("duplicate RefPerSys run name " << rps_run_name << " and " << std::string(arg));
      rps_run_name.assign(std::string(arg));
      RPS_INFORMOUT("set RefPerSys run name to " <<  Rps_QuotedC_String(rps_run_name));
    }
    return 0;
    case RPSPROGOPT_ECHO:
      /// example argument: --echo='Hello here'
    {
      if (!rps_run_name.empty())
        RPS_INFORMOUT(rps_run_name << " echo:" << std::string (arg)
                      << " git:" << rps_shortgitid);
      else
        RPS_INFORMOUT("echo:" << std::string(arg)
                      << " git:" << rps_shortgitid);
    }
    return 0;
    case RPSPROGOPT_USER_PREFERENCES:
      /// example argument: -U ~/myrefpersys.pref
      /// other example: --user-pref=$HOME/myrps.pref
    {
      RPS_POSSIBLE_BREAKPOINT();
      if (!arg || !arg[0] || !strcmp(arg, ".") || !strcmp(arg, "/"))
        {
          RPS_INFORMOUT("no user preferences");
          return 0;
        };
      if (!access(arg, R_OK))
        rps_set_user_preferences(arg);
      else
        RPS_FATALOUT("missing user preferences file " << arg
                     << ":" << strerror(errno));
    }
    return 0;
    case RPSPROGOPT_RUN_DELAY:
    {
      int pos= -1;
      long dl= -1;
      /// example argument: --run-delay=45s for elapsed seconds
      if ((pos= -1), sscanf(arg, "%i s%n", &rps_run_delay, &pos) > 0
          && rps_run_delay>0 && pos>0)
        RPS_INFORMOUT("RefPerSys will run its agenda and eventloop for "
                      <<  rps_run_delay
                      << " elapsed seconds.");
      ///
      /// example argument: --run-delay=10m for elapsed minutes
      else if ( ((pos= -1), (dl=0)),
                sscanf(arg, "%li m%n", &dl, &pos) > 0
                && dl>0 && pos>0)
        {
          rps_run_delay = dl*60;
          RPS_INFORMOUT("RefPerSys will run its agenda and eventloop for "
                        << dl << " minutes so "
                        << rps_run_delay << " elapsed seconds");
        }

      ///
      /// example argument: --run-delay=3h for elapsed hours
      else if ( ((pos= -1), (dl=0)),
                sscanf(arg, "%li h%n", &dl, &pos) > 0
                && dl>0 && pos>0)
        {
          rps_run_delay = dl*3600;
          RPS_INFORMOUT("RefPerSys will run its agenda and eventloop for "
                        << dl << " hours so " << rps_run_delay
                        << " elapsed seconds");
        }
      else
        RPS_FATAL("invalid --run-delay=%s argument.\n"
                  "\t (should be like 90s or 20m or 2h)",
                  arg);
    }
    return 0;
    case RPSPROGOPT_RANDOMOID:
    {
      int nbrand = atoi(arg);
      if (nbrand <= 0) nbrand = 2;
      else if (nbrand > 100) nbrand = 100;
      if (side_effect)
        {
          RPS_INFORM("output of %d random objids generated on %.2f\n", nbrand,
                     rps_wallclock_real_time());
          printf("*    %-20s" "\t  %-19s" "   %-12s" "\t %-10s\n",
                 " objid", "hi", "lo", "hash");
          printf("========================================================"
                 "===========================\n");
          for (int ix = 0; ix<nbrand; ix++)
            {
              auto rid = Rps_Id::random();
              printf("! %22s" "\t  %19lld" " %12lld" "\t %10u\n",
                     rid.to_string().c_str(),
                     (long long) rid.hi(),
                     (long long) rid.lo(),
                     (unsigned) rid.hash());
            }
          printf("--------------------------------------------------------"
                 "---------------------------\n");
          fflush(nullptr);
        }
    }
    return 0;
    case RPSPROGOPT_TYPEINFO:
    {
      if (side_effect)
        rps_print_types_info ();
      rps_batch = true;
    }
    return 0;
    case RPSPROGOPT_SYSLOG:
    {
      if (side_effect && !rps_syslog_enabled)
        {
          rps_syslog_enabled = true;
          openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
          RPS_INFORM("using syslog");
        }
    }
    return 0;
    case RPSPROGOPT_SCRIPT:
    {
      if (arg && !strcmp(arg, "help"))
        rps_scripting_help();
      else if (arg)
        rps_scripting_add_script(arg);
    }
    return 0;
    case RPSPROGOPT_NO_TERMINAL:
    {
      rps_without_terminal_escape = true;
    }
    return 0;
    case RPSPROGOPT_DAEMON:
    {
      rps_without_terminal_escape = true;
      char cwdbuf[rps_path_byte_size];
      memset(cwdbuf, 0, sizeof(cwdbuf));
      const char*cwd = getcwd(cwdbuf, sizeof(cwdbuf)-1);
      if (side_effect)
        {
          if (!rps_syslog_enabled)
            {
              rps_syslog_enabled = true;
              openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
            };
          RPS_INFORM("using syslog with daemon");
          if (daemon(/*nochdir:*/1,  /*noclose:*/0))
            RPS_FATAL("failed to daemon");
          rps_daemonized = true;
          RPS_INFORM("daemonized pid %d in dir %s git %s",
                     (int)getpid(), cwd, rps_shortgitid);
        };
    }
    return 0;
    case RPSPROGOPT_PID_FILE:
    {
      if (side_effect)
        rps_pidfile_path = arg;
    }
    return 0;
    case RPSPROGOPT_FULL_GIT:
    {
      if (side_effect)   /// see also rps_early_initialization
        {
          printf("%s\n", rps_gitid);
          fflush(nullptr);
          exit(EXIT_SUCCESS);
        }
    }
    return 0;
    case RPSPROGOPT_SHORT_GIT:
    {
      if (side_effect)   /// see also rps_early_initialization
        {
          printf("%s\n", rps_shortgitid);
          fflush(nullptr);
          exit(EXIT_SUCCESS);
        }
    }
    return 0;
    case RPSPROGOPT_NO_ASLR:
    {
      // was already handled
      RPS_ASSERT(rps_disable_aslr);
    }
    return 0;
    case RPSPROGOPT_NO_QUICK_TESTS:
    {
      rps_without_quick_tests = true;
    }
    return 0;
    case RPSPROGOPT_TEST_REPL_LEXER:
    {
      if (side_effect)
        {
          if (!rps_test_repl_string.empty())
            RPS_FATALOUT("only one --test-repl-lexer=TESTLEXSTRING can"
                         " be given, but already got "
                         << rps_test_repl_string);
          rps_test_repl_string = arg;
          RPS_INFORMOUT("will test the REPL lexer on string:"
                        << std::endl
                        << rps_test_repl_string
                        << std::endl << "… that is the "
                        << rps_test_repl_string.size()
                        << " bytes string "
                        << Rps_QuotedC_String(rps_test_repl_string));
        }
    }
    return 0;
    case RPSPROGOPT_FILE_REPL_LEXER:
    {
      if (side_effect)
        {
          if (!rps_file_repl_string.empty())
            RPS_FATALOUT("only one --file-repl-lexer=TESTLEX can"
                         " be given, but already got "
                         << rps_file_repl_string);
          rps_file_repl_string = arg;
          bool isfile=arg[0] != '!' && arg[0] != '|';
          RPS_WARNOUT("should test the REPL lexer on "
                      << (isfile?"file":"pipe") << " "
                      <<  Rps_QuotedC_String(rps_file_repl_string));
        }
    }
    return 0;
    case RPSPROGOPT_DEBUG_AFTER_LOAD:
    {
      if (!rps_debugflags_after_load || side_effect)
        rps_debugflags_after_load = arg;
    }
    return 0;
    case RPSPROGOPT_DEBUG_EXIT:
    {
      if (!rps_debugflags_exit || side_effect)
        rps_debugflags_exit = arg;
    }
    return 0;
    case RPSPROGOPT_EXTRA_ARG:
    {
      int eqnextpos= -1;
      char extraname[80];
      memset (extraname, 0, sizeof(extraname));
      if (sscanf(arg, "%72[A-Za-z0-9_]=%n", extraname, &eqnextpos) >= 1
          && isalpha(extraname[0])
          && eqnextpos > 1 && arg[eqnextpos-1] == '=')
        {
          if (strlen(extraname) > sizeof(extraname)-10)
            RPS_WARNOUT("too long extraname " << extraname
                        << " in " << arg);
          for (const char*n = extraname; *n; n++)
            if (!isalnum(*n) && *n != '_')
              RPS_FATALOUT("invalid extra named argument " << extraname);
          if (rps_dict_extra_arg.find(extraname) != rps_dict_extra_arg.end())
            RPS_FATALOUT("extra named argument " << extraname
                         << " cannot be set more than once");
          std::string extraval{arg+eqnextpos};
          rps_dict_extra_arg.insert({extraname, extraval});
          RPS_INFORMOUT("set extra argument " << extraname
                        << " to '" << Rps_QuotedC_String(extraval)
                        << "'");
        }
      else
        RPS_FATALOUT("bad extra named argument " << arg
                     << " that is '" << Rps_QuotedC_String(arg)
                     << "' extra name is '" << Rps_QuotedC_String(extraname) << '"'
                    );
    }
    return 0;
    case RPSPROGOPT_RUN_AFTER_LOAD:
    {
      if (rps_run_command_after_load)
        RPS_FATALOUT("only one --run-after-load command can be given, not both "
                     << rps_run_command_after_load
                     << " and " << arg);
      rps_run_command_after_load = arg;
    }
    return 0;
    case RPSPROGOPT_PLUGIN_AFTER_LOAD:
    {
      char cwdbuf[rps_path_byte_size];
      memset (cwdbuf, 0, sizeof(cwdbuf));
      if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
        strcpy(cwdbuf, "./");
      void* dlh = dlopen(arg, RTLD_NOW|RTLD_GLOBAL);
      if (!dlh)
        RPS_FATALOUT("failed to dlopen plugin " << arg
                     << " : " << dlerror()
                     << " in " << cwdbuf);
      const char* bnplug = basename(arg);
      Rps_Plugin curplugin(bnplug, dlh);
      RPS_INFORMOUT("loaded plugin#" << rps_plugins_vector.size()
                    << " from " << arg << " from process pid#"
                    << (int)getpid()
                    << " basenamed " << Rps_QuotedC_String(bnplug)
                    << " in " << cwdbuf);
      rps_plugins_vector.push_back(curplugin);
    }
    return 0;
    case RPSPROGOPT_PLUGIN_ARG:
    {
      char plugname[80];
      char plugarg[128];
      memset (plugname, 0, sizeof(plugname));
      memset (plugarg, 0, sizeof(plugarg));
      if (!arg)
        RPS_FATALOUT("missing --plugin-arg");
      if (strlen(arg) >= sizeof(plugname) + sizeof(plugarg) - 1)
        RPS_FATALOUT("too long --plugin-arg" << arg
                     << " should be shorter than "
                     << (sizeof(plugname) + sizeof(plugarg)) << " bytes");
      if (sscanf(arg, "%78[a-zA-Z0-9_]:%126s", plugname, plugarg) < 2)
        RPS_FATALOUT("expecting --plugin-arg=<plugin-name>:<plugin-arg-string but got " << arg);
      int pluginix= -1;
      int plugcnt = 0;
      for (Rps_Plugin curplugin: rps_plugins_vector)
        {
          std::string curplugname = curplugin.plugin_name;
          int pluglenam= curplugname.length();
          if (pluglenam > 4 && curplugname.substr(pluglenam-3) == ".so")
            curplugname.erase(pluglenam-3);
          RPS_POSSIBLE_BREAKPOINT();
          RPS_DEBUG_LOG (REPL, "plugin#" << plugcnt << " is named " << Rps_QuotedC_String(curplugname));
          if (curplugname == plugname)
            {
              pluginix = plugcnt;
              break;
            };
          plugcnt++;
        };
      RPS_POSSIBLE_BREAKPOINT();
      if (pluginix<0)
        RPS_FATALOUT("--plugin-arg=" << plugname << ":" << plugarg
                     << " without such loaded plugin"
                     << " (loaded " << plugcnt << " plugins)");
      Rps_Plugin thisplugin = rps_plugins_vector[pluginix];
      rps_pluginargs_map[plugname] = std::string{plugarg};
      RPS_INFORMOUT("registering plugin argument of --plugin-arg " << arg
                    << " plugname=" << Rps_QuotedC_String(plugname)
                    << " plugarg=" << Rps_QuotedC_String(plugarg)
                    << std::endl
                    << RPS_FULL_BACKTRACE(1, "--plugin-arg processing"));
    }
    return 0;
    case RPSPROGOPT_CPLUSPLUSEDITOR_AFTER_LOAD:
    {
      RPS_DEBUG_LOG(CMD, "option --cplusplus-editor "
                    << (arg?" with '":" without ")
                    << (arg?arg:" argument")
                    << (arg?"'":" !!!")
                    << (side_effect?" side-effecting"
                        :" without side effect"));
      if (side_effect)
        {
          if (!arg || !arg[0])
            {
              char* editor = getenv("EDITOR");
              if (editor && editor[0])
                {
                  arg = editor;
                  RPS_INFORMOUT("using $EDITOR variable " << editor << " as C++ editor");
                }
            };
          if (!arg || !arg[0])
            RPS_FATALOUT("program option --cplusplus-editor-after-load"
                         " without explicit editor,\n"
                         "… and no $EDITOR environment variable");
          if (!rps_cpluspluseditor_str.empty())
            RPS_FATALOUT("program option --cplusplus-editor-after-load"
                         " given twice with "
                         << rps_cpluspluseditor_str << " and " << arg);
          rps_cpluspluseditor_str.assign(arg);
        };
    }
    return 0;
    case RPSPROGOPT_CPLUSPLUSFLAGS_AFTER_LOAD:
    {
      if (side_effect)
        {
          if (!rps_cplusplusflags_str.empty())
            RPS_FATALOUT("program option --cplusplus-flags-after-load given twice with "
                         << rps_cplusplusflags_str << " and " << arg);
          rps_cplusplusflags_str.assign(arg);
        }
    }
    return 0;
    case RPSPROGOPT_VERSION:
    {
      if (side_effect)
        {
          rps_show_version();
          exit(EXIT_SUCCESS);
        }
    }
    return 0;
    };        // end switch key
  return ARGP_ERR_UNKNOWN;
} // end rps_parse1opt

struct argp argparser_rps;



// rps_parse_program_arguments is called very early from main...
void
rps_parse_program_arguments(int &argc, char**argv)
{
  errno = 0;
  rps_early_initialization  (argc, argv);
  errno = 0;
  struct argp_state argstate;
  memset (&argstate, 0, sizeof(argstate));
  argparser_rps.options = rps_progoptions; // defined in main_rps.cc
  argparser_rps.parser = rps_parse1opt;
  argparser_rps.args_doc = " ; # ";
  argparser_rps.doc =
    "RefPerSys - an opensource Artificial Intelligence inference engine project,\n"
    " open science, for Linux/x86-64; see refpersys.org for more.\n"
    " (REFlexive PERsystem SYStem is GPLv3+ licensed free software)\n"
    " You should have received a copy of the GNU General Public License\n"
    " along with this program.  If not, see www.gnu.org/licenses\n"
    " *** NO WARRANTY, not even for FITNESS FOR A PARTICULAR PURPOSE ***\n"
    " +++!!! use at your own risk !!!+++\n"
    " (shortgitid " RPS_SHORTGITID ")\n"
    "\n Accepted program options are:\n";
  argparser_rps.children = nullptr;
  argparser_rps.help_filter = nullptr;
  argparser_rps.argp_domain = nullptr;
  int aix= -1;
  if (argp_parse(&argparser_rps, argc, argv, 0, &aix, nullptr))
    RPS_FATALOUT("failed to parse program arguments to " << argv[0]
                 << " at program argument index aix=" << aix);
  if (rps_helpwanted)
    {
      RPS_UNIQUE_BREAKPOINT();
      std::cout << "*** debug level flag in C++ code ***" << std::endl;
      std::cout << "# levelname | explanation" << std::endl;
      char levbuf[80];
#define Rps_Explain_Level_Help(Level,Str) do {          \
    memset(levbuf, 0, sizeof(levbuf));                  \
    snprintf(levbuf, sizeof(levbuf), "\t %14s # %s",    \
             #Level, Str);                              \
    std::cout << levbuf << std::endl;                   \
  } while(0);
      RPS_DEBUG_OPTIONS(Rps_Explain_Level_Help);
#undef Rps_Explain_Level_Help
    } // end if rps_helpwanted
  RPS_POSSIBLE_BREAKPOINT();
} // end rps_parse_program_arguments



/// most of the time this function is used thru RPS_OUT_PROGARGS macro
void
rps_output_program_arguments(std::ostream& out, int argc,
                             const char*const*argv)
{
  for (int i=0; i<argc; i++)
    {
      if (i>0) out << ' ';
      const char*curparg = argv[i];
      if (!curparg)
        break;
      bool goodchar = true;
      for (const char* pc = curparg; goodchar && *pc; pc++)
        {
          if (isalnum(*pc) || *pc=='_' || *pc=='-' || *pc=='+'
              || *pc=='/' || *pc=='.' || *pc==',' || *pc==':'
              || *pc=='=' || *pc=='%' || *pc=='@')
            continue;
          else
            {
              goodchar = false;
              RPS_POSSIBLE_BREAKPOINT();
              break;
            }
        };
      if (goodchar)
        out << curparg;
      else
        out << Rps_QuotedC_String(curparg);
    };
  out << std::endl;
} // end rps_output_program_arguments

void
rps_compute_program_invocation(int argc, char**argv)
{
  std::ostringstream outs;
  rps_output_program_arguments(outs, argc, argv);
  outs.flush();
  std::string pstr = outs.str();
  size_t plen = pstr.size();
  rps_program_invocation = (char*)calloc(1, ((plen+20)|0x1f)+1);
  if (rps_program_invocation)
    strncpy(rps_program_invocation, pstr.c_str(), plen);
} // end rps_compute_program_invocation

const char*
rps_get_plugin_cstr_argument(const Rps_Plugin*plugin)
{
  if (!plugin)
    return nullptr;
  auto it = rps_pluginargs_map.find(plugin->plugin_name);
  if (it == rps_pluginargs_map.end())
    return nullptr;
  return it->second.c_str();
} // end rps_get_plugin_cstr_argument

void
rps_postponed_remove_file(const std::string& path)
{
  std::lock_guard<std::mutex> gu(rps_postponed_lock);
  if (rps_postponed_removed_files_vector.empty())
    rps_atexit(rps_schedule_files_postponed_removal);
  rps_postponed_removed_files_vector.push_back(std::string(path));
} // end rps_postponed_remove_file

void
rps_schedule_files_postponed_removal(void)
{
  std::lock_guard<std::mutex> gu(rps_postponed_lock);
  if (rps_postponed_removed_files_vector.empty())
    return;
  if (access("/bin/rm", X_OK))
    {
      RPS_WARNOUT("missing /bin/rm executable file " << strerror(errno));
      return;
    };
  if (access("/bin/at", X_OK))
    {
      RPS_WARNOUT("missing /bin/at executable file " << strerror(errno));
      return;
    };

  RPS_DEBUG_LOG(EXIT, "rps_schedule_files_postponed_removal" << std::endl
                << RPS_FULL_BACKTRACE(1, "rps_schedule_files_postponed_removal"));
  //// commands will be executed using /bin/sh
  /// the -M option never send mail to user
  FILE* pat = popen("/bin/at -M now + 4 minutes > /dev/null 2>&1", "w");
  if (!pat)
    {
      RPS_WARNOUT("failed to open /bin/at now + 4 minutes :"
                  << strerror(errno));
      return;
    };
  if (rps_syslog_enabled)
    syslog(LOG_NOTICE, "RefPerSys will later remove %d files "
                       "(in four minutes, with /bin/at)",
           (int) rps_postponed_removed_files_vector.size());
  else
    RPS_INFORM("RefPerSys will later remove %d files "
               "(in four minutes, with /bin/at)",
               (int) rps_postponed_removed_files_vector.size());
  for  (auto rf: rps_postponed_removed_files_vector)
    {
      if (rps_syslog_enabled)
        syslog(LOG_NOTICE, "*°postpone removing %s",
               Rps_SingleQuotedC_String(rf).c_str());
      else
        RPS_INFORM("*°postpone removing %s\n",
                   Rps_SingleQuotedC_String(rf).c_str());
      fprintf(pat, "/bin/rm -f '%s'\n",
              Rps_SingleQuotedC_String(rf).c_str());
    }
  fflush(pat);
  rps_postponed_removed_files_vector.clear();
  pclose(pat);
} // end rps_schedule_files_postponed_removal

////////////////
void
rps_fatal_stop_at (const char *filnam, int lin)
{
  static constexpr int skipfatal=2;
  assert(filnam != nullptr);
  assert (lin>=0);
  char errbuf[128];
  memset (errbuf, 0, sizeof(errbuf));
  char cwdbuf[rps_path_byte_size];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  if (!getcwd(cwdbuf, sizeof(cwdbuf)) || cwdbuf[0] == (char)0)
    strcpy(cwdbuf, "./");
  snprintf (errbuf, sizeof(errbuf)-1, "FATAL STOP (%s:%d)/%s",
            filnam, lin, rps_current_pthread_name().c_str());
  /* we always syslog.... */
  syslog(LOG_EMERG, "RefPerSys fatal stop (%s:%d) git %s,\n"
                    "… pid %d on %s,\n"
                    "… elapsed %.3f, process %.3f sec in %s\n%s%s%s%s",
         filnam, lin, rps_shortgitid,
         (int)getpid(), rps_hostname(),
         rps_elapsed_real_time(), rps_process_cpu_time(), cwdbuf,
         (rps_program_invocation?"… started as ":""),
         (rps_program_invocation?:""),
         (rps_run_name.empty()?"":" run "),
         rps_run_name.c_str());
  bool ontty = isatty(STDERR_FILENO);
  if (rps_debug_file)
    {
      fprintf(rps_debug_file, "\n*§*§* RPS FATAL %s:%d %s*§*§*\n", filnam, lin, rps_run_name.c_str());
      if (rps_program_invocation)
        fprintf(rps_debug_file,
                "… started as %s\n", rps_program_invocation);
    }
  if (!rps_syslog_enabled)
    fprintf(stderr, "\n" "%s%sRPS FATAL:%s\n"
                    " RefPerSys gitid %s,\n"
                    "\t on host %s, md5sum %s,\n"
                    "\t elapsed %.3f, process %.3f sec in %s thread %s\n",
            ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
            ontty?RPS_TERMINAL_BLINK_ESCAPE:"",
            ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
            rps_gitid,  rps_hostname(), rps_md5sum,
            rps_elapsed_real_time(), rps_process_cpu_time(), cwdbuf,
            rps_current_pthread_name().c_str());
  if (rps_debug_file && rps_debug_file != stderr && rps_debug_path[0])
    {
      fprintf(stderr, "*°* see debug output in %s\n", rps_debug_path);
      fprintf(rps_debug_file, "RefPerSys gitid %s was started on %s pid %d as:\n",
              rps_shortgitid, rps_hostname(), (int)getpid());
      for (int aix=0; aix<rps_argc; aix++)
        {
          fputc(' ', rps_debug_file);
          const char*curarg = rps_argv[aix];
          bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
                            || curarg[0]=='_' || curarg[0]=='.'
                            || curarg[0]=='-' || curarg[0]=='=' || curarg[0]=='@';
          for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
            isplainarg = *pc>' ' && *pc<(char)127
                         && *pc != '\'' && *pc != '\\' && *pc != '\"'
                         && isprint(*pc);
          if (isplainarg)
            fputs(curarg, rps_debug_file);
          else
            fprintf(rps_debug_file, "'%s'", Rps_QuotedC_String(curarg).c_str());
        }
      fputc('\n', rps_debug_file);
    }
  fflush (stderr);
  fflush (rps_debug_file);
  if (rps_syslog_enabled)
    {
      std::ostringstream outl;
      auto backt= Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},
                                 filnam, lin,
                                 skipfatal, "RefPerSys FATAL ERROR",
                                 &outl);
      backt.output(outl);
      outl << "===== end fatal error at " << filnam << ":" << lin
           << " ======" << std::endl << std::flush;
      outl << "RefPerSys gitid " << rps_shortgitid
           << " was started on " << rps_hostname() << " pid "
           << (int)getpid() << " as";
      if (!rps_run_name.empty())
        outl << " run " << rps_run_name;
      outl << ':' << std::endl;
      for (int aix=0; aix<rps_argc; aix++)
        {
          const char*curarg = rps_argv[aix];
          bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
                            || curarg[0]=='_' || curarg[0]=='.'
                            || curarg[0]=='-' || curarg[0]=='=' || curarg[0]=='@';
          for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
            isplainarg = *pc>' ' && *pc<(char)127
                         && *pc != '\'' && *pc != '\\' && *pc != '\"'
                         && isprint(*pc);
          if (isplainarg)
            outl << ' ' << curarg;
          else
            outl << ' ' << Rps_SingleQuotedC_String(curarg);
        }
      outl << std::endl << "DGBCNT#" << rps_debug_counter() << "(a)"
           << std::flush;
      syslog(LOG_EMERG, "RefPerSys fatal from %s", outl.str().c_str());
    } // end if syslog enabled
  else
    {
      auto backt= Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},
                                 filnam, lin,
                                 skipfatal, "RefPerSys FATAL ERROR",
                                 &std::clog);
      backt.output(std::clog);
      std::clog << "===== end fatal error at " << filnam << ":" << lin
                << " ======" << std::endl << std::flush;
      std::clog << "RefPerSys gitid "
                << rps_shortgitid;
      if (!rps_run_name.empty())
        std::clog << " run " << rps_run_name;
      std::clog << std::endl
                << "… was started on " << rps_hostname()
                << " pid " << (int)getpid() << " as:" << std::endl;
      for (int aix=0; aix<rps_argc; aix++)
        {
          const char*curarg = rps_argv[aix];
          bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
                            || curarg[0]=='_' || curarg[0]=='.'  || curarg[0]=='-';
          for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
            isplainarg = *pc>' ' && *pc<(char)127
                         && *pc != '\'' && *pc != '\\' && *pc != '\"'
                         && isprint(*pc);
          if (isplainarg)
            std::clog << ' ' << curarg;
          else
            std::clog << ' ' << Rps_SingleQuotedC_String(curarg);
        }
      std::clog << std::endl << "DGBCNT#" << rps_debug_counter() << "(b)"
                << std::flush;
      std::clog << std::endl << std::flush;
    } // end if syslog disabled
  fflush(nullptr);
  RPS_POSSIBLE_BREAKPOINT();
  fprintf(stderr, "RefPerSys (git %s run %s) fatal stop\n"
                  "… °aborting at %s:%d\n"
                  "… invocation %s\n",
          rps_shortgitid, rps_run_name.c_str(), filnam, lin,
          rps_program_invocation);
  fflush(nullptr);
  RPS_POSSIBLE_BREAKPOINT();
  rps_schedule_files_postponed_removal();
  abort();
} // end rps_fatal_stop_at


void rps_debug_warn_at_msg(const char*file, int line, const char*msg)
{
  if (rps_syslog_enabled)
    {
      if (msg)
        syslog(LOG_WARNING,
               "** REFPERSYS WARNING AT %s:%d :\n"
               " [%s] (git %s pid %d) **", file, line, msg,
               rps_shortgitid, (int)getpid());
      else
        syslog(LOG_WARNING, "** REFPERSYS WARNING AT %s:%d\n"
                            " (git %s pid %d) **", file, line,
               rps_shortgitid, (int)getpid());
    }
  else
    {
      std::cerr << std::flush;
      if (msg)
        std::cerr << std::endl << "**!** REFPERSYS WARNING at "
                  << file << ":" << line << std::endl
                  << " " << msg << std::endl;
      else
        std::cerr << std::endl << "**!** REFPERSYS WARNING at "
                  << file << ":" << line
                  << std::endl;

    };
  if (rps_debug_file)
    {
      if (msg)
        fprintf(rps_debug_file,
                "\n*** REFPERSYS WARNING at %s:%d\n %s ***\n",
                file, line, msg);
      else
        fprintf(rps_debug_file,
                "\n*** REFPERSYS WARNING at %s:%d ***\n",
                file, line);
      fflush(rps_debug_file);
    }
} // end rps_debug_warn_at


////////////////////////////////////////////////////////////////
// TIME ROUTINES
////////////////////////////////////////////////////////////////

double rps_elapsed_real_time(void)
{
  return rps_monotonic_real_time() - rps_start_monotonic_time;
}

double rps_get_start_wallclock_real_time()
{
  return rps_start_wallclock_real_time;
}


void
rps_print_objectref(Rps_ObjectRef ob)
{
  std::cout << ob << std::endl;
} // end rps_print_objectref

////////////////////////////////////////////////////////////////
///// global roots for garbage collection and persistence

static std::set<Rps_ObjectRef> rps_object_root_set;
static std::mutex rps_object_root_mtx;
static std::unordered_map<Rps_Id,Rps_ObjectRef*,Rps_Id::Hasher> rps_object_global_root_hashtable;

void
rps_each_root_object (const std::function<void(Rps_ObjectRef)>&fun)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (auto ob: rps_object_root_set)
    fun(ob);
} // end rps_each_root_object


void
rps_add_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_root_set.insert(ob);
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      *(rootit->second) = ob;
  }
} // end rps_add_root_object


bool
rps_remove_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  if (it == rps_object_root_set.end())
    return false;
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      (*(rootit->second)) = Rps_ObjectRef(nullptr);
  }
  rps_object_root_set.erase(it);
  return true;
} // end rps_remove_root_object

void
rps_initialize_roots_after_loading (Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_global_root_hashtable.max_load_factor(3.5);
  rps_object_global_root_hashtable.reserve(5*rps_hardcoded_number_of_roots()/4+3);
  ///
#define RPS_INSTALL_ROOT_OB(Oid) {              \
    const char*end##Oid = nullptr;              \
    bool ok##Oid = false;                       \
    Rps_Id id##Oid(#Oid, &end##Oid, &ok##Oid);  \
    RPS_ASSERT (end##Oid && !*end##Oid);        \
    RPS_ASSERT (ok##Oid);                       \
    RPS_ASSERT (id##Oid.valid());               \
    rps_object_global_root_hashtable[id##Oid]   \
      = &RPS_ROOT_OB(Oid);                      \
  };
#include "generated/rps-roots.hh"
} // end of rps_initialize_roots_after_loading

bool rps_is_root_object (const Rps_ObjectRef ob)
{
  if (!ob)
    return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  return it != rps_object_root_set.end();
} // end rps_is_root_object

std::set<Rps_ObjectRef>
rps_set_root_objects(void)
{
  std::set<Rps_ObjectRef> set;

  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (Rps_ObjectRef ob: rps_object_root_set)
    set.insert(ob);
  return set;
} // end rps_set_root_objects

unsigned
rps_nb_root_objects(void)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  return (unsigned) rps_object_root_set.size();
} // end rps_nb_root_objects


void
rps_add_constant_object(Rps_CallFrame*callframe, const Rps_ObjectRef argob)
{
  if (!argob)
    return;
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp), //"constant"∈named_attribute
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obconst;
                           Rps_ObjectRef obsystem;
                           Rps_ObjectRef obnamedattr;
                           Rps_ObjectRef oboldroot;
                           Rps_Value oldsetv;
                           Rps_Value newsetv;
                           Rps_Value xtrav;
                );
  _f.obconst = argob;
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object start adding " << _f.obconst
                << " of class " <<  _f.obconst->get_class()
                << " in space " << _f.obconst->get_space() << std::endl
                << RPS_FULL_BACKTRACE(1, "rps_add_constant_object/start"));
  RPS_POSSIBLE_BREAKPOINT();
  if (false
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_10YXWeY7lYc01RpQTA) //the_system_class∈class
      || _f.obconst == RPS_ROOT_OB(_1Io89yIORqn02SXx4p) //RefPerSys_system∈the_system_class
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) //int∈class
      || _f.obconst == RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC) //"code_module"∈named_attribute
      || _f.obconst == RPS_ROOT_OB(_3rXxMck40kz03RxRLM) //code_chunk∈class
      || _f.obconst == RPS_ROOT_OB(_3s7ztCCoJsj04puTdQ) //agenda∈class
      || _f.obconst == RPS_ROOT_OB(_3GHJQW0IIqS01QY8qD) //json∈class
      || _f.obconst == RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5) //symbol∈symbol
      || _f.obconst == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) //class∈class
      || _f.obconst == RPS_ROOT_OB(_4jISxMJ4PYU0050nUl) //closure∈class
      || _f.obconst == RPS_ROOT_OB(_4pSwobFHGf301Qgwzh) //named_attribute∈class
      || _f.obconst == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
      || _f.obconst == RPS_ROOT_OB(_5CYWxcChKN002rw1fI) //contributor_to_RefPerSys∈class
      || _f.obconst == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
      || _f.obconst == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
      || _f.obconst == RPS_ROOT_OB(_6fmq7pZkmNd03UyPuO) //class∈symbol
      || _f.obconst == RPS_ROOT_OB(_6gxiw0snqrX01tZWW9) //"set_of_core_functions"∈mutable_set
      || _f.obconst == RPS_ROOT_OB(_6ulDdOP2ZNr001cqVZ) //immutable_instance∈class
      || _f.obconst == RPS_ROOT_OB(_6JYterg6iAu00cV9Ye) //set∈class
      || _f.obconst == RPS_ROOT_OB(_6NVM7sMcITg01ug5TC) //tuple∈class
      || _f.obconst == RPS_ROOT_OB(_6XLY6QfcDre02922jz) //value∈class
      || _f.obconst == RPS_ROOT_OB(_7OrPRWQEg2o043XvK2) //rps_routine∈class
      || _f.obconst == RPS_ROOT_OB(_7Y3AyF9gNx700bQJXc) //string_buffer∈class
      || _f.obconst == RPS_ROOT_OB(_8fYqEw8vTED03wsznt) //tasklet∈class
      || _f.obconst == RPS_ROOT_OB(_8J6vNYtP5E800eCr5q) //"initial_space"∈space
      || _f.obconst == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
      || _f.obconst == RPS_ROOT_OB(_9uwZtDshW4401x6MsY) //space∈symbol
      || _f.obconst == RPS_ROOT_OB(_9BnrMLXUhfG00llx8X) //function∈class
      || _f.obconst == RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS) //core_function∈class
     )
    {
      RPS_POSSIBLE_BREAKPOINT();
      RPS_WARNOUT("cannot add core sacred root object as constant "
                  << RPS_OBJECT_DISPLAY(_f.obconst)
                  << " of class " << _f.obconst->get_class()
                  << " thread " << rps_current_pthread_name()
                  << std::endl
                  << RPS_FULL_BACKTRACE(1, "rps_add_constant_object")
                 );
      return;
    };
  RPS_POSSIBLE_BREAKPOINT();
  _f.obsystem = RPS_ROOT_OB(_1Io89yIORqn02SXx4p); //RefPerSys_system∈the_system_class
  std::lock_guard<std::recursive_mutex> gu(*_f.obsystem->objmtxptr());
  _f.oldsetv
    = _f.obsystem->get_physical_attr (RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp)); // //"constant"∈named_attribute
  RPS_ASSERT(_f.oldsetv.is_set());
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst="  << _f.obconst << " oldset=" << _f.oldsetv);
  if (_f.oldsetv.as_set()->contains(_f.obconst))
    {
      RPS_POSSIBLE_BREAKPOINT();
      // if the constant is already known, we issue a warning
      RPS_WARNOUT("adding already known constant " << _f.obconst
                  << " of class " << _f.obconst->get_class()
                  << " in  thread " << rps_current_pthread_name()
                  << std::endl
                  << RPS_FULL_BACKTRACE(1, "rps_add_constant_object/known")
                 );
      return;
    };
  RPS_POSSIBLE_BREAKPOINT();
  _f.newsetv = Rps_SetValue({_f.oldsetv, Rps_Value(_f.obconst)});
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst="
                << _f.obconst << " oldset=" << _f.oldsetv
                << " newset=" << _f.newsetv
                << " obsystem=" << _f.obsystem);
  RPS_ASSERT(_f.newsetv.is_set() && _f.newsetv.as_set()->cardinal() > 0);
  RPS_ASSERT(_f.newsetv.as_set()->cardinal() > _f.oldsetv.as_set()->cardinal());
  RPS_POSSIBLE_BREAKPOINT();
  /// update the set of constants
  _f.obsystem->put_attr(RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp), // //"constant"∈named_attribute
                        _f.newsetv);
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst=" << _f.obconst
                << " of class " << _f.obconst->get_class() << " space " << _f.obconst->get_space()
                << std::endl
                << "… oldfsetv=" << _f.oldsetv << " newsetv=" << _f.newsetv << " in " << _f.obsystem
                << RPS_FULL_BACKTRACE(1, "rps_add_constant_object/ending"));
  RPS_POSSIBLE_BREAKPOINT();
  _f.xtrav = _f.obsystem->get_physical_attr(RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp));
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst=" << _f.obconst
                << " of class " << _f.obconst->get_class() << " space " << _f.obconst->get_space()
                << std::endl
                << "… oldfsetv=" << _f.oldsetv
                << std::endl << "… newsetv=" << _f.newsetv
                << std::endl << "… xtrav=" << _f.xtrav << " " << ((_f.xtrav  == _f.newsetv)?"same":"different")
                << " in " << _f.obsystem
                << RPS_FULL_BACKTRACE(1, "rps_add_constant_object/ending2"));
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst="
                << RPS_OBJECT_DISPLAY(_f.obconst) << std::endl
                << " obsystem=" << RPS_OBJECT_DISPLAY(_f.obsystem)
                << std::endl << "xtrav=" << _f.xtrav
                << " newsetv=" << _f.newsetv);
  RPS_ASSERT(_f.xtrav  == _f.newsetv);
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object final obsystem=" << _f.obsystem);
  RPS_POSSIBLE_BREAKPOINT();
#pragma message "perhaps rps_add_constant_object should remove obconst from the set of roots?"
} // end rps_add_constant_object

void
rps_remove_constant_object(Rps_CallFrame*callframe, const Rps_ObjectRef argobconst)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp), //"constant"∈named_attribute
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obconst;
                           Rps_ObjectRef obsystem;
                           Rps_ObjectRef obnamedattr;
                           Rps_ObjectRef oboldroot;
                           Rps_Value oldsetv;
                           Rps_Value newsetv;
                );
  _f.obconst = argobconst;
  if (false
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_10YXWeY7lYc01RpQTA) //the_system_class∈class
      || _f.obconst == RPS_ROOT_OB(_1Io89yIORqn02SXx4p) //RefPerSys_system∈the_system_class
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) //int∈class
      || _f.obconst == RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC) //"code_module"∈named_attribute
      || _f.obconst == RPS_ROOT_OB(_3rXxMck40kz03RxRLM) //code_chunk∈class
      || _f.obconst == RPS_ROOT_OB(_3s7ztCCoJsj04puTdQ) //agenda∈class
      || _f.obconst == RPS_ROOT_OB(_3GHJQW0IIqS01QY8qD) //json∈class
      || _f.obconst == RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5) //symbol∈symbol
      || _f.obconst == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) //class∈class
      || _f.obconst == RPS_ROOT_OB(_4jISxMJ4PYU0050nUl) //closure∈class
      || _f.obconst == RPS_ROOT_OB(_4pSwobFHGf301Qgwzh) //named_attribute∈class
      || _f.obconst == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
      || _f.obconst == RPS_ROOT_OB(_5CYWxcChKN002rw1fI) //contributor_to_RefPerSys∈class
      || _f.obconst == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
      || _f.obconst == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
      || _f.obconst == RPS_ROOT_OB(_6fmq7pZkmNd03UyPuO) //class∈symbol
      || _f.obconst == RPS_ROOT_OB(_6gxiw0snqrX01tZWW9) //"set_of_core_functions"∈mutable_set
      || _f.obconst == RPS_ROOT_OB(_6ulDdOP2ZNr001cqVZ) //immutable_instance∈class
      || _f.obconst == RPS_ROOT_OB(_6JYterg6iAu00cV9Ye) //set∈class
      || _f.obconst == RPS_ROOT_OB(_6NVM7sMcITg01ug5TC) //tuple∈class
      || _f.obconst == RPS_ROOT_OB(_6XLY6QfcDre02922jz) //value∈class
      || _f.obconst == RPS_ROOT_OB(_7OrPRWQEg2o043XvK2) //rps_routine∈class
      || _f.obconst == RPS_ROOT_OB(_7Y3AyF9gNx700bQJXc) //string_buffer∈class
      || _f.obconst == RPS_ROOT_OB(_8fYqEw8vTED03wsznt) //tasklet∈class
      || _f.obconst == RPS_ROOT_OB(_8J6vNYtP5E800eCr5q) //"initial_space"∈space
      || _f.obconst == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
      || _f.obconst == RPS_ROOT_OB(_9uwZtDshW4401x6MsY) //space∈symbol
      || _f.obconst == RPS_ROOT_OB(_9BnrMLXUhfG00llx8X) //function∈class
      || _f.obconst == RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS) //core_function∈class
     )
    {
      RPS_WARNOUT("cannot remove core sacred root object as constant " << _f.obconst
                  << " thread " << rps_current_pthread_name()
                  << std::endl
                  << RPS_FULL_BACKTRACE(1, "rps_remove_constant_object")
                 );
      return;
    };
#pragma message "rps_remove_constant_object unimplemented"
  RPS_FATALOUT("rps_remove_constant_object unimplemented obconst=" << RPS_OBJECT_DISPLAY(_f.obconst));
} // end rps_remove_constant_object

void
rps_initialize_symbols_after_loading(Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::recursive_mutex> gu(Rps_PayloadSymbol::symb_tablemtx);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.max_load_factor(2.5);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.reserve(5*rps_hardcoded_number_of_symbols()/4+3);
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Name) {           \
    Rps_PayloadSymbol::symb_hardcoded_hashtable[#Name]  \
      = &RPS_SYMB_OB(Name);                             \
  };
#include "generated/rps-names.hh"
} // end of rps_initialize_symbols_after_loading

///////////////////////////////////////////////////////// debugging support
/// X macro tricks used below... see en.wikipedia.org/wiki/X_Macro

bool
rps_is_set_debug(const std::string &curlev)
{
  if (curlev.empty()) return false;
#define Rps_IS_SET_DEBUG(Opt,Help) else if (curlev == #Opt)     \
    return  rps_debug_flags & (1 << RPS_DEBUG_##Opt);
  RPS_DEBUG_OPTIONS(Rps_IS_SET_DEBUG);
#undef Rps_IS_SET_DEBUG
  return false;
} // end rps_is_set_debug

Rps_Debug
rps_debug_of_string(const std::string &deblev)
{
  if (deblev.empty()) return RPS_DEBUG__NONE;
#define Rps_TEST_DEBUG(Opt,Help) else if (deblev == #Opt) return RPS_DEBUG_##Opt;
  RPS_DEBUG_OPTIONS(Rps_TEST_DEBUG);
#undef Rps_TEST_DEBUG
  return RPS_DEBUG__NONE;
} // end rps_debug_of_string

bool
rps_set_debug_flag(const std::string &curlev)
{
  bool goodflag = false;
  if (curlev == "NEVER")
    {
      RPS_WARNOUT("forbidden debug level " << curlev);
    }
  else if (curlev == "help")
    {
      goodflag = true;
    }
  ///
  /* second X macro trick for processing several comma-separated debug flags, in all cases as else if branch  */
  ///
#define Rps_SET_DEBUG(Opt,Hlp)                        \
  else if (curlev == #Opt) {                          \
    bool alreadygiven = rps_debug_flags               \
      & (1 << RPS_DEBUG_##Opt);                       \
    rps_debug_flags |= (1 << RPS_DEBUG_##Opt);        \
    goodflag = true;                                  \
    if (!alreadygiven)                                \
      RPS_INFORMOUT("setting debugging flag "         \
                    << #Opt << " for " << Hlp);  }
  ///
  RPS_DEBUG_OPTIONS(Rps_SET_DEBUG);
#undef Rps_SET_DEBUG
  ////
  if (!goodflag)
    RPS_WARNOUT("unknown debug level " << curlev);
  return goodflag;
} // end rps_set_debug_flag

void
rps_set_debug(const std::string &deblev)
{
  static bool didhelp;
  if (deblev == "help" && !didhelp)
    {
      /* first X macro for help debug flag.... */
      didhelp = true;
      fprintf(stderr, "%s debugging options for git %s ...\n",
              rps_progname, rps_shortgitid);
      fprintf(stderr, "Comma separated debugging levels with -D<debug-level>\n"
                      "\tor --debug=<debug-level> or --debug-after-load=<debug-level>:\n");

#define Rps_SHOW_DEBUG(Opt,Hlp) fprintf(stderr, "\t%s [%s]\n", #Opt, Hlp);
      RPS_DEBUG_OPTIONS(Rps_SHOW_DEBUG);
#undef Rps_SHOW_DEBUG
      fflush(nullptr);
    }
  else if (deblev.empty())
    {
      RPS_WARNOUT("empty debugging from " << RPS_FULL_BACKTRACE(1, "rps_set_debug/empty"));
    }
  else if (isdigit(deblev[0]))
    {
      char*pend = nullptr;
      long lev = strtol(&deblev[0], &pend, 0);
      if (pend && *pend != (char)0)
        RPS_WARNOUT("bad numerical debug level " << lev << " in " << deblev);
      rps_debug_flags = lev;
    }
  else
    {
      const char*comma=nullptr;
      for (const char*pc = deblev.c_str(); pc && *pc; pc = comma?(comma+1):nullptr)
        {
          comma = strchr(pc, ',');
          std::string curlev;
          if (comma && comma>pc)
            curlev = std::string(pc, comma-pc);
          else
            curlev = std::string(pc);
          if (!rps_set_debug_flag(curlev))
            RPS_FATALOUT("unexpected debug level " << curlev
                         << "; use --debug=help to get all known debug levels");
        };      // end for const char*pc ...

    } // else case, for deblev which is not help

  RPS_DEBUG_LOG(MISC, "rps_debug_flags=" << rps_debug_flags);
} // end rps_set_debug

void
rps_add_debug_cstr(const char*d)
{
  rps_set_debug(std::string(d));
} // end rps_add_debug_cstr


const char*
rps_cstr_of_debug(Rps_Debug dbglev)
{
  switch (dbglev)
    {
#define Rps_CSTR_DEBUG(Lev,Help) case RPS_DEBUG_##Lev: return #Lev;
      RPS_DEBUG_OPTIONS(Rps_CSTR_DEBUG);
#undef Rps_CSTR_DEBUG
    default:
      ;
    }
  return nullptr;
} // end rps_cstr_of_debug

void
rps_output_debug_flags(std::ostream&out,  unsigned flags)
{
  if (!flags)
    flags = rps_debug_flags.load();
  out << flags << "=" ;
  int nbf = 0;
  //
#define SHOW_DBGFLAG(Lev,Hlp)                   \
  do {                                          \
    if (flags & (1<< RPS_DEBUG_##Lev)) {        \
      if (nbf > 0)                              \
  out << ',';                                   \
      out << #Lev << "//" << Hlp << std::endl;  \
      nbf++;                                    \
    }                                           \
  } while(0);
  ///
  RPS_DEBUG_OPTIONS(SHOW_DBGFLAG);
#undef SHOW_DBGFLAG
  out << std::flush;
} // end rps_output_debug_flags





////////////////////////////////////////////////////////////////

static std::recursive_mutex rps_aftevntloop_mtx;
static std::vector<std::function<void(void)>> rps_aftevntloop_vec;

void
rps_register_after_event_loop(std::function<void(void)>f)
{
  std::lock_guard<std::recursive_mutex> gu(rps_aftevntloop_mtx);
  rps_aftevntloop_vec.push_back(f);
} // end rps_register_after_event_loop

void
rps_run_after_event_loop(void)
{
  std::lock_guard<std::recursive_mutex> gu(rps_aftevntloop_mtx);
  int size = (int) rps_aftevntloop_vec.size();
  RPS_DEBUG_LOG(EXIT, "rps_run_after_event_loop size=" << size
                << std::endl
                << RPS_FULL_BACKTRACE(1, "rps_run_after_event_loop"));
  for (std::function<void(void)> f: rps_aftevntloop_vec)
    f();
  rps_aftevntloop_vec.clear();
  RPS_DEBUG_LOG(EXIT, "rps_run_after_event_loop ending size=" << size);
} // end rps_run_after_event_loop



////////////////////////////////////////////////////////////////
std::string
rps_stringprintf(const char*fmt, ...)
{
  va_list args;
  char smallbuf[256];
  memset (smallbuf, 0, sizeof(smallbuf));
  RPS_ASSERT(fmt);
  va_start(args, fmt);
  size_t l = vsnprintf(smallbuf, sizeof(smallbuf)-2, fmt, args);
  va_end(args);
  if (l < sizeof(smallbuf)-4)
    {
      return std::string{smallbuf};
    }
  else
    {
      std::string res;
      size_t ml = ((l+4)|0xf)+4;
      char*buf = (char*)calloc(1, ml);
      if (!buf)
        RPS_FATALOUT("rps_stringprintf fmt " << fmt
                     << " fail to calloc " << ml << " bytes");
      va_start(args, fmt);
      size_t ll =  vsnprintf(buf, ml, fmt, args);
      RPS_ASSERT(ll == l);
      va_end(args);
      res=std::string(buf);
      free(buf);
      return res;
    }
} // end rps_stringprintf


#warning we need some rps_initialize_indented_ostream

void
rps_initialize_indented_ostream(std::ostream&out, unsigned linewidth)
{
#warning unimplemented rps_initialize_indented_ostream
  /// which would use register_callback and xalloc and pword and iword
  /// and we would have some rps_nl C++ output manipulator
  //! https://en.cppreference.com/cpp/io/ios_base/register_callback
} // end rps_initialize_indented_ostream

/// which would use register_callback and xalloc and pword and iword

#warning and also need some rps_nl C++ output manipulator
void
rps_output_vector_string(std::ostream&out, const std::vector<std::string>&vecstr, int indent)
{
  size_t sizvec = vecstr.size();
  std::streampos inipos = out.tellp();
  char bufsiz[32];
  memset (bufsiz, 0, sizeof(bufsiz));
  int sizln = snprintf(bufsiz, sizeof(bufsiz), "vecstr.ℓ%zd(", sizvec);
  RPS_ASSERT(sizln>0);
  out << bufsiz;
  int vecix=0;
#warning TODO: use rps_nl once defined
  for (const std::string& curstr: vecstr)
    {
      char bufidx[16];
      memset(bufidx, 0, sizeof(bufidx));
      snprintf(bufidx, sizeof(bufidx), "[%d]", vecix);
      if (vecix>0)
        {
          out << std::endl;
          for (int i=0; i<indent; i++) out << ' ';
        };
      out << bufidx;
      out << Rps_QuotedC_String(curstr) << std::flush;
    }
  out << ")endvecstr" << std::flush;
#warning TODO: use rps_nl once defined
} // end rps_output_vector_string


const std::string
rps_real_shell_file_path(const std::string& filpath)
{
  std::string restr;
  RPS_DEBUG_LOG(REPL, "+rps_real_shell_file_path filpath="
                << Rps_QuotedC_String(filpath)
                << std::endl
                << RPS_FULL_BACKTRACE(1, "+rps_real_shell_file_path"));
  if (filpath.empty())
    return filpath;
  char buf[rps_path_byte_size+8];
  memset(buf, 0, sizeof(buf));
  std::size_t pathsize = filpath.size();
  if (pathsize > rps_path_byte_size)
    {
      RPS_WARNOUT("rps_real_shell_file_path with too long path "
                  << Rps_QuotedC_String(filpath)
                  << std::endl
                  << RPS_FULL_BACKTRACE(1, "rps_real_shell_file_path"));
      return filpath;
    };
  RPS_POSSIBLE_BREAKPOINT();
  if (access(filpath.c_str(), F_OK))
    {
      /// non-existent or inaccessible filpath...
      int lastslash = filpath.find_last_of('/');
      if (lastslash>0 && lastslash+1 < (int)pathsize
          && filpath[lastslash+1]!='.')
        {
          std::string dirpath = filpath.substr(0, lastslash);
          std::string basepath = filpath.substr(lastslash+1);
          RPS_DEBUG_LOG(REPL, "rps_real_shell_file_path dirpath="
                        << Rps_QuotedC_String(dirpath)
                        << " basepath="
                        << Rps_QuotedC_String(basepath)
                        << " for filpath="
                        << Rps_QuotedC_String(filpath));
          std::string realdirpath;
          char*rp = realpath(dirpath.c_str(), nullptr);
          RPS_POSSIBLE_BREAKPOINT();
          if (!rp)
            {
              RPS_WARNOUT("rps_real_shell_file_path realpath("
                          << Rps_QuotedC_String(dirpath)
                          << ") for "
                          << Rps_QuotedC_String(filpath)
                          << " failure: " << strerror(errno));
              return filpath;
            };
          RPS_DEBUG_LOG(REPL, "rps_real_shell_file_path rp="
                        << Rps_QuotedC_String(rp));
          int rplen = (int)strlen(rp);
          RPS_POSSIBLE_BREAKPOINT();
          realdirpath.reserve(rplen+2);
          realdirpath.copy(rp, rplen);
          free(rp), rp=nullptr;
          restr = realdirpath + "/" + basepath;
          RPS_DEBUG_LOG(REPL, "rps_real_shell_file_path restr="
                        << Rps_QuotedC_String(restr));
        };
    }
  else
    {
      /** The access(2) system call tells us that the file existed, in
       * an hostile environment it could have been removed since by an
       * other process... We deliberately ignore such a scenario.
       **/
      char*rp = realpath(filpath.c_str(), nullptr);
      if (!rp || !rp[0])
        {
          RPS_WARNOUT("rps_real_shell_file_path filpath("
                      << Rps_QuotedC_String(filpath)
                      << " failure: " << strerror(errno));
          return filpath;
        };
      int rplen = (int)strlen(rp);
      RPS_DEBUG_LOG(REPL, "rps_real_shell_file_path rp="
                    << Rps_QuotedC_String(rp)
                    << std::endl
                    << "… rplen=" << rplen
                    << " filpath=" << Rps_QuotedC_String(filpath));
      restr.reserve(rplen);
      restr.assign(rp,rplen);
      RPS_ASSERT_LOG((int)restr.size() == rplen,
                     "rplen=" << rplen
                     << " rp=" << Rps_QuotedC_String(rp)
                     << std::endl
                     << "… restr=" << Rps_QuotedC_String(restr)
                     << " of size=" << restr.size());
      RPS_ASSERT(restr.c_str() != rp);
      RPS_DEBUG_LOG(REPL, "rps_real_shell_file_path restr="
                    << Rps_QuotedC_String(restr));
      free(rp);
    }
  RPS_ASSERT(!restr.empty());
#warning rps_real_shell_file_path to be improved for pathological paths
  // pathological path could contain control characters
  const char*homedir = rps_homedir();
  int homelen = strlen(homedir);
  if ((int)restr.size() > homelen && restr[homelen] == '/'
      && !strncmp(restr.c_str(), homedir, homelen)
     )
    return std::string ("~/") + restr.substr(homelen+1);
  else
    return restr;
} // end of rps_real_shell_file_path


constexpr unsigned rps_numlen = 40;
const std::string
rps_decimal_string(intptr_t i)
{
  bool neg = (i<0);
  char buf[rps_numlen] = {0};
  char revbuf[rps_numlen] = {0};
  intptr_t argi= i;
  int p=0;
  if (i==0)
    return std::string("0");
  RPS_DELETED_BREAKPOINT();
  memset (buf, 0, sizeof(buf));
  memset (revbuf, 0, sizeof(revbuf));
  if (i<0)
    i = -i;
  //RPS_UNIQUE_BREAKPOINT();
  while (i>0)
    {
      RPS_ASSERT(p>=0 && p<rps_numlen);
      revbuf[p++] = '0' + (i%10);
      i = i / 10;
    };
  //RPS_UNIQUE_BREAKPOINT();
  if (neg)
    revbuf[p++] = '-';
  RPS_ASSERT(p<rps_numlen-1 && p>=0);
  //RPS_UNIQUE_BREAKPOINT();
  for (int j=p-1; j>=0; j--)
    buf[p-1-j] = revbuf[j];
  //RPS_UNIQUE_BREAKPOINT();
  RPS_ASSERT(buf[0] != (char)0 && strlen(buf)<rps_numlen);
  RPS_ASSERT(argi!=0);
  //RPS_UNIQUE_BREAKPOINT();
  return std::string(buf);
} // end rps_decimal_string


const std::string
rps_hex_string(intptr_t i)
{
  bool neg = (i<0);
  char buf[rps_numlen] = {0};
  char revbuf[rps_numlen] = {0};
  int p=0;
  intptr_t argi= i;
  RPS_DELETED_BREAKPOINT();
  if (i==0)
    return std::string("0");
  memset (buf, 0, sizeof(buf));
  memset (revbuf, 0, sizeof(revbuf));
  if (i<0)
    i = -i;
  //RPS_UNIQUE_BREAKPOINT();
  while (i>0)
    {
      RPS_ASSERT(p>=0 && p<(int)rps_numlen);
      revbuf[p++] = "0123456789abcdef" [i % 16];
      i = i / 16;
    };
  if (neg)
    revbuf[p++] = '-';
  RPS_ASSERT(p<(int)rps_numlen-1 && p>=0);
  //RPS_UNIQUE_BREAKPOINT();
  for (int j=p-1; j>=0; j--)
    buf[p-1-j] = revbuf[j];
  //RPS_UNIQUE_BREAKPOINT();
  RPS_ASSERT(buf[0] != (char)0 && strlen(buf)<(size_t)rps_numlen);
  RPS_ASSERT(argi!=0);
  //RPS_UNIQUE_BREAKPOINT();
  return std::string(buf);
} // end rps_hex_string


const std::string
rps_unsigned_dec_string(uintptr_t i)
{
  char buf[rps_numlen] = {0};
  char revbuf[rps_numlen] = {0};
  int p=0;
  if (i==0)
    return std::string("0");
  memset (buf, 0, sizeof(buf));
  memset (revbuf, 0, sizeof(revbuf));
  while (i>0)
    {
      RPS_ASSERT(p>=0 && p<rps_numlen);
      revbuf[p++] = '0' + (i%10);
      i = i / 10;
    };
  RPS_ASSERT(p<rps_numlen-1 && p>=0);
  for (int j=p-1; j>=0; j--)
    buf[p-1-j] = revbuf[j];
  RPS_ASSERT(buf[0] != (char)0 && strlen(buf)<rps_numlen);
  return std::string(buf);
} // end rps_unsigned_dec_string


const std::string
rps_unsigned_hex_string(uintptr_t i)
{
  char buf[rps_numlen] = {0};
  char revbuf[rps_numlen] = {0};
  int p=0;
  if (i==0)
    return std::string("0");
  memset (buf, 0, sizeof(buf));
  memset (revbuf, 0, sizeof(revbuf));
  while (i>0)
    {
      RPS_ASSERT(p>=0 && p<(int)rps_numlen);
      revbuf[p++] = "0123456789abcdef" [i % 16];
      i = i / 16;
    };
  RPS_ASSERT(p<(int)rps_numlen-1 && p>=0);
  for (int j=p-1; j>=0; j--)
    buf[p-1-j] = revbuf[j];
  RPS_ASSERT(buf[0] != (char)0 && strlen(buf)<(size_t)rps_numlen);
  return std::string(buf);
} // end rps_unsigned_hex_string


/// called to give the interactive plugin
void
rps_util_interactive_plugin(const char*arg)
{
  rps_interact_dlh = dlopen(arg, RTLD_GLOBAL|RTLD_NOW);
  if (!rps_interact_dlh)
    RPS_FATALOUT("fail to open interactive plugin "
                 << Rps_QuotedC_String(arg)
                 << " : " << dlerror());
  void*iad = dlsym(rps_interact_dlh, RPS_INTERACTIVE_PLUGIN_INIT_NAME);
  if (!iad)
    RPS_FATALOUT("interactive plugin "
                 << Rps_QuotedC_String(arg)
                 << " without mandatory "
                 << RPS_INTERACTIVE_PLUGIN_INIT_NAME
                 << " : "<< dlerror());
  rps_interactive_plugin_init_sig_t*ifun
    = (rps_interactive_plugin_init_sig_t*)iad;
  (*ifun)(rps_interact_arg);
} // end rps_util_interactive_plugin


/// called to give the optional argument to the interactive plugin
void
rps_util_arg_interact_plugin(const char*arg)
{
  RPS_ASSERT(rps_interact_arg == nullptr);
  rps_interact_arg = arg;
} // end rps_util_arg_interact_plugin

const char*
rps_strstr(const char*haystack, const char *needle)
{
  if (RPS_LIKELY(haystack != nullptr && needle != nullptr))
    return strstr(haystack, needle);
  return nullptr;
} // end rps_strstr

const char*
rps_strchr(const char*s, int c)
{
  if (RPS_LIKELY(s != nullptr))
    return strchr(s,c);
  return nullptr;
} // end rps_strchr

/************************/
#pragma message "may need to define output of more vectors (of objects, values, ...) and indented output"
//// end of file utilities_rps.cc
