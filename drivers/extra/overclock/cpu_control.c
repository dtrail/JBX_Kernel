/*
 * CPU frequency & voltage control for Motorola 3.0.8 kernel
 *
 * Copyright (C) 2012 Project Lense (@whirleyes)
 *
 * Based on
 *  - Motorola Milestone overclock module (by Tiago Sousa <mirage@kaotik.org>, modified by nadlabak, Skrilax_CZ)
 *  - opptimizer module by Jeffrey Kawika Patricio <jkp@tekahuna.net>
 *	
 * License: GNU GPLv3
 * <http://www.gnu.org/licenses/gpl-3.0.html>
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/clk.h>
#include <linux/kallsyms.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <plat/omap-pm.h>
#include <plat/omap_device.h>
#include <plat/common.h>
#include "../../../arch/arm/mach-omap2/omap_opp_data.h"
#include "../../../arch/arm/mach-omap2/voltage.h"
#include "../symsearch/symsearch.h"
//opp.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(int, opp_add_s, struct device *dev, unsigned long freq, unsigned long u_volt);
SYMSEARCH_DECLARE_FUNCTION_STATIC(int, opp_get_opp_count_s, struct device *dev);
SYMSEARCH_DECLARE_FUNCTION_STATIC(int, opp_enable_s, struct device *dev, unsigned long freq);
SYMSEARCH_DECLARE_FUNCTION_STATIC(struct opp *, opp_find_freq_floor_s, struct device *dev, unsigned long *freq);
SYMSEARCH_DECLARE_FUNCTION_STATIC(struct opp *, opp_find_freq_ceil_s, struct device *dev, unsigned long *freq);
//voltage.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(struct voltagedomain *, voltdm_lookup_s, char *name);
SYMSEARCH_DECLARE_FUNCTION_STATIC(void, voltdm_reset_s, struct voltagedomain *voltdm);
// cpufreq.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(struct cpufreq_governor *, __find_governor_s, const char *str_governor);
SYMSEARCH_DECLARE_FUNCTION_STATIC(int, __cpufreq_set_policy_s, struct cpufreq_policy *data, struct cpufreq_policy *policy);

struct opp {
	char *hwmod_name;
	char *voltdm_name;
	char *clk_name;

	unsigned long rate;
	unsigned long u_volt;

	bool default_available;
};

struct opp_table {
	int index;
	unsigned long rate;
	unsigned long u_volt;
	struct opp *opp;
};

static struct cpufreq_frequency_table *freq_table;
static struct cpufreq_policy *policy;
static struct device *mpu_dev, *gpu_dev;
static struct opp *gpu_opp;
static struct voltagedomain *voltdm;
static struct omap_vdd_info *vdd;
static struct clk *mpu_clk, *gpu_clk;
extern struct mutex omap_dvfs_lock;
static struct opp_table *def_ft;
DEFINE_MUTEX(omap_dvfs_lock);

int opp_count;
static char def_governor[16];
static char good_governor[16] = "hotplug";

#define BUF_SIZE PAGE_SIZE
static char *buf;

static int set_governor(struct cpufreq_policy *policy, char str_governor[16]) {
	unsigned int ret = -EINVAL;
	struct cpufreq_governor *t;
	struct cpufreq_policy new_policy;
	if (!policy)
		return ret;

	memcpy(&new_policy, policy, sizeof(struct cpufreq_policy));
	cpufreq_get_policy(&new_policy, policy->cpu);
	t = __find_governor_s(str_governor);
	if (t != NULL) {
		new_policy.governor = t;
	} else {
		return ret;
	}

	ret = __cpufreq_set_policy_s(policy, &new_policy);

	policy->user_policy.policy = policy->policy;
	policy->user_policy.governor = policy->governor;
	return ret;
}

/*  proc fs */
static int proc_gpu_cpu_speed(char *buffer, char **buffer_location, off_t offset, int count, int *eof, void *data) {
	int ret = 0;
	if (offset > 0)
		ret = 0;
	else
		ret += scnprintf(buffer+ret, count-ret,"CPU : %lu Mhz\nGPU : %lu Mhz\n", clk_get_rate(mpu_clk)/1000000, clk_get_rate(gpu_clk)/1000000);
	return ret;
}

static int proc_mpu_opp_def(char *buffer, char **buffer_location, off_t offset, int count, int *eof, void *data) {
	int i, ret = 0;

	if (offset > 0)
		ret = 0;
	else
		ret += scnprintf(buffer+ret, count-ret, "Id\tFreq\tVolt(mV)\n");
		for(i = 0;i <opp_count; i++) {
			ret += scnprintf(buffer+ret, count-ret, "%d\t%lu\t%lu\n", def_ft[i].index, def_ft[i].rate/1000000, def_ft[i].u_volt/1000);
		}
	return ret;
}

