#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/wait.h>

#define BUFF_SIZE 50

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;


int endRead = 0;
int alu_open(struct inode *pinode, struct file *pfile);
int alu_close(struct inode *pinode, struct file *pfile);
ssize_t alu_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t alu_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = alu_open,
	.read = alu_read,
	.write = alu_write,
	.release = alu_close,
};

unsigned char reg[4];
unsigned char result, carry;
int alu_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

int alu_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}

ssize_t alu_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len;
	int minor = MINOR(pfile->f_inode->i_rdev);

	if (endRead) {	
		endRead = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
		
	if (minor >= 0 && minor <= 3) {
		len = scnprintf(buff,BUFF_SIZE,"0x%x\n", reg[minor]);
	} else if (minor == 5) {
		len = scnprintf(buff, BUFF_SIZE, "0x%x %d\n", result, carry);
	} 
	
	ret = copy_to_user(buffer, buff, len);
	
	if (ret)
		return -EFAULT;
	endRead = 1;
	return len;
}

ssize_t alu_write(struct file *pfile, const char *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	char prvi_reg, drugi_reg, operacija;
	unsigned int vrednost;
	unsigned char reg1, reg2;
	int minor=MINOR(pfile->f_inode->i_rdev);

	ret = copy_from_user(buff, buffer, length);
	buff[length-1]='\0';


	if (minor >= 0 && minor <= 3) {
  		ret=sscanf(buff, "%4x", &vrednost);
		
  		if(ret == 1 ) {	
  			reg[minor] = vrednost;
  			printk(KERN_INFO "Upisana je vrednost %d", vrednost);
		}
		
		else printk(KERN_WARNING "Nedozvoljen unos");

  	}

  	else if (minor == 4) 
  	{
		
  		ret = sscanf(buff, "reg%c %c reg%c", &prvi_reg, &operacija, &drugi_reg);
  		
		if(ret == 3) {
			printk(KERN_INFO "Upisana je operacija");	
			switch (prvi_reg) {
				case 'A': 
				case 'a': reg1 = reg[0];
					  break;
				case 'B':
				case 'b': reg1 = reg[1];
					  break;

				case 'C': 
				case 'c': reg1 = reg[2];;
					  break;
				
				case 'D': 
				case 'd': reg1 = reg[3];;
					  break;

				default: printk("Nedozvoljen unos");
					 break;
			}
			switch (drugi_reg) {
				case 'A': 
				case 'a': reg2 = reg[0];
					  break;
				case 'B':
				case 'b': reg2 = reg[1];
					  break;

				case 'C': 
				case 'c': reg2 = reg[2];;
					  break;
				
				case 'D': 
				case 'd': reg2 = reg[3];;
					  break;

				default: printk("Nedozvoljen unos");
					 break;
			}
			switch (operacija) {
				case '+': 
					  carry = ((reg1 + reg2) > 255) ? 1 : 0;
					  result = reg1 + reg2;
					  break;
				case '-':
					  carry = 0 + (reg2 > reg1);
					  result = reg1 - reg2;
					  break;

				case '*': 
					  carry = 0 + (255 < (reg1 *reg2));
					  result = reg1 * reg2;
					  break;
				
				case '/': carry = 0;
					  result = reg1 / reg2;
					  break;
					
				default: printk("Nedozvoljen unos");
					 break;
			}

		}
		else 
			printk(KERN_WARNING "Nedozvoljen unos");

	}
	

	return length;
}

static int __init alu_init(void)
{
    int ret = 0;
    carry = 0;

    //device region 
    ret = alloc_chrdev_region(&my_dev_id, 0, 6, "alu");
    if (ret) {
    	printk(KERN_ERR "failed to register char device\n");
    	return ret;
    }
    printk(KERN_INFO "char device region allocated\n");

	//class create
    my_class = class_create(THIS_MODULE, "alu_class");
    if (my_class == NULL){
    	printk(KERN_ERR "failed to create class\n");
        goto fail_0;
    }
    printk(KERN_INFO "class created\n");
   
 	//alu_rega
    my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 0), NULL, "alu_rega");
    if(my_device == NULL){
   		printk(KERN_INFO "failed to create device\n");
		goto fail_1;
    }
	printk(KERN_INFO "created nod alu_rega\n ");
   
    //alu_regb
    my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 1), NULL, "alu_regb");
    if(my_device == NULL) {
   		printk(KERN_INFO "failed to create device\n");
		goto fail_1;
    }
	printk(KERN_INFO "created nod alu_regb\n ");
   
    //alu_regc
    my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 2), NULL, "alu_regc");
    if(my_device == NULL){ 
   		printk(KERN_INFO "failed to create device\n");
		goto fail_1;
    }
	printk(KERN_INFO "created nod alu_regc\n ");
	
	//alu_regd
    printk(KERN_INFO "created nod alu_regd\n ");
    my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 3), NULL, "alu_regd");
    if(my_device == NULL){
   		printk(KERN_INFO "failed to create device\n");
		goto fail_1;
    }

	//alu_op
    my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 4), NULL, "alu_op");
    if(my_device == NULL){
   		printk(KERN_INFO "failed to create device\n");
		goto fail_1;
    }
	printk(KERN_INFO "created nod alu_op\n ");
	 
	//alu_result 
    my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), 5), NULL, "alu_result");
    if(my_device == NULL){
   		printk(KERN_INFO "failed to create device\n");
		goto fail_1;
    }
    printk(KERN_INFO "created nod alu_result\n ");
    printk(KERN_INFO "device created\n");

    my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 6);
	if (ret) {
      	printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
	
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit alu_exit(void)
{
   int i;
   cdev_del(my_cdev);
   
   for(i = 0; i < 6; i++)
   	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),i));

   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id, 6);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(alu_init);
module_exit(alu_exit);

