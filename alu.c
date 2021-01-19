#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	
#include <linux/types.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/kdev_t.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#define SIZE 50



MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

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

int endRead = 0;
unsigned char reg[4];
unsigned char result, carry;
unsigned char ispis = 1; //ispis = 0 to znaci dec
		     //  ispis = 1 to znaci hex
			// ispis = 2 to znaci bin
			// ispis = 3 znaci app

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

bool is_read = false;
int velicina(unsigned int broj)
{
	if (broj <= 1)
		return 1;
	else if (broj <= 3)
		return 2;
	else if (broj <= 7)
		return 3;
	else if (broj <= 15)
		return 4;
	else if (broj <= 31)
		return 5;
	else if (broj <= 63)
		return 6;
	else if (broj <= 127)
		return 7;
	else if (broj <= 255)
		return 8;
	return 0;
}
ssize_t alu_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{	
	int ret;
	long int len;
	char input[SIZE];
	unsigned int broj = (unsigned int) result;
	int br_velicina = velicina(broj);
	unsigned char tmp[9];
	int i = 0;
	int byte;
	
	printk(KERN_INFO "KERNEL READ OPERATION");
	printk(KERN_WARNING "Result: %d, carry: %d", result, carry);
	
	
	switch (ispis)
	{
		case 3: len = scnprintf(input, SIZE, "%d %d", result, carry); break;
		case 2: 
			while (i < br_velicina)  // i < 5
			{
				byte = broj % 2;
				//printk(KERN_INFO "Broj %d", byte);
				tmp[br_velicina - i - 1] = byte + '0';   // 4 - i
				//printk(KERN_INFO "Broj u charu %c", tmp[br_velicina - i + 1] );
				broj = broj / 2;
				i++;
			}
		
			tmp[i] = '\0'; //tmp[5]
			printk(KERN_INFO "tmp 0b%s", tmp);
			len = scnprintf(input, SIZE, "0b%s %d", tmp, carry);
			break;
		case 1: len = scnprintf(input, SIZE, "0x%x %d\n", result, carry); break;
		case 0: len = scnprintf(input, SIZE, "%d %d\n", result, carry); break;

	}
	
	ret = copy_to_user(buffer, input, len);
	
	if (ret) return -EFAULT;

	if (is_read) {
		is_read = false;
		return 0;
	}
	
	is_read = true;
	return len;
}

ssize_t alu_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char input[SIZE];
	int ret;
	char registar1[5]/*registar2[5]*/;
	unsigned int vrednost;
	unsigned char reg1, reg2;
	char koja_operacija;
	char reg1_ABCD;
	char reg2_ABCD;
	char koji_registar;
	char dec_bin_hex;

	printk(KERN_INFO "KERNEL WRITE OPERATION");
	ret = copy_from_user(input, buffer, length);

	if(ret)
		return -EFAULT;

	input[length-1] = '\0';

	if(input[4] == '=')
	{
		sscanf(input, "%4s=%x", registar1, &vrednost);
		
		koji_registar = registar1[3];

		switch(koji_registar)
		{
			case 'A': 
			case 'a': reg[0] = vrednost;
				  break;
			case 'B':
			case 'b': reg[1] = vrednost;
				  break;

			case 'C': 
			case 'c': reg[2] = vrednost;
				  break;
			
			case 'D': 
			case 'd': reg[3] = vrednost;
				  break;
			default: printk("Nedozvoljen unos");
			         break;
		}

	}
	else if(input[6] == '=')
	{
		sscanf(input, "format=%c", &dec_bin_hex);

		if(dec_bin_hex == 'd')
			ispis = 0;
		else if (dec_bin_hex == 'b')
			ispis = 2;
		else if (dec_bin_hex == 'h')
			ispis = 1;
		else
			ispis = 3;
	}
	else
	{
		koja_operacija = input[5];
		reg1_ABCD = input[3];
		reg2_ABCD = input[10];
		
		switch(reg1_ABCD)
		{
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

		switch(reg2_ABCD)
		{
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

		switch(koja_operacija)
		{
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
	
		return length;
}

static int __init alu_init(void)
{
   int ret = 0;
   ret = alloc_chrdev_region(&my_dev_id, 1, 1, "alu");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "alu_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "alu");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "/dev/alu added\n");

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
	cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(alu_init);
module_exit(alu_exit);
