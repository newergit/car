#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h> //设备号和设备号分配
#include <linux/io.h> //ioremap和iounmap函数
#include <linux/uaccess.h> //copy_from_user函数
#include <mach/gpio.h>  //s5pv210 的gpio端口的定义

//insmod 模块后，cat /proc/devices，找到本模块的设备号
//然后用 “mknod /dev/自定义的文件名 c 主设备号 次设备号 ” 创建设备文件




//*定义端口*/
int led_port[4]={S5PV210_GPJ2(0),
				S5PV210_GPJ2(1),
				S5PV210_GPJ2(2),
				S5PV210_GPJ2(3)};




/*自定义的open函数，也就是需要重新解读的open
需要参考file_operations结构体里面成员的写法*/
int led_open(struct inode *inode, struct file *pfile)
{
	int i;
	for(i=0;i<4;i++)
	{
		gpio_direction_output(led_port[i],1);
	}

    return 0;
}

static void close_all(void)
{
	int i;
	for(i=0;i<4;i++)
		gpio_set_value(led_port[i],1);
}

/*自定义的close函数 */
int led_close(struct inode *inode, struct file *pfile)
{
	close_all();
	return 0;
}




//led灯的写函数,将write 设备文件的操作重新解读
ssize_t led_write(struct file *pfile, const char __user *buffer, size_t len, loff_t *loff)
{
	char data;
	copy_from_user(&data,buffer,1);

	if(data>0&&data<5)
	{
		close_all();
		gpio_set_value(led_port[data-1],0);
	}

	  	
	return 1;
}


/*文件操作结构体定义,顺便初始化,这个变量在注册驱动的时候要用到*/
struct file_operations fops={
    .owner=THIS_MODULE,
    .open=led_open,//将自定义的函数赋值给文件操作结构体，实际是注册
	.release=led_close,//同上
	.write=led_write,//同上
};



dev_t devid=0;//设备号

/*static 在这里表示本函数只在本文件可见，其它文件看不到，更谈不上调用
__init是将函数放入特定的区域，在初始化结束后该内存区域会被释放，以回收空间
*/ 
static int __init led_init(void)
{
    printk(KERN_EMERG "hello world!I m a module.\n");

    // 动态分配设备号
    if(alloc_chrdev_region(&devid,11,1,"ledid")==0)
    {
        printk(KERN_INFO "dev id was allocated. id is %u\n",devid);
        printk(KERN_INFO "major is %d, minor is %d\n",MAJOR(devid),MINOR(devid));
    }

    /*将驱动注册到系统里*/
    register_chrdev(MAJOR(devid),"justforshow",&fops);


	/*申请gpio端口*/
	int i=0;
	for(i=0;i<4;i++)
		{
			if(gpio_request(led_port[i],"led")<0)
				printk(KERN_INFO "request gpio%d failed.\n",i);
		}


    return 0;
}

/*__exit在这里表示，如果模块不允许卸载（例如编进了内核镜像里）,
那么本函数就可以直接丢弃，不编译到具体的二进制文件中。*/
static void __exit led_exit(void)
{
    printk(KERN_EMERG "Goodby cruel world!\n");
		/*释放gpio端口*/
	int i=0;
	for(i=0;i<4;i++)
		{
			gpio_free(led_port[i]);			
		}


	unregister_chrdev(MAJOR(devid),"justforshow");//驱动的反注册
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
