
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include "signal_jprobe.h"
#include "attr.h"
#define SYSFS_NAME "signaljprobe"

typedef struct demo_info
{
    struct kobject kobj;
    int32_t attr_count;
}demo_info_t;

typedef struct demo_attribute
{
    struct attribute attr;
    ssize_t (*show)(char *buf);
    ssize_t (*store)(const char *buf, size_t count);
}demo_attribute_t;

#define DEMO_ATTR(_name, _mode, _show, _store)    \ 
    demo_attribute_t demo_attr_##_name = {       \ 
    .attr = {.name  = __stringify(_name) , .mode   = _mode },\
    .show   = _show,                               \ 
    .store  = _store,                             \ 
    };


/* public func declare */
int32_t demo_attr_install(void);
void    demo_attr_uninstall(void);

/* common func declare */
void    demo_attr_release(struct kobject *ko);
ssize_t demo_attr_show(struct kobject *ko, struct attribute *attr, char *buf);
ssize_t demo_attr_store(struct kobject * kobj, struct attribute * attr, 
                            const char * buf, size_t count);

/* private func declare of specified attr */
ssize_t show_attr_status(char *buf);
ssize_t show_attr_target_pid(char *buf);
ssize_t show_attr_target_tid(char *buf);
ssize_t show_attr_target_name(char *buf);
ssize_t show_attr_source_pid(char *buf);
ssize_t show_attr_source_tid(char *buf);
ssize_t show_attr_source_name(char *buf);
ssize_t show_attr_trace_level(char *buf);

//ssize_t store_attr_interval(const char *buf, size_t count);
//ssize_t store_attr_loglevel(const char *buf, size_t count);

ssize_t store_attr_status(const char *buf, size_t count);
ssize_t store_attr_target_pid(const char *buf, size_t count);
ssize_t store_attr_target_tid(const char *buf, size_t count);
ssize_t store_attr_target_name(const char *buf, size_t count);
ssize_t store_attr_source_pid(const char *buf, size_t count);
ssize_t store_attr_source_tid(const char *buf, size_t count);
ssize_t store_attr_source_name(const char *buf, size_t count);
ssize_t store_attr_trace_level(const char *buf, size_t count);
/* grobal vars */
demo_info_t *demo_info;
struct sysfs_ops demo_sysfs_ops = {
       .show  = demo_attr_show,
       .store = demo_attr_store
   };

static DEMO_ATTR(status, S_IRUGO |S_IWUGO, 
        show_attr_status, 
        store_attr_status);
static DEMO_ATTR(target_pid, S_IRUGO |S_IWUGO, 
        show_attr_target_pid, store_attr_target_pid);
static DEMO_ATTR(target_tid, S_IRUGO |S_IWUGO, 
        show_attr_target_tid, store_attr_target_tid);
static DEMO_ATTR(target_name, S_IRUGO |S_IWUGO, 
        show_attr_target_name, store_attr_target_name);
static DEMO_ATTR(source_pid, S_IRUGO |S_IWUGO, 
        show_attr_source_pid, store_attr_source_pid);
static DEMO_ATTR(source_tid, S_IRUGO |S_IWUGO, 
        show_attr_source_tid, store_attr_source_tid);
static DEMO_ATTR(source_name, S_IRUGO |S_IWUGO, 
        show_attr_source_name, store_attr_source_name);
static DEMO_ATTR(trace_level, S_IRUGO |S_IWUGO, 
        show_attr_trace_level, store_attr_trace_level);


struct attribute * demo_default_attrs[] = {
       &demo_attr_status.attr,
       &demo_attr_target_pid.attr,
       &demo_attr_target_tid.attr,
       &demo_attr_target_name.attr,
       &demo_attr_source_pid.attr,
       &demo_attr_source_tid.attr,
       &demo_attr_source_name.attr,
       &demo_attr_trace_level.attr,
       NULL
   };
struct kobj_type demo_ktype = {
       .release       = demo_attr_release,
       .sysfs_ops     = &demo_sysfs_ops,
       .default_attrs = demo_default_attrs
   };

/* common func impl */
ssize_t demo_attr_show(struct kobject *ko, struct attribute *attr, char *buf)
{
    ssize_t len = 0;
    demo_attribute_t *a  = container_of(attr, demo_attribute_t, attr);

    if (a->show)
    {
        len = a->show(buf);
    }
    
    return len;

}

ssize_t demo_attr_store(struct kobject * kobj, struct attribute * attr, 
          const char * buf, size_t count)
{
    ssize_t len = 0;
    demo_attribute_t *a  = container_of(attr, demo_attribute_t, attr);

    if (a->store)
    {
        len = a->store(buf, count);
    }

    return len;
}

void demo_attr_release(struct kobject *ko)
{
    demo_info_t *mi = container_of(ko, demo_info_t, kobj);
    kfree(mi);
    demo_info = NULL;
}

