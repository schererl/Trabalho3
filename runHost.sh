gcc ./setpriority.c -o setpriority -lpthread && sudo taskset --cpu-list 1 ./setpriority 3 10000000000 sched_rr 99 sched_rr 99 sched_other 0
