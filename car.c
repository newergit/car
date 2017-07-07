#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h> //设备号和设备号分配
#include <linux/io.h> //ioremap和iounmap函数
#include <linux/uaccess.h> //copy_from_user函数
#include <mach/gpio.h>  //s5pv210 的gpio端口的定义

//insmod 模块后，cat /proc/devices，找到本模块的设备号
//然后用 “mknod /dev/自定义的文件名 c 主设备号 次设备号 ” 创建设备文件
void front_left(void);
void front_right(void);
void back_left(void);
void back_right(void);
void front(void);
void back(void);



//*定义端口*/
int car_port[10]={S5PV210_GPE0(3),
				S5PV210_GPE0(4),//前左      
    			S5PV210_GPE0(5),      
    			S5PV210_GPE0(6),//前右     
    			S5PV210_GPE0(7),      
    			S5PV210_GPE1(0),//后左      
    			S5PV210_GPE1(1),      
    			S5PV210_GPE1(2),//后右


				S5PV210_GPH2(2),
				S5PV210_GPH2(3), 
			};




/*自定义的open函数，也就是需要重新解读的open
需要参考file_operations结构体里面成员的写法*/
int car_open(struct inode *inode, struct file *pfile)
{
	int i;
	for(i=0;i<8;i++)
	{
		gpio_request(car_port[i],"car");
		gpio_direction_output(car_port[i],1);
	}
	for(i = 8; i < 10; i++)
    {
		gpio_request(car_port[i],"car");
        gpio_direction_input(car_port[i]);
    }
    return 0;
}

/*自定义的close函数 */
int car_close(struct inode *inode, struct file *pfile)
{
	int i=0;
	for(i;i<8;i++)
	{
		gpio_free(car_port[i]);
	}
	return 0;
}

void front(void)
{
	int i,j;
	for(i=0;i<8;i+=2)
		gpio_set_value(car_port[i],0);
	for(j=1;j<8;j+=2)
		gpio_set_value(car_port[j],1);
}

void back(void)
{
	int i,j;
	for(i=0;i<8;i+=2)
		gpio_set_value(car_port[i],1);
	for(j=1;j<8;j+=2)
		gpio_set_value(car_port[j],0);
}

void stop(void)
{
	int i,j;
	for(i=0;i<8;i+=2)
		gpio_set_value(car_port[i],0);
	for(j=1;j<8;j+=2)
		gpio_set_value(car_port[j],0);
}

void front_left(void)
{
    int i = 0;
    for (i = 0; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],0);
    }
    for (i = 1; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],1);
        if(i==1)
            gpio_set_value(car_port[i],0);   
    }

}

void front_right(void)
{
    int i = 0;
    for (i = 0; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],0);
    }
    for (i = 1; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],1);
        if(i==3)
            gpio_set_value(car_port[i],0);    
    }

}

void back_left(void)
{
    int i = 0;
    for (i = 0; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],1);
        if(i==4)
            gpio_set_value(car_port[i],0); 
    }
    for (i = 1; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],0);
    }

}

void back_right(void)
{
    int i = 0;
    for (i = 0; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],1);
        if(i==6)
            gpio_set_value(car_port[i],0); 
    }
    for (i = 1; i < 8;i+=2)
    {
        gpio_set_value(car_port[i],0);
    }

}

/*
获取左右障碍信息：
    0:前方全障碍
    1:左方障碍
    2:右方障碍
    3:无障碍
*/
int get_value(void)     
{
    int right_value = gpio_get_value(car_port[8]);
    int left_value = gpio_get_value(car_port[9]);
    return (right_value + left_value*2);
}

void auto_mod(void)  //进入自动模式
{
    while(1)
    {
        switch(get_value())
        {
            case 0:
            {
                stop();
                front_right();
                break;
            }
            case 1:{front_right();printk("right");};break;
            case 2:{front_left();printk("left");};break;
            case 3:front();break;
        }
    }
}



ssize_t car_write(struct file *pfile, const char __user *buffer, size_t len, loff_t *loff)
{
	char data;
	copy_from_user(&data,buffer,1);
	if(data>0&&data<9)
	{
		if(data==1)
			front();
		if(data==2)
			back();
		if(data==3)
			stop();
		if(data==4)
			front_left();
		if(data==5)
			front_right();
		if(data==6)
			back_left();
		if(data==7)
			back_right();
		if(data==8)
			auto_mod();
	}
	  	
	return 1;
 }


/*文件操作结构体定义,顺便初始化,这个变量在注册驱动的时候要用到*/
struct file_operations fops={
    .owner=THIS_MODULE,
    .open=car_open,//将自定义的函数赋值给文件操作结构体，实际是注册
	.release=car_close,//同上
	.write=car_write,//同上
};



dev_t devid=0;//设备号

/*static 在这里表示本函数只在本文件可见，其它文件看不到，更谈不上调用
__init是将函数放入特定的区域，在初始化结束后该内存区域会被释放，以回收空间
*/ 
static __init int car_init(void)
{
    printk(KERN_EMERG "hello world!i am a module.\n");
    
    if(alloc_chrdev_region(&devid,11,1,"carid")==0)
    {
        printk(KERN_INFO "dev id was allocated. id is %u\n",devid);
        printk(KERN_INFO "major is %d,minor is %d\n",MAJOR(devid),MINOR(devid));
    }
    register_chrdev(MAJOR(devid),"just for show",&fops);

    return 0;
}


static void __exit car_exit(void)
{
    printk("Goodby!");
	unregister_chrdev_region(devid,1);
    unregister_chrdev(MAJOR(devid),"just for show");
}

module_init(car_init);
module_exit(car_exit);
MODULE_LICENSE("GPL");
