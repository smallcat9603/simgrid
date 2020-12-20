#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2014-2020. The SimGrid Team. All rights reserved.

# This program is free software; you can redistribute it and/or modify it
# under the terms of the license (GNU LGPL) which comes with this package.

import re
import glob
import sys

class Arg(object):

    def __init__(self, name, thetype):
        self.name = name
        self.type = thetype

    def field(self):
        return self.simcall_types[self.type]

    def rettype(self):
        return self.type


class Simcall(object):
    simcalls_body = None
    simcalls_pre = None

    def __init__(self, name, handler, res, args, call_kind):
        self.name = name
        self.res = res
        self.args = args
        self.need_handler = handler
        self.call_kind = call_kind

    def check(self):
        # libsmx.c  simcall_BODY_
        if self.simcalls_body is None:
            f = open('libsmx.cpp')
            self.simcalls_body = set(re.findall(r'simcall_BODY_(.*?)\(', f.read()))
            f.close()
        if self.name not in self.simcalls_body:
            print ('# ERROR: No function calling simcall_BODY_%s' % self.name)
            print ('# Add something like this to libsmx.c:')
            print ('%s simcall_%s(%s)' % (self.res.rettype(), self.name, ', '.
                                          join('%s %s' % (arg.rettype(), arg.name) for arg in self.args)))
            print ('{')
            print ('  return simcall_BODY_%s(%s);' % (self.name, "..."))
            print ('}')
            return False

        # smx_*.c void simcall_HANDLER_host_on(smx_simcall_t simcall,
        # smx_host_t h)
        if self.simcalls_pre is None:
            self.simcalls_pre = set()
            for fn in glob.glob('smx_*') + glob.glob('../kernel/actor/ActorImpl*') + \
                    glob.glob('../mc/*cpp') + glob.glob('../kernel/activity/*cpp'):
                f = open(fn)
                self.simcalls_pre |= set(re.findall(r'simcall_HANDLER_(.*?)\(', f.read()))
                f.close()
        if self.need_handler:
            if self.name not in self.simcalls_pre:
                print ('# ERROR: No function called simcall_HANDLER_%s' % self.name)
                print ('# Add something like this to the relevant C file (like smx_io.c if it\'s an IO call):')
                print ('%s simcall_HANDLER_%s(smx_simcall_t simcall%s)' % (self.res.rettype(), self.name, ''.
                                                                           join(', %s %s' % (arg.rettype(), arg.name)for arg in self.args)))
                print ('{')
                print ('  // Your code handling the simcall')
                print ('}')
                return False
        else:
            if self.name in self.simcalls_pre:
                print (
                    '# ERROR: You have a function called simcall_HANDLER_%s, but that simcall is not using any handler' %
                    self.name)
                print ('# Either change your simcall definition, or kill that function')
                return False
        return True

    def enum(self):
        return '  %s,' % (self.name.upper())

    def string(self):
        return '    "Simcall::%s",' % self.name.upper()

    def accessors(self):
        res = []
        res.append('')
        regex = re.compile(r"^boost::intrusive_ptr<(.*?)>(.*)$")  # to compute the raw type
        # Arguments getter/setters
        for i in range(len(self.args)):
            arg = self.args[i]
            rawtype = regex.sub(r'\1*\2', arg.rettype())
            res.append('static inline %s simcall_%s__get__%s(smx_simcall_t simcall)' % (
                arg.rettype(), self.name, arg.name))
            res.append('{')
            res.append('  return simgrid::simix::unmarshal<%s>(simcall->args_[%i]);' % (arg.rettype(), i))
            res.append('}')
            res.append('static inline %s simcall_%s__getraw__%s(smx_simcall_t simcall)' % (
                rawtype, self.name, arg.name))
            res.append('{')
            res.append('  return simgrid::simix::unmarshal_raw<%s>(simcall->args_[%i]);' % (rawtype, i))
            res.append('}')
            res.append('static inline void simcall_%s__set__%s(smx_simcall_t simcall, %s arg)' % (
                self.name, arg.name, arg.rettype()))
            res.append('{')
            res.append('  simgrid::simix::marshal<%s>(simcall->args_[%i], arg);' % (arg.rettype(), i))
            res.append('}')

        # Return value getter/setters
        if self.res.type != 'void':
            rawtype = regex.sub(r'\1*\2', self.res.rettype())
            res.append(
                'static inline %s simcall_%s__get__result(smx_simcall_t simcall)' % (self.res.rettype(), self.name))
            res.append('{')
            res.append('  return simgrid::simix::unmarshal<%s>(simcall->result_);' % self.res.rettype())
            res.append('}')
            res.append('static inline %s simcall_%s__getraw__result(smx_simcall_t simcall)' % (rawtype, self.name))
            res.append('{')
            res.append('  return simgrid::simix::unmarshal_raw<%s>(simcall->result_);' % rawtype)
            res.append('}')
            res.append(
                'static inline void simcall_%s__set__result(smx_simcall_t simcall, %s result)' % (self.name, self.res.rettype()))
            res.append('{')
            res.append('  simgrid::simix::marshal<%s>(simcall->result_, result);' % (self.res.rettype()))
            res.append('}')
        return '\n'.join(res)

    def case(self):
        res = []
        indent = '    '
        args = ["simgrid::simix::unmarshal<%s>(simcall_.args_[%d])" % (arg.rettype(), i)
                for i, arg in enumerate(self.args)]
        res.append(indent + 'case Simcall::%s:' % (self.name.upper()))
        if self.need_handler:
            call = "simcall_HANDLER_%s(&simcall_%s%s)" % (self.name,
                                                        ", " if args else "",
                                                        ', '.join(args))
        else:
            call = "SIMIX_%s(%s)" % (self.name, ', '.join(args))
        if self.call_kind == 'Func':
            res.append(indent + "  simgrid::simix::marshal<%s>(simcall_.result_, %s);" % (self.res.rettype(), call))
        else:
            res.append(indent + "  " + call + ";")
        if self.call_kind != 'Blck':
            res.append(indent + '  simcall_answer();')
        res.append(indent + '  break;')
        res.append('')
        return '\n'.join(res)

    def body(self):
        res = ['']
        res.append(
            'inline static %s simcall_BODY_%s(%s)' % (self.res.rettype(),
                                                      self.name,
                                                      ', '.join('%s %s' % (arg.rettype(), arg.name) for arg in self.args)))
        res.append('{')
        res.append('  if (false) /* Go to that function to follow the code flow through the simcall barrier */')
        if self.need_handler:
            res.append('    simcall_HANDLER_%s(%s);' % (self.name,
                                                        ', '.join(["&SIMIX_process_self()->simcall_"] + [arg.name for arg in self.args])))
        else:
            res.append('    SIMIX_%s(%s);' % (self.name,
                                              ', '.join(arg.name for arg in self.args)))
        res.append('  return simcall<%s%s>(Simcall::%s%s);' % (
            self.res.rettype(),
            "".join([", " + arg.rettype() for i, arg in enumerate(self.args)]),
            self.name.upper(),
            "".join([", " + arg.name for i, arg in enumerate(self.args)])
        ))
        res.append('}')
        return '\n'.join(res)

    def handler_prototype(self):
        if self.need_handler:
            return "XBT_PRIVATE %s simcall_HANDLER_%s(smx_simcall_t simcall%s);" % (self.res.rettype() if self.call_kind == 'Func' else 'void',
                                                                                    self.name,
                                                                                    ''.join(', %s %s' % (arg.rettype(), arg.name)
                                                                                            for i, arg in enumerate(self.args)))
        return ""


