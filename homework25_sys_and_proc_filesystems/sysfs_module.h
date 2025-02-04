/*
* Eltex's academy part of homework #25 for lecture 55 
* "Sys and Proc RAM Filesystems".
*
* Sysfs module
*
* This code compiles into kernel object file, which creates a set inside of
* /sys/kernel/KSET_NAME directory a set of other directories: INT_OBJ_NAME,
* STR_OBJ_NAME, whose contain a single file which represents a variable inside
* a this module, either dec or str.
* Additional directories are created by adding calling create_example_obj with
* unique name and new or already existing ktype variable, describing sysfs
* operations, release function and mapped variable with its permissions and
* behavior. 
* Variable to particular directory is added by initializing
* `struct example_attribute` with __ATTR macro, containing variable name,
* permissions and show and store functions, then add it to existing
* `struct attribute` array of pointers or put it in a new one, call
* ATTRIBUTE_GROUPS macro and add newly created attribute group to new
* `struct kobj_type` ktype.
*
* Based on:
* - <linux/Documentation/filesystems/sysfs.rst> doc;
* - <linux/samples/kobject/kset_example.c> module.
*/

#ifndef _SYSFS_MODULE_H
#define _SYSFS_MODULE_H

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>

/*
 * This module shows how to create a kset in sysfs called
 * /sys/kernel/kset-example
 * Then tree kobjects are created and assigned to this kset, "foo", "baz",
 * and "bar".  In those kobjects, attributes of the same name are also
 * created and if an integer is written to these files, it can be later
 * read out of it.
 */

#define BUF_LEN								15
#define KSET_NAME							"example_kset"
#define INT_OBJ_NAME						"decimal"
#define STR_OBJ_NAME						"string"

/*
 * This is our "object" that we will create a few of and register them with
 * sysfs.
 */
struct example_obj
{
	struct kobject kobj;
	int dec;
	char str[BUF_LEN];
};
#define to_test_obj(x) container_of(x, struct example_obj, kobj)

/* a custom attribute that works just for a struct example_obj. */
struct example_attribute
{
	struct attribute attr;
	ssize_t (*show)(struct example_obj *tmp_obj, 
                        struct example_attribute *attr, char *buf);
	ssize_t (*store)(struct example_obj *tmp_obj, 
                        struct example_attribute *attr, const char *buf, 
                        size_t count);
};
#define to_test_attr(x) container_of(x, struct example_attribute, attr)


static struct kset *example_kset;
static struct example_obj *test_obj;
static struct example_obj *string_obj;


static ssize_t attr_show(struct kobject *kobj, struct attribute *attr,
			                    char *buf);

static ssize_t attr_store(struct kobject *kobj, struct attribute *attr,
			                    const char *buf, size_t len);

static void sysfs_release(struct kobject *kobj);

static ssize_t int_show(struct example_obj *tmp_obj, struct example_attribute *attr,
			char *buf);

static ssize_t int_store(struct example_obj *tmp_obj, struct example_attribute *attr,
			 const char *buf, size_t count);

static ssize_t char_array_show(struct example_obj *tmp_obj, struct example_attribute *attr,
			char *buf);

static ssize_t char_array_store(struct example_obj *tmp_obj, struct example_attribute *attr,
			 const char *buf, size_t count);

static struct example_obj *create_example_obj(const char *name, const struct kobj_type * ktype);

static void destroy_example_obj(struct example_obj *tmp_obj);

#endif // _SYSFS_MODULE_H