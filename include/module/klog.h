#ifndef FIBERS_MODULE_KLOG_H
#define FIBERS_MODULE_KLOG_H

#include <linux/string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define warn(fmt, ...) \
		pr_warn("[%s] %s:%d " fmt ,__FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define alert(fmt, ...) \
		pr_alert("[%s] %s:%d " fmt ,__FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif
