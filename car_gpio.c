//电机驱动GPIO端口控制部分
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>   //copy_from_
#include <linux/gpio.h>

unsigned int state[8];

// void set_state(int *state);
void go(void);
void retreat(void);
void front_left(void);
void front_right(void);
void back_left(void);
void back_right(void);
void stop(void);

void auto_mod(void);
int get_value(void); 


//GPIO对应车轮控制
/*******************
        ^
        |
        |
<----左 前

    GPH2_01---GPH2_23
            |
            |
            |
            |
            |
    GPH3_01---GPH3_23

********************/

unsigned int car_port[10] = {
    //电机驱动IO
    S5PV210_GPE0(3),      
    S5PV210_GPE0(4),//前左      
    S5PV210_GPE0(5),      
    S5PV210_GPE0(6),//前右     
    S5PV210_GPE0(7),      
    S5PV210_GPE1(0),//后左      
    S5PV210_GPE1(1),      
    S5PV210_GPE1(2),//后右       
    //红外驱动IO
    S5PV210_GPH2(2),      //引脚23----2
    S5PV210_GPH2(3),      //引脚24----3
};



//car_driver文件的OPEN函数
int car_open(struct inode *ino,struct file *f)
{
    int i;
    //初始化查询
    for(i = 0;i < 10;i++)
    {
        gpio_free(car_port[i]); 
		if (gpio_request(car_port[i],"car") < 0)
		{
			printk("GPIO %d request failed\n",i);
		}
    }

    //方向设置
    for (i = 0;i < 8;i++)
    {
        gpio_direction_output(car_port[i],0);
    }
    for(i = 8; i < 10; i++)
    {
        gpio_direction_input(car_port[i]);
    }
    return 0;
}

//文件关闭时执行的函数
int car_close(struct inode *ino,struct file *f)
{
    int i;
    for(i = 0;i < 10;i++)
    {
 		gpio_set_value(car_port[i],0);
		gpio_free(car_port[i]);       
    }
    return 0;
}

static ssize_t car_write(struct file *f, const char __user *buffer,size_t len, loff_t *off)
{
    char buffer_ker;
    copy_from_user(&buffer_ker,buffer,len);
    
    switch(buffer_ker)
    {
        case 0:stop();break;
        case 1:go();break;
        case 2:retreat();break; 
        case 3:front_left();break;
        case 4:front_right();break; 
        case 5:back_left();break; 
        case 6:back_right();break;
        case 7:auto_mod();break;
        default: 
            break;
    }
    printk("write running...\n");
    return sizeof(buffer_ker);
}


//+++++++++++++++++++static int __init car_init(void)+++++++++++++++++++++++++++++++++++++++++++++++++

void stop(void)
{
    int i = 0;
    for (i = 0; i < 8;)
    {
        gpio_set_value(car_port[i++],0);
    }
}

void go(void)
{
    printk("go.....\n");
    int i = 0;
    for (i = 0; i < 8;)
    {
        gpio_set_value(car_port[i++],0);
        gpio_set_value(car_port[i++],1);
        gpio_set_value(car_port[i++],0);
        gpio_set_value(car_port[i++],1);
    }
    // state[8]={0,1,0,1,0,1,0,1};
    // return state;
}

void retreat(void)
{
    int i = 0;
    for (i = 0; i < 8;)
    {
        gpio_set_value(car_port[i++],1);
        gpio_set_value(car_port[i++],0);
        gpio_set_value(car_port[i++],1);
        gpio_set_value(car_port[i++],0);
    }
    printk("retreat.....\n");
    // state[8]={1,0,1,0,1,0,1,0};
    // return state;
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
     printk("front_left.....\n");
    // state[8]={0,0,1,0,0,0,1,0};
    // return state;
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
    printk("front_right.....\n");
    // state[8]={1,0,0,0,1,0,0,0};
    // return state;
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
    // state[8]={0,0,1,0,0,0,1,0};
    // return state;
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
    // state[8]={0,1,0,0,0,1,0,0};
    // return state;
}

//+++++++++++++++++++++++ yourself +++++++++++++++++++++++++++++++++++++
void auto_mod(void)  //进入自动模式
{
    printk("auto is running.....");
    while(1)
    {
        switch(get_value())
        {
            case 0:
            {
                retreat();
                front_right();
                break;
            }
            case 1:front_right();break;
            case 2:front_left();break;
            case 3:go();break;
        }
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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
long car_ioctl(struct file *f,unsigned int cmd1,unsigned long cmd2)
{
    gpio_set_value(car_port[cmd1-1],cmd2);
    printk("GPIO num: %d ;pin level: %ld\n",cmd1,cmd2);

    return 0;
}

//设备文件名
char name[]="car_linux";

//主设备号
int major=210;
//次设备号
int minor=0;
//设备的class
struct class *car_class = NULL;
//对应的device class
struct device_class *car_device_class =NULL;

//文件操作函数的集合
struct file_operations fops=
{
	.owner = THIS_MODULE,
    .write = car_write,
	.open = car_open,
	.release = car_close,
	.unlocked_ioctl = car_ioctl
};

static int __init car_init(void)
{
	//注册设备,将设备号，设备名，文件操作函数三者绑定
	register_chrdev(major,name,&fops);

	//创建一个class
	car_class = class_create(THIS_MODULE,name);

	//根据class创建class_device
	device_create(car_class,NULL,MKDEV(major,minor),NULL,name);

	return 0;
}
module_init(car_init);

static void __exit car_exit(void)
{
	device_destroy(car_class,MKDEV(major,minor));
	class_destroy(car_class);
	unregister_chrdev(major,name);
}

module_exit(car_exit);

//驱动描述
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FlorHop");//作者名
MODULE_DESCRIPTION("car driver");//模块信息