static int proc_mpu_opp_cur(char *buffer, char **buffer_location, off_t offset, int count, int *eof, void *data) {
	int i, ret = 0;

	if (offset > 0)
		ret = 0;
	else
		ret += scnprintf(buffer+ret, count-ret, "Id\tFreq\tVolt(mV)\n");
		for(i = 0;i <opp_count; i++) {
			ret += scnprintf(buffer+ret, count-ret, "%d\t%lu\t%lu\n", def_ft[i].index, def_ft[i].opp->rate/1000000, def_ft[i].opp->u_volt/1000);
		}
	return ret;
}

static int proc_cpu_tweak(struct file *filp, const char __user *buffer, unsigned long len, void *data) {
	int id, freq,volt;
	bool change;
	if(!len || len >= BUF_SIZE)
		return -ENOSPC;

	if(copy_from_user(buf, buffer, len))
		return -EFAULT;

	buf[len] = 0;
	if(sscanf(buf, "%d %d %d", &id, &freq, &volt) == 3) {
		if ((id > opp_count-1) || (id < 0)) {
			pr_err("cpu-control : wrong cpu opp id @ %d", id);
			return -EFAULT;
		}
		//some filter
		if (volt > OMAP4_VP_MPU_VLIMITTO_VDDMAX/1000) volt = OMAP4_VP_MPU_VLIMITTO_VDDMAX/1000;
		if (volt < OMAP4_VP_MPU_VLIMITTO_VDDMIN/1000) volt = OMAP4_VP_MPU_VLIMITTO_VDDMIN/1000;
		if (freq > 2000) freq = def_ft[opp_count-1].rate/1000;
		if (freq < 0) freq = def_ft[0].rate/1000;

		pr_info("cpu-control : Change operating point : %d %d Mhz %d mV\n", id, freq, volt);

		policy->min = policy->cpuinfo.min_freq = policy->user_policy.min =
		policy->max = policy->cpuinfo.max_freq = policy->user_policy.max =
		def_ft[opp_count-1].rate/1000;

		pr_info("cpu-control : Current cpufreq gov : %s\n", policy->governor->name);
		if (policy->governor->name != good_governor) {
			strcpy(def_governor, policy->governor->name);
			set_governor(policy, good_governor);
			change = true;
			pr_info("cpu-control : Change cpufreq gov : %s\n", policy->governor->name);
		}

		mutex_lock(&omap_dvfs_lock);

		freq_table[id].frequency = freq * 1000;
		vdd->volt_data[id].volt_nominal = volt * 1000;
		vdd->dep_vdd_info[0].dep_table[id].main_vdd_volt = volt * 1000;
		def_ft[id].opp->u_volt = volt * 1000;
		def_ft[id].opp->rate = freq * 1000000;

		voltdm_reset_s(voltdm);
		mutex_unlock(&omap_dvfs_lock);

		if (change) {
			set_governor(policy, def_governor);
			pr_info("cpu_control : Revert cpufreq gov : %s\n", policy->governor->name);
		}

		policy->min = policy->cpuinfo.min_freq = policy->user_policy.min = freq_table[0].frequency;
		policy->max = policy->cpuinfo.max_freq = policy->user_policy.max = freq_table[opp_count-1].frequency;
	}
	return len;
}
/* proc fs end */

