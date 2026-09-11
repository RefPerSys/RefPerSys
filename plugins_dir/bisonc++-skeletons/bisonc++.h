#ifndef \@$_h_included
#define \@$_h_included
#pragma message  "RefPerSys bisonc++ bisonc++.h"
$insert baseclass
$insert scanner.h

$insert namespace-open

$insert undefparser

class \@: public \@Base
{
$insert 4 scannerobject
        
    public:
        \@() = default;
        int parse();

    private:
        void error();                   // called on (syntax) errors
        int lex();                      // returns the next token from the
                                        // lexical scanner. 
        void print();                   // use, e.g., d_token, d_loc
        void exceptionHandler(std::exception const &exc);

    // support functions for parse():
        void executeAction_(int ruleNr);
        void errorRecovery_();
        void nextCycle_();
        void nextToken_();
        void print_();
};

$insert namespace-close

#endif
