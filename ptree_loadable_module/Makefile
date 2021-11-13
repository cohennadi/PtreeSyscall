CONFIG_MODULE_SIG=n

KDIR ?= ..

kbuild:
	make -C $(KDIR) M=`pwd` modules_install
clean:
	make -C $(KDIR) M=`pwd` clean