static int __init cpu_control_init(void) {
	struct proc_dir_entry *proc_entry;

	int i;
	unsigned long freq = ULONG_MAX;

	pr_info("cpu-control : Hello world! @ cpu_control version : a3\n");
	//opp.c
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, opp_add, opp_add_s);
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, opp_enable, opp_enable_s);
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, opp_get_opp_count, opp_get_opp_count_s);
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, opp_find_freq_floor, opp_find_freq_floor_s);
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, opp_find_freq_ceil, opp_find_freq_ceil_s);
	//voltage.c
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, voltdm_lookup, voltdm_lookup_s);
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, voltdm_reset, voltdm_reset_s);
	// cpufreq.c
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, __find_governor, __find_governor_s);
	SYMSEARCH_BIND_FUNCTION_TO(cpu_control, __cpufreq_set_policy, __cpufreq_set_policy_s);

	freq_table = cpufreq_frequency_get_table(0);
	policy = cpufreq_cpu_get(0);
	mpu_clk = clk_get(NULL, "dpll_mpu_ck");
	gpu_clk = clk_get(NULL, "gpu_fck");

	voltdm = voltdm_lookup_s("mpu");
	if (!voltdm || IS_ERR(voltdm)) {
		pr_err("cpu-control : No voltage domain?\n");
		return -ENODEV;
	}
	vdd = voltdm->vdd;

	mpu_dev = omap2_get_mpuss_device();
	if (!mpu_dev || IS_ERR(mpu_dev)) {
		return -ENODEV;
	}
	opp_count = opp_get_opp_count_s(mpu_dev);

	gpu_dev = omap_hwmod_name_get_dev("gpu");
	if (!gpu_dev || IS_ERR(gpu_dev)) {
		return -ENODEV;
	}

	def_ft = kzalloc(sizeof(struct opp_table) * (opp_count), GFP_KERNEL);
	for(i = 0; i<opp_count; i++) {
		def_ft[i].index = i;
		freq = (freq_table[i].frequency-1000) * 1000;
		def_ft[i].opp = opp_find_freq_ceil_s(mpu_dev, &freq);
		def_ft[i].rate = def_ft[i].opp->rate;
		def_ft[i].u_volt = def_ft[i].opp->u_volt;
		pr_info("Map %d : %lu Mhz : %lu mV\n", def_ft[i].index, def_ft[i].rate/1000000, def_ft[i].u_volt/1000);
	}
		
	//pr_info("GPU OPP counts : %d\n", opp_get_opp_count_s(gpu_dev));
	gpu_opp = opp_find_freq_floor_s(gpu_dev, &freq);
	//gpu_opp->rate = 384000000;
	pr_info("cpu-control : GPU Default max value : %lu mV : %lu\n", gpu_opp->u_volt/1000, gpu_opp->rate/1000);
/*	if (opp_get_opp_count_s(gpu_dev) == 2) {
		//opp_disable_s(gpu_dev, 384000000);
		opp_add_s(gpu_dev, 384000000, gpu_opp->u_volt);
		opp_enable_s(gpu_dev, 384000000);
	}
*/
	buf = (char *)vmalloc(BUF_SIZE);

	proc_mkdir("cpu_control", NULL);
	proc_entry = create_proc_read_entry("cpu_control/frequency_current", 0444, NULL, proc_gpu_cpu_speed, NULL);
	proc_entry = create_proc_read_entry("cpu_control/opp_table_current", 0444, NULL, proc_mpu_opp_cur, NULL);
	proc_entry = create_proc_read_entry("cpu_control/opp_table_default", 0444, NULL, proc_mpu_opp_def, NULL);
	proc_entry = create_proc_read_entry("cpu_control/tweak_cpu", 0644, NULL, NULL, NULL);
	proc_entry->write_proc = proc_cpu_tweak;
