#!/usr/bin/env python

import os
import json
from jinja2 import Template

dir_path = os.path.dirname(os.path.realpath(__file__))
commands = json.load(open(os.path.join(dir_path, 'commands.json')))
template = Template(open(os.path.join(dir_path, 'commands.h.tpl')).read())

def c_stringify_hex_str(str):
    return ''.join(map(lambda s: '\\x' + s[2:], str.split()))

print(template.render(commands=commands, len=len, c_stringify_hex_str=c_stringify_hex_str))