def parse(fn):
    simcalls = []
    resdi = None
    simcalls_guarded = {}
    for line in open(fn).read().split('\n'):
        if line.startswith('##'):
            resdi = []
            simcalls_guarded[re.search(r'## *(.*)', line).group(1)] = resdi
        if line.startswith('#') or not line:
            continue
        match = re.match(
            r'^(\S+)\s+([^\)\(\s]+)\s*\(*(.*)\)\s*(\[\[.*\]\])?\s*;\s*?$', line)
        if not match:
            raise AssertionError(line)
        ret, name, args, attrs = match.groups()
        sargs = []
        if not re.match(r"^\s*$", args):
            for arg in re.split(",", args):
                args = args.strip()
                match = re.match(r"^(.*?)\s*?(\S+)$", arg)
                t, n = match.groups()
                t = t.strip()
                n = n.strip()
                sargs.append(Arg(n, t))
        if ret == "void":
            ans = "Proc"
        else:
            ans = "Func"
        handler = True
        if attrs:
            attrs = attrs[2:-2]
            for attr in re.split(",", attrs):
                if attr == "block":
                    ans = "Blck"
                elif attr == "nohandler":
                    handler = False
                else:
                    raise AssertionError("Unknown attribute %s in: %s" % (attr, line))
        sim = Simcall(name, handler, Arg('result', ret), sargs, ans)
        if resdi is None:
            simcalls.append(sim)
        else:
            resdi.append(sim)
    return simcalls, simcalls_guarded


