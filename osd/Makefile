SRCS := lib/schrift.c compat.c bitmap.c net.c region.c text.c main.c
BUILD = $(CC) $(SRCS) -I $(SDK)/include -L $(DRV) $(LIB) -Os -s -o $(or $(TARGET),$@)

osd-goke:
	$(eval SDK = ../sdk/gk7205v300)
	$(eval LIB = -ldnvqe -lgk_api -lhi_mpi -lsecurec -lupvqe -lvoice_engine)
	$(BUILD)

osd-hisi:
	$(eval SDK = ../sdk/hi3516ev300)
	$(eval LIB = -ldnvqe -lmpi -lsecurec -lupvqe -lVoiceEngine)
	$(BUILD)

osd-star6b0:
	$(eval SDK = ../sdk/infinity6)
	$(eval LIB = -D__SIGMASTAR__ -D__INFINITY6__ -D__INFINITY6B0__ -lcam_os_wrapper -lm -lmi_rgn -lmi_sys)
	$(BUILD)

osd-star6e:
	$(eval SDK = ../sdk/infinity6)
	$(eval LIB = -D__SIGMASTAR__ -D__INFINITY6__ -D__INFINITY6E__ -lcam_os_wrapper -lm -lmi_rgn -lmi_sys)
	$(BUILD)