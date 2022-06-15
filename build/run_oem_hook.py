#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import argparse
import os
import runpy
import sys

DEBUG = False

def find_oem_script(path):
    for relpath, dirs, files in os.walk(path):
        if "oem_hook.py" in files:
            script_path = os.path.join(path, relpath, "oem_hook.py")
            return os.path.normpath(os.path.abspath(script_path))
    return ""


def main():
    if DEBUG:
        print("Run oem hook start")

    parser = argparse.ArgumentParser()
    parser.add_argument('--out_build_file', help='oem build', required=True)
    args = parser.parse_args()
    output_file = args.out_build_file
    if DEBUG:
        print("output_file:" + output_file)

    hiview_build_path = os.path.split(os.path.realpath(__file__))[0]
    hiview_path = os.path.realpath(os.path.join(hiview_build_path, ".."))
    script_path = find_oem_script(hiview_path)
    globals_args = {'oem_dynamic_deps': [], 'oem_test_deps': []}
    if script_path.strip() == '':
        print("Could not find oem hook")
    else:
        runpy.run_path(script_path, init_globals=globals_args ,run_name='__main__')
    if DEBUG:
        print(globals_args)

    dynamic_info = 'oem_dynamic_deps = [\n%s\n]\n' % (''.join(globals_args.get('oem_dynamic_deps', '')))
    test_info = ('oem_test_deps = [\n%s\n]\n' % (''.join(globals_args.get('oem_test_deps', ''))))
    buff = '%s%s' % (dynamic_info, test_info)
    out_dir = os.path.dirname(output_file)
    print('out_dir:' + out_dir)
    if os.path.isdir(out_dir) is False:
        os.makedirs(out_dir)
    with open(output_file, 'w') as file_id:
        if DEBUG:
            print(buff)
        file_id.write(buff)

    if DEBUG:
        print("Run oem hook end")
    return 0


if __name__ == '__main__':
    sys.exit(main())
