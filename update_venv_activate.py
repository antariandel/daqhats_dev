#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script updates the activate script of a virtual environment to
# set additional environment variables.
# LD_LIBRARY_PATH will be set to $VIRTUAL_ENV/lib
# C_INCLUDE_PATH will be set to $VIRTUAL_ENV/include

import os


venv_path = os.environ['VIRTUAL_ENV']
if not venv_path:
	print('Must run from within a virtual environment!')
	sys.exit(1)

activate_bash_string = """# This file must be used with "source bin/activate" *from bash*
# you cannot run it directly

deactivate () {
    unset -f pydoc >/dev/null 2>&1

    # reset old environment variables
    # ! [ -z ${VAR+_} ] returns true if VAR is declared at all
    if ! [ -z "${_OLD_VIRTUAL_PATH+_}" ] ; then
        PATH="$_OLD_VIRTUAL_PATH"
        export PATH
        unset _OLD_VIRTUAL_PATH
	LD_LIBRARY_PATH=$_OLD_LD_LIBRARY_PATH
	export LD_LIBRARY_PATH
	unset _OLD_LD_LIBRARY_PATH
	C_INCLUDE_PATH=$_OLD_C_INCLUDE_PATH
	export C_INCLUDE_PATH
	unset _OLD_C_INCLUDE_PATH
    fi
    if ! [ -z "${_OLD_VIRTUAL_PYTHONHOME+_}" ] ; then
        PYTHONHOME="$_OLD_VIRTUAL_PYTHONHOME"
        export PYTHONHOME
        unset _OLD_VIRTUAL_PYTHONHOME
    fi

    # This should detect bash and zsh, which have a hash command that must
    # be called to get it to forget past commands.  Without forgetting
    # past commands the $PATH changes we made may not be respected
    if [ -n "${BASH-}" ] || [ -n "${ZSH_VERSION-}" ] ; then
        hash -r 2>/dev/null
    fi

    if ! [ -z "${_OLD_VIRTUAL_PS1+_}" ] ; then
        PS1="$_OLD_VIRTUAL_PS1"
        export PS1
        unset _OLD_VIRTUAL_PS1
    fi

    unset VIRTUAL_ENV
    if [ ! "${1-}" = "nondestructive" ] ; then
    # Self destruct!
        unset -f deactivate
    fi
}

# unset irrelevant variables
deactivate nondestructive

VIRTUAL_ENV="##VENVPATH##"
export VIRTUAL_ENV

_OLD_VIRTUAL_PATH="$PATH"
PATH="$VIRTUAL_ENV/bin:$PATH"
export PATH

# unset PYTHONHOME if set
if ! [ -z "${PYTHONHOME+_}" ] ; then
    _OLD_VIRTUAL_PYTHONHOME="$PYTHONHOME"
    unset PYTHONHOME
fi

if [ -z "${VIRTUAL_ENV_DISABLE_PROMPT-}" ] ; then
    _OLD_VIRTUAL_PS1="$PS1"
    if [ "x" != x ] ; then
        PS1="$PS1"
    else
        PS1="(`basename \"$VIRTUAL_ENV\"`) $PS1"
    fi
    export PS1
fi

# Make sure to unalias pydoc if it's already there
alias pydoc 2>/dev/null >/dev/null && unalias pydoc

pydoc () {
    python -m pydoc "$@"
}

# This should detect bash and zsh, which have a hash command that must
# be called to get it to forget past commands.  Without forgetting
# past commands the $PATH changes we made may not be respected
if [ -n "${BASH-}" ] || [ -n "${ZSH_VERSION-}" ] ; then
    hash -r 2>/dev/null
fi

_OLD_LD_LIBRARY_PATH=$LD_LIBRARY_PATH
export _OLD_LD_LIBRARY_PATH
LD_LIBRARY_PATH=$VIRTUAL_ENV/lib
export LD_LIBRARY_PATH

_OLD_C_INCLUDE_PATH=$C_INCLUDE_PATH
export _OLD_C_INCLUDE_PATH
C_INCLUDE_PATH=$VIRTUAL_ENV/include
export C_INCLUDE_PATH""".replace('##VENVPATH##', venv_path)

activate_bash_filepath = os.path.join(venv_path, 'bin', 'activate')

with open(activate_bash_filepath, 'w') as f:
	f.write(activate_bash_string)

print('{0} is updated.\nRun "source {0}" again to effectuate.'.format(activate_bash_filepath))
