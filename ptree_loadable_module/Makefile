CONFIG_MODULE_SIG=n

KDIR ?= ..
MODULES_EXTRA ?= /lib/modules/5.4.0-custom+/extra

kbuild:
	make -C $(KDIR) M=`pwd` modules
	mkdir -p $(MODULES_EXTRA)
	rm -f $(MODULES_EXTRA)/ptree_loadable_module.ko
	cp ptree_loadable_module.ko $(MODULES_EXTRA)
	depmod
	modprobe -r ptree_loadable_module
clean:
	make -C $(KDIR) M=`pwd` clean
