trace-cmd record -e sched:sched_switch -F setpriority 5 10000000 SCHED_OTHER 0
 SCHED_OTHER 0 SCHED_OTHER 0 SCHED_OTHER 0 SCHED_RR 99;trace-cmd report

