#!/usr/bin/python
import os, sys, socket, random
import time
from test_common import *

def test_function(opts, port):
    print "Starting stress client at ", time.strftime("%H:%M:%S")
    stress_client(port, workload= { "deletes":  opts["ndeletes"],
                                    "updates":  opts["nupdates"],
                                    "inserts":  opts["ninserts"],
                                    "gets":     opts["nreads"],
                                    "appends":  opts["nappends"],
                                    "prepends": opts["nprepends"] },
                  duration="%ds" % opts["duration"])
        
if __name__ == "__main__":
    op = make_option_parser()
    op["ndeletes"]  = IntFlag("--ndeletes",  1)
    op["nupdates"]  = IntFlag("--nupdates",  4)
    op["ninserts"]  = IntFlag("--ninserts",  8)
    op["nreads"]    = IntFlag("--nreads",    64)
    op["nappends"]  = IntFlag("--nappends",  0)
    op["nprepends"] = IntFlag("--nprepends", 0)
    opts = op.parse(sys.argv)
    opts["netrecord"] = False   # We don't want to slow down the network
    auto_server_test_main(test_function, opts, timeout = opts["duration"] + 60)
