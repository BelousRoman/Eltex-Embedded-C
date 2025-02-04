#include "sysfs_module.h"

/* Our custom sysfs_ops that we will associate with our ktype later on */
static const struct sysfs_ops sops = {
	.show = attr_show,
	.store = attr_store,
};

/*
 * The default show function that must be passed to sysfs.  This will be
 * called by sysfs for whenever a show function is called by the user on a
 * sysfs file associated with the kobjects we have registered.  We need to
 * transpose back from a "default" kobject to our custom struct example_obj and
 * then call the show function for that specific object.
 */
static ssize_t attr_show(struct kobject *kobj,
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
static ssize_t attr_store(struct kobject *kobj,
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

/*
 * The release function for our object.  This is REQUIRED by the kernel to
 * have.  We free the memory held in our object here.
 *
 * NEVER try to get away with just a "blank" release function to try to be
 * smarter than the kernel.  Turns out, no one ever is...
 */
static void sysfs_release(struct kobject *kobj)
{
	struct example_obj *tmp_obj;

	tmp_obj = to_test_obj(kobj);
	kfree(tmp_obj);
}

/* Sysfs attributes cannot be world-writable. */
static struct example_attribute int_attribute =
	__ATTR(dec, 0664, int_show, int_store);

static struct example_attribute string_attribute =
	__ATTR(str, 0664, char_array_show, char_array_store);

/*
 * The "dec" file where the .dec variable is read from and written to.
 */
static ssize_t int_show(struct example_obj *tmp_obj, struct example_attribute *attr,
			char *buf)
{
	return sysfs_emit(buf, "%d\n", tmp_obj->dec);
}

static ssize_t int_store(struct example_obj *tmp_obj, struct example_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &tmp_obj->dec);
	if (ret < 0)
		return ret;

	return count;
}

/*
 * The "str" file where the .str variable is read from and written to.
 */
static ssize_t char_array_show(struct example_obj *tmp_obj, struct example_attribute *attr,
			char *buf)
{
	return sysfs_emit(buf, "%s", tmp_obj->str);
}

static ssize_t char_array_store(struct example_obj *tmp_obj, struct example_attribute *attr,
			 const char *buf, size_t count)
{
	if (count > BUF_LEN)
		return -EINVAL;
	
	strncpy(tmp_obj->str, buf, BUF_LEN);

	return count;
}

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *int_default_attrs[] = {
	&int_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};
static struct attribute *string_default_attrs[] = {
	&string_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/* Argument must be equal to name of above struct attribute with exception '_attrs' suffix */
ATTRIBUTE_GROUPS(int_default);
ATTRIBUTE_GROUPS(string_default);

/*
 * Our own ktype for our kobjects.  Here we specify our sysfs ops, the
 * release function, and the set of default attributes we want created
 * whenever a kobject of this type is registered with the kernel.
 */
static const struct kobj_type int_ktype = {
	.sysfs_ops = &sops,
	.release = sysfs_release,
	.default_groups = int_default_groups,
};
static const struct kobj_type string_ktype = {
	.sysfs_ops = &sops,
	.release = sysfs_release,
	.default_groups = string_default_groups,
};

static struct example_obj *create_example_obj(const char *name, const struct kobj_type *ktype)
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
	retval = kobject_init_and_add(&tmp_obj->kobj, ktype, NULL, "%s", name);
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

static void destroy_example_obj(struct example_obj *tmp_obj)
{
	kobject_put(&tmp_obj->kobj);
}

static int __init example_init(void)
{
	/*
	 * Create a kset with the name of KSET_NAME, located under /sys/kernel/.
	 */
	example_kset = kset_create_and_add(KSET_NAME, NULL, kernel_kobj);
	if (!example_kset)
		return -ENOMEM;

	/*
	 * Create two objects and register them with our kset.
	 */
	test_obj = create_example_obj(INT_OBJ_NAME, &int_ktype);
	if (test_obj == NULL)
		goto test_error;

	string_obj = create_example_obj(STR_OBJ_NAME, &string_ktype);
	if (string_obj == NULL)
		goto string_error;

	return 0;

	string_error:
		destroy_example_obj(test_obj);

	test_error:
		kset_unregister(example_kset);

	return -EINVAL;
}

static void __exit example_exit(void)
{
	destroy_example_obj(string_obj);
	destroy_example_obj(test_obj);
	kset_unregister(example_kset);
}

module_init(example_init);
module_exit(example_exit);

MODULE_LICENSE("GPL");
