#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>


MODULE_AUTHOR("Ryuichi Ueda and Souya Watanabe");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL; 
static volatile u32 *gpio_base = NULL;

static int seg_gpio[7] = {16, 20, 19, 27, 17, 23, 18};
static int seg_1[7]= {0, 1, 1, 0, 0, 0, 0};
static int seg_2[7]= {1, 1, 0, 1, 1, 0, 1};
static int seg_3[7]= {1, 1, 1, 1, 0, 0, 1};
static int seg_4[7]= {0, 1, 1, 0, 0, 1, 1};
static int seg_5[7]= {1, 0, 1, 1, 0, 1, 1};
static int seg_6[7]= {1, 0, 1, 1, 1, 1, 1};


static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t* pos)
{
    int size = 0;
    char sushi[] = {'d','i','c','e',0x0A}; //寿司の絵文字のバイナリ
    if(copy_to_user(buf+size,(const char *)sushi, sizeof(sushi))){
        printk( KERN_ERR "sushi : copy_to_user failed\n" );
        return -EFAULT;
    }
    size += sizeof(sushi);
    return size;
}

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{    
     int n;
     char c;   //読み込んだ字を入れる変数
     if(copy_from_user(&c,buf,sizeof(char)))
        return -EFAULT;

     //printk(KERN_INFO "receive %c\n",c);
     if(c == '-')
     {
         for( n = 0; n < 7; n++)
               gpio_base[7] = 1 << seg_gpio[n];
     }
     else if(c == '1')
     {
         for( n = 0; n < 7; n++ )
         {
             if( seg_1[n] == 0)
             {
                 gpio_base[7] = 1 << seg_gpio[n];
             }else
             {
                 gpio_base[10] = 1 << seg_gpio[n];
             }
         }
     }
     else if(c == '2')
     {
         for( n = 0; n < 7; n++ )
         {
             if( seg_2[n] == 0)
             {
                 gpio_base[7] = 1 << seg_gpio[n];
             }else
                 gpio_base[10] = 1 << seg_gpio[n];
         }
     }
     else if(c == '3')
     {
         for( n = 0; n < 7; n++ )
         {
             if( seg_3[n] == 0)
             {
                 gpio_base[7] = 1 << seg_gpio[n];
             }else
                 gpio_base[10] = 1 << seg_gpio[n];
         }
     }
     else if(c == '4')
     {
         for( n = 0; n < 7; n++ )
         {
             if( seg_4[n] == 0)
             {
                 gpio_base[7] = 1 << seg_gpio[n];
             }else
                 gpio_base[10] = 1 << seg_gpio[n];
         }
     }

     else if(c == '5')
     {
         for( n = 0; n < 7; n++ )
         {
             if( seg_5[n] == 0)
             {
                 gpio_base[7] = 1 << seg_gpio[n];
             }else
                 gpio_base[10] = 1 << seg_gpio[n];
         }
     }
     else if(c == '6')
     {
         for( n = 0; n < 7; n++ )
         {
             if( seg_6[n] == 0)
             {
                 gpio_base[7] = 1 << seg_gpio[n];
             }else
                 gpio_base[10] = 1 << seg_gpio[n];
         }
     }
     return 1;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
    .read = sushi_read
};

static int __init init_mod(void)
{
    int retval;
    int i;

    retval =  alloc_chrdev_region(&dev, 0, 1, "myled");
    if(retval < 0){
        printk(KERN_ERR "alloc_chrdev_region failed.\n");
        return retval;
    }
    printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));
    cdev_init(&cdv, &led_fops);
    retval = cdev_add(&cdv, dev, 1);
    if(retval < 0)
    {
        printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
        return retval;
    }
    cls = class_create(THIS_MODULE,"myled");   //ここから追加
    if(IS_ERR(cls)){
        printk(KERN_ERR "class_create failed.");
        return PTR_ERR(cls);
    }
    device_create(cls, NULL, dev, NULL, "myled%d",MINOR(dev));

    gpio_base = ioremap_nocache(0x3f200000, 0xA0);
    
    for( i = 0 ; i < 7 ; i++)
    { 
        const u32 led = seg_gpio[i];
        const u32 index = led/10;//GPFSEL2
        const u32 shift = (led%10)*3;//15bit
        const u32 mask = ~(0x7 << shift);//11111111111111000111111111111111
        gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);//001: output flag
    }
    

    return 0;
}

static void __exit cleanup_mod(void)
{
    cdev_del(&cdv);
    device_destroy(cls, dev);
    class_destroy(cls);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "%s is unloaded. major:%d\n",__FILE__,MAJOR(dev));
}

module_init(init_mod); //マクロで関数を登録
module_exit(cleanup_mod); //同上

