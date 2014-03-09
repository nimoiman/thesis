"""Module with function decorator to register functions as commandline
commands and to generate a commandline help text.

Adapted from @uniphil's code for Qcumber API
https://github.com/Queens-Hacks/qcumber-api/blob/master/manage.py

Usage:
.. code-block::
    from api_docs import command, parse_args, help

    def my_func(arg_1, arg_2):
        \"\"\"does something cool\"\"\"
        return int(arg_1) ^ int(arg_2)

    if __name__ == '__main__':
        parse_args()
"""

from functools import wraps
import sys
import inspect
import __main__

def command(func, _funcs={}):
    """Decorate functions with this to register them as commands"""

    # register the command
    func_name = func.__name__.lower()
    if func_name in _funcs:
        raise Exception('Duplicate definition for command {}'.format(func_name))
    _funcs[func_name] = func

    # play nice and leave the command where it was in this script
    @wraps(func)
    def wrapped(*args):
        return func(*args)
    return wrapped

def parse_args():
    """Get the command, or run 'help' if no command is provided"""
    if len(sys.argv) < 2:
        cmd, args = 'help', []
    else:
        cmd, args = sys.argv[1].lower(), sys.argv[2:]

    # Map the command to a function, falling back to 'help' if it's not found
    funcs = command.__defaults__[0]  # _funcs={}
    if cmd not in funcs:
        print('Command "{}" not found :('.format(cmd))
        cmd, args = 'help', []

    # do it!
    try:
        output = funcs[cmd](*args)
        # (and print the output)
        if output:
            print(output)
    except TypeError as e:
        help(cmd)
        raise e


def _signature(name, func):
    """Return string repr of function signature"""
    defaults = inspect.getargspec(func).defaults or []
    args = inspect.getargspec(func).args or []
    arg_str_list = []

    n_positional_args = len(args) - len(defaults)

    for i in range(n_positional_args):
        # positional arguments
        arg_str_list.append('<' + args[i] + '>')

    for i in range(len(defaults)):
        # optional arguments
        arg_str_list.append('[' + args[i + n_positional_args] + '=' + str(defaults[i]) + ']')

    return '{} {}'.format(name, ' '.join(arg_str_list))

def _format_doc(doc):
    doc_lines = doc.splitlines()
    for i, line in enumerate(doc_lines):
        if i == 0: continue
        if line[:4] == ' ' * len(line[:4]): # starts with 4 spaces
            doc_lines[i] = '    ? ' + line[4:]

    return '\n'.join(doc_lines)

@command
def help(f_name=None):
    """Get usage information about this script"""
    if f_name is not None:
        # Find f_name in available commands
        try:
            func = command.__defaults__[0][f_name]
            print('Usage: {} {} [params]'.format(sys.argv[0], f_name))
            if func.__doc__:
                print('  * {}'.format(_signature(f_name, func)))
                print('    ? {}\n'.format(_format_doc(func.__doc__.strip())))
            else:
                print('  * {}\n'.format(_signature(f_name, func)))
            return
        except KeyError:
            print("Unknown command {}".format(f_name))
    print('Usage: {} [command]'.format(sys.argv[0]))

    # print module help here
    if __main__.__doc__ is not None:
        print(__main__.__doc__.strip())

    print('')
    print('Available commands:')
    for name, func in sorted(command.__defaults__[0].items()):  # _funcs={}

        if func.__doc__:
            print('  * {}'.format(_signature(name, func)))
            print('    ? {}\n'.format(_format_doc(func.__doc__.strip())))
        else:
            print('  * {}\n'.format(_signature(name, func)))