def header(name):
    fd = open(name, 'w')
    fd.write(
        '/**********************************************************************/\n')
    fd.write(
        '/* File generated by src/simix/simcalls.py from src/simix/simcalls.in */\n')
    fd.write(
        '/*                                                                    */\n')
    fd.write(
        '/*                    DO NOT EVER CHANGE THIS FILE                    */\n')
    fd.write(
        '/*                                                                    */\n')
    fd.write(
        '/* change simcalls specification in src/simix/simcalls.in             */\n')
    fd.write(
        '/* Copyright (c) 2014-2020. The SimGrid Team. All rights reserved.    */\n')
    fd.write(
        '/**********************************************************************/\n\n')
    fd.write('/*\n')
    fd.write(
        ' * Note that the name comes from http://en.wikipedia.org/wiki/Popping\n')
    fd.write(
        ' * Indeed, the control flow is doing a strange dance in there.\n')
    fd.write(' *\n')
    fd.write(
        ' * That\'s not about http://en.wikipedia.org/wiki/Poop, despite the odor :)\n')
    fd.write(' */\n\n')
    return fd


def handle(fd, func, simcalls, guarded_simcalls):
    def nonempty(e):
        return e != ''
    fd.write('\n'.join(filter(nonempty, (func(simcall) for simcall in simcalls))))

    for guard, ll in guarded_simcalls.items():
        fd.write('\n#if %s\n' % (guard))
        fd.write('\n'.join(func(simcall) for simcall in ll))
        fd.write('\n#endif')

    fd.write('\n')

