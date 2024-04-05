SRCS := lib/schrift.c compat.c bitmap.c net.c region.c text.c main.c
BUILD = $(CC) $(SRCS) -I $(SDK)/include -L $(DRV) $(LIB) -Os -s -o $(or $(TARGET),$@)

osd-goke:
	$(eval SDK = ../sdk/gk7205v300)
	$(eval LIB = -lhi_mpi -lgk_api -ldnvqe -lupvqe -lvoice_engine -lsecurec)
	$(BUILD)

osd-hisi:
	$(eval SDK = ../sdk/hi3516ev300)
	$(eval LIB = -lmpi -ldnvqe -lupvqe -lVoiceEngine -lsecurec)
	$(BUILD)

osd-star:
	$(eval SDK = ../sdk/infinity6)
	$(eval LIB = -D__SIGMASTAR__ -lcam_os_wrapper -lm -lmi_rgn -lmi_sys)
	$(BUILD)
