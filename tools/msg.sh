msg_ok()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[0;32m OK \033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1"
}


msg_info()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[0;34mINFO\033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1"
}


msg_warn()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[1;33mWARN\033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1"
}


msg_fail()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf                                                          \
            '[\033[1;31mFAIL\033[0m] \033[0;35m%s\033[0m: %s...\n'      \
            "$ts"                                                       \
            "$1"                                                        \
            2>&1

        exit 1
}

