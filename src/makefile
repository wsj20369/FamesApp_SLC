COMPILER    = tcc -c -1 -B -S -ml -w -Ic:\tc\include -I..\..\include
HEADFILE    = common.h config.h keycmd.h order.h ord_edit.h preview.h slc_core.h slc_mntr.h slc_para.h slc_whet.h passwd.h yes_no.h yx_dat.h slc_lb.h cim_link.h io_card.h slc_ctrl.h plc_def.h        

dstfile     = slc.lib

Lib         = $(dstfile)
LibList1    =  start.obj  gui.obj  slc_core.obj  config.obj  tmp_cfg.obj  show_plt.obj  language.obj
LibFile1    = +start.obj +gui.obj +slc_core.obj +config.obj +tmp_cfg.obj +show_plt.obj +language.obj
LibList2    =  preload.obj  welcome.obj  main.obj  keycmd.obj  preview.obj  passwd.obj  yes_no.obj  slc_send.obj
LibFile2    = +preload.obj +welcome.obj +main.obj +keycmd.obj +preview.obj +passwd.obj +yes_no.obj +slc_send.obj
LibList3    =  slc_mntr.obj  order.obj  ord_edit.obj  slc_whet.obj  yx_dat.obj  slc_lb.obj  cim_link.obj
LibFile3    = +slc_mntr.obj +order.obj +ord_edit.obj +slc_whet.obj +yx_dat.obj +slc_lb.obj +cim_link.obj
LibList4    =  io_card.obj  user_reg.obj  version.obj  slc_ctrl.obj  m1_plc.obj  m2_plc.obj  plc.obj  plc_test.obj
LibFile4    = +io_card.obj +user_reg.obj +version.obj +slc_ctrl.obj +m1_plc.obj +m2_plc.obj +plc.obj +plc_test.obj
LibList5    =  slc_malf.obj  slc_para.obj  slc_par2.obj
LibFile5    = +slc_malf.obj +slc_para.obj +slc_par2.obj

Libtemp1    = __tmp1.lib
Libtemp2    = __tmp2.lib
Libtemp3    = __tmp3.lib
Libtemp4    = __tmp4.lib
Libtemp5    = __tmp5.lib

start.obj: start.c $(HEADFILE)
	$(COMPILER) start.c

gui.obj: gui.c $(HEADFILE)
	$(COMPILER) gui.c

preload.obj: preload.c $(HEADFILE)
	$(COMPILER) preload.c

welcome.obj: welcome.c $(HEADFILE)
	$(COMPILER) welcome.c
	
config.obj: config.c $(HEADFILE)
	$(COMPILER) config.c

tmp_cfg.obj: tmp_cfg.c $(HEADFILE)
	$(COMPILER) tmp_cfg.c

order.obj: order.c $(HEADFILE)
	$(COMPILER) order.c

main.obj: main.c $(HEADFILE)
	$(COMPILER) main.c	

keycmd.obj: keycmd.c $(HEADFILE)
	$(COMPILER) keycmd.c	

preview.obj: preview.c $(HEADFILE)
	$(COMPILER) preview.c

ord_edit.obj: ord_edit.c $(HEADFILE)
	$(COMPILER) ord_edit.c

slc_core.obj: slc_core.c $(HEADFILE)
	$(COMPILER) slc_core.c

slc_mntr.obj: slc_mntr.c $(HEADFILE)
	$(COMPILER) slc_mntr.c

slc_para.obj: slc_para.c $(HEADFILE)
	$(COMPILER) slc_para.c

slc_par2.obj: slc_par2.c $(HEADFILE)
	$(COMPILER) slc_par2.c

slc_whet.obj: slc_whet.c $(HEADFILE)
	$(COMPILER) slc_whet.c

slc_lb.obj: slc_lb.c $(HEADFILE)
	$(COMPILER) slc_lb.c

passwd.obj: passwd.c $(HEADFILE)
	$(COMPILER) passwd.c

yes_no.obj: yes_no.c $(HEADFILE)
	$(COMPILER) yes_no.c

yx_dat.obj: yx_dat.c $(HEADFILE)
	$(COMPILER) yx_dat.c

show_plt.obj: show_plt.c $(HEADFILE)
	$(COMPILER) show_plt.c

cim_link.obj: cim_link.c $(HEADFILE)
	$(COMPILER) cim_link.c

io_card.obj: io_card.c $(HEADFILE)
	$(COMPILER) io_card.c

user_reg.obj: user_reg.c $(HEADFILE)
	$(COMPILER) user_reg.c

version.obj: version.c $(HEADFILE)
	$(COMPILER) version.c

slc_ctrl.obj: slc_ctrl.c $(HEADFILE)
	$(COMPILER) slc_ctrl.c

m1_plc.obj: m1_plc.c $(HEADFILE)
	$(COMPILER) m1_plc.c

m2_plc.obj: m2_plc.c $(HEADFILE)
	$(COMPILER) m2_plc.c

plc.obj: plc.c $(HEADFILE)
	$(COMPILER) plc.c

plc_test.obj: plc_test.c $(HEADFILE)
	$(COMPILER) plc_test.c

slc_send.obj: slc_send.c $(HEADFILE)
	$(COMPILER) slc_send.c

slc_malf.obj: slc_malf.c $(HEADFILE)
	$(COMPILER) slc_malf.c

language.obj: language.c $(HEADFILE)
	$(COMPILER) language.c

$(Libtemp1):  $(LibList1)
	del  $(Libtemp1)
	tlib $(Libtemp1) $(LibFile1)

$(Libtemp2):  $(LibList2)
	del  $(Libtemp2)
	tlib $(Libtemp2) $(LibFile2)

$(Libtemp3):  $(LibList3)
	del  $(Libtemp3)
	tlib $(Libtemp3) $(LibFile3)

$(Libtemp4):  $(LibList4)
	del  $(Libtemp4)
	tlib $(Libtemp4) $(LibFile4)

$(Libtemp5):  $(LibList5)
	del  $(Libtemp5)
	tlib $(Libtemp5) $(LibFile5)

$(Lib):  $(LibList1) $(LibList2) $(LibList3) $(LibList4) $(LibList5)
	del  $(Lib)
	tlib $(Lib) +$(Libtemp1) +$(Libtemp2) +$(Libtemp3) +$(Libtemp4) +$(Libtemp5)
	copy $(Lib) ..


