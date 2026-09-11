// base/comment
#pragma message  "RefPerSys bisonc++ bisonc++.cc"
$insert class.ih

// The FIRST element of SR arrays shown below uses `d_type', defining the
// state's type, and `d_lastIdx' containing the last element's index. If
// d_lastIdx contains the REQ_TOKEN bitflag (see below) then the state needs
// a token: if in this state d_token is Reserved_::UNDETERMINED_, nextToken() will be
// called

// The LAST element of SR arrays uses `d_token' containing the last retrieved
// token to speed up the (linear) seach.  Except for the first element of SR
// arrays, the field `d_action' is used to determine what to do next. If
// positive, it represents the next state (used with SHIFT); if zero, it
// indicates `ACCEPT', if negative, -d_action represents the number of the
// rule to reduce to.

// `lookup()' tries to find d_token in the current SR array. If it fails, and
// there is no default reduction UNEXPECTED_TOKEN_ is thrown, which is then
// caught by the error-recovery function.

// The error-recovery function will pop elements off the stack until a state
// having bit flag ERR_ITEM is found. This state has a transition on errTok_
// which is applied. In this errTok_ state, while the current token is not a
// proper continuation, new tokens are obtained by nextToken(). If such a
// token is found, error recovery is successful and the token is
// handled according to the error state's SR table and parsing continues.
// During error recovery semantic actions are ignored.

// A state flagged with the DEF_RED flag will perform a default
// reduction if no other continuations are available for the current token.

// The ACCEPT STATE never shows a default reduction: when it is reached the
// parser returns ACCEPT(). During the grammar
// analysis phase a default reduction may have been defined, but it is
// removed during the state-definition phase.

// So:
//      s_x[] = 
//      {
//                  [_field_1_]         [_field_2_]
//
// First element:   {state-type,        idx of last element},
// Other elements:  {required token,    action to perform},
//                                      ( < 0: reduce, 
//                                          0: ACCEPT,
//                                        > 0: next state)
//      }

// base/declarations
$insert debugincludes

namespace // anonymous
{
    char const author[] = "Frank B. Brokken (f.b.brokken@rug.nl)";

    enum Reserved_
    {
        UNDETERMINED_   = -2,
        EOF_            = -1,
        errTok_         = 256
    };
    enum StateType       // modify statetype/data.cc when this enum changes
    {
        NORMAL,
        ERR_ITEM,
        REQ_TOKEN,
        ERR_REQ,    // ERR_ITEM | REQ_TOKEN
        DEF_RED,    // state having default reduction
        ERR_DEF,    // ERR_ITEM | DEF_RED
        REQ_DEF,    // REQ_TOKEN | DEF_RED
        ERR_REQ_DEF // ERR_ITEM | REQ_TOKEN | DEF_RED
    };    
    inline bool operator&(StateType lhs, StateType rhs)
    {
        return (static_cast<int>(lhs) & rhs) != 0;
    }
    enum StateTransition
    {
        ACCEPT_   = 0,     // `ACCEPT' TRANSITION
    };

    struct PI_     // Production Info
    {
        size_t d_nonTerm; // identification number of this production's
                            // non-terminal 
        size_t d_size;    // number of elements in this production 
    };

    struct SR_     // Shift Reduce info, see its description above
    {
        union
        {
            int _field_1_;      // initializer, allowing initializations 
                                // of the SR s_[] arrays
            StateType d_type;
            int       d_token;
        };
        union
        {
            int _field_2_;

            int d_lastIdx;          // if negative, the state uses SHIFT
            int d_action;           // may be negative (reduce), 
                                    // postive (shift), or 0 (accept)
        };
    };

$insert 4 staticdata 

$insert namespace-open

$insert polymorphicCode

// If the parsing function call (i.e., parse()' needs arguments, then provide
// an overloaded function.  The code below doesn't rely on parameters, so no
// arguments are required.  Furthermore, parse uses a function try block to
// allow us to do ACCEPT and ABORT from anywhere, even from within members
// called by actions, simply throwing the appropriate exceptions.


// base/base1
\@Base::\@Base()
:
    d_token(Reserved_::UNDETERMINED_),
$insert 4 baseclasscode
}

// base/clearin
void \@Base::clearin_()
{
    d_nErrors_ = 0;
    d_stackIdx = -1;
    d_stateStack.clear();
$insert 4 LTYPEclear
    d_token = Reserved_::UNDETERMINED_;
    d_next = TokenPair{ Reserved_::UNDETERMINED_, STYPE_{} };
    d_recovery = false;
    d_acceptedTokens_ = d_requiredTokens_;
    d_val_ = STYPE_{};

    push_(0);
}

// base/debugfunctions
$insert debugfunctions

void \@Base::setDebug(bool mode)
{
    d_actionCases_ = false;
    d_debug_ = mode;
}

