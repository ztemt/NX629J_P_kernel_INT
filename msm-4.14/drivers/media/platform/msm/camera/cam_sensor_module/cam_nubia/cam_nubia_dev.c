/* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include "cam_nubia_dev.h"



static struct kobject *nubia_camera_kobj;
static int torch_switch;
static int file_dump;
static int ois_switch;


static unsigned char* buffer0;



/*ZTEMT: fengxun add for file_dump--------Start*/
static const char FileDumpPath[]        = "/data/vendor/camera/";

enum file_dump_type {
	CAMERA_FILE_EEPROM_BACK_MAIN     = 0x01,
	CAMERA_FILE_EEPROM_BACK_AUX      = 0x02,
	CAMERA_FILE_EEPROM_FRONT_MAIN	 = 0x03,
	CAMERA_FILE_EEPROM_FRONT_AUX 	 = 0x04,
	CAMERA_FILE_EEPROM_ARCSOFT 	     = 0x05,
	CAMERA_FILE_EEPROM_ARCSOFT_NEW 	 = 0x06,
	CAMERA_FILE_WATERMARK		     = 0x07,
	CAMERA_FILE_AI              	 = 0x08,
	CAMERA_FILE_DEPTH              	 = 0x09,
	CAMERA_FILE_MAX,
};
/*ZTEMT: fengxun add for file_dump--------End*/


static struct cam_flash_ctrl *nubia_flash_ctrl;
void nubia_flash_node_save_ctrl(struct cam_flash_ctrl *flash_ctrl)
{
	pr_err("[CAM_NODE] nubia_flash_node_save_ctrl \n");
	nubia_flash_ctrl = flash_ctrl;
}


/*ZTEMT: fengxun add for torch--------Start*/
static ssize_t torch_switch_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", torch_switch);
}

static ssize_t torch_switch_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &torch_switch);
	if (ret < 0)
		return ret;

	if(NULL == nubia_flash_ctrl){
		pr_err("[CAM_NODE] Invalid nubia_flash_ctrl \n");
		return ret;
	}

	cam_flash_switch(nubia_flash_ctrl,torch_switch);


	return count;
}

static struct kobj_attribute torch_switch_attribute =
	__ATTR(torch_switch, 0664, torch_switch_show, torch_switch_store);
/*ZTEMT: fengxun add for torch--------End*/

/*ZTEMT: fengxun add for file_dump--------Start*/
static ssize_t file_dump_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", file_dump);
}

static ssize_t file_dump_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret = 0;
	char filename[80];
	struct file *f;
	mm_segment_t fs;
	loff_t pos = 0;


/*
	ret = kstrtoint(buf, 10, &file_dump_type);
	if (ret < 0)
		return ret;
*/
	pr_err("[CAM_NODE] file_dump_store count = %d \n",count);

	if (count < 2){
		return ret;
	}
	pr_err("[CAM_NODE] file_dump_store buf[0] = %d \n",buf[0]);
	pr_err("[CAM_NODE] file_dump_store buf[1] = %d \n",buf[1]);



	pr_err("[CAM_NODE] file_dump_store FileDumpPath = %s \n",FileDumpPath);

//	if(CAMERA_FILE_EEPROM_BACK_MAIN == file_dump)
	if(0)
	{
		sprintf(filename, "%s%s",FileDumpPath,"eeprom.dat");

		pr_err("[CAM_NODE] file_dump_store filename = %s \n",filename);

		f = filp_open(filename, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG );
		if(IS_ERR(f)){
			pr_err("[CAM_NODE] filp_open error");
			return -1;
		}else{
			pr_err("[CAM_NODE] filp_open OK");
		}

		fs = get_fs();
		set_fs(KERNEL_DS);

		vfs_write(f,buf,count,&pos);

		set_fs(fs);

		filp_close(f, NULL);
		pr_err("[CAM_NODE] write OK");


	}else{

		buffer0 = (unsigned char*)vmalloc(count);
		memcpy(buffer0,buf,count);
		pr_err("[CAM_NODE] file_dump_store buffer0[0] = %d \n",buffer0[0]);
		pr_err("[CAM_NODE] file_dump_store buffer0[1] = %d \n",buffer0[1]);

		pr_err("[CAM_NODE] file_dump_store buffer0[1] = %d \n",buffer0[count-2]);
		pr_err("[CAM_NODE] file_dump_store buffer0[1] = %d \n",buffer0[count-1]);


	}



	return count;
}

static struct kobj_attribute file_dump_attribute =
	__ATTR(file_dump, 0664, file_dump_show, file_dump_store);
/*ZTEMT: fengxun add for file_dump--------End*/


/*ZTEMT: fengxun add for ois--------Start*/
static ssize_t ois_switch_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", ois_switch);
}

static ssize_t ois_switch_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &ois_switch);
	if (ret < 0){
		pr_err("[CAM_NODE] ois_switch error");
		return ret;
	}

	if(0 == ois_switch){
		pr_err("[CAM_NODE] ois_switch enable");
		msm_ois_lc898124_enable(0);
	}else if(1 == ois_switch){
		pr_err("[CAM_NODE] ois_switch disable");
		msm_ois_lc898124_enable(1);
	}else{
		pr_err("[CAM_NODE] ois_switch value error");
	}

	return count;
}

static struct kobj_attribute ois_switch_attribute =
	__ATTR(ois_switch, 0664, ois_switch_show, ois_switch_store);
/*ZTEMT: fengxun add for ois--------End*/



static struct attribute *attrs[] = {
	&torch_switch_attribute.attr,
	&file_dump_attribute.attr,
	&ois_switch_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};


static int32_t __init cam_nubia_node_init_module(void)
{
	int retval;

	pr_err("[CAM_NODE] cam_nubia_node_init_module \n");

	nubia_camera_kobj = kobject_create_and_add("camera", kernel_kobj);
	if (!nubia_camera_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(nubia_camera_kobj, &attr_group);
	if (retval)
		kobject_put(nubia_camera_kobj);

	return retval;

}

static void __exit cam_nubia_node_exit_module(void)
{
	pr_err("[CAM_NODE] cam_nubia_node_exit_module \n");
	kobject_put(nubia_camera_kobj);
}

module_init(cam_nubia_node_init_module);
module_exit(cam_nubia_node_exit_module);
MODULE_DESCRIPTION("CAM NUBIA");
MODULE_LICENSE("GPL v2");

