/*
myled.c
Device driver that displays numbers using LEDs

Copyright (c) 2018, Yohei Ashino

This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/io.h>
MODULE_AUTHOR("Yohei Ashino");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

static ssize_t led_write(struct file* flip, const char* buf, size_t count, loff_t* pos)
{
	char c;	//読み込んだ数字を入れる変数
	if(copy_from_user(&c, buf, sizeof(char)))
		return -EFAULT;

	//キーボードの入力"d"を受け付けたとき、LEDを消灯させる
	if(c == 'd'){
		gpio_base[10] = 1 << 18;
		gpio_base[10] = 1 << 19;
		gpio_base[10] = 1 << 21;
		gpio_base[10] = 1 << 22;
		gpio_base[10] = 1 << 23;
		gpio_base[10] = 1 << 24;
		gpio_base[10] = 1 << 25;
	}
	//入力された数字1～9と同じ数字を点灯させる
	else if(c == '0'){
		gpio_base[7] = 1 << 18;
		gpio_base[7] = 1 << 19;
		gpio_base[7] = 1 << 22;
		gpio_base[7] = 1 << 23;
		gpio_base[7] = 1 << 24;
		gpio_base[7] = 1 << 25;
	}
	else if(c == '1'){
		gpio_base[7] = 1 << 18;
		gpio_base[7] = 1 << 19;
	}
	else if(c == '2'){
		gpio_base[7] = 1 << 24;
		gpio_base[7] = 1 << 19;
		gpio_base[7] = 1 << 21;
		gpio_base[7] = 1 << 23;
		gpio_base[7] = 1 << 25;
	}
	else if(c == '3'){
		gpio_base[7] = 1 << 18;
		gpio_base[7] = 1 << 19;
		gpio_base[7] = 1 << 21;
		gpio_base[7] = 1 << 23;
		gpio_base[7] = 1 << 25;
	}
	else if(c == '4'){
		gpio_base[7] = 1 << 18;
		gpio_base[7] = 1 << 19;
		gpio_base[7] = 1 << 21;
		gpio_base[7] = 1 << 22;
	}
	else if(c == '5'){
		gpio_base[7] = 1 << 18;	
		gpio_base[7] = 1 << 21;
		gpio_base[7] = 1 << 22;
		gpio_base[7] = 1 << 23;
		gpio_base[7] = 1 << 25;
	}
	else if(c == '6'){
		gpio_base[7] = 1 << 18;	
		gpio_base[7] = 1 << 21;
		gpio_base[7] = 1 << 22;
		gpio_base[7] = 1 << 23;
		gpio_base[7] = 1 << 24;
		gpio_base[7] = 1 << 25;
	}
	else if(c == '7'){
		gpio_base[7] = 1 << 18;	
		gpio_base[7] = 1 << 19;
		gpio_base[7] = 1 << 22;
		gpio_base[7] = 1 << 23;
	}
	else if(c == '8'){
		gpio_base[7] = 1 << 18;	
		gpio_base[7] = 1 << 19;
		gpio_base[7] = 1 << 21;
		gpio_base[7] = 1 << 22;
		gpio_base[7] = 1 << 23;
		gpio_base[7] = 1 << 24;
		gpio_base[7] = 1 << 25;
	}
	else if(c == '9'){
		gpio_base[7] = 1 << 18;
		gpio_base[7] = 1 << 19;
		gpio_base[7] = 1 << 21;
		gpio_base[7] = 1 << 22;
		gpio_base[7] = 1 << 23;
		gpio_base[7] = 1 << 25;
	}

	return 1;
} 
static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_write,
};

static int __init init_mod(void)
{
	int retval;
	retval = alloc_chrdev_region(&dev, 0, 1, "myled");
	printk(KERN_INFO "%s is loaded.major:%d\n",__FILE__,MAJOR(dev));
	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval < 0){
		printk(KERN_ERR "cdev_add falled.major:%d, minor:%d\n",MAJOR(dev),MINOR(dev));
		return retval;
	}
	cls = class_create(THIS_MODULE,"myled");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	device_create(cls, NULL, dev, NULL, "myled%d", MINOR(dev));
	gpio_base = ioremap_nocache(0x3f200000, 0xA0);	//物理アドレスから仮想アドレスに変換
	int number;
	for(number = 18; number < 26; number++){
		const u32 led = number;
		const u32 index = led / 10;//GPFSEL2
		const u32 shift = (led%10)*3;//15bit
		const u32 mask = ~(0x7 << shift);//11111111111111000111111111111111
		gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);//001: output flag
	}
	return 0;
}

static void __exit cleanup_mod(void)
{
	cdev_del(&cdv);
	class_destroy(cls);
	device_destroy(cls, dev);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded.major:%d\n",__FILE__,MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
