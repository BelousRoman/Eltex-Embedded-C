obj-m += procfs_module.o

PWD := $(CURDIR)
ccflags-y += -g -DDEBUG

all:
	$(MAKE) -C /home/roman/git/linux M=$(PWD) modules

clean:
	$(MAKE) -C /home/roman/git/linux M=$(PWD) clean

rebuild: clean all
