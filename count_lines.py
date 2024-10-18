#!/usr/bin/env python3
import os
import subprocess
from pathlib import Path

subprocess.run(f"cloc {Path(__file__).parent} --exclude-dir=third_party,single_file_compilation_tests,old_gpu,b_lib,gen --exclude-ext=txt --by-file", shell=True)
