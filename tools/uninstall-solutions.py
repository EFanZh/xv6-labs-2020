#!/usr/bin/env python3

import os;
from os import path;
import subprocess


def main():
    os.chdir(path.dirname(path.dirname(path.abspath(__file__))))

    subprocess.run(['git', 'submodule', 'foreach', 'git', 'clean', '-fd'], check=True)
    subprocess.run(['git', 'submodule', 'foreach', 'git', 'reset', '--hard'], check=True)


if __name__ == '__main__':
    main()