if __name__ == '__main__':
    simcalls, simcalls_dict = parse('simcalls.in')

    ok = True
    ok &= all(map(Simcall.check, simcalls))
    for k, v in simcalls_dict.items():
        ok &= all(map(Simcall.check, v))
    if not ok:
      print ("Some checks fail!")
      sys.exit(1)

    #
    # popping_accessors.hpp
    #
    fd = header('popping_accessors.hpp')
    fd.write('#include "src/simix/popping_private.hpp"')
    handle(fd, Simcall.accessors, simcalls, simcalls_dict)
    fd.write(
        "\n/* The prototype of all simcall handlers, automatically generated for you */\n\n")
    handle(fd, Simcall.handler_prototype, simcalls, simcalls_dict)
    fd.close()

    #
    # popping_enum.hpp
    #
    fd = header("popping_enum.hpp")
    fd.write('namespace simgrid {\n')
    fd.write('namespace simix {\n')
    fd.write('/**\n')
    fd.write(' * @brief All possible simcalls.\n')
    fd.write(' */\n')
    fd.write('enum class Simcall {\n')
    fd.write('  NONE,\n')

    handle(fd, Simcall.enum, simcalls, simcalls_dict)

    fd.write('};\n')
    fd.write('\n')
    fd.write('constexpr int NUM_SIMCALLS = ' + str(1 + len(simcalls)) + ';\n')
    fd.write('} // namespace simix\n')
    fd.write('} // namespace simgrid\n')
    fd.close()

    #
    # popping_generated.cpp
    #

    fd = header("popping_generated.cpp")

    fd.write('#include "smx_private.hpp"\n')
    fd.write('#include <simgrid/host.h>\n')
    fd.write('#include <xbt/base.h>\n')
    fd.write('#if SIMGRID_HAVE_MC\n')
    fd.write('#include "src/mc/mc_forward.hpp"\n')
    fd.write('#endif\n')
    fd.write('#include "src/kernel/activity/ConditionVariableImpl.hpp"\n')

    fd.write('\n')
    fd.write('XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(simix_popping);\n\n')

    fd.write('using simgrid::simix::Simcall;')
    fd.write('\n')
    fd.write(
        '/** @brief Simcalls\' names (generated from src/simix/simcalls.in) */\n')
    fd.write('constexpr std::array<const char*, simgrid::simix::NUM_SIMCALLS> simcall_names{{\n')

    fd.write('    "Simcall::NONE",\n')
    handle(fd, Simcall.string, simcalls, simcalls_dict)

    fd.write('}};\n\n')

    fd.write('/** @private\n')
    fd.write(
        ' * @brief (in kernel mode) unpack the simcall and activate the handler\n')
    fd.write(' *\n')
    fd.write(' * This function is generated from src/simix/simcalls.in\n')
    fd.write(' */\n')
    fd.write(
        'void simgrid::kernel::actor::ActorImpl::simcall_handle(int value) {\n')
    fd.write(
        '  XBT_DEBUG("Handling simcall %p: %s", &simcall_, SIMIX_simcall_name(simcall_.call_));\n')
    fd.write('  SIMCALL_SET_MC_VALUE(simcall_, value);\n')
    fd.write(
        '  if (context_->wannadie())\n')
    fd.write('    return;\n')
    fd.write('  switch (simcall_.call_) {\n')

    handle(fd, Simcall.case, simcalls, simcalls_dict)

    fd.write('    case Simcall::NONE:\n')
    fd.write('      throw std::invalid_argument(simgrid::xbt::string_printf("Asked to do the noop syscall on %s@%s",\n')
    fd.write('                                                              get_cname(),\n')
    fd.write('                                                              sg_host_get_name(get_host())));\n')
    fd.write('    default:\n')
    fd.write('      THROW_IMPOSSIBLE;\n')
    fd.write('  }\n')
    fd.write('}\n')

    fd.close()

    #
    # popping_bodies.cpp
    #
    fd = header('popping_bodies.cpp')
    fd.write('#include "smx_private.hpp"\n')
    fd.write('#include "src/mc/mc_forward.hpp"\n')
    fd.write('#include "xbt/ex.h"\n')
    fd.write('#include <functional>\n')
    fd.write('#include <simgrid/simix.hpp>\n')
    fd.write('#include <xbt/log.h>\n')

    fd.write("/** @cond */ // Please Doxygen, don't look at this\n")
    fd.write('''
XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(simix);

using simgrid::simix::Simcall;

template<class R, class... T>
inline static R simcall(Simcall call, T const&... t)
{
  smx_actor_t self = SIMIX_process_self();
  simgrid::simix::marshal(&self->simcall_, call, t...);
  if (self != simix_global->maestro_) {
    XBT_DEBUG("Yield process '%s' on simcall %s (%d)", self->get_cname(), SIMIX_simcall_name(self->simcall_.call_),
              (int)self->simcall_.call_);
    self->yield();
  } else {
    self->simcall_handle(0);
  }
  return simgrid::simix::unmarshal<R>(self->simcall_.result_);
}
''')
    handle(fd, Simcall.body, simcalls, simcalls_dict)
    fd.write("/** @endcond */\n")
    fd.close()