void \@Base::setDebug(DebugMode_ mode)
{
    d_actionCases_ = mode & ACTIONCASES;
    d_debug_ =       mode & ON;
}

// base/lex
void \@Base::lex_(int token)
{
    d_token = token;

    if (d_token <= 0)
        d_token = Reserved_::EOF_;

    d_terminalToken = true;
}

// base/lookup
int \@Base::lookup_() const
{
    // if the final transition is negative, then we should reduce by the rule
    // given by its positive value.

    SR_ const *sr = s_state[d_state];
    SR_ const *last = sr + sr->d_lastIdx;

    for ( ; ++sr != last; )           // visit all but the last SR entries
    {
        if (sr->d_token == d_token)
            return sr->d_action;
    }

    if (sr == last)   // reached the last element
    {
        if (sr->d_action < 0)   // default reduction
        {
$insert 12 debug "\nLOOKUP: [" << state_() << ", " << symbol_(d_token) << "] -> default reduce using rule " << -sr->d_action
            return sr->d_action;                
        }

        // No default reduction, so token not found, so error.
        throw UNEXPECTED_TOKEN_;
    }

    // not at the last element: inspect the nature of the action
    // (< 0: reduce, 0: ACCEPT, > 0: shift)

    int action = sr->d_action;

$insert 0 debuglookup

    return action;
}

// base/pop
void \@Base::pop_(size_t count)
{
$insert 4 debug "pop " << count << " elements from the stack having size " << stackSize_()
    if (d_stackIdx < static_cast<int>(count))
    {
$insert 8 debug "Terminating parse(): state stack underflow  at token " << symbol_(d_token)
        ABORT();
    }

    d_stackIdx -= count;
    d_state = d_stateStack[d_stackIdx].first;
    d_vsp = &d_stateStack[d_stackIdx];

$insert 4 LTYPEpop
$insert 4 debug "next: [" << state_() << ", " << symbol_(d_token) << ']' << stype_(". Semantic: ", vs_(0))
}

// base/poptoken
void \@Base::popToken_()
{
    d_token = d_next.first;
    d_val_ = d_token >= 0 ? std::move(d_next.second) : STYPE_{};

    d_next.first = Reserved_::UNDETERMINED_;
}

// base/push
void \@Base::push_(size_t state)
{
    size_t currentSize = d_stateStack.size();
    if (stackSize_() == currentSize)
    {
        size_t newSize = currentSize + STACK_EXPANSION_;
        d_stateStack.resize(newSize);
$insert 8 LTYPEresize
    }

    ++d_stackIdx;
$insert 4 LTYPEpush
    d_stateStack[d_stackIdx] = 
                    StatePair{ d_state = state, std::move(d_val_) };

    d_vsp = &d_stateStack[d_stackIdx];

    if (d_stackIdx == 0)
    {
$insert 4 debug  "\nPUSH 0 (initializing the state stack)"
    }
    else
    {
$insert 4 debug  "\nPUSH:   [" << (d_vsp - 1)->first << ", " << symbol_(d_token) << "] -> " << state << stype_(" (semantic TOS = ", d_val_, ")")
    }
}

// base/pushtoken
void \@Base::pushToken_(int token)
{
    d_next = TokenPair{ d_token, std::move(d_val_) };
    d_token = token;
}

// base/redotoken
void \@Base::redoToken_()
{
    if (d_token != Reserved_::UNDETERMINED_)
        pushToken_(d_token);
}

// base/reduce
void \@Base::reduce_(int rule)
{
    PI_ const &pi = s_productionInfo[rule];

    d_token = pi.d_nonTerm;
$insert 4 debug "rule " << (&pi - s_productionInfo) << ": pop " << pi.d_size << " elements. Next will be: [" << d_stateStack[d_stackIdx - pi.d_size].first << ", " << symbol_(d_token) << ']'
    pop_(pi.d_size);

    d_terminalToken = false;
}

// base/shift
void \@Base::shift_(int action)
{
    push_(action);
    popToken_();               // token processed

    if (d_recovery and d_terminalToken)
    {
$insert 8 debug "ERROR RECOVERED: next state " << action
        d_recovery = false;
        d_acceptedTokens_ = 0;
    }
}

// base/startrecovery
void \@Base::startRecovery_()
{
    int lastToken = d_token;                // give the unexpected token a
                                            // chance to be processed
                                            // again.

    pushToken_(Reserved_::errTok_);      // specify errTok_ as next token
    push_(lookup_());                     // push the error state

    d_token = lastToken;                    // reactivate the unexpected
                                            // token (we're now in an
                                            // ERROR state).

    d_recovery = true;
}

// base/top
inline size_t \@Base::top_() const
{
    return d_stateStack[d_stackIdx].first;
}