void demo_attr_uninstall(void)
{
    int32_t i;

    /* remv all files in sysfs arch */
    for (i=0; i<demo_info->attr_count; i++)
    {
        sysfs_remove_file(&demo_info->kobj,
        demo_default_attrs[i]);
    }

    /* deref the kobject */
    kobject_del(&demo_info->kobj);
    kobject_put(&demo_info->kobj);

    while (demo_info)
    {
        schedule_timeout(HZ/2);
    }

   return;
}

int32_t demo_attr_install(void)
{
    int32_t error;

    demo_info = kmalloc(sizeof(demo_info_t), GFP_KERNEL);
    if (!demo_info)
    {
        return -ENOMEM;
    }
    memset(demo_info, 0, sizeof(demo_info_t));

    /* init the kobject */
    //kobject_init(&demo_info->kobj,&demo_ktype );
    //demo_info->attr_count = 1;
    //demo_info->kobj.ktype = &demo_ktype;

    /* generate dir & attr_file */
    error=kobject_init_and_add(&demo_info->kobj,&demo_ktype,NULL,SYSFS_NAME);
    //error = kobject_set_name(&demo_info->kobj, SYSFS_NAME);
    if (!error)
    {
        int32_t i;
        for (i=0; i<demo_info->attr_count; i++)
        {
           sysfs_create_file(&demo_info->kobj,
                          demo_default_attrs[i]
                         );
        }
    }
    if (error)
    {
        kfree(demo_info);
    }

    return error;
	
}

ssize_t show_attr_status(char *buf)
{
    if(sig_info.status)
        sprintf(buf, "SIG_JPROBE is running");
    else
        sprintf(buf, "SIG_JPROBE is stop");
        
    return strlen(buf);
}

ssize_t store_attr_status(const char *buf, size_t count)
{
    int32_t status= 0;
    sscanf(buf, "%d", &status);

    if (status==1||status==0) {
        sig_info.status=status;
    }
    else
        return -1;
    printk(KERN_INFO "[mogu_ker::signal_jprobe shut down]");
    return strlen(buf);
}

ssize_t show_attr_target_pid(char *buf)
{
    sprintf(buf, "target_pid::%d", sig_info.target_pid);
    return strlen(buf);
}

ssize_t store_attr_target_pid(const char *buf, size_t count)
{
    int32_t target_pid= 0;
    sscanf(buf, "%d", &target_pid);

    if (target_pid> 1) {
        sig_info.target_pid=target_pid;
    }
    else 
        return -1;
    return strlen(buf);
}

ssize_t show_attr_target_tid(char *buf)
{
    sprintf(buf, "target_tid::%d", sig_info.target_tid);
    return strlen(buf);
}

ssize_t store_attr_target_tid(const char *buf, size_t count)
{
    int32_t target_tid= 0;
    sscanf(buf, "%d", &target_tid);

    if (target_tid> 1) {
        sig_info.target_tid=target_tid;
    }
    else
        return -1;
    return strlen(buf);
}
ssize_t show_attr_source_pid(char *buf)
{
    sprintf(buf, "source_pid::%d", sig_info.source_pid);
    return strlen(buf);
}

ssize_t store_attr_source_pid(const char *buf, size_t count)
{
    int32_t source_pid= 0;
    sscanf(buf, "%d", &source_pid);

    if (source_pid> 1) {
        sig_info.source_pid=source_pid;
    }
    else
        return -1;
    return strlen(buf);
}
ssize_t show_attr_source_tid(char *buf)
{
    sprintf(buf, "source_tid::%d", sig_info.source_tid);
    return strlen(buf);
}

ssize_t store_attr_source_tid(const char *buf, size_t count)
{
    int32_t source_tid= 0;
    sscanf(buf, "%d", &source_tid);

    if (source_tid> 1) {
        sig_info.source_tid=source_tid;
    }
    else
        return -1;
    return strlen(buf);
}
ssize_t show_attr_target_name(char *buf)
{
    sprintf(buf, "target_name::%s", sig_info.target_name);
    return strlen(buf);
}

ssize_t store_attr_target_name(const char *buf, size_t count)
{
    if(count<20){
        sscanf(buf, "%s", sig_info.target_name);
    }
    else
        return -1;
    return strlen(buf);
}
ssize_t show_attr_source_name(char *buf)
{
    sprintf(buf, "source_name::%s", sig_info.source_name);
    return strlen(buf);
}

ssize_t store_attr_source_name(const char *buf, size_t count)
{
    if(count<20){
        sscanf(buf, "%s", sig_info.source_name);
    }
    else
        return -1;
    return strlen(buf);
}

ssize_t show_attr_trace_level(char *buf)
{
    sprintf(buf, "trace_level::%d", sig_info.trace_level);
    return strlen(buf);
}

ssize_t store_attr_trace_level(const char *buf, size_t count)
{
    int32_t trace_level= 0;
    sscanf(buf, "%d", &trace_level);

    if (trace_level>-1 && trace_level<16) {
        sig_info.trace_level=trace_level;
    }
    else
        return -1;
    return strlen(buf);
}




MODULE_LICENSE("GPL");