/*
	//prepare
	pr_info("Current cpufreq gov : %s\n", policy->governor->name);
	if (policy->governor->name != good_governor) {
		strcpy(def_governor, policy->governor->name);
		set_governor(policy, good_governor);
		policy = cpufreq_cpu_get(0);
		change = true;
		pr_info("Current cpufreq gov : %s\n", policy->governor->name);
	}

	policy->min = policy->cpuinfo.min_freq = policy->user_policy.min =
	policy->max = policy->cpuinfo.max_freq = policy->user_policy.max =
	def_max_freq/1000;

	mutex_lock(&omap_dvfs_lock);
	//hack

	freq_table[0].frequency = LOW_FREQ/1000;
	vdd->volt_data[0].volt_nominal = LOW_VOLTAGE;
	vdd->dep_vdd_info[0].dep_table[0].main_vdd_volt = LOW_VOLTAGE;
	gpu_opp->u_volt = LOW_VOLTAGE;
	gpu_opp->rate = LOW_FREQ;

	freq_table[4].frequency = HIGH_FREQ/1000;
	vdd->volt_data[4].volt_nominal = HIGH_VOLTAGE;
	vdd->dep_vdd_info[0].dep_table[4].main_vdd_volt = HIGH_VOLTAGE;
	mpu_opp->u_volt = HIGH_VOLTAGE;
	mpu_opp->rate = HIGH_FREQ;

	//min_opp->rate= 384000000;
/*
	if (opp_count == 5) {
		opp_add_s(mpu_dev, LOW_FREQ, LOW_VOLTAGE);
		opp_add_s(mpu_dev, HIGH_FREQ, HIGH_VOLTAGE);
		opp_enable_s(mpu_dev, LOW_FREQ);
		opp_enable_s(mpu_dev, HIGH_FREQ);
		opp_count = opp_get_opp_count_s(mpu_dev);
		temp_ft = kzalloc(sizeof(struct cpufreq_frequency_table) * (opp_count + 1), GFP_KERNEL);
		for(i = 0; i<8; i++) {
			temp_ft[i].index = i;
			switch (i) {
				case 0:
					temp_ft[i].frequency = LOW_FREQ/1000;
					break;
				case 6:
					temp_ft[i].frequency = HIGH_FREQ/1000;
					break;
				case 7:					
					temp_ft[i].frequency = CPUFREQ_TABLE_END;
					break;
				default:
					temp_ft[i].frequency = freq_table[i-1].frequency;
			}
		}
		pr_info("Temp voltage & frequency map\n");
		for(i = 0; i<opp_count; i++) {
			pr_info("%d : %u mV %u Mhz\n",i,vdd->volt_data[i].volt_nominal/1000, temp_ft[i].frequency/1000);
		}
		cpufreq_frequency_table_put_attr(0);
		cpufreq_frequency_table_get_attr(temp_ft,0);
		temp_ft = NULL;
		freq_table = cpufreq_frequency_get_table(0);
	}
	if (opp_get_opp_count_s(gpu_dev) == 2) {
		//opp_disable_s(gpu_dev, 384000000);
		opp_add_s(gpu_dev, 384000000, 1127000);
		opp_enable_s(gpu_dev, 384000000);
	}
	voltdm_reset_s(voltdm);
	mutex_unlock(&omap_dvfs_lock);

	policy->min = policy->cpuinfo.min_freq = policy->user_policy.min = freq_table[0].frequency;
	policy->max = policy->cpuinfo.max_freq = policy->user_policy.max = freq_table[opp_count-1].frequency;

	pr_info("Optimize voltage & frequency map\n");
	for(i = 0; i<opp_count; i++) {
		pr_info("%d : %u mV %u Mhz\n",i,vdd->volt_data[i].volt_nominal/1000, freq_table[i].frequency/1000);
	}

	pr_info("hacked mpu OPP counts : %d\n", opp_get_opp_count_s(mpu_dev));
	pr_info("hacked GPU OPP counts : %d\n", opp_get_opp_count_s(gpu_dev));

	if (change == true) {
		set_governor(policy, def_governor);
		policy = cpufreq_cpu_get(0);
		pr_info("Current cpufreq gov : %s\n", policy->governor->name);
	}
*/
	return 0;
}

static void __exit cpu_control_exit(void){
	int i;
	bool change;

	pr_info("cpu_control : Reset opp table to default.\n");
	remove_proc_entry("cpu_control/frequency_current", NULL);
	remove_proc_entry("cpu_control/opp_table_current", NULL);
	remove_proc_entry("cpu_control/opp_table_default", NULL);
	remove_proc_entry("cpu_control/tweak_cpu", NULL);
	remove_proc_entry("cpu_control", NULL);
	vfree(buf);

	policy->min = policy->cpuinfo.min_freq = policy->user_policy.min =
	policy->max = policy->cpuinfo.max_freq = policy->user_policy.max =
	def_ft[opp_count-1].rate/1000;

	pr_info("cpu-control : Current cpufreq gov : %s\n", policy->governor->name);
	if (policy->governor->name != good_governor) {
		strcpy(def_governor, policy->governor->name);
		set_governor(policy, good_governor);
		change = true;
		pr_info("cpu-control : Change cpufreq gov : %s\n", policy->governor->name);
	}

	mutex_lock(&omap_dvfs_lock);
	for(i = 0; i<opp_count; i++) {
		freq_table[i].frequency = def_ft[i].rate/1000;
		vdd->volt_data[i].volt_nominal = def_ft[i].u_volt;
		vdd->dep_vdd_info[0].dep_table[i].main_vdd_volt = def_ft[i].u_volt;
		def_ft[i].opp->u_volt = def_ft[i].u_volt;
		def_ft[i].opp->rate = def_ft[i].rate;
	}
	voltdm_reset_s(voltdm);
	mutex_unlock(&omap_dvfs_lock);

	if (change) {
		set_governor(policy, def_governor);
		pr_info("cpu_control : Revert cpufreq gov : %s\n", policy->governor->name);
	}

	policy->min = policy->cpuinfo.min_freq = policy->user_policy.min = freq_table[0].frequency;
	policy->max = policy->cpuinfo.max_freq = policy->user_policy.max = freq_table[opp_count-1].frequency;
	pr_info("cpu_control : Goodbye\n");
}

module_init(cpu_control_init);
module_exit(cpu_control_exit);

MODULE_AUTHOR("whirleyes");
MODULE_DESCRIPTION("cpu_control - Tweak cpu/gpu voltage & frequency.\nextra : Moto**** version.");
MODULE_LICENSE("GPL");
