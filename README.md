omap_jbx_kernel
===============

JBX-Kernel for Motorola XT910/XT912 OMAP4 (D-WiZ JBX Rom Series - Battery Saving Technologies)

This Kernel is based on the Motorola 3.0.8 Kernel for Kexec which was initiated by the STS-Dev-Team.
See this link for the original source: https://github.com/STS-Dev-Team/kernel_mapphone_kexec
See credits below

There are two different versions, JBX-Kernel and JBX-Kernel Battery Saver Edition



Some features of JBX-Kernel:

- Underclocked (stable 100mhz min frequency)
- Undervolted (~10mV)
- Modified Smartreflex driver with Custom Sensor Values to Override factory defaults. 
  This allows us to overclock stable to ~1,5ghz!
- Custom Voltage Interface
- Live Overclock Interface
- GPU Overclock
- GPU frequency control sysfs interface 
- Support for GPU_OC App
- Custom OPP-Table
- Reduced latency
- Tweaked governors
- Support for Trickster Mod App (from Gnex) [INOFFICIAL]
- Increased R/W Ahead
- Overclocking Kernel Modules based on Milestone Overclock (Ported by Whirleyes) [NOT INCLUDED IN BUILD BECAUSE WE HAVE LIVE OC]
	---> /drivers/extra/ [include this folder in Makefile to build. Needs precompiled kernel!!)


...more to come


JBX-Kernel BSE hast almost the same features and 
configuration except voltage, max frequency policy
and some other values to reach more power saving.



WIP:

- Need fix to force the CPU to stay on custom min freq OPP0 [FIXED]
- Live OC function included but doesn't work [FIXED]
- Fixing compile warnings [...]
- USB driver needs a workaround [...]
- Fix hotplugging feature for related governors (sort out bad governors) [FIXED]
- Start fixing build warnings [...]





*Credits to 

- Kholk & [mbm] for Kexec Initial Release
- Hashcode & dhacker29 (STS-Dev-Team) for Kexec Kernel 3.0.31 mr1)
- Surdu_Petru for support and knowledge
