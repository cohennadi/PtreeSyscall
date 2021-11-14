CONFIG_MODULE_SIG=n

KDIR ?= ..

kbuild:
	make -C $(KDIR) M=`pwd` modules
	mkdir -p /lib/modules/5.4.0-custom+/extra
	rm -f /lib/modules/5.4.0-custom+/extra/ptree_loadable_module.ko
	cp /data/ubuntu/linux/ptree_loadable_module/ptree_loadable_module.ko /lib/modules/5.4.0-custom+/extra/
	depmod
	modprobe -r ptree_loadable_module
clean:
	make -C $(KDIR) M=`pwd` clean
