# we would prefer a symbolic link, but it does not work on windows
from past.builtins import execfile # for python3
import os
target = os.path.join(os.path.dirname(__file__),
                      './jansel-opentuner/opentuner/utils/adddeps.py')

execfile(target, dict(__file__=target))
