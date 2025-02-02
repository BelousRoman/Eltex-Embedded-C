// SPDX-License-Identifier: GPL-2.0
/*
 * Sample kset and ktype implementation
 *
 * Copyright (C) 2004-2007 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2007 Novell Inc.
 */
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

#define BUF_LEN				15

/*
 * This is our "object" that we will create a few of and register them with
 * sysfs.
 */
struct example_obj {
	struct kobject kobj;
	int test;
	char string[BUF_LEN];
};
#define to_test_obj(x) container_of(x, struct example_obj, kobj)

/* a custom attribute that works just for a struct example_obj. */
struct example_attribute {
	struct attribute attr;
	ssize_t (*show)(struct example_obj *tmp_obj, struct example_attribute *attr, char *buf);
	ssize_t (*store)(struct example_obj *tmp_obj, struct example_attribute *attr, const char *buf, size_t count);
};
#define to_test_attr(x) container_of(x, struct example_attribute, attr)

/*
 * The default show function that must be passed to sysfs.  This will be
 * called by sysfs for whenever a show function is called by the user on a
 * sysfs file associated with the kobjects we have registered.  We need to
 * transpose back from a "default" kobject to our custom struct example_obj and
 * then call the show function for that specific object.
 */
static ssize_t test_attr_show(struct kobject *kobj,
			     struct attribute *attr,
			     char *buf)
{
	struct example_attribute *attribute;
	struct example_obj *tmp_obj;

	attribute = to_test_attr(attr);
	tmp_obj = to_test_obj(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(tmp_obj, attribute, buf);
}

/*
 * Just like the default show function above, but this one is for when the
 * sysfs "store" is requested (when a value is written to a file.)
 */
static ssize_t test_attr_store(struct kobject *kobj,
			      struct attribute *attr,
			      const char *buf, size_t len)
{
	struct example_attribute *attribute;
	struct example_obj *tmp_obj;

	attribute = to_test_attr(attr);
	tmp_obj = to_test_obj(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(tmp_obj, attribute, buf, len);
}

/* Our custom sysfs_ops that we will associate with our ktype later on */
static const struct sysfs_ops test_sysfs_ops = {
	.show = test_attr_show,
	.store = test_attr_store,
};

/*
 * The release function for our object.  This is REQUIRED by the kernel to
 * have.  We free the memory held in our object here.
 *
 * NEVER try to get away with just a "blank" release function to try to be
 * smarter than the kernel.  Turns out, no one ever is...
 */
static void test_release(struct kobject *kobj)
{
	struct example_obj *tmp_obj;

	tmp_obj = to_test_obj(kobj);
	kfree(tmp_obj);
}

/*
 * The "test" file where the .test variable is read from and written to.
 */
static ssize_t test_show(struct example_obj *tmp_obj, struct example_attribute *attr,
			char *buf)
{
	return sysfs_emit(buf, "%d\n", tmp_obj->test);
}

static ssize_t test_store(struct example_obj *tmp_obj, struct example_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &tmp_obj->test);
	if (ret < 0)
		return ret;

	return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct example_attribute example_attribute =
	__ATTR(test, 0664, test_show, test_store);

/*
 * The "test" file where the .test variable is read from and written to.
 */
static ssize_t string_show(struct example_obj *tmp_obj, struct example_attribute *attr,
			char *buf)
{
	return sysfs_emit(buf, "%s\n", tmp_obj->string);
}

static ssize_t string_store(struct example_obj *tmp_obj, struct example_attribute *attr,
			 const char *buf, size_t count)
{
	if (count > BUF_LEN)
		return -EINVAL;
	
	strncpy(tmp_obj->string, buf, BUF_LEN);

	return count;
}

static struct example_attribute string_attribute =
	__ATTR(string, 0664, string_show, string_store);

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *test_default_attrs[] = {
	&example_attribute.attr,
	&string_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};
/* Argument must be equal to name of above struct attribute with exception '_attrs' suffix */
ATTRIBUTE_GROUPS(test_default);

/*
 * Our own ktype for our kobjects.  Here we specify our sysfs ops, the
 * release function, and the set of default attributes we want created
 * whenever a kobject of this type is registered with the kernel.
 */
static const struct kobj_type test_ktype = {
	.sysfs_ops = &test_sysfs_ops,
	.release = test_release,
	.default_groups = test_default_groups,
};

static struct kset *example_kset;
static struct example_obj *test_obj;
static struct example_obj *string_obj;

static struct example_obj *create_test_obj(const char *name)
{
	struct example_obj *tmp_obj;
	int retval;

	/* allocate the memory for the whole object */
	tmp_obj = kzalloc(sizeof(*tmp_obj), GFP_KERNEL);
	if (!tmp_obj)
		return NULL;

	/*
	 * As we have a kset for this kobject, we need to set it before calling
	 * the kobject core.
	 */
	tmp_obj->kobj.kset = example_kset;

	/*
	 * Initialize and add the kobject to the kernel.  All the default files
	 * will be created here.  As we have already specified a kset for this
	 * kobject, we don't have to set a parent for the kobject, the kobject
	 * will be placed beneath that kset automatically.
	 */
	retval = kobject_init_and_add(&tmp_obj->kobj, &test_ktype, NULL, "%s", name);
	if (retval) {
		kobject_put(&tmp_obj->kobj);
		return NULL;
	}

	/*
	 * We are always responsible for sending the uevent that the kobject
	 * was added to the system.
	 */
	kobject_uevent(&tmp_obj->kobj, KOBJ_ADD);

	return tmp_obj;
}

static void destroy_test_obj(struct example_obj *tmp_obj)
{
	kobject_put(&tmp_obj->kobj);
}

static int __init example_init(void)
{
	/*
	 * Create a kset with the name of "kset_example",
	 * located under /sys/kernel/
	 */
	example_kset = kset_create_and_add("kset_example", NULL, kernel_kobj);
	if (!example_kset)
		return -ENOMEM;

	/*
	 * Create two objects and register them with our kset
	 */
	test_obj = create_test_obj("test");
	if (!test_obj)
		goto test_error;

	string_obj = create_test_obj("string");
	if (!string_obj)
		goto string_error;

	return 0;

	string_error:
		destroy_test_obj(test_obj);

	test_error:
		kset_unregister(example_kset);

	return -EINVAL;
}

static void __exit example_exit(void)
{
	destroy_test_obj(string_obj);
	destroy_test_obj(test_obj);
	kset_unregister(example_kset);
}

module_init(example_init);
module_exit(example_exit);

MODULE_LICENSE("GPL");
