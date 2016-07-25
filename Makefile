obj-m :=io_trace.o do_fork_trace.o
io_trace-objs :=io_delay_trace.o
do_fork_trace-objs :=kretprobe_example.o

PWD  := /home/chaisong/kprobe_test
KVER := $(shell uname -r)
KDIR :=/usr/src/kernels/2.6.32-431.el6.x86_64
all:  
	    make -C $(KDIR) M=$(PWD) modules  
