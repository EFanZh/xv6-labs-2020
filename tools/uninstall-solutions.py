#!/usr/bin/env python3

import os;
from os import path;


def main():
    os.chdir(path.dirname(path.dirname(path.abspath(__file__))))

    for dir_path, _, file_names in os.walk('solutions'):
        for file_name in file_names:
            source = path.join(dir_path, file_name)
            target = path.join('labs', path.relpath(source, 'solutions'))

            try:
                os.remove(target)
            except FileNotFoundError:
                pass


if __name__ == '__main__':
    main()
