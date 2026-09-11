// hdr/includes
#ifndef \@$Base_h_included
#define \@$Base_h_included
#pragma message "RefPerSys bisonc++ bisonc++base.h"

#include <exception>
#include <vector>
#include <iostream>
$insert polyincludes
$insert preincludes
$insert debugincludes

// hdr/baseclass

namespace // anonymous
{
    struct PI_;
}

$insert namespace-open

$insert polymorphic

$insert parserbase
{
    public:
        enum DebugMode_
        {
            OFF           = 0,
            ON            = 1 << 0,
            ACTIONCASES   = 1 << 1
        };

$insert tokens
$insert LTYPE
$insert STYPE

    private:
                        // state  semval
        using StatePair = std::pair<size_t, STYPE_>;
                       // token   semval
        using TokenPair = std::pair<int,    STYPE_>;

        int d_stackIdx = -1;
        std::vector<StatePair> d_stateStack;
        StatePair  *d_vsp = 0;       // points to the topmost value stack
        size_t      d_state = 0;

        TokenPair   d_next;
        int         d_token;

        bool        d_terminalToken = false;
        bool        d_recovery = false;

$insert LTYPEstack

    protected:
        enum Return_
        {
            PARSE_ACCEPT_ = 0,   // values used as parse()'s return values
            PARSE_ABORT_  = 1
        };
        enum ErrorRecovery_
        {
            UNEXPECTED_TOKEN_,
        };

        bool        d_actionCases_ = false;    // set by options/directives
        bool        d_debug_ = true;
        size_t      d_requiredTokens_;
        size_t      d_nErrors_;                // initialized by clearin()
        size_t      d_acceptedTokens_;
        STYPE_     d_val_;

$insert LTYPEdata

        \@Base();

$insert debugdecl
        void ABORT() const;
        void ACCEPT() const;
        void ERROR() const;

        STYPE_ &vs_(int idx);             // value stack element idx 
        int  lookup_() const;
        int  savedToken_() const;
        int  token_() const;
        size_t stackSize_() const;
        size_t state_() const;
        size_t top_() const;
        void clearin_();
        void errorVerbose_();
        void lex_(int token);
        void popToken_();
        void pop_(size_t count = 1);
        void pushToken_(int token);
        void push_(size_t nextState);
        void redoToken_();
        bool recovery_() const;
        void reduce_(int rule);
        void shift_(int state);
        void startRecovery_();

    public:
        void setDebug(bool mode);
        void setDebug(DebugMode_ mode);
}; 

// hdr/abort
inline void \@Base::ABORT() const
{
$insert 4 debug "ABORT(): Parsing unsuccessful"
    throw PARSE_ABORT_;
}

// hdr/accept
inline void \@Base::ACCEPT() const
{
$insert 4 debug "ACCEPT(): Parsing successful"
    throw PARSE_ACCEPT_;
}


// hdr/error
inline void \@Base::ERROR() const
{
$insert 4 debug "ERROR(): Forced error condition"
    throw UNEXPECTED_TOKEN_;
}

// hdr/savedtoken
inline int \@Base::savedToken_() const
{
    return d_next.first;
}

// hdr/opbitand
inline \@Base::DebugMode_ operator&(\@Base::DebugMode_ lhs,
                                     \@Base::DebugMode_ rhs)
{
    return static_cast<\@Base::DebugMode_>(
            static_cast<int>(lhs) & rhs);
}

// hdr/opbitor
inline \@Base::DebugMode_ operator|(\@Base::DebugMode_ lhs, 
                                     \@Base::DebugMode_ rhs)
{
    return static_cast<\@Base::DebugMode_>(static_cast<int>(lhs) | rhs);
};

// hdr/recovery
inline bool \@Base::recovery_() const
{
    return d_recovery;
}

// hdr/stacksize
inline size_t \@Base::stackSize_() const
{
    return d_stackIdx + 1;
}

// hdr/state
inline size_t \@Base::state_() const
{
    return d_state;
}

// hdr/token
inline int \@Base::token_() const
{
    return d_token;
}

// hdr/vs
inline \@Base::STYPE_ &\@Base::vs_(int idx) 
{
    return (d_vsp + idx)->second;
}

$insert notokens

$insert namespace-close

#endif



