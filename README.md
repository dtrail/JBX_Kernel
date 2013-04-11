omap_jbx_kernel
===============

JBX-Kernel for Motorola XT910/XT912 OMAP4 (D-WiZ JBX Rom Series - Battery Saving Technologies)



Some features:

- Custom Voltage
- Live Overclock
- GPU Overclock
- GPU frequency control sysfs interface 
- Support for GPU_OC App
- Custom OPP-Table
- Reduced latency
- Underclocked
- Undervolted
- Tweaked governors
- Support for Trickster Mod App (from Gnex)
- Increased R/W Ahead
- Overclocking Kernel Modules based on Milestone Overclock (Ported by Whirleyes) [NOT INCLUDED IN BUILD]
	---> /drivers/extra/ [include this folder in Makefile to build. Needs precompiled kernel!!)


...more to come





WIP:

- Need fix to force the CPU to stay on custom min freq OPP0 (FIXED)
- Live OC function included but doesn't work (FIXED)
- Fixing compile warnings
- USB driver needs a workaround
- Fix hotplugging feature for related governors (sort out bad governors)
- Start fixing build warnings


This Kernel is based on the Motorola 3.0.8 Kernel for Kexec which was initiated by the STS-Dev-Team.
See this link for the original source: https://github.com/STS-Dev-Team/kernel_mapphone_kexec


*Credits to 

- Kholk & [mbm] for Kexec Initial Release
- Hashcode & dhacker29 (STS-Dev-Team) for Kexec Kernel 3.0.31 mr1)
- Surdu_Petru for support and knowledge
