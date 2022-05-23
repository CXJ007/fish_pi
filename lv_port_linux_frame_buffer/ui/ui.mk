CSRCS += $(shell find -L $(LVGL_DIR)/ui -path $(LVGL_DIR)/ui/wpa -prune -o  -name "*.c")
#find多了一个文件
TMPVAR := $(CSRCS) 
CSRCS = $(filter-out $(LVGL_DIR)/ui/wpa, $(TMPVAR))

include $(LVGL_DIR)/ui/wpa/wpa.mk