// derived/errorrecovery
void \@::errorRecovery_()
{
    // When an error has occurred, pop elements off the stack until the top
    // state has an error-item. If none is found, the default recovery
    // mode (which is to abort) is activated. 
    //
    // If EOF is encountered without being appropriate for the current state,
    // then the error recovery will fall back to the default recovery mode.
    // (i.e., parsing terminates)

$insert 4 debug "\nERROR:  [" << top_() << ", " << symbol_(token_()) << "] -> ??. Errors: " << (d_nErrors_ + 1)


    if (d_acceptedTokens_ >= d_requiredTokens_)// only generate an error-
    {                                           // message if enough tokens 
        ++d_nErrors_;                          // were accepted. Otherwise
        error();                                // simply skip input
$insert 8 errorverbose
    }

    // get the error state
    while (not (s_state[top_()][0].d_type & ERR_ITEM))
    {
$insert 8 debug "pop state: " << top_() << " (not an ERROR state)"
        pop_();
    }
$insert 4 debug "Reached ERROR state " << top_()

    // In the error state, looking up a token allows us to proceed.
    // Continuation may be require multiple reductions, but eventually a
    // terminal-token shift is used. See nextCycle_ for details.

    startRecovery_();
}

// derived/executeaction
void \@::executeAction_(int production)
try
{
    if (token_() != Reserved_::UNDETERMINED_)
        pushToken_(token_());     // save an already available token
$insert 4 debug "execute action " << production << " ..."
$insert executeactioncases
    switch (production)
    {
$insert 8 actioncases
    }
$insert 4 debug "... completed" << stype_(", semantic: ", vs_(0))
}
catch (std::exception const &exc)
{
    exceptionHandler(exc);
}

// derived/nextcycle
void \@::nextCycle_()
try
{
    if (s_state[state_()]->d_type & REQ_TOKEN)
        nextToken_();              // obtain next token


    int action = lookup_();        // lookup d_token in d_state

    if (action > 0)                 // SHIFT: push a new state
    {
        shift_(action);
        return;
    }

    if (action < 0)            // REDUCE: execute and pop.
    {
$insert 8 debug "\nREDUCE: rule " << -action

        if (recovery_())
            redoToken_();
        else
            executeAction_(-action);
                                            // next token is the rule's LHS
        reduce_(-action); 
        return;
    }

    if (recovery_())
        ABORT();
    else 
        ACCEPT();
}
catch (ErrorRecovery_)
{
    if (not recovery_())
        errorRecovery_();
    else
    {
        if (token_() == Reserved_::EOF_)
            ABORT();
        popToken_();               // skip the failing token
    }
}


// derived/nexttoken
void \@::nextToken_()
{ 
    // If d_token is Reserved_::UNDETERMINED_ then if savedToken_() is
    // Reserved_::UNDETERMINED_ another token is obtained from lex(). Then
    // savedToken_() is assigned to d_token.

                                    // no need for a token: got one already
    if (token_() != Reserved_::UNDETERMINED_) 
    {
$insert 8 debug "available token " << symbol_(token_())
        return;                             
    }

    if (savedToken_() != Reserved_::UNDETERMINED_)
    {
        popToken_();               // consume pending token
$insert 8 debug "retrieved token " << symbol_(token_()) << stype_(", semantic = ", d_val_)
    }
    else
    {
        ++d_acceptedTokens_;       // accept another token (see
                                    // errorRecover())
        lex_(lex());
        print_();
$insert 8 debug "scanner token " << symbol_(token_())
    }
    print();
}

// derived/print
void \@::print_()
{
$insert print
}

// derived/parse
int \@::parse()
try 
{
    // The parsing algorithm:
    // Initially, state 0 is pushed on the stack, and all relevant variables
    // are initialized by Base::clearin_.
    //
    // Then, in an eternal loop:
    //
    //  1. If a state is a REQ_TOKEN type, then the next token is obtained
    //     from nextToken().  This may very well be the currently available
    //     token. When retrieving a terminal token d_terminal is set to true.
    //
    //  2. lookup() is called, d_token is looked up in the current state's
    //     SR_ array.
    //
    //  4. Depending on the result of the lookup() function the next state is
    //     shifted on the parser's stack, a reduction by some rule is applied,
    //     or the parsing function returns ACCEPT(). When a reduction is
    //     called for, any action that may have been defined for that
    //     reduction is executed.
    //
    //  5. An error occurs if d_token is not found, and the state has no
    //     default reduction.

$insert 4 debug "parse(): Parsing starts"
    clearin_();                            // initialize, push(0)

    while (true)
    {
$insert prompt
        nextCycle_();
    }
}
catch (Return_ retValue)
{
$insert 4 debug "parse(): returns " << retValue << " or " << d_nErrors_ 
    return retValue or d_nErrors_;
}


// derived/tail

$insert namespace-close

