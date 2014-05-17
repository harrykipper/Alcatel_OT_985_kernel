import os
import sys

sys.path.append(os.environ['PERF_EXEC_PATH'] + \
	'/scripts/python/Perf-Trace-Util/lib/Perf/Trace')

from Core import *
from perf_trace_context import *

unhandled = autodict()

def trace_begin():
	print "trace_begin"
	pass

def trace_end():
        print_unhandled()

def irq__softirq_entry(event_name, context, common_cpu,
	common_secs, common_nsecs, common_pid, common_comm,
	vec):
		print_header(event_name, common_cpu, common_secs, common_nsecs,
			common_pid, common_comm)

                print_uncommon(context)

		print "vec=%s\n" % \
		(symbol_str("irq__softirq_entry", "vec", vec)),

def kmem__kmalloc(event_name, context, common_cpu,
	common_secs, common_nsecs, common_pid, common_comm,
	call_site, ptr, bytes_req, bytes_alloc,
	gfp_flags):
		print_header(event_name, common_cpu, common_secs, common_nsecs,
			common_pid, common_comm)

                print_uncommon(context)

		print "call_site=%u, ptr=%u, bytes_req=%u, " \
		"bytes_alloc=%u, gfp_flags=%s\n" % \
		(call_site, ptr, bytes_req, bytes_alloc,

		flag_str("kmem__kmalloc", "gfp_flags", gfp_flags)),

def trace_unhandled(event_name, context, event_fields_dict):
    try:
        unhandled[event_name] += 1
    except TypeError:
        unhandled[event_name] = 1

def print_header(event_name, cpu, secs, nsecs, pid, comm):
	print "%-20s %5u %05u.%09u %8u %-20s " % \
	(event_name, cpu, secs, nsecs, pid, comm),

# print trace fields not included in handler args
def print_uncommon(context):
    print "common_preempt_count=%d, common_flags=%s, common_lock_depth=%d, " \
        % (common_pc(context), trace_flag_str(common_flags(context)), \
               common_lock_depth(context))

def print_unhandled():
    keys = unhandled.keys()
    if not keys:
        return

    print "\nunhandled events:\n\n",

    print "%-40s  %10s\n" % ("event", "count"),
    print "%-40s  %10s\n" % ("----------------------------------------", \
                                 "-----------"),

    for event_name in keys:
	print "%-40s  %10d\n" % (event_name, unhandled[event_name])
